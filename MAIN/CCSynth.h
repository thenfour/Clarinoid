
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


#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputTDM            voice1;           //xy=106.25003051757812,135.00001525878906
AudioInputTDM            voice2; //xy=222.5000343322754,326.25001525878906
AudioInputTDM            voice3; //xy=328.75001525878906,492.5000419616699
AudioInputTDM            voice4; //xy=436.25000381469727,610.0000228881836
AudioMixer4              voice3Mix;         //xy=643.7500267028809,472.5000305175781
AudioMixer4              voice4Mix;         //xy=643.7500190734863,538.7500343322754
AudioMixer4              voice1Mix;         //xy=645.0000457763672,337.5000190734863
AudioMixer4              voice2Mix;         //xy=645.0000228881836,407.5000190734863
AudioMixer4              waveMixer;      //xy=840.0000114440918,416.25000762939453
AudioFilterStateVariable waveFilter;     //xy=1007.0000114440918,422.25000762939453
AudioEffectFreeverbStereo verb;           //xy=1325.0000114440918,390.25000762939453
AudioAmplifier           verbWetAmpLeft; //xy=1506.0000114440918,386.25000762939453
AudioAmplifier           verbWetAmpRight; //xy=1508.0000114440918,423.25000762939453
AudioMixer4              postMixerLeft;  //xy=1671.0000114440918,575.2500076293945
AudioMixer4              postMixerRight; //xy=1675.0000114440918,641.2500076293945
AudioAmplifier           ampLeft;        //xy=1832.0000114440918,588.2500076293945
AudioAmplifier           ampRight;       //xy=1834.0000114440918,623.2500076293945
AudioOutputI2S           i2s1;           //xy=1977.0000114440918,606.2500076293945
AudioConnection          patchCord1(voice1, 0, voice1Mix, 0);
AudioConnection          patchCord2(voice1, 1, voice1Mix, 1);
AudioConnection          patchCord3(voice1, 2, voice1Mix, 2);
AudioConnection          patchCord4(voice2, 0, voice2Mix, 0);
AudioConnection          patchCord5(voice2, 1, voice2Mix, 1);
AudioConnection          patchCord6(voice2, 2, voice2Mix, 2);
AudioConnection          patchCord7(voice3, 0, voice3Mix, 0);
AudioConnection          patchCord8(voice3, 1, voice3Mix, 1);
AudioConnection          patchCord9(voice3, 2, voice3Mix, 2);
AudioConnection          patchCord10(voice4, 0, voice4Mix, 0);
AudioConnection          patchCord11(voice4, 1, voice4Mix, 1);
AudioConnection          patchCord12(voice4, 2, voice4Mix, 2);
AudioConnection          patchCord13(voice3Mix, 0, waveMixer, 2);
AudioConnection          patchCord14(voice4Mix, 0, waveMixer, 3);
AudioConnection          patchCord15(voice1Mix, 0, waveMixer, 0);
AudioConnection          patchCord16(voice2Mix, 0, waveMixer, 1);
AudioConnection          patchCord17(waveMixer, 0, waveFilter, 0);
AudioConnection          patchCord18(waveFilter, 0, verb, 0);
AudioConnection          patchCord19(waveFilter, 0, postMixerLeft, 1);
AudioConnection          patchCord20(waveFilter, 0, postMixerRight, 1);
AudioConnection          patchCord21(verb, 0, verbWetAmpLeft, 0);
AudioConnection          patchCord22(verb, 1, verbWetAmpRight, 0);
AudioConnection          patchCord23(verbWetAmpLeft, 0, postMixerLeft, 0);
AudioConnection          patchCord24(verbWetAmpRight, 0, postMixerRight, 0);
AudioConnection          patchCord25(postMixerLeft, ampLeft);
AudioConnection          patchCord26(postMixerRight, ampRight);
AudioConnection          patchCord27(ampLeft, 0, i2s1, 0);
AudioConnection          patchCord28(ampRight, 0, i2s1, 1);
AudioControlSGTL5000     audioShield;    //xy=1944.0000114440918,536.2500076293945
// GUItool: end automatically generated code


*/



