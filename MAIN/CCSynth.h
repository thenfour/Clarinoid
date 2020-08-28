
#ifndef CCSYNTH_H
#define CCSYNTH_H

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// https://gitlab.com/flojawi/teensy-polyblep-oscillator/-/tree/master/polySynth
#include <polyBlepOscillator.h>

#include "Shared_CCUtil.h"

#include "CCSynthUtils.h"
#include "CCEWIControl.h"
namespace CCSynthGraph
{
/*
https://www.pjrc.com/teensy/gui/index.html?info=AudioEffectDelay
// this is the after-oscillator processing.

*/

// GUItool: begin automatically generated code
AudioMixer4              waveMixer;      //xy=840.0000114440918,416.25000762939453
AudioSynthWaveformSine   metronomeOsc;          //xy=842.1818466186523,757.1818141937256
AudioFilterStateVariable waveFilter;     //xy=1007.0000114440918,422.25000762939453
AudioEffectEnvelope      metronomeEnv;      //xy=1048.1818542480469,741.1818132400513
AudioEffectFreeverbStereo verb;           //xy=1176.0000457763672,350.2500247955322
AudioAmplifier           verbWetAmpLeft; //xy=1357.0000457763672,346.2500247955322
AudioAmplifier           verbWetAmpRight; //xy=1359.0000457763672,383.2500247955322
AudioMixer4              postMixerLeft;  //xy=1671.0000114440918,575.2500076293945
AudioMixer4              postMixerRight; //xy=1675.0000114440918,641.2500076293945
AudioAmplifier           ampRight;       //xy=1877.0000839233398,623.2500238418579
AudioAmplifier           ampLeft;        //xy=1878.000087738037,585.2500228881836
AudioOutputI2S           i2s1;           //xy=2065.999969482422,580.2500228881836
AudioConnection          patchCord18(metronomeOsc, metronomeEnv);
AudioConnection          patchCord17(waveMixer, 0, waveFilter, 0);
AudioConnection          patchCord19(waveFilter, 0, postMixerLeft, 1);
AudioConnection          patchCord20(waveFilter, 0, postMixerRight, 1);
AudioConnection          patchCord21(waveFilter, 0, verb, 0);
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

struct CCPatch : public AudioConnection
{
  CCPatch(AudioStream &source, uint8_t sourceOutput, AudioStream &destination, uint8_t destinationInput) :
    AudioConnection(source, sourceOutput, destination, destinationInput)
  {
  }
  
};

struct Voice
{
  AudioBandlimitedOsci osc;
  AudioMixer4 oscMix;
  CCPatch patch1;
  CCPatch patch2;
  CCPatch patch3;
  CCPatch patchOut;

  int16_t mVoiceId = -1;
  int16_t mMusicalVoiceId = -1;
  int16_t mSynthPatch = -1;
  int16_t mNote = -1;// currently playing note.
  Stopwatch mTimeSinceNoteOn;
  bool mIsCurrentlyPlaying = false;
  
  void UpdateStopped()
  {
    mMusicalVoiceId = -1;
    mSynthPatch = -1;
    mNote = -1;
    mIsCurrentlyPlaying = false;

    osc.amplitude(1, 0.0);
    osc.amplitude(2, 0.0);
    osc.amplitude(3, 0.0);
  }
  
  void UpdatePlaying(const CCEWIMusicalState& state, const MusicalVoice& mv)
  {
    float breathAdj = state.breath01.GetValue();

    bool voiceOrPatchChanged = (mMusicalVoiceId != mv.mVoiceId) || (mSynthPatch != mv.mSynthPatch);
    if (voiceOrPatchChanged) {
      // init synth patch.
      osc.waveform(1, 1); // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
      osc.waveform(2, 3);
      osc.waveform(3, 1);
    
      osc.pulseWidth(1, 0.5);
      osc.pulseWidth(2, 0.5);
      osc.pulseWidth(3, 0.5);
  
      osc.amplitude(1, 0.0);
      osc.amplitude(2, 0.3);
      osc.amplitude(3, 0.3);
      
      osc.addNote(); // this enables portamento. we never need to do note-offs because this synth just plays a continuous single note.
    }

    // reset timer if pretty much anything changed.
    if (!mIsCurrentlyPlaying || voiceOrPatchChanged || (mNote != mv.mNote)) {
      mTimeSinceNoteOn.Restart();
    }

    mMusicalVoiceId = mv.mVoiceId;
    mSynthPatch = mv.mSynthPatch;
    mNote = mv.mNote;
    mIsCurrentlyPlaying = true;

    // update
    float midiNote = (float)mv.mNote + state.pitchBendN11.GetValue() * 2;

    osc.portamentoTime(1, gAppSettings.mPortamentoTime);
    osc.portamentoTime(2, gAppSettings.mPortamentoTime);
    osc.portamentoTime(3, gAppSettings.mPortamentoTime);

    float freq = MIDINoteToFreq(midiNote);
    float freqSync = map(breathAdj, 0.0f, 1.0f, freq * 2, freq * 7);
  
    osc.frequency(1, freq);
    osc.frequency(2, freqSync);
    osc.frequency(3, freq);
  }

