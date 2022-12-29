// https://nbviewer.org/github/wrl/pd-blosc/blob/master/notebook/Polyblep.ipynb
// 

#pragma once

#include <arm_math.h>

namespace clarinoid
{
/*
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
    float mCurrentTarget = 0;

    // call this every audio buffer interval; returns a smoothed frequency signal.
    float KStep(float targetValue, int durationMS)
    {
        // calculate params
        if (durationMS > 0)
        {
            // used to be a manual "reset" argument, but this is more simple.
            if (!FloatEquals(mCurrentTarget, targetValue))
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

        mCurrentTarget = targetValue;

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

/*
   Adapted from https://gitlab.com/flojawi/teensy-polyblep-oscillator
   by Florian Wirth

   - The PolyBLEP algorithm is from the user "mystran" (https://www.kvraudio.com/forum/viewtopic.php?t=398553)
    also see relevant papers regarding BLAMP and BLEP band limiting:
   https://www.dafx.de/paper-archive/2016/dafxpapers/18-DAFx-16_paper_33-PN.pdf
   - The hard-sync sawtooth is from the user "Tale" (https://www.kvraudio.com/forum/viewtopic.php?t=425054)
*/

// tracks phase and frequency supporting portamento
struct PhaseAccumulator
{
    float mFreq =
        0; // the LIVE frequency the oscillator is actually using (only differs from mFrequency during portamento)
    float mT = 0; // position in wave cycle, [0-1) // osc1_t
    float mPhaseOffset = 0;
    float mDt = 0; // cycles per sample, very small. amount of 0-1 cycle to advance each sample. // osc1_dt

    void ResetPhase()
    {
        mT = mPhaseOffset;
    }

    // it's easier to control implementation details when all params are set at once. this is also how SynthVoice does
    // things anyway so..
    void SetParams(float freq, float phaseOffset01)
    {
        mFreq = freq;
        mDt = mFreq / AUDIO_SAMPLE_RATE_EXACT;

        if (FloatEquals(phaseOffset01, mPhaseOffset))
        {
            return;
        }
        mT = Frac(mT + (phaseOffset01 - mPhaseOffset));
        mPhaseOffset = phaseOffset01;
    }

    // steps the phase accum, but does not track crossing over cycle
    void StepWithoutFrac()
    {
        mT += mDt;
    }

    // steps the phase accum, and frac()s back to 0 if it crosses 1.
    // returns true if phase has cycled, and then x is populated with the subsample
    bool StepWithFrac(float &x)
    {
        mT += mDt;
        if (mT < 1)
            return false;
        mT -= 1;
        x = mT / mDt;
        return true;
    }
};

struct SineWaveformProvider
{
    template <typename TOscillator>
    static void ResetPhaseDueToSync(TOscillator &caller, float x)
    {
        caller.mMainPhase.mT = x * caller.mMainPhase.mDt + caller.mMainPhase.mPhaseOffset;
        caller.mPulseStage = false;
    }

    template <typename TOscillator>
    static float Step(int i,
                      TOscillator &caller,
                      float pulseWidthShift,
                      float myLastSample,
                      float otherOsc0Sample,
                      float otherOsc1Sample)
    {
        caller.mMainPhase.mT -= floorf(caller.mMainPhase.mT); // keep within 0-1 range to help float precision.
        float ret = fast::cos((caller.mMainPhase.mT + caller.mMainPhase.mPhaseOffset + // use cos() because it's 0 at 0, so starting is a pleasant ramp.
                               myLastSample * caller.mPrecalc.mPMAmt0 + // fm feedback - too strong at full level
                               otherOsc0Sample * caller.mPrecalc.mPMAmt1 +     // fm input 1
                               otherOsc1Sample * caller.mPrecalc.mPMAmt2) *    // fm input 2
                              TWO_PI);
        caller.mOutput = ret;
        return ret;
    }
};

struct VarTriangleWaveformProvider // : public WaveformProviderBase
{
    template <typename TOscillator>
    static void ResetPhaseDueToSync(TOscillator &caller, float x)
    {
        caller.mMainPhase.mT = x * caller.mMainPhase.mDt + caller.mMainPhase.mPhaseOffset;
        caller.mPulseStage = false;
    }

