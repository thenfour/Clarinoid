#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"
#include "NumericSettingItem.hpp"
#include "EnumSettingItem.hpp"
#include "FunctionListSettingItem.hpp"
#include "GainSettingItem.hpp"

namespace clarinoid
{

struct EnvelopeMenuApp
{
    BoolSettingItem mLegatoRetrig = {"Legato Retrig",
                                "On",
                                "Off",
                                Property<bool>{[](void *cap) FLASHMEM {
                                                     auto *pThis = (EnvelopeMenuApp *)cap;
                                                     return pThis->mBinding->mLegatoRestart;
                                               },
                                               [](void *cap, const bool &v) FLASHMEM {
                                                     auto *pThis = (EnvelopeMenuApp *)cap;
                                                     pThis->mBinding->mLegatoRestart = v;
                                               },
                                               this},
                                AlwaysEnabled};


    FloatSettingItem mDelayMS = {"Delay",
                                 StandardRangeSpecs::gEnvDelayMS,
                                 Property<float>{[](void *cap) FLASHMEM {
                                                     auto *pThis = (EnvelopeMenuApp *)cap;
                                                     return pThis->mBinding->mDelayMS;
                                                 },
                                                 [](void *cap, const float &v) FLASHMEM {
                                                     auto *pThis = (EnvelopeMenuApp *)cap;
                                                     pThis->mBinding->mDelayMS = v;
                                                 },
                                                 this},
                                 AlwaysEnabled};

    FloatSettingItem mAttackMS = {"Attack",
                                  StandardRangeSpecs::gEnvAttackMS,
                                  Property<float>{[](void *cap) FLASHMEM {
                                                      auto *pThis = (EnvelopeMenuApp *)cap;
                                                      return pThis->mBinding->mAttackMS;
                                                  },
                                                  [](void *cap, const float &v) FLASHMEM {
                                                      auto *pThis = (EnvelopeMenuApp *)cap;
                                                      pThis->mBinding->mAttackMS = v;
                                                  },
                                                  this},
                                  AlwaysEnabled};

    FloatSettingItem mHoldMS = {"Hold",
                                StandardRangeSpecs::gEnvHoldMS,
                                Property<float>{[](void *cap) FLASHMEM {
                                                    auto *pThis = (EnvelopeMenuApp *)cap;
                                                    return pThis->mBinding->mHoldMS;
                                                },
                                                [](void *cap, const float &v) FLASHMEM {
                                                    auto *pThis = (EnvelopeMenuApp *)cap;
                                                    pThis->mBinding->mHoldMS = v;
                                                },
                                                this},
                                AlwaysEnabled};

    FloatSettingItem mDecayMS = {"Decay",
                                 StandardRangeSpecs::gEnvDecayMS,
                                 Property<float>{[](void *cap) FLASHMEM {
                                                     auto *pThis = (EnvelopeMenuApp *)cap;
                                                     return pThis->mBinding->mDecayMS;
                                                 },
                                                 [](void *cap, const float &v) FLASHMEM {
                                                     auto *pThis = (EnvelopeMenuApp *)cap;
                                                     pThis->mBinding->mDecayMS = v;
                                                 },
                                                 this},
                                 AlwaysEnabled};

    FloatSettingItem mSustainLevel = {"Sustain",
                                      StandardRangeSpecs::gEnvSustainLevel,
                                      Property<float>{[](void *cap) FLASHMEM {
                                                          auto *pThis = (EnvelopeMenuApp *)cap;
                                                          return pThis->mBinding->mSustainLevel;
                                                      },
                                                      [](void *cap, const float &v) FLASHMEM {
                                                          auto *pThis = (EnvelopeMenuApp *)cap;
                                                          pThis->mBinding->mSustainLevel = v;
                                                      },
                                                      this},
                                      AlwaysEnabled};

    FloatSettingItem mReleaseMS = {"Release",
                                   StandardRangeSpecs::gEnvReleaseMS,
                                   Property<float>{[](void *cap) FLASHMEM {
                                                       auto *pThis = (EnvelopeMenuApp *)cap;
                                                       return pThis->mBinding->mReleaseMS;
                                                   },
                                                   [](void *cap, const float &v) FLASHMEM {
                                                       auto *pThis = (EnvelopeMenuApp *)cap;
                                                       pThis->mBinding->mReleaseMS = v;
                                                   },
                                                   this},
                                   AlwaysEnabled};

    // FloatSettingItem mReleaseNoteOnMS = {"ReleaseNoteOn",
    //                                      StandardRangeSpecs::gEnvReleaseMS,
    //                                      Property<float>{[](void *cap) FLASHMEM {
    //                                                          auto *pThis = (EnvelopeMenuApp *)cap;
    //                                                          return pThis->mBinding->mReleaseNoteOnMS;
    //                                                      },
    //                                                      [](void *cap, const float &v) FLASHMEM {
    //                                                          auto *pThis = (EnvelopeMenuApp *)cap;
    //                                                          pThis->mBinding->mReleaseNoteOnMS = v;
    //                                                      },
    //                                                      this},
    //                                      AlwaysEnabled};

    ISettingItem *mArray[7] = {
        &mLegatoRetrig,
        &mDelayMS,
        &mAttackMS,
        &mHoldMS,
        &mDecayMS,
        &mSustainLevel,
        &mReleaseMS,
        //&mReleaseNoteOnMS,
    };

    SettingsList mRootList = {mArray};

    EnvelopeSpec *mBinding = nullptr;

    SettingsList *GetSubmenuList(EnvelopeSpec &binding)
    {
        mBinding = &binding;
        return &mRootList;
    }
};

struct SynthPatchOscillatorMenuStuff
{
    SynthOscillatorSettings *mpBinding = nullptr;
    SynthOscillatorSettings &GetBinding()
    {
        return *mpBinding;
    }