  Voice(int16_t vid, AudioMixer4& dest, int destPort) : 
    patch1(osc, 0, oscMix, 0),
    patch2(osc, 1, oscMix, 1),
    patch3(osc, 2, oscMix, 2),
    patchOut(oscMix, 0, dest, destPort),
    mVoiceId(vid)
  {
  }
};


struct VoiceList
{
  Voice mVoices[MAX_VOICES] =
  { 
    { 0, mix1, 0 },
    { 1, mix1, 1 },
    { 2, mix1, 2 },
    { 3, mix1, 3 }, // 4
    { 4, mix2, 0 },
    { 5, mix2, 1 },
    { 6, mix2, 2 },
    { 7, mix2, 3 }, // 8
//    { 8, mix3, 0 },
//    { 9, mix3, 1 },
//    { 10, mix3, 2 },
//    { 11, mix3, 3 }, // 12
//    { 12, mix4, 0 },
//    { 13, mix4, 1 },
//    { 14, mix4, 2 },
//    { 15, mix4, 3 },
  };

  AudioMixer4 mix1;
  AudioMixer4 mix2;
  AudioMixer4 mix3;
  AudioMixer4 mix4;
  AudioMixer4 mixOutp;

  AudioConnection patch1, patch2, patch3, patch4;
  AudioConnection patchOut;

  VoiceList(AudioMixer4& destMix, int destPort) :
    patch1(mix1, 0, mixOutp, 0),
    patch2(mix2, 0, mixOutp, 1),
    patch3(mix3, 0, mixOutp, 2),
    patch4(mix4, 0, mixOutp, 3),
    patchOut(mixOutp, 0, destMix, destPort)
  {
  }

  // returns a voice that's either already assigned to this voice, or the best one to free up for it.
  Voice* FindAssignedOrAvailable(int16_t musicalVoiceId) {
    Voice* firstFree = nullptr;
    for (auto& v : mVoices) {
      if (v.mMusicalVoiceId == musicalVoiceId) {
        return &v; // already assigned to this voice.
      }
      if (!firstFree && (v.mMusicalVoiceId == -1)) {
        firstFree = &v;
      }
    }
    if (firstFree) {
      return firstFree;
    }
    // no free voices. in this case find the oldest.
    // TODO.
    return &mVoices[0];
  }

  // returns null if this voice isn't assigned.
  Voice* FindAssigned(int16_t musicalVoiceId) {
    for (auto& v : mVoices) {
      if (v.mMusicalVoiceId == musicalVoiceId) {
        return &v;
      }
    }
    return nullptr;
  }
};


VoiceList gVoices(CCSynthGraph::waveMixer, 1);

class CCSynth : UpdateObjectT<ProfileObjectType::Synth>
{
public:

  CCThrottlerT<500> mMetronomeTimer;

  virtual void setup() {
    AudioMemory(5 + (MAX_VOICES * 2)); // rough estimate

    // for some reason patches really don't like to connect unless they are
    // last in the initialization order. Here's a workaround to force them to connect.
    for (auto& v : gVoices.mVoices) {
      v.patchOut.connect();
    }
    
    CCSynthGraph::audioShield.enable();
    CCSynthGraph::audioShield.volume(.7); // headphone vol
    CCSynthGraph::ampLeft.gain(.01);
    CCSynthGraph::ampRight.gain(.01);
    delay(200); // why?

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

  void Update(const CCEWIMusicalState& state) {
    // AudioNoInterrupts  https://www.pjrc.com/teensy/td_libs_AudioProcessorUsage.html
    AudioNoInterrupts();

    for (auto& mv : state.mMusicalVoices) {
      if (mv.mIsNoteCurrentlyOn) {
        Voice* pv = gVoices.FindAssignedOrAvailable(mv.mVoiceId);
        CCASSERT(!!pv);
        pv->UpdatePlaying(state, mv);
      } else {
        Voice* pv = gVoices.FindAssigned(mv.mVoiceId);
        if (pv) {
          pv->UpdateStopped();
        }
      }
    }

    float breathAdj = state.breath01.GetValue();

    CCSynthGraph::verb.roomsize(.6f);
    CCSynthGraph::verb.damping(.7f);
    CCSynthGraph::verbWetAmpLeft.gain(gAppSettings.mReverbGain);
    CCSynthGraph::verbWetAmpRight.gain(gAppSettings.mReverbGain);

    float filterFreq = map(breathAdj, 0.01, 1, 0, 15000);
    CCSynthGraph::waveFilter.frequency(filterFreq);

    if (!gAppSettings.mMetronomeOn) {
        CCSynthGraph::metronomeOsc.amplitude(0);      
    } else {
      CCSynthGraph::metronomeEnv.decay(gAppSettings.mMetronomeDecayMS);
      CCSynthGraph::metronomeOsc.amplitude(gAppSettings.mMetronomeGain);
      CCSynthGraph::metronomeOsc.frequency(MIDINoteToFreq(gAppSettings.mMetronomeNote));
      if (mMetronomeTimer.IsReady(60000.0f / gAppSettings.mBPM)) {
        CCSynthGraph::metronomeEnv.noteOn();
      }
    }

    AudioInterrupts();
  }
};



#endif
