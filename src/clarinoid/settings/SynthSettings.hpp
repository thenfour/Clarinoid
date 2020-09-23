
#pragma once

#include <clarinoid/basic/Basic.hpp>


struct SynthPreset
{
  PresetName mName;
  float mPortamentoTime = 0.005f;
  float mOsc1Gain = 0;
  float mOsc2Gain = 0.3f;
  float mOsc3Gain = 0.3f;
  int mOsc1Waveform = 1;//  // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
  int mOsc2Waveform = 3;//  // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
  int mOsc3Waveform = 1;//  // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
  float mOsc1PulseWidth = 0.5f;
  float mOsc2PulseWidth = 0.5f;
  float mOsc3PulseWidth = 0.5f;
  bool mSync = true;
  float mDetune = 0;
};

struct SynthSettings
{
  SynthPreset mPresets[SYNTH_PRESET_COUNT];

  SynthSettings() 
  {
      mPresets[1].mOsc1Gain = .2f;
      mPresets[1].mOsc2Gain = .2f;
      mPresets[1].mOsc3Gain = .2f;

      mPresets[1].mOsc1Waveform = 3;
      mPresets[1].mOsc2Waveform = 3;
      mPresets[1].mOsc3Waveform = 3;
      mPresets[1].mSync = false;
      mPresets[1].mDetune = 0.06f;
  }
};