    BoolSettingItem mEnabled = {"Enabled",
                                "Yes",
                                "No",
                                Property<bool>{[](void *cap) FLASHMEM {
                                                   auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                   return pThis->GetBinding().mEnabled;
                                               },
                                               [](void *cap, const bool &v) FLASHMEM {
                                                   auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                   pThis->GetBinding().mEnabled = v;
                                               },
                                               this},
                                AlwaysEnabled};

    // EnumSettingItem(const String& name, const EnumInfo<T>& enumInfo, const Property<T>& binding,
    // cc::function<bool()>::ptr_t isEnabled) :
    EnumSettingItem<OscWaveformShape> mWaveform = {
        "Waveform",
        gOscWaveformShapeInfo,
        Property<OscWaveformShape>{[](void *cap) FLASHMEM {
                                       auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                       return pThis->GetBinding().mWaveform;
                                   },
                                   [](void *cap, const OscWaveformShape &v) FLASHMEM {
                                       auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                       pThis->GetBinding().mWaveform = v;
                                   },
                                   this},
        AlwaysEnabled};

    FloatSettingItem mGain = {"Volume",
                             StandardRangeSpecs::gFloat_0_1,
                             Property<float>{[](void *cap) FLASHMEM {
                                                 auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                 return pThis->GetBinding().mVolume.GetParamValue();
                                             },
                                             [](void *cap, const float &v) FLASHMEM {
                                                 auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                 pThis->GetBinding().mVolume.SetValue(v);
                                             },
                                             this},
                             AlwaysEnabled};

    FloatSettingItem mFMFeedback = {"FM Feedback",
                                    StandardRangeSpecs::gFloat_0_1,
                                    Property<float>{[](void *cap) FLASHMEM {
                                                        auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                        return pThis->GetBinding().mFMFeedbackGain;
                                                    },
                                                    [](void *cap, const float &v) FLASHMEM {
                                                        auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                        pThis->GetBinding().mFMFeedbackGain = v;
                                                    },
                                                    this},
                                    AlwaysEnabled};

    // IntSettingItem mCurve = {"Curve",
    //                          StandardRangeSpecs::gCurveIndexRange,
    //                          Property<int>{[](void *cap) FLASHMEM {
    //                                            auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
    //                                            return (int)pThis->GetBinding().mCurveIndex;
    //                                        },
    //                                        [](void *cap, const int &v) FLASHMEM {
    //                                            auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
    //                                            pThis->GetBinding().mCurveIndex = v;
    //                                        },
    //                                        this},
    //                          AlwaysEnabled};

    FloatSettingItem mPan = {"Pan",
                             StandardRangeSpecs::gFloat_N1_1,
                             Property<float>{[](void *cap) FLASHMEM {
                                                 auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                 return pThis->GetBinding().mPan;
                                             },
                                             [](void *cap, const float &v) FLASHMEM {
                                                 auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                 pThis->GetBinding().mPan = v;
                                             },
                                             this},
                             AlwaysEnabled};

    IntSettingItem mPortamentoTimeMS = {"Portamento MS",
                                        StandardRangeSpecs::gPortamentoRange,
                                        Property<int>{[](void *cap) FLASHMEM {
                                                          auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                          return pThis->GetBinding().mPortamentoTimeMS;
                                                      },
                                                      [](void *cap, const int &v) FLASHMEM {
                                                          auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                          pThis->GetBinding().mPortamentoTimeMS = v;
                                                      },
                                                      this},
                                        AlwaysEnabled};

    FloatSettingItem mFreqMul = {"FreqMul",
                                 StandardRangeSpecs::gFreqMulRange,
                                 Property<float>{[](void *cap) FLASHMEM {
                                                     auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                     return pThis->GetBinding().mFreqMultiplier;
                                                 },
                                                 [](void *cap, const float &v) FLASHMEM {
                                                     auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                     pThis->GetBinding().mFreqMultiplier = v;
                                                 },
                                                 this},
                                 AlwaysEnabled};

    FloatSettingItem mFreqOffset = {"FreqOffset",
                                    StandardRangeSpecs::gFreqOffsetRange,
                                    Property<float>{[](void *cap) FLASHMEM {
                                                        auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                        return pThis->GetBinding().mFreqOffsetHz;
                                                    },
                                                    [](void *cap, const float &v) FLASHMEM {
                                                        auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                        pThis->GetBinding().mFreqOffsetHz = v;
                                                    },
                                                    this},
                                    AlwaysEnabled};

    FloatSettingItem mFreqParam = {"FreqParam",
                                   StandardRangeSpecs::gFloat_0_1,
                                   Property<float>{[](void *cap) FLASHMEM {
                                                       auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                       return pThis->GetBinding().mFreqParam;
                                                   },
                                                   [](void *cap, const float &v) FLASHMEM {
                                                       auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                       pThis->GetBinding().mFreqParam = v;
                                                   },
                                                   this},
                                   AlwaysEnabled};

    FloatSettingItem mFreqParamKT = {"FreqParamKT",
                                     StandardRangeSpecs::gFloat_0_1,
                                     Property<float>{[](void *cap) FLASHMEM {
                                                         auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                         return pThis->GetBinding().mFreqParamKT;
                                                     },
                                                     [](void *cap, const float &v) FLASHMEM {
                                                         auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                         pThis->GetBinding().mFreqParamKT = v;
                                                     },
                                                     this},
                                     AlwaysEnabled};

    IntSettingItem mPitchSemis = {"PitchSemis",
                                  StandardRangeSpecs::gTransposeRange,
                                  Property<int>{[](void *cap) FLASHMEM {
                                                    auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                    return pThis->GetBinding().mPitchSemis;
                                                },
                                                [](void *cap, const int &v) FLASHMEM {
                                                    auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                    pThis->GetBinding().mPitchSemis = v;
                                                },
                                                this},
                                  AlwaysEnabled};

