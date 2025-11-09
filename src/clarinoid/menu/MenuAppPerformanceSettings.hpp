#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"
#include "NumericSettingItem.hpp"
#include "EnumSettingItem.hpp"
#include "FunctionListSettingItem.hpp"
#include "GainSettingItem.hpp"

namespace clarinoid
{

struct PerformancePatchSettingsApp : public SettingsMenuApp
{
    virtual const char *DisplayAppGetName() override
    {
        return "PerformancePatchSettingsApp";
    }

    PerformancePatch &GetBinding()
    {
        return mAppSettings->GetCurrentPerformancePatch();
    }

    PerformancePatchSettingsApp(IDisplay &d) : SettingsMenuApp(d)
    {
    }

    GainSettingItem mMasterGain = {"Master gain",
                                   StandardRangeSpecs::gMasterGainDb,
                                   Property<float>{[](void *cap) FLASHMEM {
                                                       auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                       return pThis->GetBinding().mMasterGain;
                                                   },
                                                   [](void *cap, const float &v) {
                                                       auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                       pThis->GetBinding().mMasterGain = v;
                                                   },
                                                   this},
                                   AlwaysEnabled};

    IntSettingItem mTranspose = {"Transpose",
                                 StandardRangeSpecs::gTransposeRange,
                                 Property<int>{[](void *cap) FLASHMEM {
                                                   auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                   return (int)pThis->GetBinding().mTranspose;
                                               },
                                               [](void *cap, const int &v) {
                                                   auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                   pThis->GetBinding().mTranspose = v;
                                               },
                                               this},
                                 AlwaysEnabled};

    IntSettingItem mSynthPatchATranspose = {"TransposeA",
                                            StandardRangeSpecs::gTransposeRange,
                                            Property<int>{[](void *cap) FLASHMEM {
                                                              auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                              return (int)pThis->GetBinding().mSynthATranspose;
                                                          },
                                                          [](void *cap, const int &v) {
                                                              auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                              pThis->GetBinding().mSynthATranspose = v;
                                                          },
                                                          this},
                                            AlwaysEnabled};

    IntSettingItem mSynthPatchBTranspose = {"TransposeB",
                                            StandardRangeSpecs::gTransposeRange,
                                            Property<int>{[](void *cap) FLASHMEM {
                                                              auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                              return (int)pThis->GetBinding().mSynthBTranspose;
                                                          },
                                                          [](void *cap, const int &v) {
                                                              auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                              pThis->GetBinding().mSynthBTranspose = v;
                                                          },
                                                          this},
                                            AlwaysEnabled};

    EnumSettingItem<PerfDisplayStyle> mPerfDisplayStyle = {
        "Display style",
        gPerfDisplayStyleInfo,
        Property<PerfDisplayStyle>{[](void *cap) FLASHMEM {
                                       auto *pThis = (PerformancePatchSettingsApp *)cap;
                                       return pThis->GetBinding().mPerfDisplayStyle;
                                   },
                                   [](void *cap, const PerfDisplayStyle &v) {
                                       auto *pThis = (PerformancePatchSettingsApp *)cap;
                                       pThis->GetBinding().mPerfDisplayStyle = v;
                                   },
                                   this},
        AlwaysEnabled};

    FloatSettingItem mDetune = {"Detune",
                                StandardRangeSpecs::gFloat_0_1_Fine,
                                Property<float>{[](void *cap) FLASHMEM {
                                                    auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                    return pThis->GetBinding().mDetuneSemis;
                                                },
                                                [](void *cap, const float &v) FLASHMEM {
                                                    auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                    pThis->GetBinding().mDetuneSemis = v;
                                                },
                                                this},
                                AlwaysEnabled};

    EnumSettingItem<GlobalScaleRefType> mGlobalScaleRef = {
        "Scale ref",
        gGlobalScaleRefTypeInfo,
        Property<GlobalScaleRefType>{[](void *cap) FLASHMEM {
                                         auto *pThis = (PerformancePatchSettingsApp *)cap;
                                         return pThis->GetBinding().mGlobalScaleRef;
                                     },
                                     [](void *cap, const GlobalScaleRefType &v) {
                                         auto *pThis = (PerformancePatchSettingsApp *)cap;
                                         pThis->GetBinding().mGlobalScaleRef = v;
                                     },
                                     this},
        AlwaysEnabled};

