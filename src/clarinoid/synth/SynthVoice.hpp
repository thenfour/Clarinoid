
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
AudioAmplifier           delayFeedbackAmpLeft; //xy=2906.6818466186523,634.6817741394043
AudioEffectDelay         delayLeft;      //xy=2908.681854248047,725.6817779541016
AudioEffectDelay         delayRight;     //xy=2918.6816749572754,955.6817398071289
AudioAmplifier           delayFeedbackAmpRight; //xy=2919.681671142578,867.6817388534546
AudioMixer4              verbInputMixer; //xy=3291.4318046569824,1025.6818313598633
AudioSynthWaveformSine   metronomeOsc;   //xy=3327.6817626953125,1454.6817626953125
AudioEffectFreeverbStereo verb;           //xy=3441.6817626953125,1031.6817626953125
AudioEffectEnvelope      metronomeEnv;   //xy=3528.6817626953125,1456.6817626953125
AudioAmplifier           verbWetAmpLeft; //xy=3600.6817626953125,1012.6817626953125
AudioAmplifier           verbWetAmpRight; //xy=3607.6817626953125,1050.6817626953125
AudioMixer4              postMixerLeft;  //xy=3782.6817626953125,1239.6817626953125
AudioMixer4              postMixerRight; //xy=3784.6817626953125,1322.6817626953125
AudioAmplifier           ampLeft;        //xy=3944.6817626953125,1240.6817626953125
AudioAmplifier           ampRight;       //xy=3947.6817626953125,1321.6817626953125
AudioOutputI2S           i2s1;           //xy=4129.6817626953125,1273.6817626953125
AudioAnalyzePeak         peak1;          //xy=4172.6817626953125,1052.6817626953125
AudioConnection          patchCord1(verbInputMixer, verb);
AudioConnection          patchCord2(metronomeOsc, metronomeEnv);
AudioConnection          patchCord3(verb, 0, verbWetAmpLeft, 0);
AudioConnection          patchCord4(verb, 1, verbWetAmpRight, 0);
AudioConnection          patchCord5(metronomeEnv, 0, postMixerRight, 3);
AudioConnection          patchCord6(metronomeEnv, 0, postMixerLeft, 3);
AudioConnection          patchCord7(verbWetAmpLeft, 0, postMixerLeft, 2);
AudioConnection          patchCord8(verbWetAmpRight, 0, postMixerRight, 2);
AudioConnection          patchCord9(postMixerLeft, ampLeft);
AudioConnection          patchCord10(postMixerRight, ampRight);
//AudioConnection          patchCord11(ampLeft, peak1);
//AudioConnection          patchCord11(verbInputMixer, peak1);
AudioConnection          patchCord12(ampLeft, 0, i2s1, 1);
AudioConnection          patchCord13(ampRight, 0, i2s1, 0);
// GUItool: end automatically generated code


    // insert the delay filters.
    // ...[delayLeft]---->[delayFilterLeft]--------------->[delayFeedbackAmpLeft]...
    //                                     \-------------->[waveMixerLeft]...
    ::clarinoid::FilterNode delayFilterLeft;
    AudioConnection mPatchDelayToFilterLeft = { delayLeft, 0, delayFilterLeft, 0 };
    AudioConnection mPatchDelayFilterToFeedbackAmpLeft = { delayFilterLeft, 0, delayFeedbackAmpLeft, 0 };

    ::clarinoid::FilterNode delayFilterRight;
    AudioConnection mPatchDelayToFilterRight = { delayRight, 0, delayFilterRight, 0 };
    AudioConnection mPatchDelayFilterToFeedbackAmpRight = { delayFilterRight, 0, delayFeedbackAmpRight, 0 };

    // delay output connection
    AudioConnection mPatchDelayFilterToWaveMixerLeft = { delayFilterLeft, 0, postMixerLeft, 1 };
    AudioConnection mPatchDelayFilterToWaveMixerRight = { delayFilterRight, 0, postMixerRight, 1 };

    // voice mixer & dry output connection
    ::clarinoid::MultiMixer<MAX_SYNTH_VOICES> voiceMixerDryLeft; // all voices input here.
    ::clarinoid::MultiMixer<MAX_SYNTH_VOICES> voiceMixerDryRight; // all voices input here.
    AudioConnection patchVoicesDryToOutpLeft = { voiceMixerDryLeft, 0, postMixerLeft, 0 };
    AudioConnection patchVoicesDryToOutpRight = { voiceMixerDryRight, 0, postMixerRight, 0 };

    // delay input mix
    ::clarinoid::MultiMixer<MAX_SYNTH_VOICES + 1> delayInputMixerLeft; // +1 to account for the delay feedback signal
    ::clarinoid::MultiMixer<MAX_SYNTH_VOICES + 1> delayInputMixerRight; // +1 to account for the delay feedback signal
    AudioConnection patchDelayInputMixToDelayL = { delayInputMixerLeft, 0, delayLeft, 0 };
    AudioConnection patchDelayInputMixToDelayR = { delayInputMixerRight, 0, delayRight, 0 };

    // delay fb
    AudioConnection patchDelayFBBackToInputLeft = { delayFeedbackAmpLeft, 0, delayInputMixerLeft, MAX_SYNTH_VOICES };
    AudioConnection patchDelayFBBackToInputRight = { delayFeedbackAmpRight, 0, delayInputMixerRight, MAX_SYNTH_VOICES };

    // verb input mix
    ::clarinoid::MultiMixer<MAX_SYNTH_VOICES + 1> verbInputMixerLeft; // all voices input here + 1 for delay line
    ::clarinoid::MultiMixer<MAX_SYNTH_VOICES + 1> verbInputMixerRight; // all voices input here + 1 for delay line
    AudioConnection patchVerbInputToVerbL = { verbInputMixerLeft, 0, verbInputMixer, 0 };
    AudioConnection patchVerbInputToVerbR = { verbInputMixerRight, 0, verbInputMixer, 1 };

    // delay verb
    AudioConnection patchDelayVerbInputLeft = { delayFilterLeft, 0, verbInputMixerLeft, MAX_SYNTH_VOICES };
    AudioConnection patchDelayVerbInputRight = { delayFilterRight, 0, verbInputMixerRight, MAX_SYNTH_VOICES };

