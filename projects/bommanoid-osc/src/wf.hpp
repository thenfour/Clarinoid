// todo:
// - combine 3 into 1, and process <=1 modulation sample delay
// - portamento
// - unfortunately, sync has artifacting, probably requires adaptation of the bandlimiting code.

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
          sample(0),                       //
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

    void amplitude(float n)
    { // 0 to 1.0
        if (n < 0)
        {
            n = 0;
        }
        else if (n > 1.0f)
        {
            n = 1.0f;
        }
        magnitude = n * 65536.0f; // saturate unsigned 16-bits
    }

    void arbitraryWaveform(const int16_t *data, float maxFreq)
    {
        arbdata = data;
    }

    void phaseModulation(float degrees)
    {
        if (degrees > 9000.0f)
        {
            degrees = 9000.0f;
        }
        else if (degrees < 30.0f)
        {
            degrees = 30.0f;
        }
        modulation_factor = degrees * (float)(65536.0 / 180.0);
    }

    void phaseModulationFeedback(float degrees)
    {
        mPhaseModFeedbackAmt = degrees * (float)(65536.0 / 180.0);
    }

    // void ResetPhase()
    // {
    //     mMainPhase.ResetPhase();
    // }

    bool IsHardSyncEnabled() const
    {
        return mHardSyncEnabled;
    }

    virtual void update(void);

  private:
    PhaseAccumulator32 mMainPhase;
    PhaseAccumulator32 mSyncPhase;
    audio_block_t *inputQueueArray[2];
    uint32_t modulation_factor; // i think this is 16.16 fp?
    int16_t mFBN1;              // previous sample
    uint32_t mPhaseModFeedbackAmt;
    int32_t magnitude;
    const int16_t *arbdata;
    int16_t sample; // for WAVEFORM_SAMPLE_HOLD
    uint8_t tone_type;
    BandLimitedWaveform band_limit_waveform;
    int16_t mZeroBuffer[AUDIO_BLOCK_SAMPLES] = {0};

    bool mHardSyncEnabled = false;
    uint32_t priorphase = 0;
};

