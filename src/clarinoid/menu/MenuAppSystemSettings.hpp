// 0               0x7FFFF - RAM1: ITCM (512KB) (FASTRUN), copied from flash
// 0x20000000 - 0x2007FFFF - RAM1: DTCM (512KB) (variables, initialized
// copied from flash) 0x20200000 - 0x2027FFFF - RAM2: OCRMA2 (512KB)
// (malloc,new) 0x60000000 - 0x6FFFFFFF - FLEXSPI ... (PROGMEM, modcurve data
// progmem)

// usb
// usb midi
// usb storage
// ssd1306

// int m = millis();

// Serial.println();
// // Serial.println(String("lambda                       : ") +
// // PointerToString(cc::function<void()>::ptr_t([](){})));
// Serial.println(String("fn : ") +
// // PointerToString(&PointerToString));
// Serial.println(String("global var                   : ") +
// PointerToString(&gOscWaveformShapeInfo)); Serial.println(String("FLASH
// static str             : ") + PointerToString("tes9t"));
// Serial.println(String("FLASH F      str             : ") +
// PointerToString("ttc#$t"));

// Serial.println(String("DMAMEM gJSONBuffer           : ") +
// PointerToString(gClarinoidDmaMem.gJSONBuffer));
// Serial.println(String("DMAMEM gAudioMemory          : ") +
// PointerToString(gClarinoidDmaMem.gAudioMemory));
// Serial.println(String("DMAMEM gLoopStationBuffer    : ") +
//                PointerToString(gClarinoidDmaMem.gLoopStationBuffer));
// Serial.println(String("stack var                    : ") +
// PointerToString(&m)); Serial.println(String("gModCurveLUTData_PROGMEM : ")
// + PointerToString(gModCurveLUTData_PROGMEM)); Serial.println(String(" : ")
// + PointerToString(gModCurveLUTData_PROGMEM));

