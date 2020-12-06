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

    BreathCalibrationSettingItem mBreath = {
      Property<BreathCalibrationSettings> {
        [](void* cap) { auto pThis = (SystemSettingsApp*)cap; return pThis->mAppSettings->mBreathCalibration; }, // getter
        [](void* cap, const BreathCalibrationSettings& x) { auto pThis = (SystemSettingsApp*)cap; pThis->mAppSettings->mBreathCalibration = x; },
        this
      },
      [](void* cap) {
        auto pThis = (SystemSettingsApp*)cap;
        return pThis->mControlMapper->BreathSensor()->CurrentValue01();
      },
      this
    };

    LabelSettingItem mL1 = { []() {return (String)"hi 1."; }, AlwaysEnabled };
    LabelSettingItem mL2 = { []() {return (String)"hi 2."; }, AlwaysEnabled };
    LabelSettingItem mL3 = { []() {return (String)"hi 3."; }, AlwaysEnabled };
    LabelSettingItem mL4 = { []() {return (String)"hi 4."; }, AlwaysEnabled };
    LabelSettingItem mL5 = { []() {return (String)"hi 5."; }, AlwaysEnabled };
    LabelSettingItem mL6 = { []() {return (String)"hi 6."; }, AlwaysEnabled };

  ISettingItem* mArray[7] =
  {
    &mDimDisplay,
    &mBreath,
    &mL2,
    &mL3,
    &mL4,
    &mL5,
    &mL6,
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
