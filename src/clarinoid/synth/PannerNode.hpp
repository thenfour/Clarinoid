// PannerNode
// GainAndPanSplitterNode
// MultiMixer

#pragma once

#include <algorithm>
#include <cfloat>

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/loopstation/LoopstationMemory.hpp>
#include <clarinoid/synth/filters/filters.hpp>

namespace clarinoid
{
static inline int32_t gainToSignedMultiply32x16(float n)
{
    if (n > 32767.0f)
        n = 32767.0f;
    else if (n < -32767.0f)
        n = -32767.0f;
    return n * 65536.0f;
}

template <size_t NOutputs>
inline static void audioBufferCopyAndApplyGainMulti(int16_t *inp,
                                                    int16_t *outputs[NOutputs],
                                                    int32_t multipliers[NOutputs])
{
    uint32_t *pInp32 = (uint32_t *)inp;
    for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES / 2; ++i) // because we read 2 samples at a time, count in DWORDs
    {
        uint32_t tmp32 = pInp32[i]; // reads 2 packed samples.

        for (size_t io = 0; io < NOutputs; ++io)
        {
            int32_t val1 = signed_multiply_32x16b(multipliers[io], tmp32);
            int32_t val2 = signed_multiply_32x16t(multipliers[io], tmp32);
            val1 = signed_saturate_rshift(val1, 16, 0);
            val2 = signed_saturate_rshift(val2, 16, 0);
            uint32_t *pOutp32 = (uint32_t *)outputs[io];
            pOutp32[i] = pack_16b_16b(val2, val1);
        }
    }
}

inline static void audioBufferMixInPlace(int16_t *data /* in/out */, const int16_t *in)
{
    uint32_t *dst = (uint32_t *)data;
    const uint32_t *src = (uint32_t *)in;
    const uint32_t *end = (uint32_t *)(data + AUDIO_BLOCK_SAMPLES);

    do
    {
        uint32_t tmp32 = *dst;
        *dst++ = signed_add_16_and_16(tmp32, *src++);
        tmp32 = *dst;
        *dst++ = signed_add_16_and_16(tmp32, *src++);
    } while (dst < end);
}

// inline static void audioBufferCopyAndApplyGainTwice(int16_t *inp,
//                                                     int16_t *outp1,
//                                                     int32_t mult1,
//                                                     int16_t *outp2,
//                                                     int32_t mult2)
// {
//     uint32_t *pIn = (uint32_t *)inp;
//     uint32_t *pOut1 = (uint32_t *)outp1;
//     uint32_t *pOut2 = (uint32_t *)outp2;
//     const uint32_t *inpEnd = (uint32_t *)(inp + AUDIO_BLOCK_SAMPLES);

//     do
//     {
//         uint32_t tmp32 = *pIn; // read 2 samples from *inp
//         int32_t val1 = signed_multiply_32x16b(mult1, tmp32);
//         int32_t val2 = signed_multiply_32x16t(mult1, tmp32);
//         val1 = signed_saturate_rshift(val1, 16, 0);
//         val2 = signed_saturate_rshift(val2, 16, 0);
//         *pOut1++ = pack_16b_16b(val2, val1);

//         val1 = signed_multiply_32x16b(mult2, tmp32);
//         val2 = signed_multiply_32x16t(mult2, tmp32);
//         val1 = signed_saturate_rshift(val1, 16, 0);
//         val2 = signed_saturate_rshift(val2, 16, 0);
//         *pOut2++ = pack_16b_16b(val2, val1);

//         ++pIn;
//     } while (pIn < inpEnd);
// }

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// struct PannerNode : public AudioStream
// {
//     PannerNode() : AudioStream(1, inputQueueArray)
//     {
//         SetPan(0);
//     }

//     // -1 = left, 1 = right
//     void SetPan(float n11)
//     {
//         if (n11 < -1)
//             n11 = -1;
//         if (n11 > 1)
//             n11 = 1;
//         if (FloatEquals(n11, mPanN11, 0.00001))
//         {
//             return;
//         }

//         mPanN11 = n11;

//         // SQRT pan law
//         // -1..+1  -> 1..0
//         float normPan = (-n11 + 1) / 2;
//         float leftChannel = sqrtf(normPan);
//         float rightChannel = sqrtf(1.0f - normPan);

//         mMultiplierLeft = gainToSignedMultiply32x16(leftChannel);
//         mMultiplierRight = gainToSignedMultiply32x16(rightChannel);

//         // Serial.println(String("pan ") + n11 + " -> norm=" + normPan + " lgain=" +
//         // leftChannel + " rgain=" + rightChannel + " lmult32=" + mMultiplierLeft +
//         // " rmult32=" + mMultiplierRight);
//     }

