
#pragma once

#include <arm_math.h>
#include <cmath>
#include <cstdint>

namespace clarinoid
{
/*
   Following waveforms are available:

   osc1: sine(0), variable triangle(1), pulse(2)
   osc2: sine(0), variable triangle(1), pulse(2), hard-synced sawtooth(3)
   osc3: sine(0), variable triangle(1), pulse(2), hard-synced sawtooth(3)

   When choosing the synced saw for oscillator 2 or 3, oscillator 1 is always the master
   The hard-synced sawtooth does not have PWM

    NOTE: VARTRI @ pulsewidth 0 = "leaning to left", so you get a saw wave
    |`.        |`.        |`.        |`.
    |  `.      |  `.      |  `.      |  `.
    |    `.    |    `.    |    `.    |    `.
    |      `.  |      `.  |      `.  |      `.
    |        `.|        `.|        `.|        `.

    pulsewidth 0.5 = triangle
         /\         /\         /\         /\
        /  \       /  \       /  \       /  \
       /    \     /    \     /    \     /    \
      /      \   /      \   /      \   /      \
     /        \ /        \ /        \ /        \

    pulsewidth 1.0 = leaning to right
    |        .`|        .`|        .`|        .`
    |      .`  |      .`  |      .`  |      .`
    |    .`    |    .`    |    .`    |    .`
    |  .`      |  .`      |  .`      |  .`
    |.`        |.`        |.`        |.`

*/

struct PortamentoCalc
{
    float mValue = 0;
    int mDurationBlocks = 0;
    float mDeltaPerBlock = 0;
    int mCursorBlocks = 0; // cursor

    // call this every audio buffer interval; returns a smoothed frequency signal.
    float KStep(float targetValue, int durationMS, bool restart)
    {
        // calculate params
        if (durationMS > 0)
        {
            if (restart)
            {
                mCursorBlocks = 0;
                float durationSamples = float(durationMS) / 1000 * AUDIO_SAMPLE_RATE_EXACT;
                mDurationBlocks = ::floorf(durationSamples / AUDIO_BLOCK_SAMPLES);
                mDeltaPerBlock = (targetValue - mValue) / mDurationBlocks;
            }
        }
        else
        {
            // duration 0; instant transition and no running
            mDurationBlocks = 0;
            mCursorBlocks = 0;
            mDeltaPerBlock = 0;
            mValue = targetValue;
        }

        // and evaluate this block.
        if (mCursorBlocks >= mDurationBlocks)
        {
            return mValue;
        }

        mValue += mDeltaPerBlock;
        mCursorBlocks++;
        return mValue;
    }
};

// tracks phase and frequency supporting portamento
struct PhaseAccumulator
{
    static constexpr uint64_t kPhaseScaleRaw = uint64_t(1) << 32;

    float mFreq = 0;              // live oscillator frequency (differs from request during portamento)
    uint32_t mPhase = 0;          // current accumulator phase in [0, 2^32)
    uint32_t mPhaseOffset = 0;    // stored phase offset in fixed-point form
    uint32_t mPhaseIncrement = 0; // per-sample phase step in fixed-point
    float mDt = 0;                // cycles per sample
    float mPhaseNoWrap = 0;       // phase after last step before wrapping to [0,1)
    float mWrapFraction = 0;      // fractional sample position when wrap occurred
    bool mWrapped = false;

    static uint32_t FloatToPhase(float phase01)
    {
        float wrapped = Frac(phase01);
        if (wrapped < 0.0f)
        {
            wrapped += 1.0f;
        }
        float scaled = static_cast<float>(wrapped) * static_cast<float>(kPhaseScaleRaw);
        if (scaled <= 0)
        {
            return 0;
        }
        uint64_t value = static_cast<uint64_t>(scaled + 0.5);
        if (value >= kPhaseScaleRaw)
        {
            value = kPhaseScaleRaw - 1;
        }
        return static_cast<uint32_t>(value);
    }

