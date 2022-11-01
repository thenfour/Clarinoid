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

    PerformancePatchSettingsApp(IDisplay &d, AppSettings& appSettings, InputDelegator& input) : SettingsMenuApp(d, appSettings, input)
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

    FloatSettingItem mStereoSpread = {"B Spread",
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

    SubmenuSettingItem mMasterFX = {String("Master FX"), &mMasterFXList, AlwaysEnabled};

    ISettingItem *mArray[10] = {
        &mMasterGain,
        &mTranspose,
        &mSelectedSynthPatchA,
        &mSelectedSynthPatchB,
        &mStereoSpread,
        &mSelectedHarmPatch,
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

} // namespace clarinoid
