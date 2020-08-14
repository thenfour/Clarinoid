
#ifndef CCSYNTH_H
#define CCSYNTH_H

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include "CCUtil.h"

namespace CCSynthGraph
{
  
// GUItool: begin automatically generated code
AudioSynthWaveform       waveformLeft;      //xy=709.8888549804688,324
AudioAmplifier           ampLeft;           //xy=917.0000038146973,323.99999713897705
AudioOutputI2S           i2s1;           //xy=1112.888916015625,371
AudioConnection          patchCord1(waveformLeft, ampLeft);
AudioConnection          patchCord2(ampLeft, 0, i2s1, 0);
AudioControlSGTL5000     audioShield;    //xy=1092.8889236450195,294.9999990463257
// GUItool: end automatically generated code


}


class CCSynth : IUpdateObject
{
public:
  virtual void setup() {
    AudioMemory(15);
    CCSynthGraph::audioShield.enable();
    CCSynthGraph::audioShield.volume(1); // headphone vol
    delay(200);
    CCSynthGraph::ampLeft.gain(.6);
    CCSynthGraph::waveformLeft.begin(.5, 440, WAVEFORM_TRIANGLE);
  }

  virtual void loop() {
  }

  void SetGain(float f) {
    CCSynthGraph::ampLeft.gain(f);
  }
};



#endif
