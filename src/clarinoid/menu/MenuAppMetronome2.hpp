#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"
#include "NumericSettingItem.hpp"
#include "EnumSettingItem.hpp"
#include "BoolSettingItem.hpp"
#include <clarinoid/application/MetronomeVis2.hpp>
#include <clarinoid/application/Metronome.hpp>

namespace clarinoid
{

struct MetronomeSettings2App : public SettingsMenuApp
{
    virtual const char *DisplayAppGetName() override
    {
        return "MetronomeSettings2App";
    }

    Metronome *mpMetronome;
    AppSettings *mpAppSettings;
    MetronomeVis2 mMetronomeVis;

    MetronomeSettings2App(Metronome *pm, AppSettings *pas, IDisplay &d)
        : SettingsMenuApp(d), mpMetronome(pm), mpAppSettings(pas)
    {
    }

    FloatSettingItem mBPM = {"BPM",
                             StandardRangeSpecs::gBPMRange,
                             Property<float>{[](void *cap) FLASHMEM {
                                                 auto *pThis = (MetronomeSettings2App *)cap;
                                                 return pThis->mpAppSettings->GetCurrentPerformancePatch().mBPM;
                                             },
                                             [](void *cap, const float &v) FLASHMEM {
                                                 auto *pThis = (MetronomeSettings2App *)cap;
                                                 pThis->mpAppSettings->GetCurrentPerformancePatch().mBPM = v;
                                                 pThis->mpMetronome->OnBPMChanged();
                                                 MetronomeVis2::SettingsChangedTrigger = true;
                                             },
                                             this},
                             AlwaysEnabled};

    // beats per bar
    IntSettingItem mBeatsPerBar = {
        "Beats per bar",
        NumericEditRangeSpec<int>{1, kMaxBeatsPerBar},
        Property<int>{[](void *cap) FLASHMEM {
                          auto *pThis = (MetronomeSettings2App *)cap;
                          return (int)pThis->mpAppSettings->GetCurrentPerformancePatch().mBeatsPerBar;
                      },
                      [](void *cap, const int &v) FLASHMEM {
                          auto *pThis = (MetronomeSettings2App *)cap;
                          pThis->mpAppSettings->GetCurrentPerformancePatch().mBeatsPerBar = v;
                          MetronomeVis2::SettingsChangedTrigger = true;
                      },
                      this},
        AlwaysEnabled};

    IntSettingItem mBeatSubdivisions = {
        "Beat subdivs",
        NumericEditRangeSpec<int>{1, kMaxSubdivisionsPerBeat},
        Property<int>{[](void *cap) FLASHMEM {
                          auto *pThis = (MetronomeSettings2App *)cap;
                          return (int)pThis->mpAppSettings->GetCurrentPerformancePatch().mBeatSubdivisions;
                      },
                      [](void *cap, const int &v) FLASHMEM {
                          auto *pThis = (MetronomeSettings2App *)cap;
                          pThis->mpAppSettings->GetCurrentPerformancePatch().mBeatSubdivisions = v;
                          MetronomeVis2::SettingsChangedTrigger = true;
                      },
                      this},
        AlwaysEnabled};

    // MetronomeVis2::kOuterRadius
    FloatSettingItem mOuterRadius = {"OuterRadius",
                                     NumericEditRangeSpec{10.0f, 50.0f},
                                     Property<float>{[](void *cap) FLASHMEM {
                                                         // auto *pThis = (MetronomeSettings2App *)cap;
                                                         return MetronomeVis2::kOuterRadius;
                                                     },
                                                     [](void *cap, const float &v) {
                                                         MetronomeVis2::kOuterRadius = v;
                                                         MetronomeVis2::SettingsChangedTrigger = true;
                                                     },
                                                     this},
                                     AlwaysEnabled};

