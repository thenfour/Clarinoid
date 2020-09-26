
#pragma once

#include <clarinoid/basic/Basic.hpp>

static const size_t MAX_SYNTH_VOICES = 8;

static const size_t HARM_PRESET_COUNT = 16;
static const size_t HARM_VOICES = 6;
static const size_t HARM_SEQUENCE_LEN = 8;

static const size_t LOOP_LAYERS = 6;
static constexpr size_t MAX_MUSICAL_VOICES = LOOP_LAYERS * (HARM_VOICES + 1 /* each harmonized preset can also output the playing (live) note as well, so make room.*/);

static const size_t PRESET_NAME_LEN = 16;

static const size_t SYNTH_PRESET_COUNT = 16;

bool gTouchKeyGraphsIsRunning = false; // todo: this is a hack


#include "HarmonizerSettings.hpp"
#include "SynthSettings.hpp"
#include "LoopstationSettings.hpp"




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
  bool mDisplayDim = true;
  bool mOrangeLEDs = false;

  float mBreathLowerBound = 0.07f;
  float mBreathUpperBound = 0.7f;
  float mBreathNoteOnThreshold = 0.01f;
  
  float mTouchMaxFactor = 1.5f;
  float mPitchDownMin = 0.15f;
  float mPitchDownMax = 0.6f;

  float mReverbGain = 0.2f;

  bool mMetronomeOn = false;
  float mMetronomeGain = 0.8f;
  int mMetronomeNote = 80;
  int mMetronomeDecayMS= 15;

  float mBPM = 104.0f;

  int mTranspose = 0;

  GlobalScaleRefType mGlobalScaleRef = GlobalScaleRefType::Chosen;
  Scale mGlobalScale = Scale { Note::E, ScaleFlavorIndex::MajorPentatonic }; // you can set this
  Scale mDeducedScale; // this is automatically populated always
  
  HarmSettings mHarmSettings;
  LooperSettings mLooperSettings;
  SynthSettings mSynthSettings;

  // these are for the live playing voice. a harmonizer's voices can override the synth preset though.
  uint16_t mGlobalSynthPreset = 0;
  uint16_t mGlobalHarmPreset = 0;
};

AppSettings gAppSettings;





///////////////////////////////////////////////////////////////////////////////////////////////////
HarmPreset& FindHarmPreset(uint16_t id) {
  if (id < 0) return gAppSettings.mHarmSettings.mDisabledPreset;
  if (id >= SYNTH_PRESET_COUNT) return gAppSettings.mHarmSettings.mDisabledPreset;
  return gAppSettings.mHarmSettings.mPresets[id];
}

SynthPreset& FindSynthPreset(uint16_t id)
{
  if (id < 0) id = 0;
  if (id >= SYNTH_PRESET_COUNT) id = 0;
  return gAppSettings.mSynthSettings.mPresets[id];
}