    static uint32_t FloatToIncrement(float dt)
    {
        if (dt <= 0.0f)
        {
            return 0;
        }
        float scaled = static_cast<float>(dt) * static_cast<float>(kPhaseScaleRaw);
        if (scaled <= 0)
        {
            return 0;
        }
        uint64_t value = static_cast<uint64_t>(scaled + 0.5f);
        if (value >= kPhaseScaleRaw)
        {
            value = kPhaseScaleRaw - 1;
        }
        return static_cast<uint32_t>(value);
    }

    static float PhaseToFloat(uint32_t phase)
    {
        return static_cast<float>(static_cast<float>(phase) / static_cast<float>(kPhaseScaleRaw));
    }

    float GetPhase01() const
    {
        return PhaseToFloat(mPhase);
    }

    float GetPhaseNoWrap() const
    {
        return mPhaseNoWrap;
    }

    float GetPhaseOffset01() const
    {
        return PhaseToFloat(mPhaseOffset);
    }

    bool DidWrap() const
    {
        return mWrapped;
    }

    float GetWrapFraction() const
    {
        return mWrapFraction;
    }

    void ResetPhase()
    {
        mPhase = mPhaseOffset;
        mPhaseNoWrap = GetPhase01();
        mWrapFraction = 0.0f;
        mWrapped = false;
    }

    void SetParams(float freq, float phaseOffset01)
    {
        mFreq = freq;
        mDt = mFreq / AUDIO_SAMPLE_RATE_EXACT;
        mPhaseIncrement = FloatToIncrement(mDt);

        uint32_t newOffset = FloatToPhase(phaseOffset01);
        if (newOffset != mPhaseOffset)
        {
            mPhase += newOffset - mPhaseOffset;
            mPhaseOffset = newOffset;
        }

        mPhaseNoWrap = GetPhase01();
        mWrapFraction = 0.0f;
        mWrapped = false;
    }

    void StepWithoutFrac()
    {
        uint32_t prev = mPhase;
        mPhase += mPhaseIncrement;
        mWrapped = mPhase < prev;
        float phase01 = GetPhase01();
        mPhaseNoWrap = mWrapped ? phase01 + 1.0f : phase01;
        if (mWrapped && mDt > 0.0f)
        {
            mWrapFraction = Clamp(phase01 / mDt, 0.0f, 1.0f);
        }
        else
        {
            mWrapFraction = 0.0f;
        }
    }

    bool StepWithFrac(float &x)
    {
        StepWithoutFrac();
        if (!mWrapped)
        {
            return false;
        }
        x = mWrapFraction;
        return true;
    }

    void SetPhaseNoWrap(float phase)
    {
        float normalized = phase - floorf(phase);
        if (normalized < 0.0f)
        {
            normalized += 1.0f;
        }
        mPhase = FloatToPhase(normalized);
        mPhaseNoWrap = normalized;
        mWrapped = false;
        mWrapFraction = 0.0f;
    }

    void WrapPhaseToUnitInterval()
    {
        SetPhaseNoWrap(mPhaseNoWrap);
    }
};

struct SineWaveformProvider // : public WaveformProviderBase
{
    template <typename TOscillator>
    static void ResetPhaseDueToSync(TOscillator &caller, float x)
    {
        caller.mMainPhase.SetPhaseNoWrap(x * caller.mMainPhase.mDt + caller.mMainPhase.GetPhaseOffset01());
        caller.mPulseStage = false;
    }
    template <typename TOscillator>
    static void Step(TOscillator &caller, float &fboutput, float phaseShift)
    {
        caller.mMainPhase.WrapPhaseToUnitInterval();
        float phase = caller.mMainPhase.GetPhase01();
        caller.mOutput = fast::sin((phase + phaseShift + fboutput * caller.mPMFeedbackAmt) * TWO_PI);
        // caller.mOutput = sinf((phase) * TWO_PI);
    }
};

struct VarTriangleWaveformProvider // : public WaveformProviderBase
{
    template <typename TOscillator>
    static void ResetPhaseDueToSync(TOscillator &caller, float x)
    {
        caller.mMainPhase.SetPhaseNoWrap(x * caller.mMainPhase.mDt + caller.mMainPhase.GetPhaseOffset01());
        caller.mPulseStage = false;
    }