    FloatSettingItem mPitchFine = {"PitchFine",
                                   StandardRangeSpecs::gFloat_N1_1,
                                   Property<float>{[](void *cap) FLASHMEM {
                                                       auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                       return pThis->GetBinding().mPitchFine;
                                                   },
                                                   [](void *cap, const float &v) FLASHMEM {
                                                       auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                       pThis->GetBinding().mPitchFine = v;
                                                   },
                                                   this},
                                   AlwaysEnabled};

    FloatSettingItem mPulseWidth = {"PulseWidth",
                                    StandardRangeSpecs::gFloat_0_1,
                                    Property<float>{[](void *cap) FLASHMEM {
                                                        auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                        return pThis->GetBinding().mPulseWidth;
                                                    },
                                                    [](void *cap, const float &v) FLASHMEM {
                                                        auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                        pThis->GetBinding().mPulseWidth = v;
                                                    },
                                                    this},
                                    AlwaysEnabled};

    BoolSettingItem mPhaseRestart = {"PhaseRestart",
                                     "Yes",
                                     "No",
                                     Property<bool>{[](void *cap) FLASHMEM {
                                                        auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                        return pThis->GetBinding().mPhaseRestart;
                                                    },
                                                    [](void *cap, const bool &v) FLASHMEM {
                                                        auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                        pThis->GetBinding().mPhaseRestart = v;
                                                    },
                                                    this},
                                     AlwaysEnabled};

    FloatSettingItem mPhaseOffset = {"PhaseOffset",
                                     StandardRangeSpecs::gFloat_0_1,
                                     Property<float>{[](void *cap) FLASHMEM {
                                                         auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                         return pThis->GetBinding().mPhase01;
                                                     },
                                                     [](void *cap, const float &v) FLASHMEM {
                                                         auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                         pThis->GetBinding().mPhase01 = v;
                                                     },
                                                     this},
                                     AlwaysEnabled};

    ISettingItem *mArray[15] = {
        &mEnabled,
        &mWaveform,
        &mGain,
        &mFMFeedback,
        //&mCurve,
        //&mAMMinimumGain,
        &mPan,
        &mPortamentoTimeMS,
        &mFreqMul,
        &mFreqOffset,
        &mFreqParam,
        &mFreqParamKT,
        &mPitchSemis,
        &mPitchFine,
        &mPulseWidth,
        &mPhaseRestart,
        &mPhaseOffset,
    };

    SettingsList mSubmenuList = {mArray};
};

// acts as a standalone "current patch" editor app,
// as well as sub-app of general synth settings. so you can change params of patches without them even being selected.
struct SynthPatchMenuApp : public SettingsMenuApp
{
    virtual const char *DisplayAppGetName() override
    {
        return "SynthPatchMenuApp";
    }

    SynthPatchMenuApp(IDisplay &d) : SettingsMenuApp(d) //, mDisplay(d)
    {
    }

    // int16_t mBindingID = 0;
    SynthPreset &GetBinding()
    {
        return GetAppSettings()->FindSynthPreset(GetBindingID());
    }
    int16_t GetBindingID()
    {
        // return mBindingID;
        return GetAppSettings()->GetCurrentPerformancePatch().mSynthPresetA;
    }

    // NumericSettingItem(const String& name, T min_, T max_, const Property<T>& binding, typename
    // cc::function<bool()>::ptr_t isEnabled) : BoolSettingItem(const String& name, const String& trueCaption, const
    // String& falseCaption, const Property<bool>& binding, typename cc::function<bool()>::ptr_t isEnabled) :
    FloatSettingItem mDetune = {"Detune",
                                StandardRangeSpecs::gFloat_0_2,
                                Property<float>{[](void *cap) FLASHMEM {
                                                    auto *pThis = (SynthPatchMenuApp *)cap;
                                                    return pThis->GetBinding().mDetune;
                                                },
                                                [](void *cap, const float &v) FLASHMEM {
                                                    auto *pThis = (SynthPatchMenuApp *)cap;
                                                    pThis->GetBinding().mDetune = v;
                                                },
                                                this},
                                AlwaysEnabled};

    FloatSettingItem mPan = {"Pan",
                             StandardRangeSpecs::gFloat_N1_1,
                             Property<float>{[](void *cap) FLASHMEM {
                                                 auto *pThis = (SynthPatchMenuApp *)cap;
                                                 return pThis->GetBinding().mPan;
                                             },
                                             [](void *cap, const float &v) FLASHMEM {
                                                 auto *pThis = (SynthPatchMenuApp *)cap;
                                                 pThis->GetBinding().mPan = v;
                                             },
                                             this},
                             AlwaysEnabled};

    FloatSettingItem mVerbMix = {"Verb Mix",
                                 StandardRangeSpecs::gFloat_0_1,
                                 Property<float>{[](void *cap) FLASHMEM {
                                                     auto *pThis = (SynthPatchMenuApp *)cap;
                                                     return pThis->GetBinding().mVerbMix;
                                                 },
                                                 [](void *cap, const float &v) {
                                                     auto *pThis = (SynthPatchMenuApp *)cap;
                                                     pThis->GetBinding().mVerbMix = v;
                                                 },
                                                 this},
                                 AlwaysEnabled};

    FloatSettingItem mDelayMix = {"Dly Mix",
                                  StandardRangeSpecs::gFloat_0_1,
                                  Property<float>{[](void *cap) FLASHMEM {
                                                      auto *pThis = (SynthPatchMenuApp *)cap;
                                                      return pThis->GetBinding().mDelayMix;
                                                  },
                                                  [](void *cap, const float &v) {
                                                      auto *pThis = (SynthPatchMenuApp *)cap;
                                                      pThis->GetBinding().mDelayMix = v;
                                                  },
                                                  this},
                                  AlwaysEnabled};

