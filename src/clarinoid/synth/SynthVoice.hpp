
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
    AudioMixer4 voiceMix2;               //xy=787.2500190734863,825.0000057220459
    AudioMixer4 voiceMix4;               //xy=794.7500190734863,1000.0000171661377
    AudioMixer4 voiceMix1;               //xy=796.0000190734863,751.250020980835
    AudioMixer4 voiceMix3;               //xy=803.5000267028809,915.0000076293945
    AudioMixer4 voiceMixOutp;            //xy=1067.2500267028809,872.5000228881836
    AudioMixer4 waveMixer;               //xy=1289.7500343322754,873.5000228881836
    AudioEffectFreeverbStereo verb;      //xy=1459.50004196167,777.5000190734863
    AudioSynthWaveformSine metronomeOsc; //xy=1478.0000381469727,964.5001258850098
    AudioAmplifier verbWetAmpLeft;       //xy=1654.2500457763672,774.7500228881836
    AudioEffectEnvelope metronomeEnv;    //xy=1679.0000457763672,966.0000286102295
    AudioMixer4 postMixerLeft;           //xy=1942,885
    AudioAmplifier ampLeft;              //xy=2151.5000648498535,891.2500267028809
    AudioOutputI2S i2s1;                 //xy=2389.750068664551,901.2500267028809
    AudioAnalyzePeak peak1;              //xy=2393.500068664551,1005.0000290870667
    AudioConnection patchCord1(voiceMix2, 0, voiceMixOutp, 1);
    AudioConnection patchCord2(voiceMix4, 0, voiceMixOutp, 3);
    AudioConnection patchCord3(voiceMix1, 0, voiceMixOutp, 0);
    AudioConnection patchCord4(voiceMix3, 0, voiceMixOutp, 2);
    AudioConnection patchCord5(voiceMixOutp, 0, waveMixer, 1);
    AudioConnection patchCord6(waveMixer, 0, postMixerLeft, 1);
    AudioConnection patchCord7(waveMixer, verb);
    AudioConnection patchCord8(verb, 0, verbWetAmpLeft, 0);
    AudioConnection patchCord9(metronomeOsc, metronomeEnv);
    AudioConnection patchCord10(verbWetAmpLeft, 0, postMixerLeft, 0);
    AudioConnection patchCord11(metronomeEnv, 0, postMixerLeft, 2);
    AudioConnection patchCord12(postMixerLeft, ampLeft);
    AudioConnection patchCord13(ampLeft, 0, i2s1, 0);
    AudioConnection patchCord14(ampLeft, peak1);
    AudioConnection patchCord16(ampLeft, 0, i2s1, 1);
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

      if (mv.IsPlaying())
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

      //CCSynthGraph::audioShield.enable();
      //CCSynthGraph::audioShield.volume(.9); // headphone vol
      CCSynthGraph::ampLeft.gain(1);
      //CCSynthGraph::ampRight.gain(.9);
      //delay(100); // why?

      CCSynthGraph::metronomeEnv.delay(0);
      CCSynthGraph::metronomeEnv.attack(0);
      CCSynthGraph::metronomeEnv.hold(0);
      CCSynthGraph::metronomeEnv.releaseNoteOn(0);
      CCSynthGraph::metronomeEnv.sustain(0);
    }

    // void SetGain(float f) {
    //   //Serial.println(String("SetGain: ") + f);
    //   CCSynthGraph::ampLeft.gain(f);
    //   //CCSynthGraph::ampRight.gain(f);
    // }

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
      CCSynthGraph::verb.roomsize(.6f);
      CCSynthGraph::verb.damping(.7f);
      CCSynthGraph::verbWetAmpLeft.gain(mAppSettings->mSynthSettings.mReverbGain);
      CCSynthGraph::ampLeft.gain(mAppSettings->mSynthSettings.mMasterGain);

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
