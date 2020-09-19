
#pragma once

#include <clarinoid/basic/Basic.hpp>

static const size_t MAX_SYNTH_VOICES = 8;

static const size_t HARM_PRESET_COUNT = 16;
static const size_t HARM_VOICES = 6;
static const size_t HARM_SEQUENCE_LEN = 8;

static const size_t LOOP_LAYERS = 3;
static constexpr size_t MAX_MUSICAL_VOICES = LOOP_LAYERS * (HARM_VOICES + 1 /* each harmonized preset can also output the playing (live) note as well, so make room.*/);

static const size_t PRESET_NAME_LEN = 16;

static const size_t SYNTH_PRESET_COUNT = 10;

bool gTouchKeyGraphsIsRunning = false; // todo: this is a hack

class PresetName
{
  char buf[PRESET_NAME_LEN];
};

///////////////////////////////////////////////////////////////////
enum class SynthPresetRefType : uint8_t
{
  Global,
  Local
};

EnumItemInfo<SynthPresetRefType> gSynthPresetRefTypeItems[2] = {
  { SynthPresetRefType::Global, "Global" },
  { SynthPresetRefType::Local, "Local" },
};

EnumInfo<SynthPresetRefType> gSynthPresetRefTypeInfo (gSynthPresetRefTypeItems);

///////////////////////////////////////////////////////////////////
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


///////////////////////////////////////////////////////////////////
struct HarmVoiceSequenceEntry
{
  bool mEnd = true;
  int8_t mInterval = 0;
};

enum class NoteOOBBehavior : uint8_t
{
  Drop,
  TransposeOctave
};

enum class NonDiatonicBehavior : uint8_t
{
  NextDiatonicNote,
  PrevDiatonicNote,
  FollowMelodyFromBelow, // so this voice plays a nondiatonic note too, based on distance from lower note
  FollowMelodyFromAbove, // so this voice plays a nondiatonic note too, based on distance from lower note
  Drop,
  DontMove,
  TryAlternateScale, // could be interesting to have a list of alternative scales to try.
};

enum class HarmVoiceType : uint8_t
{
  Off,
  Interval,
  Sequence
};

EnumItemInfo<HarmVoiceType> gHarmVoiceTypeItems[3] = {
  { HarmVoiceType::Off, "Off" },
  { HarmVoiceType::Interval, "Interval" },
  { HarmVoiceType::Sequence, "Sequence" },
};

EnumInfo<HarmVoiceType> gHarmVoiceTypeInfo (gHarmVoiceTypeItems);



struct HarmVoiceSettings
{
  HarmVoiceType mVoiceType = HarmVoiceType::Off;
  HarmVoiceSequenceEntry mSequence[HARM_SEQUENCE_LEN];

  SynthPresetRefType mSynthPresetRef = SynthPresetRefType::Global;
  int mLocalSynthPreset = 0;

  ScaleRefType mScaleRef = ScaleRefType::Global;
  Scale mLocalScale;
  int mMinOutpNote = 0;
  int mMaxOutpNote = 127;
  NoteOOBBehavior mNoteOOBBehavior = NoteOOBBehavior::Drop;
  NonDiatonicBehavior mNonDiatonicBehavior = NonDiatonicBehavior::Drop;
  int mMinOutpVel = 0;
  int mMaxOutpVel = 127;
};

struct HarmPreset
{
  PresetName mName;
  bool mEmitLiveNote = true;
  HarmVoiceSettings mVoiceSettings[HARM_VOICES];
};

struct HarmSettings
{
  bool mIsEnabled = false;
  HarmPreset mPresets[HARM_PRESET_COUNT];

  int mGlobalSynthPreset;
  Scale mGlobalScale;
  int mMinRotationTimeMS;
};

struct AppSettings
{
  float mPortamentoTime = 0.005f;
  float mReverbGain = .2f;

  bool mDisplayDim = true;
  bool mOrangeLEDs = false;

  float mBreathLowerBound = 0.1f;
  float mBreathUpperBound = 0.7f;
  float mBreathNoteOnThreshold = 0.01f;
  
  bool mMetronomeOn = false;
  float mMetronomeGain = 0.8f;
  int mMetronomeNote = 80;
  int mMetronomeDecayMS= 15;

  int mTranspose = 0;
  Scale mGlobalScale; // you can set this
  Scale mDeducedScale; // this is automatically populated always
  float mBPM = 90.0f;
  
  HarmSettings mHarmSettings;

  float mTouchMaxFactor = 1.5f;
  float mPitchDownMin = 0.15f;
  float mPitchDownMax = 0.6f;
};

AppSettings gAppSettings;

// default values = not POD.
//static_assert(std::is_pod<AppSettings>::value, "Settings must be a POD to be reliably serializable");


HarmPreset& FindHarmPreset(bool enabled, size_t id) {
  static bool defaultInitialized = false;
  static HarmPreset defaultHarmPreset;
  if (!defaultInitialized) {
    defaultHarmPreset.mVoiceSettings[0].mVoiceType = HarmVoiceType::Interval;
    defaultHarmPreset.mVoiceSettings[0].mSequence[0].mInterval = 0;
    defaultHarmPreset.mVoiceSettings[0].mSequence[0].mEnd = true;
    defaultHarmPreset.mVoiceSettings[0].mScaleRef = ScaleRefType::Local;
    defaultHarmPreset.mVoiceSettings[0].mLocalScale = Scale { ScaleFlavorIndex::Chromatic, 0 };
  }
  if (!enabled) {
    return defaultHarmPreset;
  }
  CCASSERT(id > 0 && id <= SizeofStaticArray(gAppSettings.mHarmSettings.mPresets));
  return gAppSettings.mHarmSettings.mPresets[id];
}