    template <typename TOscillator>
    static float Step(int i,
                      TOscillator &caller,
                      float pulseWidthShift,
                      float myLastSample,
                      float otherOsc0Sample,
                      float otherOsc1Sample)
    {
        // TODO: use phase shift
        float pulseWidth = Clamp(pulseWidthShift + caller.mPulseWidth, 0.001f, 0.999f);
        while (true)
        {
            if (!caller.mPulseStage)
            {
                if (caller.mMainPhase.mT < pulseWidth)
                    break;

                float x =
                    (caller.mMainPhase.mT - pulseWidth) / (caller.mWidthDelay - pulseWidth + caller.mMainPhase.mDt);
                float scale = caller.mMainPhase.mDt / (pulseWidth - pulseWidth * pulseWidth);

                caller.mOutput -= scale * blamp0(x);
                caller.mBlepDelay -= scale * blamp1(x);

                caller.mPulseStage = true;
            }
            if (caller.mPulseStage)
            {
                if (caller.mMainPhase.mT < 1)
                    break;

                // we have crossed over phase,
                // remainder phase 0-1 / (freq/samplerate)
                caller.mMainPhase.mT -= 1;
                // x = number of master samples crossed over phase, but because we're processing 1 sample at a
                // time, this is always 0-1.
                float x = caller.mMainPhase.mT / caller.mMainPhase.mDt;

                float scale = caller.mMainPhase.mDt / (pulseWidth - pulseWidth * pulseWidth);

                caller.mOutput += scale * blamp0(x);
                caller.mBlepDelay += scale * blamp1(x);

                caller.mPulseStage = false;
            }
        }

        float naiveWave;

        if (caller.mMainPhase.mT <= pulseWidth)
        {
            naiveWave = 2 * caller.mMainPhase.mT / pulseWidth - 1;
        }
        else
        {
            naiveWave = -2 * (caller.mMainPhase.mT - pulseWidth) / (1 - pulseWidth) + 1;
        }

        caller.mBlepDelay += naiveWave;

        caller.mWidthDelay = pulseWidth;
        return caller.mOutput;
    }
};

struct PulseWaveformProvider
{
    template <typename TOscillator>
    static void ResetPhaseDueToSync(TOscillator &caller, float x)
    {
        caller.mMainPhase.mT = x * caller.mMainPhase.mDt + caller.mMainPhase.mPhaseOffset;
        caller.mPulseStage = false;
    }
    template <typename TOscillator>
    static float Step(int i,
                      TOscillator &caller,
                      float pulseWidthShift,
                      float myLastSample,
                      float otherOsc0Sample,
                      float otherOsc1Sample)
    {
        // TODO: use phase shift
        float pulseWidth = Clamp(pulseWidthShift + caller.mPulseWidth, 0.001f, 0.999f);
        while (true)
        {
            if (!caller.mPulseStage)
            {
                if (caller.mMainPhase.mT < pulseWidth)
                    break;

                float x =
                    (caller.mMainPhase.mT - pulseWidth) / (caller.mWidthDelay - pulseWidth + caller.mMainPhase.mDt);

                caller.mOutput -= blep0(x);
                caller.mBlepDelay -= blep1(x);

                caller.mPulseStage = true;
            }
            if (caller.mPulseStage)
            {
                if (caller.mMainPhase.mT < 1)
                    break;

                // we have crossed over phase.
                caller.mMainPhase.mT -= 1;
                // x = number of master samples crossed over phase, but because we're processing 1 sample at a
                // time, this is always 0-1.
                float x = caller.mMainPhase.mT / caller.mMainPhase.mDt;

                caller.mOutput += blep0(x);
                caller.mBlepDelay += blep1(x);

                caller.mPulseStage = false;
            }
        }

        float naiveWave = caller.mPulseStage ? -1.0f : 1.0f;

        caller.mBlepDelay += naiveWave;

        caller.mWidthDelay = pulseWidth;

        return caller.mOutput;
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

        caller.mOutput -= 0.5 * scale * blep0(x);
        caller.mBlepDelay -= 0.5 * scale * blep1(x);

        // increase slave phase by partial sample
        float dt = (1 - x) * caller.mMainPhase.mDt;
        caller.mMainPhase.mT += dt;
        caller.mMainPhase.mT -= floorf(caller.mMainPhase.mT);

        if (caller.mMainPhase.mT < dt)
        {
            caller.mMainPhase.mT += x * caller.mMainPhase.mDt;
            caller.mMainPhase.mT -= floorf(caller.mMainPhase.mT);

            // process transition for the slave
            float x2 = caller.mMainPhase.mT / caller.mMainPhase.mDt;
            caller.mOutput -= 0.5 * blep0(x2);
            caller.mBlepDelay -= 0.5 * blep1(x2);
        }

        // reset slave phase:
        caller.mMainPhase.mT = x * caller.mMainPhase.mDt;

        caller.mBlepDelay += caller.mMainPhase.mT;

        caller.mOutput = caller.mOutput * 2 - 1;
    }

