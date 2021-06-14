
#pragma once

#include <clarinoid/basic/Basic.hpp>

#include "HarmonizerSettings.hpp"
#include "SynthSettings.hpp"
#include "LoopstationSettings.hpp"
#include "ControlMapping.hpp"

namespace clarinoid
{



enum class GlobalScaleRefType : uint8_t
{
  Chosen,
  Deduced,
};

EnumItemInfo<GlobalScaleRefType> gGlobalScaleRefTypeItems[2] = {
  { GlobalScaleRefType::Chosen, "Chosen" },
  { GlobalScaleRefType::Deduced, "Deduced" },
};

EnumInfo<GlobalScaleRefType> gGlobalScaleRefTypeInfo ("GlobalScaleRefType", gGlobalScaleRefTypeItems);


struct AppSettings
{
  ControlMapping mControlMappings[MAX_CONTROL_MAPPINGS];
  
  bool mDisplayDim = true;

  bool mMetronomeSoundOn = false;
  float mMetronomeGain = 0.8f;
  int mMetronomeNote = 80;
  int mMetronomeDecayMS= 15;

  bool mMetronomeLED = false;
  int mMetronomeBrightness = 255;
  float mMetronomeLEDDecay = 0.1f;

  float mBPM = 104.0f;

  int mTranspose = 0;

  GlobalScaleRefType mGlobalScaleRef = GlobalScaleRefType::Deduced;
  Scale mGlobalScale = Scale { Note::E, ScaleFlavorIndex::MajorPentatonic }; // you can set this in menus
  Scale mDeducedScale = Scale { Note::C, ScaleFlavorIndex::MajorPentatonic }; // this is automatically populated always
  
  HarmSettings mHarmSettings;
  LooperSettings mLooperSettings;
  SynthSettings mSynthSettings;

  // these are for the live playing voice. a harmonizer's voices can override the synth preset though.
  uint16_t mGlobalSynthPreset = 0;
  uint16_t mGlobalHarmPreset = 0;


  HarmPreset& FindHarmPreset(uint16_t id) {
    if (id < 0) return mHarmSettings.mDisabledPreset;
    if (id >= SYNTH_PRESET_COUNT) return mHarmSettings.mDisabledPreset;
    return mHarmSettings.mPresets[id];
  }

  SynthPreset& FindSynthPreset(uint16_t id)
  {
    if (id < 0) id = 0;
    if (id >= SYNTH_PRESET_COUNT) id = 0;
    return mSynthSettings.mPresets[id];
  }

};

//static constexpr auto appsettingssize = sizeof(AppSettings);
//static constexpr auto rththth = sizeof(AppSettings::mControlMappings);


} // namespace clarinoid

