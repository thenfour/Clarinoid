#pragma once

#include <clarinoid/application/Metronome.hpp>

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"
#include "UnipolarCalibration.hpp"
#include "BoolSettingItem.hpp"
#include "NumericSettingItem.hpp"

namespace clarinoid
{

/////////////////////////////////////////////////////////////////////////////////////////////////
struct SystemSettingsApp : SettingsMenuApp
{
    virtual const char *DisplayAppGetName() override
    {
        return "SystemSettingsApp";
    }
    int mBreathMappingIndex;
    int mPitchUpMappingIndex;
    int mPitchDownMappingIndex;
    cc::function<float(void *)>::ptr_t mRawBreathGetter;
    cc::function<float(void *)>::ptr_t mRawPitchBendGetter;
    void *mpCapture;
    Metronome *mpMetronome = nullptr;

    SystemSettingsApp(IDisplay &d,
                      int breathMappingIndex,
                      int pitchUpMappingIndex,
                      int pitchDownMappingIndex,
                      cc::function<float(void *)>::ptr_t rawBreathGetter,
                      cc::function<float(void *)>::ptr_t rawPitchBendGetter,
                      void *capture,
                      Metronome *pm,
                      AppSettings &appSettings,
                      InputDelegator &input)
        : SettingsMenuApp(d, appSettings, input),        //
          mBreathMappingIndex(breathMappingIndex),       //
          mPitchUpMappingIndex(pitchUpMappingIndex),     //
          mPitchDownMappingIndex(pitchDownMappingIndex), //
          mRawBreathGetter(rawBreathGetter),             //
          mRawPitchBendGetter(rawPitchBendGetter),       //
          mpCapture(capture),                            //
          mpMetronome(pm)
    {
    }

    BoolSettingItem mDimDisplay = {"Display dim?",
                                   "Yes",
                                   "No",
                                   Property<bool>{[](void *cap) FLASHMEM {
                                                      auto pThis = (SystemSettingsApp *)cap;
                                                      return pThis->mAppSettings->mDisplayDim.GetValue();
                                                  }, // getter
                                                  [](void *cap, const bool &x) {
                                                      auto pThis = (SystemSettingsApp *)cap;
                                                      pThis->mAppSettings->mDisplayDim.SetValue(x);
                                                      pThis->mDisplay.dim(x);
                                                  },
                                                  this},
                                   AlwaysEnabled};

    BoolSettingItem mPedalPolarity = {"Pedal polarity",
                                      "+",
                                      "-",
                                      Property<bool>{[](void *cap) FLASHMEM {
                                                         auto pThis = (SystemSettingsApp *)cap;
                                                         return pThis->mAppSettings->mSustainPedalPolarity.GetValue();
                                                     }, // getter
                                                     [](void *cap, const bool &x) {
                                                         auto pThis = (SystemSettingsApp *)cap;
                                                         pThis->mAppSettings->mSustainPedalPolarity.SetValue(x);
                                                     },
                                                     this},
                                      AlwaysEnabled};

    // save settings...
    // [ ] system
    // [ ] performances
    // [ ] synth patches
    // [ ] harmonizer
    // back up existing,

    // Load settings ...
    // show file list

    // file explorer to delete

    TriggerSettingItem mExportToSerial{String("Export-> Serial"),
                                       [](void *cap) {
                                                         auto pThis = (SystemSettingsApp *)cap;

                                                         while(!Serial);
                                                         auto str = pThis->mAppSettings->SerializableObject_ToString();
                                                         Serial.println(str);
                                                         pThis->mDisplay.ShowToast(String("Exported ") + str.length() + " bytes.");
                                       },
                                       this,
                                       AlwaysEnabled};

    UnipolarCalibrationSettingItem mBreath = {
        "Breath",
        Property<UnipolarMapping>{
            [](void *cap) FLASHMEM {
                auto pThis = (SystemSettingsApp *)cap;
                if (pThis->mBreathMappingIndex < 0)
                {
                    return UnipolarMapping{};
                }
                UnipolarMapping ret =
                    pThis->mAppSettings->mControlMappings[pThis->mBreathMappingIndex].mUnipolarMapping;
                return ret;
            }, // getter
            [](void *cap, const UnipolarMapping &x) {
                auto pThis = (SystemSettingsApp *)cap;
                if (pThis->mBreathMappingIndex < 0)
                {
                    return;
                }
                pThis->mAppSettings->mControlMappings[pThis->mBreathMappingIndex].mUnipolarMapping = x;
            },
            this},
        [](void *cap) FLASHMEM {
            auto pThis = (SystemSettingsApp *)cap;
            return pThis->mRawBreathGetter(pThis->mpCapture);
        },
        this};

