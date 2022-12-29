// smooth square

// - consider smoothing k-rate params across the buffer; especially mod amounts
// - unfortunately, sync has artifacting, probably requires adaptation of the bandlimiting code. in many cases we might
// be able to use a BLEP

#pragma once

#include <Arduino.h>
#include <AudioStream.h>
//#include <synth_waveform.h>
#include <dspinst.h>
#include <arm_math.h>
#include <array>

#include "boiler.hpp"

// 16-bit PCM gaining is done by multiplying using signed_multiply_32x16b, which requires the multiplier to be saturated
// to 32-bits. this does this.
static inline int32_t gainToSignedMultiply32x16(float n)
{
    if (n > 32767.0f)
        n = 32767.0f;
    else if (n < -32767.0f)
        n = -32767.0f;
    return (int32_t)(n * 65536.0f);
}

static const int16_t gZeroAudioBuffer[AUDIO_BLOCK_SAMPLES] = {0}; // sentinel buffer when no modulation data exists.

static constexpr int16_t gAudioWaveformSine[257] = {
    0,      804,    1608,   2410,   3212,   4011,   4808,   5602,   6393,   7179,   7962,   8739,   9512,   10278,
    11039,  11793,  12539,  13279,  14010,  14732,  15446,  16151,  16846,  17530,  18204,  18868,  19519,  20159,
    20787,  21403,  22005,  22594,  23170,  23731,  24279,  24811,  25329,  25832,  26319,  26790,  27245,  27683,
    28105,  28510,  28898,  29268,  29621,  29956,  30273,  30571,  30852,  31113,  31356,  31580,  31785,  31971,
    32137,  32285,  32412,  32521,  32609,  32678,  32728,  32757,  32767,  32757,  32728,  32678,  32609,  32521,
    32412,  32285,  32137,  31971,  31785,  31580,  31356,  31113,  30852,  30571,  30273,  29956,  29621,  29268,
    28898,  28510,  28105,  27683,  27245,  26790,  26319,  25832,  25329,  24811,  24279,  23731,  23170,  22594,
    22005,  21403,  20787,  20159,  19519,  18868,  18204,  17530,  16846,  16151,  15446,  14732,  14010,  13279,
    12539,  11793,  11039,  10278,  9512,   8739,   7962,   7179,   6393,   5602,   4808,   4011,   3212,   2410,
    1608,   804,    0,      -804,   -1608,  -2410,  -3212,  -4011,  -4808,  -5602,  -6393,  -7179,  -7962,  -8739,
    -9512,  -10278, -11039, -11793, -12539, -13279, -14010, -14732, -15446, -16151, -16846, -17530, -18204, -18868,
    -19519, -20159, -20787, -21403, -22005, -22594, -23170, -23731, -24279, -24811, -25329, -25832, -26319, -26790,
    -27245, -27683, -28105, -28510, -28898, -29268, -29621, -29956, -30273, -30571, -30852, -31113, -31356, -31580,
    -31785, -31971, -32137, -32285, -32412, -32521, -32609, -32678, -32728, -32757, -32767, -32757, -32728, -32678,
    -32609, -32521, -32412, -32285, -32137, -31971, -31785, -31580, -31356, -31113, -30852, -30571, -30273, -29956,
    -29621, -29268, -28898, -28510, -28105, -27683, -27245, -26790, -26319, -25832, -25329, -24811, -24279, -23731,
    -23170, -22594, -22005, -21403, -20787, -20159, -19519, -18868, -18204, -17530, -16846, -16151, -15446, -14732,
    -14010, -13279, -12539, -11793, -11039, -10278, -9512,  -8739,  -7962,  -7179,  -6393,  -5602,  -4808,  -4011,
    -3212,  -2410,  -1608,  -804,   0};

// Fixed-point per-sample transitioning of a value.
// T can be signed or unsigned.
template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
struct PortamentoCalcFP
{
    using Tvalue = T;
    using Tdelta = typename std::make_signed<T>::type; // to support negatives

    // caller params
    T mTargetValue = 0;
    float mDurationMS = 0;

    // precalcs
    uint32_t mDurationSamples = 0;
    Tdelta mValueDeltaPerSample = 0;

    // state
    T mCurrentValue = 0;

    static constexpr float gMaxDurationMS =
        std::numeric_limits<T>::max() / AUDIO_SAMPLE_RATE; // because of how stuff is calculated there's a max.

