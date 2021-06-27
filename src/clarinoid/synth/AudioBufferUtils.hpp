
#pragma once

namespace clarinoid
{

// 16-bit PCM gaining is done by multiplying using signed_multiply_32x16b, which requires the multiplier to be saturated
// to 32-bits. this does this.
static inline int32_t gainToSignedMultiply32x16(float n)
{
    if (n > 32767.0f)
        n = 32767.0f;
    else if (n < -32767.0f)
        n = -32767.0f;
    return n * 65536.0f;
}

// takes a single input buffer
// and copies it to N output buffers, with a per-output multiplier in the process.
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

// takes a single input buffer, copies to 1 output buffer with float multiplier.
inline static void audioBufferCopyAndApplyGain(int16_t *inp, int16_t *output, float mul)
{
    uint32_t *pInp32 = (uint32_t *)inp;
    int32_t mul32 = gainToSignedMultiply32x16(mul);
    for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES / 2; ++i) // because we read 2 samples at a time, count in DWORDs
    {
        uint32_t tmp32 = pInp32[i]; // reads 2 packed samples.

        int32_t val1 = signed_multiply_32x16b(mul32, tmp32);
        int32_t val2 = signed_multiply_32x16t(mul32, tmp32);
        val1 = signed_saturate_rshift(val1, 16, 0);
        val2 = signed_saturate_rshift(val2, 16, 0);
        uint32_t *pOutp32 = (uint32_t *)output;
        pOutp32[i] = pack_16b_16b(val2, val1);
    }
}

// simply adds 16-bit PCM samples, data = data + in.
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

// simple memcpy
inline static void audioBufferCopy(int16_t *out /* in/out */, const int16_t *in)
{
    uint32_t *out32 = (uint32_t *)out;
    const uint32_t *in32 = (const uint32_t *)in;
    for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES / 2; ++i)
    {
        out32[i] = in32[i];
    }
}

// this does effectively DATA += (IN * MULT)
// useful for adding a signal to another one with gain applied.
// copied from applyGainThenAdd
static void audioBufferMixInPlaceWithGain(int16_t *data, const int16_t *in, int32_t mult)
{
    static constexpr int32_t MULTI_UNITYGAIN = 65536;
    uint32_t *dst = (uint32_t *)data;
    const uint32_t *src = (uint32_t *)in;
    const uint32_t *end = (uint32_t *)(data + AUDIO_BLOCK_SAMPLES);

    if (mult == MULTI_UNITYGAIN)
    {
        do
        {
            uint32_t tmp32 = *dst;
            *dst++ = signed_add_16_and_16(tmp32, *src++);
            tmp32 = *dst;
            *dst++ = signed_add_16_and_16(tmp32, *src++);
        } while (dst < end);
    }
    else
    {
        do
        {
            uint32_t tmp32 = *src++; // read 2 samples from *data
            int32_t val1 = signed_multiply_32x16b(mult, tmp32);
            int32_t val2 = signed_multiply_32x16t(mult, tmp32);
            val1 = signed_saturate_rshift(val1, 16, 0);
            val2 = signed_saturate_rshift(val2, 16, 0);
            tmp32 = pack_16b_16b(val2, val1);
            uint32_t tmp32b = *dst;
            *dst++ = signed_add_16_and_16(tmp32, tmp32b);
        } while (dst < end);
    }
}

} // namespace clarinoid