    UnipolarCalibrationSettingItem mPitchUp = {
        "Pitch Up",
        Property<UnipolarMapping>{
            [](void *cap) FLASHMEM {
                auto pThis = (SystemSettingsApp *)cap;
                if (pThis->mPitchUpMappingIndex < 0)
                {
                    return UnipolarMapping{};
                }
                UnipolarMapping ret =
                    pThis->mAppSettings->mControlMappings[pThis->mPitchUpMappingIndex].mUnipolarMapping;
                return ret;
            }, // getter
            [](void *cap, const UnipolarMapping &x) {
                auto pThis = (SystemSettingsApp *)cap;
                if (pThis->mPitchUpMappingIndex < 0)
                {
                    return;
                }
                pThis->mAppSettings->mControlMappings[pThis->mPitchUpMappingIndex].mUnipolarMapping = x;
            },
            this},
        [](void *cap) FLASHMEM {
            auto pThis = (SystemSettingsApp *)cap;
            return pThis->mRawPitchBendGetter(pThis->mpCapture);
        },
        this};

    UnipolarCalibrationSettingItem mPitchDown = {
        "Pitch Down",
        Property<UnipolarMapping>{
            [](void *cap) FLASHMEM {
                auto pThis = (SystemSettingsApp *)cap;
                if (pThis->mPitchDownMappingIndex < 0)
                {
                    return UnipolarMapping{};
                }
                UnipolarMapping ret =
                    pThis->mAppSettings->mControlMappings[pThis->mPitchDownMappingIndex].mUnipolarMapping;
                return ret;
            }, // getter
            [](void *cap, const UnipolarMapping &x) {
                auto pThis = (SystemSettingsApp *)cap;
                if (pThis->mPitchDownMappingIndex < 0)
                {
                    return;
                }
                pThis->mAppSettings->mControlMappings[pThis->mPitchDownMappingIndex].mUnipolarMapping = x;
            },
            this},
        [](void *cap) FLASHMEM {
            auto pThis = (SystemSettingsApp *)cap;
            return pThis->mRawPitchBendGetter(pThis->mpCapture);
        },
        this};

    // IntSettingItem mNoteChangeFrames = {"Note chg frames",
    //                                     NumericEditRangeSpec<int>{0, 25},
    //                                     Property<int>{
    //                                         [](void *cap) FLASHMEM {
    //                                             auto *pThis = (SystemSettingsApp *)cap;
    //                                             return (int)pThis->mAppSettings->mNoteChangeSmoothingFrames;
    //                                         }, // getter
    //                                         [](void *cap, const int &val) {
    //                                             auto *pThis = (SystemSettingsApp *)cap;
    //                                             pThis->mAppSettings->mNoteChangeSmoothingFrames = val;
    //                                         },   // setter
    //                                         this // capture val
    //                                     },
    //                                     AlwaysEnabled};

    // FloatSettingItem mNoteChangeIntFrames = {
    //     "Note delta*",
    //     StandardRangeSpecs::gFloat_0_1,
    //     Property<float>{
    //         [](void *cap) FLASHMEM {
    //             auto *pThis = (SystemSettingsApp *)cap;
    //             return pThis->mAppSettings->mNoteChangeSmoothingIntervalFrameFactor;
    //         }, // getter
    //         [](void *cap, const float &val) {
    //             auto *pThis = (SystemSettingsApp *)cap;
    //             pThis->mAppSettings->mNoteChangeSmoothingIntervalFrameFactor = val;
    //         },   // setter
    //         this // capture val
    //     },
    //     AlwaysEnabled};

    IntSettingItem mSelectedPerfPatch = {"Perf patch",
                                         NumericEditRangeSpec<int>{0, clarinoid::PERFORMANCE_PATCH_COUNT - 1},
                                         Property<int>{
                                             [](void *cap) FLASHMEM {
                                                 auto *pThis = (SystemSettingsApp *)cap;
                                                 return (int)pThis->mAppSettings->mCurrentPerformancePatch.GetValue();
                                             }, // getter
                                             [](void *cap, const int &val) {
                                                 auto *pThis = (SystemSettingsApp *)cap;
                                                 pThis->mAppSettings->mCurrentPerformancePatch.SetValue(val);
                                                 pThis->mpMetronome->OnBPMChanged();
                                             },   // setter
                                             this // capture val
                                         },
                                         [](void *cap, int n) { // formatter
                                             auto *pThis = (SystemSettingsApp *)cap;
                                             return pThis->mAppSettings->GetPerfPatchName(n);
                                         },
                                         AlwaysEnabled,
                                         this};

    ISettingItem *mArray[7] = {
        &mExportToSerial,
        &mSelectedPerfPatch,
        &mPedalPolarity,
        &mDimDisplay,
        &mBreath,
        &mPitchUp,
        &mPitchDown,
        // &mNoteChangeFrames,
        // &mNoteChangeIntFrames,
    };

    SettingsList mRootList = {mArray};

  public:
    virtual SettingsList *GetRootSettingsList()
    {
        return &mRootList;
    }

    virtual void RenderFrontPage()
    {
        mDisplay.ClearState();
        mDisplay.println(String("System >"));
        SettingsMenuApp::RenderFrontPage();
    }
};

} // namespace clarinoid
