#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"




struct LoopSettingsApp : public SettingsMenuApp
{
  TriggerSettingItem mClearAll = { "Clear All", [&](){ gEWIControl.mMusicalState.mLooper.Clear(); }, AlwaysEnabled };
  TriggerSettingItem mLoopIt = { "Loop it", [&](){ gEWIControl.mMusicalState.mLooper.LoopIt(gEWIControl.mMusicalState.mLiveVoice); }, AlwaysEnabled };

  ISettingItem* mArray[2] =
  {
    &mClearAll, &mLoopIt
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
    gDisplay.mDisplay.println(String("Looper [") + (gAppSettings.mHarmSettings.mIsEnabled ? "on" : "off") + "]");
    gDisplay.mDisplay.println(String("Scale: "));
    gDisplay.mDisplay.println(String("Preset: "));
    gDisplay.mDisplay.println(String("                  -->"));

    SettingsMenuApp::RenderFrontPage();
  }

};
