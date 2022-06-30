
#pragma once

#include "AudioBufferUtils.hpp"

namespace clarinoid
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <size_t NInputs>
struct MultiMixerNode : public AudioStream
{
    MultiMixerNode() : AudioStream(NInputs, inputQueueArray)
    {
    }

    virtual void update() override
    {
        audio_block_t *out = nullptr;

        for (size_t ichannel = 0; ichannel < NInputs; ++ichannel)
        {
            if (!out)
            {
                // take the 1st connected channel as writable. no mixing yet to be applied.
                out = receiveWritable(ichannel); // may return null if not connected!
            }
            else
            {
                // subsequent iterations, receive read only & apply to out buffer.
                audio_block_t *in = receiveReadOnly(ichannel);
                if (in)
                {
                    audioBufferMixInPlace(out->data, in->data);
                    release(in);
                }
            }
        }

        if (out)
        {
            transmit(out);
            release(out);
        }
    }

    audio_block_t *inputQueueArray[NInputs];
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// takes N inputs, applies pan and mixes them all together to create a single pair of L R output
template <size_t NInputs>
struct MultiMixerPannerNode : public AudioStream
{
    audio_block_t *inputQueueArray[NInputs];

    float mInputPanN11[NInputs];
    float mInputGain01[NInputs];
    int32_t mInputMultipliersLeft[NInputs]; // bakes inputpan into L/R multipliers for each input. After this multiply,
                                            // the samples can be added together for a single L/R channel
    int32_t mInputMultipliersRight[NInputs];
    bool mEnabled[NInputs];

    MultiMixerPannerNode() : AudioStream(NInputs, inputQueueArray)
    {
        for (size_t i = 0; i < NInputs; ++i)
        {
            mEnabled[i] = true;
            mInputPanN11[i] = 0;
            mInputGain01[i] = 0;
            RecalcInput(i);
        }
    }

    void RecalcInput(size_t inputChannel)
    {
        CCASSERT(inputChannel < SizeofStaticArray(mInputPanN11));

        if (!mEnabled[inputChannel])
        {
            mInputMultipliersLeft[inputChannel] = 0;
            mInputMultipliersRight[inputChannel] = 0;
            return;
        }

        float panN11 = mInputPanN11[inputChannel];
        float gain01 = mInputGain01[inputChannel];

        // SQRT pan law
        // -1..+1  -> 1..0
        float normPan = (-panN11 + 1) / 2;
        float leftChannel = sqrtf(normPan);
        float rightChannel = sqrtf(1.0f - normPan);

        mInputMultipliersLeft[inputChannel] = gainToSignedMultiply32x16(leftChannel * gain01);
        mInputMultipliersRight[inputChannel] = gainToSignedMultiply32x16(rightChannel * gain01);
    }

    void SetInputPanGainAndEnabled(size_t inputChannel, float panN11, float gain01, bool enabled)
    {
        if (panN11 < -1)
            panN11 = -1;
        if (panN11 > 1)
            panN11 = 1;
        if ((mEnabled[inputChannel] == enabled) && FloatEquals(panN11, mInputPanN11[inputChannel]) && FloatEquals(gain01, mInputGain01[inputChannel]))
        {
            return;
        }
        mEnabled[inputChannel] = enabled;
        mInputPanN11[inputChannel] = panN11;
        mInputGain01[inputChannel] = gain01;
        RecalcInput(inputChannel);
    }

    virtual void update() override
    {
        // get inputs & output buffers
        audio_block_t *inputBufs[NInputs] = {nullptr};
        for (size_t i = 0; i < NInputs; ++i)
        {
            inputBufs[i] = this->receiveReadOnly(i);
        }

        audio_block_t *outputBufferL = this->allocate();
        audio_block_t *outputBufferR = this->allocate();

        if (outputBufferL && outputBufferR)
        {
            uint32_t *pOutp32L = (uint32_t *)(outputBufferL->data);
            uint32_t *pOutp32R = (uint32_t *)(outputBufferR->data);

            // because we read 2 samples at a time, count in DWORDs
            for (size_t iSample32 = 0; iSample32 < AUDIO_BLOCK_SAMPLES / 2; ++iSample32)
            {
                // we go through each input buffer, apply multipliers, and end up with these intermediate samples.
                uint32_t intermediateL = 0;
                uint32_t intermediateR = 0;
                for (size_t iInput = 0; iInput < NInputs; ++iInput)
                {
                    if (!inputBufs[iInput])
                    {
                        continue;
                    }
                    uint32_t *pInp32 = (uint32_t *)(inputBufs[iInput]->data);

                    uint32_t l = pInp32[iSample32]; // reads 2 packed samples.

                    auto mul = mInputMultipliersRight[iInput];
                    int32_t val1 = signed_multiply_32x16b(mul,
                                                          l); // applies mult to the 1st packed sample, as 32-bit val
                    int32_t val2 = signed_multiply_32x16t(mul,
                                                          l);   // applies mult to the 2nd packed sample, as 32-bit val
                    val1 = signed_saturate_rshift(val1, 16, 0); // 1st packed sample back to 16-bit
                    val2 = signed_saturate_rshift(val2, 16, 0); // 2nd packed sample back to 16-bit
                    uint32_t r = pack_16b_16b(val2, val1);      // re-pack samples for output

                    // now process left panning
                    mul = mInputMultipliersLeft[iInput];
                    val1 = signed_multiply_32x16b(mul,
                                                  l); // applies mult to the 1st packed sample, as 32-bit val
                    val2 = signed_multiply_32x16t(mul,
                                                  l);           // applies mult to the 2nd packed sample, as 32-bit val
                    val1 = signed_saturate_rshift(val1, 16, 0); // 1st packed sample back to 16-bit
                    val2 = signed_saturate_rshift(val2, 16, 0); // 2nd packed sample back to 16-bit
                    l = pack_16b_16b(val2, val1);               // re-pack samples

                    // mix together input channels
                    intermediateL = signed_add_16_and_16(intermediateL, l);
                    intermediateR = signed_add_16_and_16(intermediateR, r);
                }

                pOutp32L[iSample32] = intermediateL;
                pOutp32R[iSample32] = intermediateR;
            }
        } // if valid output buffers

        // transmit & release input & output buffers
        for (size_t i = 0; i < NInputs; ++i)
        {
            if (inputBufs[i])
                this->release(inputBufs[i]);
        }

        if (outputBufferL)
        {
            this->transmit(outputBufferL, 0);
            this->release(outputBufferL);
        }
        if (outputBufferR)
        {
            this->transmit(outputBufferR, 1);
            this->release(outputBufferR);
        }
    }
};

// takes 2 inputs, outputs 2*NSplits, with gain applied to each output channel
template <size_t NSplits>
struct StereoGainerSplitterNode : public AudioStream
{
    audio_block_t *inputQueueArray[2];

