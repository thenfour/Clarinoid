#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"
#include "UnipolarCalibration.hpp"
#include "BoolSettingItem.hpp"

namespace clarinoid
{

/////////////////////////////////////////////////////////////////////////////////////////////////
struct SystemSettingsApp :
    SettingsMenuApp
{
  virtual const char *DisplayAppGetName() override { return "SystemSettingsApp"; }
  size_t mBreathMappingIndex;
  size_t mJoyPitchMappingIndex;
  cc::function<float(void *)>::ptr_t mRawBreathGetter;
  cc::function<float(void *)>::ptr_t mRawPitchBendGetter;
  void* mpCapture;

    SystemSettingsApp(CCDisplay& d, size_t breathMappingIndex, size_t joyPitchMappingIndex, cc::function<float(void *)>::ptr_t rawBreathGetter, cc::function<float(void *)>::ptr_t rawPitchBendGetter, void* capture) :
      SettingsMenuApp(d),
      mBreathMappingIndex(breathMappingIndex),
      mJoyPitchMappingIndex(joyPitchMappingIndex),
      mRawBreathGetter(rawBreathGetter),
      mRawPitchBendGetter(rawPitchBendGetter),
      mpCapture(capture)
    {}

  BoolSettingItem mDimDisplay = { "Display dim?", "Yes", "No",
    Property<bool>{
        [](void* cap) { auto pThis = (SystemSettingsApp*)cap; return pThis->mAppSettings->mDisplayDim; }, // getter
        [](void* cap, const bool& x) { auto pThis = (SystemSettingsApp*)cap; pThis->mAppSettings->mDisplayDim = x; pThis->mDisplay.mDisplay.dim(x); },
        this
        },
    AlwaysEnabled
    };

    BreathCalibrationSettingItem mBreath = {
      "Breath",
      Property<UnipolarMapping> {
        [](void* cap) { auto pThis = (SystemSettingsApp*)cap; UnipolarMapping ret = pThis->mAppSettings->mControlMappings[pThis->mBreathMappingIndex].mNPolarMapping.Unipolar(); return ret; }, // getter
        [](void* cap, const UnipolarMapping& x) { auto pThis = (SystemSettingsApp*)cap; pThis->mAppSettings->mControlMappings[pThis->mBreathMappingIndex].mNPolarMapping.Unipolar() = x; },
        this
      },
      Property<float> {
        [](void* cap) { auto pThis = (SystemSettingsApp*)cap; return pThis->mAppSettings->mBreathNoteOnThreshold; }, // getter
        [](void* cap, const float& x) { auto pThis = (SystemSettingsApp*)cap; pThis->mAppSettings->mBreathNoteOnThreshold = x; },
        this
      },
      [](void* cap) {
        auto pThis = (SystemSettingsApp*)cap;
        return pThis->mRawBreathGetter(pThis->mpCapture);
        //return pThis->mInput->mBreath.CurrentValue01();
      },
      this
    };

// until we get REAL mappings, this is special cases per device.
#ifdef BASSOONOID1
    BreathCalibrationSettingItem mPitchBend = {
      "Pitch bend",
      Property<UnipolarMapping> {
        [](void* cap) { auto pThis = (SystemSettingsApp*)cap; UnipolarMapping ret = pThis->mAppSettings->mControlMappings[pThis->mJoyPitchMappingIndex].mNPolarMapping.Unipolar(); return ret; }, // getter
        [](void* cap, const UnipolarMapping& x) { auto pThis = (SystemSettingsApp*)cap; pThis->mAppSettings->mControlMappings[pThis->mJoyPitchMappingIndex].mNPolarMapping.Unipolar() = x; },
        this
      },
      Property<float> { // note on threshold... not relevent for pitch; for the moment just ignore it.
        [](void* cap) { /*auto pThis = (SystemSettingsApp*)cap; */return 0.0f; }, // getter
        [](void* cap, const float& x) { /*auto pThis = (SystemSettingsApp*)cap; */}, 
        this
      },
      [](void* cap) {
        auto pThis = (SystemSettingsApp*)cap;
        return pThis->mRawPitchBendGetter(pThis->mpCapture);
      },
      this
    };
#endif

#ifdef BASSOONOID1
  ISettingItem* mArray[3] =
  {
    &mDimDisplay,
    &mBreath,
    &mPitchBend,
  };
#else
  ISettingItem* mArray[2] =
  {
    &mDimDisplay,
    &mBreath,
  };
#endif

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
