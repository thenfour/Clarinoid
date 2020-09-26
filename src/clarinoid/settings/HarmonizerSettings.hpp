
#pragma once

#include <clarinoid/basic/Basic.hpp>



////////////////////////////////////////////////////
enum class NonDiatonicBehavior : uint8_t
{
  NextDiatonicNote,
  PrevDiatonicNote,
  //FollowMelodyFromBelow, // so this voice plays a nondiatonic note too, based on distance from lower note
  //FollowMelodyFromAbove, // so this voice plays a nondiatonic note too, based on distance from upper note
  Drop,
  //DontMove,
  //TryAlternateScale, // could be interesting to have a list of alternative scales to try.
};

static constexpr int8_t HarmVoiceSequenceEntry_END = -127;


// ////////////////////////////////////////////////////
// struct HarmVoiceSequenceEntry
// {
//   bool mEnd = true;
//   int8_t mInterval = 0;
// };

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
  int8_t mSequence[HARM_SEQUENCE_LEN];
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

  HarmSettings() 
  {
    mPresets[1].mEmitLiveNote = true;

    mPresets[1].mVoiceSettings[0].mScaleRef = ScaleRefType::Local;
    mPresets[1].mVoiceSettings[0].mLocalScale = Scale { 5, ScaleFlavorIndex::MinorPentatonic };
    mPresets[1].mVoiceSettings[0].mSequenceLength = 1;
    mPresets[1].mVoiceSettings[0].mVoiceType = HarmVoiceType::Sequence;
    mPresets[1].mVoiceSettings[0].mSequence[0] = -1;
    mPresets[1].mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Voice;

    mPresets[1].mVoiceSettings[1].mScaleRef = ScaleRefType::Local;
    mPresets[1].mVoiceSettings[1].mLocalScale = Scale { 5, ScaleFlavorIndex::MinorPentatonic };
    mPresets[1].mVoiceSettings[1].mSequenceLength = 2;
    mPresets[1].mVoiceSettings[1].mVoiceType = HarmVoiceType::Sequence;
    mPresets[1].mVoiceSettings[1].mSequence[0] = -2;
    mPresets[1].mVoiceSettings[1].mSequence[1] = -3;
    mPresets[1].mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Voice;

  }
};