    // BoolSettingItem mSync = {"Sync",
    //                          "On",
    //                          "Off",
    //                          Property<bool>{[](void *cap) FLASHMEM {
    //                                             auto *pThis = (SynthPatchMenuApp *)cap;
    //                                             return pThis->GetBinding().mSync;
    //                                         },
    //                                         [](void *cap, const bool &v) {
    //                                             auto *pThis = (SynthPatchMenuApp *)cap;
    //                                             pThis->GetBinding().mSync = v;
    //                                         },
    //                                         this},
    //                          AlwaysEnabled};

    FloatSettingItem mStereoSpread = {"Stereo Spread",
                                      StandardRangeSpecs::gFloat_0_1,
                                      Property<float>{[](void *cap) FLASHMEM {
                                                          auto *pThis = (SynthPatchMenuApp *)cap;
                                                          return pThis->GetBinding().mStereoSpread;
                                                      },
                                                      [](void *cap, const float &v) {
                                                          auto *pThis = (SynthPatchMenuApp *)cap;
                                                          pThis->GetBinding().mStereoSpread = v;
                                                      },
                                                      this},
                                      AlwaysEnabled};

    FloatSettingItem mFMStrength = {"FM strength",
                                    StandardRangeSpecs::gOverallFMStrengthRange,
                                    Property<float>{[](void *cap) FLASHMEM {
                                                        auto *pThis = (SynthPatchMenuApp *)cap;
                                                        return pThis->GetBinding().mOverallFMStrength;
                                                    },
                                                    [](void *cap, const float &v) {
                                                        auto *pThis = (SynthPatchMenuApp *)cap;
                                                        pThis->GetBinding().mOverallFMStrength = v;
                                                    },
                                                    this},
                                    AlwaysEnabled};

    FloatSettingItem mFMStrength1To2 = {"  1 -> 2",
                                        StandardRangeSpecs::gFloat_0_1_Fine,
                                        Property<float>{[](void *cap) FLASHMEM {
                                                            auto *pThis = (SynthPatchMenuApp *)cap;
                                                            return pThis->GetBinding().mFMStrength1To2;
                                                        },
                                                        [](void *cap, const float &v) {
                                                            auto *pThis = (SynthPatchMenuApp *)cap;
                                                            pThis->GetBinding().mFMStrength1To2 = v;
                                                        },
                                                        this},
                                        AlwaysEnabled};

    FloatSettingItem mFMStrength1To3 = {"  1 -> 3",
                                        StandardRangeSpecs::gFloat_0_1_Fine,
                                        Property<float>{[](void *cap) FLASHMEM {
                                                            auto *pThis = (SynthPatchMenuApp *)cap;
                                                            return pThis->GetBinding().mFMStrength1To3;
                                                        },
                                                        [](void *cap, const float &v) {
                                                            auto *pThis = (SynthPatchMenuApp *)cap;
                                                            pThis->GetBinding().mFMStrength1To3 = v;
                                                        },
                                                        this},
                                        AlwaysEnabled};

    FloatSettingItem mFMStrength2To1 = {"  2 -> 1",
                                        StandardRangeSpecs::gFloat_0_1_Fine,
                                        Property<float>{[](void *cap) FLASHMEM {
                                                            auto *pThis = (SynthPatchMenuApp *)cap;
                                                            return pThis->GetBinding().mFMStrength2To1;
                                                        },
                                                        [](void *cap, const float &v) {
                                                            auto *pThis = (SynthPatchMenuApp *)cap;
                                                            pThis->GetBinding().mFMStrength2To1 = v;
                                                        },
                                                        this},
                                        AlwaysEnabled};

    FloatSettingItem mFMStrength3To1 = {"  3 -> 1",
                                        StandardRangeSpecs::gFloat_0_1_Fine,
                                        Property<float>{[](void *cap) FLASHMEM {
                                                            auto *pThis = (SynthPatchMenuApp *)cap;
                                                            return pThis->GetBinding().mFMStrength3To1;
                                                        },
                                                        [](void *cap, const float &v) {
                                                            auto *pThis = (SynthPatchMenuApp *)cap;
                                                            pThis->GetBinding().mFMStrength3To1 = v;
                                                        },
                                                        this},
                                        AlwaysEnabled};

    FloatSettingItem mFMStrength2To3 = {"  2 -> 3",
                                        StandardRangeSpecs::gFloat_0_1_Fine,
                                        Property<float>{[](void *cap) FLASHMEM {
                                                            auto *pThis = (SynthPatchMenuApp *)cap;
                                                            return pThis->GetBinding().mFMStrength2To3;
                                                        },
                                                        [](void *cap, const float &v) {
                                                            auto *pThis = (SynthPatchMenuApp *)cap;
                                                            pThis->GetBinding().mFMStrength2To3 = v;
                                                        },
                                                        this},
                                        AlwaysEnabled};

    FloatSettingItem mFMStrength3To2 = {"  3 -> 2",
                                        StandardRangeSpecs::gFloat_0_1_Fine,
                                        Property<float>{[](void *cap) FLASHMEM {
                                                            auto *pThis = (SynthPatchMenuApp *)cap;
                                                            return pThis->GetBinding().mFMStrength3To2;
                                                        },
                                                        [](void *cap, const float &v) {
                                                            auto *pThis = (SynthPatchMenuApp *)cap;
                                                            pThis->GetBinding().mFMStrength3To2 = v;
                                                        },
                                                        this},
                                        AlwaysEnabled};

    // EnumSettingItem<FMAlgo> mFMAlgo = {"FM Algo",
    //                                    gFMAlgoInfo,
    //                                    Property<FMAlgo>{[](void *cap) FLASHMEM {
    //                                                         auto *pThis = (SynthPatchMenuApp *)cap;
    //                                                         return pThis->GetBinding().mFMAlgo;
    //                                                     },
    //                                                     [](void *cap, const FMAlgo &v) {
    //                                                         auto *pThis = (SynthPatchMenuApp *)cap;
    //                                                         pThis->GetBinding().mFMAlgo = v;
    //                                                     },
    //                                                     this},
    //                                    AlwaysEnabled};

