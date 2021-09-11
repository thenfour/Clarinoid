
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
  private:
    float mFrequency = 0; // frequency1 - the frequency the user has specified.
    float mPortamentoSeconds =
        0; // osc1_portamentoTime - the configured duration, in seconds, of the portamento duration
    uint64_t mPortamentoSamples = 0; // osc1_portamentoSamples - the configured duration, in complete samples
    float mPortamentoIncrement = 0; // osc1_portamentoIncrement - frequency delta per sample, within portamento duration
    uint64_t mCurrentPortamentoSample =
        0; // osc1_currentPortamentoSample -- position within portamento segemnt (0-mPortamentoSamples)

  public:
    float mFreq = 0; // the LIVE frequency the oscillator is actually using; is adjusted
    float mT = 0;    // position in wave cycle, [0-1) // osc1_t
    float mPhaseOffset = 0;
    float mDt = 0; // cycles per sample, very small. amount of 0-1 cycle to advance each sample. // osc1_dt

    void ResetPhase()
    {
        mT = mPhaseOffset;
    }

    // it's easier to control implementation details when all params are set at once. this is also how SynthVoice does
    // things anyway so..
    void SetParams(float freq, bool instantFreq, float phaseOffset01, float portamentoSeconds)
    {
        // set how long the oscillator sweeps up or down to the new frequency
        mPortamentoSeconds = portamentoSeconds;
        mPortamentoSamples = floorf(portamentoSeconds * AUDIO_SAMPLE_RATE_EXACT);

        // frequency
        freq = Clamp(freq, 0.0f, AUDIO_SAMPLE_RATE_EXACT / 2.0f);
        if (mPortamentoSamples > 0 && !instantFreq)
        {
            mPortamentoIncrement = (freq - mFrequency) / (float)mPortamentoSamples;
            mCurrentPortamentoSample = 0;
        }
        else
        {
            mFrequency = freq;
        }
        mDt = mFrequency / AUDIO_SAMPLE_RATE_EXACT;

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
        if (mPortamentoSamples > 0 && mCurrentPortamentoSample++ < mPortamentoSamples)
        {
            mFrequency += mPortamentoIncrement;
            mFreq = mFrequency;
            mDt = mFreq / AUDIO_SAMPLE_RATE_EXACT; // cycles per sample. the amount of waveform to advance each
                                                   // sample. very small.
        }
        mT += mDt;
        if (mT < 1)
            return false;
        mT -= 1;
        x = mT / mDt;
        return true;
    }
};

struct Oscillator
{
    PhaseAccumulator mMainPhase;

    PhaseAccumulator mSyncPhase; // when sync is enabled, this is the phase of the "main" frequency. it resets the phase
                                 // of mMainPhase.
    bool mSyncEnabled = false;
    float mAmplitude = 0;
    //float mWaveformMorph01 = 0.5f; // curve: gModCurveLUT.LinearYIndex;

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

    void SetAmplitude(float amplitude01) {
        mAmplitude = amplitude01;
    }

    void SetBasicParams(float freq,
                        bool instantFreq,
                        float phaseOffset01,
                        float portamentoSeconds,
                        bool syncEnabled,
                        float syncFreq)
    {
        mSyncEnabled = syncEnabled;
        if (syncEnabled)
        {
            mSyncPhase.SetParams(freq, instantFreq, phaseOffset01, portamentoSeconds);
            mMainPhase.SetParams(syncFreq, true, 0, 0);
        }
        else
        {
            mMainPhase.SetParams(freq, instantFreq, phaseOffset01, portamentoSeconds);
        }
    }

    void pulseWidth(float pulseWidth)
    {
        mPulseWidth = Clamp(pulseWidth, 0.001f, 0.999f);
        mPulseWidth = mPulseWidthTarget01 = pulseWidth;
    }