    template <typename TOscillator>
    static void Step(TOscillator &caller, float &fboutput, float phaseShift)
    {
        // TODO: use phase shift
        float phase = caller.mMainPhase.GetPhaseNoWrap();
        while (true)
        {
            if (!caller.mPulseStage)
            {
                if (phase < caller.mPulseWidth)
                    break;

                float x =
                    (phase - caller.mPulseWidth) / (caller.mWidthDelay - caller.mPulseWidth + caller.mMainPhase.mDt);
                float scale = caller.mMainPhase.mDt / (caller.mPulseWidth - caller.mPulseWidth * caller.mPulseWidth);

                caller.mOutput -= scale * blamp0(x);
                caller.mBlepDelay -= scale * blamp1(x);

                caller.mPulseStage = true;
            }
            if (caller.mPulseStage)
            {
                if (phase < 1)
                    break;

                // we have crossed over phase,
                // remainder phase 0-1 / (freq/samplerate)
                phase -= 1;
                // x = number of master samples crossed over phase, but because we're processing 1 sample at a
                // time, this is always 0-1.
                float x = phase / caller.mMainPhase.mDt;

                float scale = caller.mMainPhase.mDt / (caller.mPulseWidth - caller.mPulseWidth * caller.mPulseWidth);

                caller.mOutput += scale * blamp0(x);
                caller.mBlepDelay += scale * blamp1(x);

                caller.mPulseStage = false;
            }
        }

        float naiveWave;

        if (phase <= caller.mPulseWidth)
        {
            naiveWave = 2 * phase / caller.mPulseWidth - 1;
        }
        else
        {
            naiveWave = -2 * (phase - caller.mPulseWidth) / (1 - caller.mPulseWidth) + 1;
        }

        caller.mBlepDelay += naiveWave;

        caller.mWidthDelay = caller.mPulseWidth;
        caller.mMainPhase.SetPhaseNoWrap(phase);
    }
};

struct PulseWaveformProvider
{
    template <typename TOscillator>
    static void ResetPhaseDueToSync(TOscillator &caller, float x)
    {
        caller.mMainPhase.SetPhaseNoWrap(x * caller.mMainPhase.mDt + caller.mMainPhase.GetPhaseOffset01());
        caller.mPulseStage = false;
    }
    template <typename TOscillator>
    static void Step(TOscillator &caller, float &fboutput, float phaseShift)
    {
        // TODO: use phase shift
        float phase = caller.mMainPhase.GetPhaseNoWrap();
        while (true)
        {
            if (!caller.mPulseStage)
            {
                if (phase < caller.mPulseWidth)
                    break;

                float x =
                    (phase - caller.mPulseWidth) / (caller.mWidthDelay - caller.mPulseWidth + caller.mMainPhase.mDt);

                caller.mOutput -= blep0(x);
                caller.mBlepDelay -= blep1(x);

                caller.mPulseStage = true;
            }
            if (caller.mPulseStage)
            {
                if (phase < 1)
                    break;

                // we have crossed over phase.
                phase -= 1;
                // x = number of master samples crossed over phase, but because we're processing 1 sample at a
                // time, this is always 0-1.
                float x = phase / caller.mMainPhase.mDt;

                caller.mOutput += blep0(x);
                caller.mBlepDelay += blep1(x);

                caller.mPulseStage = false;
            }
        }

        float naiveWave = caller.mPulseStage ? -1.0f : 1.0f;

        caller.mBlepDelay += naiveWave;

        caller.mWidthDelay = caller.mPulseWidth;
        caller.mMainPhase.SetPhaseNoWrap(phase);
    }
};

struct SawWaveformProvider
{
    template <typename TOscillator>
    static void ResetPhaseDueToSync(TOscillator &caller, float x)
    {
        caller.mOutput = caller.mBlepDelay;
        caller.mBlepDelay = 0;

        float scale = caller.mMainPhase.mDt / caller.mSyncPhase.mDt;
        scale -= floorf(scale);
        if (scale <= 0)
        {
            scale = 1;
        }

        caller.mOutput -= 0.5f * scale * blep0(x);
        caller.mBlepDelay -= 0.5f * scale * blep1(x);

        // increase slave phase by partial sample
        float dt = (1 - x) * caller.mMainPhase.mDt;
        float phase = Frac(caller.mMainPhase.GetPhase01() + dt);

        if (phase < dt)
        {
            phase = Frac(phase + x * caller.mMainPhase.mDt);

            // process transition for the slave
            float x2 = phase / caller.mMainPhase.mDt;
            caller.mOutput -= 0.5f * blep0(x2);
            caller.mBlepDelay -= 0.5f * blep1(x2);
        }

        // reset slave phase:
        float resetPhase = x * caller.mMainPhase.mDt;
        caller.mMainPhase.SetPhaseNoWrap(resetPhase);

        caller.mBlepDelay += resetPhase;

        caller.mOutput = caller.mOutput * 2 - 1;
    }