// GUItool: begin automatically generated code
AudioBandlimitedOsci            voice1;           //xy=106.25003051757812,135.00001525878906
AudioBandlimitedOsci            voice2; //xy=222.5000343322754,326.25001525878906
AudioBandlimitedOsci            voice3; //xy=328.75001525878906,492.5000419616699
AudioBandlimitedOsci            voice4; //xy=436.25000381469727,610.0000228881836
AudioMixer4              voice3Mix;         //xy=643.7500267028809,472.5000305175781
AudioMixer4              voice4Mix;         //xy=643.7500190734863,538.7500343322754
AudioMixer4              voice1Mix;         //xy=645.0000457763672,337.5000190734863
AudioMixer4              voice2Mix;         //xy=645.0000228881836,407.5000190734863
AudioMixer4              waveMixer;      //xy=840.0000114440918,416.25000762939453
AudioFilterStateVariable waveFilter;     //xy=1007.0000114440918,422.25000762939453
AudioEffectFreeverbStereo verb;           //xy=1325.0000114440918,390.25000762939453
AudioAmplifier           verbWetAmpLeft; //xy=1506.0000114440918,386.25000762939453
AudioAmplifier           verbWetAmpRight; //xy=1508.0000114440918,423.25000762939453
AudioMixer4              postMixerLeft;  //xy=1671.0000114440918,575.2500076293945
AudioMixer4              postMixerRight; //xy=1675.0000114440918,641.2500076293945
AudioAmplifier           ampLeft;        //xy=1832.0000114440918,588.2500076293945
AudioAmplifier           ampRight;       //xy=1834.0000114440918,623.2500076293945
AudioOutputI2S           i2s1;           //xy=1977.0000114440918,606.2500076293945
AudioConnection          patchCord1(voice1, 0, voice1Mix, 0);
AudioConnection          patchCord2(voice1, 1, voice1Mix, 1);
AudioConnection          patchCord3(voice1, 2, voice1Mix, 2);
AudioConnection          patchCord4(voice2, 0, voice2Mix, 0);
AudioConnection          patchCord5(voice2, 1, voice2Mix, 1);
AudioConnection          patchCord6(voice2, 2, voice2Mix, 2);
AudioConnection          patchCord7(voice3, 0, voice3Mix, 0);
AudioConnection          patchCord8(voice3, 1, voice3Mix, 1);
AudioConnection          patchCord9(voice3, 2, voice3Mix, 2);
AudioConnection          patchCord10(voice4, 0, voice4Mix, 0);
AudioConnection          patchCord11(voice4, 1, voice4Mix, 1);
AudioConnection          patchCord12(voice4, 2, voice4Mix, 2);
AudioConnection          patchCord13(voice3Mix, 0, waveMixer, 2);
AudioConnection          patchCord14(voice4Mix, 0, waveMixer, 3);
AudioConnection          patchCord15(voice1Mix, 0, waveMixer, 0);
AudioConnection          patchCord16(voice2Mix, 0, waveMixer, 1);
AudioConnection          patchCord17(waveMixer, 0, waveFilter, 0);
AudioConnection          patchCord18(waveFilter, 0, verb, 0);
AudioConnection          patchCord19(waveFilter, 0, postMixerLeft, 1);
AudioConnection          patchCord20(waveFilter, 0, postMixerRight, 1);
AudioConnection          patchCord21(verb, 0, verbWetAmpLeft, 0);
AudioConnection          patchCord22(verb, 1, verbWetAmpRight, 0);
AudioConnection          patchCord23(verbWetAmpLeft, 0, postMixerLeft, 0);
AudioConnection          patchCord24(verbWetAmpRight, 0, postMixerRight, 0);
AudioConnection          patchCord25(postMixerLeft, ampLeft);
AudioConnection          patchCord26(postMixerRight, ampRight);
AudioConnection          patchCord27(ampLeft, 0, i2s1, 0);
AudioConnection          patchCord28(ampRight, 0, i2s1, 1);
AudioControlSGTL5000     audioShield;    //xy=1944.0000114440918,536.2500076293945
// GUItool: end automatically generated code

}


//
//class CCSynthVoice
//{
//  AudioBandlimitedOsci& mOsc;
//public:
//  CCSynthVoice(AudioBandlimitedOsci& _osc) :
//    mOsc(_osc)
//  {
//  }
//};

class CCSynth : IUpdateObject
{
  bool isPlaying = false;
  uint8_t playingMidiNote = 0;
  AudioBandlimitedOsci voices[4] = { CCSynthGraph::voice1, CCSynthGraph::voice2, CCSynthGraph::voice3, CCSynthGraph::voice4 };
  const int voiceCount = SizeofStaticArray(voices);

  bool mHarmonizerOn = false;

public:

  void SetHarmonizer(bool b) {
    mHarmonizerOn = b;
    if (mHarmonizerOn) {
      CCSynthGraph::voice3.amplitude(1, 0.2);
      CCSynthGraph::voice3.amplitude(2, 0.2);
      CCSynthGraph::voice4.amplitude(1, 0.2);
      CCSynthGraph::voice4.amplitude(2, 0.2);
    } else {
      CCSynthGraph::voice3.amplitude(1, 0.0);
      CCSynthGraph::voice3.amplitude(2, 0.0);
      CCSynthGraph::voice4.amplitude(1, 0.0);
      CCSynthGraph::voice4.amplitude(2, 0.0);
    }
  }

  bool IsHarmonizerEnabled() const { return mHarmonizerOn; }

