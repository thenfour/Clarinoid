
#pragma once

#ifdef CLARINOID_MODULE_TEST
#error not for x86 tests
#endif

#include <cfloat>

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/loopstation/LoopstationMemory.hpp>

#include "FilterNode.hpp"
#include "PannerNode.hpp"
#include "MultiMixer.hpp"
#include "ModulationMatrixNode.hpp"
#include "Patch.hpp"
#include "polyBlepOscillator.hpp"

namespace clarinoid
{

namespace CCSynthGraph
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
AudioConnection patchCord1(delayWetAmpLeft, 0, postMixerLeft, 1);
AudioConnection patchCord2(delayWetAmpRight, 0, postMixerRight, 1);
AudioConnection patchCord3(verbInputMixer, verb);
AudioConnection patchCord4(metronomeOsc, metronomeEnv);
AudioConnection patchCord5(verb, 0, verbWetAmpLeft, 0);
AudioConnection patchCord6(verb, 1, verbWetAmpRight, 0);
AudioConnection patchCord7(metronomeEnv, 0, postMixerRight, 3);
AudioConnection patchCord8(metronomeEnv, 0, postMixerLeft, 3);
AudioConnection patchCord9(verbWetAmpLeft, 0, postMixerLeft, 2);
AudioConnection patchCord10(verbWetAmpRight, 0, postMixerRight, 2);
AudioConnection patchCord11(postMixerLeft, ampLeft);
AudioConnection patchCord12(postMixerRight, ampRight);
AudioConnection patchCord13(ampLeft, peakL);
AudioConnection patchCord14(ampRight, peakR);
AudioConnection patchCord15(ampLeft, 0, i2s1, 1);
AudioConnection patchCord16(ampRight, 0, i2s1, 0);
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
::clarinoid::MultiMixerNode<MAX_SYNTH_VOICES + 1> delayInputMixerLeft;  // +1 to account for the delay feedback signal
::clarinoid::MultiMixerNode<MAX_SYNTH_VOICES + 1> delayInputMixerRight; // +1 to account for the delay feedback signal
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

} // namespace CCSynthGraph

struct Voice
{
    //
    // [mOsc] osc1 --> 0
    // [mOsc] osc2 --> 1 [mOscMixerPanner] --> L [mFilter] L -> L [mSplitter] ... out
    // [mOsc] osc3 --> 2                   --> R           R -> R
    //
    AudioBandlimitedOsci mOsc;

    // Modulation sources
    VoiceModulationMatrixNode mModMatrix;
    AudioEffectEnvelope mEnv1;
    AudioEffectEnvelope mEnv2;
    AudioSynthWaveformDc mEnv1PeakDC;
    AudioSynthWaveformDc mEnv2PeakDC;
    CCPatch mPatchDCToEnv1 = {mEnv1PeakDC, 0, mEnv1, 0};
    CCPatch mPatchDCToEnv2 = {mEnv2PeakDC, 0, mEnv2, 0};
    AudioSynthWaveform mLfo1;
    AudioSynthWaveform mLfo2;
    AudioSynthWaveformDc mBreathModSource;
    AudioSynthWaveformDc mPitchBendModSource;

    // have to track these because they're private members of AudioWaveform.
    short mLfo1Waveshape = 0xff; // invalid so 1st run will always set the shape.
    short mLfo2Waveshape = 0xff; // invalid so 1st run will always set the shape.

    // patch modulation sources in
    CCPatch mPatchEnv1ToMod = {mEnv1, 0, mModMatrix, (uint8_t)ModulationSourceToIndex(ModulationSource::ENV1)};
    CCPatch mPatchEnv2ToMod = {mEnv2, 0, mModMatrix, (uint8_t)ModulationSourceToIndex(ModulationSource::ENV2)};
    CCPatch mPatchLfo1ToMod = {mLfo1, 0, mModMatrix, (uint8_t)ModulationSourceToIndex(ModulationSource::LFO1)};
    CCPatch mPatchLfo2ToMod = {mLfo2, 0, mModMatrix, (uint8_t)ModulationSourceToIndex(ModulationSource::LFO2)};

    CCPatch mPatchBreathToMod = {mBreathModSource,
                                 0,
                                 mModMatrix,
                                 (uint8_t)ModulationSourceToIndex(ModulationSource::Breath)};
    CCPatch mPatchPitchStripToMod = {mPitchBendModSource,
                                     0,
                                     mModMatrix,
                                     (uint8_t)ModulationSourceToIndex(ModulationSource::PitchStrip)};