    template <typename TOscillator>
    static float Step(int i,
                      TOscillator &caller,
                      float pulseWidthShift,
                      float myLastSample,
                      float otherOsc0Sample,
                      float otherOsc1Sample)
    {
        // TODO: use phase shift
        caller.mMainPhase.mT -= floorf(caller.mMainPhase.mT);

        if (caller.mMainPhase.mT < caller.mMainPhase.mDt)
        {
            float x = caller.mMainPhase.mT / caller.mMainPhase.mDt;
            caller.mOutput -= 0.5 * blep0(x);
            caller.mBlepDelay -= 0.5 * blep1(x);
        }

        caller.mBlepDelay += caller.mMainPhase.mT;

        caller.mOutput = caller.mOutput * 2 - 1;
        return caller.mOutput;
    }
};

struct Oscillator
{
    PhaseAccumulator mMainPhase;

    bool mHardSyncEnabled = false;
    PhaseAccumulator mSyncPhase; // when sync is enabled, this is the phase of the "main" frequency. it resets the phase
                                 // of mMainPhase.

    bool mEnabled = true;

    OscWaveformShape mWaveformShape = OscWaveformShape::Sine;

    float mPulseWidthTarget01 = 0; // pulseWidth1
    float mPulseWidth = 0.5;       // osc1_pulseWidth 0,1

    float mBlepDelay = 0;     // osc1_blepDelay
    float mWidthDelay = 0;    // osc1_widthDelay
    bool mPulseStage = false; // osc1_pulseStage

    float mOutput = 0;           // osc1_output
    float mPMMultiplier = 0.01f; // scaling PM input 0-1 phase is EXTREME, so we need a reasonable maximum.
    // float mPMFeedbackAmt = 0.0f;
    // bool mRingMod0 = false;
    // bool mRingMod1 = false;
    float mRingModStrengthN11 = 1.0f;

    // save this so it can be used for efficient a-rate modulation
    float mBuffer[AUDIO_BLOCK_SAMPLES] = {0};

    float mPMAmt0 = 0;
    float mPMAmt1 = 0;
    float mPMAmt2 = 0;

    float mRMAmt0 = 0;
    float mRMAmt1 = 0;
    float mRMAmt2 = 0;

    struct
    {
        float mPMAmt0 = 0;
        float mPMAmt1 = 0;
        float mPMAmt2 = 0;
        float mRMAmt0 = 0;
        float mRMAmt1 = 0;
        float mRMAmt2 = 0;
        float mRMAmt0Inv = 0; // 1-mRMAmtN
        float mRMAmt1Inv = 0; // 1-mRMAmtN
        float mRMAmt2Inv = 0; // 1-mRMAmtN
    } mPrecalc;

    //  Use this for phase-retrigger everytime you send a noteOn, if you want a consistent sound.
    //  Don't use this for hard-sync, it will cause aliasing.
    //  You can also set the phases of the oscillators to different starting points.
    void ResetPhase()
    {
        mMainPhase.ResetPhase();
    }

    void SetBasicParams(OscWaveformShape wform,
                        float freq,
                        float phaseOffset01,
                        int portamentoMS,
                        bool hardSyncEnabled,
                        float syncFreq,
                        float ringModStrengthN11,
                        float pmMultiplier)
    {
        mWaveformShape = wform;
        mHardSyncEnabled = hardSyncEnabled;
        mPMMultiplier = pmMultiplier;
        mRingModStrengthN11 = ringModStrengthN11;
        if (mHardSyncEnabled)
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
    inline float Step(size_t i, float *pwm32, float *out, float otherOsc0Sample, float otherOsc1Sample)
    {
        if (!mEnabled)
        {
            out[i] = 0;
            return mOutput;
        }

        float pulseWidthShift = 0;
        if (pwm32)
        {
            pulseWidthShift = pwm32[i];
        }

        mMainPhase.StepWithoutFrac();

        float fboutput = mOutput;
        mOutput = mBlepDelay;
        mBlepDelay = 0;

        TWaveformProvider::Step(i, *this, pulseWidthShift, fboutput, otherOsc0Sample, otherOsc1Sample);

        float x;
        if (mHardSyncEnabled && mSyncPhase.StepWithFrac(x))
        {
            TWaveformProvider::ResetPhaseDueToSync(*this, x);
        }

        mOutput = mOutput * mOutput * mPrecalc.mRMAmt0 + mOutput * mPrecalc.mRMAmt0Inv;
        mOutput = mOutput * otherOsc0Sample * mPrecalc.mRMAmt1 + mOutput * mPrecalc.mRMAmt1Inv;
        mOutput = mOutput * otherOsc1Sample * mPrecalc.mRMAmt2 + mOutput * mPrecalc.mRMAmt2Inv;

        out[i] = mOutput;
        return mOutput;
    }

    float ProcessSample(int i, float *pwm, float otherOsc0Sample, float otherOsc1Sample)
    {
        switch (mWaveformShape)
        {
        case OscWaveformShape::VarTriangle:
            return this->Step<VarTriangleWaveformProvider>(i, pwm, mBuffer, otherOsc0Sample, otherOsc1Sample);
        case OscWaveformShape::Pulse:
            return this->Step<PulseWaveformProvider>(i, pwm, mBuffer, otherOsc0Sample, otherOsc1Sample);
        case OscWaveformShape::SawSync:
            return this->Step<SawWaveformProvider>(i, pwm, mBuffer, otherOsc0Sample, otherOsc1Sample);
        case OscWaveformShape::Sine:
            return this->Step<SineWaveformProvider>(i, pwm, mBuffer, otherOsc0Sample, otherOsc1Sample);
        default:
            CCASSERT(!"unsupported waveform in processblock");
            break;
        }
        return 0;
    }

}; // struct Oscillator

//////////////////////////////////////////////////////////////////////////////
struct AudioBandlimitedOsci : public AudioStream
{
    enum class INPUT_INDEX
    {
        pwm1 = 0,
        pwm2,
        pwm3,
    };
    static constexpr size_t INPUT_CONNECTION_COUNT = 3;

