#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"

namespace clarinoid
{

/////////////////////////////////////////////////////////////////////////////////////////////////
struct SystemSettingsApp :
    SettingsMenuApp
{
  virtual const char *DisplayAppGetName() override { return "SystemSettingsApp"; }

    SystemSettingsApp(CCDisplay& d) : SettingsMenuApp(d) {}
  //TriggerSettingItem mResetKeys = { "Reset all keys", [](void* capture){ auto pthis = (SystemSettingsApp*)capture; pthis->OnResetKeys(); }, this, AlwaysEnabled };
//   FloatSettingItem mTouchMaxFactor = { "Touch max fact", 1, 2, Property<float> {
//     [](void*) { return gAppSettings.mTouchMaxFactor; },
//     [](void*, const float& f)
//       {
//         gAppSettings.mTouchMaxFactor = f;
//         gApp.SendCmd(CommandFromMain::SetTouchMaxFactor, f);
//       },
//       nullptr
//   }, AlwaysEnabled };
  
  //FloatSettingItem mPitchDownMin = { "Pitchdown min", 0, 1, gAppSettings.mPitchDownMin, AlwaysEnabled };
  //FloatSettingItem mPitchDownMax = { "Pitchdown max", 0, 1, gAppSettings.mPitchDownMax, AlwaysEnabled };

//   FloatSettingItem mBreathLowerBound = { "Breath inp min", 0, 1, gAppSettings.mBreathLowerBound, AlwaysEnabled };
//   FloatSettingItem mBreathUpperBound = { "Breath inp max", 0, 1, gAppSettings.mBreathUpperBound, AlwaysEnabled };
//   FloatSettingItem mBreathNoteOnThreshold = { "Breath note on thresh", 0, 1, gAppSettings.mBreathNoteOnThreshold, AlwaysEnabled };

  BoolSettingItem mDimDisplay = { "Display dim?", "Yes", "No",
    Property<bool>{
        [](void* cap) { auto pThis = (SystemSettingsApp*)cap; return pThis->mAppSettings->mDisplayDim; }, // getter
        [](void* cap, const bool& x) { auto pThis = (SystemSettingsApp*)cap; pThis->mAppSettings->mDisplayDim = x; pThis->mDisplay.mDisplay.dim(x); },
        this
        },
    AlwaysEnabled
    };

    LabelSettingItem mL1 = { []() {return (String)"hi 1."; }, AlwaysEnabled };
    LabelSettingItem mL2 = { []() {return (String)"hi 2."; }, AlwaysEnabled };
    LabelSettingItem mL3 = { []() {return (String)"hi 3."; }, AlwaysEnabled };
    LabelSettingItem mL4 = { []() {return (String)"hi 4."; }, AlwaysEnabled };
    LabelSettingItem mL5 = { []() {return (String)"hi 5."; }, AlwaysEnabled };
    LabelSettingItem mL6 = { []() {return (String)"hi 6."; }, AlwaysEnabled };


//   BoolSettingItem mOrangeLEDs = { "Orange LEDs?", "On", "Off", Property<bool>{
//     [](void*) { return gAppSettings.mOrangeLEDs; },
//     [](void*, const bool& x)
//       {
//         gAppSettings.mOrangeLEDs = x;
//         gApp.SendCmd(x ? CommandFromMain::EnableOrangeLED : CommandFromMain::DisableOrangeLED);
//       }, nullptr
//     }, AlwaysEnabled };


  ISettingItem* mArray[7] =
  {
    //&mResetKeys, &mTouchMaxFactor,
    //&mPitchDownMin,
    //&mPitchDownMax,
    //&mBreathLowerBound, &mBreathUpperBound,
    //&mBreathNoteOnThreshold,
    &mDimDisplay,
    &mL1,
    &mL2,
    &mL3,
    &mL4,
    &mL5,
    &mL6,
    //, &mOrangeLEDs
  };
  SettingsList mRootList = { mArray };

public:

//   void OnResetKeys() {
//     gApp.SendCmd(CommandFromMain::ResetTouchKeys);
//     gDisplay.ShowToast("Reset done");
//   }

  virtual SettingsList* GetRootSettingsList()
  {
    return &mRootList;
  }

  virtual void RenderFrontPage() 
  {
    mDisplay.mDisplay.setTextSize(1);
    mDisplay.mDisplay.setTextColor(WHITE);
    mDisplay.mDisplay.setCursor(0,0);

    mDisplay.mDisplay.println(String("System Settings"));
    SettingsMenuApp::RenderFrontPage();
  }
};

} // namespace clarinoid
