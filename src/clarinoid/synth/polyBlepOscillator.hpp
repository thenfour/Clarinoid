// todo:

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

    INPUT CONNECTIONS (12):
    0 = fm1
    1 = pwm1
    2 = pm1
    3 = am1

    4 = fm2
    5 = pwm2
    6 = pm2
    7 = am2

    8 = fm3
    9 = pwm3
    10 = pm3
    11 = am3

    OUTPUT CONNECTIONS:
    0 = osc1
    1 = osc2
    2 = osc3
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

    The frequency modulation uses Paul Stoffregens adaption of the exp2 algorithm from Laurent de Soras
    - Original algorithm (https://www.musicdsp.org/en/latest/Other/106-fast-exp2-approximation.html)
    - Pauls adaptation for Teensy (https://github.com/PaulStoffregen/Audio/blob/master/synth_waveform.cpp)

   This Software is published under the MIT License, use at your own risk
*/

struct AudioBandlimitedOsci : public AudioStream
{
    static constexpr size_t INPUT_CONNECTION_COUNT = 12;
    enum class INPUT_INDEX
    {
        fm1 = 0,
        pwm1 = 1,
        pm1 = 2,
        am1 = 3,

        fm2 = 4,
        pwm2 = 5,
        pm2 = 6,
        am2 = 7,

        fm3 = 8,
        pwm3 = 9,
        pm3 = 10,
        am3 = 11,
    };

    AudioBandlimitedOsci() : AudioStream(INPUT_CONNECTION_COUNT, inputQueueArray)
    {
    }

    void frequency(uint8_t oscillator, float freq)
    {
        mOsc[oscillator - 1].frequency(freq, mNotesPlaying);
    }

    void waveform(uint8_t oscillator, OscWaveformShape wform)
    {
        mOsc[oscillator - 1].waveform(wform);
    }

    void pulseWidth(uint8_t oscillator, float pulseWidth)
    {
        mOsc[oscillator - 1].pulseWidth(pulseWidth);
    }

    void portamentoTime(uint8_t oscillator, float seconds)
    {
        mOsc[oscillator - 1].portamentoTime(seconds);
    }

    void fmAmount(uint8_t oscillator, float octaves)
    {
        mOsc[oscillator - 1].fmAmount(octaves);
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

    bool mEnabled = true;
    void Disable()
    {
        mEnabled = false;
    }
    void Enable()
    {
        mEnabled = true;
    }

    virtual void update(void);

    inline void osc1Step();
    inline void osc2Step();
    inline void osc2Sync(float x);
    inline void osc3Step();
    inline void osc3Sync(float x);

    audio_block_t *inputQueueArray[INPUT_CONNECTION_COUNT];

    uint8_t mNotesPlaying;

    struct OscillatorState
    {
        float mFrequency = 0; // frequency1
        float mFreq = 0;      // osc1_freq

        float mGain = 0; // osc1_gain
        OscWaveformShape mWaveformShape = OscWaveformShape::Sine;
        uint32_t mPitchModAmount = 4096; // osc1_pitchModAmount

        float mPulseWidthTarget01 = 0; // pulseWidth1
        float mPulseWidth = 0.5;       // osc1_pulseWidth

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
        float mDt = 0; // cycles per sample, very small. amount of cycle to advance each sample. // osc1_dt

        float mOutput = 0;           // osc1_output
        float mPMMultiplier = 0.01f; // scaling PM input 0-1 phase is EXTREME, so we need a reasonable maximum.
        float mAMMinimumGain = 0.0f; // when modulating amplitude, this is the minimum
        float mPMFeedbackAmt = 0.0f;

        void amplitude(float a)
        {
            mGain = Clamp(a, -1.0f, 1.0f);
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
            // pulseWidth gets limited to [0.001, 0.999] later on
            mPulseWidth = mPulseWidthTarget01 = pulseWidth;
        }

        void fmAmount(float octaves)
        {
            octaves = Clamp(octaves, 0.1f, 12.0f);
            mPitchModAmount = octaves * 4096.0f;
        }

        // call before calculating the sample; this does A-rate modulation stuff
        inline void PreStep(size_t i, audio_block_t *fm1, audio_block_t *pwm1, audio_block_t *pm1)
        {
            if (mPortamentoSamples > 0 && mCurrentPortamentoSample++ < mPortamentoSamples)
            {
                mFrequency += mPortamentoIncrement;
            }

            if (pm1)
            {
                mT += fast::Sample16To32(pm1->data[i]) * mPMMultiplier;
            }

            if (fm1)
            {
                int32_t n = fm1->data[i] * mPitchModAmount;
                int32_t ipart = n >> 27;
                n = n & 0x7FFFFFF;
                n = (n + 134217728) << 3;
                n = multiply_32x32_rshift32_rounded(n, n);
                n = multiply_32x32_rshift32_rounded(n, 715827883) << 3;
                n = n + 715827882;

                uint32_t scale = n >> (15 - ipart);
                mFreq = mFrequency * scale * 0.00003051757;
            }
            else
            {
                mFreq = mFrequency;
            }

            mDt = mFreq / AUDIO_SAMPLE_RATE_EXACT; // cycles per sample. the amount of waveform to advance each
                                                   // sample. very small.

            // pulse Width Modulation:
            if (pwm1)
            {
                mPulseWidth = mPulseWidthTarget01 + fast::Sample16To32(pwm1->data[i]);
            }
            mPulseWidth = Clamp(mPulseWidth, 0.001f, 0.999f);
        }

        inline void PostStep(size_t i, audio_block_t *out, audio_block_t *am)
        {
            float o = mOutput * mGain;
            if (am)
            {
                o *= std::abs(fast::Sample16To32(am->data[i])) + mAMMinimumGain;
            }

            out->data[i] = o * 32768.0f;
        }

        template <bool TperformSync>
        inline void Step(AudioBandlimitedOsci &owner)
        {
            float fboutput = mOutput;
            mOutput = mBlepDelay;
            mBlepDelay = 0;

            mT += mDt;

            if (TperformSync)
            {
                owner.osc2Step();
                owner.osc3Step();
            }

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

                        float x = (mT - 1) / mDt;

                        if (TperformSync)
                        {
                            if (owner.mOsc[1].mWaveformShape == OscWaveformShape::SawSync)
                            {
                                owner.osc2Sync(x);
                            }
                            if (owner.mOsc[2].mWaveformShape == OscWaveformShape::SawSync)
                            {
                                owner.osc3Sync(x);
                            }
                        }

                        float scale = mDt / (mPulseWidth - mPulseWidth * mPulseWidth);

                        mOutput += scale * blamp0(x);
                        mBlepDelay += scale * blamp1(x);

                        mPulseStage = false;
                        mT -= 1;
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

                        float x = (mT - 1) / mDt;

                        if (TperformSync)
                        {
                            if (owner.mOsc[1].mWaveformShape == OscWaveformShape::SawSync)
                            {
                                owner.osc2Sync(x);
                            }
                            if (owner.mOsc[2].mWaveformShape == OscWaveformShape::SawSync)
                            {
                                owner.osc3Sync(x);
                            }
                        }

                        mOutput += blep0(x);
                        mBlepDelay += blep1(x);

                        mPulseStage = false;
                        mT -= 1;
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

        inline void Sync(float x, OscillatorState &masterOsc)
        {
            mOutput = mBlepDelay;
            mBlepDelay = 0;

            float scale = mDt / masterOsc.mDt;
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

            if (mT < dt) // if (osc2_t < dt && scale < 1)
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
    };

    OscillatorState mOsc[3];
}; // namespace clarinoid

void AudioBandlimitedOsci::update()
{
    if (!mEnabled)
        return;
    audio_block_t *out1 = allocate();
    if (!out1)
        return;
    audio_block_t *out2 = allocate();
    if (!out2)
        return;
    audio_block_t *out3 = allocate();
    if (!out3)
        return;

    audio_block_t *fm1 = receiveReadOnly((int)INPUT_INDEX::fm1);
    audio_block_t *pwm1 = receiveReadOnly((int)INPUT_INDEX::pwm1);
    audio_block_t *pm1 = receiveReadOnly((int)INPUT_INDEX::pm1);
    audio_block_t *am1 = receiveReadOnly((int)INPUT_INDEX::am1);
    audio_block_t *fm2 = receiveReadOnly((int)INPUT_INDEX::fm2);
    audio_block_t *pwm2 = receiveReadOnly((int)INPUT_INDEX::pwm2);
    audio_block_t *pm2 = receiveReadOnly((int)INPUT_INDEX::pm2);
    audio_block_t *am2 = receiveReadOnly((int)INPUT_INDEX::am2);
    audio_block_t *fm3 = receiveReadOnly((int)INPUT_INDEX::fm3);
    audio_block_t *pwm3 = receiveReadOnly((int)INPUT_INDEX::pwm3);
    audio_block_t *pm3 = receiveReadOnly((int)INPUT_INDEX::pm3);
    audio_block_t *am3 = receiveReadOnly((int)INPUT_INDEX::am3);

    for (uint16_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
    {
        mOsc[0].PreStep(i, fm1, pwm1, pm1);
        mOsc[1].PreStep(i, fm2, pwm2, pm2);
        mOsc[2].PreStep(i, fm3, pwm3, pm3);

        osc1Step(); // This steps actually all oscillators.

        mOsc[0].PostStep(i, out1, am1);
        mOsc[1].PostStep(i, out2, am2);
        mOsc[2].PostStep(i, out3, am3);

        // out1->data[i] = (int16_t)(mOsc[0].mOutput * 32768.0 * mOsc[0].mGain);
        // out2->data[i] = (int16_t)(mOsc[1].mOutput * 32768.0 * mOsc[1].mGain);
        // out3->data[i] = (int16_t)(mOsc[2].mOutput * 32768.0 * mOsc[2].mGain);
    }

    transmit(out1, 0);
    release(out1);

    transmit(out2, 1);
    release(out2);

    transmit(out3, 2);
    release(out3);

    if (fm1)
        release(fm1);
    if (pwm1)
        release(pwm1);
    if (pm1)
        release(pm1);
    if (am1)
        release(am1);
    if (fm2)
        release(fm2);
    if (pwm2)
        release(pwm2);
    if (pm2)
        release(pm2);
    if (am2)
        release(am2);

    if (fm3)
        release(fm3);
    if (pwm3)
        release(pwm3);
    if (pm3)
        release(pm3);
    if (am3)
        release(am3);
}

inline void AudioBandlimitedOsci::osc3Step()
{
    mOsc[2].Step<false>(*this);
}

inline void AudioBandlimitedOsci::osc3Sync(float x)
{
    mOsc[2].Sync(x, mOsc[0]);
}

inline void AudioBandlimitedOsci::osc2Step()
{
    mOsc[1].Step<false>(*this);
}

inline void AudioBandlimitedOsci::osc2Sync(float x)
{
    mOsc[1].Sync(x, mOsc[0]);
}

inline void AudioBandlimitedOsci::osc1Step()
{
    mOsc[0].Step<true>(*this);
}

} // namespace clarinoid