    CCPatch mOsc1FBToMod = {mOsc, 0, mModMatrix, (uint8_t)ModulationSourceToIndex(ModulationSource::Osc1FB)};
    CCPatch mOsc2FBToMod = {mOsc, 1, mModMatrix, (uint8_t)ModulationSourceToIndex(ModulationSource::Osc2FB)};
    CCPatch mOsc3FBToMod = {mOsc, 2, mModMatrix, (uint8_t)ModulationSourceToIndex(ModulationSource::Osc3FB)};

    // Patch modulation destinations
    /*
Input 0: Frequency Modulation for Oscillator 1
Input 1: Pulse Width Modulation for Oscillator 1
Input 2: Frequency Modulation for Oscillator 2
Input 3: Pulse Width Modulation for Oscillator 2
Input 4: Frequency Modulation for Oscillator 3
Input 5: Pulse Width Modulation for Oscillator 3
    */
    CCPatch mPatchModToOsc1PWM = {mModMatrix,
                                  (uint8_t)ModulationDestinationToIndex(ModulationDestination::Osc1PulseWidth),
                                  mOsc,
                                  (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::pwm1};
    CCPatch mPatchModToOsc2PWM = {mModMatrix,
                                  (uint8_t)ModulationDestinationToIndex(ModulationDestination::Osc2PulseWidth),
                                  mOsc,
                                  (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::pwm2};
    CCPatch mPatchModToOsc3PWM = {mModMatrix,
                                  (uint8_t)ModulationDestinationToIndex(ModulationDestination::Osc3PulseWidth),
                                  mOsc,
                                  (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::pwm3};

    CCPatch mPatchModToOsc1Freq = {mModMatrix,
                                   (uint8_t)ModulationDestinationToIndex(ModulationDestination::Osc1Frequency),
                                   mOsc,
                                   (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::fm1};
    CCPatch mPatchModToOsc2Freq = {mModMatrix,
                                   (uint8_t)ModulationDestinationToIndex(ModulationDestination::Osc2Frequency),
                                   mOsc,
                                   (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::fm2};
    CCPatch mPatchModToOsc3Freq = {mModMatrix,
                                   (uint8_t)ModulationDestinationToIndex(ModulationDestination::Osc3Frequency),
                                   mOsc,
                                   (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::fm3};

    CCPatch mPatchModToOsc1Phase = {mModMatrix,
                                    (uint8_t)ModulationDestinationToIndex(ModulationDestination::Osc1Phase),
                                    mOsc,
                                    (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::pm1};
    CCPatch mPatchModToOsc2Phase = {mModMatrix,
                                    (uint8_t)ModulationDestinationToIndex(ModulationDestination::Osc2Phase),
                                    mOsc,
                                    (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::pm2};
    CCPatch mPatchModToOsc3Phase = {mModMatrix,
                                    (uint8_t)ModulationDestinationToIndex(ModulationDestination::Osc3Phase),
                                    mOsc,
                                    (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::pm3};

    CCPatch mPatchModToOsc1Amp = {mModMatrix,
                                  (uint8_t)ModulationDestinationToIndex(ModulationDestination::Osc1Amplitude),
                                  mOsc,
                                  (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::am1};
    CCPatch mPatchModToOsc2Amp = {mModMatrix,
                                  (uint8_t)ModulationDestinationToIndex(ModulationDestination::Osc2Amplitude),
                                  mOsc,
                                  (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::am2};
    CCPatch mPatchModToOsc3Amp = {mModMatrix,
                                  (uint8_t)ModulationDestinationToIndex(ModulationDestination::Osc3Amplitude),
                                  mOsc,
                                  (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::am3};

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

    MusicalVoice mRunningVoice;
    SynthPreset *mPreset = nullptr;
    AppSettings *mAppSettings;
    bool mTouched = false;

    void EnsurePatchConnections(AppSettings *appSettings)
    {
        mAppSettings = appSettings;

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

        mPatchBreathToMod.connect();
        mPatchPitchStripToMod.connect();

        mPatchModToOsc1PWM.connect();
        mPatchModToOsc2PWM.connect();
        mPatchModToOsc3PWM.connect();

        mPatchModToOsc1Freq.connect();
        mPatchModToOsc2Freq.connect();
        mPatchModToOsc3Freq.connect();

        mPatchModToOsc1Phase.connect();
        mPatchModToOsc2Phase.connect();
        mPatchModToOsc3Phase.connect();

        mPatchModToOsc1Amp.connect();
        mPatchModToOsc2Amp.connect();
        mPatchModToOsc3Amp.connect();

        mPatchDCToEnv1.connect();
        mPatchDCToEnv2.connect();
    }

    static float CalcFilterCutoffFreq(float breath01,
                                      float midiNote,
                                      float keyTrackingAmt,
                                      float freqMin,
                                      float freqMax)
    {
        // perform breath & key tracking for filter. we will basically multiply the
        // effects. velocity we will only track between notes from 7jam code: const
        // halfKeyScaleRangeSemis = 12 * 4; from 7jam code: let ks = 1.0 -
        // DF.remap(this.midiNote, 60.0 /* middle C */ -
        // halfKeyScaleRangeSemis, 60.0 + halfKeyScaleRangeSemis, ksAmt, -ksAmt); //
        // when vsAmt is 0, the range of vsAmt,-vsAmt is 0. hence making this 1.0-x
        float filterKS = map(midiNote, 20, 120, 0.0f, 1.0f); // map midi note to full ks effect
        filterKS = map(keyTrackingAmt, 0, 1.0f, 1.0,
                       filterKS); // map ks amt 0-1 to 1-fulleffect

        float filterP = filterKS * breath01;
        filterP = ClampInclusive(filterP, 0.0f, 1.0f);

        float filterFreq = map(filterP, 0.0f, 1.0f, freqMin, freqMax);
        return filterFreq;
    }

    void Update(const MusicalVoice &mv)
    {
        // NOTE: we don't care about SynthPatchB at this point.
        mPreset = &mAppSettings->FindSynthPreset(mv.mSynthPatchA);
        mModMatrix.SetSynthPatch(mPreset);
        auto transition = CalculateTransitionEvents(mRunningVoice, mv);
        bool voiceOrPatchChanged =
            (mRunningVoice.mVoiceId != mv.mVoiceId) || (mRunningVoice.mSynthPatchA != mv.mSynthPatchA);
        if (voiceOrPatchChanged || transition.mNeedsNoteOff)
        {
            mOsc.removeNote();
        }
        if (voiceOrPatchChanged || transition.mNeedsNoteOn)
        {
            mOsc.addNote(); // this also engages portamento

            for (size_t i = 0; i < POLYBLEP_OSC_COUNT; ++i)
            {
                if (mPreset->mOsc[i].mPhaseRestart)
                {
                    mOsc.mOsc[i].ResetPhase();
                }
            }
        }

        if (!mv.IsPlaying() || mv.mIsNoteCurrentlyMuted)
        {
            for (size_t i = 0; i < POLYBLEP_OSC_COUNT; ++i)
            {
                mOsc.mOsc[i].amplitude(0);
            }
            // for CPU saving (currently causes glitches)
            // mOsc.Disable();
            // mFilter.Disable();
            mRunningVoice = mv;
            return;
        }
        else
        {
            // mOsc.Enable();
            // mFilter.Enable();
        }

        // configure envelopes (DADSR x 3)
        mEnv1.delay(mPreset->mEnv1.mDelayMS);
        mEnv1.attack(mPreset->mEnv1.mAttackMS);
        mEnv1.hold(mPreset->mEnv1.mHoldMS);
        mEnv1.decay(mPreset->mEnv1.mDecayMS);
        mEnv1.sustain(mPreset->mEnv1.mSustainLevel);
        mEnv1.release(mPreset->mEnv1.mReleaseMS);
        mEnv1.releaseNoteOn(mPreset->mEnv1.mReleaseNoteOnMS);

        mEnv2.delay(mPreset->mEnv2.mDelayMS);
        mEnv2.attack(mPreset->mEnv2.mAttackMS);
        mEnv2.hold(mPreset->mEnv2.mHoldMS);
        mEnv2.decay(mPreset->mEnv2.mDecayMS);
        mEnv2.sustain(mPreset->mEnv2.mSustainLevel);
        mEnv2.release(mPreset->mEnv2.mReleaseMS);
        mEnv2.releaseNoteOn(mPreset->mEnv2.mReleaseNoteOnMS);

        auto convertWaveType = [](OscWaveformShape s) {
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

        short wantsWaveType1 = convertWaveType(mPreset->mLfo1Shape);
        short wantsWaveType2 = convertWaveType(mPreset->mLfo2Shape);
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

        mLfo1.frequency(mPreset->mLfo1Rate);
        mLfo2.frequency(mPreset->mLfo2Rate);

        mBreathModSource.amplitude(mv.mBreath01.GetFloatVal());
        mPitchBendModSource.amplitude(mv.mPitchBendN11.GetFloatVal());

        if (voiceOrPatchChanged || transition.mNeedsNoteOff)
        {
            mEnv1.noteOff();
            mEnv2.noteOff();
        }

        if (voiceOrPatchChanged || transition.mNeedsNoteOn)
        {
            mEnv1.noteOn();
            mEnv2.noteOn();
        }

        // update
        float midiNote = (float)mv.mMidiNote;

        bool outputEnable[POLYBLEP_OSC_COUNT];
        outputEnable[0] = true;
        outputEnable[1] = true;
        outputEnable[2] = true;
        static_assert(POLYBLEP_OSC_COUNT == 3, "this is designed only for 3 oscillators");

        switch (mPreset->mFMAlgo)
        {
        default:
        case FMAlgo::c1c2c3_NoFM: // [1][2][3]
            break;
        case FMAlgo::c1m2m3_Chain: // [1<2<3]
            outputEnable[1] = false;
            outputEnable[2] = false;
            break;
        case FMAlgo::m1c2c3_FM21_NoFM3: // [1>2][3]
            outputEnable[0] = false;
            break;
        case FMAlgo::c1m2c3_FM12_NoFM3:  // [1<2][3]
        case FMAlgo::c2m2c3_FM13_Split2: // [1<2][2>3]
            outputEnable[1] = false;
            break;
        case FMAlgo::c1c2m3_FM32_NoFM1: // [1][2<3]
            outputEnable[2] = false;
            break;
        case FMAlgo::c1m2c3_FM23_NoFM1: // [1][2>3]
            outputEnable[1] = false;
            break;
        case FMAlgo::c1m23: // [1<(2&3)]
            outputEnable[1] = false;
            outputEnable[2] = false;
            break;
        }

        for (size_t i = 0; i < POLYBLEP_OSC_COUNT; ++i)
        {
            // if the output is disabled, then the multimixerpanner will disable this channel,
            // and its output is only designed for modulation sources. then the gain will be
            // specified in the modulation amount.
            mOsc.mOsc[i].amplitude(outputEnable[i] ? mPreset->mOsc[i].mGain : 1.0f);
            mOsc.mOsc[i].SetPhaseOffset(mPreset->mOsc[i].mPhase01);

            mOsc.mOsc[i].portamentoTime(mPreset->mOsc[i].mPortamentoTime);

            mOsc.mOsc[i].waveform(mPreset->mOsc[i].mWaveform);
            mOsc.mOsc[i].pulseWidth(mPreset->mOsc[i].mPulseWidth);
            mOsc.mOsc[i].fmAmount(1);
            mOsc.mOsc[i].mPMMultiplier = mPreset->mFMStrength;
            mOsc.mOsc[i].mAMMinimumGain = mPreset->mOsc[i].mAMMinimumGain;
            mOsc.mOsc[i].mPMFeedbackAmt = mPreset->mOsc[i].mFMFeedbackGain;
        }

        auto calcFreq = [](float midiNote,
                           float pbSemis,
                           float pbSnap,
                           float pitchFine,
                           int pitchSemis,
                           float detune,
                           float freqMul,
                           float freqOffset) {
            float ret = midiNote + pitchFine + pitchSemis + detune + SnapPitchBend(pbSemis, pbSnap);
            ret = (MIDINoteToFreq(ret) * freqMul) + freqOffset;
            return Clamp(ret, 0.0f, 22050.0f);
        };

        mOsc.frequency(1,
                       calcFreq(midiNote,
                                mPreset->mOsc[0].mPitchBendRange * mv.mPitchBendN11.GetFloatVal(),
                                mPreset->mOsc[0].mPitchBendSnap,
                                mPreset->mOsc[0].mPitchFine,
                                mPreset->mOsc[0].mPitchSemis,
                                -mPreset->mDetune,
                                mPreset->mOsc[0].mFreqMultiplier,
                                mPreset->mOsc[0].mFreqOffset));
        mOsc.frequency(3,
                       calcFreq(midiNote,
                                mPreset->mOsc[2].mPitchBendRange * mv.mPitchBendN11.GetFloatVal(),
                                mPreset->mOsc[2].mPitchBendSnap,
                                mPreset->mOsc[2].mPitchFine,
                                mPreset->mOsc[2].mPitchSemis,
                                mPreset->mDetune,
                                mPreset->mOsc[2].mFreqMultiplier,
                                mPreset->mOsc[2].mFreqOffset));

        if (mPreset->mSync)
        {
            float freq = calcFreq(midiNote,
                                  mPreset->mOsc[1].mPitchBendRange * mv.mPitchBendN11.GetFloatVal(),
                                  mPreset->mOsc[1].mPitchBendSnap,
                                  mPreset->mOsc[1].mPitchFine,
                                  mPreset->mOsc[1].mPitchSemis,
                                  0,
                                  mPreset->mOsc[1].mFreqMultiplier,
                                  mPreset->mOsc[1].mFreqOffset);
            float freqSync =
                map(mv.mBreath01.GetFloatVal(), 0.0f, 1.0f, freq * mPreset->mSyncMultMin, freq * mPreset->mSyncMultMax);
            mOsc.frequency(2, freqSync);
        }
        else
        {
            mOsc.frequency(2,
                           calcFreq(midiNote,
                                    mPreset->mOsc[1].mPitchBendRange * mv.mPitchBendN11.GetFloatVal(),
                                    mPreset->mOsc[1].mPitchBendSnap,
                                    mPreset->mOsc[1].mPitchFine,
                                    mPreset->mOsc[1].mPitchSemis,
                                    0,
                                    mPreset->mOsc[1].mFreqMultiplier,
                                    mPreset->mOsc[1].mFreqOffset));
        }

        // perform breath & key tracking for filter. we will basically multiply the
        // effects.
        float filterFreq = CalcFilterCutoffFreq(mv.mBreath01.GetFloatVal(),
                                                midiNote,
                                                mPreset->mFilterKeytracking,
                                                mPreset->mFilterMinFreq,
                                                mPreset->mFilterMaxFreq);

        mFilter.SetParams(mPreset->mFilterType, filterFreq, mPreset->mFilterQ, mPreset->mFilterSaturation);
        mFilter.EnableDCFilter(mPreset->mDCFilterEnabled, mPreset->mDCFilterCutoff);

        // mOscMixerPanner.SetInputPan();
        mSplitter.SetOutputGain(0, 1.0f);
        mSplitter.SetOutputGain(1, mPreset->mDelaySend);
        mSplitter.SetOutputGain(2, mPreset->mVerbSend);

        mOscMixerPanner.SetInputPanAndEnabled(
            0, mv.mPan + mPreset->mPan + mPreset->mOsc[0].mPan + mPreset->mStereoSpread, outputEnable[0]);
        mOscMixerPanner.SetInputPanAndEnabled(1, mv.mPan + mPreset->mPan + mPreset->mOsc[1].mPan, outputEnable[1]);
        mOscMixerPanner.SetInputPanAndEnabled(
            2, mv.mPan + mPreset->mPan + mPreset->mOsc[2].mPan - mPreset->mStereoSpread, outputEnable[2]);

        mRunningVoice = mv;
    }

    bool IsPlaying() const
    {
        // this function lets us delay for example, if there's a release stage
        // (theoretically)
        return mRunningVoice.IsPlaying();
    }

    void Unassign()
    {
        mRunningVoice.mVoiceId = MAGIC_VOICE_ID_UNASSIGNED;
    }

    Voice(int16_t vid)
        : mPatchOutDryLeft(mSplitter, 0, CCSynthGraph::voiceMixerDryLeft, vid),
          mPatchOutDryRight(mSplitter, 1, CCSynthGraph::voiceMixerDryRight, vid),
          mPatchOutDelayLeft(mSplitter, 2, CCSynthGraph::delayInputMixerLeft, vid),
          mPatchOutDelayRight(mSplitter, 3, CCSynthGraph::delayInputMixerRight, vid),
          mPatchOutVerbLeft(mSplitter, 4, CCSynthGraph::verbInputMixerLeft, vid),
          mPatchOutVerbRight(mSplitter, 5, CCSynthGraph::verbInputMixerLeft, vid)
    {
    }
};

Voice gVoices[MAX_SYNTH_VOICES] = {{0}, {1}, {2}, {3}, {4}, {5}, /* {6}, {7}*/};

struct SynthGraphControl
{
    float mPrevMetronomeBeatFrac = 0;
    AppSettings *mAppSettings;
    Metronome *mMetronome;

    void Setup(AppSettings *appSettings, Metronome *metronome /*, IModulationSourceSource *modulationSourceSource*/)
    {
        AudioStream::initialize_memory(CLARINOID_AUDIO_MEMORY, SizeofStaticArray(CLARINOID_AUDIO_MEMORY));

        mAppSettings = appSettings;
        mMetronome = metronome;

        // for some reason patches really don't like to connect unless they are
        // last in the initialization order. Here's a workaround to force them to
        // connect.
        for (auto &v : gVoices)
        {
            v.EnsurePatchConnections(appSettings /*, modulationSourceSource*/);
        }

        CCSynthGraph::ampLeft.gain(1);
        CCSynthGraph::ampRight.gain(1);

        CCSynthGraph::metronomeEnv.delay(0);
        CCSynthGraph::metronomeEnv.attack(0);
        CCSynthGraph::metronomeEnv.hold(0);
        CCSynthGraph::metronomeEnv.releaseNoteOn(0);
        CCSynthGraph::metronomeEnv.sustain(0);
    }

    void BeginUpdate()
    {
        AudioNoInterrupts(); // https://www.pjrc.com/teensy/td_libs_AudioProcessorUsage.html
    }

    void EndUpdate()
    {
        AudioInterrupts();
    }

    void UpdatePostFx()
    {
        auto &perf = mAppSettings->GetCurrentPerformancePatch();

        CCSynthGraph::delayFeedbackAmpLeft.gain(perf.mDelayFeedbackLevel);
        CCSynthGraph::delayFeedbackAmpRight.gain(perf.mDelayFeedbackLevel);
        CCSynthGraph::delayLeft.delay(0, perf.mDelayMS);
        CCSynthGraph::delayRight.delay(0, perf.mDelayMS + perf.mDelayStereoSep);

        CCSynthGraph::delayFilterLeft.SetParams(
            perf.mDelayFilterType, perf.mDelayCutoffFrequency, perf.mDelayQ, perf.mDelaySaturation);
        CCSynthGraph::delayFilterRight.SetParams(
            perf.mDelayFilterType, perf.mDelayCutoffFrequency, perf.mDelayQ, perf.mDelaySaturation);

        CCSynthGraph::delayWetAmpLeft.gain(perf.mMasterFXEnable ? perf.mDelayGain : 0.0f);
        CCSynthGraph::delayWetAmpRight.gain(perf.mMasterFXEnable ? perf.mDelayGain : 0.0f);

        CCSynthGraph::verb.roomsize(perf.mReverbSize);
        CCSynthGraph::verb.damping(perf.mReverbDamping);

        CCSynthGraph::verbWetAmpLeft.gain(perf.mMasterFXEnable ? perf.mReverbGain : 0.0f);
        CCSynthGraph::verbWetAmpRight.gain(perf.mMasterFXEnable ? perf.mReverbGain : 0.0f);

        CCSynthGraph::ampLeft.gain(perf.mMasterGain);
        CCSynthGraph::ampRight.gain(perf.mMasterGain);

        if (!mAppSettings->mMetronomeSoundOn)
        {
            CCSynthGraph::metronomeOsc.amplitude(0);
        }
        else
        {
            CCSynthGraph::metronomeEnv.decay(mAppSettings->mMetronomeDecayMS);
            CCSynthGraph::metronomeOsc.amplitude(mAppSettings->mMetronomeGain);
            CCSynthGraph::metronomeOsc.frequency(MIDINoteToFreq(mAppSettings->mMetronomeNote));

            float metronomeBeatFrac = mMetronome->GetBeatFrac();
            if (metronomeBeatFrac < mPrevMetronomeBeatFrac)
            { // beat boundary is when the frac drops back
              // to 0
                CCSynthGraph::metronomeEnv.noteOn();
            }
            mPrevMetronomeBeatFrac = metronomeBeatFrac;
        }
    }
};

SynthGraphControl gSynthGraphControl;

} // namespace clarinoid
