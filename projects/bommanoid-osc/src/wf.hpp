// todo:
// - portamento
// - unfortunately, sync has artifacting, probably requires adaptation of the bandlimiting code. in many cases we might
// be able to use a BLEP

// with N oscillators, it means N*N modulation matrix

#pragma once

#include <Arduino.h>
#include <AudioStream.h>
#include <synth_waveform.h>
#include <dspinst.h>
#include <arm_math.h>
#include <array>

static const int16_t gZeroAudioBuffer[AUDIO_BLOCK_SAMPLES] = {0}; // sentinel buffer when no modulation data exists.

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

    void SetParams(float mainFreq, float syncFreq, bool syncEnable, short waveformType)
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
            if (waveformType == WAVEFORM_BANDLIMIT_SQUARE)
                band_limit_waveform.init_square(blincrement);
            else if (waveformType == WAVEFORM_BANDLIMIT_PULSE)
                band_limit_waveform.init_pulse(blincrement, 0x80000000u);
            else if (waveformType == WAVEFORM_BANDLIMIT_SAWTOOTH || waveformType == WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE)
                band_limit_waveform.init_sawtooth(blincrement);
        }
        tone_type = waveformType;

        switch (tone_type)
        {
        case WAVEFORM_SINE:
            mCurrentSampleProc = &MyT::SampleSine;
            break;
        case WAVEFORM_ARBITRARY:
            mCurrentSampleProc = &MyT::SampleArbitrary;
            break;
        case WAVEFORM_PULSE:
        case WAVEFORM_SQUARE:
            mCurrentSampleProc = &MyT::SamplePulse;
            break;
        case WAVEFORM_BANDLIMIT_PULSE:
        case WAVEFORM_BANDLIMIT_SQUARE:
            mCurrentSampleProc = &MyT::SampleBandlimitPulse;
            break;
        case WAVEFORM_SAWTOOTH:
            mCurrentSampleProc = &MyT::SampleSawtooth;
            break;
        case WAVEFORM_SAWTOOTH_REVERSE:
            mCurrentSampleProc = &MyT::SampleSawtoothReverse;
            break;
        case WAVEFORM_BANDLIMIT_SAWTOOTH:
            mCurrentSampleProc = &MyT::SampleBandlimitSawtooth;
            break;
        case WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE:
            mCurrentSampleProc = &MyT::SampleBandlimitSawtoothReverse;
            break;
        case WAVEFORM_TRIANGLE_VARIABLE:
        case WAVEFORM_TRIANGLE:
            mCurrentSampleProc = &MyT::SampleVarTriangle;
            break;
        case WAVEFORM_SAMPLE_HOLD:
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

    // void phaseModulation(float degrees)
    // {
    //     degrees = Clamp(degrees, 0, 9000); // why 9000? why was 30 the lower bound before?
    //     modulation_factor = degrees * (float)(65536.0 / 180.0);
    // }

    // void phaseModulationFeedback(float degrees)
    // {
    //     mPhaseModFeedbackAmt = degrees * (float)(65536.0 / 180.0);
    // }

    std::array<uint32_t, OscillatorCount> mRMMatrix;
    std::array<uint32_t, OscillatorCount> mFMMatrix;

    void SetRMMatrix(size_t iFromOsc, float degrees)
    {
        mRMMatrix[iFromOsc] = degrees * 65536.0;
    }

    void SetFMMatrix(size_t iFromOsc, float degrees)
    {
        degrees = Clamp(degrees, 0, 9000); // why 9000? why was 30 the lower bound before?
        mFMMatrix[iFromOsc] = degrees * (float)(65536.0 / 180.0);
    }

    int16_t ProcessSample(int iOsc, int iSample, const int16_t (&currentOscSamples)[OscillatorCount])
    {
        if (!mCurrentSampleProc)
            return 0;

        uint32_t phase = mMainPhase.mAccumulator;
        uint32_t rmFact = 0;
        for (size_t i = 0; i < OscillatorCount; ++i)
        {
            phase += currentOscSamples[i] * mFMMatrix[i];
            rmFact += currentOscSamples[i] * mRMMatrix[i];
        }

        mMainPhase.OnSample();
        if (mHardSyncEnabled && mSyncPhase.OnSample())
        {
            mMainPhase.mAccumulator = mSyncPhase.mAccumulator;
        }

        mFBN1 = (this->*mCurrentSampleProc)(iSample, phase, 0);
        priorphase = phase;
        return mFBN1;
    }

    int16_t GetLastSample() const
    {
        return mFBN1;
    }

    int16_t SampleSine(int i, uint32_t ph, int16_t shapeMod)
    {
        // TODO: band-limit phase discontinuity, probably via blep
        int32_t val1, val2;
        uint32_t index, scale;
        index = ph >> 24;
        val1 = AudioWaveformSine[index];
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
    static constexpr int32_t magnitude = 0.5f * 65536.0f; // 0.5 amplitude | saturate unsigned 16-bits
    const int16_t *arbdata;                               // 256-element waveform
    int16_t mSHSample = 0;                                // for WAVEFORM_SAMPLE_HOLD
    uint8_t tone_type;
    BandLimitedWaveform band_limit_waveform; // helper for generating band-limited versions of various waves.
};

template <size_t OscillatorCount>
struct AudioSynthWaveformModulated2 : public AudioStream
{
    Oscillator<OscillatorCount> mOscillators[OscillatorCount];

    AudioSynthWaveformModulated2(void) : AudioStream(0, nullptr)
    {
    }

    void update(void)
    {
        audio_block_t *outputBlocks[OscillatorCount] = {nullptr};
        int16_t fbSamples[OscillatorCount] = {0};

        for (size_t iosc = 0; iosc < OscillatorCount; ++iosc)
        {
            outputBlocks[iosc] = allocate();
            fbSamples[iosc] = mOscillators[iosc].GetLastSample();
        }

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
    }
};