    // MetronomeVis2::kBigFlashBrightnessQp8
    IntSettingItem mBigFlashBrightnessQp8 = {"BigFlashBright",
                                             NumericEditRangeSpec<int>{0, 255},
                                             Property<int>{[](void *cap) FLASHMEM {
                                                               // auto *pThis = (MetronomeSettings2App *)cap;
                                                               return (int)MetronomeVis2::kBigFlashBrightnessQp8;
                                                           },
                                                           [](void *cap, const int &v) FLASHMEM {
                                                               // auto *pThis = (MetronomeSettings2App *)cap;
                                                               MetronomeVis2::kBigFlashBrightnessQp8 = v;
                                                               MetronomeVis2::SettingsChangedTrigger = true;
                                                           },
                                                           this},
                                             AlwaysEnabled};
    // MetronomeVis2::kBigFlashHoldMs
    IntSettingItem mBigFlashHoldMs = {"BigFlashHold",
                                      NumericEditRangeSpec<int>{1, 1000},
                                      Property<int>{[](void *cap) FLASHMEM {
                                                        // auto *pThis = (MetronomeSettings2App *)cap;
                                                        return (int)MetronomeVis2::kBigFlashHoldMs;
                                                    },
                                                    [](void *cap, const int &v) FLASHMEM {
                                                        // auto *pThis = (MetronomeSettings2App *)cap;
                                                        MetronomeVis2::kBigFlashHoldMs = v;
                                                        MetronomeVis2::SettingsChangedTrigger = true;
                                                    },
                                                    this},
                                      AlwaysEnabled};

    // MetronomeVis2::kBigFlashDecayMs
    IntSettingItem mBigFlashDecayMs = {"BigFlashDecay",
                                       NumericEditRangeSpec<int>{1, 1000},
                                       Property<int>{[](void *cap) FLASHMEM {
                                                         // auto *pThis = (MetronomeSettings2App *)cap;
                                                         return (int)MetronomeVis2::kBigFlashDecayMs;
                                                     },
                                                     [](void *cap, const int &v) FLASHMEM {
                                                         // auto *pThis = (MetronomeSettings2App *)cap;
                                                         MetronomeVis2::kBigFlashDecayMs = v;
                                                         MetronomeVis2::SettingsChangedTrigger = true;
                                                     },
                                                     this},
                                       AlwaysEnabled};
    // MetronomeVis2::kBigFlashCurveN11
    FloatSettingItem mBigFlashCurveN11 = {"BigFlashCurve",
                                          StandardRangeSpecs::gFloat_N1_1,
                                          Property<float>{[](void *cap) FLASHMEM {
                                                              // auto *pThis = (MetronomeSettings2App *)cap;
                                                              return MetronomeVis2::kBigFlashCurveN11;
                                                          },
                                                          [](void *cap, const float &v) {
                                                              MetronomeVis2::kBigFlashCurveN11 = v;
                                                              MetronomeVis2::SettingsChangedTrigger = true;
                                                          },
                                                          this},
                                          AlwaysEnabled};

    // kBigFlashRadius
    FloatSettingItem mBigFlashRadius = {"BigFlashRadius",
                                        NumericEditRangeSpec{1.0f, 50.0f},
                                        Property<float>{[](void *cap) FLASHMEM {
                                                            // auto *pThis = (MetronomeSettings2App *)cap;
                                                            return MetronomeVis2::kBigFlashRadius;
                                                        },
                                                        [](void *cap, const float &v) {
                                                            MetronomeVis2::kBigFlashRadius = v;
                                                            MetronomeVis2::SettingsChangedTrigger = true;
                                                        },
                                                        this},
                                        AlwaysEnabled};

