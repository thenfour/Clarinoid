
#pragma once

#ifdef CLARINOID_MODULE_TEST
#error not for x86 tests
#endif

#include <cfloat>

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/loopstation/LoopstationMemory.hpp>
#include <clarinoid/synth/EnvelopeNode.hpp>

#include "FilterNode.hpp"
#include "PannerNode.hpp"
#include "MultiMixer.hpp"
#include "ModulationMatrixNode.hpp"
#include "Patch.hpp"
#include "polyBlepOscillator.hpp"

namespace clarinoid
{
static constexpr float KRateFrequencyModulationMultiplier = 12.0f;

struct SynthVoiceState
{
    MusicalEventSource mSource;
    HeldNoteInfo mNoteInfo;
    SynthPreset *mSynthPatch = nullptr;
    int mSynthPatchIndex = -1;
    uint32_t mReleaseTimestampMS = 0; // 0 means the note is still active.
    // float mExtraGain;                 // for performance, harmonizer voice, etc, additional gain can be applied.
    // float mExtraPan;                  // same.
};

struct SynthGraph
{
    /*
    https://www.pjrc.com/teensy/gui/index.html
    // this is the after-oscillator processing.

    */
    // GUItool: begin automatically generated code
    AudioAmplifier delayFeedbackAmpLeft;  // xy=535,170
    AudioEffectDelay delayRight;          // xy=536,478
    AudioEffectDelay delayLeft;           // xy=537,261
    AudioAmplifier delayFeedbackAmpRight; // xy=537,390
    AudioAmplifier delayWetAmpLeft;       // xy=781,234
    AudioAmplifier delayWetAmpRight;      // xy=802,469
    AudioMixer4 verbInputMixer;           // xy=920,561
    AudioSynthWaveformSine metronomeOsc;  // xy=956,990
    AudioEffectFreeverbStereo verb;       // xy=1070,567
    AudioEffectEnvelope metronomeEnv;     // xy=1157,992
    AudioAmplifier verbWetAmpLeft;        // xy=1229,548
    AudioAmplifier verbWetAmpRight;       // xy=1236,586
    AudioMixer4 postMixerLeft;            // xy=1411,775
    AudioMixer4 postMixerRight;           // xy=1413,858
    AudioAmplifier ampLeft;               // xy=1573,776
    AudioAmplifier ampRight;              // xy=1576,857
    AudioOutputI2S i2s1;                  // xy=1758,809
    AudioAnalyzePeak peakL;               // xy=1801,588
    AudioAnalyzePeak peakR;               // xy=1851,598
    AudioConnection patchCord1 = {delayWetAmpLeft, 0, postMixerLeft, 1};
    AudioConnection patchCord2 = {delayWetAmpRight, 0, postMixerRight, 1};
    AudioConnection patchCord3 = {verbInputMixer, verb};
    AudioConnection patchCord4 = {metronomeOsc, metronomeEnv};
    AudioConnection patchCord5 = {verb, 0, verbWetAmpLeft, 0};
    AudioConnection patchCord6 = {verb, 1, verbWetAmpRight, 0};
    AudioConnection patchCord7 = {metronomeEnv, 0, postMixerRight, 3};
    AudioConnection patchCord8 = {metronomeEnv, 0, postMixerLeft, 3};
    AudioConnection patchCord9 = {verbWetAmpLeft, 0, postMixerLeft, 2};
    AudioConnection patchCord10 = {verbWetAmpRight, 0, postMixerRight, 2};
    AudioConnection patchCord11 = {postMixerLeft, ampLeft};
    AudioConnection patchCord12 = {postMixerRight, ampRight};
    AudioConnection patchCord13 = {ampLeft, peakL};
    AudioConnection patchCord14 = {ampRight, peakR};
    AudioConnection patchCord15 = {ampLeft, 0, i2s1, 1};
    AudioConnection patchCord16 = {ampRight, 0, i2s1, 0};
    // GUItool: end automatically generated code

    // insert the delay filters.
    // ...[delayLeft]---->[delayFilterLeft]--------------->[delayFeedbackAmpLeft]...
    //                                     \-------------->[delayWetAmpLeft]...
    ::clarinoid::FilterNode delayFilterLeft;
    AudioConnection mPatchDelayToFilterLeft = {delayLeft, 0, delayFilterLeft, 0};
    AudioConnection mPatchDelayFilterToFeedbackAmpLeft = {delayFilterLeft, 0, delayFeedbackAmpLeft, 0};

    ::clarinoid::FilterNode delayFilterRight;
    AudioConnection mPatchDelayToFilterRight = {delayRight, 0, delayFilterRight, 0};
    AudioConnection mPatchDelayFilterToFeedbackAmpRight = {delayFilterRight, 0, delayFeedbackAmpRight, 0};

    // delay output connection
    AudioConnection mPatchDelayFilterToAmpLeft = {delayFilterLeft, 0, delayWetAmpLeft, 0};
    AudioConnection mPatchDelayFilterToAmpRight = {delayFilterRight, 0, delayWetAmpRight, 0};

    // voice mixer & dry output connection
    ::clarinoid::MultiMixerNode<MAX_SYNTH_VOICES> voiceMixerDryLeft;  // all voices input here.
    ::clarinoid::MultiMixerNode<MAX_SYNTH_VOICES> voiceMixerDryRight; // all voices input here.
    AudioConnection patchVoicesDryToOutpLeft = {voiceMixerDryLeft, 0, postMixerLeft, 0};
    AudioConnection patchVoicesDryToOutpRight = {voiceMixerDryRight, 0, postMixerRight, 0};

