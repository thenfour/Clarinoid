// todo:
// - combine 3 into 1, and process <=1 modulation sample delay
// - portamento
// - unfortunately, sync has artifacting, probably requires adaptation of the bandlimiting code. in many cases we might
// be able to use a BLEP

#pragma once

#include <Arduino.h>
#include <AudioStream.h>
#include <synth_waveform.h>
#include <dspinst.h>
#include <arm_math.h>

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

struct AudioSynthWaveformModulated2 : public AudioStream
{
    AudioSynthWaveformModulated2(void)
        : AudioStream(2, inputQueueArray), //
                                           // phase_accumulator(0),            //
                                           // phase_increment(0),              //
          modulation_factor(32768),        //
          magnitude(0),                    //
          arbdata(NULL),                   //
          // sample(0),                       //
          tone_type(WAVEFORM_SINE)
    {
    }

    void SetParams(float mainFreq, float syncFreq, bool syncEnable, short waveformType)
    {
        mHardSyncEnabled = syncEnable;
        if (syncEnable)
        {
            mMainPhase.frequency(syncFreq);
            mSyncPhase.frequency(mainFreq);

            if (waveformType != tone_type)
            {
                if (waveformType == WAVEFORM_BANDLIMIT_SQUARE)
                    band_limit_waveform.init_square(mSyncPhase.mIncrement);
                else if (waveformType == WAVEFORM_BANDLIMIT_PULSE)
                    band_limit_waveform.init_pulse(mSyncPhase.mIncrement, 0x80000000u);
                else if (waveformType == WAVEFORM_BANDLIMIT_SAWTOOTH ||
                         waveformType == WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE)
                    band_limit_waveform.init_sawtooth(mSyncPhase.mIncrement);
            }
        }
        else
        {
            mMainPhase.frequency(mainFreq);

            if (waveformType != tone_type)
            {
                if (waveformType == WAVEFORM_BANDLIMIT_SQUARE)
                    band_limit_waveform.init_square(mMainPhase.mIncrement);
                else if (waveformType == WAVEFORM_BANDLIMIT_PULSE)
                    band_limit_waveform.init_pulse(mMainPhase.mIncrement, 0x80000000u);
                else if (waveformType == WAVEFORM_BANDLIMIT_SAWTOOTH ||
                         waveformType == WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE)
                    band_limit_waveform.init_sawtooth(mMainPhase.mIncrement);
            }
        }
        tone_type = waveformType;
    }

    // 0 to 1.0, but be careful of gibbs phenomenon + whatever other artifacting. I'd say max 0.5.
    void amplitude(float n)
    {
        n = Clamp(n, 0, 1);
        magnitude = n * 65536.0f; // saturate unsigned 16-bits
    }

    void arbitraryWaveform(const int16_t *data /* 256-element cycle */)
    {
        arbdata = data;
    }

    void phaseModulation(float degrees)
    {
        degrees = Clamp(degrees, 0, 9000); // why 9000? why was 30 the lower bound before?
        modulation_factor = degrees * (float)(65536.0 / 180.0);
    }

    void phaseModulationFeedback(float degrees)
    {
        mPhaseModFeedbackAmt = degrees * (float)(65536.0 / 180.0);
    }

    bool IsHardSyncEnabled() const
    {
        return mHardSyncEnabled;
    }

    virtual void update(void);

    int16_t SampleSine(int i, uint32_t ph, uint16_t shapeMod)
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

    int16_t SampleArbitrary(int i, uint32_t ph, uint16_t shapeMod)
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

    int16_t SamplePulse(int i, uint32_t ph, uint16_t shapeMod)
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

    int16_t SampleBandlimitPulse(int i, uint32_t ph, uint16_t shapeMod)
    {
        uint32_t width = ((shapeMod + 0x8000) & 0xFFFF) << 16;
        int32_t val = band_limit_waveform.generate_pulse(ph, width, i);
        return (int16_t)((val * magnitude) >> 16);
    }

    int16_t SampleSawtooth(int i, uint32_t ph, uint16_t shapeMod)
    {
        return signed_multiply_32x16t(magnitude, ph);
    }

    int16_t SampleSawtoothReverse(int i, uint32_t ph, uint16_t shapeMod)
    {
        return signed_multiply_32x16t(0xFFFFFFFFu - magnitude, ph);
    }

    int16_t SampleBandlimitSawtooth(int i, uint32_t ph, uint16_t shapeMod)
    {
        int16_t val = band_limit_waveform.generate_sawtooth(ph, i);
        return (int16_t)((val * magnitude) >> 16);
    }

