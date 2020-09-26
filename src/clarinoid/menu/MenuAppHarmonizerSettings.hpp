#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"

size_t gEditingHarmVoice = 0;
size_t gEditingHarmPreset = 0;

struct HarmVoiceSettingsApp
{
  static HarmVoiceSettings& EditingVoice() { return gAppSettings.mHarmSettings.mPresets[gEditingHarmPreset].mVoiceSettings[gEditingHarmVoice]; }

  EnumSettingItem<HarmSynthPresetRefType> mSynthPresetRef = { "Synth preset ref", gHarmSynthPresetRefTypeInfo, Property<HarmSynthPresetRefType> {
    [](void* cap) { return EditingVoice().mSynthPresetRef; }, // getter
    [](void* cap, const HarmSynthPresetRefType& val) { EditingVoice().mSynthPresetRef = val; }, // setter
    this // capture val
    }, AlwaysEnabled};
  IntSettingItem mMinRotationTimeMS = { "Synth Preset", 0, SYNTH_PRESET_COUNT - 1, Property<int> {
    [](void* cap) { return (int)EditingVoice().mVoiceSynthPreset; }, // getter
    [](void* cap, const int& val) { EditingVoice().mVoiceSynthPreset = val; }, // setter
    this // capture val
    }, AlwaysEnabled };

  // HarmScaleRefType mScaleRef = HarmScaleRefType::Global;
  // Scale mLocalScale;
  // uint8_t mMinOutpNote = 0;
  // uint8_t mMaxOutpNote = 127;
  // NoteOOBBehavior mNoteOOBBehavior = NoteOOBBehavior::TransposeOctave;
  // NonDiatonicBehavior mNonDiatonicBehavior = NonDiatonicBehavior::NextDiatonicNote;

  // int8_t mSequence[HARM_SEQUENCE_LEN] = { HarmVoiceSequenceEntry_END };
  // uint8_t mSequenceLength = 0;

  ISettingItem* mArray[2] =
  {
    &mSynthPresetRef, &mMinRotationTimeMS
  };
  SettingsList mRootList = { mArray };
};

HarmVoiceSettingsApp gHarmVoiceSettingsApp;

struct HarmPatchSettingsApp
{
  static HarmPreset& EditingPreset() { return gAppSettings.mHarmSettings.mPresets[gEditingHarmPreset]; }

  StringSettingItem mName = { [](){ return gAppSettings.mHarmSettings.mPresets[gEditingHarmPreset].mName; }, AlwaysEnabled };
  BoolSettingItem mEmitLiveNote = { "Emit live note?", "Yes", "No", Property<bool> {
    [](void* cap) { return EditingPreset().mEmitLiveNote; }, // getter
    [](void* cap, const bool& val) { EditingPreset().mEmitLiveNote = val; }, // setter
    this // capture val
    }, AlwaysEnabled };
  IntSettingItem mMinRotationTimeMS = { "Min Rotation MS", 0, 10000, Property<int> {
    [](void* cap) { return (int)EditingPreset().mMinRotationTimeMS; }, // getter
    [](void* cap, const int& val) { EditingPreset().mMinRotationTimeMS = val; }, // setter
    this // capture val
    }, AlwaysEnabled };
  IntSettingItem mSynthPreset1 = { "Synth preset 1", 0, SYNTH_PRESET_COUNT - 1,Property<int> {
    [](void* cap) { return (int)EditingPreset().mSynthPreset1; }, // getter
    [](void* cap, const int& val) { EditingPreset().mSynthPreset1 = val; }, // setter
    this // capture val
    }, AlwaysEnabled };
  IntSettingItem mSynthPreset2 = { "Synth preset 2", 0, SYNTH_PRESET_COUNT - 1, Property<int> {
    [](void* cap) { return (int)EditingPreset().mSynthPreset2; }, // getter
    [](void* cap, const int& val) { EditingPreset().mSynthPreset2 = val; }, // setter
    this // capture val
    }, AlwaysEnabled };
  IntSettingItem mSynthPreset3 = { "Synth preset 3", 0, SYNTH_PRESET_COUNT - 1, Property<int> {
    [](void* cap) { return (int)EditingPreset().mSynthPreset3; }, // getter
    [](void* cap, const int& val) { EditingPreset().mSynthPreset3 = val; }, // setter
    this // capture val
    }, AlwaysEnabled };

  MultiSubmenuSettingItem mVoiceSubmenu = {
    HARM_VOICES,
    [](void* cap,size_t mi) { return (String)(String("Voice ") + mi); }, // return string name
    [](void* cap,size_t mi) { gEditingHarmVoice = mi; return &gHarmVoiceSettingsApp.mRootList; }, // return submenu
    [](void* cap,size_t mi) { return true; },
    this
  };

  ISettingItem* mArray[6] =
  {
    &mEmitLiveNote,
    &mMinRotationTimeMS,
    &mSynthPreset1,
    &mSynthPreset2,
    &mSynthPreset3,
    &mVoiceSubmenu,
  };
  SettingsList mRootList = { mArray };
};

HarmPatchSettingsApp gHarmPatchSettings;

struct HarmSettingsApp : public SettingsMenuApp
{
  //LabelSettingItem mNothing = { []() {return String("nothing"); }, AlwaysEnabled };

  MultiSubmenuSettingItem mPatchSubmenu = {
    HARM_PRESET_COUNT,
    [](void*,size_t mi) { return (String)(String("") + mi + ":" + gAppSettings.mHarmSettings.mPresets[mi].mName); },
    [](void*,size_t mi) { gEditingHarmPreset = mi; return &gHarmPatchSettings.mRootList; },
    [](void*,size_t mi){ return true; },
    this
  };

  ISettingItem* mArray[1] =
  {
    //&mNothing
    &mPatchSubmenu
  };
  SettingsList mRootList = { mArray };

  virtual SettingsList* GetRootSettingsList()
  {
    return &mRootList;
  }

  virtual void RenderFrontPage() 
  {
    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setTextColor(WHITE);
    gDisplay.mDisplay.setCursor(0,0);
    gDisplay.mDisplay.println("Harmonizer Settings");
    gDisplay.mDisplay.println(String(""));
    gDisplay.mDisplay.println(String(""));
    gDisplay.mDisplay.println(String("                  -->"));

    SettingsMenuApp::RenderFrontPage();
  }

};



