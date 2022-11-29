// smooth square

// - consider smoothing k-rate params across the buffer; especially mod amounts
// - unfortunately, sync has artifacting, probably requires adaptation of the bandlimiting code. in many cases we might
// be able to use a BLEP

#pragma once

#include <Arduino.h>
#include <AudioStream.h>
#include <synth_waveform.h>
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
        case OscWaveformShape::SawRev_Bandlimited:
            mCurrentSampleProc = &MyT::SampleBandlimitSawtoothReverse;
            break;
        case OscWaveformShape::VarTriangle:
            mCurrentSampleProc = &MyT::SampleVarTriangle;
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
        // TODO: band-limit phase discontinuity
        int32_t val1, val2;
        uint32_t index, scale;
        index = ph >> 24;                // 8 top bits = index to LUT
        val1 = AudioWaveformSine[index]; // -32768...32767
        val2 = AudioWaveformSine[index + 1];
        scale = (ph >> 8) & 0xFFFF;
        val2 *= scale;
        val1 *= 0x10000 - scale;
        return multiply_32x32_rshift32(val1 + val2, magnitude);
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