    // delay input mix
    ::clarinoid::MultiMixerNode<MAX_SYNTH_VOICES + 1>
        delayInputMixerLeft; // +1 to account for the delay feedback signal
    ::clarinoid::MultiMixerNode<MAX_SYNTH_VOICES + 1>
        delayInputMixerRight; // +1 to account for the delay feedback signal
    AudioConnection patchDelayInputMixToDelayL = {delayInputMixerLeft, 0, delayLeft, 0};
    AudioConnection patchDelayInputMixToDelayR = {delayInputMixerRight, 0, delayRight, 0};

    // delay fb
    AudioConnection patchDelayFBBackToInputLeft = {delayFeedbackAmpLeft, 0, delayInputMixerLeft, MAX_SYNTH_VOICES};
    AudioConnection patchDelayFBBackToInputRight = {delayFeedbackAmpRight, 0, delayInputMixerRight, MAX_SYNTH_VOICES};

    // verb input mix
    ::clarinoid::MultiMixerNode<MAX_SYNTH_VOICES + 1> verbInputMixerLeft;  // all voices input here + 1 for delay line
    ::clarinoid::MultiMixerNode<MAX_SYNTH_VOICES + 1> verbInputMixerRight; // all voices input here + 1 for delay line
    AudioConnection patchVerbInputToVerbL = {verbInputMixerLeft, 0, verbInputMixer, 0};
    AudioConnection patchVerbInputToVerbR = {verbInputMixerRight, 0, verbInputMixer, 1};

    // delay verb
    AudioConnection patchDelayVerbInputLeft = {delayFilterLeft, 0, verbInputMixerLeft, MAX_SYNTH_VOICES};
    AudioConnection patchDelayVerbInputRight = {delayFilterRight, 0, verbInputMixerRight, MAX_SYNTH_VOICES};

}; // namespace CCSynthGraph

// dynamic allocate to ensure it goes into RAM2
SynthGraph *gpSynthGraph = nullptr;

StaticInit __synthGraphInit([]() { gpSynthGraph = new SynthGraph(); });

struct Voice : IModulationKRateProvider
{
    //
    // [mOsc] osc1 --> 0
    // [mOsc] osc2 --> 1 [mOscMixerPanner] --> L [mFilter] L -> L [mSplitter] ... out
    // [mOsc] osc3 --> 2                   --> R           R -> R
    //
    AudioBandlimitedOsci mOsc;

    // Modulation sources
    VoiceModulationMatrixNode mModMatrix;
    EnvelopeNode mEnv1;
    EnvelopeNode mEnv2;
    AudioSynthWaveformDc mEnv1PeakDC;
    AudioSynthWaveformDc mEnv2PeakDC;
    CCPatch mPatchDCToEnv1 = {mEnv1PeakDC, 0, mEnv1, 0};
    CCPatch mPatchDCToEnv2 = {mEnv2PeakDC, 0, mEnv2, 0};
    AudioSynthWaveform mLfo1;
    AudioSynthWaveform mLfo2;

    // have to track these because they're private members of AudioWaveform.
    short mLfo1Waveshape = 0xff; // invalid so 1st run will always set the shape.
    short mLfo2Waveshape = 0xff; // invalid so 1st run will always set the shape.

    // patch a-rate modulation sources in
    CCPatch mPatchEnv1ToMod = {mEnv1, 0, mModMatrix, (uint8_t)ARateModulationSource::ENV1};
    CCPatch mPatchEnv2ToMod = {mEnv2, 0, mModMatrix, (uint8_t)ARateModulationSource::ENV2};
    CCPatch mPatchLfo1ToMod = {mLfo1, 0, mModMatrix, (uint8_t)ARateModulationSource::LFO1};
    CCPatch mPatchLfo2ToMod = {mLfo2, 0, mModMatrix, (uint8_t)ARateModulationSource::LFO2};

    CCPatch mOsc1FBToMod = {mOsc, 0, mModMatrix, (uint8_t)ARateModulationSource::Osc1FB};
    CCPatch mOsc2FBToMod = {mOsc, 1, mModMatrix, (uint8_t)ARateModulationSource::Osc2FB};
    CCPatch mOsc3FBToMod = {mOsc, 2, mModMatrix, (uint8_t)ARateModulationSource::Osc3FB};

