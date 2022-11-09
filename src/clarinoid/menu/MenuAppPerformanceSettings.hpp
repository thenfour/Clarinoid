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

    PerformancePatchSettingsApp(IDisplay &d, AppSettings &appSettings, InputDelegator &input)
        : SettingsMenuApp(d, appSettings, input)
    {
    }

    GainSettingItem mMasterGain = {"Master gain",
                                   StandardRangeSpecs::gMasterGainDb,
                                   Property<float>{[](void *cap) FLASHMEM {
                                                       auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                       return pThis->GetBinding().mMasterGain.GetValue();
                                                   },
                                                   [](void *cap, const float &v) {
                                                       auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                       pThis->GetBinding().mMasterGain.SetValue(v);
                                                   },
                                                   this},
                                   AlwaysEnabled};

    IntSettingItem mTranspose = {"Transpose",
                                 StandardRangeSpecs::gTransposeRange,
                                 Property<int>{[](void *cap) FLASHMEM {
                                                   auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                   return (int)pThis->GetBinding().mTranspose.GetValue();
                                               },
                                               [](void *cap, const int &v) {
                                                   auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                   pThis->GetBinding().mTranspose.SetValue(v);
                                               },
                                               this},
                                 AlwaysEnabled};

    GainSettingItem mReverbGain = {"Reverb gain",
                                   StandardRangeSpecs::gGeneralGain,
                                   Property<float>{[](void *cap) FLASHMEM {
                                                       auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                       return pThis->GetBinding().mReverb.mGain.GetValue();
                                                   },
                                                   [](void *cap, const float &v) {
                                                       auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                       pThis->GetBinding().mReverb.mGain.SetValue(v);
                                                   },
                                                   this},
                                   AlwaysEnabled};

    FloatSettingItem mStereoSpread = {"B Spread",
                                      StandardRangeSpecs::gFloat_0_1,
                                      Property<float>{[](void *cap) FLASHMEM {
                                                          auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                          return pThis->GetBinding().mSynthStereoSpread.GetValue();
                                                      },
                                                      [](void *cap, const float &v) {
                                                          auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                          pThis->GetBinding().mSynthStereoSpread.SetValue(v);
                                                      },
                                                      this},
                                      AlwaysEnabled};

    FloatSettingItem mReverbDamping = {"Reverb damp",
                                       StandardRangeSpecs::gFloat_0_1,
                                       Property<float>{[](void *cap) FLASHMEM {
                                                           auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                           return pThis->GetBinding().mReverb.mDamping.GetValue();
                                                       },
                                                       [](void *cap, const float &v) {
                                                           auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                           pThis->GetBinding().mReverb.mDamping.SetValue(v);
                                                       },
                                                       this},
                                       AlwaysEnabled};

    FloatSettingItem mReverbSize = {"Reverb size",
                                    StandardRangeSpecs::gFloat_0_1,
                                    Property<float>{[](void *cap) FLASHMEM {
                                                        auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                        return pThis->GetBinding().mReverb.mSize.GetValue();
                                                    },
                                                    [](void *cap, const float &v) {
                                                        auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                        pThis->GetBinding().mReverb.mSize.SetValue(v);
                                                    },
                                                    this},
                                    AlwaysEnabled};

    GainSettingItem mDelayGain = {"Delay gain",
                                  StandardRangeSpecs::gGeneralGain,
                                  Property<float>{[](void *cap) FLASHMEM {
                                                      auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                      return pThis->GetBinding().mDelay.mGain.GetValue();
                                                  },
                                                  [](void *cap, const float &v) {
                                                      auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                      pThis->GetBinding().mDelay.mGain.SetValue(v);
                                                  },
                                                  this},
                                  AlwaysEnabled};

    FloatSettingItem mDelayTimeMS = {"Delay Time",
                                     NumericEditRangeSpec<float>(1, MAX_DELAY_MS),
                                     Property<float>{[](void *cap) FLASHMEM {
                                                         auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                         return pThis->GetBinding().mDelay.mTime.mParamValue.GetValue();
                                                     },
                                                     [](void *cap, const float &v) {
                                                         auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                         pThis->GetBinding().mDelay.mTime.mParamValue.SetValue(v);
                                                     },
                                                     this},
                                     AlwaysEnabled};

    FloatSettingItem mDelayStereoSep = {
        " >Width",
        NumericEditRangeSpec<float>(1, 100),
        Property<float>{[](void *cap) FLASHMEM {
                            auto *pThis = (PerformancePatchSettingsApp *)cap;
                            return pThis->GetBinding().mDelay.mStereoSeparationDelayMS.GetValue();
                        },
                        [](void *cap, const float &v) {
                            auto *pThis = (PerformancePatchSettingsApp *)cap;
                            pThis->GetBinding().mDelay.mStereoSeparationDelayMS.SetValue(v);
                        },
                        this},
        AlwaysEnabled};

    FloatSettingItem mDelayFeedbackLevel = {
        " >FB",
        StandardRangeSpecs::gFloat_0_1,
        Property<float>{[](void *cap) FLASHMEM {
                            auto *pThis = (PerformancePatchSettingsApp *)cap;
                            return pThis->GetBinding().mDelay.mFeedbackGain.GetValue();
                        },
                        [](void *cap, const float &v) {
                            auto *pThis = (PerformancePatchSettingsApp *)cap;
                            pThis->GetBinding().mDelay.mFeedbackGain.SetValue(v);
                        },
                        this},
        AlwaysEnabled};

    EnumSettingItem<ClarinoidFilterType> mDelayFilterType = {
        " >Filter",
        gClarinoidFilterTypeInfo,
        Property<ClarinoidFilterType>{[](void *cap) FLASHMEM {
                                          auto *pThis = (PerformancePatchSettingsApp *)cap;
                                          return pThis->GetBinding().mDelay.mFilter.mType.GetValue();
                                      },
                                      [](void *cap, const ClarinoidFilterType &v) {
                                          auto *pThis = (PerformancePatchSettingsApp *)cap;
                                          pThis->GetBinding().mDelay.mFilter.mType.SetValue(v);
                                      },
                                      this},
        AlwaysEnabled};

    FloatSettingItem mDelayCutoffFrequency = {
        " > >Freq",
        StandardRangeSpecs::gFloat_0_1,
        Property<float>{[](void *cap) FLASHMEM {
                            auto *pThis = (PerformancePatchSettingsApp *)cap;
                            return pThis->GetBinding().mDelay.mFilter.mFrequency.GetParamValue();
                        },
                        [](void *cap, const float &v) {
                            auto *pThis = (PerformancePatchSettingsApp *)cap;
                            pThis->GetBinding().mDelay.mFilter.mFrequency.SetParamValue(v);
                        },
                        this},
        AlwaysEnabled};

    FloatSettingItem mDelaySaturation = {
        " > >Sat",
        StandardRangeSpecs::gFloat_0_1,
        Property<float>{[](void *cap) FLASHMEM {
                            auto *pThis = (PerformancePatchSettingsApp *)cap;
                            return pThis->GetBinding().mDelay.mFilter.mSaturation.GetValue();
                        },
                        [](void *cap, const float &v) {
                            auto *pThis = (PerformancePatchSettingsApp *)cap;
                            pThis->GetBinding().mDelay.mFilter.mSaturation.SetValue(v);
                        },
                        this},
        AlwaysEnabled};

    FloatSettingItem mDelayQ = {" > >Q",
                                StandardRangeSpecs::gFloat_0_1,
                                Property<float>{[](void *cap) FLASHMEM {
                                                    auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                    return pThis->GetBinding().mDelay.mFilter.mQ.GetValue();
                                                },
                                                [](void *cap, const float &v) {
                                                    auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                    pThis->GetBinding().mDelay.mFilter.mQ.SetValue(v);
                                                },
                                                this},
                                AlwaysEnabled};

    BoolSettingItem mMasterFXEnable = {"MasterFX Enable",
                                       "Yes",
                                       "No",
                                       Property<bool>{[](void *cap) FLASHMEM {
                                                          auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                          return pThis->GetBinding().mMasterFXEnable.GetValue();
                                                      },
                                                      [](void *cap, const bool &v) {
                                                          auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                          pThis->GetBinding().mMasterFXEnable.SetValue(v);
                                                      },
                                                      this},
                                       AlwaysEnabled};

    IntSettingItem mSelectedSynthPatchA = {"Synth patch A",
                                           NumericEditRangeSpec<int>{0, clarinoid::SYNTH_PRESET_COUNT - 1},
                                           Property<int>{
                                               [](void *cap) FLASHMEM {
                                                   auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                   return (int)pThis->GetBinding().mSynthPatchA.GetValue();
                                               }, // getter
                                               [](void *cap, const int &val) {
                                                   auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                   pThis->GetBinding().mSynthPatchA.SetValue(val);
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
                                                   return (int)pThis->GetBinding().mSynthPatchB.GetValue();
                                               }, // getter
                                               [](void *cap, const int &val) {
                                                   auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                   pThis->GetBinding().mSynthPatchB.SetValue(val);
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
                                                 return (int)pThis->GetBinding().mHarmPreset.GetValue();
                                             }, // getter
                                             [](void *cap, const int &val) {
                                                 auto *pThis = (PerformancePatchSettingsApp *)cap;
                                                 pThis->GetBinding().mHarmPreset.SetValue(val);
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
        mDisplay.println(GetAppSettings()->GetPerfPatchName(GetAppSettings()->mCurrentPerformancePatch.GetValue()));

        SettingsMenuApp::RenderFrontPage();
    }
};

} // namespace clarinoid