    template <typename TOscillator>
    static void Step(TOscillator &caller, float &fboutput, float phaseShift)
    {
        // TODO: use phase shift
        caller.mMainPhase.WrapPhaseToUnitInterval();
        float phase = caller.mMainPhase.GetPhase01();

        if (phase < caller.mMainPhase.mDt)
        {
            float x = phase / caller.mMainPhase.mDt;
            caller.mOutput -= 0.5f * blep0(x);
            caller.mBlepDelay -= 0.5f * blep1(x);
        }

        caller.mBlepDelay += phase;

        caller.mOutput = caller.mOutput * 2 - 1;
    }
};

struct Oscillator
{
    PhaseAccumulator mMainPhase;

    PhaseAccumulator mSyncPhase; // when sync is enabled, this is the phase of the "main" frequency. it resets the phase
                                 // of mMainPhase.
    bool mSyncEnabled = false;
    // float mAmplitude = 0;
    // float mWaveformMorph01 = 0.5f; // curve: gModCurveLUT.LinearYIndex;

    OscWaveformShape mWaveformShape = OscWaveformShape::Sine;

    float mPulseWidthTarget01 = 0; // pulseWidth1
    float mPulseWidth = 0.5;       // osc1_pulseWidth 0,1

    float mBlepDelay = 0;     // osc1_blepDelay
    float mWidthDelay = 0;    // osc1_widthDelay
    bool mPulseStage = false; // osc1_pulseStage

    float mOutput = 0;           // osc1_output
    float mPMMultiplier = 0.01f; // scaling PM input 0-1 phase is EXTREME, so we need a reasonable maximum.
    float mPMFeedbackAmt = 0.0f;

    //  Use this for phase-retrigger everytime you send a noteOn, if you want a consistent sound.
    //  Don't use this for hard-sync, it will cause aliasing.
    //  You can also set the phases of the oscillators to different starting points.
    void ResetPhase()
    {
        mMainPhase.ResetPhase();
    }

    void waveform(OscWaveformShape wform)
    {
        mWaveformShape = wform;
    }

    // void SetAmplitude(float amplitude01)
    // {
    //     mAmplitude = amplitude01;
    // }

    void SetBasicParams(float freq, float phaseOffset01, int portamentoMS, bool syncEnabled, float syncFreq)
    {
        mSyncEnabled = syncEnabled;
        if (syncEnabled)
        {
            mSyncPhase.SetParams(freq, phaseOffset01);
            mMainPhase.SetParams(syncFreq, 0);
        }
        else
        {
            mMainPhase.SetParams(freq, phaseOffset01);
        }
    }

    void pulseWidth(float pulseWidth)
    {
        mPulseWidth = Clamp(pulseWidth, 0.001f, 0.999f);
        mPulseWidth = mPulseWidthTarget01 = pulseWidth;
    }

