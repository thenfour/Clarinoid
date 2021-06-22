
#pragma once

#ifdef CLARINOID_MODULE_TEST
#error not for x86 tests
#endif

#include <cfloat>

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/loopstation/LoopstationMemory.hpp>

#include "Patch.hpp"
#include "FilterNode.hpp"
#include "PannerNode.hpp"

namespace clarinoid
{

  namespace CCSynthGraph
  {
    /*
https://www.pjrc.com/teensy/gui/index.html
// this is the after-oscillator processing.

*/




// GUItool: begin automatically generated code
AudioMixer4              voiceMix2Left;      //xy=224.00569915771484,595.9999580383301
AudioMixer4              voiceMix1Left;      //xy=225.0056915283203,527.9999923706055
AudioMixer4              voiceMix2Right; //xy=230.0056915283203,782.0056762695312
AudioMixer4              voiceMix1Right; //xy=231.00568389892578,714.0057106018066
AudioMixer4              voiceMixOutpLeft;   //xy=436.00565338134766,559.9999732971191
AudioMixer4              voiceMixOutpRight; //xy=442.0056457519531,746.0056915283203
AudioMixer4              delayInputMixerLeft; //xy=734.0056915283203,230
AudioMixer4              delayInputMixerRight; //xy=768.0056915283203,508
AudioAmplifier           delayFeedbackAmpLeft; //xy=807.0056915283203,85
AudioAmplifier           delayFeedbackAmpRight; //xy=864.0056915283203,370
AudioEffectDelay         delayLeft;      //xy=905.0056915283203,230
AudioEffectDelay         delayRight;     //xy=947.0056915283203,508
AudioMixer4              waveMixerLeft;  //xy=1270.0056915283203,647
AudioMixer4              waveMixerRight; //xy=1275.0056915283203,725
AudioMixer4              verbInputMixer; //xy=1491.0056915283203,526
AudioSynthWaveformSine   metronomeOsc;   //xy=1536.0056915283203,939
AudioEffectFreeverbStereo verb;           //xy=1650.0056915283203,516
AudioEffectEnvelope      metronomeEnv;   //xy=1737.0056915283203,941
AudioAmplifier           verbWetAmpLeft; //xy=1809.0056915283203,497
AudioAmplifier           verbWetAmpRight; //xy=1816.0056915283203,535
AudioMixer4              postMixerLeft;  //xy=1991.0056915283203,724
AudioMixer4              postMixerRight; //xy=1993.0056915283203,807
AudioAmplifier           ampLeft;        //xy=2153.0056915283203,725
AudioAmplifier           ampRight;       //xy=2156.0056915283203,806
AudioOutputI2S           i2s1;           //xy=2338.0056915283203,758
AudioAnalyzePeak         peak1;          //xy=2381.0056915283203,537
AudioConnection          patchCord1(voiceMix2Left, 0, voiceMixOutpLeft, 1);
AudioConnection          patchCord2(voiceMix1Left, 0, voiceMixOutpLeft, 0);
AudioConnection          patchCord3(voiceMix2Right, 0, voiceMixOutpRight, 1);
AudioConnection          patchCord4(voiceMix1Right, 0, voiceMixOutpRight, 0);
AudioConnection          patchCord5(voiceMixOutpLeft, 0, waveMixerLeft, 1);
AudioConnection          patchCord6(voiceMixOutpLeft, 0, delayInputMixerLeft, 3);
AudioConnection          patchCord7(voiceMixOutpRight, 0, delayInputMixerRight, 1);
AudioConnection          patchCord8(voiceMixOutpRight, 0, waveMixerRight, 1);
AudioConnection          patchCord9(delayInputMixerLeft, delayLeft);
AudioConnection          patchCord10(delayInputMixerRight, delayRight);
AudioConnection          patchCord11(delayFeedbackAmpLeft, 0, delayInputMixerLeft, 0);
AudioConnection          patchCord12(delayFeedbackAmpRight, 0, delayInputMixerRight, 0);
AudioConnection          patchCord13(waveMixerLeft, 0, postMixerLeft, 1);
AudioConnection          patchCord14(waveMixerLeft, 0, verbInputMixer, 0);
AudioConnection          patchCord15(waveMixerRight, 0, verbInputMixer, 1);
AudioConnection          patchCord16(waveMixerRight, 0, postMixerRight, 0);
AudioConnection          patchCord17(verbInputMixer, verb);
AudioConnection          patchCord18(metronomeOsc, metronomeEnv);
AudioConnection          patchCord19(verb, 0, verbWetAmpLeft, 0);
AudioConnection          patchCord20(verb, 1, verbWetAmpRight, 0);
AudioConnection          patchCord21(metronomeEnv, 0, postMixerLeft, 2);
AudioConnection          patchCord22(metronomeEnv, 0, postMixerRight, 2);
AudioConnection          patchCord23(verbWetAmpLeft, 0, postMixerLeft, 0);
AudioConnection          patchCord24(verbWetAmpRight, 0, postMixerRight, 1);
AudioConnection          patchCord25(postMixerLeft, ampLeft);
AudioConnection          patchCord26(postMixerRight, ampRight);
AudioConnection          patchCord27(ampLeft, peak1);
AudioConnection          patchCord28(ampLeft, 0, i2s1, 1);
AudioConnection          patchCord29(ampRight, 0, i2s1, 0);
// GUItool: end automatically generated code



  } // namespace CCSynthGraph

