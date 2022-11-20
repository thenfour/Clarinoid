
#pragma once

#include <arm_math.h>

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

// https://gitlab.com/flojawi/teensy-polyblep-oscillator

/*
   Written by Florian Wirth, August 2020

   This is a module for the Teensy Audio Library which contains 3 independent
   oscillators with bandlimited basic waveforms, frequency modulation, PWM and hard-sync.

   The oscillators are based on two pieces of code from users at the kvraudio forum, namely:

   - The PolyBLEP algorithm is from the user "mystran" (https://www.kvraudio.com/forum/viewtopic.php?t=398553)
   - The hard-sync sawtooth is from the user "Tale" (https://www.kvraudio.com/forum/viewtopic.php?t=425054)

    The handling of portamento is from "Chip Audette's" OpenAudio_ArduinoLibrary
    (https://github.com/chipaudette/OpenAudio_ArduinoLibrary/blob/master/synth_waveform_F32.h)
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

struct SineWaveformProvider // : public WaveformProviderBase
{
    template <typename TOscillator>
    static void ResetPhaseDueToSync(TOscillator &caller, float x)
    {
        caller.mMainPhase.mT = x * caller.mMainPhase.mDt + caller.mMainPhase.mPhaseOffset;
        caller.mPulseStage = false;
    }

    template <typename TOscillator>
    static void Step(int i,
                     TOscillator &caller,
                     float pulseWidthShift,
                     float myLastSample,
                     float otherOsc0Sample,
                     float otherOsc1Sample)
    {
        caller.mMainPhase.mT -= floorf(caller.mMainPhase.mT);
        caller.mOutput = fast::sin((caller.mMainPhase.mT + caller.mMainPhase.mPhaseOffset +    //
                                    myLastSample * caller.mPMAmt0 * caller.mPMMultiplier +     // fm feedback
                                    otherOsc0Sample * caller.mPMAmt1 * caller.mPMMultiplier +  // fm input 1
                                    otherOsc1Sample * caller.mPMAmt2 * caller.mPMMultiplier) * // fm input 2
                                   TWO_PI);
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
    static void Step(int i,
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
    static void Step(int i,
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
    static void Step(int i,
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

    // float mPremultipliedFMfeedbackAmt = 0;
    // float mPremultipliedFMAmt0 = 0;
    // float mPremultipliedFMAmt1 = 0;

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
        // mRingMod0 = ringMod0;
        // mRingMod1 = ringMod1;
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
    inline void Step(size_t i, float *pwm32, float *out, float otherOsc0Sample, float otherOsc1Sample)
    {
        if (!mEnabled)
        {
            out[i] = 0;
            return;
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

        out[i] = mOutput;

        // hm you can't really continuously mix this in a straightforward way; the moment the mod crosses from positive
        // to negative, the whole thing instantly flips around.
        out[i] *= 1.0f - mRMAmt0 * mRingModStrengthN11 * fboutput;
        out[i] *= 1.0f - mRMAmt1 * mRingModStrengthN11 * otherOsc0Sample;
        out[i] *= 1.0f - mRMAmt2 * mRingModStrengthN11 * otherOsc1Sample;
    }

    void ProcessSample(int i, float *pwm, float otherOsc0Sample, float otherOsc1Sample)
    {
        switch (mWaveformShape)
        {
        case OscWaveformShape::VarTriangle:
            this->Step<VarTriangleWaveformProvider>(i, pwm, mBuffer, otherOsc0Sample, otherOsc1Sample);
            break;
        case OscWaveformShape::Pulse:
            this->Step<PulseWaveformProvider>(i, pwm, mBuffer, otherOsc0Sample, otherOsc1Sample);
            break;
        case OscWaveformShape::SawSync:
            this->Step<SawWaveformProvider>(i, pwm, mBuffer, otherOsc0Sample, otherOsc1Sample);
            break;
        case OscWaveformShape::Sine:
            this->Step<SineWaveformProvider>(i, pwm, mBuffer, otherOsc0Sample, otherOsc1Sample);
            break;
        default:
            CCASSERT(!"unsupported waveform in processblock");
            break;
        }
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
            if (pwmInputs[i] == nullptr)
                continue;
            fast::Sample16To32Buffer(pwmInputs[i]->data, pwmInputsF[i]);
            pwmInputPtrs[i] = pwmInputsF[i];

            // mOsc[i].mPremultipliedFMfeedbackAmt = mOsc[i].mPMFeedbackAmt * mOsc[i].mPMMultiplier;
            // mOsc[i].mPremultipliedFMAmt0 = mOsc[i].mPMAmt0 * mOsc[i].mPMMultiplier;
            // mOsc[i].mPremultipliedFMAmt1 = mOsc[i].mPMAmt1 * mOsc[i].mPMMultiplier;
        }

        for (uint16_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            mOsc[0].ProcessSample(i, pwmInputPtrs[0], mOsc[1].mOutput, mOsc[2].mOutput);
            mOsc[1].ProcessSample(i, pwmInputPtrs[1], mOsc[0].mOutput, mOsc[2].mOutput);
            mOsc[2].ProcessSample(i, pwmInputPtrs[2], mOsc[0].mOutput, mOsc[1].mOutput);
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