    // Patch a-rate modulation destinations
    CCPatch mPatchModToOsc1PWM = {mModMatrix,
                                  (uint8_t)ARateModulationDestination::Osc1PulseWidth,
                                  mOsc,
                                  (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::pwm1};
    CCPatch mPatchModToOsc2PWM = {mModMatrix,
                                  (uint8_t)ARateModulationDestination::Osc2PulseWidth,
                                  mOsc,
                                  (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::pwm2};
    CCPatch mPatchModToOsc3PWM = {mModMatrix,
                                  (uint8_t)ARateModulationDestination::Osc3PulseWidth,
                                  mOsc,
                                  (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::pwm3};

    CCPatch mPatchModToOsc1Phase = {mModMatrix,
                                    (uint8_t)ARateModulationDestination::Osc1Phase,
                                    mOsc,
                                    (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::pm1};
    CCPatch mPatchModToOsc2Phase = {mModMatrix,
                                    (uint8_t)ARateModulationDestination::Osc2Phase,
                                    mOsc,
                                    (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::pm2};
    CCPatch mPatchModToOsc3Phase = {mModMatrix,
                                    (uint8_t)ARateModulationDestination::Osc3Phase,
                                    mOsc,
                                    (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::pm3};

    // ...
    MultiMixerPannerNode<3> mOscMixerPanner;

    CCPatch mPatchOsc1ToMixer = {mOsc, 0, mOscMixerPanner, 0};
    CCPatch mPatchOsc2ToMixer = {mOsc, 1, mOscMixerPanner, 1};
    CCPatch mPatchOsc3ToMixer = {mOsc, 2, mOscMixerPanner, 2};

    ::clarinoid::StereoFilterNode mFilter;
    CCPatch mPatchMixToFilterL = {mOscMixerPanner, 0, mFilter, 0};
    CCPatch mPatchMixToFilterR = {mOscMixerPanner, 1, mFilter, 1};

    ::clarinoid::StereoGainerSplitterNode<3> mSplitter; // handles panning and send levels to delay/verb/dry
    CCPatch mPatchFilterToPannerSplitterL = {mFilter, 0, mSplitter, 0};
    CCPatch mPatchFilterToPannerSplitterR = {mFilter, 1, mSplitter, 1};

    CCPatch mPatchOutDryLeft;
    CCPatch mPatchOutDryRight;

    CCPatch mPatchOutDelayLeft;
    CCPatch mPatchOutDelayRight;

    CCPatch mPatchOutVerbLeft;
    CCPatch mPatchOutVerbRight;

    int mVoiceIndex;
    SynthVoiceState mRunningVoice;
    AppSettings *mAppSettings;
    ISynthParamProvider *mParamProvider;
    bool mTouched = false;

    void EnsurePatchConnections(AppSettings *appSettings, ISynthParamProvider *paramProvider)
    {
        mAppSettings = appSettings;
        mParamProvider = paramProvider;

        mEnv1PeakDC.amplitude(1.0f);
        mEnv2PeakDC.amplitude(1.0f);

        mPatchOsc1ToMixer.connect();
        mPatchOsc2ToMixer.connect();
        mPatchOsc3ToMixer.connect();
        mPatchMixToFilterL.connect();
        mPatchMixToFilterR.connect();
        mPatchFilterToPannerSplitterL.connect();
        mPatchFilterToPannerSplitterR.connect();
        mPatchOutDryLeft.connect();
        mPatchOutDryRight.connect();
        mPatchOutDelayLeft.connect();
        mPatchOutDelayRight.connect();
        mPatchOutVerbLeft.connect();
        mPatchOutVerbRight.connect();

        mOsc1FBToMod.connect();
        mOsc2FBToMod.connect();
        mOsc3FBToMod.connect();

        mPatchEnv1ToMod.connect();
        mPatchEnv2ToMod.connect();
        mPatchLfo1ToMod.connect();
        mPatchLfo2ToMod.connect();

        mPatchModToOsc1PWM.connect();
        mPatchModToOsc2PWM.connect();
        mPatchModToOsc3PWM.connect();

        mPatchModToOsc1Phase.connect();
        mPatchModToOsc2Phase.connect();
        mPatchModToOsc3Phase.connect();

        mPatchDCToEnv1.connect();
        mPatchDCToEnv2.connect();
    }

    float CalcParamFrequency(float freqParam, float noteHz, float ktAmountN11, float krateModulationN11)
    {
        freqParam += krateModulationN11; // apply current modulation value.
        // at 0.5, we use 1khz.
        // for each 0.1 param value, it's +/- one octave

        float centerFreq = 1000; // the cutoff frequency at 0.5 param value.

        // with no KT,
        // so if param is 0.8, we want to multiply by 8 (2^3)
        // if param is 0.3, multiply by 1/4 (2^(1/4))

        // with full KT,
        // at 0.3, we use playFrequency.
        // for each 0.1 param value, it's +/- one octave.
        float ktFreq = noteHz * 4; // to copy massive, 1:1 is at paramvalue 0.3. 0.5 is 2 octaves above playing freq.
        centerFreq = Lerp(centerFreq, ktFreq, ktAmountN11);

        freqParam -= 0.5f; // signed distance from 0.5 -.2 (0.3 = -.2, 0.8 = .3)
        freqParam *= 10.0f; // (.3 = -2, .8 = 3)
        float fact = fast::pow(2, freqParam);
        return Clamp(centerFreq * fact, 0.0f, 22050.0f);
    }

    float CalcFilterCutoffFreq(float noteHz)
    {
        return CalcParamFrequency(mRunningVoice.mSynthPatch->mFilterFreq,
                                  noteHz,
                                  mRunningVoice.mSynthPatch->mFilterKeytracking,
                                  mKRateVoiceFilterCutoffN11);
        // // perform breath & key tracking for filter. we will basically multiply the
        // // effects. velocity we will only track between notes from 7jam code: const
        // // halfKeyScaleRangeSemis = 12 * 4; from 7jam code: let ks = 1.0 -
        // // DF.remap(this.midiNote, 60.0 /* middle C */ -
        // // halfKeyScaleRangeSemis, 60.0 + halfKeyScaleRangeSemis, ksAmt, -ksAmt); //
        // // when vsAmt is 0, the range of vsAmt,-vsAmt is 0. hence making this 1.0-x
        // float filterKS = map(
        //     mRunningVoice.mNoteInfo.mMidiNote.GetMidiValue(), 20, 120, 0.0f, 1.0f); // map midi note to full ks
        //     effect
        // filterKS = map(mRunningVoice.mSynthPatch->mFilterKeytracking,
        //                0,
        //                1.0f,
        //                1.0,
        //                filterKS); // map ks amt 0-1 to 1-fulleffect

        // if (USE_BREATH_FILTER)
        // {
        //     float filterP = filterKS * mParamProvider->SynthParamProvider_GetBreath01();
        //     filterP = ClampInclusive(filterP + mKRateVoiceFilterCutoffN11, 0.0f, 1.0f);

        //     float filterFreq = map(filterP,
        //                            0.0f,
        //                            1.0f,
        //                            mRunningVoice.mSynthPatch->mFilterMinFreq,
        //                            mRunningVoice.mSynthPatch->mFilterMaxFreq);
        //     return filterFreq;
        // }

        // float freq = ClampInclusive(mKRateVoiceFilterCutoffN11, 0.0f, 1.0f) *
        // mRunningVoice.mSynthPatch->mFilterMaxFreq; return freq;
    }

    // float mLatestBreathVal = 0.0f;
    // float mLatestPitchbendVal = 0.0f;

    virtual float IModulationProvider_GetKRateModulationSourceValueN11(KRateModulationSource src) override
    {
        switch (src)
        {
        case KRateModulationSource::Breath:
            return mParamProvider->SynthParamProvider_GetBreath01();

        default:
        case KRateModulationSource::PitchStrip:
            break;
        }
        return mParamProvider->SynthParamProvider_GetPitchBendN11();
    }

    virtual float IModulationProvider_GetKRateModulationDestinationValueN11(KRateModulationDestination dest) override
    {
        switch (dest)
        {
        case KRateModulationDestination::VoiceFilterCutoff:
            // Serial.println(String("Getting krate filter cutoff, val=") + mKRateVoiceFilterCutoffN11);
            return mKRateVoiceFilterCutoffN11;
        case KRateModulationDestination::Osc1FMFeedback:
            return mKRateOscFMFeedback[0];
        case KRateModulationDestination::Osc2FMFeedback:
            return mKRateOscFMFeedback[1];
        case KRateModulationDestination::Osc3FMFeedback:
            return mKRateOscFMFeedback[2];
        case KRateModulationDestination::OverallFMStrength:
            return mKRateOverallFMStrength;
        case KRateModulationDestination::FMStrength2To1:
            return mKRateFMStrength2To1;
        case KRateModulationDestination::FMStrength3To1:
            return mKRateFMStrength3To1;
        case KRateModulationDestination::FMStrength1To2:
            return mKRateFMStrength1To2;
        case KRateModulationDestination::FMStrength3To2:
            return mKRateFMStrength3To2;
        case KRateModulationDestination::FMStrength1To3:
            return mKRateFMStrength1To3;
        case KRateModulationDestination::FMStrength2To3:
            return mKRateFMStrength2To3;
        default:
            return 0;
        }
    }

    // store k-rate running values here
    float mKRateVoiceFilterCutoffN11 = 0;
    float mKRateFrequencyN11[3] = {0};
    float mKRateAmplitudeN11[3] = {0};
    float mKRateOscFMFeedback[3] = {0};
    float mKRateOscFreqMul[3] = {0};
    float mKRateOscFreqOffset[3] = {0};
    float mKRateOverallFMStrength = 0;
    float mKRateFMStrength2To1 = 0;
    float mKRateFMStrength3To1 = 0;
    float mKRateFMStrength1To2 = 0;
    float mKRateFMStrength3To2 = 0;
    float mKRateFMStrength1To3 = 0;
    float mKRateFMStrength2To3 = 0;

    // these will not track FREQUENCY, but rather MIDI NOTE. this lets us apply portamento to certain parameters but not
    // others. it also may have the benefit of feeling more linear.
    PortamentoCalc mPortamentoCalc[3];

    virtual void IModulationProvider_SetKRateModulationDestinationValueN11(KRateModulationDestination d,
                                                                           float val) override
    {
        // Serial.println(String("Setting krate. d=") + (int)d + ", val=" + val);
        switch (d)
        {
        case KRateModulationDestination::VoiceFilterCutoff:
            mKRateVoiceFilterCutoffN11 = val;
            return;
        case KRateModulationDestination::Osc1Frequency:
            mKRateFrequencyN11[0] = val;
            return;
        case KRateModulationDestination::Osc1Amplitude:
            mKRateAmplitudeN11[0] = val;
            return;
        case KRateModulationDestination::Osc2Frequency:
            mKRateFrequencyN11[1] = val;
            return;
        case KRateModulationDestination::Osc2Amplitude:
            mKRateAmplitudeN11[1] = val;
            return;
        case KRateModulationDestination::Osc3Frequency:
            mKRateFrequencyN11[2] = val;
            return;
        case KRateModulationDestination::Osc3Amplitude:
            mKRateAmplitudeN11[2] = val;
            return;
        case KRateModulationDestination::Osc1FMFeedback:
            mKRateOscFMFeedback[0] = val;
            return;
        case KRateModulationDestination::Osc2FMFeedback:
            mKRateOscFMFeedback[1] = val;
            return;
        case KRateModulationDestination::Osc3FMFeedback:
            mKRateOscFMFeedback[2] = val;
            return;
        case KRateModulationDestination::OverallFMStrength:
            mKRateOverallFMStrength = val;
            return;
        case KRateModulationDestination::FMStrength2To1:
            mKRateFMStrength2To1 = val;
            return;
        case KRateModulationDestination::FMStrength3To1:
            mKRateFMStrength3To1 = val;
            return;
        case KRateModulationDestination::FMStrength1To2:
            mKRateFMStrength1To2 = val;
            return;
        case KRateModulationDestination::FMStrength3To2:
            mKRateFMStrength3To2 = val;
            return;
        case KRateModulationDestination::FMStrength1To3:
            mKRateFMStrength1To3 = val;
            return;
        case KRateModulationDestination::FMStrength2To3:
            mKRateFMStrength2To3 = val;
            return;

        case KRateModulationDestination::Osc1FreqMul:
            mKRateOscFreqMul[0] = val;
            return;

        case KRateModulationDestination::Osc2FreqMul:
            mKRateOscFreqMul[1] = val;
            return;

        case KRateModulationDestination::Osc3FreqMul:
            mKRateOscFreqMul[2] = val;
            return;

        case KRateModulationDestination::Osc1FreqOffset:
            mKRateOscFreqOffset[0] = val;
            return;

        case KRateModulationDestination::Osc2FreqOffset:
            mKRateOscFreqOffset[1] = val;
            return;

        case KRateModulationDestination::Osc3FreqOffset:
            mKRateOscFreqOffset[2] = val;
            return;
        }
    }

    void ResetKRateModulations()
    {
        // reset saved krate mod values, so modulations don't leak across patch changes
        mKRateVoiceFilterCutoffN11 = 0;
        for (auto &v : mKRateFrequencyN11)
        {
            v = 0;
        }

        for (auto &v : mKRateAmplitudeN11)
        {
            v = 1;
        }

        for (auto &v : mKRateOscFMFeedback)
        {
            v = 0;
        }

        mKRateOverallFMStrength = 0;
        mKRateFMStrength2To1 = 0;
        mKRateFMStrength3To1 = 0;
        mKRateFMStrength1To2 = 0;
        mKRateFMStrength3To2 = 0;
        mKRateFMStrength1To3 = 0;
        mKRateFMStrength2To3 = 0;

        mModMatrix.SetSynthPatch(mRunningVoice.mSynthPatch, this);
    }

    short convertWaveType(OscWaveformShape s)
    {
        switch (s)
        {
        case OscWaveformShape::Pulse:
            return WAVEFORM_PULSE;
        case OscWaveformShape::SawSync:
            return WAVEFORM_SAWTOOTH;
        case OscWaveformShape::VarTriangle:
            return WAVEFORM_TRIANGLE;
        case OscWaveformShape::Sine:
            break;
        }
        return WAVEFORM_SINE;
    };

    void ApplyPatchToGraph()
    {
        const auto &patch = *mRunningVoice.mSynthPatch;
        const auto &perf = mAppSettings->GetCurrentPerformancePatch();
        // configure envelopes (DADSR x 3)
        mEnv1.delay(patch.mEnv1.mDelayMS);
        mEnv1.attack(patch.mEnv1.mAttackMS);
        mEnv1.hold(patch.mEnv1.mHoldMS);
        mEnv1.decay(patch.mEnv1.mDecayMS);
        mEnv1.sustain(patch.mEnv1.mSustainLevel);
        mEnv1.release(patch.mEnv1.mReleaseMS);
        // mEnv1.releaseNoteOn(patch.mEnv1.mReleaseNoteOnMS);

        mEnv2.delay(patch.mEnv2.mDelayMS);
        mEnv2.attack(patch.mEnv2.mAttackMS);
        mEnv2.hold(patch.mEnv2.mHoldMS);
        mEnv2.decay(patch.mEnv2.mDecayMS);
        mEnv2.sustain(patch.mEnv2.mSustainLevel);
        mEnv2.release(patch.mEnv2.mReleaseMS);
        // mEnv2.releaseNoteOn(patch.mEnv2.mReleaseNoteOnMS);

        short wantsWaveType1 = convertWaveType(patch.mLFO1.mWaveShape);
        short wantsWaveType2 = convertWaveType(patch.mLFO2.mWaveShape);
        if (mLfo1Waveshape != wantsWaveType1)
        {
            mLfo1.begin(wantsWaveType1);
            mLfo1.amplitude(1.0f);
        }
        if (mLfo2Waveshape != wantsWaveType2)
        {
            mLfo2.begin(wantsWaveType2);
            mLfo2.amplitude(1.0f);
        }

        mLfo1.frequency(patch.mLFO1.mTime.ToHertz(perf.mBPM));
        mLfo2.frequency(patch.mLFO2.mTime.ToHertz(perf.mBPM));

        mOsc.mIsPlaying = true;
        for (size_t i = 0; i < POLYBLEP_OSC_COUNT; ++i)
        {
            mOsc.mOsc[i].waveform(patch.mOsc[i].mWaveform);
            mOsc.mOsc[i].pulseWidth(patch.mOsc[i].mPulseWidth);
            mOsc.mOsc[i].mPMMultiplier = patch.mOverallFMStrength + mKRateOverallFMStrength;
            mOsc.mOsc[i].mPMFeedbackAmt = patch.mOsc[i].mFMFeedbackGain + mKRateOscFMFeedback[i];
        }
    }

    float CalcFreq(const SynthOscillatorSettings &osc,
                   float detune,
                   float krateFreqModN11,
                   float krateFreqMul,
                   float krateFreqOff,
                   PortamentoCalc &portamento)
    {
        float userPB = mParamProvider->SynthParamProvider_GetPitchBendN11(); // mv.mPitchBendN11.GetFloatVal();
        float pbSemis = 0;
        if (userPB > 0)
        {
            pbSemis = userPB * osc.mPitchBendRangePositive;
        }
        else
        {
            pbSemis = userPB * (-osc.mPitchBendRangeNegative);
        }
        float ret = portamento.KStep(mRunningVoice.mNoteInfo.mMidiNote.GetMidiValue(), osc.mPortamentoTimeMS) +
                    osc.mPitchFine + osc.mPitchSemis + detune + pbSemis +
                    (KRateFrequencyModulationMultiplier * krateFreqModN11);
        ret = (MIDINoteToFreq(ret) * (osc.mFreqMultiplier + krateFreqMul)) + osc.mFreqOffset + krateFreqOff;
        return Clamp(ret, 0.0f, 22050.0f);
    };

    void Update()
    {
        // basically the only time this will become -1 is at startup
        if (mRunningVoice.mSynthPatch == nullptr)
        {
            // if (IsConsideredPlaying_ForDisplay())
            // {
            //     Serial.println(String("synth patch is null; no sound focr you"));
            // }

            mOsc.mIsPlaying = false;
            return;
        }

        mOsc.mIsPlaying = true;

        const auto &patch = *mRunningVoice.mSynthPatch;
        // const auto &perf = mAppSettings->GetCurrentPerformancePatch();

        // apply ongoing params
        ApplyPatchToGraph();

        // apply frequency params
        float freq0 = CalcFreq(patch.mOsc[0],
                               -patch.mDetune,
                               mKRateFrequencyN11[0],
                               mKRateOscFreqMul[0],
                               mKRateOscFreqOffset[0],
                               mPortamentoCalc[0]);
        float freq1 = CalcFreq(
            patch.mOsc[1], 0, mKRateFrequencyN11[1], mKRateOscFreqMul[1], mKRateOscFreqOffset[1], mPortamentoCalc[1]);
        float freq2 = CalcFreq(patch.mOsc[2],
                               -patch.mDetune,
                               mKRateFrequencyN11[2],
                               mKRateOscFreqMul[2],
                               mKRateOscFreqOffset[2],
                               mPortamentoCalc[2]);

        // if (patch.mSync)
        // {
        //     // sync only supported in osc1 for the moment.
        //     float freqSync = map(
        //         mv.mBreath01.GetFloatVal(), 0.0f, 1.0f, freq1 * mPreset->mSyncMultMin, freq1 *
        //         mPreset->mSyncMultMax);

        //     mOsc.mOsc[0].SetBasicParams(freq0, mPreset->mOsc[0].mPhase01, mPreset->mOsc[0].mPortamentoTimeMS, false,
        //     0); mOsc.mOsc[1].SetBasicParams(
        //         freq0, mPreset->mOsc[1].mPhase01, mPreset->mOsc[1].mPortamentoTimeMS, true, freqSync);
        //     mOsc.mOsc[2].SetBasicParams(freq2, mPreset->mOsc[2].mPhase01, mPreset->mOsc[2].mPortamentoTimeMS, false,
        //     0);
        // }
        // else
        // {
        mOsc.mOsc[0].SetBasicParams(freq0, patch.mOsc[0].mPhase01, patch.mOsc[0].mPortamentoTimeMS, false, 0);
        mOsc.mOsc[1].SetBasicParams(freq1, patch.mOsc[1].mPhase01, patch.mOsc[1].mPortamentoTimeMS, false, 0);
        mOsc.mOsc[2].SetBasicParams(freq2, patch.mOsc[2].mPhase01, patch.mOsc[2].mPortamentoTimeMS, false, 0);
        // }

        // perform breath & key tracking for filter. we will basically multiply the
        // effects.
        // TODO: portamento vs. frequency.
        float filterFreq = CalcFilterCutoffFreq(MIDINoteToFreq(mRunningVoice.mNoteInfo.mMidiNote.GetMidiValue()));
        // filterFreq = 3000;
        mFilter.SetParams(patch.mFilterType, filterFreq, patch.mFilterQ, patch.mFilterSaturation);
        mFilter.EnableDCFilter(patch.mDCFilterEnabled, patch.mDCFilterCutoff);

        // TODO: consider patch, perf, harm, loop gain & panning
        float mExtraPan = 0;
        float mExtraGain = 1.0f;

        mSplitter.SetOutputGain(0, mExtraGain);
        mSplitter.SetOutputGain(1, mExtraGain * patch.mDelaySend);
        mSplitter.SetOutputGain(2, mExtraGain * patch.mVerbSend);

        // if (IsConsideredPlaying_ForDisplay())
        //  {
        //      //float g = patch.mOsc[0].mGain * mKRateAmplitudeN11[0];
        //      Serial.println(String("[") + mVoiceIndex + "] frame @ freq=" + freq0 + ", filterfreq=" + filterFreq + ",
        //      krateamp=" + mKRateAmplitudeN11[0] +
        //                     ", kratefilt=" + mKRateVoiceFilterCutoffN11 + ", env1.isactive=" + (mEnv1.isActive() ?
        //                     "yes" : "no") + ", env1.issustain=" + (mEnv1.isSustain() ? "yes" : "no"));
        //  }

        mOscMixerPanner.SetInputPanGainAndEnabled(0,
                                                  mExtraPan + patch.mPan + patch.mOsc[0].mPan + patch.mStereoSpread,
                                                  patch.mOsc[0].mGain * mKRateAmplitudeN11[0],
                                                  true);

        mOscMixerPanner.SetInputPanGainAndEnabled(
            1, mExtraPan + patch.mPan + patch.mOsc[1].mPan, patch.mOsc[1].mGain * mKRateAmplitudeN11[1], true);

        mOscMixerPanner.SetInputPanGainAndEnabled(2,
                                                  mExtraPan + patch.mPan + patch.mOsc[2].mPan - patch.mStereoSpread,
                                                  patch.mOsc[2].mGain * mKRateAmplitudeN11[2],
                                                  true);
    }

    // if there's already a note playing, it gets cut off
    void IncomingMusicalEvents_OnNoteOn(const MusicalEventSource &source,
                                        const HeldNoteInfo &noteInfo,
                                        uint16_t synthPatchIndex)
    {
        IncomingMusicalEvents_OnNoteOff();

        // adjust running voice.
        auto newSynthPatch = &mAppSettings->FindSynthPreset(synthPatchIndex);
        mRunningVoice.mSource = source;
        mRunningVoice.mNoteInfo = noteInfo;
        mRunningVoice.mReleaseTimestampMS = 0; // important so we know the note is playing

        if (mRunningVoice.mSynthPatch != newSynthPatch)
        {
            mRunningVoice.mSynthPatchIndex = synthPatchIndex;
            mRunningVoice.mSynthPatch = newSynthPatch;
            ResetKRateModulations();
        }

        // if (this->mVoiceIndex == 0) {
        // Serial.println(String("[") + mVoiceIndex + "] voice env.NoteOn. " + noteInfo.mMidiNote.GetNoteDesc().mName);
        //}

        for (size_t i = 0; i < POLYBLEP_OSC_COUNT; ++i)
        {
            if (mRunningVoice.mSynthPatch->mOsc[i].mPhaseRestart)
            {
                mOsc.mOsc[i].ResetPhase();
            }
        }

        mEnv1.noteOn();
        mEnv2.noteOn();
        // Serial.println("voice note on");

        if (mRunningVoice.mSynthPatch->mLFO1.mPhaseRestart)
        {
            mLfo1.begin(convertWaveType(mRunningVoice.mSynthPatch->mLFO1.mWaveShape));
            mLfo1.amplitude(1.0f);
        }

        if (mRunningVoice.mSynthPatch->mLFO2.mPhaseRestart)
        {
            mLfo2.begin(convertWaveType(mRunningVoice.mSynthPatch->mLFO2.mWaveShape));
            mLfo2.amplitude(1.0f);
        }
    }

    void IncomingMusicalEvents_OnNoteOff()
    {
        mRunningVoice.mReleaseTimestampMS = millis();
        // if (this->mVoiceIndex == 0) {
        // Serial.println(String("[") + mVoiceIndex + "] voice env.NoteOff. " +
        // mRunningVoice.mNoteInfo.mMidiNote.GetNoteDesc().mName);
        //}
        // even after a note off, the note continues to play through the envelope release stage. so don't really change
        // much here.
        mEnv1.noteOff();
        mEnv2.noteOff();
    }

    // a number explaining how happy this voice is to be released / note-off'd.
    float GetReleaseability() const
    {
        // TODO: consider physically held or not.
        // if never played, then 4
        if (mRunningVoice.mSource.mType == MusicalEventSourceType::Null)
        {
            return 4;
        }

        // if playing, before any release, then 0-1
        if (mRunningVoice.mReleaseTimestampMS == 0)
        {
            return TimeTo01(millis() - mRunningVoice.mNoteInfo.mAttackTimestampMS);
        }

        auto timeSinceReleaseMS = millis() - mRunningVoice.mReleaseTimestampMS;
        float releaseTimeMS =
            std::max(mRunningVoice.mSynthPatch->mEnv1.mReleaseMS, mRunningVoice.mSynthPatch->mEnv2.mReleaseMS);
        if (timeSinceReleaseMS < releaseTimeMS)
        {
            // if playing during release phase of envelope, 1-2.
            return 1 + TimeTo01(timeSinceReleaseMS);
        }

        // if idle, then 2-3 based on time since release.
        return 2 + TimeTo01(timeSinceReleaseMS - releaseTimeMS);
    }

    // only for display purposes
    bool IsConsideredPlaying_ForDisplay() const
    {
        if (mRunningVoice.mSource.mType == MusicalEventSourceType::Null)
        {
            return false;
        }

        if (mRunningVoice.mReleaseTimestampMS == 0)
        {
            // if (this->mVoiceIndex == 0) {
            //     Serial.println(String("isconsideredplaying? mRunningVoice.mReleaseTimestampMS = 0"));
            // }
            return true;
        }

        auto timeSinceReleaseMS = millis() - mRunningVoice.mReleaseTimestampMS;
        float releaseTimeMS =
            std::max(mRunningVoice.mSynthPatch->mEnv1.mReleaseMS, mRunningVoice.mSynthPatch->mEnv2.mReleaseMS);
        // TODO: if there are NO modulations that use env as source, then don't consider release time.

        // if (this->mVoiceIndex == 0)
        // {
        //     Serial.println(String("isconsideredplaying? timeSinceRelease:") + timeSinceReleaseMS +
        //                    "; releaseTime=" + releaseTimeMS);
        // }

        return timeSinceReleaseMS < releaseTimeMS;
    }

    String ToString() const
    {
        // idle
        // p1 C#9 idle
        auto ret = String("v") + mVoiceIndex + " r" + GetReleaseability() + "> ";
        if (!mRunningVoice.mSynthPatch)
        {
            ret += "<idle>";
            return ret;
        }
        // synth patch exists.
        ret += String() // + String("p") + mRunningVoice.mSynthPatchIndex + " "
               + mRunningVoice.mNoteInfo.mMidiNote.ToString() + " " +
               gEnvelopeStageInfo.GetValueString(mEnv1.GetStage());
        return ret;
    }

    Voice(int16_t vid)
        : mPatchOutDryLeft(mSplitter, 0, gpSynthGraph->voiceMixerDryLeft, vid),
          mPatchOutDryRight(mSplitter, 1, gpSynthGraph->voiceMixerDryRight, vid),
          mPatchOutDelayLeft(mSplitter, 2, gpSynthGraph->delayInputMixerLeft, vid),
          mPatchOutDelayRight(mSplitter, 3, gpSynthGraph->delayInputMixerRight, vid),
          mPatchOutVerbLeft(mSplitter, 4, gpSynthGraph->verbInputMixerLeft, vid),
          mPatchOutVerbRight(mSplitter, 5, gpSynthGraph->verbInputMixerLeft, vid), mVoiceIndex(vid)
    {
    }
};

Voice gVoices[MAX_SYNTH_VOICES] = {VOICE_INITIALIZER};

struct SynthGraphControl
{
    float mPrevMetronomeBeatFrac = 0;
    AppSettings *mAppSettings;
    Metronome *mMetronome;
    ISynthParamProvider *mParamProvider;

    void Setup(AppSettings *appSettings, Metronome *metronome, ISynthParamProvider *paramProvider)
    {
        AudioStream::initialize_memory(CLARINOID_AUDIO_MEMORY, SizeofStaticArray(CLARINOID_AUDIO_MEMORY));

        mAppSettings = appSettings;
        mMetronome = metronome;
        mParamProvider = paramProvider;

        // for some reason patches really don't like to connect unless they are
        // last in the initialization order. Here's a workaround to force them to
        // connect.
        for (auto &v : gVoices)
        {
            v.EnsurePatchConnections(appSettings, paramProvider);
        }

        gpSynthGraph->ampLeft.gain(1);
        gpSynthGraph->ampRight.gain(1);

        gpSynthGraph->metronomeEnv.delay(0);
        gpSynthGraph->metronomeEnv.attack(0);
        gpSynthGraph->metronomeEnv.hold(0);
        gpSynthGraph->metronomeEnv.releaseNoteOn(0);
        gpSynthGraph->metronomeEnv.sustain(0);
    }

    void UpdatePostFx()
    {
        auto &perf = mAppSettings->GetCurrentPerformancePatch();

        gpSynthGraph->delayFeedbackAmpLeft.gain(perf.mDelayFeedbackLevel);
        gpSynthGraph->delayFeedbackAmpRight.gain(perf.mDelayFeedbackLevel);
        gpSynthGraph->delayLeft.delay(0, perf.mDelayTime.ToMS(perf.mBPM) - perf.mDelayStereoSep * .5f);
        gpSynthGraph->delayRight.delay(0, perf.mDelayTime.ToMS(perf.mBPM) + perf.mDelayStereoSep * .5f);

        gpSynthGraph->delayFilterLeft.SetParams(
            perf.mDelayFilterType, perf.mDelayCutoffFrequency, perf.mDelayQ, perf.mDelaySaturation);
        gpSynthGraph->delayFilterRight.SetParams(
            perf.mDelayFilterType, perf.mDelayCutoffFrequency, perf.mDelayQ, perf.mDelaySaturation);

        gpSynthGraph->delayWetAmpLeft.gain(
            (perf.mMasterFXEnable && perf.mDelayEnabled) ? (perf.mDelayGain * perf.mMasterFXGain) : 0.0f);
        gpSynthGraph->delayWetAmpRight.gain(
            (perf.mMasterFXEnable && perf.mDelayEnabled) ? (perf.mDelayGain * perf.mMasterFXGain) : 0.0f);

        gpSynthGraph->verb.roomsize(perf.mReverbSize);
        gpSynthGraph->verb.damping(perf.mReverbDamping);

        gpSynthGraph->verbWetAmpLeft.gain(
            (perf.mMasterFXEnable && perf.mReverbEnabled) ? (perf.mReverbGain * perf.mMasterFXGain) : 0.0f);
        gpSynthGraph->verbWetAmpRight.gain(
            (perf.mMasterFXEnable && perf.mReverbEnabled) ? (perf.mReverbGain * perf.mMasterFXGain) : 0.0f);

        gpSynthGraph->ampLeft.gain(perf.mMasterGain);
        gpSynthGraph->ampRight.gain(perf.mMasterGain);

        if (!mAppSettings->mMetronomeSoundOn)
        {
            gpSynthGraph->metronomeOsc.amplitude(0);
        }
        else
        {
            gpSynthGraph->metronomeEnv.decay(mAppSettings->mMetronomeDecayMS);
            gpSynthGraph->metronomeOsc.amplitude(mAppSettings->mMetronomeGain);
            gpSynthGraph->metronomeOsc.frequency(MIDINoteToFreq(mAppSettings->mMetronomeNote));

            float metronomeBeatFrac = mMetronome->GetBeatFrac();
            if (metronomeBeatFrac < mPrevMetronomeBeatFrac)
            { // beat boundary is when the frac drops back
              // to 0
                gpSynthGraph->metronomeEnv.noteOn();
            }
            mPrevMetronomeBeatFrac = metronomeBeatFrac;
        }
    }
};

SynthGraphControl gSynthGraphControl;

} // namespace clarinoid
