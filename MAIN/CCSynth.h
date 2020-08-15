
#ifndef CCSYNTH_H
#define CCSYNTH_H

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include "CCUtil.h"
#include "CCSynthUtils.h"
#include "CCEWIControl.h"

// oscillator sync: https://forum.pjrc.com/threads/55878-Waveform-sync-question
// plenty more: https://www.google.com/search?q=waveform+sync+teensy&oq=waveform+sync+teensy&aqs=chrome..69i57.4186j0j7&sourceid=chrome&ie=UTF-8
// https://gitlab.com/flojawi/teensy-polyblep-oscillator/-/blob/master/polySynth/polyBlepOscillator.h
namespace CCSynthGraph
{





// GUItool: begin automatically generated code
AudioSynthWaveform       waveformLeft2; //xy=343.9147186279297,322.9147148132324
AudioSynthWaveform       waveformLeft1; //xy=355.9147186279297,276.9147434234619
AudioSynthWaveform       waveformLeft;      //xy=365.88880920410156,233.99990701675415
AudioMixer4              mixer1;         //xy=549.9147796630859,293.9147720336914
AudioFilterBiquad        biquad1;        //xy=710.8238410949707,288.91475009918213
AudioAmplifier           ampLeft;           //xy=902.9999580383301,328.999888420105
AudioOutputI2S           i2s1;           //xy=1080.8889389038086,331.9999694824219
AudioConnection          patchCord1(waveformLeft2, 0, mixer1, 2);
AudioConnection          patchCord2(waveformLeft1, 0, mixer1, 1);
AudioConnection          patchCord3(waveformLeft, 0, mixer1, 0);
AudioConnection          patchCord4(mixer1, biquad1);
AudioConnection          patchCord5(biquad1, ampLeft);
AudioConnection          patchCord6(ampLeft, 0, i2s1, 0);
AudioConnection          patchCord7(ampLeft, 0, i2s1, 1);
AudioControlSGTL5000     audioShield;    //xy=1076.8887405395508,265.9998970031738
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
    CCSynthGraph::waveformLeft.begin(.3, 440, WAVEFORM_SAWTOOTH);
    CCSynthGraph::waveformLeft1.begin(.3, 440, WAVEFORM_SAWTOOTH);
    CCSynthGraph::waveformLeft2.begin(.3, 440, WAVEFORM_SAWTOOTH);
  }

  virtual void loop() {
  }

  void SetGain(float f) {
    CCSynthGraph::ampLeft.gain(f);
  }

  void Update(const CCEWIMusicalState& state) {
    // AudioNoInterrupts  https://www.pjrc.com/teensy/td_libs_AudioProcessorUsage.html
    AudioNoInterrupts();

    float freq = MIDINoteToFreq(state.MIDINote + state.pitchBendN11 * 0);

    CCSynthGraph::waveformLeft.frequency(freq * 1.005);
    CCSynthGraph::waveformLeft1.frequency(freq);
    CCSynthGraph::waveformLeft2.frequency(freq * .75);
    CCSynthGraph::biquad1.setLowpass(0, map(state.breath01, .06, 1, 0, 10000), 1.5);
    
    AudioInterrupts();
  }
};



#endif
