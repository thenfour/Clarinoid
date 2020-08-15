
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
 replace blep osc with AudioInputTDM for guitool


// GUItool: begin automatically generated code
AudioInputTDM            waveform;       //xy=107,244
AudioInputTDM            waveform1; //xy=107,492
AudioMixer4              waveMixer;      //xy=269,179
AudioFilterStateVariable waveFilter;     //xy=436,185
AudioEffectFreeverbStereo verb;           //xy=754.0000228881836,153.00002479553223
AudioAmplifier           verbWetAmpLeft; //xy=935.000114440918,149.9090919494629
AudioAmplifier           verbWetAmpRight; //xy=937.0000228881836,186.9090929031372
AudioMixer4              postMixerLeft;  //xy=1100,338
AudioMixer4              postMixerRight; //xy=1104,404
AudioAmplifier           ampLeft;        //xy=1261,351
AudioAmplifier           ampRight;       //xy=1263,386
AudioOutputI2S           i2s1;           //xy=1406,369
AudioConnection          patchCord1(waveform, 1, waveMixer, 0);
AudioConnection          patchCord2(waveform, 2, waveMixer, 1);
AudioConnection          patchCord3(waveform1, 0, waveMixer, 2);
AudioConnection          patchCord4(waveform1, 1, waveMixer, 3);
AudioConnection          patchCord5(waveMixer, 0, waveFilter, 0);
AudioConnection          patchCord6(waveFilter, 0, verb, 0);
AudioConnection          patchCord7(waveFilter, 0, postMixerLeft, 1);
AudioConnection          patchCord8(waveFilter, 0, postMixerRight, 1);
AudioConnection          patchCord9(verb, 0, verbWetAmpLeft, 0);
AudioConnection          patchCord10(verb, 1, verbWetAmpRight, 0);
AudioConnection          patchCord11(verbWetAmpLeft, 0, postMixerLeft, 0);
AudioConnection          patchCord12(verbWetAmpRight, 0, postMixerRight, 0);
AudioConnection          patchCord13(postMixerLeft, ampLeft);
AudioConnection          patchCord14(postMixerRight, ampRight);
AudioConnection          patchCord15(ampLeft, 0, i2s1, 0);
AudioConnection          patchCord16(ampRight, 0, i2s1, 1);
AudioControlSGTL5000     audioShield;    //xy=1373,299
// GUItool: end automatically generated code



*/

// GUItool: begin automatically generated code
AudioBandlimitedOsci            waveform;       //xy=107,244
AudioBandlimitedOsci            waveform1; //xy=107,492
AudioMixer4              waveMixer;      //xy=269,179
AudioFilterStateVariable waveFilter;     //xy=436,185
AudioEffectFreeverbStereo verb;           //xy=754.0000228881836,153.00002479553223
AudioAmplifier           verbWetAmpLeft; //xy=935.000114440918,149.9090919494629
AudioAmplifier           verbWetAmpRight; //xy=937.0000228881836,186.9090929031372
AudioMixer4              postMixerLeft;  //xy=1100,338
AudioMixer4              postMixerRight; //xy=1104,404
AudioAmplifier           ampLeft;        //xy=1261,351
AudioAmplifier           ampRight;       //xy=1263,386
AudioOutputI2S           i2s1;           //xy=1406,369
AudioConnection          patchCord1(waveform, 1, waveMixer, 0);
AudioConnection          patchCord2(waveform, 2, waveMixer, 1);
AudioConnection          patchCord3(waveform1, 0, waveMixer, 2);
AudioConnection          patchCord4(waveform1, 1, waveMixer, 3);
AudioConnection          patchCord5(waveMixer, 0, waveFilter, 0);
AudioConnection          patchCord6(waveFilter, 0, verb, 0);
AudioConnection          patchCord7(waveFilter, 0, postMixerLeft, 1);
AudioConnection          patchCord8(waveFilter, 0, postMixerRight, 1);
AudioConnection          patchCord9(verb, 0, verbWetAmpLeft, 0);
AudioConnection          patchCord10(verb, 1, verbWetAmpRight, 0);
AudioConnection          patchCord11(verbWetAmpLeft, 0, postMixerLeft, 0);
AudioConnection          patchCord12(verbWetAmpRight, 0, postMixerRight, 0);
AudioConnection          patchCord13(postMixerLeft, ampLeft);
AudioConnection          patchCord14(postMixerRight, ampRight);
AudioConnection          patchCord15(ampLeft, 0, i2s1, 0);
AudioConnection          patchCord16(ampRight, 0, i2s1, 1);
AudioControlSGTL5000     audioShield;    //xy=1373,299
// GUItool: end automatically generated code


}