//AudioConnection          patchCord11(verbInputMixerLeft, peak1);

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

    CCPatch mPatchFilterToPannerSplitter;
    ::clarinoid::GainAndPanSplitterNode<3> mPannerSplitter; // handles panning and send levels to delay/verb/dry

    CCPatch mPatchOutDryLeft;
    CCPatch mPatchOutDryRight;

    CCPatch mPatchOutDelayLeft;
    CCPatch mPatchOutDelayRight;

    CCPatch mPatchOutVerbLeft;
    CCPatch mPatchOutVerbRight;

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
      mPatchFilterToPannerSplitter.connect();

      mPatchOutDryLeft.connect();
      mPatchOutDryRight.connect();
      mPatchOutDelayLeft.connect();
      mPatchOutDelayRight.connect();
      mPatchOutVerbLeft.connect();
      mPatchOutVerbRight.connect();
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

      mPannerSplitter.SetPanAndGain(0, 1.0f, mv.mPan + mPreset->mPan);
      mPannerSplitter.SetPanAndGain(1, mPreset->mDelaySend, mv.mPan + mPreset->mPan);
      mPannerSplitter.SetPanAndGain(2, mPreset->mVerbSend, mv.mPan + mPreset->mPan);

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

    Voice(int16_t vid) :
        mPatchOsc1ToMix(mOsc, 0, mOscMixer, 0),
        mPatchOsc2ToMix(mOsc, 1, mOscMixer, 1),
        mPatchOsc3ToMix(mOsc, 2, mOscMixer, 2),
        mPatchMixToFilter(mOscMixer, 0, mFilter, 0),
        mPatchFilterToPannerSplitter(mFilter, 0, mPannerSplitter, 0),
        mPatchOutDryLeft(mPannerSplitter, 0, CCSynthGraph::voiceMixerDryLeft, vid),
        mPatchOutDryRight(mPannerSplitter, 1, CCSynthGraph::voiceMixerDryRight, vid),
        mPatchOutDelayLeft(mPannerSplitter, 2, CCSynthGraph::delayInputMixerLeft, vid),
        mPatchOutDelayRight(mPannerSplitter, 3, CCSynthGraph::delayInputMixerRight, vid),
        mPatchOutVerbLeft(mPannerSplitter, 4, CCSynthGraph::verbInputMixerLeft, vid),
        mPatchOutVerbRight(mPannerSplitter, 5, CCSynthGraph::verbInputMixerLeft, vid)
    {
    }
  };

  Voice gVoices[MAX_SYNTH_VOICES] =
      {
          {0},
          {1},
          {2},
          {3},
          {4},
          {5},
  };

  struct SynthGraphControl
  {
    float mPrevMetronomeBeatFrac = 0;
    AppSettings *mAppSettings;
    Metronome *mMetronome;

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

      CCSynthGraph::delayFilterLeft.SetParams(mAppSettings->mSynthSettings.mDelayFilterType, mAppSettings->mSynthSettings.mDelayCutoffFrequency, mAppSettings->mSynthSettings.mDelayQ, mAppSettings->mSynthSettings.mDelaySaturation);
      CCSynthGraph::delayFilterRight.SetParams(mAppSettings->mSynthSettings.mDelayFilterType, mAppSettings->mSynthSettings.mDelayCutoffFrequency, mAppSettings->mSynthSettings.mDelayQ, mAppSettings->mSynthSettings.mDelaySaturation);

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
