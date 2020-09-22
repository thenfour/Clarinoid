
#pragma once

#ifdef CLARINOID_MODULE_TEST
#error not for x86 tests
#endif

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/loopstation/LoopstationMemory.hpp>

#include "Patch.hpp"

namespace CCSynthGraph
{
/*
https://www.pjrc.com/teensy/gui/index.html?info=AudioEffectDelay
// this is the after-oscillator processing.

*/

// GUItool: begin automatically generated code
AudioMixer4              waveMixer;      //xy=840.0000114440918,416.25000762939453
AudioSynthWaveformSine   metronomeOsc;          //xy=842.1818466186523,757.1818141937256
AudioEffectEnvelope      metronomeEnv;      //xy=1048.1818542480469,741.1818132400513
AudioEffectFreeverbStereo verb;           //xy=1176.0000457763672,350.2500247955322
AudioAmplifier           verbWetAmpLeft; //xy=1357.0000457763672,346.2500247955322
AudioAmplifier           verbWetAmpRight; //xy=1359.0000457763672,383.2500247955322
AudioMixer4              postMixerLeft;  //xy=1671.0000114440918,575.2500076293945
AudioMixer4              postMixerRight; //xy=1675.0000114440918,641.2500076293945
AudioAmplifier           ampRight;       //xy=1877.0000839233398,623.2500238418579
AudioAmplifier           ampLeft;        //xy=1878.000087738037,585.2500228881836
AudioOutputI2S           i2s1;           //xy=2065.999969482422,580.2500228881836
AudioMixer4 voiceMix1;
AudioMixer4 voiceMix2;
AudioMixer4 voiceMix3;
AudioMixer4 voiceMix4;
AudioMixer4 voiceMixOutp;
AudioConnection voiceMixPatch1(voiceMix1, 0, voiceMixOutp, 0);
AudioConnection voiceMixPatch2(voiceMix2, 0, voiceMixOutp, 1);
AudioConnection voiceMixPatch3(voiceMix3, 0, voiceMixOutp, 2);
AudioConnection voiceMixPatch4(voiceMix4, 0, voiceMixOutp, 3);
AudioConnection voiceMixPatchOut(voiceMixOutp, 0, waveMixer, 1);
AudioConnection          patchCord18(metronomeOsc, metronomeEnv);
AudioConnection          patchCord19(waveMixer, 0, postMixerLeft, 1);
AudioConnection          patchCord20(waveMixer, 0, postMixerRight, 1);
AudioConnection          patchCord21(waveMixer, 0, verb, 0);
AudioConnection          patchCord22(metronomeEnv, 0, postMixerLeft, 2);
AudioConnection          patchCord23(metronomeEnv, 0, postMixerRight, 2);
AudioConnection          patchCord24(verb, 0, verbWetAmpLeft, 0);
AudioConnection          patchCord25(verb, 1, verbWetAmpRight, 0);
AudioConnection          patchCord26(verbWetAmpLeft, 0, postMixerLeft, 0);
AudioConnection          patchCord27(verbWetAmpRight, 0, postMixerRight, 0);
AudioConnection          patchCord28(postMixerLeft, ampLeft);
AudioConnection          patchCord29(postMixerRight, ampRight);
AudioConnection          patchCord30(ampRight, 0, i2s1, 1);
AudioConnection          patchCord32(ampLeft, 0, i2s1, 0);
AudioControlSGTL5000     audioShield;    //xy=1944.0000114440918,536.2500076293945
// GUItool: end automatically generated code

}

struct Voice
{
  AudioBandlimitedOsci mOsc;
  CCPatch mPatchOsc1ToMix;
  CCPatch mPatchOsc2ToMix;
  CCPatch mPatchOsc3ToMix;
  AudioMixer4 mOscMixer; // mixes down the 3 oscillators
  CCPatch mPatchMixToFilter; // then into filter.
  AudioFilterStateVariable mFilter;
  CCPatch mPatchOut;

  MusicalVoice mRunningVoice;

  void EnsurePatchConnections()
  {
    mPatchOsc1ToMix.connect();
    mPatchOsc2ToMix.connect();
    mPatchOsc3ToMix.connect();
    mPatchMixToFilter.connect();
    mPatchOut.connect();
  }
  