    LabelSettingItem mDeducedScale = {Property<String>{[](void *cap) FLASHMEM {
                                                           auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                           return pThis->GetBinding().mDeducedScale.ToString();
                                                       },
                                                       this},
                                      Property<bool>{[](void *cap) FLASHMEM { return true; }, this}};

    EnumSettingItem<Note> mChosenScaleNote = {"Scale note",
                                              gNoteInfo,
                                              Property<Note>{[](void *cap) FLASHMEM {
                                                                 auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                                 return pThis->GetBinding().mGlobalScale.mRootNoteIndex;
                                                             },
                                                             [](void *cap, const Note &v) {
                                                                 auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                                 pThis->GetBinding().mGlobalScale.mRootNoteIndex = v;
                                                             },
                                                             this},
                                              Property<bool>{[](void *cap) FLASHMEM {
                                                                 auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                                 return pThis->GetBinding().mGlobalScaleRef ==
                                                                        GlobalScaleRefType::Chosen;
                                                             },
                                                             this}};

    EnumSettingItem<ScaleFlavorIndex> mChosenScaleFlavor = {
        "Scale flavor",
        gScaleFlavorIndexInfo,
        Property<ScaleFlavorIndex>{[](void *cap) FLASHMEM {
                                       auto *pThis = (PerformancePatchSettingsApp *)cap;
                                       return pThis->GetBinding().mGlobalScale.mFlavorIndex;
                                   },
                                   [](void *cap, const ScaleFlavorIndex &v) {
                                       auto *pThis = (PerformancePatchSettingsApp *)cap;
                                       pThis->GetBinding().mGlobalScale.mFlavorIndex = v;
                                   },
                                   this},
        Property<bool>{[](void *cap) FLASHMEM {
                           auto *pThis = (PerformancePatchSettingsApp *)cap;
                           return pThis->GetBinding().mGlobalScaleRef == GlobalScaleRefType::Chosen;
                       },
                       this}};

    FloatSettingItem mStereoSpread = {"Width",
                                      StandardRangeSpecs::gFloat_0_1,
                                      Property<float>{[](void *cap) FLASHMEM {
                                                          auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                          return pThis->GetBinding().mSynthStereoSpread;
                                                      },
                                                      [](void *cap, const float &v) {
                                                          auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                          pThis->GetBinding().mSynthStereoSpread = v;
                                                      },
                                                      this},
                                      AlwaysEnabled};

    GainSettingItem mReverbGain = {"Reverb gain",
                                   StandardRangeSpecs::gGeneralGain,
                                   Property<float>{[](void *cap) FLASHMEM {
                                                       auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                       return pThis->GetBinding().mReverbGain;
                                                   },
                                                   [](void *cap, const float &v) {
                                                       auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                       pThis->GetBinding().mReverbGain = v;
                                                   },
                                                   this},
                                   AlwaysEnabled};

    GainSettingItem mDelayGain = {"Delay gain",
                                  StandardRangeSpecs::gGeneralGain,
                                  Property<float>{[](void *cap) FLASHMEM {
                                                      auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                      return pThis->GetBinding().mDelayGain;
                                                  },
                                                  [](void *cap, const float &v) {
                                                      auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                      pThis->GetBinding().mDelayGain = v;
                                                  },
                                                  this},
                                  AlwaysEnabled};

    BoolSettingItem mMasterFXEnable = {"MasterFX Enable",
                                       "Yes",
                                       "No",
                                       Property<bool>{[](void *cap) FLASHMEM {
                                                          auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                          return pThis->GetBinding().mMasterFXEnable;
                                                      },
                                                      [](void *cap, const bool &v) {
                                                          auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                          pThis->GetBinding().mMasterFXEnable = v;
                                                      },
                                                      this},
                                       AlwaysEnabled};