    inline void Step(size_t i, audio_block_t *pwm1, audio_block_t *pm1, audio_block_t *out)
    {
        if (pm1)
        {
            mMainPhase.mT += fast::Sample16To32(pm1->data[i]) * mPMMultiplier;
        }

        // pulse Width Modulation:
        if (pwm1)
        {
            mPulseWidth = mPulseWidthTarget01 + fast::Sample16To32(pwm1->data[i]);
            mPulseWidth = Clamp(mPulseWidth, 0.001f, 0.999f);
        }

        float fboutput = mOutput;
        mOutput = mBlepDelay;
        mBlepDelay = 0;

        mMainPhase.StepWithoutFrac();

        // triangle and sawtooth wave
        switch (mWaveformShape)
        {
        case OscWaveformShape::Sine: {
            mOutput = fast::sin((mMainPhase.mT + fboutput * mPMFeedbackAmt) * TWO_PI);
            break;
        }

        case OscWaveformShape::VarTriangle: {
            while (true)
            {
                if (!mPulseStage)
                {
                    if (mMainPhase.mT < mPulseWidth)
                        break;

                    float x = (mMainPhase.mT - mPulseWidth) / (mWidthDelay - mPulseWidth + mMainPhase.mDt);
                    float scale = mMainPhase.mDt / (mPulseWidth - mPulseWidth * mPulseWidth);

                    mOutput -= scale * blamp0(x);
                    mBlepDelay -= scale * blamp1(x);

                    mPulseStage = true;
                }
                if (mPulseStage)
                {
                    if (mMainPhase.mT < 1)
                        break;

                    // we have crossed over phase,
                    // remainder phase 0-1 / (freq/samplerate)
                    mMainPhase.mT -= 1;
                    // x = number of master samples crossed over phase, but because we're processing 1 sample at a
                    // time, this is always 0-1.
                    float x = mMainPhase.mT / mMainPhase.mDt;

                    float scale = mMainPhase.mDt / (mPulseWidth - mPulseWidth * mPulseWidth);

                    mOutput += scale * blamp0(x);
                    mBlepDelay += scale * blamp1(x);

                    mPulseStage = false;
                }
            }

            float naiveWave;

            if (mMainPhase.mT <= mPulseWidth)
            {
                naiveWave = 2 * mMainPhase.mT / mPulseWidth - 1;
            }
            else
            {
                naiveWave = -2 * (mMainPhase.mT - mPulseWidth) / (1 - mPulseWidth) + 1;
            }

            mBlepDelay += naiveWave;

            mWidthDelay = mPulseWidth;
        }
        break;

        case OscWaveformShape::Pulse: {
            while (true)
            {
                if (!mPulseStage)
                {
                    if (mMainPhase.mT < mPulseWidth)
                        break;

                    float x = (mMainPhase.mT - mPulseWidth) / (mWidthDelay - mPulseWidth + mMainPhase.mDt);

                    mOutput -= blep0(x);
                    mBlepDelay -= blep1(x);

                    mPulseStage = true;
                }
                if (mPulseStage)
                {
                    if (mMainPhase.mT < 1)
                        break;

                    // we have crossed over phase.
                    mMainPhase.mT -= 1;
                    // x = number of master samples crossed over phase, but because we're processing 1 sample at a
                    // time, this is always 0-1.
                    float x = mMainPhase.mT / mMainPhase.mDt;

                    mOutput += blep0(x);
                    mBlepDelay += blep1(x);

                    mPulseStage = false;
                }
            }

            float naiveWave = mPulseStage ? -1.0f : 1.0f;

            mBlepDelay += naiveWave;

            mWidthDelay = mPulseWidth;
        }
        break;

        case OscWaveformShape::SawSync: {
            mMainPhase.mT -= floorf(mMainPhase.mT);

            if (mMainPhase.mT < mMainPhase.mDt)
            {
                float x = mMainPhase.mT / mMainPhase.mDt;
                mOutput -= 0.5 * blep0(x);
                mBlepDelay -= 0.5 * blep1(x);
            }

            mBlepDelay += mMainPhase.mT;

            mOutput = mOutput * 2 - 1;
        }
        break;

        default: {
            mMainPhase.mT -= floorf(mMainPhase.mT);
            mOutput = 0;
            break;
        }
        }

        // do some sync
        float x;
        if (mSyncEnabled && mSyncPhase.StepWithFrac(x))
        {
            this->ResetPhaseDueToSync(x);
        }

        //float o = gModCurveLUT.Transfer32(mOutput, curveLookupState);

        out->data[i] = fast::Sample32To16(mOutput * mAmplitude);
    } // void Step() {

    inline void ResetPhaseDueToSync(float x)
    {
        if (mWaveformShape != OscWaveformShape::SawSync)
        {
            // this is actually kinda broken, but better than the fallthrough case
            mMainPhase.mT = x * mMainPhase.mDt + mMainPhase.mPhaseOffset;
            mPulseStage = false;
            return;
        }

        // OscWaveformShape::SawSync
        mOutput = mBlepDelay;
        mBlepDelay = 0;

        float scale = mMainPhase.mDt / mSyncPhase.mDt;
        scale -= floorf(scale);
        if (scale <= 0)
        {
            scale = 1;
        }

        mOutput -= 0.5 * scale * blep0(x);
        mBlepDelay -= 0.5 * scale * blep1(x);

        // increase slave phase by partial sample
        float dt = (1 - x) * mMainPhase.mDt;
        mMainPhase.mT += dt;
        mMainPhase.mT -= floorf(mMainPhase.mT);

        if (mMainPhase.mT < dt)
        {
            mMainPhase.mT += x * mMainPhase.mDt;
            mMainPhase.mT -= floorf(mMainPhase.mT);

            // process transition for the slave
            float x2 = mMainPhase.mT / mMainPhase.mDt;
            mOutput -= 0.5 * blep0(x2);
            mBlepDelay -= 0.5 * blep1(x2);
        }

        // reset slave phase:
        mMainPhase.mT = x * mMainPhase.mDt;

        mBlepDelay += mMainPhase.mT;

        mOutput = mOutput * 2 - 1;
    }

    void ProcessBlock(audio_block_t *pwm, audio_block_t *pm, audio_block_t *pOut)
    {
        //this->curveLookupState = gModCurveLUT.BeginLookupF(this->mWaveformMorph01 * 2 - 1);
        for (uint16_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            this->Step(i, pwm, pm, pOut);
        }
    }
}; // struct OscillatorState

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

    void ProcessOsc(Oscillator& osc, int pwmId, int pmId, int outId) {
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