    // FloatSettingItem mFMStrength = {"FM Strength",
    //                                 NumericEditRangeSpec<float>{0.0f, 0.25f},
    //                                 Property<float>{[](void *cap) FLASHMEM {
    //                                                     auto *pThis = (SynthPatchMenuApp *)cap;
    //                                                     return pThis->GetBinding().mFMStrength;
    //                                                 },
    //                                                 [](void *cap, const float &v) {
    //                                                     auto *pThis = (SynthPatchMenuApp *)cap;
    //                                                     pThis->GetBinding().mFMStrength = v;
    //                                                 },
    //                                                 this},
    //                                 AlwaysEnabled};

    // FloatSettingItem mSyncMultMin = {" - mult min",
    //                                  NumericEditRangeSpec<float>{0.0f, 15.0f},
    //                                  Property<float>{[](void *cap) FLASHMEM {
    //                                                      auto *pThis = (SynthPatchMenuApp *)cap;
    //                                                      return pThis->GetBinding().mSyncMultMin;
    //                                                  },
    //                                                  [](void *cap, const float &v) {
    //                                                      auto *pThis = (SynthPatchMenuApp *)cap;
    //                                                      pThis->GetBinding().mSyncMultMin = v;
    //                                                  },
    //                                                  this},
    //                                  Property<bool>{[](void *cap) FLASHMEM {
    //                                                     auto *pThis = (SynthPatchMenuApp *)cap;
    //                                                     return pThis->GetBinding().mSync;
    //                                                 },
    //                                                 this}};

    // FloatSettingItem mSyncMultMax = {" - mult max",
    //                                  NumericEditRangeSpec<float>{0.0f, 15.0f},
    //                                  Property<float>{[](void *cap) FLASHMEM {
    //                                                      auto *pThis = (SynthPatchMenuApp *)cap;
    //                                                      return pThis->GetBinding().mSyncMultMax;
    //                                                  },
    //                                                  [](void *cap, const float &v) {
    //                                                      auto *pThis = (SynthPatchMenuApp *)cap;
    //                                                      pThis->GetBinding().mSyncMultMax = v;
    //                                                  },
    //                                                  this},
    //                                  Property<bool>{[](void *cap) FLASHMEM {
    //                                                     auto *pThis = (SynthPatchMenuApp *)cap;
    //                                                     return pThis->GetBinding().mSync;
    //                                                 },
    //                                                 this}};


    // EnumSettingItem(const String& name, const EnumInfo<T>& enumInfo, const Property<T>& binding,
    // cc::function<bool()>::ptr_t isEnabled) :
    EnumSettingItem<VoicingMode> mVoicingMode = {
        "Voicing",
        gVoicingModeInfo,
        Property<VoicingMode>{[](void *cap) FLASHMEM {
                                          auto *pThis = (SynthPatchMenuApp *)cap;
                                          return pThis->GetBinding().mVoicingMode;
                                      },
                                      [](void *cap, const VoicingMode &v) {
                                          auto *pThis = (SynthPatchMenuApp *)cap;
                                          pThis->GetBinding().mVoicingMode = v;
                                      },
                                      this},
        AlwaysEnabled};

    // EnumSettingItem(const String& name, const EnumInfo<T>& enumInfo, const Property<T>& binding,
    // cc::function<bool()>::ptr_t isEnabled) :
    EnumSettingItem<ClarinoidFilterType> mBreathFiltType = {
        "Filter",
        gClarinoidFilterTypeInfo,
        Property<ClarinoidFilterType>{[](void *cap) FLASHMEM {
                                          auto *pThis = (SynthPatchMenuApp *)cap;
                                          return pThis->GetBinding().mFilterType;
                                      },
                                      [](void *cap, const ClarinoidFilterType &v) {
                                          auto *pThis = (SynthPatchMenuApp *)cap;
                                          pThis->GetBinding().mFilterType = v;
                                      },
                                      this},
        AlwaysEnabled};

    FloatSettingItem mBreathFiltQ = {" - Reso",
                                     NumericEditRangeSpec<float>{0.0f, 1.0f, .1f, .05f, 0.1f},
                                     Property<float>{[](void *cap) FLASHMEM {
                                                         auto *pThis = (SynthPatchMenuApp *)cap;
                                                         return pThis->GetBinding().mFilterQ;
                                                     },
                                                     [](void *cap, const float &v) {
                                                         auto *pThis = (SynthPatchMenuApp *)cap;
                                                         pThis->GetBinding().mFilterQ = v;
                                                     },
                                                     this},
                                     AlwaysEnabled};

    FloatSettingItem mBreathFiltSaturation = {" - saturation",
                                              NumericEditRangeSpec<float>{0.0f, 1.0f},
                                              Property<float>{[](void *cap) FLASHMEM {
                                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                                  return pThis->GetBinding().mFilterSaturation;
                                                              },
                                                              [](void *cap, const float &v) {
                                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                                  pThis->GetBinding().mFilterSaturation = v;
                                                              },
                                                              this},
                                              AlwaysEnabled};

    FloatSettingItem mBreathFiltMax = {" - filt freq",
                                       StandardRangeSpecs::gFloat_0_1_Fine,
                                       Property<float>{[](void *cap) FLASHMEM {
                                                           auto *pThis = (SynthPatchMenuApp *)cap;
                                                           return pThis->GetBinding().mFilterFreq;
                                                       },
                                                       [](void *cap, const float &v) {
                                                           auto *pThis = (SynthPatchMenuApp *)cap;
                                                           pThis->GetBinding().mFilterFreq = v;
                                                       },
                                                       this},
                                       AlwaysEnabled};

