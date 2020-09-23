
#pragma once

#include <clarinoid/basic/Basic.hpp>

static const size_t MAX_SYNTH_VOICES = 8;

static const size_t HARM_PRESET_COUNT = 16;
static const size_t HARM_VOICES = 6;
static const size_t HARM_SEQUENCE_LEN = 8;

static const size_t LOOP_LAYERS = 6;
static constexpr size_t MAX_MUSICAL_VOICES = LOOP_LAYERS * (HARM_VOICES + 1 /* each harmonized preset can also output the playing (live) note as well, so make room.*/);

static const size_t PRESET_NAME_LEN = 16;

static const size_t SYNTH_PRESET_COUNT = 10;

bool gTouchKeyGraphsIsRunning = false; // todo: this is a hack

class PresetName
{
  char buf[PRESET_NAME_LEN];
};

enum class ScaleRefType : uint8_t
{
  Global,
  Deduced,
  Local
};

EnumItemInfo<ScaleRefType> gScaleRefTypeItems[3] = {
  { ScaleRefType::Global, "Global" },
  { ScaleRefType::Deduced, "Deduced" },
  { ScaleRefType::Local, "Local" },
};

EnumInfo<ScaleRefType> gScaleRefTypeInfo (gScaleRefTypeItems);






#include "HarmonizerSettings.hpp"
#include "SynthSettings.hpp"
#include "LoopstationSettings.hpp"


struct AppSettings
{
  float mReverbGain = 0.2f;

  bool mDisplayDim = true;
  bool mOrangeLEDs = false;

  float mBreathLowerBound = 0.07f;
  float mBreathUpperBound = 0.7f;
  float mBreathNoteOnThreshold = 0.01f;
  
  bool mMetronomeOn = false;
  float mMetronomeGain = 0.8f;
  int mMetronomeNote = 80;
  int mMetronomeDecayMS= 15;

  int mTranspose = 0;
  ScaleRefType mGlobalScaleRef;
  Scale mGlobalScale; // you can set this
  Scale mDeducedScale; // this is automatically populated always
  float mBPM = 90.0f;
  
  HarmSettings mHarmSettings;
  LooperSettings mLooperSettings;
  SynthSettings mSynthSettings;

  // these are for the live playing voice. a harmonizer's voices can override the synth preset though.
  uint16_t mGlobalSynthPreset = 0;
  uint16_t mSelectedHarmPreset = 0;

  float mTouchMaxFactor = 1.5f;
  float mPitchDownMin = 0.15f;
  float mPitchDownMax = 0.6f;
};

AppSettings gAppSettings;

HarmPreset& FindHarmPreset(uint16_t id) {
  static bool defaultInitialized = false;
  static HarmPreset defaultHarmPreset;
  if (!defaultInitialized) {
    defaultHarmPreset.mVoiceSettings[0].mVoiceType = HarmVoiceType::Sequence;
    defaultHarmPreset.mVoiceSettings[0].mSequence[0].mInterval = 0;
    defaultHarmPreset.mVoiceSettings[0].mSequence[0].mEnd = true;
    defaultHarmPreset.mVoiceSettings[0].mScaleRef = ScaleRefType::Local;
    defaultHarmPreset.mVoiceSettings[0].mLocalScale = Scale { ScaleFlavorIndex::Chromatic, 0 };
  }
  // if (!enabled) {
  //   return defaultHarmPreset;
  // }
  if (id < 0) {
    return defaultHarmPreset;
  }
  if (id >= SYNTH_PRESET_COUNT) {
    return defaultHarmPreset;
  }
  return gAppSettings.mHarmSettings.mPresets[id];
}


SynthPreset& FindSynthPreset(uint16_t id)
{
  if (id < 0) id = 0;
  if (id >= SYNTH_PRESET_COUNT) id = 0;
  return gAppSettings.mSynthSettings.mPresets[id];
}