    // MetronomeVis2::kMinorInnerRadius
    FloatSettingItem mMinorInnerRadius = {"MinorInnerRadius",
                                          NumericEditRangeSpec{1.0f, 50.0f},
                                          Property<float>{[](void *cap) FLASHMEM {
                                                              // auto *pThis = (MetronomeSettings2App *)cap;
                                                              return MetronomeVis2::kMinorInnerRadius;
                                                          },
                                                          [](void *cap, const float &v) {
                                                              MetronomeVis2::kMinorInnerRadius = v;
                                                              MetronomeVis2::SettingsChangedTrigger = true;
                                                          },
                                                          this},
                                          AlwaysEnabled};
    // // MetronomeVis2::kMinorBrightnessQp8
    // IntSettingItem mMinorBrightnessQp8 = {"MinorBright",
    //                                       NumericEditRangeSpec<int>{0, 255},
    //                                       Property<int>{[](void *cap) FLASHMEM {
    //                                                         // auto *pThis = (MetronomeSettings2App *)cap;
    //                                                         return (int)MetronomeVis2::kMinorBrightnessQp8;
    //                                                     },
    //                                                     [](void *cap, const int &v) FLASHMEM {
    //                                                         // auto *pThis = (MetronomeSettings2App *)cap;
    //                                                         MetronomeVis2::kMinorBrightnessQp8 = v;
    //                                                         MetronomeVis2::SettingsChangedTrigger = true;
    //                                                     },
    //                                                     this},
    //                                       AlwaysEnabled};
    // // MetronomeVis2::kMinorHoldMs
    // IntSettingItem mMinorHoldMs = {"MinorHold",
    //                                NumericEditRangeSpec<int>{1, 1000},
    //                                Property<int>{[](void *cap) FLASHMEM {
    //                                                  // auto *pThis = (MetronomeSettings2App *)cap;
    //                                                  return (int)MetronomeVis2::kMinorHoldMs;
    //                                              },
    //                                              [](void *cap, const int &v) FLASHMEM {
    //                                                  // auto *pThis = (MetronomeSettings2App *)cap;
    //                                                  MetronomeVis2::kMinorHoldMs = v;
    //                                                  MetronomeVis2::SettingsChangedTrigger = true;
    //                                              },
    //                                              this},
    //                                AlwaysEnabled};
    // // MetronomeVis2::kMinorDecayMs
    // IntSettingItem mMinorDecayMs = {"MinorDecay",
    //                                 NumericEditRangeSpec<int>{1, 1000},
    //                                 Property<int>{[](void *cap) FLASHMEM {
    //                                                   // auto *pThis = (MetronomeSettings2App *)cap;
    //                                                   return (int)MetronomeVis2::kMinorDecayMs;
    //                                               },
    //                                               [](void *cap, const int &v) FLASHMEM {
    //                                                   // auto *pThis = (MetronomeSettings2App *)cap;
    //                                                   MetronomeVis2::kMinorDecayMs = v;
    //                                                   MetronomeVis2::SettingsChangedTrigger = true;
    //                                               },
    //                                               this},
    //                                 AlwaysEnabled};
    // // MetronomeVis2::kMinorCurveN11
    // FloatSettingItem mMinorCurveN11 = {"MinorCurve",
    //                                    StandardRangeSpecs::gFloat_N1_1,
    //                                    Property<float>{[](void *cap) FLASHMEM {
    //                                                        // auto *pThis = (MetronomeSettings2App *)cap;
    //                                                        return MetronomeVis2::kMinorCurveN11;
    //                                                    },
    //                                                    [](void *cap, const float &v) {
    //                                                        MetronomeVis2::kMinorCurveN11 = v;
    //                                                        MetronomeVis2::SettingsChangedTrigger = true;
    //                                                    },
    //                                                    this},
    //                                    AlwaysEnabled};