    float mGain01[NSplits];
    int32_t mMultipliers[NSplits];

    StereoGainerSplitterNode() : AudioStream(2, inputQueueArray)
    {
        for (size_t i = 0; i < NSplits; ++i)
        {
            SetOutputGainUnchecked(i, 0);
        }
    }

    void SetOutputGainUnchecked(size_t chan, float gain01)
    {
        CCASSERT(chan < SizeofStaticArray(mGain01));
        mGain01[chan] = gain01;
        mMultipliers[chan] = gainToSignedMultiply32x16(gain01);
    }

    void SetOutputGain(size_t chan, float gain01)
    {
        if (gain01 < 0)
            gain01 = 0;
        if (FloatEquals(gain01, mGain01[chan]))
        {
            return;
        }
        SetOutputGainUnchecked(chan, gain01);
    }

    virtual void update() override
    {
        audio_block_t *inputBufL = receiveReadOnly(0);
        if (!inputBufL)
        {
            return;
        }

        audio_block_t *inputBufR = receiveReadOnly(1);
        if (!inputBufR)
        {
            release(inputBufL);
            return;
        }

        audio_block_t *outputBuffers[NSplits * 2] = {nullptr};
        for (size_t i = 0; i < SizeofStaticArray(outputBuffers); ++i)
        {
            outputBuffers[i] = allocate();
        }

        uint32_t *pInp32L = (uint32_t *)(inputBufL->data);
        uint32_t *pInp32R = (uint32_t *)(inputBufR->data);
        // because we read 2 samples at a time, count in DWORDs
        for (size_t iSample32 = 0; iSample32 < AUDIO_BLOCK_SAMPLES / 2; ++iSample32)
        {
            for (size_t iSplit = 0; iSplit < NSplits; ++iSplit)
            {
                if (!outputBuffers[iSplit * 2] || !outputBuffers[(iSplit * 2) + 1])
                {
                    continue;
                }
                auto mul = mMultipliers[iSplit];

                // do left channel
                uint32_t tmp32 = pInp32L[iSample32]; // reads 2 packed samples.
                int32_t val1 = signed_multiply_32x16b(mul, tmp32);
                int32_t val2 = signed_multiply_32x16t(mul, tmp32);
                val1 = signed_saturate_rshift(val1, 16, 0);
                val2 = signed_saturate_rshift(val2, 16, 0);
                uint32_t *pOutp32L = (uint32_t *)(outputBuffers[iSplit * 2]->data);
                pOutp32L[iSample32] = pack_16b_16b(val2, val1);

                // do right channel
                tmp32 = pInp32R[iSample32]; // reads 2 packed samples.
                val1 = signed_multiply_32x16b(mul, tmp32);
                val2 = signed_multiply_32x16t(mul, tmp32);
                val1 = signed_saturate_rshift(val1, 16, 0);
                val2 = signed_saturate_rshift(val2, 16, 0);
                uint32_t *pOutp32R = (uint32_t *)(outputBuffers[(iSplit * 2) + 1]->data);
                pOutp32R[iSample32] = pack_16b_16b(val2, val1);
            }
        }

        release(inputBufL);
        release(inputBufR);

        for (size_t i = 0; i < SizeofStaticArray(outputBuffers); ++i)
        {
            if (!outputBuffers[i])
                continue;
            transmit(outputBuffers[i], i);
            release(outputBuffers[i]);
        }
    }
};

} // namespace clarinoid