    template <typename TWaveformProvider>
    inline void Step(size_t i, bool doPWM, float *pwm32, bool doPM, float *pm32, float *out)
    {
        float phaseShift = 0;
        if (doPM)
        {
            phaseShift += pm32[i];
            // Potential extension: apply PM directly to the phase accumulator here.
        }

        if (doPWM)
        {
            mPulseWidth = pwm32[i];
            mPulseWidth = Clamp(mPulseWidth, 0.001f, 0.999f);
        }

        mMainPhase.StepWithoutFrac();

        float fboutput = mOutput;
        mOutput = mBlepDelay;
        mBlepDelay = 0;

        TWaveformProvider::Step(*this, fboutput, phaseShift);

        float x;
        if (mSyncEnabled && mSyncPhase.StepWithFrac(x))
        {
            TWaveformProvider::ResetPhaseDueToSync(*this, x);
        }

        out[i] = mOutput; // * mAmplitude;
    } // void Step() {

    void ProcessBlock(audio_block_t *pwm, audio_block_t *pm, audio_block_t *pOut)
    {
        float pwm32[AUDIO_BLOCK_SAMPLES];
        float pm32[AUDIO_BLOCK_SAMPLES];
        float out32[AUDIO_BLOCK_SAMPLES];
        if (pwm)
        {
            fast::Sample16To32Buffer(pwm->data, pwm32);
            fast::BufferOffsetInPlace(pwm32, mPulseWidthTarget01);
        }
        if (pm)
        {
            fast::Sample16To32Buffer(pm->data, pm32);
            fast::BufferScaleInPlace(pm32, mPMMultiplier);
        }
        switch (mWaveformShape)
        {
        case OscWaveformShape::VarTriangle:
            for (uint16_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
            {
                this->Step<VarTriangleWaveformProvider>(i, !!pwm, pwm32, !!pm, pm32, out32);
            }
            break;
        case OscWaveformShape::Pulse:
            for (uint16_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
            {
                this->Step<PulseWaveformProvider>(i, !!pwm, pwm32, !!pm, pm32, out32);
            }
            break;
        case OscWaveformShape::SawSync:
            for (uint16_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
            {
                this->Step<SawWaveformProvider>(i, !!pwm, pwm32, !!pm, pm32, out32);
            }
            break;
        case OscWaveformShape::Sine:
        default:
            for (uint16_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
            {
                this->Step<SineWaveformProvider>(i, !!pwm, pwm32, !!pm, pm32, out32);
            }
            break;
        }
        fast::Sample32To16Buffer(out32, pOut->data);
    }
}; // struct Oscillator

//////////////////////////////////////////////////////////////////////////////
struct AudioBandlimitedOsci : public AudioStream
{
    enum class INPUT_INDEX
    {
        pwm1 = 0,
        pm1,
        pwm2,
        pm2,
        pwm3,
        pm3,
    };
    static constexpr size_t INPUT_CONNECTION_COUNT = 6;

    AudioBandlimitedOsci() : AudioStream(INPUT_CONNECTION_COUNT, inputQueueArray)
    {
    }

    audio_block_t *inputQueueArray[INPUT_CONNECTION_COUNT];

    bool mIsPlaying = false;

    Oscillator mOsc[3];

    void ProcessOsc(Oscillator &osc, int pwmId, int pmId, int outId)
    {
        audio_block_t *out = allocate();
        if (!out)
            return;
        audio_block_t *pwm = receiveReadOnly(pwmId);
        audio_block_t *pm = receiveReadOnly(pmId);
        osc.ProcessBlock(pwm, pm, out);
        transmit(out, outId);
        release(out);

        if (pwm)
            release(pwm);
        if (pm)
            release(pm);
    }

    virtual void update() override
    {
        if (!mIsPlaying)
            return;
        ProcessOsc(mOsc[0], (int)INPUT_INDEX::pwm1, (int)INPUT_INDEX::pm1, 0);
        ProcessOsc(mOsc[1], (int)INPUT_INDEX::pwm2, (int)INPUT_INDEX::pm2, 1);
        ProcessOsc(mOsc[2], (int)INPUT_INDEX::pwm3, (int)INPUT_INDEX::pm3, 2);
    }

}; // class AudioBandlimitedOsci

} // namespace clarinoid
