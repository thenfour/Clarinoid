
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
    static void Step(TOscillator &caller, float &fboutput, float phaseShift)
    {
        caller.mMainPhase.mT -= floorf(caller.mMainPhase.mT);
        caller.mOutput = fast::sin((caller.mMainPhase.mT + phaseShift + fboutput * caller.mPMFeedbackAmt) * TWO_PI);
        // caller.mOutput = sinf((caller.mMainPhase.mT) * TWO_PI);
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
    static void Step(TOscillator &caller, float &fboutput, float phaseShift)
    {
        // TODO: use phase shift
        while (true)
        {
            if (!caller.mPulseStage)
            {
                if (caller.mMainPhase.mT < caller.mPulseWidth)
                    break;

                float x = (caller.mMainPhase.mT - caller.mPulseWidth) /
                          (caller.mWidthDelay - caller.mPulseWidth + caller.mMainPhase.mDt);
                float scale = caller.mMainPhase.mDt / (caller.mPulseWidth - caller.mPulseWidth * caller.mPulseWidth);

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

                float scale = caller.mMainPhase.mDt / (caller.mPulseWidth - caller.mPulseWidth * caller.mPulseWidth);

                caller.mOutput += scale * blamp0(x);
                caller.mBlepDelay += scale * blamp1(x);

                caller.mPulseStage = false;
            }
        }

        float naiveWave;

        if (caller.mMainPhase.mT <= caller.mPulseWidth)
        {
            naiveWave = 2 * caller.mMainPhase.mT / caller.mPulseWidth - 1;
        }
        else
        {
            naiveWave = -2 * (caller.mMainPhase.mT - caller.mPulseWidth) / (1 - caller.mPulseWidth) + 1;
        }

        caller.mBlepDelay += naiveWave;

        caller.mWidthDelay = caller.mPulseWidth;
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
    static void Step(TOscillator &caller, float &fboutput, float phaseShift)
    {
        // TODO: use phase shift
        while (true)
        {
            if (!caller.mPulseStage)
            {
                if (caller.mMainPhase.mT < caller.mPulseWidth)
                    break;

                float x = (caller.mMainPhase.mT - caller.mPulseWidth) /
                          (caller.mWidthDelay - caller.mPulseWidth + caller.mMainPhase.mDt);

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

        caller.mWidthDelay = caller.mPulseWidth;
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
    static void Step(TOscillator &caller, float &fboutput, float phaseShift)
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
            //mMainPhase.mT += pm32[i];
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
    }                     // void Step() {

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
