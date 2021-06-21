
#pragma once

#include <algorithm>
#include <cfloat>

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/loopstation/LoopstationMemory.hpp>
#include <clarinoid/synth/filters/filters.hpp>

namespace clarinoid
{
	int32_t gainToSignedMultiply32x16(float n) {
		if (n > 32767.0f) n = 32767.0f;
		else if (n < -32767.0f) n = -32767.0f;
		return n * 65536.0f;
	}

    inline static void audioBufferCopyAndApplyGainTwice(int16_t *inp, int16_t* outp1, int32_t mult1, int16_t* outp2, int32_t mult2)
    {
        uint32_t *pIn = (uint32_t *)inp;
        uint32_t *pOut1 = (uint32_t *)outp1;
        uint32_t *pOut2 = (uint32_t *)outp2;
        const uint32_t *inpEnd = (uint32_t *)(inp + AUDIO_BLOCK_SAMPLES);

        do {
            uint32_t tmp32 = *pIn; // read 2 samples from *inp
            int32_t val1 = signed_multiply_32x16b(mult1, tmp32);
            int32_t val2 = signed_multiply_32x16t(mult1, tmp32);
            val1 = signed_saturate_rshift(val1, 16, 0);
            val2 = signed_saturate_rshift(val2, 16, 0);
            *pOut1++ = pack_16b_16b(val2, val1);

            val1 = signed_multiply_32x16b(mult2, tmp32);
            val2 = signed_multiply_32x16t(mult2, tmp32);
            val1 = signed_saturate_rshift(val1, 16, 0);
            val2 = signed_saturate_rshift(val2, 16, 0);
            *pOut2++ = pack_16b_16b(val2, val1);

            ++ pIn;
        } while (pIn < inpEnd);
    }

    struct PannerNode : public AudioStream
    {
        PannerNode() : AudioStream(1, inputQueueArray)
        {
            SetPan(0);
        }

        // -1 = left, 1 = right
        void SetPan(float n11)
        {
            if (n11 < -1) n11 = -1;
            if (n11 > 1) n11 = 1;
            if (FloatEquals(n11, mPanN11, 0.00001)) {
                return;
            }

            mPanN11 = n11;

            // SQRT pan law
            // -1..+1  -> 1..0
            float normPan = (-n11 + 1) / 2;
            float leftChannel = sqrtf(normPan);
            float rightChannel = sqrtf(1.0f - normPan);

            mMultiplierLeft = gainToSignedMultiply32x16(leftChannel);
            mMultiplierRight = gainToSignedMultiply32x16(rightChannel);

            //Serial.println(String("pan ") + n11 + " -> norm=" + normPan + " lgain=" + leftChannel + " rgain=" + rightChannel + " lmult32=" + mMultiplierLeft + " rmult32=" + mMultiplierRight);
        }

        float mPanN11 = 0.0f; // center
        int32_t mMultiplierLeft;
        int32_t mMultiplierRight;

        virtual void update() override
        {
            audio_block_t *inputBuf = receiveReadOnly();
            if (!inputBuf)
                return;
            audio_block_t *outputBufLeft = allocate();
            audio_block_t *outputBufRight = allocate();

            audioBufferCopyAndApplyGainTwice(inputBuf->data, outputBufLeft->data, mMultiplierLeft, outputBufRight->data, mMultiplierRight);

            // for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++ i) {
            //     outputBufLeft->data[i] = inputBuf->data[i];
            //     outputBufRight->data[i] = inputBuf->data[i];
            // }

            transmit(outputBufLeft, 0);
            transmit(outputBufRight, 1);
            release(outputBufLeft);
            release(outputBufRight);
            release(inputBuf);
        }

        audio_block_t *inputQueueArray[1];
    };

} // namespace clarinoid