void AudioSynthWaveformModulated2::update(void)
{
    int16_t *bp;
    int32_t val1, val2;
    int16_t magnitude15;
    uint32_t i, ph, index, index2, scale;

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

    auto block = allocate();
    if (!block)
    {
        return;
    }
    auto releaseBlock = OnScopeExit([&]() { release(block); });

    bp = block->data;

    // Now generate the output samples using the pre-computed phase angles
    switch (tone_type)
    {
    case WAVEFORM_SINE:
        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            mMainPhase.OnSample();
            ph = mMainPhase.mAccumulator + (mFBN1 * mPhaseModFeedbackAmt) +
                 (uint32_t(*phaseModBuf++) * modulation_factor);
            if (mHardSyncEnabled && mSyncPhase.OnSample())
            {
                mMainPhase.mAccumulator = mSyncPhase.mAccumulator;
            }

            index = ph >> 24;
            val1 = AudioWaveformSine[index];
            val2 = AudioWaveformSine[index + 1];
            scale = (ph >> 8) & 0xFFFF;
            val2 *= scale;
            val1 *= 0x10000 - scale;
            *bp = multiply_32x32_rshift32(val1 + val2, magnitude);
            mFBN1 = *bp;
            ++bp;
        }
        break;

    case WAVEFORM_ARBITRARY:
        if (!arbdata)
        {
            return;
        }
        // len = 256
        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            mMainPhase.OnSample();
            ph = mMainPhase.mAccumulator + (mFBN1 * mPhaseModFeedbackAmt) +
                 (uint32_t(*phaseModBuf++) * modulation_factor);
            if (mHardSyncEnabled && mSyncPhase.OnSample())
            {
                mMainPhase.mAccumulator = mSyncPhase.mAccumulator;
            }

            index = ph >> 24;
            index2 = index + 1;
            if (index2 >= 256)
                index2 = 0;
            val1 = *(arbdata + index);
            val2 = *(arbdata + index2);
            scale = (ph >> 8) & 0xFFFF;
            val2 *= scale;
            val1 *= 0x10000 - scale;
            *bp = multiply_32x32_rshift32(val1 + val2, magnitude);
            mFBN1 = *bp;
            ++bp;
        }
        break;

    case WAVEFORM_PULSE:
    case WAVEFORM_SQUARE:
        magnitude15 = signed_saturate_rshift(magnitude, 16, 1);
        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            mMainPhase.OnSample();
            ph = mMainPhase.mAccumulator + (mFBN1 * mPhaseModFeedbackAmt) +
                 (uint32_t(*phaseModBuf++) * modulation_factor);
            if (mHardSyncEnabled && mSyncPhase.OnSample())
            {
                mMainPhase.mAccumulator = mSyncPhase.mAccumulator;
            }

            uint32_t width = (((*shapeModBuf++) + 0x8000) & 0xFFFF) << 16;
            if (ph < width)
            {
                *bp = magnitude15;
            }
            else
            {
                *bp = -magnitude15;
            }
            mFBN1 = *bp;
            ++bp;
        }
        break;

    case WAVEFORM_BANDLIMIT_PULSE:
    case WAVEFORM_BANDLIMIT_SQUARE:
        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            mMainPhase.OnSample();
            ph = mMainPhase.mAccumulator + (mFBN1 * mPhaseModFeedbackAmt) +
                 (uint32_t(*phaseModBuf++) * modulation_factor);
            if (mHardSyncEnabled && mSyncPhase.OnSample())
            {
                mMainPhase.mAccumulator = mSyncPhase.mAccumulator;
            }

            uint32_t width = (((*shapeModBuf++) + 0x8000) & 0xFFFF) << 16;
            int32_t val = band_limit_waveform.generate_pulse(ph, width, i);
            *bp = (int16_t)((val * magnitude) >> 16);
            mFBN1 = *bp;
            ++bp;
        }
        break;

    case WAVEFORM_SAWTOOTH:
        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            mMainPhase.OnSample();
            ph = mMainPhase.mAccumulator + (mFBN1 * mPhaseModFeedbackAmt) +
                 (uint32_t(*phaseModBuf++) * modulation_factor);
            if (mHardSyncEnabled && mSyncPhase.OnSample())
            {
                mMainPhase.mAccumulator = mSyncPhase.mAccumulator;
            }

            *bp = signed_multiply_32x16t(magnitude, ph);
            mFBN1 = *bp;
            ++bp;
        }
        break;

    case WAVEFORM_SAWTOOTH_REVERSE:
        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            mMainPhase.OnSample();
            ph = mMainPhase.mAccumulator + (mFBN1 * mPhaseModFeedbackAmt) +
                 (uint32_t(*phaseModBuf++) * modulation_factor);
            if (mHardSyncEnabled && mSyncPhase.OnSample())
            {
                mMainPhase.mAccumulator = mSyncPhase.mAccumulator;
            }

            *bp = signed_multiply_32x16t(0xFFFFFFFFu - magnitude, ph);
            mFBN1 = *bp;
            ++bp;
        }
        break;

    case WAVEFORM_BANDLIMIT_SAWTOOTH:
    case WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE:
        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            mMainPhase.OnSample();
            ph = mMainPhase.mAccumulator + (mFBN1 * mPhaseModFeedbackAmt) +
                 (uint32_t(*phaseModBuf++) * modulation_factor);
            if (mHardSyncEnabled && mSyncPhase.OnSample())
            {
                mMainPhase.mAccumulator = mSyncPhase.mAccumulator;
            }

            int16_t val = band_limit_waveform.generate_sawtooth(ph, i);
            val = (int16_t)((val * magnitude) >> 16);
            *bp = tone_type == WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE ? (int16_t)-val : (int16_t) + val;
            mFBN1 = *bp;
            ++bp;
        }
        break;

    case WAVEFORM_TRIANGLE_VARIABLE:
    case WAVEFORM_TRIANGLE:
        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            mMainPhase.OnSample();
            ph = mMainPhase.mAccumulator + (mFBN1 * mPhaseModFeedbackAmt) +
                 (uint32_t(*phaseModBuf++) * modulation_factor);
            if (mHardSyncEnabled && mSyncPhase.OnSample())
            {
                mMainPhase.mAccumulator = mSyncPhase.mAccumulator;
            }

            uint32_t width = ((*shapeModBuf++) + 0x8000) & 0xFFFF;
            uint32_t rise = 0xFFFFFFFF / width;
            uint32_t fall = 0xFFFFFFFF / (0xFFFF - width);
            uint32_t halfwidth = width << 15;
            uint32_t n;

            if (ph < halfwidth)
            {
                n = (ph >> 16) * rise;
                *bp = ((n >> 16) * magnitude) >> 16;
            }
            else if (ph < 0xFFFFFFFF - halfwidth)
            {
                n = 0x7FFFFFFF - (((ph - halfwidth) >> 16) * fall);
                *bp = (((int32_t)n >> 16) * magnitude) >> 16;
            }
            else
            {
                n = ((ph + halfwidth) >> 16) * rise + 0x80000000;
                *bp = (((int32_t)n >> 16) * magnitude) >> 16;
            }
            mFBN1 = *bp;
            ++bp;
        }
        break;
    case WAVEFORM_SAMPLE_HOLD: {

        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            mMainPhase.OnSample();
            ph = mMainPhase.mAccumulator + (mFBN1 * mPhaseModFeedbackAmt) +
                 (uint32_t(*phaseModBuf++) * modulation_factor);
            if (mHardSyncEnabled && mSyncPhase.OnSample())
            {
                mMainPhase.mAccumulator = mSyncPhase.mAccumulator;
            }

            if (ph < priorphase)
            { // sorta breaks phase modulation
                sample = random(magnitude) - (magnitude >> 1);
            }
            priorphase = ph;
            *bp = sample;
            mFBN1 = *bp;
            ++bp;
        }
        break;
    }
    }

    transmit(block, 0);
}
