
#ifndef CCMENU_H
#define CCMENU_H

#include "Shared_CCUtil.h"
#include "CCMenuDebug.h"








// scale setting item
// scale editor
// grayed

struct HarmVoiceSettingsApp
{
  HarmVoiceSettings* mpBinding = nullptr;
  SettingsList mList;

  EnumSettingItem<HarmVoiceType> mVoiceType = { mList, "Voice Type", gHarmVoiceTypeInfo, mpBinding->mVoiceType };
  IntSettingItem mInterval = { mList, "Interval", -48, 48, MakePropertyByCasting<int>(mpBinding->mSequence[0].mInterval) };

//  HarmVoiceSequenceEntry mSequence[HARM_SEQUENCE_LEN];
//  GlobalLocal mSynthPresetRef = GlobalLocal::Global;
//  int mLocalSynthPreset = 0;
//  GlobalLocal mScaleRef = GlobalLocal::Global;
//  Scale mLocalScale;
//  IntSettingItem mMinOutpNote;
//  IntSettingItem mMaxOutpNote;
//  NoteOOBBehavior mNoteOOBBehavior = NoteOOBBehavior::Drop;
//  NonDiatonicBehavior mNonDiatonicBehavior = NonDiatonicBehavior::Drop;
//  IntSettingItem mMinOutpVel;
//  IntSettingItem mMaxOutpVel;
  
  HarmVoiceSettingsApp(HarmVoiceSettings& binding) :
    mpBinding(&binding)
  {
  }
};

class HarmSettingsApp : public SettingsMenuApp
{
  SettingsList mRootList;
  HarmVoiceSettingsApp mVoiceSettings[HARM_VOICES] = {
    { gAppSettings.mHarmSettings.mVoiceSettings[0] },
    { gAppSettings.mHarmSettings.mVoiceSettings[1] },
    { gAppSettings.mHarmSettings.mVoiceSettings[2] },
    { gAppSettings.mHarmSettings.mVoiceSettings[3] },
  };
  
  BoolSettingItem mIsEnabled = { mRootList, "Enabled?", "On", "Off", gAppSettings.mHarmSettings.mIsEnabled };
  SubmenuSettingItem mVoiceSubmenu1 = { mRootList, "Voice1", &mVoiceSettings[0].mList };
  SubmenuSettingItem mVoiceSubmenu2 = { mRootList, "Voice2", &mVoiceSettings[1].mList };
  SubmenuSettingItem mVoiceSubmenu3 = { mRootList, "Voice3", &mVoiceSettings[2].mList };
  SubmenuSettingItem mVoiceSubmenu4 = { mRootList, "Voice4", &mVoiceSettings[3].mList };
  IntSettingItem mGlobalSynthPreset = { mRootList, "Global synth preset", 0, SYNTH_PRESET_COUNT - 1, gAppSettings.mHarmSettings.mGlobalSynthPreset };
  IntSettingItem mMinRotationTimeMS = { mRootList, "Min Rotation", 0, 10000, gAppSettings.mHarmSettings.mMinRotationTimeMS};

//  int mGlobalSynthPreset;
//  int mMinRotationTimeMS;
//  Scale mGlobalScale;

public:
  virtual SettingsList* GetRootSettingsList()
  {
    return &mRootList;
  }

  virtual void RenderFrontPage() 
  {
    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setTextColor(WHITE);
    gDisplay.mDisplay.setCursor(0,0);
    gDisplay.mDisplay.println(String("~~Harmonizer~~ [") + (gAppSettings.mHarmSettings.mIsEnabled ? "on" : "off") + "]");
    gDisplay.mDisplay.println(String("Scale: "));
    gDisplay.mDisplay.println(String("Preset: "));
    gDisplay.mDisplay.println(String("                  -->"));

    SettingsMenuApp::RenderFrontPage();
  }

//  virtual ISettingItemEditor* GetBackEditor() {
//    return mBPM.GetEditor();
//  }
};


#endif