    void SetParams(T targetValue, float durationMS)
    {
        if (targetValue == mTargetValue && mDurationSamples == durationMS)
            return;
        mTargetValue = targetValue;
        mDurationMS = std::min(gMaxDurationMS, durationMS);

        mDurationSamples = mDurationMS * AUDIO_SAMPLE_RATE_EXACT / 1000;
        if (mDurationSamples < 1)
        {
            mValueDeltaPerSample = 0;
            return;
        }
        mValueDeltaPerSample = (targetValue - mCurrentValue) / mDurationSamples;
    }

    T SampleStep()
    {
        if (mDurationSamples <= 0)
        {
            return mCurrentValue; // reach target
        }

        // slide it
        mCurrentValue += mValueDeltaPerSample;
        mDurationSamples--;
        return mCurrentValue;
    }
};

using PortamentoCalcU32 = PortamentoCalcFP<uint32_t>;

struct PhaseAccumulator32
{
    uint32_t mAccumulator = 0;
    uint32_t mIncrement = 0;

    void frequency(float freq)
    {
        if (freq < 0.0f)
        {
            freq = 0.0;
        }
        else if (freq > AUDIO_SAMPLE_RATE_EXACT / 2.0f)
        {
            freq = AUDIO_SAMPLE_RATE_EXACT / 2.0f;
        }

        mIncrement = freq * (4294967296.0f / AUDIO_SAMPLE_RATE_EXACT);
        if (mIncrement > 0x7FFE0000u)
            mIncrement = 0x7FFE0000;
    }
    void ResetPhase()
    {
        // todo: band-limit ?
        mAccumulator = 0;
    }

    bool OnSample()
    {
        auto oldAcc = mAccumulator;
        mAccumulator = mAccumulator + mIncrement;
        return mAccumulator < oldAcc; // detect overflow
    }
};

template <size_t OscillatorCount>
struct Oscillator
{
    using MyT = Oscillator<OscillatorCount>;
    size_t mMyIndex;

    void SetParams(float mainFreq, float syncFreq, bool syncEnable, OscWaveformShape waveformType)
    {
        mHardSyncEnabled = syncEnable;
        uint32_t blincrement = 0;
        if (syncEnable)
        {
            mMainPhase.frequency(syncFreq);
            mSyncPhase.frequency(mainFreq);
            blincrement = mSyncPhase.mIncrement;
        }
        else
        {
            mMainPhase.frequency(mainFreq);
            blincrement = mMainPhase.mIncrement;
        }

        if (waveformType != tone_type)
        {
            if (waveformType == OscWaveformShape::Pulse_Bandlimited)
                band_limit_waveform.init_pulse(blincrement, 0x80000000u);
            else if (waveformType == OscWaveformShape::Saw_Bandlimited ||
                     waveformType == OscWaveformShape::SawRev_Bandlimited)
                band_limit_waveform.init_sawtooth(blincrement);
        }
        tone_type = waveformType;

        switch (tone_type)
        {
        case OscWaveformShape::Sine:
            mCurrentSampleProc = &MyT::SampleSine;
            break;
        case OscWaveformShape::Harmonics:
            mCurrentSampleProc = &MyT::SampleHarmonics;
            break;
        case OscWaveformShape::Arbitrary:
            mCurrentSampleProc = &MyT::SampleArbitrary;
            break;
        case OscWaveformShape::Pulse:
            mCurrentSampleProc = &MyT::SamplePulse;
            break;
        case OscWaveformShape::Pulse_Bandlimited:
            mCurrentSampleProc = &MyT::SampleBandlimitPulse;
            break;
        case OscWaveformShape::Saw:
            mCurrentSampleProc = &MyT::SampleSawtooth;
            break;
        case OscWaveformShape::SawRev:
            mCurrentSampleProc = &MyT::SampleSawtoothReverse;
            break;
        case OscWaveformShape::Saw_Bandlimited:
            mCurrentSampleProc = &MyT::SampleBandlimitSawtooth;
            break;
        case OscWaveformShape::Saw_Bandlimited2:
            mCurrentSampleProc = &MyT::SampleBandlimitSawtooth2;
            break;
        case OscWaveformShape::SawRev_Bandlimited:
            mCurrentSampleProc = &MyT::SampleBandlimitSawtoothReverse;
            break;
        case OscWaveformShape::VarTriangle:
            mCurrentSampleProc = &MyT::SampleVarTriangle;
            break;
        case OscWaveformShape::Tri2_Bandlimited:
            mCurrentSampleProc = &MyT::SampleBandlimitTriangle2;
            break;
        case OscWaveformShape::Tri2:
            mCurrentSampleProc = &MyT::SampleTriangle2;
            break;
        case OscWaveformShape::Noise:
            mCurrentSampleProc = &MyT::SampleSH;
            break;
        default:
            return;
        }
    }

