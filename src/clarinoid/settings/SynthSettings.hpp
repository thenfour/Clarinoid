
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{


enum class OscWaveformShape
{
  Sine = 0,
  VarTriangle = 1,
  Pulse = 2,
  SawSync = 3 // not available in osc 1 (only 2 & 3)
};

EnumItemInfo<OscWaveformShape> gOscWaveformShapeItems[4] = {
  { OscWaveformShape::Sine, "Sine" },
  { OscWaveformShape::VarTriangle, "VarTriangle" },
  { OscWaveformShape::Pulse, "Pulse" },
  { OscWaveformShape::SawSync, "SawSync" },
};

EnumInfo<OscWaveformShape> gOscWaveformShapeInfo ("OscWaveformShape", gOscWaveformShapeItems);




struct SynthPreset
{
  String mName;
  float mPortamentoTime = 0.005f;
  float mOsc1Gain = 0;
  float mOsc2Gain = 0.9f;
  float mOsc3Gain = 0.9f;

  int mOsc1PitchSemis = 0;
  int mOsc2PitchSemis = 0;
  int mOsc3PitchSemis = 0;
  float mOsc1PitchFine = 0;
  float mOsc2PitchFine = 0;
  float mOsc3PitchFine = 0;

  OscWaveformShape mOsc1Waveform = OscWaveformShape::VarTriangle;//  // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
  OscWaveformShape mOsc2Waveform = OscWaveformShape::SawSync;//  // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
  OscWaveformShape mOsc3Waveform = OscWaveformShape::VarTriangle;//  // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
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
    mPresets[0].mName = "Sync.";

    // detuned saw.
    mPresets[1].mName = "SAW IV";
    mPresets[1].mOsc1Gain = .9f;
    mPresets[1].mOsc2Gain = .9f;
    mPresets[1].mOsc3Gain = .9f;

    mPresets[1].mOsc1Waveform = OscWaveformShape::SawSync;
    mPresets[1].mOsc2Waveform = OscWaveformShape::SawSync;
    mPresets[1].mOsc3Waveform = OscWaveformShape::SawSync;
    mPresets[1].mSync = false;
    mPresets[1].mDetune = 0.06f;
  }
};

} // namespace
