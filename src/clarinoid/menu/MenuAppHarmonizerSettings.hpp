#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"


struct HarmVoiceSettingsApp
{
  HarmVoiceSettings* mpBinding = nullptr;

  EnumSettingItem<HarmVoiceType> mVoiceType = { "Voice Type", gHarmVoiceTypeInfo, mpBinding->mVoiceType, AlwaysEnabled };
  IntSettingItem mInterval = { "Interval", -48, 48, MakePropertyByCasting<int>(&mpBinding->mSequence[0]), AlwaysEnabled };

  ISettingItem* mArray[2] =
  {
    &mVoiceType, &mInterval
  };
  SettingsList mRootList = { mArray };

  HarmVoiceSettingsApp(HarmVoiceSettings& binding) :
    mpBinding(&binding)
  {
  }
};

struct HarmPatchSettingsApp
{
  HarmPreset* mpBinding = nullptr;
  HarmPatchSettingsApp(HarmPreset& binding) :
    mpBinding(&binding)
  {
  }

  HarmVoiceSettingsApp mVoiceSettings = { mpBinding->mVoiceSettings[0] };
  // [HARM_VOICES] = {
  //   { mpBinding->mVoiceSettings[0] },
  //   { mpBinding->mVoiceSettings[1] },
  //   { mpBinding->mVoiceSettings[2] },
  //   { mpBinding->mVoiceSettings[3] },
  //   { mpBinding->mVoiceSettings[4] },
  //   { mpBinding->mVoiceSettings[5] },
  // };
  
  //BoolSettingItem mIsEnabled = { "Enabled?", "On", "Off", gAppSettings.mHarmSettings.mIsEnabled, AlwaysEnabled };
  IntSettingItem mMinRotationTimeMS = { "Min Rotation MS", 0, 10000, MakePropertyByCasting<int>(&mpBinding->mMinRotationTimeMS), AlwaysEnabled };
  BoolSettingItem mEmitLiveNote = { "Emit live note?", "Yes", "No", mpBinding->mEmitLiveNote, AlwaysEnabled };

  MultiSubmenuSettingItem mVoiceSubmenu = { HARM_VOICES, [](size_t mi) { return (String)(String("Voice ") + mi); }, &mVoiceSettings.mRootList, [](size_t mi) {return true;} };
  // SubmenuSettingItem mVoiceSubmenu2 = { "Voice2", &mVoiceSettings[1].mRootList, AlwaysEnabled };
  // SubmenuSettingItem mVoiceSubmenu3 = { "Voice3", &mVoiceSettings[2].mRootList, AlwaysEnabled };
  // SubmenuSettingItem mVoiceSubmenu4 = { "Voice4", &mVoiceSettings[3].mRootList, AlwaysEnabled };
  // SubmenuSettingItem mVoiceSubmenu5 = { "Voice5", &mVoiceSettings[4].mRootList, AlwaysEnabled };
  // SubmenuSettingItem mVoiceSubmenu6 = { "Voice6", &mVoiceSettings[5].mRootList, AlwaysEnabled };

  ISettingItem* mArray[3] =
  {
    &mMinRotationTimeMS, &mEmitLiveNote, &mVoiceSubmenu,
    // &mVoiceSubmenu2,
    //  &mVoiceSubmenu3, &mVoiceSubmenu4, &mVoiceSubmenu5,
    //   &mVoiceSubmenu6
  };
  SettingsList mRootList = { mArray };


};



struct HarmSettingsApp : public SettingsMenuApp
{
  HarmPatchSettingsApp mPatchSettings = { gAppSettings.mHarmSettings.mPresets[0] };
  // [HARM_PRESET_COUNT] = {
  //   { gAppSettings.mHarmSettings.mPresets[0] },
  //   { gAppSettings.mHarmSettings.mPresets[1] },
  //   { gAppSettings.mHarmSettings.mPresets[2] },
  //   { gAppSettings.mHarmSettings.mPresets[3] },
  //   { gAppSettings.mHarmSettings.mPresets[4] },
  //   { gAppSettings.mHarmSettings.mPresets[5] },
  //   { gAppSettings.mHarmSettings.mPresets[6] },
  //   { gAppSettings.mHarmSettings.mPresets[7] },
  //   { gAppSettings.mHarmSettings.mPresets[8] },
  //   { gAppSettings.mHarmSettings.mPresets[9] },
  //   { gAppSettings.mHarmSettings.mPresets[10] },
  //   { gAppSettings.mHarmSettings.mPresets[11] },
  //   { gAppSettings.mHarmSettings.mPresets[12] },
  //   { gAppSettings.mHarmSettings.mPresets[13] },
  //   { gAppSettings.mHarmSettings.mPresets[14] },
  //   { gAppSettings.mHarmSettings.mPresets[15] },
  // };
  
