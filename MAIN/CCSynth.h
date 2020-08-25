
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


struct Voice
{
  AudioBandlimitedOsci            osc;           //xy=106.25003051757812,135.00001525878906
  AudioMixer4              oscMix;         //xy=645.0000457763672,337.5000190734863
  AudioConnection patch1;
  AudioConnection patch2;
  AudioConnection patch3;
  Voice(AudioMixer4& dest, int destPort) : 
    patch1(osc, 0, oscMix, 0),
    patch2(osc, 1, oscMix, 1),
    patch3(osc, 2, oscMix, 2),
    patchOut(oscMix, 0, dest, destPort)
  {
  }
  AudioConnection patchOut;
};


struct VoiceList
{
  Voice voices[MAX_VOICES] =
  { 
    { mix1, 0 },
    { mix1, 1 },
    { mix1, 2 },
    { mix1, 3 },
//    { mix2, 0 },
//    { mix2, 1 },
//    { mix2, 2 },
//    { mix2, 3 },
//    { mix3, 0 },
//    { mix3, 1 },
//    { mix3, 2 },
//    { mix3, 3 },
//    { mix4, 0 },
//    { mix4, 1 },
//    { mix4, 2 },
//    { mix4, 3 },
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
    for (size_t i = 1; i < SizeofStaticArray(voices); ++ i) {
//      // disable all but 1 layer
      voices[i].patchOut.disconnect();
    }
  }
};


VoiceList gVoices(CCSynthGraph::waveMixer, 1);

//AudioSynthWaveform   testOsc;
//AudioConnection testPatchOut(testOsc, 0, CCSynthGraph::waveMixer, 3);


class CCSynth : IUpdateObject
{
public:

  CCThrottlerT<500> mMetronomeTimer;

  virtual void setup() {
    AudioMemory(12);
    CCSynthGraph::audioShield.enable();
    CCSynthGraph::audioShield.volume(.7); // headphone vol
    CCSynthGraph::ampLeft.gain(.01);
    CCSynthGraph::ampRight.gain(.01);
    delay(200); // why?

//    //testOsc.amplitude(.2);
//    testOsc.begin(WAVEFORM_SAWTOOTH);
//    testOsc.frequency(220);

//    for (int i = 0; i < HARMONIZER_VOICES; ++ i) {
//      gLayers.layers[0].voices[i].osc.amplitude(1, .3);
//      gLayers.layers[0].voices[i].osc.amplitude(2, .3);
//      gLayers.layers[0].voices[i].osc.amplitude(3, .3);
//    }

    auto& liveVoice = gVoices.voices[0];

    // voice 1 setup
    {
      liveVoice.osc.waveform(1, 1); // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
      liveVoice.osc.waveform(2, 3);
      liveVoice.osc.waveform(3, 1);
    
      liveVoice.osc.pulseWidth(1, 0.5);
      liveVoice.osc.pulseWidth(2, 0.5);
      liveVoice.osc.pulseWidth(3, 0.5);
  
      liveVoice.osc.amplitude(1, 0.0);
      liveVoice.osc.amplitude(2, 0.3);
      liveVoice.osc.amplitude(3, 0.3);
      
      liveVoice.osc.addNote(); // this enables portamento. we never need to do note-offs because this synth just plays a continuous single note.
    }
//
//    // voice 2 setup
//    {
//      CCSynthGraph::voice2.waveform(1, 1); // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync.
//      CCSynthGraph::voice2.waveform(2, 1);
//      //CCSynthGraph::voice2.waveform(3, 1);
//  
//      CCSynthGraph::voice2.pulseWidth(1, 0.0);
//      CCSynthGraph::voice2.pulseWidth(2, 0.0);
//      //CCSynthGraph::voice2.pulseWidth(3, 0.0);
//  
//      CCSynthGraph::voice2.amplitude(1, 0.2);
//      CCSynthGraph::voice2.amplitude(2, 0.2);
//      CCSynthGraph::voice2.amplitude(3, 0.0);
//  
//      CCSynthGraph::voice2.addNote(); // this enables portamento. we never need to do note-offs because this synth just plays a continuous single note.
//
//    }
//
//    // voice 3 setup
//    {
//      CCSynthGraph::voice3.waveform(1, 1); // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync.
//      CCSynthGraph::voice3.waveform(2, 1);
//      CCSynthGraph::voice3.pulseWidth(1, 0.0);
//      CCSynthGraph::voice3.pulseWidth(2, 0.0);
//      CCSynthGraph::voice3.addNote(); // this enables portamento. we never need to do note-offs because this synth just plays a continuous single note.
//    }
//
//    // voice 4 setup
//    {
//      CCSynthGraph::voice4.waveform(1, 1); // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync.
//      CCSynthGraph::voice4.waveform(2, 1);
//      CCSynthGraph::voice4.pulseWidth(1, 0.0);
//      CCSynthGraph::voice4.pulseWidth(2, 0.0);
//      CCSynthGraph::voice4.addNote(); // this enables portamento. we never need to do note-offs because this synth just plays a continuous single note.
//    }

    CCSynthGraph::metronomeEnv.delay(0);
    CCSynthGraph::metronomeEnv.attack(0);
    CCSynthGraph::metronomeEnv.hold(0);
    CCSynthGraph::metronomeEnv.releaseNoteOn(0);
    CCSynthGraph::metronomeEnv.sustain(0);

    //CCSynthGraph::waveFilter.resonance(1.3);
  }