    FloatSettingItem mBreathFiltKS = {" - filt KT",
                                      NumericEditRangeSpec<float>{0.0f, 2.0f, .2f, .1f, 0.01f},
                                      Property<float>{[](void *cap) FLASHMEM {
                                                          auto *pThis = (SynthPatchMenuApp *)cap;
                                                          return pThis->GetBinding().mFilterKeytracking;
                                                      },
                                                      [](void *cap, const float &v) {
                                                          auto *pThis = (SynthPatchMenuApp *)cap;
                                                          pThis->GetBinding().mFilterKeytracking = v;
                                                      },
                                                      this},
                                      AlwaysEnabled};

    BoolSettingItem mDCEnabled = {"DC filter",
                                  "On",
                                  "Off",
                                  Property<bool>{[](void *cap) FLASHMEM {
                                                     auto *pThis = (SynthPatchMenuApp *)cap;
                                                     return pThis->GetBinding().mDCFilterEnabled;
                                                 },
                                                 [](void *cap, const bool &v) {
                                                     auto *pThis = (SynthPatchMenuApp *)cap;
                                                     pThis->GetBinding().mDCFilterEnabled = v;
                                                 },
                                                 this},
                                  AlwaysEnabled};

    FloatSettingItem mDCCutoff = {" - cutoff",
                                  NumericEditRangeSpec<float>{0.01f, 100.0f},
                                  Property<float>{[](void *cap) FLASHMEM {
                                                      auto *pThis = (SynthPatchMenuApp *)cap;
                                                      return pThis->GetBinding().mDCFilterCutoff;
                                                  },
                                                  [](void *cap, const float &v) {
                                                      auto *pThis = (SynthPatchMenuApp *)cap;
                                                      pThis->GetBinding().mDCFilterCutoff = v;
                                                  },
                                                  this},
                                  Property<bool>{[](void *cap) FLASHMEM {
                                                     auto *pThis = (SynthPatchMenuApp *)cap;
                                                     return pThis->GetBinding().mDCFilterEnabled;
                                                 },
                                                 this}};

    FunctionListSettingItem mCopyPreset = {
        "Copy to ...",
        SYNTH_PRESET_COUNT,
        [](void *cap, size_t i) { // itemNameGetter,
            auto *pThis = (SynthPatchMenuApp *)cap;
            return pThis->GetAppSettings()->mSynthSettings.mPresets[i].ToString(i);
        },
        [](void *cap, size_t i) { // cc::function<void(void*,size_t)>::ptr_t onClick,
            auto *pThis = (SynthPatchMenuApp *)cap;
            pThis->GetAppSettings()->mSynthSettings.mPresets[i] = pThis->GetBinding();

            auto fromName = pThis->GetAppSettings()->GetSynthPatchName(pThis->GetBindingID());
            auto toName = pThis->GetAppSettings()->GetSynthPatchName(i);

            pThis->mDisplay.ShowToast(String("Copied ") + fromName + "\nto\n" + toName);
        },
        AlwaysEnabled,
        this};

    // FloatSettingItem mLfo1Frequency = {"LFO1 Freq",
    //                                    StandardRangeSpecs::gLFOFrequency,
    //                                    Property<float>{[](void *cap) FLASHMEM {
    //                                                        auto *pThis = (SynthPatchMenuApp *)cap;
    //                                                        return pThis->GetBinding().mLfo1Rate;
    //                                                    },
    //                                                    [](void *cap, const float &v) {
    //                                                        auto *pThis = (SynthPatchMenuApp *)cap;
    //                                                        pThis->GetBinding().mLfo1Rate = v;
    //                                                    },
    //                                                    this},
    //                                    AlwaysEnabled};

    // EnumSettingItem<OscWaveformShape> mLfo1Waveform = {
    //     " >Wave",
    //     gOscWaveformShapeInfo,
    //     Property<OscWaveformShape>{[](void *cap) FLASHMEM {
    //                                    auto *pThis = (SynthPatchMenuApp *)cap;
    //                                    return pThis->GetBinding().mLfo1Shape;
    //                                },
    //                                [](void *cap, const OscWaveformShape &v) {
    //                                    auto *pThis = (SynthPatchMenuApp *)cap;
    //                                    pThis->GetBinding().mLfo1Shape = v;
    //                                },
    //                                this},
    //     AlwaysEnabled};

    // FloatSettingItem mLfo2Frequency = {"LFO2 Freq",
    //                                    StandardRangeSpecs::gLFOFrequency,
    //                                    Property<float>{[](void *cap) FLASHMEM {
    //                                                        auto *pThis = (SynthPatchMenuApp *)cap;
    //                                                        return pThis->GetBinding().mLfo2Rate;
    //                                                    },
    //                                                    [](void *cap, const float &v) {
    //                                                        auto *pThis = (SynthPatchMenuApp *)cap;
    //                                                        pThis->GetBinding().mLfo2Rate = v;
    //                                                    },
    //                                                    this},
    //                                    AlwaysEnabled};

    // EnumSettingItem<OscWaveformShape> mLfo2Waveform = {
    //     " >Wave",
    //     gOscWaveformShapeInfo,
    //     Property<OscWaveformShape>{[](void *cap) FLASHMEM {
    //                                    auto *pThis = (SynthPatchMenuApp *)cap;
    //                                    return pThis->GetBinding().mLfo2Shape;
    //                                },
    //                                [](void *cap, const OscWaveformShape &v) {
    //                                    auto *pThis = (SynthPatchMenuApp *)cap;
    //                                    pThis->GetBinding().mLfo2Shape = v;
    //                                },
    //                                this},
    //     AlwaysEnabled};

    size_t mEditingModulationIndex = 0;
    SynthModulationSpec &GetModulationBinding()
    {
        return GetBinding().mModulations[mEditingModulationIndex];
    }