    // MetronomeVis2::kMajorInnerRadius
    FloatSettingItem mMajorInnerRadius = {"MajorInnerRadius",
                                          NumericEditRangeSpec{1.0f, 50.0f},
                                          Property<float>{[](void *cap) FLASHMEM {
                                                              // auto *pThis = (MetronomeSettings2App *)cap;
                                                              return MetronomeVis2::kMajorInnerRadius;
                                                          },
                                                          [](void *cap, const float &v) {
                                                              MetronomeVis2::kMajorInnerRadius = v;
                                                              MetronomeVis2::SettingsChangedTrigger = true;
                                                          },
                                                          this},
                                          AlwaysEnabled};
    // // MetronomeVis2::kMajorBrightnessQp8
    // IntSettingItem mMajorBrightnessQp8 = {"MajorBright",
    //                                       NumericEditRangeSpec<int>{0, 255},
    //                                       Property<int>{[](void *cap) FLASHMEM {
    //                                                         // auto *pThis = (MetronomeSettings2App *)cap;
    //                                                         return (int)MetronomeVis2::kMajorBrightnessQp8;
    //                                                     },
    //                                                     [](void *cap, const int &v) FLASHMEM {
    //                                                         // auto *pThis = (MetronomeSettings2App *)cap;
    //                                                         MetronomeVis2::kMajorBrightnessQp8 = v;
    //                                                         MetronomeVis2::SettingsChangedTrigger = true;
    //                                                     },
    //                                                     this},
    //                                       AlwaysEnabled};
    // // MetronomeVis2::kMajorHoldMs
    // IntSettingItem mMajorHoldMs = {"MajorHold",
    //                                NumericEditRangeSpec<int>{1, 1000},
    //                                Property<int>{[](void *cap) FLASHMEM {
    //                                                  // auto *pThis = (MetronomeSettings2App *)cap;
    //                                                  return (int)MetronomeVis2::kMajorHoldMs;
    //                                              },
    //                                              [](void *cap, const int &v) FLASHMEM {
    //                                                  // auto *pThis = (MetronomeSettings2App *)cap;
    //                                                  MetronomeVis2::kMajorHoldMs = v;
    //                                                  MetronomeVis2::SettingsChangedTrigger = true;
    //                                              },
    //                                              this},
    //                                AlwaysEnabled};
    // // MetronomeVis2::kMajorDecayMs
    // IntSettingItem mMajorDecayMs = {"MajorDecay",
    //                                 NumericEditRangeSpec<int>{1, 1000},
    //                                 Property<int>{[](void *cap) FLASHMEM {
    //                                                   // auto *pThis = (MetronomeSettings2App *)cap;
    //                                                   return (int)MetronomeVis2::kMajorDecayMs;
    //                                               },
    //                                               [](void *cap, const int &v) FLASHMEM {
    //                                                   // auto *pThis = (MetronomeSettings2App *)cap;
    //                                                   MetronomeVis2::kMajorDecayMs = v;
    //                                                   MetronomeVis2::SettingsChangedTrigger = true;
    //                                               },
    //                                               this},
    //                                 AlwaysEnabled};
    // // MetronomeVis2::kMajorCurveN11
    // FloatSettingItem mMajorCurveN11 = {"MajorCurve",
    //                                    StandardRangeSpecs::gFloat_N1_1,
    //                                    Property<float>{[](void *cap) FLASHMEM {
    //                                                        // auto *pThis = (MetronomeSettings2App *)cap;
    //                                                        return MetronomeVis2::kMajorCurveN11;
    //                                                    },
    //                                                    [](void *cap, const float &v) {
    //                                                        MetronomeVis2::kMajorCurveN11 = v;
    //                                                        MetronomeVis2::SettingsChangedTrigger = true;
    //                                                    },
    //                                                    this},
    //                                    AlwaysEnabled};

