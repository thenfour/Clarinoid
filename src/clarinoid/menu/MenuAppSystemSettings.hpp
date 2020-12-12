#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"
#include "BreathSettingItem.hpp"
#include "BoolSettingItem.hpp"

namespace clarinoid
{

/////////////////////////////////////////////////////////////////////////////////////////////////
struct SystemSettingsApp :
    SettingsMenuApp
{
  virtual const char *DisplayAppGetName() override { return "SystemSettingsApp"; }

    SystemSettingsApp(CCDisplay& d) : SettingsMenuApp(d) {}

  BoolSettingItem mDimDisplay = { "Display dim?", "Yes", "No",
    Property<bool>{
        [](void* cap) { auto pThis = (SystemSettingsApp*)cap; return pThis->mAppSettings->mDisplayDim; }, // getter
        [](void* cap, const bool& x) { auto pThis = (SystemSettingsApp*)cap; pThis->mAppSettings->mDisplayDim = x; pThis->mDisplay.mDisplay.dim(x); },
        this
        },
    AlwaysEnabled
    };

    // BreathCalibrationSettingItem mBreath = {
    //   Property<BreathCalibrationSettings> {
    //     [](void* cap) { auto pThis = (SystemSettingsApp*)cap; return pThis->mAppSettings->mBreathCalibration; }, // getter
    //     [](void* cap, const BreathCalibrationSettings& x) { auto pThis = (SystemSettingsApp*)cap; pThis->mAppSettings->mBreathCalibration = x; },
    //     this
    //   },
    //   [](void* cap) {
    //     auto pThis = (SystemSettingsApp*)cap;
    //     return pThis->mInput->mBreath.CurrentValue01();
    //   },
    //   this
    // };

  ISettingItem* mArray[1] =
  {
    &mDimDisplay,
    //&mBreath,
  };
  SettingsList mRootList = { mArray };

public:

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
