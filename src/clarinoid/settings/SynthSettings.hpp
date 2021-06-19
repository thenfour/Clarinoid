
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
    float mFilterMaxFreq = 16000.0f;
    float mFilterMinFreq = 0.0f;
    float mFilterSaturation = 0.2f;
    float mFilterKeytracking = 0.0f; // 0 = no keytracking affect. 1.0 = full effect applied, -1.0 = negative effect applied (low notes get higher freq cutoff)
  };

  struct SynthSettings
  {
    SynthPreset mPresets[SYNTH_PRESET_COUNT];
    float mReverbGain = 0.0f;
    float mMasterGain = 1.0f;



    static void InitClarinoid2Preset(SynthPreset& p, const char* name, ClarinoidFilterType filt, float filterKeyScaling, float q, float filterMaxFreq)
    {
      // detuned saw.
      p.mName = name;
      p.mOsc1Gain = 0.0f;
      p.mOsc3Gain = 0.0f;

      p.mOsc1Waveform = OscWaveformShape::SawSync;
      p.mOsc2Waveform = OscWaveformShape::SawSync;
      p.mOsc3Waveform = OscWaveformShape::SawSync;
      p.mOsc2Gain = .99f;
      p.mSync = false;
      p.mDetune = 0.0f;

      p.mFilterType = filt;
      p.mFilterMinFreq = 0.0f;
      p.mFilterMaxFreq = filterMaxFreq;
      p.mFilterSaturation = 0;
      p.mFilterQ = q;
      p.mFilterKeytracking = filterKeyScaling;
    }

    static void InitBasicLeadPreset(const char* name, OscWaveformShape shape, float pulseWidth, SynthPreset& p) {
      p.mName = name;
      p.mOsc1Gain = 0.0f;
      p.mOsc3Gain = 0.0f;
      p.mSync = false;
      p.mDetune = 0.0f;

      p.mOsc2Gain = .99f;
      p.mOsc2Waveform = shape;
      p.mOsc2PulseWidth = pulseWidth;

      p.mFilterType = ClarinoidFilterType::LP_Moog4;
      p.mFilterMinFreq = 0.0f;
      p.mFilterMaxFreq = 16000;
      p.mFilterSaturation = 0.30f;
      p.mFilterQ = 0.25f;
      p.mFilterKeytracking = 0.0f;
    }

    static void InitDetunedLeadPreset(const char* name, OscWaveformShape shape, float pulseWidth, SynthPreset& p) {
      p.mName = name;
      p.mDetune = 0.1f;
      p.mSync = false;

      p.mOsc1Gain = 0.99f;
      p.mOsc1Waveform = shape;
      p.mOsc1PulseWidth = pulseWidth;

      p.mOsc2Gain = .99f;
      p.mOsc2Waveform = shape;
      p.mOsc2PulseWidth = 1.0f - pulseWidth;

      p.mOsc3Gain = .99f;
      p.mOsc3Waveform = shape;
      p.mOsc2PulseWidth = pulseWidth;

      p.mFilterType = ClarinoidFilterType::LP_Moog4;
      p.mFilterMinFreq = 0.0f;
      p.mFilterMaxFreq = 16000;
      p.mFilterSaturation = 0.30f;
      p.mFilterQ = 0.25f;
      p.mFilterKeytracking = 0.0f;
    }

    static void InitFifthLeadPresetA(SynthPreset& p) {
      p.mName = "5th lead A";
      p.mSync = false;
      p.mDetune = 0.0f;

      p.mOsc1Gain = 0.99f;
      p.mOsc1Waveform = OscWaveformShape::Pulse;
      p.mOsc1PulseWidth = 0.08f;
      p.mOsc1PitchSemis = -5;

      p.mOsc3Gain = 0.0f;

      p.mOsc2Gain = .99f;
      p.mOsc2Waveform = OscWaveformShape::Pulse;
      p.mOsc2PulseWidth = 0.92f;
      p.mOsc2PitchSemis = -12;

      p.mFilterType = ClarinoidFilterType::LP_Moog4;
      p.mFilterMinFreq = 0.0f;
      p.mFilterMaxFreq = 18000;
      p.mFilterSaturation = 0.0f;
      p.mFilterQ = 0.1f;
      p.mFilterKeytracking = 0.0f;
    }

    static void InitFifthLeadPresetB(SynthPreset& p) {
      p.mName = "5th lead B";
      p.mSync = false;
      p.mDetune = 0.0f;

      p.mOsc1Gain = 0.99f;
      p.mOsc1Waveform = OscWaveformShape::Pulse;
      p.mOsc1PulseWidth = 0.4f;
      p.mOsc1PitchSemis = -5;

      p.mOsc3Gain = 0.0f;

      p.mOsc2Gain = .99f;
      p.mOsc2Waveform = OscWaveformShape::Pulse;
      p.mOsc2PitchSemis = -12;

      p.mFilterType = ClarinoidFilterType::LP_Moog4;
      p.mFilterMinFreq = 0.0f;
      p.mFilterMaxFreq = 16000;
      p.mFilterSaturation = 0.0f;
      p.mFilterQ = 0.0f;
      p.mFilterKeytracking = 0.0f;
    }

    SynthSettings()
    {
      mPresets[0].mName = "Sync Lead"; // default.

      size_t i = 1; // 0 = default = sync
      InitBasicLeadPreset("One Saw", OscWaveformShape::SawSync, 0.5f, mPresets[i++]);
      InitBasicLeadPreset("One Tri", OscWaveformShape::VarTriangle, 0.5f, mPresets[i++]);
      InitBasicLeadPreset("One Pulse 8%", OscWaveformShape::Pulse, 0.08f, mPresets[i++]);
      InitBasicLeadPreset("One Pulse 50%", OscWaveformShape::Pulse, 0.50f, mPresets[i++]);
      InitDetunedLeadPreset("Detuned saws", OscWaveformShape::SawSync, 0.5f, mPresets[i++]);
      InitDetunedLeadPreset("Detuned sine", OscWaveformShape::Sine, 0.5f, mPresets[i++]);
      InitDetunedLeadPreset("Detuned pulse 10", OscWaveformShape::Pulse, 0.1f, mPresets[i++]);
      InitDetunedLeadPreset("Detuned pulse 50", OscWaveformShape::Pulse, 0.5f, mPresets[i++]);
      InitDetunedLeadPreset("Detuned tri 10", OscWaveformShape::VarTriangle, 0.1f, mPresets[i++]);
      InitDetunedLeadPreset("Detuned tri 50", OscWaveformShape::VarTriangle, 0.5f, mPresets[i++]);
      InitFifthLeadPresetA(mPresets[i++]);
      InitFifthLeadPresetB(mPresets[i++]);

      // InitClarinoid2Preset(mPresets[i++], "Diode-ks7-q15", ClarinoidFilterType::LP_Diode, 0.7f, 0.15f, 10000);
      // InitClarinoid2Preset(mPresets[i++], "Diode-ks7-q0", ClarinoidFilterType::LP_Diode, 0.7f, 0.0f, 10000);
      // InitClarinoid2Preset(mPresets[i++], "Diode-ks9-q15", ClarinoidFilterType::LP_Diode, 0.9f, 0.15f, 10000);
      // InitClarinoid2Preset(mPresets[i++], "Diode-ks9-q0", ClarinoidFilterType::LP_Diode, 0.9f, 0.0f, 10000);

      // InitClarinoid2Preset(mPresets[i++], "Moog-ks7-q15", ClarinoidFilterType::LP_Moog4, 0.7f, 0.15f, 8000);
      // InitClarinoid2Preset(mPresets[i++], "Moog-ks7-q0", ClarinoidFilterType::LP_Moog4, 0.7f, 0.0f, 8000);
      // InitClarinoid2Preset(mPresets[i++], "Moog-ks9-q15", ClarinoidFilterType::LP_Moog4, 0.9f, 0.15f, 8000);
      // InitClarinoid2Preset(mPresets[i++], "Moog-ks9-q0", ClarinoidFilterType::LP_Moog4, 0.9f, 0.0f, 8000);

      // InitClarinoid2Preset(mPresets[i++], "K35-ks7-q15", ClarinoidFilterType::LP_K35, 0.7f, 0.15f, 750);
      // InitClarinoid2Preset(mPresets[i++], "K35-ks7-q0", ClarinoidFilterType::LP_K35, 0.7f, 0.0f, 750);
      // InitClarinoid2Preset(mPresets[i++], "K35-ks9-q15", ClarinoidFilterType::LP_K35, 0.9f, 0.15f, 750);
      // InitClarinoid2Preset(mPresets[i++], "K35-ks9-q0", ClarinoidFilterType::LP_K35, 0.9f, 0.0f, 750);

      //mPresets[i++].mName = "Sync";
    }    
  };

} // namespace
