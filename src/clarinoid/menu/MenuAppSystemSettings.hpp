#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"

/////////////////////////////////////////////////////////////////////////////////////////////////
class SystemSettingsApp : public SettingsMenuApp
{
  TriggerSettingItem mResetKeys = { "Reset all keys", [&](){ this->OnResetKeys(); }, AlwaysEnabled };
  FloatSettingItem mTouchMaxFactor = { "Touch max fact", 1, 2, Property<float> {
    [&]() { return gAppSettings.mTouchMaxFactor; },
    [&](const float& f)
      {
        gAppSettings.mTouchMaxFactor = f;
        gApp.SendCmd(CommandFromMain::SetTouchMaxFactor, f);
      }
  }, AlwaysEnabled };
  
  FloatSettingItem mPitchDownMin = { "Pitchdown min", 0, 1, gAppSettings.mPitchDownMin, AlwaysEnabled };
  FloatSettingItem mPitchDownMax = { "Pitchdown max", 0, 1, gAppSettings.mPitchDownMax, AlwaysEnabled };

  FloatSettingItem mBreathLowerBound = { "Breath inp min", 0, 1, gAppSettings.mBreathLowerBound, AlwaysEnabled };
  FloatSettingItem mBreathUpperBound = { "Breath inp max", 0, 1, gAppSettings.mBreathUpperBound, AlwaysEnabled };
  FloatSettingItem mBreathNoteOnThreshold = { "Breath note on thresh", 0, 1, gAppSettings.mBreathNoteOnThreshold, AlwaysEnabled };

  BoolSettingItem mDimDisplay = { "Display dim?", "Yes", "No", Property<bool>{ [&]() { return gAppSettings.mDisplayDim; },
    [&](const bool& x) { gAppSettings.mDisplayDim = x; gDisplay.mDisplay.dim(x); }}, AlwaysEnabled };

  BoolSettingItem mOrangeLEDs = { "Orange LEDs?", "On", "Off", Property<bool>{
    [&]() { return gAppSettings.mOrangeLEDs; },
    [&](const bool& x)
      {
        gAppSettings.mOrangeLEDs = x;
        gApp.SendCmd(x ? CommandFromMain::EnableOrangeLED : CommandFromMain::DisableOrangeLED);
      }
    }, AlwaysEnabled };


  ISettingItem* mArray[9] =
  {
    &mResetKeys, &mTouchMaxFactor, &mPitchDownMin,
    &mPitchDownMax, &mBreathLowerBound, &mBreathUpperBound,
    &mBreathNoteOnThreshold, &mDimDisplay, &mOrangeLEDs
  };
  SettingsList mRootList = { mArray };

public:

  void OnResetKeys() {
    gApp.SendCmd(CommandFromMain::ResetTouchKeys);
    gDisplay.ShowToast("Reset done");
  }

  virtual SettingsList* GetRootSettingsList()
  {
    return &mRootList;
  }

  virtual void RenderFrontPage() 
  {
    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setTextColor(WHITE);
    gDisplay.mDisplay.setCursor(0,0);

    gDisplay.mDisplay.println(String("SYSTEM SETTINGS"));
    SettingsMenuApp::RenderFrontPage();
  }
};

