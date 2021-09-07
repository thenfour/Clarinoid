
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

struct PhaseAccumulator
{
    float mFrequency = 0.;
    float mT = 0; // position in wave cycle, [0-1) // osc1_t
    float mPhaseOffset = 0;
    float mDt = 0; // cycles per sample, very small. amount of 0-1 cycle to advance each sample. // osc1_dt

    void SetFrequency(float freq)
    {
        freq = Clamp(freq, 0.0f, AUDIO_SAMPLE_RATE_EXACT / 2.0f);
        mFrequency = freq;
        mDt = mFrequency / AUDIO_SAMPLE_RATE_EXACT;
    }
    // returns true if phase has cycled, and then x is populated with the subsample
    bool Step(float &x)
    {
        mT += mDt;
        if (mT < 1)
            return false;
        mT -= 1;
        x = mT / mDt;
        return true;
    }
};

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

    void frequency(uint8_t oscillator, float freq)
    {
        mOsc[oscillator - 1].frequency(freq, mNotesPlaying);
    }

    void SetPhaseModRange(float r)
    {
        mOsc[0].mPMMultiplier = r;
        mOsc[1].mPMMultiplier = r;
        mOsc[2].mPMMultiplier = r;
    }

    void addNote()
    {
        // Call with your "noteOn" when hitting a new note, this is needed for portamento
        mNotesPlaying++;
    }

    void removeNote()
    {
        // Call with your "noteOff" command
        if (mNotesPlaying > 0)
        {
            mNotesPlaying--;
        }
    }

    virtual void update(void);

    audio_block_t *inputQueueArray[INPUT_CONNECTION_COUNT];

    uint8_t mNotesPlaying;

    struct OscillatorState
    {
        bool mSyncEnabled = false;
        PhaseAccumulator mSyncPhase;
        float mWaveformMorph01 = 0.5f; // curve: gModCurveLUT.LinearYIndex;

        float mFrequency = 0; // frequency1 - the frequency the user has specified.
        float mFreq = 0;      // the frequency the oscillator is actually using. think portamento.

        float mGain = 0; // osc1_gain
        OscWaveformShape mWaveformShape = OscWaveformShape::Sine;

        float mPulseWidthTarget01 = 0; // pulseWidth1
        float mPulseWidth = 0.5;       // osc1_pulseWidth 0,1

        float mBlepDelay = 0;     // osc1_blepDelay
        float mWidthDelay = 0;    // osc1_widthDelay
        bool mPulseStage = false; // osc1_pulseStage

        float mPortamentoTime =
            0; // osc1_portamentoTime - the configured duration, in seconds, of the portamento duration
        uint64_t mPortamentoSamples = 0; // osc1_portamentoSamples - the configured duration, in complete samples
        float mPortamentoIncrement =
            0; // osc1_portamentoIncrement - frequency delta per sample, within portamento duration
        uint64_t mCurrentPortamentoSample =
            0; // osc1_currentPortamentoSample -- position within portamento segemnt (0-mPortamentoSamples)

        float mT = 0; // position in wave cycle, [0-1) // osc1_t
        float mPhaseOffset = 0;
        float mDt = 0; // cycles per sample, very small. amount of 0-1 cycle to advance each sample. // osc1_dt

        float mOutput = 0;           // osc1_output
        float mPMMultiplier = 0.01f; // scaling PM input 0-1 phase is EXTREME, so we need a reasonable maximum.
        float mPMFeedbackAmt = 0.0f;

        void amplitude(float a)
        {
            mGain = Clamp(a, -1.0f, 1.0f);
        }

        void SetSyncParams(bool enabled, float freq)
        {
            mSyncEnabled = enabled;
            mSyncPhase.SetFrequency(freq);
        }

        void frequency(float freq, int notesPlaying)
        {
            freq = Clamp(freq, 0.0f, AUDIO_SAMPLE_RATE_EXACT / 2.0f);
            if (mPortamentoSamples > 0 && notesPlaying > 0)
            {
                mPortamentoIncrement = (freq - mFrequency) / (float)mPortamentoSamples;
                mCurrentPortamentoSample = 0;
            }
            else
            {
                mFrequency = freq;
            }
            mDt = mFrequency / AUDIO_SAMPLE_RATE_EXACT;
        }

        void portamentoTime(float seconds)
        {
            // set how long the oscillator sweeps up or down to the new frequency
            mPortamentoTime = seconds;
            mPortamentoSamples = floorf(seconds * AUDIO_SAMPLE_RATE_EXACT);
        }

        //  Use this for phase-retrigger everytime you send a noteOn, if you want a consistent sound.
        //  Don't use this for hard-sync, it will cause aliasing.
        //  You can also set the phases of the oscillators to different starting points.
        void ResetPhase()
        {
            mT = mPhaseOffset;
        }

        void SetPhaseOffset(float t01)
        {
            if (FloatEquals(t01, mPhaseOffset))
            {
                return;
            }
            mT = Frac(mT + (t01 - mPhaseOffset));
            mPhaseOffset = t01;
        }

        void waveform(OscWaveformShape wform)
        {
            mWaveformShape = wform;
        }

        void pulseWidth(float pulseWidth)
        {
            mPulseWidth = Clamp(pulseWidth, 0.001f, 0.999f);
            mPulseWidth = mPulseWidthTarget01 = pulseWidth;
        }

        int16_t *curveLookupState;

        // call before calculating the sample; this does A-rate modulation stuff
        inline void PreStep(size_t i, /* audio_block_t *fm1,*/ audio_block_t *pwm1, audio_block_t *pm1)
        {
            if (mPortamentoSamples > 0 && mCurrentPortamentoSample++ < mPortamentoSamples)
            {
                mFrequency += mPortamentoIncrement;
                mFreq = mFrequency;
                mDt = mFreq / AUDIO_SAMPLE_RATE_EXACT; // cycles per sample. the amount of waveform to advance each
                                                       // sample. very small.
            }

            if (pm1)
            {
                mT += fast::Sample16To32(pm1->data[i]) * mPMMultiplier;
            }

            // pulse Width Modulation:
            if (pwm1)
            {
                mPulseWidth = mPulseWidthTarget01 + fast::Sample16To32(pwm1->data[i]);
                mPulseWidth = Clamp(mPulseWidth, 0.001f, 0.999f);
            }
        }

        inline void PostStep(size_t i, audio_block_t *out)
        {
            // do some sync
            float x;
            if (mSyncEnabled && mSyncPhase.Step(x))
            {
                this->Sync(x);
            }

            float o = gModCurveLUT.Transfer32(mOutput, curveLookupState);
            o = o * mGain;
            out->data[i] = fast::Sample32To16(o); // o * 32768.0f;
        }

        inline void Step()
        {
            float fboutput = mOutput;
            mOutput = mBlepDelay;
            mBlepDelay = 0;

            mT += mDt;

            // triangle and sawtooth wave
            switch (mWaveformShape)
            {
            case OscWaveformShape::Sine: {
                mT -= floorf(mT);
                mOutput = fast::sin((mT + fboutput * mPMFeedbackAmt) * TWO_PI);
                break;
            }

            case OscWaveformShape::VarTriangle: {
                while (true)
                {
                    if (!mPulseStage)
                    {
                        if (mT < mPulseWidth)
                            break;

                        float x = (mT - mPulseWidth) / (mWidthDelay - mPulseWidth + mDt);
                        float scale = mDt / (mPulseWidth - mPulseWidth * mPulseWidth);

                        mOutput -= scale * blamp0(x);
                        mBlepDelay -= scale * blamp1(x);

                        mPulseStage = true;
                    }
                    if (mPulseStage)
                    {
                        if (mT < 1)
                            break;

                        // we have crossed over phase,
                        // remainder phase 0-1 / (freq/samplerate)
                        mT -= 1;
                        // x = number of master samples crossed over phase, but because we're processing 1 sample at a
                        // time, this is always 0-1.
                        float x = mT / mDt;

                        float scale = mDt / (mPulseWidth - mPulseWidth * mPulseWidth);

                        mOutput += scale * blamp0(x);
                        mBlepDelay += scale * blamp1(x);

                        mPulseStage = false;
                    }
                }

                float naiveWave;

                if (mT <= mPulseWidth)
                {
                    naiveWave = 2 * mT / mPulseWidth - 1;
                }
                else
                {
                    naiveWave = -2 * (mT - mPulseWidth) / (1 - mPulseWidth) + 1;
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
                        if (mT < mPulseWidth)
                            break;

                        float x = (mT - mPulseWidth) / (mWidthDelay - mPulseWidth + mDt);

                        mOutput -= blep0(x);
                        mBlepDelay -= blep1(x);

                        mPulseStage = true;
                    }
                    if (mPulseStage)
                    {
                        if (mT < 1)
                            break;

                        // we have crossed over phase.
                        mT -= 1;
                        // x = number of master samples crossed over phase, but because we're processing 1 sample at a
                        // time, this is always 0-1.
                        float x = mT / mDt;

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
                mT -= floorf(mT);

                if (mT < mDt)
                {
                    float x = mT / mDt;
                    mOutput -= 0.5 * blep0(x);
                    mBlepDelay -= 0.5 * blep1(x);
                }

                mBlepDelay += mT;

                mOutput = mOutput * 2 - 1;
            }
            break;

            default: {
                mT -= floorf(mT);
                mOutput = 0;
                break;
            }
            }
        }

        inline void Sync(float x)
        {
            if (mWaveformShape != OscWaveformShape::SawSync)
            {
                // this is actually kinda broken. aliased and 
                mT = x * mDt + mPhaseOffset;
                mPulseStage = false;
                return;
            }

            // OscWaveformShape::SawSync
            mOutput = mBlepDelay;
            mBlepDelay = 0;

            float scale = mDt / mSyncPhase.mDt;
            scale -= floorf(scale);
            if (scale <= 0)
            {
                scale = 1;
            }

            mOutput -= 0.5 * scale * blep0(x);
            mBlepDelay -= 0.5 * scale * blep1(x);

            // increase slave phase by partial sample
            float dt = (1 - x) * mDt;
            mT += dt;
            mT -= floorf(mT);

            if (mT < dt)
            {
                mT += x * mDt;
                mT -= floorf(mT);

                // process transition for the slave
                float x2 = mT / mDt;
                mOutput -= 0.5 * blep0(x2);
                mBlepDelay -= 0.5 * blep1(x2);
            }

            // reset slave phase:
            mT = x * mDt;

            mBlepDelay += mT;

            mOutput = mOutput * 2 - 1;
        }

        template <typename Ta, typename Tb>
        void update(AudioStream &stream, Ta &&receiveReadOnly, Tb &&transmit, uint8_t outId, int pwmId, int pmId)
        {
            audio_block_t *out1 = stream.allocate();
            if (!out1)
                return;
            audio_block_t *pwm1 = receiveReadOnly(pwmId);
            audio_block_t *pm1 = receiveReadOnly(pmId);
            for (uint16_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
            {
                this->curveLookupState = gModCurveLUT.BeginLookupF(this->mWaveformMorph01 * 2 - 1);
                this->PreStep(i, pwm1, pm1);
                this->Step();
                this->PostStep(i, out1);
            }

            transmit(out1, outId);
            stream.release(out1);

            if (pwm1)
                stream.release(pwm1);
            if (pm1)
                stream.release(pm1);
        }
    }; // struct OscillatorState

    OscillatorState mOsc[3];
}; // namespace clarinoid

void AudioBandlimitedOsci::update()
{
    if (mNotesPlaying <= 0)
        return;
    auto callableReceiveReadOnly = [&](unsigned int i) { return this->receiveReadOnly(i); };
    auto callableTransmit = [&](audio_block_t *block, uint8_t id) { this->transmit(block, id); };
    mOsc[0].update(*this, callableReceiveReadOnly, callableTransmit, 0, (int)INPUT_INDEX::pwm1, (int)INPUT_INDEX::pm1);
    mOsc[1].update(*this, callableReceiveReadOnly, callableTransmit, 1, (int)INPUT_INDEX::pwm2, (int)INPUT_INDEX::pm2);
    mOsc[2].update(*this, callableReceiveReadOnly, callableTransmit, 2, (int)INPUT_INDEX::pwm3, (int)INPUT_INDEX::pm3);
}

} // namespace clarinoid