    AudioBandlimitedOsci() : AudioStream(INPUT_CONNECTION_COUNT, inputQueueArray)
    {
    }

    audio_block_t *inputQueueArray[INPUT_CONNECTION_COUNT];

    bool mIsPlaying = false;

    Oscillator mOsc[3];

    virtual void update() override
    {
        if (!mIsPlaying)
            return;

        audio_block_t *pwmInputs[POLYBLEP_OSC_COUNT] = {
            receiveReadOnly((int)INPUT_INDEX::pwm1),
            receiveReadOnly((int)INPUT_INDEX::pwm2),
            receiveReadOnly((int)INPUT_INDEX::pwm3),
        };

        float pwmInputsF[AUDIO_BLOCK_SAMPLES][POLYBLEP_OSC_COUNT];

        float *pwmInputPtrs[POLYBLEP_OSC_COUNT] = {nullptr};
        for (size_t i = 0; i < POLYBLEP_OSC_COUNT; ++i)
        {
            mOsc[i].mPrecalc.mPMAmt0 = mOsc[i].mPMAmt0 * mOsc[i].mPMMultiplier * 0.3f; // feedback, use less.
            mOsc[i].mPrecalc.mPMAmt1 = mOsc[i].mPMAmt1 * mOsc[i].mPMMultiplier * 5.0f;
            mOsc[i].mPrecalc.mPMAmt2 = mOsc[i].mPMAmt2 * mOsc[i].mPMMultiplier * 5.0f;

            mOsc[i].mPrecalc.mRMAmt0 = mOsc[i].mRMAmt0 * mOsc[i].mRingModStrengthN11 * 2.0f;
            mOsc[i].mPrecalc.mRMAmt1 = mOsc[i].mRMAmt1 * mOsc[i].mRingModStrengthN11 * 2.0f;
            mOsc[i].mPrecalc.mRMAmt2 = mOsc[i].mRMAmt2 * mOsc[i].mRingModStrengthN11 * 2.0f;

            mOsc[i].mPrecalc.mRMAmt0Inv = 1.0f - mOsc[i].mPrecalc.mRMAmt0;
            mOsc[i].mPrecalc.mRMAmt1Inv = 1.0f - mOsc[i].mPrecalc.mRMAmt1;
            mOsc[i].mPrecalc.mRMAmt2Inv = 1.0f - mOsc[i].mPrecalc.mRMAmt2;

            if (pwmInputs[i] != nullptr)
            {
                fast::Sample16To32Buffer(pwmInputs[i]->data, pwmInputsF[i]);
                pwmInputPtrs[i] = pwmInputsF[i];
            }
        }

        float s0 = mOsc[0].mOutput;
        float s1 = mOsc[1].mOutput;
        float s2 = mOsc[2].mOutput;

        for (uint16_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            s0 = mOsc[0].ProcessSample(i, pwmInputPtrs[0], s1, s2);
            s1 = mOsc[1].ProcessSample(i, pwmInputPtrs[1], s0, s2);
            s2 = mOsc[2].ProcessSample(i, pwmInputPtrs[2], s0, s1);
        }

        for (size_t i = 0; i < POLYBLEP_OSC_COUNT; ++i)
        {
            if (pwmInputs[i])
            {
                release(pwmInputs[i]);
            }

            audio_block_t *out = allocate();
            if (!out)
                return;
            fast::Sample32To16Buffer(mOsc[i].mBuffer, out->data);
            transmit(out, i);
            release(out);
        }
    }

}; // class AudioBandlimitedOsci

} // namespace clarinoid