    EnumSettingItem<AnyModulationSource> mModSource = {
        "Source",
        gAnyModulationSourceInfo,
        Property<AnyModulationSource>{[](void *cap) FLASHMEM {
                                          auto *pThis = (SynthPatchMenuApp *)cap;
                                          return pThis->GetModulationBinding().mSource;
                                      },
                                      [](void *cap, const AnyModulationSource &v) {
                                          auto *pThis = (SynthPatchMenuApp *)cap;
                                          pThis->GetModulationBinding().mSource = v;
                                      },
                                      this},
        AlwaysEnabled};

    EnumSettingItem<AnyModulationDestination> mModDest = {
        "Dest",
        gAnyModulationDestinationInfo,
        Property<AnyModulationDestination>{[](void *cap) FLASHMEM {
                                               auto *pThis = (SynthPatchMenuApp *)cap;
                                               return pThis->GetModulationBinding().mDest;
                                           },
                                           [](void *cap, const AnyModulationDestination &v) {
                                               auto *pThis = (SynthPatchMenuApp *)cap;
                                               pThis->GetModulationBinding().mDest = v;
                                           },
                                           this},
        AlwaysEnabled};

    FloatSettingItem mModScale = {"Scale",
                                  StandardRangeSpecs::gFloat_N1_1,
                                  Property<float>{[](void *cap) FLASHMEM {
                                                      auto *pThis = (SynthPatchMenuApp *)cap;
                                                      return pThis->GetModulationBinding().mScaleN11;
                                                  },
                                                  [](void *cap, const float &v) {
                                                      auto *pThis = (SynthPatchMenuApp *)cap;
                                                      pThis->GetModulationBinding().mScaleN11 = v;
                                                  },
                                                  this},
                                  AlwaysEnabled};

    EnumSettingItem<ModulationPolarityTreatment> mModSourcePolarity = {
        "Polarity",
        gModulationPolarityTreatmentInfo,
        Property<ModulationPolarityTreatment>{[](void *cap) FLASHMEM {
                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                  return pThis->GetModulationBinding().mSourcePolarity;
                                              },
                                              [](void *cap, const ModulationPolarityTreatment &v) {
                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                  pThis->GetModulationBinding().mSourcePolarity = v;
                                              },
                                              this},
        AlwaysEnabled};

    IntSettingItem mModCurve = {"Curve",
                                StandardRangeSpecs::gCurveIndexRange,
                                Property<int>{[](void *cap) FLASHMEM {
                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                  return (int)pThis->GetModulationBinding().mCurveShape;
                                              },
                                              [](void *cap, const int &v) FLASHMEM {
                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                  pThis->GetModulationBinding().mCurveShape = v;
                                              },
                                              this},
                                AlwaysEnabled};

    BoolSettingItem mModAuxEnable = {"Aux enable",
                                     "On",
                                     "Off",
                                     Property<bool>{[](void *cap) FLASHMEM {
                                                        auto *pThis = (SynthPatchMenuApp *)cap;
                                                        return pThis->GetModulationBinding().mAuxEnabled;
                                                    },
                                                    [](void *cap, const bool &v) {
                                                        auto *pThis = (SynthPatchMenuApp *)cap;
                                                        pThis->GetModulationBinding().mAuxEnabled = v;
                                                    },
                                                    this},
                                     AlwaysEnabled};

    EnumSettingItem<AnyModulationSource> mModAuxSource = {
        " - Aux source",
        gAnyModulationSourceInfo,
        Property<AnyModulationSource>{[](void *cap) FLASHMEM {
                                          auto *pThis = (SynthPatchMenuApp *)cap;
                                          return pThis->GetModulationBinding().mAuxSource;
                                      },
                                      [](void *cap, const AnyModulationSource &v) {
                                          auto *pThis = (SynthPatchMenuApp *)cap;
                                          pThis->GetModulationBinding().mAuxSource = v;
                                      },
                                      this},
        AlwaysEnabled};

    FloatSettingItem mModAuxAmount = {" - Aux amt",
                                      StandardRangeSpecs::gFloat_0_1,
                                      Property<float>{[](void *cap) FLASHMEM {
                                                          auto *pThis = (SynthPatchMenuApp *)cap;
                                                          return pThis->GetModulationBinding().mAuxAmount01;
                                                      },
                                                      [](void *cap, const float &v) {
                                                          auto *pThis = (SynthPatchMenuApp *)cap;
                                                          pThis->GetModulationBinding().mAuxAmount01 = v;
                                                      },
                                                      this},
                                      AlwaysEnabled};

    EnumSettingItem<ModulationPolarityTreatment> mModAuxPolarity = {
        " - Aux polarity",
        gModulationPolarityTreatmentInfo,
        Property<ModulationPolarityTreatment>{[](void *cap) FLASHMEM {
                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                  return pThis->GetModulationBinding().mAuxPolarity;
                                              },
                                              [](void *cap, const ModulationPolarityTreatment &v) {
                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                  pThis->GetModulationBinding().mAuxPolarity = v;
                                              },
                                              this},
        AlwaysEnabled};

    IntSettingItem mModAuxCurve = {" - Aux Curve",
                                   StandardRangeSpecs::gCurveIndexRange,
                                   Property<int>{[](void *cap) FLASHMEM {
                                                     auto *pThis = (SynthPatchMenuApp *)cap;
                                                     return (int)pThis->GetModulationBinding().mAuxCurveShape;
                                                 },
                                                 [](void *cap, const int &v) FLASHMEM {
                                                     auto *pThis = (SynthPatchMenuApp *)cap;
                                                     pThis->GetModulationBinding().mAuxCurveShape = v;
                                                 },
                                                 this},
                                   AlwaysEnabled};