  virtual void setup() {
    AudioMemory(15);
    CCSynthGraph::audioShield.enable();
    CCSynthGraph::audioShield.volume(.7); // headphone vol
    delay(200); // why?
    CCSynthGraph::ampLeft.gain(.5);

    for (int i = 0; i < voiceCount; ++ i) {
      voices[i].amplitude(1, 0);
      voices[i].amplitude(2, 0);
      voices[i].amplitude(3, 0);
    }

    float p = 0.005;

    // voice 1 setup
    {
      CCSynthGraph::voice1.waveform(1, 1); // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
      CCSynthGraph::voice1.waveform(2, 3);
      CCSynthGraph::voice1.waveform(3, 1);
    
      CCSynthGraph::voice1.pulseWidth(1, 0.5);
      CCSynthGraph::voice1.pulseWidth(2, 0.5);
      CCSynthGraph::voice1.pulseWidth(3, 0.5);
  
      CCSynthGraph::voice1.amplitude(1, 0.0);
      CCSynthGraph::voice1.amplitude(2, 0.3);
      CCSynthGraph::voice1.amplitude(3, 0.3);
      
      CCSynthGraph::voice1.portamentoTime(1, p);
      CCSynthGraph::voice1.portamentoTime(2, p);
      CCSynthGraph::voice1.portamentoTime(3, p);  
      CCSynthGraph::voice1.addNote(); // this enables portamento. we never need to do note-offs because this synth just plays a continuous single note.
    }

    // voice 2 setup
    {
      CCSynthGraph::voice2.waveform(1, 1); // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync.
      CCSynthGraph::voice2.waveform(2, 1);
      //CCSynthGraph::voice2.waveform(3, 1);
  
      CCSynthGraph::voice2.pulseWidth(1, 0.0);
      CCSynthGraph::voice2.pulseWidth(2, 0.0);
      //CCSynthGraph::voice2.pulseWidth(3, 0.0);
  
      CCSynthGraph::voice2.amplitude(1, 0.2);
      CCSynthGraph::voice2.amplitude(2, 0.2);
      CCSynthGraph::voice2.amplitude(3, 0.0);
  
      CCSynthGraph::voice2.portamentoTime(1, p);
      CCSynthGraph::voice2.portamentoTime(2, p);
      CCSynthGraph::voice2.portamentoTime(3, p);
      CCSynthGraph::voice2.addNote(); // this enables portamento. we never need to do note-offs because this synth just plays a continuous single note.

    }

    // voice 3 setup
    {
      CCSynthGraph::voice3.waveform(1, 1); // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync.
      CCSynthGraph::voice3.waveform(2, 1);
      CCSynthGraph::voice3.pulseWidth(1, 0.0);
      CCSynthGraph::voice3.pulseWidth(2, 0.0);
      CCSynthGraph::voice3.portamentoTime(1, .05);
      CCSynthGraph::voice3.portamentoTime(2, .05);
      CCSynthGraph::voice3.addNote(); // this enables portamento. we never need to do note-offs because this synth just plays a continuous single note.
    }

    // voice 4 setup
    {
      CCSynthGraph::voice4.waveform(1, 1); // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync.
      CCSynthGraph::voice4.waveform(2, 1);
      CCSynthGraph::voice4.pulseWidth(1, 0.0);
      CCSynthGraph::voice4.pulseWidth(2, 0.0);
      CCSynthGraph::voice4.portamentoTime(1, .01);
      CCSynthGraph::voice4.portamentoTime(2, .01);
      CCSynthGraph::voice4.addNote(); // this enables portamento. we never need to do note-offs because this synth just plays a continuous single note.
    }
    CCSynthGraph::verb.roomsize(.6f);
    CCSynthGraph::verb.damping(.7f);
    CCSynthGraph::verbWetAmpLeft.gain(.3);
    CCSynthGraph::verbWetAmpRight.gain(.3);
    
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

    // voice 1
    {
      float freq = MIDINoteToFreq(midiNote);
      float freq2 = MIDINoteToFreq(midiNote + 7);
      float freqSync = map(breathAdj, 0.0f, 1.0f, freq * 2, freq * 7);
    
      CCSynthGraph::voice1.frequency(1, freq);
      CCSynthGraph::voice1.frequency(2, freqSync);
      CCSynthGraph::voice1.frequency(3, freq2);
    }

    // voice 2
    {
      float freq = MIDINoteToFreq(midiNote);
      CCSynthGraph::voice2.frequency(1, freq);
      CCSynthGraph::voice2.frequency(2, freq * .995);
    }

    // voice 3
    {
      float freq = MIDINoteToFreq(midiNote - 4);
      CCSynthGraph::voice3.frequency(1, freq);
      CCSynthGraph::voice3.frequency(2, freq);
    }

    // voice 4
    {
      float freq = MIDINoteToFreq(midiNote - 11);
      CCSynthGraph::voice4.frequency(1, freq);
      CCSynthGraph::voice4.frequency(2, freq);
    }

    float filterFreq = map(breathAdj, 0.01, 1, 0, 15000);
    //CCPlot(filterFreq);
    CCSynthGraph::waveFilter.frequency(filterFreq);

    AudioInterrupts();
  }
};



#endif
