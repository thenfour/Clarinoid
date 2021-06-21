
#pragma once

#ifdef CLARINOID_MODULE_TEST
#error not for x86 tests
#endif

#include <cfloat>

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/loopstation/LoopstationMemory.hpp>

#include "Patch.hpp"
#include "FilterNode.hpp"

namespace clarinoid
{

  namespace CCSynthGraph
  {
    /*
https://www.pjrc.com/teensy/gui/index.html
// this is the after-oscillator processing.

*/





// GUItool: begin automatically generated code
AudioMixer4              voiceMix2;      //xy=322.9147529602051,696.9999820971861
AudioMixer4              voiceMix4;      //xy=329.9147529602051,871.9999820971861
AudioMixer4              voiceMix1;      //xy=331.9147529602051,622.9999820971861
AudioMixer4              voiceMix3;      //xy=338.9147529602051,786.9999820971861
AudioMixer4              voiceMixOutp;   //xy=551.9147415161133,673.9999055862427
AudioMixer4              delayInputMixerLeft;         //xy=826.369384765625,238.00558948516846
AudioMixer4              delayInputMixerRight; //xy=860.5513305664062,516.0056533813477
AudioAmplifier           delayFeedbackAmpLeft;           //xy=899.3691177368164,93.00562858581543
AudioAmplifier           delayFeedbackAmpRight; //xy=956.5510635375977,378.00558853149414
AudioEffectDelay         delayLeft;         //xy=997.3692092895508,238.00556564331055
AudioEffectDelay         delayRight; //xy=1039.5512008666992,516.0056304931641
AudioMixer4              waveMixerLeft;      //xy=1362.914695739746,655.9999465942383
AudioMixer4              waveMixerRight; //xy=1367.3693618774414,733.0056781768799
AudioMixer4              verbInputMixer;         //xy=1583.3691940307617,534.0056552886963
AudioSynthWaveformSine   metronomeOsc;   //xy=1628.9147644042969,947.9999897265807
AudioEffectFreeverbStereo verb;           //xy=1742.9141845703125,524.9999160766602
AudioEffectEnvelope      metronomeEnv;   //xy=1829.9147644042969,949.9999897265807
AudioAmplifier           verbWetAmpLeft; //xy=1901.914405822754,505.9999418258667
AudioAmplifier           verbWetAmpRight; //xy=1908.9144248962402,543.9999570846558
AudioMixer4              postMixerLeft;  //xy=2083.914764404297,732.9999897265807
AudioMixer4              postMixerRight; //xy=2085.914764404297,815.9999897265807
AudioAmplifier           ampLeft;        //xy=2245.914810180664,733.0000047683716
AudioAmplifier           ampRight;       //xy=2248.9147872924805,814.9999914169312
AudioOutputI2S           i2s1;           //xy=2430.914936065674,766.9999485015869
AudioAnalyzePeak         peak1;          //xy=2473.9147872924805,545.999963760376
AudioConnection          patchCord1(voiceMix2, 0, voiceMixOutp, 1);
AudioConnection          patchCord2(voiceMix4, 0, voiceMixOutp, 3);
AudioConnection          patchCord3(voiceMix1, 0, voiceMixOutp, 0);
AudioConnection          patchCord4(voiceMix3, 0, voiceMixOutp, 2);
AudioConnection          patchCord5(voiceMixOutp, 0, waveMixerLeft, 1);
AudioConnection          patchCord6(voiceMixOutp, 0, delayInputMixerLeft, 3);
AudioConnection          patchCord7(voiceMixOutp, 0, delayInputMixerRight, 1);
AudioConnection          patchCord8(voiceMixOutp, 0, waveMixerRight, 1);
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
    AudioBandlimitedOsci mOsc;
    CCPatch mPatchOsc1ToMix;
    CCPatch mPatchOsc2ToMix;
    CCPatch mPatchOsc3ToMix;
    AudioMixer4 mOscMixer;     // mixes down the 3 oscillators
    CCPatch mPatchMixToFilter; // then into filter.
    ::clarinoid::FilterNode mFilter;
    CCPatch mPatchOut;

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
      mPatchOut.connect();
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

      if (mv.IsPlaying() && !mv.mIsNoteCurrentlyMuted)
      {
        mOsc.amplitude(1, GetModulatedValue(mPreset->mOsc1Gain, ModulationDestination::Osc1Volume));
        mOsc.amplitude(2, GetModulatedValue(mPreset->mOsc2Gain, ModulationDestination::Osc2Volume));
        mOsc.amplitude(3, GetModulatedValue(mPreset->mOsc3Gain, ModulationDestination::Osc3Volume));
      }
      else
      {
        mOsc.amplitude(1, 0.0);
        mOsc.amplitude(2, 0.0);
        mOsc.amplitude(3, 0.0);
      }

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

    Voice(int16_t vid, AudioMixer4 &dest, int destPort) : mPatchOsc1ToMix(mOsc, 0, mOscMixer, 0),
                                                          mPatchOsc2ToMix(mOsc, 1, mOscMixer, 1),
                                                          mPatchOsc3ToMix(mOsc, 2, mOscMixer, 2),
                                                          mPatchMixToFilter(mOscMixer, 0, mFilter, 0),
                                                          mPatchOut(mFilter, 0, dest, destPort) //,
    {
    }
  };

  Voice gVoices[MAX_SYNTH_VOICES] =
      {
          {0, CCSynthGraph::voiceMix1, 0},
          {1, CCSynthGraph::voiceMix1, 1},
          {2, CCSynthGraph::voiceMix1, 2},
          {3, CCSynthGraph::voiceMix1, 3}, // 4
          {4, CCSynthGraph::voiceMix2, 0},
          {5, CCSynthGraph::voiceMix2, 1},
          // { 6, CCSynthGraph::voiceMix2, 2 },
          //{ 7, CCSynthGraph::voiceMix2, 3 }, // 8
          //    { 8, mix3, 0 },
          //    { 9, mix3, 1 },
          //    { 10, mix3, 2 },
          //    { 11, mix3, 3 }, // 12
          //    { 12, mix4, 0 },
          //    { 13, mix4, 1 },
          //    { 14, mix4, 2 },
          //    { 15, mix4, 3 },
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
