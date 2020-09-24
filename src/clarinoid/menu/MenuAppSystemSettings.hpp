#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"

/////////////////////////////////////////////////////////////////////////////////////////////////
class SystemSettingsApp : public SettingsMenuApp
{
  TriggerSettingItem mResetKeys = { "Reset all keys", [](void* capture){ auto pthis = (SystemSettingsApp*)capture; pthis->OnResetKeys(); }, this, AlwaysEnabled };
  FloatSettingItem mTouchMaxFactor = { "Touch max fact", 1, 2, Property<float> {
    [](void*) { return gAppSettings.mTouchMaxFactor; },
    [](void*, const float& f)
      {
        gAppSettings.mTouchMaxFactor = f;
        gApp.SendCmd(CommandFromMain::SetTouchMaxFactor, f);
      },
      nullptr
  }, AlwaysEnabled };
  
  FloatSettingItem mPitchDownMin = { "Pitchdown min", 0, 1, gAppSettings.mPitchDownMin, AlwaysEnabled };
  FloatSettingItem mPitchDownMax = { "Pitchdown max", 0, 1, gAppSettings.mPitchDownMax, AlwaysEnabled };

  FloatSettingItem mBreathLowerBound = { "Breath inp min", 0, 1, gAppSettings.mBreathLowerBound, AlwaysEnabled };
  FloatSettingItem mBreathUpperBound = { "Breath inp max", 0, 1, gAppSettings.mBreathUpperBound, AlwaysEnabled };
  FloatSettingItem mBreathNoteOnThreshold = { "Breath note on thresh", 0, 1, gAppSettings.mBreathNoteOnThreshold, AlwaysEnabled };

  BoolSettingItem mDimDisplay = { "Display dim?", "Yes", "No", Property<bool>{ [](void*) { return gAppSettings.mDisplayDim; },
    [](void*, const bool& x) { gAppSettings.mDisplayDim = x; gDisplay.mDisplay.dim(x); }, nullptr}, AlwaysEnabled };

  BoolSettingItem mOrangeLEDs = { "Orange LEDs?", "On", "Off", Property<bool>{
    [](void*) { return gAppSettings.mOrangeLEDs; },
    [](void*, const bool& x)
      {
        gAppSettings.mOrangeLEDs = x;
        gApp.SendCmd(x ? CommandFromMain::EnableOrangeLED : CommandFromMain::DisableOrangeLED);
      }, nullptr
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