  void Update(const MusicalVoice& mv)
  {
    bool voiceOrPatchChanged = (mRunningVoice.mVoiceId != mv.mVoiceId) || (mRunningVoice.mSynthPatch != mv.mSynthPatch);
    if (voiceOrPatchChanged) {
      // init synth patch.
      mOsc.waveform(1, 1); // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
      mOsc.waveform(2, 3);
      mOsc.waveform(3, 1);
    
      mOsc.pulseWidth(1, 0.5);
      mOsc.pulseWidth(2, 0.5);
      mOsc.pulseWidth(3, 0.5);
  
      mOsc.removeNote();
      mOsc.addNote(); // this enables portamento. we never need to do note-offs because this synth just plays a continuous single note.
    }

    if (mv.IsPlaying()) {
      mOsc.amplitude(1, 0.0);
      mOsc.amplitude(2, 0.3);
      mOsc.amplitude(3, 0.3);
    } else {
      mOsc.amplitude(1, 0.0);
      mOsc.amplitude(2, 0.0);
      mOsc.amplitude(3, 0.0);
    }

    auto transition = CalculateTransitionEvents(mRunningVoice, mv);
    if (transition.mNeedsNoteOn) {
      mOsc.addNote(); // this enables portamento. we never need to do note-offs because this synth just plays a continuous single note.
    }
    else if (transition.mNeedsNoteOff) {
      mOsc.removeNote();
    }

    // update
    float midiNote = (float)mv.mMidiNote + mv.mPitchBendN11.GetFloatVal() * pitchBendRange;

    mOsc.portamentoTime(1, gAppSettings.mPortamentoTime);
    mOsc.portamentoTime(2, gAppSettings.mPortamentoTime);
    mOsc.portamentoTime(3, gAppSettings.mPortamentoTime);

    float freq = MIDINoteToFreq(midiNote);
    float freqSync = map(mv.mBreath01.GetFloatVal(), 0.0f, 1.0f, freq * 2, freq * 7);
  
    mOsc.frequency(1, freq);
    mOsc.frequency(2, freqSync);
    mOsc.frequency(3, freq);

    float filterFreq = map(mv.mBreath01.GetFloatVal(), 0.01, 1, 0, 15000);
    mFilter.frequency(filterFreq);

    mRunningVoice = mv;
  }

  bool IsPlaying() const {
    // this function lets us delay for example, if there's a release stage (theoretically)
    return mRunningVoice.IsPlaying();
  }

  void Unassign() { 
    mRunningVoice.mVoiceId = MAGIC_VOICE_ID_UNASSIGNED;
  }

  Voice(int16_t vid, AudioMixer4& dest, int destPort) : 
    mPatchOsc1ToMix(mOsc, 0, mOscMixer, 0),
    mPatchOsc2ToMix(mOsc, 1, mOscMixer, 1),
    mPatchOsc3ToMix(mOsc, 2, mOscMixer, 2),
    mPatchMixToFilter(mOscMixer, 0, mFilter, 0),
    mPatchOut(mFilter, 0, dest, destPort)//,
  {
  }
};





  Voice gVoices[MAX_SYNTH_VOICES] =
  { 
    { 0, CCSynthGraph::voiceMix1, 0 },
    { 1, CCSynthGraph::voiceMix1, 1 },
    { 2, CCSynthGraph::voiceMix1, 2 },
    { 3, CCSynthGraph::voiceMix1, 3 }, // 4
    { 4, CCSynthGraph::voiceMix2, 0 },
    { 5, CCSynthGraph::voiceMix2, 1 },
    { 6, CCSynthGraph::voiceMix2, 2 },
    { 7, CCSynthGraph::voiceMix2, 3 }, // 8
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

  void Setup()
  {
    //AudioMemory(AUDIO_MEMORY_TO_ALLOCATE);
    AudioStream::initialize_memory(gLoopstationMemory.gAudioMemory, SizeofStaticArray(gLoopstationMemory.gAudioMemory));

    // for some reason patches really don't like to connect unless they are
    // last in the initialization order. Here's a workaround to force them to connect.
    for (auto& v : gVoices) {
      v.EnsurePatchConnections();
    }
    
    CCSynthGraph::audioShield.enable();
    CCSynthGraph::audioShield.volume(.7); // headphone vol
    CCSynthGraph::ampLeft.gain(.01);
    CCSynthGraph::ampRight.gain(.01);
    delay(100); // why?

    CCSynthGraph::metronomeEnv.delay(0);
    CCSynthGraph::metronomeEnv.attack(0);
    CCSynthGraph::metronomeEnv.hold(0);
    CCSynthGraph::metronomeEnv.releaseNoteOn(0);
    CCSynthGraph::metronomeEnv.sustain(0);
  }

  void SetGain(float f) {
    CCSynthGraph::ampLeft.gain(f);
    CCSynthGraph::ampRight.gain(f);
  }

  void BeginUpdate() {
    AudioNoInterrupts();// https://www.pjrc.com/teensy/td_libs_AudioProcessorUsage.html
  }

  void EndUpdate() {
    AudioInterrupts();
  }

  void UpdatePostFx() {
    CCSynthGraph::verb.roomsize(.6f);
    CCSynthGraph::verb.damping(.7f);
    CCSynthGraph::verbWetAmpLeft.gain(gAppSettings.mReverbGain);
    CCSynthGraph::verbWetAmpRight.gain(gAppSettings.mReverbGain);

    if (!gAppSettings.mMetronomeOn) {
      CCSynthGraph::metronomeOsc.amplitude(0);
    } else {
      CCSynthGraph::metronomeEnv.decay(gAppSettings.mMetronomeDecayMS);
      CCSynthGraph::metronomeOsc.amplitude(gAppSettings.mMetronomeGain);
      CCSynthGraph::metronomeOsc.frequency(MIDINoteToFreq(gAppSettings.mMetronomeNote));

      float metronomeBeatFrac = gMetronome.GetBeatFrac();
      if (metronomeBeatFrac < mPrevMetronomeBeatFrac) {// beat boundary is when the frac drops back to 0
        CCSynthGraph::metronomeEnv.noteOn();
      }
      mPrevMetronomeBeatFrac = metronomeBeatFrac;
    }
  }
};

SynthGraphControl gSynthGraphControl;
