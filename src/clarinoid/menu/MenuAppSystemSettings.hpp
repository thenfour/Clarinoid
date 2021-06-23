#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"
#include "UnipolarCalibration.hpp"
#include "BoolSettingItem.hpp"

namespace clarinoid
{

/////////////////////////////////////////////////////////////////////////////////////////////////
struct SystemSettingsApp : SettingsMenuApp
{
    virtual const char *DisplayAppGetName() override
    {
        return "SystemSettingsApp";
    }
    size_t mBreathMappingIndex;
    size_t mPitchUpMappingIndex;
    size_t mPitchDownMappingIndex;
    cc::function<float(void *)>::ptr_t mRawBreathGetter;
    cc::function<float(void *)>::ptr_t mRawPitchBendGetter;
    void *mpCapture;

    SystemSettingsApp(CCDisplay &d,
                      size_t breathMappingIndex,
                      size_t pitchUpMappingIndex,
                      size_t pitchDownMappingIndex,
                      cc::function<float(void *)>::ptr_t rawBreathGetter,
                      cc::function<float(void *)>::ptr_t rawPitchBendGetter,
                      void *capture)
        : SettingsMenuApp(d), mBreathMappingIndex(breathMappingIndex), mPitchUpMappingIndex(pitchUpMappingIndex),
          mPitchDownMappingIndex(pitchDownMappingIndex), mRawBreathGetter(rawBreathGetter),
          mRawPitchBendGetter(rawPitchBendGetter), mpCapture(capture)
    {
    }

    BoolSettingItem mDimDisplay = {"Display dim?",
                                   "Yes",
                                   "No",
                                   Property<bool>{[](void *cap) {
                                                      auto pThis = (SystemSettingsApp *)cap;
                                                      return pThis->mAppSettings->mDisplayDim;
                                                  }, // getter
                                                  [](void *cap, const bool &x) {
                                                      auto pThis = (SystemSettingsApp *)cap;
                                                      pThis->mAppSettings->mDisplayDim = x;
                                                      pThis->mDisplay.mDisplay.dim(x);
                                                  },
                                                  this},
                                   AlwaysEnabled};

    UnipolarCalibrationSettingItem mBreath = {
        "Breath",
        Property<UnipolarMapping>{
            [](void *cap) {
                auto pThis = (SystemSettingsApp *)cap;
                UnipolarMapping ret =
                    pThis->mAppSettings->mControlMappings[pThis->mBreathMappingIndex].mUnipolarMapping;
                return ret;
            }, // getter
            [](void *cap, const UnipolarMapping &x) {
                auto pThis = (SystemSettingsApp *)cap;
                pThis->mAppSettings->mControlMappings[pThis->mBreathMappingIndex].mUnipolarMapping = x;
            },
            this},
        [](void *cap) {
            auto pThis = (SystemSettingsApp *)cap;
            return pThis->mRawBreathGetter(pThis->mpCapture);
        },
        this};

    UnipolarCalibrationSettingItem mPitchUp = {
        "Pitch Up",
        Property<UnipolarMapping>{
            [](void *cap) {
                auto pThis = (SystemSettingsApp *)cap;
                UnipolarMapping ret =
                    pThis->mAppSettings->mControlMappings[pThis->mPitchUpMappingIndex].mUnipolarMapping;
                return ret;
            }, // getter
            [](void *cap, const UnipolarMapping &x) {
                auto pThis = (SystemSettingsApp *)cap;
                pThis->mAppSettings->mControlMappings[pThis->mPitchUpMappingIndex].mUnipolarMapping = x;
            },
            this},
        [](void *cap) {
            auto pThis = (SystemSettingsApp *)cap;
            return pThis->mRawPitchBendGetter(pThis->mpCapture);
        },
        this};

    UnipolarCalibrationSettingItem mPitchDown = {
        "Pitch Down",
        Property<UnipolarMapping>{
            [](void *cap) {
                auto pThis = (SystemSettingsApp *)cap;
                UnipolarMapping ret =
                    pThis->mAppSettings->mControlMappings[pThis->mPitchDownMappingIndex].mUnipolarMapping;
                return ret;
            }, // getter
            [](void *cap, const UnipolarMapping &x) {
                auto pThis = (SystemSettingsApp *)cap;
                pThis->mAppSettings->mControlMappings[pThis->mPitchDownMappingIndex].mUnipolarMapping = x;
            },
            this},
        [](void *cap) {
            auto pThis = (SystemSettingsApp *)cap;
            return pThis->mRawPitchBendGetter(pThis->mpCapture);
        },
        this};

    ISettingItem *mArray[4] = {
        &mDimDisplay,
        &mBreath,
        &mPitchUp,
        &mPitchDown,
    };

    SettingsList mRootList = {mArray};

  public:
    virtual SettingsList *GetRootSettingsList()
    {
        return &mRootList;
    }

    virtual void RenderFrontPage()
    {
        mDisplay.mDisplay.setTextSize(1);
        mDisplay.mDisplay.setTextColor(WHITE);
        mDisplay.mDisplay.setCursor(0, 0);

        mDisplay.mDisplay.println(String("System Settings"));
        SettingsMenuApp::RenderFrontPage();
    }
};

} // namespace clarinoid