    bool IsHardSyncEnabled() const
    {
        return mHardSyncEnabled;
    }

    void arbitraryWaveform(const int16_t *data /* 256-element cycle */)
    {
        arbdata = data;
    }

    std::array<int16_t, OscillatorCount + 1> mFMMatrix; // 0-32767 (yes i don't use 1 bit; it goes to signed operation)

    void SetFMMatrix(size_t iFromOsc, float amt)
    {
        if (iFromOsc == mMyIndex)
        {
            // FM feedback is a very different effect than from other oscillators.
            // its scale needs to be treated.
            amt *= 0.25;
        }
        amt = Clamp(amt, 0, 1);
        uint16_t iamt = amt * 32767;
        mFMMatrix[iFromOsc] = iamt;
    }

    // currentOscSamples holds all oscillator previous samples. It always holds an even # of samples (last one can be 0
    // in the case of odd # of oscillators)
    int16_t ProcessSample(int iOsc, int iSample, const int16_t *currentOscSamples)
    {
        if (!mCurrentSampleProc)
            return 0;

        const uint32_t *currentOscSamples32 = (const uint32_t *)currentOscSamples;
        const uint32_t *fmMatrix32 = (const uint32_t *)&mFMMatrix[0];

        // because we're doing a bunch of ops in 1 optimized (multiply_accumulate_16tx16t_add_16bx16b),
        // we can't control the scaling of PM so much. By default, 1:1, the effect is too subtle
        // so we will shift phase, apply the PM, then shift it back. That effectively multiplies the
        // effect of PM.
        static constexpr int gPhaseBitShift = 4;
        uint64_t phase = mMainPhase.mAccumulator >> gPhaseBitShift;

        // process 2 oscillators at a time. actually this would be optimized for a 4-op FM synth but
        // i just don't want to deal with 4 ops.
        for (size_t i = 0; i < OscillatorCount / 2; ++i)
        {
            // phase += currentOscSamples[i] * mFMMatrix[i];
            phase = multiply_accumulate_16tx16t_add_16bx16b(phase, currentOscSamples32[i], fmMatrix32[i]);
        }
        if (OscillatorCount % 1 == 1)
        {
            // for odd # of oscillators; do phase mod for the remaining one.
            phase += currentOscSamples[OscillatorCount - 1] * mFMMatrix[OscillatorCount - 1];
        }

        phase <<= gPhaseBitShift; // see above.

        mMainPhase.OnSample();
        if (mHardSyncEnabled && mSyncPhase.OnSample())
        {
            mMainPhase.mAccumulator = mSyncPhase.mAccumulator;
        }

        uint32_t phase32 = phase & 0xffffffff;

        int32_t sample = (this->*mCurrentSampleProc)(iSample, phase32, 0);
        mFBN1 = sample;
        priorphase = phase32;
        return sample;
    }

    int16_t SampleSine(int i, uint32_t ph, int16_t shapeMod)
    {
        return SineLUT(ph);
    }

    int16_t SineLUT(uint32_t ph)
    {
        int32_t val1, val2;
        uint32_t index, scale;
        index = ph >> 24;                 // 8 top bits = index to LUT
        val1 = gAudioWaveformSine[index]; // -32768...32767
        val2 = gAudioWaveformSine[index + 1];
        scale = (ph >> 8) & 0xFFFF; // 16 significant bits of phase
        val2 *= scale;
        val1 *= 0x10000 - scale;
        return multiply_32x32_rshift32(val1 + val2, magnitude);
    }

    int16_t SampleHarmonics(int i, uint32_t ph, int16_t shapeMod)
    {
        int16_t acc = SineLUT(ph);
        uint32_t p2 = ph + ph;
        int m = 1;
        for (int h = 0; h < 8; ++ h) { // 8 of these puppies yields only 67 us update(). very cheap.
            ph += p2;
            m += 2;
            acc += SineLUT(ph) / m;
        }
        //return SineLUT(ph) + SineLUT(ph + ph + ph)/3 + SineLUT(ph + ph + ph + ph + ph)/5 + SineLUT(ph + ph + ph + ph + ph + ph + ph)/7 + SineLUT(ph + ph + ph + ph + ph + ph + ph+ph+ph)/9;
        return acc;
    }

    