    int16_t SampleBandlimitSawtoothReverse(int i, uint32_t ph, uint16_t shapeMod)
    {
        return -SampleBandlimitSawtooth(i, ph, shapeMod);
    }

    int16_t SampleVarTriangle(int i, uint32_t ph, uint16_t shapeMod)
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

    int16_t SampleSH(int i, uint32_t ph, uint16_t shapeMod)
    {
        if (ph < priorphase)
        {
            mSHSample = random(magnitude) - (magnitude >> 1); // generate new sample
        }
        return mSHSample;
    }

  private:
    audio_block_t *inputQueueArray[2];

    PhaseAccumulator32 mMainPhase;
    PhaseAccumulator32 mSyncPhase;
    bool mHardSyncEnabled = false;
    uint32_t modulation_factor; // i think this is 16.16 fixed?
    int16_t mFBN1;              // previous sample value
    uint32_t priorphase = 0;    // phase of previous sample

    uint32_t mPhaseModFeedbackAmt;
    int32_t magnitude;      // output gain
    const int16_t *arbdata; // 256-element waveform
    int16_t mSHSample = 0;  // for WAVEFORM_SAMPLE_HOLD
    uint8_t tone_type;
    BandLimitedWaveform band_limit_waveform;        // helper for generating band-limited versions of various waves.
    int16_t mZeroBuffer[AUDIO_BLOCK_SAMPLES] = {0}; // sentinel buffer when no modulation data exists.
};

void AudioSynthWaveformModulated2::update(void)
{
    using ThisT = AudioSynthWaveformModulated2;
    int16_t *bp;
    uint32_t i, ph;

    auto phaseModData = receiveReadOnly(0);
    const int16_t *phaseModBuf = mZeroBuffer;
    if (phaseModData)
    {
        phaseModBuf = phaseModData->data;
    }
    auto releasePhaseModData = OnScopeExit([&]() {
        if (phaseModData && phaseModBuf != mZeroBuffer)
        {
            release(phaseModData);
        }
    });

    auto shapeModData = receiveReadOnly(1);
    const int16_t *shapeModBuf = mZeroBuffer;
    if (shapeModData)
    {
        shapeModBuf = shapeModData->data;
    }
    auto releaseShapeModData = OnScopeExit([&]() {
        if (shapeModData)
        {
            release(shapeModData);
        }
    });

    decltype(&ThisT::SampleSine) proc = nullptr;

    switch (tone_type)
    {
    case WAVEFORM_SINE:
        proc = &ThisT::SampleSine;
        break;
    case WAVEFORM_ARBITRARY:
        proc = &ThisT::SampleArbitrary;
        break;
    case WAVEFORM_PULSE:
    case WAVEFORM_SQUARE:
        proc = &ThisT::SamplePulse;
        break;
    case WAVEFORM_BANDLIMIT_PULSE:
    case WAVEFORM_BANDLIMIT_SQUARE:
        proc = &ThisT::SampleBandlimitPulse;
        break;
    case WAVEFORM_SAWTOOTH:
        proc = &ThisT::SampleSawtooth;
        break;
    case WAVEFORM_SAWTOOTH_REVERSE:
        proc = &ThisT::SampleSawtoothReverse;
        break;
    case WAVEFORM_BANDLIMIT_SAWTOOTH:
        proc = &ThisT::SampleBandlimitSawtooth;
        break;
    case WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE:
        proc = &ThisT::SampleBandlimitSawtoothReverse;
        break;
    case WAVEFORM_TRIANGLE_VARIABLE:
    case WAVEFORM_TRIANGLE:
        proc = &ThisT::SampleVarTriangle;
        break;
    case WAVEFORM_SAMPLE_HOLD:
        proc = &ThisT::SampleSH;
        break;
    default:
        return;
    }

    auto block = allocate();
    if (!block)
    {
        return;
    }
    auto releaseBlock = OnScopeExit([&]() { release(block); });

    bp = block->data;

    for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
    {
        mMainPhase.OnSample();
        ph = mMainPhase.mAccumulator + (mFBN1 * mPhaseModFeedbackAmt) + (uint32_t(*phaseModBuf++) * modulation_factor);
        if (mHardSyncEnabled && mSyncPhase.OnSample())
        {
            mMainPhase.mAccumulator = mSyncPhase.mAccumulator;
        }

        *bp = (this->*proc)(i, ph, shapeModBuf[i]);
        priorphase = ph;
        mFBN1 = *bp;
        ++bp;
    }
    transmit(block, 0);
}
