
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{


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
  Mute,
  TransposeOctave
  // transposeoctave_but_drop_if_it_crosses_live
};

EnumItemInfo<NoteOOBBehavior> gNoteOOBBehaviorItems[2] = {
  { NoteOOBBehavior::Mute, "Mute" },
  { NoteOOBBehavior::TransposeOctave, "TransposeOctave" },
};

EnumInfo<NoteOOBBehavior> gNoteOOBBehaviorInfo ("NoteOOBBehavior", gNoteOOBBehaviorItems);



////////////////////////////////////////////////////
enum class HarmSynthPresetRefType : uint8_t
{
  Global,
  Preset1, // at the preset level i can imagine setting a bass, comp, fx synth presets. they can be used for multiple layers then.
  Preset2,
  Preset3,
  Preset4,
  Voice
};

EnumItemInfo<HarmSynthPresetRefType> gHarmSynthPresetRefTypeItems[6] = {
  { HarmSynthPresetRefType::Global, "Global" },
  { HarmSynthPresetRefType::Preset1, "Preset1" },
  { HarmSynthPresetRefType::Preset2, "Preset2" },
  { HarmSynthPresetRefType::Preset3, "Preset3" },
  { HarmSynthPresetRefType::Preset4, "Preset4" },
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

  String GetMenuDetailString() const {
    if (mSequenceLength < 1) return "<off>";
    String ret = "[";
    for (size_t i = 0; i < (size_t)mSequenceLength - 1; ++ i) {
      ret += mSequence[i];
      ret += ",";
    }
    ret += mSequence[mSequenceLength - 1];
    ret += "]";
    return ret;
  }
};

struct HarmPreset
{
  String mName = "<init>";
  bool mEmitLiveNote = true;
  HarmVoiceSettings mVoiceSettings[HARM_VOICES];
  uint32_t mMinRotationTimeMS = 150;
  uint16_t mSynthPreset1 = 1; // harm-friendly sync
  uint16_t mSynthPreset2 = 2; // harm-friendly tri
  uint16_t mSynthPreset3 = 3; // harm-friendly pulse
  uint16_t mSynthPreset4 = 4; // harm-friendly saw
};

struct HarmSettings
{
  HarmPreset mPresets[HARM_PRESET_COUNT];
  HarmPreset mDisabledPreset; // the preset that is used when harmonizer is disabled.

  HarmSettings() 
  {
    mPresets[1].mName = "Big";
    mPresets[1].mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
    mPresets[1].mVoiceSettings[0].mSequenceLength = 1;
    mPresets[1].mVoiceSettings[0].mSequence[0] = -7;
    mPresets[1].mVoiceSettings[0].mMaxOutpNote = 80;
    mPresets[1].mVoiceSettings[0].mMinOutpNote = 40;

    mPresets[1].mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
    mPresets[1].mVoiceSettings[1].mSequenceLength = 1;
    mPresets[1].mVoiceSettings[1].mSequence[0] = -11;
    mPresets[1].mVoiceSettings[1].mMaxOutpNote = 80;
    mPresets[1].mVoiceSettings[1].mMinOutpNote = 40;

    mPresets[1].mVoiceSettings[2].mSynthPresetRef = HarmSynthPresetRefType::Preset4;
    mPresets[1].mVoiceSettings[2].mSequenceLength = 2;
    mPresets[1].mVoiceSettings[2].mMaxOutpNote = 40;
    mPresets[1].mVoiceSettings[2].mSequence[0] = -4;
    mPresets[1].mVoiceSettings[2].mSequence[0] = -9;

    mPresets[2].mName = "maj inv2";
    mPresets[2].mVoiceSettings[0].mSequenceLength = 1;
    mPresets[2].mVoiceSettings[0].mSequence[0] = -3;

    mPresets[2].mVoiceSettings[1].mSequenceLength = 1;
    mPresets[2].mVoiceSettings[1].mSequence[0] = -5;
  }
};

//static constexpr auto harmsettingssize = sizeof(HarmSettings);

} // namespace clarinoid