    IntSettingItem mSelectedSynthPatchA = {"Synth patch A",
                                           NumericEditRangeSpec<int>{0, clarinoid::SYNTH_PRESET_COUNT - 1},
                                           Property<int>{
                                               [](void *cap) FLASHMEM {
                                                   auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                   return (int)pThis->GetBinding().mSynthPresetA;
                                               }, // getter
                                               [](void *cap, const int &val) {
                                                   auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                   pThis->GetBinding().mSynthPresetA = val;
                                               },   // setter
                                               this // capture val
                                           },
                                           [](void *cap, int n) { // formatter
                                               auto *pThis = (PerformancePatchSettingsApp *)cap;
                                               return pThis->mAppSettings->GetSynthPatchName(n);
                                           },
                                           AlwaysEnabled,
                                           this};

    IntSettingItem mSelectedSynthPatchB = {"Synth patch B",
                                           NumericEditRangeSpec<int>{0, clarinoid::SYNTH_PRESET_COUNT - 1},
                                           Property<int>{
                                               [](void *cap) FLASHMEM {
                                                   auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                   return (int)pThis->GetBinding().mSynthPresetB;
                                               }, // getter
                                               [](void *cap, const int &val) {
                                                   auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                   pThis->GetBinding().mSynthPresetB = val;
                                               },   // setter
                                               this // capture val
                                           },
                                           [](void *cap, int n) { // formatter
                                               auto *pThis = (PerformancePatchSettingsApp *)cap;
                                               return pThis->mAppSettings->GetSynthPatchName(n);
                                           },
                                           AlwaysEnabled,
                                           this};

    IntSettingItem mSelectedHarmPatch = {"Harm patch",
                                         NumericEditRangeSpec<int>{0, clarinoid::HARM_PRESET_COUNT - 1},
                                         Property<int>{
                                             [](void *cap) FLASHMEM {
                                                 auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                 return (int)pThis->GetBinding().mHarmPreset;
                                             }, // getter
                                             [](void *cap, const int &val) {
                                                 auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                 pThis->GetBinding().mHarmPreset = val;
                                             },   // setter
                                             this // capture val
                                         },
                                         [](void *cap, int n) { // formatter
                                             auto *pThis = (PerformancePatchSettingsApp *)cap;
                                             return pThis->mAppSettings->GetHarmPatchName(n);
                                         },
                                         AlwaysEnabled,
                                         this};

    FloatSettingItem mReverbDamping = {"Reverb damp",
                                       StandardRangeSpecs::gFloat_0_1,
                                       Property<float>{[](void *cap) FLASHMEM {
                                                           auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                           return pThis->GetBinding().mReverbDamping;
                                                       },
                                                       [](void *cap, const float &v) {
                                                           auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                           pThis->GetBinding().mReverbDamping = v;
                                                       },
                                                       this},
                                       AlwaysEnabled};

    FloatSettingItem mReverbSize = {"Reverb size",
                                    StandardRangeSpecs::gFloat_0_1,
                                    Property<float>{[](void *cap) FLASHMEM {
                                                        auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                        return pThis->GetBinding().mReverbSize;
                                                    },
                                                    [](void *cap, const float &v) {
                                                        auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                        pThis->GetBinding().mReverbSize = v;
                                                    },
                                                    this},
                                    AlwaysEnabled};

    FloatSettingItem mDelayTimeMS = {"Delay Time",
                                     NumericEditRangeSpec<float>(1, MAX_DELAY_MS),
                                     Property<float>{[](void *cap) FLASHMEM {
                                                         auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                         return pThis->GetBinding().mDelayTime.mTimeMS;
                                                     },
                                                     [](void *cap, const float &v) {
                                                         auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                         pThis->GetBinding().mDelayTime.mTimeMS = v;
                                                     },
                                                     this},
                                     AlwaysEnabled};

    FloatSettingItem mDelayStereoSep = {" >Width",
                                        NumericEditRangeSpec<float>(1, 100),
                                        Property<float>{[](void *cap) FLASHMEM {
                                                            auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                            return pThis->GetBinding().mDelayStereoSep;
                                                        },
                                                        [](void *cap, const float &v) {
                                                            auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                            pThis->GetBinding().mDelayStereoSep = v;
                                                        },
                                                        this},
                                        AlwaysEnabled};

    FloatSettingItem mDelayFeedbackLevel = {" >FB",
                                            StandardRangeSpecs::gFloat_0_1,
                                            Property<float>{[](void *cap) FLASHMEM {
                                                                auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                                return pThis->GetBinding().mDelayFeedbackLevel;
                                                            },
                                                            [](void *cap, const float &v) {
                                                                auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                                pThis->GetBinding().mDelayFeedbackLevel = v;
                                                            },
                                                            this},
                                            AlwaysEnabled};