  struct Voice
  {
    //
    // [mOsc] --> [mOscMixer] --> [mFilter] -> [panner]
    //
    AudioBandlimitedOsci mOsc;
    CCPatch mPatchOsc1ToMix;
    CCPatch mPatchOsc2ToMix;
    CCPatch mPatchOsc3ToMix;
    AudioMixer4 mOscMixer;     // mixes down the 3 oscillators
    CCPatch mPatchMixToFilter; // then into filter.
    ::clarinoid::FilterNode mFilter;

    CCPatch mPatchFilterToPanner;
    ::clarinoid::PannerNode mPanner;

    CCPatch mPatchOutLeft;
    CCPatch mPatchOutRight;

    MusicalVoice mRunningVoice;
    SynthPreset *mPreset = nullptr;

    AppSettings *mAppSettings;
    IModulationSourceSource* mModulationSourceSource;

    void EnsurePatchConnections(AppSettings *appSettings, IModulationSourceSource* modulationSourceSource)
    {
      mAppSettings = appSettings;
      mModulationSourceSource = modulationSourceSource;

      mPatchOsc1ToMix.connect();
      mPatchOsc2ToMix.connect();
      mPatchOsc3ToMix.connect();
      mPatchMixToFilter.connect();
      mPatchFilterToPanner.connect();
      mPatchOutLeft.connect();
      mPatchOutRight.connect();
    }

    static float CalcFilterCutoffFreq(float breath01, float midiNote, float keyTrackingAmt, float freqMin, float freqMax)
    {
      // perform breath & key tracking for filter. we will basically multiply the effects.
      // velocity we will only track between notes
      // from 7jam code: const halfKeyScaleRangeSemis = 12 * 4;
      // from 7jam code: let ks = 1.0 - DF.remap(this.midiNote, 60.0 /* middle C */ - halfKeyScaleRangeSemis, 60.0 + halfKeyScaleRangeSemis, ksAmt, -ksAmt); // when vsAmt is 0, the range of vsAmt,-vsAmt is 0. hence making this 1.0-x
      float filterKS = map(midiNote, 20, 120, 0.0f, 1.0f);    // map midi note to full ks effect
      filterKS = map(keyTrackingAmt, 0, 1.0f, 1.0, filterKS); // map ks amt 0-1 to 1-fulleffect

      float filterP = filterKS * breath01;
      filterP = ClampInclusive(filterP, 0.0f, 1.0f);

      float filterFreq = map(filterP, 0.0f, 1.0f, freqMin, freqMax);
      return filterFreq;
    }

    float GetModulatedValue(float baseOutpVal /* in dest range */, ModulationDestination destType) {
      float ret = baseOutpVal;
      for (const SynthModulationSpec& modSpec : mPreset->mModulations) {
        if (modSpec.mDest != destType) continue;
        float srcVal = mModulationSourceSource->GetCurrentModulationSourceValue(modSpec.mSource);
        ret += modSpec.mCurveSpec.PerformMapping(srcVal);
      }
      return ret;
    }