    int16_t SampleArbitrary(int i, uint32_t ph, int16_t shapeMod)
    {
        if (!arbdata)
        {
            return 0;
        }
        int32_t val1, val2;
        uint32_t index, index2, scale;
        index = ph >> 24; // take top 8 significant bits to create a 0-255 index
        index2 = index + 1;
        if (index2 >= 256)
            index2 = 0;
        val1 = *(arbdata + index);
        val2 = *(arbdata + index2);
        scale = (ph >> 8) & 0xFFFF;
        val2 *= scale;
        val1 *= 0x10000 - scale;
        return multiply_32x32_rshift32(val1 + val2, magnitude);
    }

    int16_t SamplePulse(int i, uint32_t ph, int16_t shapeMod)
    {
        int16_t magnitude15 = signed_saturate_rshift(magnitude, 16, 1);
        uint32_t width = ((shapeMod + 0x8000) & 0xFFFF) << 16;
        if (ph < width)
        {
            return magnitude15;
        }
        else
        {
            return -magnitude15;
        }
    }

    int16_t SampleBandlimitPulse(int i, uint32_t ph, int16_t shapeMod)
    {
        uint32_t width = ((shapeMod + 0x8000) & 0xFFFF) << 16;
        int32_t val = band_limit_waveform.generate_pulse(ph, width, i);
        return (int16_t)((val * magnitude) >> 16);
    }

    int16_t SampleSawtooth(int i, uint32_t ph, int16_t shapeMod)
    {
        return signed_multiply_32x16t(magnitude, ph);
    }

    int16_t SampleSawtoothReverse(int i, uint32_t ph, int16_t shapeMod)
    {
        return signed_multiply_32x16t(0xFFFFFFFFu - magnitude, ph);
    }

    int16_t SampleBandlimitSawtooth(int i, uint32_t ph, int16_t shapeMod)
    {
        int16_t val = band_limit_waveform.generate_sawtooth(ph, i);
        return (int16_t)((val * magnitude) >> 16);
    }

    int16_t SampleBandlimitSawtooth2(int i, uint32_t ph, int16_t shapeMod)
    {
        Fixed<int32_t, 16> s = Fixed<int32_t, 16>{Q32::FromFixed(ph)};
        static constexpr decltype(s) half{0.5f};
        s -= half;
        s -= poly_blepQ32(Q32::FromFixed(ph), Q32::FromFixed(mMainPhase.mIncrement)).NonPromotingMultiply(half);
        return Q15{s}.mValue;
    }

    int16_t SampleBandlimitTriangle2(int i, uint32_t ph, int16_t shapeMod)
    {
        Fixed<int32_t, 16> s = Fixed<int32_t, 16>{Q32::FromFixed(ph)};
        static constexpr Fixed<int32_t, 16> half{0.5f};
        static constexpr Fixed<int32_t, 16> one{1.0f};
        static constexpr Fixed<int32_t, 16> two{2.0f};
        static constexpr Fixed<int32_t, 16> four{4.0f};

        // s = abs(ph *4-2)-1;
        s.mValue <<= 2;
        s = s - two;
        s = s.Abs();
        s = s - one;

        // scale = 4 * dt; // why?
        // s -= scale * poly_blamp(ph, dt);
        // s += scale * poly_blamp(fract(ph + 0.5), dt);

        auto qdt = Q32::FromFixed(mMainPhase.mIncrement);
        auto scale = Fixed<int32_t, 16>{qdt};
        scale.mValue <<= 2;
        s -= poly_blampQ32(Q32::FromFixed(ph), qdt).NonPromotingMultiply(scale);
        s += poly_blampQ32(Q32::FromFixed(ph + 0x80000000), qdt).NonPromotingMultiply(scale);

        return Q15{s.NonPromotingMultiply(half)}.mValue;
    }

    int16_t SampleTriangle2(int i, uint32_t ph, int16_t shapeMod)
    {
        Fixed<int32_t, 16> s = Fixed<int32_t, 16>{Q32::FromFixed(ph)};
        static constexpr Fixed<int32_t, 16> half{0.5f};
        static constexpr Fixed<int32_t, 16> one{1.0f};
        static constexpr Fixed<int32_t, 16> two{2.0f};
        static constexpr Fixed<int32_t, 16> four{4.0f};

        // s = abs(ph *4-2)-1;
        s.mValue <<= 2;
        s = s - two;
        s = s.Abs();
        s = s - one;

        return Q15{s.NonPromotingMultiply(half)}.mValue;
    }

