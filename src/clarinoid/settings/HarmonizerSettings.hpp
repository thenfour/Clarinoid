
#pragma once

#include <clarinoid/basic/Basic.hpp>

////////////////////////////////////////////////////
struct HarmVoiceSequenceEntry
{
  bool mEnd = true;
  int8_t mInterval = 0;
};

////////////////////////////////////////////////////
enum class NoteOOBBehavior : uint8_t
{
  Drop,
  TransposeOctave
};

////////////////////////////////////////////////////
enum class HarmVoiceType : uint8_t
{
  Off,
  //Interval,
  Sequence
};

EnumItemInfo<HarmVoiceType> gHarmVoiceTypeItems[2] = {
  { HarmVoiceType::Off, "Off" },
  //{ HarmVoiceType::Interval, "Interval" }, // it means you only have 1 item in the sequence; it's for GUI purposes not internal.
  { HarmVoiceType::Sequence, "Sequence" },
};

EnumInfo<HarmVoiceType> gHarmVoiceTypeInfo (gHarmVoiceTypeItems);



////////////////////////////////////////////////////
enum class HarmSynthPresetRefType : uint8_t
{
  Global,
  //Harm,
  Voice
};

EnumItemInfo<HarmSynthPresetRefType> gHarmSynthPresetRefTypeItems[2] = {
  { HarmSynthPresetRefType::Global, "Global" },
  //{ HarmSynthPresetRefType::Harm, "Harm" },
  { HarmSynthPresetRefType::Voice, "Voice" },
};

EnumInfo<HarmSynthPresetRefType> gHarmSynthPresetRefTypeInfo (gHarmSynthPresetRefTypeItems);


////////////////////////////////////////////////////
// enum class HarmRotationTrigger : uint8_t
// {
//   Beat,
//   NoteOn,
//   NoteChange
// };




////////////////////////////////////////////////////
struct HarmVoiceSettings
{
  HarmVoiceType mVoiceType = HarmVoiceType::Off;
  HarmVoiceSequenceEntry mSequence[HARM_SEQUENCE_LEN];
  size_t mSequenceLength = 0;

  HarmSynthPresetRefType mSynthPresetRef = HarmSynthPresetRefType::Global;
  uint16_t mVoiceSynthPreset = 0;

  ScaleRefType mScaleRef = ScaleRefType::Global;
  Scale mLocalScale;
  int mMinOutpNote = 0;
  int mMaxOutpNote = 127;
  NoteOOBBehavior mNoteOOBBehavior = NoteOOBBehavior::Drop;
  NonDiatonicBehavior mNonDiatonicBehavior = NonDiatonicBehavior::Drop;
};

struct HarmPreset
{
  PresetName mName;
  bool mEmitLiveNote = true;
  HarmVoiceSettings mVoiceSettings[HARM_VOICES];
  uint32_t mMinRotationTimeMS;
};

struct HarmSettings
{
  bool mIsEnabled = false;
  HarmPreset mPresets[HARM_PRESET_COUNT];
};