    void Update(const MusicalVoice &mv)
    {
      mPreset = &mAppSettings->FindSynthPreset(mv.mSynthPatch);
      auto transition = CalculateTransitionEvents(mRunningVoice, mv);
      bool voiceOrPatchChanged = (mRunningVoice.mVoiceId != mv.mVoiceId) || (mRunningVoice.mSynthPatch != mv.mSynthPatch);
      if (voiceOrPatchChanged || transition.mNeedsNoteOff)
      {
        mOsc.removeNote();
      }
      if (voiceOrPatchChanged || transition.mNeedsNoteOn) {
        mOsc.addNote(); // this also engages portamento
      }

      if (!mv.IsPlaying() || mv.mIsNoteCurrentlyMuted)
      {
        mOsc.amplitude(1, 0.0);
        mOsc.amplitude(2, 0.0);
        mOsc.amplitude(3, 0.0);
        mRunningVoice = mv;
        return;
      }

      mOsc.amplitude(1, GetModulatedValue(mPreset->mOsc1Gain, ModulationDestination::Osc1Volume));
      mOsc.amplitude(2, GetModulatedValue(mPreset->mOsc2Gain, ModulationDestination::Osc2Volume));
      mOsc.amplitude(3, GetModulatedValue(mPreset->mOsc3Gain, ModulationDestination::Osc3Volume));

      // update
      float midiNote = (float)mv.mMidiNote + mv.mPitchBendN11.GetFloatVal() * mAppSettings->mSynthSettings.mPitchBendRange;

      mOsc.portamentoTime(1, mPreset->mPortamentoTime);
      mOsc.portamentoTime(2, mPreset->mPortamentoTime);
      mOsc.portamentoTime(3, mPreset->mPortamentoTime);

      mOsc.waveform(1, (uint8_t)mPreset->mOsc1Waveform);
      mOsc.waveform(2, (uint8_t)mPreset->mOsc2Waveform);
      mOsc.waveform(3, (uint8_t)mPreset->mOsc3Waveform);

      mOsc.pulseWidth(1, GetModulatedValue(mPreset->mOsc1PulseWidth, ModulationDestination::Osc1PulseWidth));
      mOsc.pulseWidth(2, GetModulatedValue(mPreset->mOsc2PulseWidth, ModulationDestination::Osc2PulseWidth));
      mOsc.pulseWidth(3, GetModulatedValue(mPreset->mOsc3PulseWidth, ModulationDestination::Osc3PulseWidth));

      mOsc.frequency(1, MIDINoteToFreq(midiNote + mPreset->mOsc1PitchFine + mPreset->mOsc1PitchSemis - mPreset->mDetune));
      mOsc.frequency(3, MIDINoteToFreq(midiNote + mPreset->mOsc3PitchFine + mPreset->mOsc3PitchSemis + mPreset->mDetune));

      if (mPreset->mSync)
      {
        float freq = MIDINoteToFreq(midiNote + mPreset->mOsc2PitchFine + mPreset->mOsc2PitchSemis);
        float freqSync = map(mv.mBreath01.GetFloatVal(), 0.0f, 1.0f, freq * mPreset->mSyncMultMin, freq * mPreset->mSyncMultMax);
        mOsc.frequency(2, freqSync);
      }
      else
      {
        mOsc.frequency(2, MIDINoteToFreq(midiNote + mPreset->mOsc2PitchFine + mPreset->mOsc2PitchSemis));
      }

      // perform breath & key tracking for filter. we will basically multiply the effects.
      float filterFreq = CalcFilterCutoffFreq(
          mv.mBreath01.GetFloatVal(),
          midiNote,
          mPreset->mFilterKeytracking,
          mPreset->mFilterMinFreq,
          mPreset->mFilterMaxFreq);

      mFilter.SetParams(mPreset->mFilterType, filterFreq, mPreset->mFilterQ, mPreset->mFilterSaturation);
      mFilter.EnableDCFilter(mPreset->mDCFilterEnabled, mPreset->mDCFilterCutoff);

      mPanner.SetPan(mv.mPan + mPreset->mPan);

      mRunningVoice = mv;
    }

    bool IsPlaying() const
    {
      // this function lets us delay for example, if there's a release stage (theoretically)
      return mRunningVoice.IsPlaying();
    }

    void Unassign()
    {
      mRunningVoice.mVoiceId = MAGIC_VOICE_ID_UNASSIGNED;
    }

    Voice(int16_t vid, AudioMixer4 &destLeft, int destPortLeft, AudioMixer4 &destRight, int destPortRight) : mPatchOsc1ToMix(mOsc, 0, mOscMixer, 0),
                                                          mPatchOsc2ToMix(mOsc, 1, mOscMixer, 1),
                                                          mPatchOsc3ToMix(mOsc, 2, mOscMixer, 2),
                                                          mPatchMixToFilter(mOscMixer, 0, mFilter, 0),
                                                          mPatchFilterToPanner(mFilter, 0, mPanner, 0),
                                                          mPatchOutLeft(mPanner, 0, destLeft, destPortLeft),
                                                          mPatchOutRight(mPanner, 1, destRight, destPortRight)
    {
    }
  };

  Voice gVoices[MAX_SYNTH_VOICES] =
      {
          {0, CCSynthGraph::voiceMix1Left, 0, CCSynthGraph::voiceMix1Right, 0},
          {1, CCSynthGraph::voiceMix1Left, 1, CCSynthGraph::voiceMix1Right, 1},
          {2, CCSynthGraph::voiceMix1Left, 2, CCSynthGraph::voiceMix1Right, 2},
          {3, CCSynthGraph::voiceMix1Left, 3, CCSynthGraph::voiceMix1Right, 3}, // 4
          {4, CCSynthGraph::voiceMix2Left, 0, CCSynthGraph::voiceMix2Right, 0},
          {5, CCSynthGraph::voiceMix2Left, 1, CCSynthGraph::voiceMix2Right, 1},
  };