    int16_t SampleBandlimitSawtoothReverse(int i, uint32_t ph, int16_t shapeMod)
    {
        return -SampleBandlimitSawtooth(i, ph, shapeMod);
    }

    int16_t SampleVarTriangle(int i, uint32_t ph, int16_t shapeMod)
    {
        uint32_t width = (shapeMod + 0x8000) & 0xFFFF;
        uint32_t rise = 0xFFFFFFFF / width;
        uint32_t fall = 0xFFFFFFFF / (0xFFFF - width);
        uint32_t halfwidth = width << 15;
        uint32_t n;

        if (ph < halfwidth)
        {
            n = (ph >> 16) * rise;
            return ((n >> 16) * magnitude) >> 16;
        }
        else if (ph < 0xFFFFFFFF - halfwidth)
        {
            n = 0x7FFFFFFF - (((ph - halfwidth) >> 16) * fall);
            return (((int32_t)n >> 16) * magnitude) >> 16;
        }

        n = ((ph + halfwidth) >> 16) * rise + 0x80000000;
        return (((int32_t)n >> 16) * magnitude) >> 16;
    }

    int16_t SampleSH(int i, uint32_t ph, int16_t shapeMod)
    {
        if (ph < priorphase)
        {
            mSHSample = random(magnitude) - (magnitude >> 1); // generate new sample
        }
        // todo: smooth it via shapemod?
        return mSHSample;
    }

    using SampleProcT = decltype(&MyT::SampleSine);
    SampleProcT mCurrentSampleProc = nullptr;

    PhaseAccumulator32 mMainPhase;
    PhaseAccumulator32 mSyncPhase;
    bool mHardSyncEnabled = false;
    uint32_t modulation_factor; // i think this is 16.16 fixed?
    int16_t mFBN1;              // previous sample value
    uint32_t priorphase = 0;    // phase of previous sample

    uint32_t mPhaseModFeedbackAmt;
    static constexpr int32_t magnitude =
        0.4f * 65536.0f;    // amplitude (account for gibbs and other) | saturate unsigned 16-bits
    const int16_t *arbdata; // 256-element waveform
    int16_t mSHSample = 0;  // for WAVEFORM_SAMPLE_HOLD
    OscWaveformShape tone_type;
    BandLimitedWaveform band_limit_waveform; // helper for generating band-limited versions of various waves.
};

template <size_t OscillatorCount>
struct AudioSynthWaveformModulated2 : public AudioStream
{
    Oscillator<OscillatorCount> mOscillators[OscillatorCount];

    AudioSynthWaveformModulated2(void) : AudioStream(0, nullptr)
    {
        for (size_t i = 0; i < OscillatorCount; ++i)
        {
            mOscillators[i].mMyIndex = i;
        }
    }

    uint32_t mUpdateTime = 0;

    int16_t mFBSamples[OscillatorCount + 1] = {
        0}; // +1 because we need to guarantee to wave providers that we have at least an even # of samples in there

    void update(void)
    {
        auto m1 = micros();
        audio_block_t *outputBlocks[OscillatorCount] = {nullptr};

        for (size_t iosc = 0; iosc < OscillatorCount; ++iosc)
        {
            outputBlocks[iosc] = allocate();
        }

        int16_t fbSamples[OscillatorCount + 1];
        memcpy(fbSamples, mFBSamples, sizeof(mFBSamples));

        for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            for (size_t iosc = 0; iosc < OscillatorCount; ++iosc)
            {
                if (!outputBlocks[iosc])
                    continue;
                int16_t outp = mOscillators[iosc].ProcessSample(iosc, i, fbSamples);
                outputBlocks[iosc]->data[i] = outp;
                fbSamples[iosc] = outp;
            }
        }

        for (size_t iosc = 0; iosc < OscillatorCount; ++iosc)
        {
            if (!outputBlocks[iosc])
                continue;
            transmit(outputBlocks[iosc], iosc);
            release(outputBlocks[iosc]);
        }
        memcpy(mFBSamples, fbSamples, sizeof(mFBSamples));
        auto m2 = micros();
        mUpdateTime = (m2 - m1);
    }
};
