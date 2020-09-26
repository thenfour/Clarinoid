
#pragma once

#include <clarinoid/basic/Basic.hpp>



enum class HarmScaleRefType : uint8_t
{
  Global,
  //Preset, // you don't really need a scale at the preset level. set it globally, or use local scales for things like chromatic etc.
  Voice,
};

EnumItemInfo<HarmScaleRefType> gHarmScaleRefTypeItems[2] = {
  { HarmScaleRefType::Global, "Global" },
  { HarmScaleRefType::Voice, "Voice" },
};

EnumInfo<HarmScaleRefType> gHarmScaleRefTypeInfo ("HarmScaleRefType", gHarmScaleRefTypeItems);



////////////////////////////////////////////////////
enum class NonDiatonicBehavior : uint8_t
{
  NextDiatonicNote,
  PrevDiatonicNote,
  PreferStay, // keep playing the same note, if it's within range. otherwise change to the next.
  PreferMove, // change to the next note (think cantaloupe comp that passing B)
  Drop, // just don't play this note.
  //FollowMelodyFromBelow, // so this voice plays a nondiatonic note too, based on distance from lower note
  //FollowMelodyFromAbove, // so this voice plays a nondiatonic note too, based on distance from upper note
  TryAlternateScale, // could be interesting to have a list of alternative scales to try. need to have a LUT of alternative scales or maybe even just use the scale follower's LUT?
};

EnumItemInfo<NonDiatonicBehavior> gNonDiatonicBehaviorItems[6] = {
  { NonDiatonicBehavior::NextDiatonicNote, "NextDiatonicNote" },
  { NonDiatonicBehavior::PrevDiatonicNote, "PrevDiatonicNote" },
  { NonDiatonicBehavior::PreferStay, "PreferStay" },
  { NonDiatonicBehavior::PreferMove, "PreferMove" },
  { NonDiatonicBehavior::Drop, "Drop" },
  { NonDiatonicBehavior::TryAlternateScale, "TryAlternateScale" },
};

EnumInfo<NonDiatonicBehavior> gNonDiatonicBehaviorInfo ("NonDiatonicBehavior", gNonDiatonicBehaviorItems);



////////////////////////////////////////////////////
enum class NoteOOBBehavior : uint8_t
{
  Drop,
  TransposeOctave
  // transposeoctave_but_drop_if_it_crosses_live
};

////////////////////////////////////////////////////
enum class HarmSynthPresetRefType : uint8_t
{
  Global,
  Preset1, // at the preset level i can imagine setting a bass, comp, fx synth presets. they can be used for multiple layers then.
  Preset2,
  Preset3,
  Voice
};

EnumItemInfo<HarmSynthPresetRefType> gHarmSynthPresetRefTypeItems[5] = {
  { HarmSynthPresetRefType::Global, "Global" },
  { HarmSynthPresetRefType::Preset1, "Preset1" },
  { HarmSynthPresetRefType::Preset2, "Preset2" },
  { HarmSynthPresetRefType::Preset3, "Preset3" },
  { HarmSynthPresetRefType::Voice, "Voice" },
};

EnumInfo<HarmSynthPresetRefType> gHarmSynthPresetRefTypeInfo ("HarmSynthPresetRefType", gHarmSynthPresetRefTypeItems);


////////////////////////////////////////////////////
struct HarmVoiceSettings
{
  int8_t mSequence[HARM_SEQUENCE_LEN] = { 0 };
  uint8_t mSequenceLength = 0;

  HarmSynthPresetRefType mSynthPresetRef = HarmSynthPresetRefType::Global;
  uint16_t mVoiceSynthPreset = 0;

  HarmScaleRefType mScaleRef = HarmScaleRefType::Global;
  Scale mLocalScale = { 0, ScaleFlavorIndex::Chromatic };
  uint8_t mMinOutpNote = 0;
  uint8_t mMaxOutpNote = 127;
  NoteOOBBehavior mNoteOOBBehavior = NoteOOBBehavior::TransposeOctave;
  NonDiatonicBehavior mNonDiatonicBehavior = NonDiatonicBehavior::NextDiatonicNote;
};

struct HarmPreset
{
  String mName = "<init>";
  bool mEmitLiveNote = true;
  HarmVoiceSettings mVoiceSettings[HARM_VOICES];
  uint32_t mMinRotationTimeMS = 150;
  uint16_t mSynthPreset1 = 0;
  uint16_t mSynthPreset2 = 1;
  uint16_t mSynthPreset3 = 2;
};

struct HarmSettings
{
  HarmPreset mPresets[HARM_PRESET_COUNT];
  HarmPreset mDisabledPreset; // the preset that is used when harmonizer is disabled.

  HarmSettings() 
  {
    mPresets[1].mName = "sample";
    mPresets[1].mVoiceSettings[0].mSequenceLength = 1;
    mPresets[1].mVoiceSettings[0].mSequence[0] = -2;

    mPresets[1].mVoiceSettings[1].mSequenceLength = 2;
    mPresets[1].mVoiceSettings[1].mSequence[0] = -3;
    mPresets[1].mVoiceSettings[1].mSequence[1] = -4;
  }
};

static constexpr auto harmsettingssize = sizeof(HarmSettings);