  BoolSettingItem mIsEnabled = { "Enabled?", "On", "Off", gAppSettings.mHarmSettings.mIsEnabled, AlwaysEnabled };
  //IntSettingItem mGlobalSynthPreset = { "Global synth preset", 0, SYNTH_PRESET_COUNT - 1, gAppSettings.mHarmSettings.mGlobalSynthPreset, AlwaysEnabled };
  //Scale mGlobalScale;
  //IntSettingItem mMinRotationTimeMS = { "Min Rotation", 0, 10000, gAppSettings.mHarmSettings.mMinRotationTimeMS, AlwaysEnabled };

  MultiSubmenuSettingItem mPatchSubmenu = { HARM_PRESET_COUNT, [](size_t mi) { return (String)(String("Preset ") + mi); }, &mPatchSettings.mRootList, [](size_t){return true;} };
  // SubmenuSettingItem mPatchSubmenu01 = { "Preset 01", &mPatchSettings[1].mRootList, AlwaysEnabled };
  // SubmenuSettingItem mPatchSubmenu02 = { "Preset 02", &mPatchSettings[2].mRootList, AlwaysEnabled };
  // SubmenuSettingItem mPatchSubmenu03 = { "Preset 03", &mPatchSettings[3].mRootList, AlwaysEnabled };
  // SubmenuSettingItem mPatchSubmenu04 = { "Preset 04", &mPatchSettings[4].mRootList, AlwaysEnabled };
  // SubmenuSettingItem mPatchSubmenu05 = { "Preset 05", &mPatchSettings[5].mRootList, AlwaysEnabled };
  // SubmenuSettingItem mPatchSubmenu06 = { "Preset 06", &mPatchSettings[6].mRootList, AlwaysEnabled };
  // SubmenuSettingItem mPatchSubmenu07 = { "Preset 07", &mPatchSettings[7].mRootList, AlwaysEnabled };
  // SubmenuSettingItem mPatchSubmenu08 = { "Preset 08", &mPatchSettings[8].mRootList, AlwaysEnabled };
  // SubmenuSettingItem mPatchSubmenu09 = { "Preset 09", &mPatchSettings[9].mRootList, AlwaysEnabled };
  // SubmenuSettingItem mPatchSubmenu10 = { "Preset 10", &mPatchSettings[10].mRootList, AlwaysEnabled };
  // SubmenuSettingItem mPatchSubmenu11 = { "Preset 11", &mPatchSettings[11].mRootList, AlwaysEnabled };
  // SubmenuSettingItem mPatchSubmenu12 = { "Preset 12", &mPatchSettings[12].mRootList, AlwaysEnabled };
  // SubmenuSettingItem mPatchSubmenu13 = { "Preset 13", &mPatchSettings[13].mRootList, AlwaysEnabled };
  // SubmenuSettingItem mPatchSubmenu14 = { "Preset 14", &mPatchSettings[14].mRootList, AlwaysEnabled };
  // SubmenuSettingItem mPatchSubmenu15 = { "Preset 15", &mPatchSettings[15].mRootList, AlwaysEnabled };

  ISettingItem* mArray[2] =
  {
    &mIsEnabled,// &mGlobalSynthPreset, &mMinRotationTimeMS,
    &mPatchSubmenu
    // &mPatchSubmenu01,
    // &mPatchSubmenu02,
    // &mPatchSubmenu03,
    // &mPatchSubmenu04,
    // &mPatchSubmenu05,
    // &mPatchSubmenu06,
    // &mPatchSubmenu07,
    // &mPatchSubmenu08,
    // &mPatchSubmenu09,
    // &mPatchSubmenu10,
    // &mPatchSubmenu11,
    // &mPatchSubmenu12,
    // &mPatchSubmenu13,
    // &mPatchSubmenu14,
    // &mPatchSubmenu15,
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
    gDisplay.mDisplay.println(String("Harmonizer [") + (gAppSettings.mHarmSettings.mIsEnabled ? "on" : "off") + "]");
    gDisplay.mDisplay.println(String("Scale: "));
    gDisplay.mDisplay.println(String("Preset: "));
    gDisplay.mDisplay.println(String("                  -->"));

    SettingsMenuApp::RenderFrontPage();
  }

};