class CCSynth : IUpdateObject
{
public:
  virtual void setup() {
    AudioMemory(15);
    CCSynthGraph::audioShield.enable();
    CCSynthGraph::audioShield.volume(.7); // headphone vol
    delay(200); // why?
    CCSynthGraph::ampLeft.gain(.5);

    CCSynthGraph::waveform.waveform(1, 1); // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
    CCSynthGraph::waveform.waveform(2, 3);
    CCSynthGraph::waveform.waveform(3, 1);
  
    CCSynthGraph::waveform.pulseWidth(1, 0.5);
    CCSynthGraph::waveform.pulseWidth(2, 0.5);
    CCSynthGraph::waveform.pulseWidth(3, 0.5);

    CCSynthGraph::waveform.amplitude(1, 0.0);
    CCSynthGraph::waveform.amplitude(2, 0.3);
    CCSynthGraph::waveform.amplitude(3, 0.3);


    CCSynthGraph::waveform1.waveform(1, 1); // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync.
    CCSynthGraph::waveform1.waveform(2, 1);
    //CCSynthGraph::waveform1.waveform(3, 1);

    CCSynthGraph::waveform1.pulseWidth(1, 0.0);
    CCSynthGraph::waveform1.pulseWidth(2, 0.0);
    //CCSynthGraph::waveform1.pulseWidth(3, 0.0);

    CCSynthGraph::waveform1.amplitude(1, 0.2);
    CCSynthGraph::waveform1.amplitude(2, 0.2);
    CCSynthGraph::waveform1.amplitude(3, 0.0);

    //CCSynthGraph::waveform.portamentoTime(1, 0.1);
    //CCSynthGraph::waveform.portamentoTime(2, 0.1);
    //CCSynthGraph::waveform.portamentoTime(3, 0.1);

    //CCSynthGraph::waveform1.portamentoTime(1, 0.1);
    //CCSynthGraph::waveform1.portamentoTime(2, 0.1);
    //CCSynthGraph::waveform1.portamentoTime(3, 0.1);

    CCSynthGraph::verb.roomsize(.6f);
    CCSynthGraph::verb.damping(.7f);
    CCSynthGraph::verbWetAmpLeft.gain(.3);
    CCSynthGraph::verbWetAmpRight.gain(.3);
    
    //CCSynthGraph::waveFilter.resonance(1.3);
  }

  virtual void loop() {
  }

  void SetGain(float f) {
    CCSynthGraph::ampLeft.gain(f);
    CCSynthGraph::ampRight.gain(f);
  }

  void Update(const CCEWIMusicalState& state) {
    // AudioNoInterrupts  https://www.pjrc.com/teensy/td_libs_AudioProcessorUsage.html
    AudioNoInterrupts();

    float midiNote = state.MIDINote + state.pitchBendN11.GetValue() * 2;

    float freq = MIDINoteToFreq(midiNote);
    float freq2 = MIDINoteToFreq(midiNote + 7);
    float freqSync = map(state.breath01.GetValue(), 0.0f, 1.0f, freq * 2, freq * 7);
  
    CCSynthGraph::waveform.frequency(1, freq);
    CCSynthGraph::waveform.frequency(2, freqSync);
    CCSynthGraph::waveform.frequency(3, freq2);
  
    CCSynthGraph::waveform1.frequency(1, freq * 1.005);
    CCSynthGraph::waveform1.frequency(2, freq * .995);

    CCSynthGraph::waveFilter.frequency(map(state.breath01.GetValue(), .06, 1, 0, 20000));
    
    AudioInterrupts();
  }
};



#endif
