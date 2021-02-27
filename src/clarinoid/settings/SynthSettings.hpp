
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
      {OscWaveformShape::Sine, "Sine"},
      {OscWaveformShape::VarTriangle, "VarTriangle"},
      {OscWaveformShape::Pulse, "Pulse"},
      {OscWaveformShape::SawSync, "SawSync"},
  };

  EnumInfo<OscWaveformShape> gOscWaveformShapeInfo("OscWaveformShape", gOscWaveformShapeItems);

  enum class ClarinoidFilterType : uint8_t
  {
    LP_OnePole = 0,
    LP_SEM12,
    LP_Diode,
    LP_K35,
    LP_Moog2,
    LP_Moog4,
    BP_Moog2,
    BP_Moog4,

    HP_OnePole, // HP filters are no good for our purpose
    HP_K35,
    HP_Moog2,
    HP_Moog4,
  };

    EnumItemInfo<ClarinoidFilterType> gClarinoidFilterTypeItems[8] = {
      {ClarinoidFilterType::LP_OnePole, "LP_OnePole"},
      {ClarinoidFilterType::LP_SEM12, "LP_SEM12"},
      {ClarinoidFilterType::LP_Diode, "LP_Diode"},
      {ClarinoidFilterType::LP_K35, "LP_K35"},
      {ClarinoidFilterType::LP_Moog2, "LP_Moog2"},
      {ClarinoidFilterType::LP_Moog4, "LP_Moog4"},
      {ClarinoidFilterType::BP_Moog2, "BP_Moog2"},
      {ClarinoidFilterType::BP_Moog4, "BP_Moog4"},
  };

  EnumInfo<ClarinoidFilterType> gClarinoidFilterTypeInfo("FilterType", gClarinoidFilterTypeItems);



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

    OscWaveformShape mOsc1Waveform = OscWaveformShape::VarTriangle; //  // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
    OscWaveformShape mOsc2Waveform = OscWaveformShape::SawSync;     //  // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
    OscWaveformShape mOsc3Waveform = OscWaveformShape::VarTriangle; //  // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
    float mOsc1PulseWidth = 0.5f;
    float mOsc2PulseWidth = 0.5f;
    float mOsc3PulseWidth = 0.5f;

    bool mSync = true;
    float mSyncMultMin = 2.0f;
    float mSyncMultMax = 7.0f;

    float mDetune = 0;

    bool mDCFilterEnabled = true;
    float mDCFilterCutoff = 10.0f;

    ClarinoidFilterType mFilterType = ClarinoidFilterType::LP_Moog4;
    float mFilterQ = 0.02f;
    float mFilterMaxFreq = 15000.0f;
    float mFilterMinFreq = 0.0f;
    float mFilterSaturation = 0.2f;
    float mFilterKeytracking = 0.0f; // 0 = no keytracking affect. 1.0 = full effect applied, -1.0 = negative effect applied (low notes get higher freq cutoff)
  };

  struct SynthSettings
  {
    SynthPreset mPresets[SYNTH_PRESET_COUNT];
    float mReverbGain = 0.0f;
    float mMasterGain = 1.0f;

    SynthSettings()
    {
      // detuned saw.
      mPresets[0].mName = "!Bassoonoid";
      mPresets[0].mOsc1Gain = 0.0f;
      mPresets[0].mOsc2Gain = 0.0f;
      mPresets[0].mOsc3Gain = 0.0f;

      mPresets[0].mOsc1Waveform = OscWaveformShape::SawSync;
      mPresets[0].mOsc2Waveform = OscWaveformShape::SawSync;
      mPresets[0].mOsc3Waveform = OscWaveformShape::SawSync;
      mPresets[0].mOsc2Gain = .99f;
      mPresets[0].mSync = false;
      mPresets[0].mDetune = 0.0f;

      mPresets[0].mFilterType = ClarinoidFilterType::LP_Diode;
      mPresets[0].mFilterMinFreq = 0.0f;
      mPresets[0].mFilterMaxFreq = 10000.0f;
      mPresets[0].mFilterQ = 0.2f;
      mPresets[0].mFilterSaturation = 0.4f;
      mPresets[0].mFilterKeytracking = 0.9f;

      mPresets[1].mName = "Sync";
    }
  };

} // namespace