//     float mPanN11 = -10.0; // so ctor will initialize properly.
//     int32_t mMultiplierLeft;
//     int32_t mMultiplierRight;

//     virtual void update() override
//     {
//         audio_block_t *inputBuf = receiveReadOnly();
//         if (!inputBuf)
//             return;
//         audio_block_t *outputBufLeft = allocate();
//         audio_block_t *outputBufRight = allocate();

//         audioBufferCopyAndApplyGainTwice(
//             inputBuf->data, outputBufLeft->data, mMultiplierLeft, outputBufRight->data, mMultiplierRight);

//         transmit(outputBufLeft, 0);
//         transmit(outputBufRight, 1);
//         release(outputBufLeft);
//         release(outputBufRight);
//         release(inputBuf);
//     }

//     audio_block_t *inputQueueArray[1];
// };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Takes a single mono input, splits into N outputs * 2 (stereo), with gain &
// pan applied to each split individually.
template <size_t NSplits>
struct GainAndPanSplitterNode : public AudioStream
{
    float mPanN11[NSplits];
    float mGain01[NSplits];
    static constexpr size_t OutputBufferCount = NSplits * 2;
    int32_t mMultipliers[OutputBufferCount];

    GainAndPanSplitterNode() : AudioStream(1, inputQueueArray)
    {
        for (size_t i = 0; i < NSplits; ++i)
        {
            SetPanAndGainUnchecked(i, 0, 0);
        }
    }

    void SetPanAndGainUnchecked(size_t chan, float gain01, float panN11)
    {
        CCASSERT(chan < SizeofStaticArray(mPanN11));
        mGain01[chan] = gain01;
        mPanN11[chan] = panN11;

        // SQRT pan law
        // -1..+1  -> 1..0
        float normPan = (-panN11 + 1) / 2;
        float leftChannel = sqrtf(normPan);
        float rightChannel = sqrtf(1.0f - normPan);

        mMultipliers[chan * 2] = gainToSignedMultiply32x16(leftChannel * gain01);
        mMultipliers[(chan * 2) + 1] = gainToSignedMultiply32x16(rightChannel * gain01);
    }

    void SetPanAndGain(size_t chan, float gain01, float panN11)
    {
        if (panN11 < -1)
            panN11 = -1;
        if (panN11 > 1)
            panN11 = 1;
        if (gain01 < 0)
            gain01 = 0;
        if (FloatEquals(panN11, mPanN11[chan], 0.00001f) && FloatEquals(gain01, mGain01[chan], 0.00001f))
        {
            return;
        }
        SetPanAndGainUnchecked(chan, gain01, panN11);
    }

    virtual void update() override
    {
        audio_block_t *inputBuf = receiveReadOnly();
        if (!inputBuf)
            return;

        audio_block_t *outputBuffers[OutputBufferCount] = {nullptr};
        int16_t *outputDataBuffers[OutputBufferCount] = {nullptr};
        for (size_t i = 0; i < SizeofStaticArray(outputBuffers); ++i)
        {
            outputBuffers[i] = allocate();
            CCASSERT(outputBuffers[i] && outputBuffers[i]->data);
            outputDataBuffers[i] = outputBuffers[i]->data;
        }

        audioBufferCopyAndApplyGainMulti<OutputBufferCount>(inputBuf->data, outputDataBuffers, mMultipliers);

        release(inputBuf);

        for (size_t i = 0; i < SizeofStaticArray(outputBuffers); ++i)
        {
            transmit(outputBuffers[i], i);
            release(outputBuffers[i]);
        }
    }

    audio_block_t *inputQueueArray[1];
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <size_t NInputs>
struct MultiMixer : public AudioStream
{
    MultiMixer() : AudioStream(NInputs, inputQueueArray)
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
// handles modulation routing (which modulation sources apply to which destinations) for VOICES.
// inputs are modulation sources,
// destinations are modulation destinations.
// 
struct VoiceModulationMatrixNode : public AudioStream
{
    audio_block_t *inputQueueArray[ModulationSourceViableCount];
    int16_t mBaseValues[ModulationSourceViableCount];
    SynthPreset& mSynthPatch;

    VoiceModulationMatrixNode(SynthPreset& patch) :
        AudioStream(ModulationSourceViableCount, inputQueueArray),
        mSynthPatch(patch)
    {
    }

    void SetBaseValues(float baseValues[ModulationSourceViableCount]) {
        //
    }

    virtual void update() override
    {
        // audio_block_t * sources[ModulationSourceViableCount] = {nullptr};
        // audio_block_t * destination[ModulationSourceViableCount] = {nullptr};

        // for (auto& modulation : mSynthPatch.mModulations) {
        //     // switch (modulation.mSource) {
        //     //     //
        //     // }
        // }
    }
};

} // namespace clarinoid