    EnumSettingItem<ClarinoidFilterType> mDelayFilterType = {
        " >Filter",
        gClarinoidFilterTypeInfo,
        Property<ClarinoidFilterType>{[](void *cap) FLASHMEM {
                                          auto *pThis = (PerformancePatchSettingsApp *)cap;
                                          return pThis->GetBinding().mDelayFilterType;
                                      },
                                      [](void *cap, const ClarinoidFilterType &v) {
                                          auto *pThis = (PerformancePatchSettingsApp *)cap;
                                          pThis->GetBinding().mDelayFilterType = v;
                                      },
                                      this},
        AlwaysEnabled};

    FloatSettingItem mDelayCutoffFrequency = {" > >Freq",
                                              NumericEditRangeSpec<float>(0, 22050),
                                              Property<float>{[](void *cap) FLASHMEM {
                                                                  auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                                  return pThis->GetBinding().mDelayCutoffFrequency;
                                                              },
                                                              [](void *cap, const float &v) {
                                                                  auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                                  pThis->GetBinding().mDelayCutoffFrequency = v;
                                                              },
                                                              this},
                                              AlwaysEnabled};

    FloatSettingItem mDelaySaturation = {" > >Sat",
                                         StandardRangeSpecs::gFloat_0_1,
                                         Property<float>{[](void *cap) FLASHMEM {
                                                             auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                             return pThis->GetBinding().mDelaySaturation;
                                                         },
                                                         [](void *cap, const float &v) {
                                                             auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                             pThis->GetBinding().mDelaySaturation = v;
                                                         },
                                                         this},
                                         AlwaysEnabled};

    FloatSettingItem mDelayQ = {" > >Q",
                                StandardRangeSpecs::gFloat_0_1,
                                Property<float>{[](void *cap) FLASHMEM {
                                                    auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                    return pThis->GetBinding().mDelayQ;
                                                },
                                                [](void *cap, const float &v) {
                                                    auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                    pThis->GetBinding().mDelayQ = v;
                                                },
                                                this},
                                AlwaysEnabled};

    ISettingItem *mMasterFXSubmenuItems[9] = {
        &mReverbDamping,
        &mReverbSize,
        &mDelayTimeMS,
        &mDelayStereoSep,
        &mDelayFeedbackLevel,
        &mDelayFilterType,
        &mDelayCutoffFrequency,
        &mDelaySaturation,
        &mDelayQ,
    };
    SettingsList mMasterFXList = {mMasterFXSubmenuItems};

    SubmenuSettingItem mMasterFX = {"Master FX", &mMasterFXList, AlwaysEnabled};

    ISettingItem *mScaleSubmenuItems[4] = {
        &mGlobalScaleRef,
        &mDeducedScale,
        &mChosenScaleNote,
        &mChosenScaleFlavor,
    };
    SettingsList mScaleList = {mScaleSubmenuItems};

    SubmenuSettingItem mScaleSubmenu = {"Scale", &mScaleList, AlwaysEnabled};

    ISettingItem *mArray[15] = {
        &mMasterGain,
        &mTranspose,
        &mDetune,
        &mPerfDisplayStyle,
        &mSynthPatchATranspose,
        &mSynthPatchBTranspose,
        &mSelectedSynthPatchA,
        &mSelectedSynthPatchB,
        &mSelectedHarmPatch,

        &mScaleSubmenu,

        &mStereoSpread,
        &mMasterFXEnable,
        &mReverbGain,
        &mDelayGain,

        &mMasterFX,
    };
    SettingsList mRootList = {mArray};

    //   public:
    virtual SettingsList *GetRootSettingsList()
    {
        return &mRootList;
    }

    virtual void RenderFrontPage()
    {
        mDisplay.ClearState();
        mDisplay.println(String("Performance > "));
        mDisplay.println(GetAppSettings()->GetPerfPatchName(GetAppSettings()->mCurrentPerformancePatch));

        SettingsMenuApp::RenderFrontPage();
    }
};

static constexpr size_t aoseunth = sizeof(PerformancePatchSettingsApp);
// 3660 = pass
// 3832 = fail

} // namespace clarinoid