  void SetGain(float f) {
    CCSynthGraph::ampLeft.gain(f);
    CCSynthGraph::ampRight.gain(f);
  }

  void Update(const CCEWIMusicalState& state) {
    // AudioNoInterrupts  https://www.pjrc.com/teensy/td_libs_AudioProcessorUsage.html
    AudioNoInterrupts();

    float breathAdj = state.breath01.GetValue();
    float midiNote = state.MIDINote + state.pitchBendN11.GetValue() * 2;

    CCSynthGraph::verb.roomsize(.6f);
    CCSynthGraph::verb.damping(.7f);
    CCSynthGraph::verbWetAmpLeft.gain(gAppSettings.mReverbGain);
    CCSynthGraph::verbWetAmpRight.gain(gAppSettings.mReverbGain);
    //CCPlot(gAppSettings.mReverbGain);

    auto& liveVoice = gVoices.voices[0];

    liveVoice.osc.portamentoTime(1, gAppSettings.mPortamentoTime);
    liveVoice.osc.portamentoTime(2, gAppSettings.mPortamentoTime);
    liveVoice.osc.portamentoTime(3, gAppSettings.mPortamentoTime);


    // voice 1
    {
      float freq = MIDINoteToFreq(midiNote);
      //float freq2 = gAppSettings.mHarmonizerOn > 0 ? MIDINoteToFreq(midiNote + 7) : freq;
      float freqSync = map(breathAdj, 0.0f, 1.0f, freq * 2, freq * 7);
    
      liveVoice.osc.frequency(1, freq);
      liveVoice.osc.frequency(2, freqSync);
      liveVoice.osc.frequency(3, freq);
    }
//
//    // voice 2
//    {
//      float freq = MIDINoteToFreq(midiNote);
//      CCSynthGraph::voice2.frequency(1, freq);
//      CCSynthGraph::voice2.frequency(2, freq * .995);
//    }
//
//    // voice 3
//    {
//      float freq = MIDINoteToFreq(midiNote - 4);
//      CCSynthGraph::voice3.frequency(1, freq);
//      CCSynthGraph::voice3.frequency(2, freq);
//    }
//
//    // voice 4
//    {
//      float freq = MIDINoteToFreq(midiNote - 11);
//      CCSynthGraph::voice4.frequency(1, freq);
//      CCSynthGraph::voice4.frequency(2, freq);
//    }

    float filterFreq = map(breathAdj, 0.01, 1, 0, 15000);
    //filterFreq = 1500;
    CCSynthGraph::waveFilter.frequency(filterFreq);

    if (!gAppSettings.mMetronomeOn) {
        CCSynthGraph::metronomeOsc.amplitude(0);      
    } else {
      CCSynthGraph::metronomeEnv.decay(gAppSettings.mMetronomeDecayMS);
      CCSynthGraph::metronomeOsc.amplitude(gAppSettings.mMetronomeGain);
      CCSynthGraph::metronomeOsc.frequency(MIDINoteToFreq(gAppSettings.mMetronomeNote));
      if (mMetronomeTimer.IsReady(60000.0f / gAppSettings.mPerfSettings.mBPM)) {
        CCSynthGraph::metronomeEnv.noteOn();
      }
    }

    AudioInterrupts();
  }
};



#endif