  struct SynthGraphControl
  {
    float mPrevMetronomeBeatFrac = 0;
    AppSettings *mAppSettings;
    Metronome *mMetronome;

    // ...[delayLeft]---->[mDelayFilterLeft]--------------->[delayFeedbackAmpLeft]...
    //                                     \-------------->[waveMixerLeft]...
    ::clarinoid::FilterNode mDelayFilterLeft;
    CCPatch mPatchDelayToFilterLeft = { CCSynthGraph::delayLeft, 0, mDelayFilterLeft, 0 };
    CCPatch mPatchDelayFilterToFeedbackAmpLeft = { mDelayFilterLeft, 0, CCSynthGraph::delayFeedbackAmpLeft, 0 };
    CCPatch mPatchDelayFilterToWaveMixerLeft = { mDelayFilterLeft, 0, CCSynthGraph::waveMixerLeft, 0 };

    ::clarinoid::FilterNode mDelayFilterRight;
    CCPatch mPatchDelayToFilterRight = { CCSynthGraph::delayRight, 0, mDelayFilterRight, 0 };
    CCPatch mPatchDelayFilterToFeedbackAmpRight = { mDelayFilterRight, 0, CCSynthGraph::delayFeedbackAmpRight, 0 };
    CCPatch mPatchDelayFilterToWaveMixerRight = { mDelayFilterRight, 0, CCSynthGraph::waveMixerRight, 0 };

    void Setup(AppSettings *appSettings, Metronome *metronome, IModulationSourceSource* modulationSourceSource)
    {
      //AudioMemory(AUDIO_MEMORY_TO_ALLOCATE);
      AudioStream::initialize_memory(CLARINOID_AUDIO_MEMORY, SizeofStaticArray(CLARINOID_AUDIO_MEMORY));

      mAppSettings = appSettings;
      mMetronome = metronome;

      // for some reason patches really don't like to connect unless they are
      // last in the initialization order. Here's a workaround to force them to connect.
      for (auto &v : gVoices)
      {
        v.EnsurePatchConnections(appSettings, modulationSourceSource);
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
      AudioNoInterrupts();// https://www.pjrc.com/teensy/td_libs_AudioProcessorUsage.html
    }

    void EndUpdate()
    {
      AudioInterrupts();
    }

    void UpdatePostFx()
    {
      CCSynthGraph::delayFeedbackAmpLeft.gain(mAppSettings->mSynthSettings.mDelayFeedbackLevel);
      CCSynthGraph::delayFeedbackAmpRight.gain(mAppSettings->mSynthSettings.mDelayFeedbackLevel);
      CCSynthGraph::delayLeft.delay(0, mAppSettings->mSynthSettings.mDelayMS);
      CCSynthGraph::delayRight.delay(0, mAppSettings->mSynthSettings.mDelayMS + mAppSettings->mSynthSettings.mDelayStereoSep);

      mDelayFilterLeft.SetParams(mAppSettings->mSynthSettings.mDelayFilterType, mAppSettings->mSynthSettings.mDelayCutoffFrequency, mAppSettings->mSynthSettings.mDelayQ, mAppSettings->mSynthSettings.mDelaySaturation);
      mDelayFilterRight.SetParams(mAppSettings->mSynthSettings.mDelayFilterType, mAppSettings->mSynthSettings.mDelayCutoffFrequency, mAppSettings->mSynthSettings.mDelayQ, mAppSettings->mSynthSettings.mDelaySaturation);

      CCSynthGraph::verb.roomsize(mAppSettings->mSynthSettings.mReverbSize);
      CCSynthGraph::verb.damping(mAppSettings->mSynthSettings.mReverbDamping);

      CCSynthGraph::verbWetAmpLeft.gain(mAppSettings->mSynthSettings.mReverbGain);
      CCSynthGraph::verbWetAmpRight.gain(mAppSettings->mSynthSettings.mReverbGain);

      CCSynthGraph::ampLeft.gain(mAppSettings->mSynthSettings.mMasterGain);
      CCSynthGraph::ampRight.gain(mAppSettings->mSynthSettings.mMasterGain);

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
        { // beat boundary is when the frac drops back to 0
          CCSynthGraph::metronomeEnv.noteOn();
        }
        mPrevMetronomeBeatFrac = metronomeBeatFrac;
      }
    }
  };

  SynthGraphControl gSynthGraphControl;

} // namespace clarinoid