// pThis->mDisplay.ShowToast(String("Exported ") + size + " bytes.");

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
    // int mBreathMappingIndex;
    // int mPitchUpMappingIndex;
    // int mPitchDownMappingIndex;
    // cc::function<float(void *)>::ptr_t mRawBreathGetter;
    // cc::function<float(void *)>::ptr_t mRawPitchBendGetter;
    void *mpCapture;
    Metronome *mpMetronome;
    IStorage *mpStorage;

    SystemSettingsApp(IDisplay &d,
                      //   int breathMappingIndex,
                      //   int pitchUpMappingIndex,
                      //   int pitchDownMappingIndex,
                      //   cc::function<float(void *)>::ptr_t rawBreathGetter,
                      //   cc::function<float(void *)>::ptr_t rawPitchBendGetter,
                      void *capture,
                      Metronome *pm,
                      AppSettings &appSettings,
                      InputDelegator &input,
                      IStorage *pStorage)
        : SettingsMenuApp(d, appSettings, input), //
                                                  //   mBreathMappingIndex(breathMappingIndex),       //
                                                  //   mPitchUpMappingIndex(pitchUpMappingIndex),     //
                                                  //   mPitchDownMappingIndex(pitchDownMappingIndex), //
                                                  //   mRawBreathGetter(rawBreathGetter),             //
                                                  //   mRawPitchBendGetter(rawPitchBendGetter),       //
          mpCapture(capture),                     //
          mpMetronome(pm),                        //
          mpStorage(pStorage)
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

    TriggerSettingItem mDirToSerial{String("dir > Serial"),
                                    [](void *cap) {
                                        auto pThis = (SystemSettingsApp *)cap;
                                        pThis->mpStorage->Dir(StorageChannel::SerialStream);
                                    },
                                    this,
                                    AlwaysEnabled};

    MultiTriggerSettingItem mExport{[](void *) { return gStorageChannelInfo.count(); },
                                    [](void *, size_t i) {
                                        return (String)(String("Save >") +
                                                        gStorageChannelInfo.GetValueDisplayName((StorageChannel)i));
                                    },
                                    [](void *cap, size_t i) {
                                        auto pThis = (SystemSettingsApp *)cap;

                                        //Serial.println("exporting ...");

                                        ClarinoidJsonDocument doc;
                                        if (!pThis->mAppSettings->SerializableObject_ToJSON(doc))
                                        {
                                            Serial.println("mAppSettings->SerializableObject_ToJSON failed");
                                            return;
                                        }
                                        //Serial.println("document has been generated. now saving...");
                                        auto ret = pThis->mpStorage->SaveDocument((StorageChannel)i, doc);
                                        if (ret.IsFailure()) {
                                            pThis->mDisplay.ShowToast(String("Fail: ") + ret.mMessage, 4000);
                                        }
                                        else {
                                            pThis->mDisplay.ShowToast(String("Success: ") + ret.mMessage, 4000);
                                        }
                                    },
                                    AlwaysEnabledMulti, this};

    MultiTriggerSettingItem mImport{[](void *) { return gStorageChannelInfo.count(); },
                                    [](void *, size_t i) {
                                        return (String)(String("Load <") +
                                                        gStorageChannelInfo.GetValueDisplayName((StorageChannel)i));
                                    },
                                    [](void *cap, size_t i) {
                                        auto pThis = (SystemSettingsApp *)cap;

                                        ClarinoidJsonDocument doc;
                                        auto err = pThis->mpStorage->LoadDocument(StorageChannel(i), doc);
                                        if (err.code() != DeserializationError::Ok) {
                                            pThis->mDisplay.ShowToast(err.c_str(), 5000);
                                            return;
                                        }

                                        auto err2 = pThis->mAppSettings->SerializableObject_Deserialize(doc);
                                        if (err2.IsSuccess()) {
                                            pThis->mDisplay.ShowToast(String("Success; ") + err2.mMessage, 4000);
                                            return;
                                        }

                                        pThis->mDisplay.ShowToast(String("FAIL: ") + err2.mMessage, 4000);
                                    },
                                    AlwaysEnabledMulti, this};

    // UnipolarCalibrationSettingItem mBreath = {
    //     "Breath",
    //     Property<UnipolarMapping>{
    //         [](void *cap) FLASHMEM {
    //             auto pThis = (SystemSettingsApp *)cap;
    //             if (pThis->mBreathMappingIndex < 0)
    //             {
    //                 return UnipolarMapping{};
    //             }
    //             UnipolarMapping ret =
    //                 pThis->mAppSettings->mControlMappings[pThis->mBreathMappingIndex].mUnipolarMapping;
    //             return ret;
    //         }, // getter
    //         [](void *cap, const UnipolarMapping &x) {
    //             auto pThis = (SystemSettingsApp *)cap;
    //             if (pThis->mBreathMappingIndex < 0)
    //             {
    //                 return;
    //             }
    //             pThis->mAppSettings->mControlMappings[pThis->mBreathMappingIndex].mUnipolarMapping = x;
    //         },
    //         this},
    //     [](void *cap) FLASHMEM {
    //         auto pThis = (SystemSettingsApp *)cap;
    //         return pThis->mRawBreathGetter(pThis->mpCapture);
    //     },
    //     this};

    // UnipolarCalibrationSettingItem mPitchUp = {
    //     "Pitch Up",
    //     Property<UnipolarMapping>{
    //         [](void *cap) FLASHMEM {
    //             auto pThis = (SystemSettingsApp *)cap;
    //             if (pThis->mPitchUpMappingIndex < 0)
    //             {
    //                 return UnipolarMapping{};
    //             }
    //             UnipolarMapping ret =
    //                 pThis->mAppSettings->mControlMappings[pThis->mPitchUpMappingIndex].mUnipolarMapping;
    //             return ret;
    //         }, // getter
    //         [](void *cap, const UnipolarMapping &x) {
    //             auto pThis = (SystemSettingsApp *)cap;
    //             if (pThis->mPitchUpMappingIndex < 0)
    //             {
    //                 return;
    //             }
    //             pThis->mAppSettings->mControlMappings[pThis->mPitchUpMappingIndex].mUnipolarMapping = x;
    //         },
    //         this},
    //     [](void *cap) FLASHMEM {
    //         auto pThis = (SystemSettingsApp *)cap;
    //         return pThis->mRawPitchBendGetter(pThis->mpCapture);
    //     },
    //     this};

    // UnipolarCalibrationSettingItem mPitchDown = {
    //     "Pitch Down",
    //     Property<UnipolarMapping>{
    //         [](void *cap) FLASHMEM {
    //             auto pThis = (SystemSettingsApp *)cap;
    //             if (pThis->mPitchDownMappingIndex < 0)
    //             {
    //                 return UnipolarMapping{};
    //             }
    //             UnipolarMapping ret =
    //                 pThis->mAppSettings->mControlMappings[pThis->mPitchDownMappingIndex].mUnipolarMapping;
    //             return ret;
    //         }, // getter
    //         [](void *cap, const UnipolarMapping &x) {
    //             auto pThis = (SystemSettingsApp *)cap;
    //             if (pThis->mPitchDownMappingIndex < 0)
    //             {
    //                 return;
    //             }
    //             pThis->mAppSettings->mControlMappings[pThis->mPitchDownMappingIndex].mUnipolarMapping = x;
    //         },
    //         this},
    //     [](void *cap) FLASHMEM {
    //         auto pThis = (SystemSettingsApp *)cap;
    //         return pThis->mRawPitchBendGetter(pThis->mpCapture);
    //     },
    //     this};

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

    ISettingItem *mArray[6] = {
        &mDirToSerial,
        &mExport,
        &mImport,
        &mSelectedPerfPatch,
        &mPedalPolarity,
        &mDimDisplay,
        // &mBreath,
        // &mPitchUp,
        // &mPitchDown,
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

static constexpr auto aoesntuh = sizeof(SystemSettingsApp);

} // namespace clarinoid