    ISettingItem *mModulationSubmenu[10] = {
        &mModSource,
        &mModDest,
        &mModScale,
        &mModSourcePolarity,
        &mModCurve,

        &mModAuxEnable,
        &mModAuxSource,
        &mModAuxAmount,
        &mModAuxPolarity,
        &mModAuxCurve,
    };
    SettingsList mModulationSubmenuList = {mModulationSubmenu};

    MultiSubmenuSettingItem mModulationsList = {[](void *cap) FLASHMEM { return SYNTH_MODULATIONS_MAX; },
                                                [](void *cap, size_t i) {
                                                    auto *pThis = (SynthPatchMenuApp *)cap;
                                                    return String(String("#") + i + " " +
                                                                  pThis->GetBinding().mModulations[i].ToString());
                                                },
                                                [](void *cap, size_t i) { // get submenu
                                                    auto *pThis = (SynthPatchMenuApp *)cap;
                                                    pThis->mEditingModulationIndex = i;
                                                    return &pThis->mModulationSubmenuList;
                                                },
                                                [](void *cap, size_t i) { return true; }, // isEnabled
                                                this};

    EnvelopeMenuApp mEnvEditor;

    SubmenuSettingItem mModEnv1SubmenuItem = {String("ENV1"),
                                              [](void *cap) FLASHMEM {
                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                  return pThis->mEnvEditor.GetSubmenuList(pThis->GetBinding().mEnv1);
                                              },
                                              AlwaysEnabled,
                                              this};

    SubmenuSettingItem mModEnv2SubmenuItem = {String("ENV2"),
                                              [](void *cap) FLASHMEM {
                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                  return pThis->mEnvEditor.GetSubmenuList(pThis->GetBinding().mEnv2);
                                              },
                                              AlwaysEnabled,
                                              this};

    ISettingItem *mModulationsSubmenuArray[3] = {
        //&mLfo1Frequency,
        //&mLfo1Waveform,
        //&mLfo2Frequency,
        //&mLfo2Waveform,
        &mModEnv1SubmenuItem,
        &mModEnv2SubmenuItem,
        &mModulationsList,
    };
    SettingsList mModulationsSubmenuList = {mModulationsSubmenuArray};

    SubmenuSettingItem mModulationsSubmenuItem = {String("Modulations"), &mModulationsSubmenuList, AlwaysEnabled};

    SynthPatchOscillatorMenuStuff mOscSubmenuStuff;

    SubmenuSettingItem mOsc1SubmenuItem = {[](void *cap) FLASHMEM -> String {
                                               auto *pThis = (SynthPatchMenuApp *)cap;
                                               return String("OSC1 ") + pThis->GetBinding().OscillatorToString(0);
                                           },
                                           [](void *cap) FLASHMEM {
                                               auto *pThis = (SynthPatchMenuApp *)cap;
                                               pThis->mOscSubmenuStuff.mpBinding = &pThis->GetBinding().mOsc[0];
                                               return &pThis->mOscSubmenuStuff.mSubmenuList;
                                           },
                                           AlwaysEnabled,
                                           this};

    SubmenuSettingItem mOsc2SubmenuItem = {String("OSC2"),
                                           [](void *cap) FLASHMEM {
                                               auto *pThis = (SynthPatchMenuApp *)cap;
                                               pThis->mOscSubmenuStuff.mpBinding = &pThis->GetBinding().mOsc[1];
                                               return &pThis->mOscSubmenuStuff.mSubmenuList;
                                           },
                                           AlwaysEnabled,
                                           this};

    SubmenuSettingItem mOsc3SubmenuItem = {String("OSC3"),
                                           [](void *cap) FLASHMEM {
                                               auto *pThis = (SynthPatchMenuApp *)cap;
                                               pThis->mOscSubmenuStuff.mpBinding = &pThis->GetBinding().mOsc[2];
                                               return &pThis->mOscSubmenuStuff.mSubmenuList;
                                           },
                                           AlwaysEnabled,
                                           this};

    ISettingItem *mBreathFilterSubmenuArray[5] = {
        &mBreathFiltType,
        &mBreathFiltMax,
        &mBreathFiltKS,
        &mBreathFiltQ,
        &mBreathFiltSaturation,
    };
    SettingsList mBreathFilterSubmenuList = {mBreathFilterSubmenuArray};

    SubmenuSettingItem mBreathFilterSubmenuItem = {String("Breath Filter"), &mBreathFilterSubmenuList, AlwaysEnabled};

    ISettingItem *mArray[19] = {
        &mVoicingMode,
        &mBreathFilterSubmenuItem,
        &mDetune,
        &mPan,
        &mStereoSpread,

        &mOsc1SubmenuItem,
        &mOsc2SubmenuItem,
        &mOsc3SubmenuItem,

        &mDelayMix,
        &mVerbMix,

        &mFMStrength,
        &mFMStrength1To2,
        &mFMStrength1To3,
        &mFMStrength2To1,
        &mFMStrength3To1,
        &mFMStrength2To3,
        &mFMStrength3To2,

        //&mSync,
        //&mSyncMultMin,
        //&mSyncMultMax,

        &mModulationsSubmenuItem,
        &mCopyPreset,
    };
    SettingsList mRootList = {mArray};

    // SettingsList *Start(size_t iPatch)
    // {
    //     this->DisplayAppInit(); // required to initialize stuff
    //     mBindingID = iPatch;
    //     return &mRootList;
    // }

    virtual SettingsList *GetRootSettingsList()
    {
        // log("synth patch app GetRootSettingsList");
        // this will only be called whne this app is standalone. that means it should always be working on the "current"
        // synth patch.
        return &mRootList;
    }

    virtual void RenderFrontPage()
    {
        mDisplay.ClearState();
        mDisplay.println(String("Synth Patch >"));
        mDisplay.println(GetAppSettings()->GetSynthPatchName(GetBindingID()));
        SettingsMenuApp::RenderFrontPage();
    }
};

} // namespace clarinoid