    // MetronomeVis2::kSweepSizeRadians
    FloatSettingItem mSweepSizeRadians = {"SweepSize",
                                          NumericEditRangeSpec{0.0f, kTwoPI_f},
                                          Property<float>{[](void *cap) FLASHMEM {
                                                              // auto *pThis = (MetronomeSettings2App *)cap;
                                                              return MetronomeVis2::kSweepSizeRadians;
                                                          },
                                                          [](void *cap, const float &v) {
                                                              MetronomeVis2::kSweepSizeRadians = v;
                                                              MetronomeVis2::SettingsChangedTrigger = true;
                                                          },
                                                          this},
                                          AlwaysEnabled};
    // MetronomeVis2::kSweepBrightnessQp8
    IntSettingItem mSweepBrightnessQp8 = {"SweepBright",
                                          NumericEditRangeSpec<int>{0, 255},
                                          Property<int>{[](void *cap) FLASHMEM {
                                                            // auto *pThis = (MetronomeSettings2App *)cap;
                                                            return (int)MetronomeVis2::kSweepBrightnessQp8;
                                                        },
                                                        [](void *cap, const int &v) FLASHMEM {
                                                            // auto *pThis = (MetronomeSettings2App *)cap;
                                                            MetronomeVis2::kSweepBrightnessQp8 = v;
                                                            MetronomeVis2::SettingsChangedTrigger = true;
                                                        },
                                                        this},
                                          AlwaysEnabled};
    // MetronomeVis2::kSweepCurveN11
    // FloatSettingItem mSweepCurveN11 = {"SweepCurve",
    //                                    StandardRangeSpecs::gFloat_N1_1,
    //                                    Property<float>{[](void *cap) FLASHMEM {
    //                                                        // auto *pThis = (MetronomeSettings2App *)cap;
    //                                                        return MetronomeVis2::kSweepCurveN11;
    //                                                    },
    //                                                    [](void *cap, const float &v) {
    //                                                        MetronomeVis2::kSweepCurveN11 = v;
    //                                                        MetronomeVis2::SettingsChangedTrigger = true;
    //                                                    },
    //                                                    this},
    //                                    AlwaysEnabled};

    ISettingItem *mArray[13] = {
        &mBPM,         //
        &mBeatsPerBar, //
        &mBeatSubdivisions,

        &mOuterRadius,           //
        &mBigFlashBrightnessQp8, //
        &mBigFlashHoldMs,        //
        &mBigFlashDecayMs,       //
        &mBigFlashCurveN11,      //
        &mBigFlashRadius,

        &mMinorInnerRadius, //
        // &mMinorBrightnessQp8, //
        // &mMinorHoldMs,        //
        // &mMinorDecayMs,       //
        // &mMinorCurveN11,      //

        &mMajorInnerRadius, //
        // &mMajorBrightnessQp8, //
        // &mMajorHoldMs,        //
        // &mMajorDecayMs,       //
        // &mMajorCurveN11,      //

        &mSweepSizeRadians,   //
        &mSweepBrightnessQp8, //
        //&mSweepCurveN11,      //

        // &mRadius,                 //
        // &mSubdivisionRadius,      //
        // &mFlashHoldMs,            //
        // &mFlashDecayMs,           //
        // &mSubdivisionFlashHoldMs, //
        // &mSubdivisionFlashDecayMs,
        // &mSweepWidthRadians, //
        // &mSweepMaxBrightnessQp8,
        // &mSubdivFlashMaxBrightnessQp8,
        // &mBeatFlashMaxBrightnessQp8,
        // &mBigFlashMaxBrightnessQp8,

    };
    SettingsList mRootList = {mArray};

    virtual SettingsList *GetRootSettingsList()
    {
        return &mRootList;
    }

    virtual void RenderFrontPage()
    {
        mMetronomeVis.Render(*mpMetronome, mDisplay, *mpAppSettings, {100, 27}, RectI::Construct(0, 0, 50, 50));

        mDisplay.println(String("METRONOM2"));

        mDisplay.println(String("bpm ") + (int)std::round(mpAppSettings->GetCurrentPerformancePatch().mBPM));
        mDisplay.println(String("n   ") + mpAppSettings->GetCurrentPerformancePatch().mBeatsPerBar);
        mDisplay.println(String("sub ") + mpAppSettings->GetCurrentPerformancePatch().mBeatSubdivisions);

        SettingsMenuApp::RenderFrontPage();
    }
};

static constexpr size_t MetronomeSettings2AppSize = sizeof(MetronomeSettings2App);
static constexpr size_t MetronomeVis2AppSize = sizeof(MetronomeVis2);

} // namespace clarinoid
