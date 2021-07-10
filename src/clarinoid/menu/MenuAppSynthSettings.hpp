#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"
#include "NumericSettingItem.hpp"
#include "EnumSettingItem.hpp"
#include "FunctionListSettingItem.hpp"

namespace clarinoid
{

struct EnvelopeMenuApp
{
    FloatSettingItem mDelayMS = {"Delay",
                                 StandardRangeSpecs::gEnvDelayMS,
                                 Property<float>{[](void *cap) {
                                                     auto *pThis = (EnvelopeMenuApp *)cap;
                                                     return pThis->mBinding->mDelayMS;
                                                 },
                                                 [](void *cap, const float &v) {
                                                     auto *pThis = (EnvelopeMenuApp *)cap;
                                                     pThis->mBinding->mDelayMS = v;
                                                 },
                                                 this},
                                 AlwaysEnabled};

    FloatSettingItem mAttackMS = {"Attack",
                                  StandardRangeSpecs::gEnvAttackMS,
                                  Property<float>{[](void *cap) {
                                                      auto *pThis = (EnvelopeMenuApp *)cap;
                                                      return pThis->mBinding->mAttackMS;
                                                  },
                                                  [](void *cap, const float &v) {
                                                      auto *pThis = (EnvelopeMenuApp *)cap;
                                                      pThis->mBinding->mAttackMS = v;
                                                  },
                                                  this},
                                  AlwaysEnabled};

    FloatSettingItem mDecayMS = {"Decay",
                                 StandardRangeSpecs::gEnvDecayMS,
                                 Property<float>{[](void *cap) {
                                                     auto *pThis = (EnvelopeMenuApp *)cap;
                                                     return pThis->mBinding->mDecayMS;
                                                 },
                                                 [](void *cap, const float &v) {
                                                     auto *pThis = (EnvelopeMenuApp *)cap;
                                                     pThis->mBinding->mDecayMS = v;
                                                 },
                                                 this},
                                 AlwaysEnabled};

    FloatSettingItem mSustainLevel = {"Sustain",
                                      StandardRangeSpecs::gEnvSustainLevel,
                                      Property<float>{[](void *cap) {
                                                          auto *pThis = (EnvelopeMenuApp *)cap;
                                                          return pThis->mBinding->mSustainLevel;
                                                      },
                                                      [](void *cap, const float &v) {
                                                          auto *pThis = (EnvelopeMenuApp *)cap;
                                                          pThis->mBinding->mSustainLevel = v;
                                                      },
                                                      this},
                                      AlwaysEnabled};

    FloatSettingItem mReleaseMS = {"Release",
                                   StandardRangeSpecs::gEnvReleaseMS,
                                   Property<float>{[](void *cap) {
                                                       auto *pThis = (EnvelopeMenuApp *)cap;
                                                       return pThis->mBinding->mReleaseMS;
                                                   },
                                                   [](void *cap, const float &v) {
                                                       auto *pThis = (EnvelopeMenuApp *)cap;
                                                       pThis->mBinding->mReleaseMS = v;
                                                   },
                                                   this},
                                   AlwaysEnabled};

    ISettingItem *mArray[5] = {
        &mDelayMS,
        &mAttackMS,
        &mDecayMS,
        &mSustainLevel,
        &mReleaseMS,
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
    // EnumSettingItem(const String& name, const EnumInfo<T>& enumInfo, const Property<T>& binding,
    // cc::function<bool()>::ptr_t isEnabled) :
    EnumSettingItem<OscWaveformShape> mWaveform = {
        "Osc1-Waveform",
        gOscWaveformShapeInfo,
        Property<OscWaveformShape>{[](void *cap) {
                                       auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                       return pThis->GetBinding().mWaveform;
                                   },
                                   [](void *cap, const OscWaveformShape &v) {
                                       auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                       pThis->GetBinding().mWaveform = v;
                                   },
                                   this},
        AlwaysEnabled};

    FloatSettingItem mGain = {" - Gain",
                              StandardRangeSpecs::gFloat_0_1,
                              Property<float>{[](void *cap) {
                                                  auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                  return pThis->GetBinding().mGain;
                                              },
                                              [](void *cap, const float &v) {
                                                  auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                  pThis->GetBinding().mGain = v;
                                              },
                                              this},
                              AlwaysEnabled};

    FloatSettingItem mPortamentoTime = {" - Portamento",
                                        StandardRangeSpecs::gPortamentoRange,
                                        Property<float>{[](void *cap) {
                                                            auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                            return pThis->GetBinding().mPortamentoTime;
                                                        },
                                                        [](void *cap, const float &v) {
                                                            auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                            pThis->GetBinding().mPortamentoTime = v;
                                                        },
                                                        this},
                                        AlwaysEnabled};

    FloatSettingItem mFreqMul = {" - FreqMul",
                                 StandardRangeSpecs::gFreqMulRange,
                                 Property<float>{[](void *cap) {
                                                     auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                     return pThis->GetBinding().mFreqMultiplier;
                                                 },
                                                 [](void *cap, const float &v) {
                                                     auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                     pThis->GetBinding().mFreqMultiplier = v;
                                                 },
                                                 this},
                                 AlwaysEnabled};

    FloatSettingItem mFreqOffset = {" - FreqOffset",
                                    StandardRangeSpecs::gFreqOffsetRange,
                                    Property<float>{[](void *cap) {
                                                        auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                        return pThis->GetBinding().mFreqOffset;
                                                    },
                                                    [](void *cap, const float &v) {
                                                        auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                        pThis->GetBinding().mFreqOffset = v;
                                                    },
                                                    this},
                                    AlwaysEnabled};

    IntSettingItem mPitchSemis = {" - PitchSemis",
                                  StandardRangeSpecs::gTransposeRange,
                                  Property<int>{[](void *cap) {
                                                    auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                    return pThis->GetBinding().mPitchSemis;
                                                },
                                                [](void *cap, const int &v) {
                                                    auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                    pThis->GetBinding().mPitchSemis = v;
                                                },
                                                this},
                                  AlwaysEnabled};

    FloatSettingItem mPitchFine = {" - PitchFine",
                                   StandardRangeSpecs::gFloat_N1_1,
                                   Property<float>{[](void *cap) {
                                                       auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                       return pThis->GetBinding().mPitchFine;
                                                   },
                                                   [](void *cap, const float &v) {
                                                       auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                       pThis->GetBinding().mPitchFine = v;
                                                   },
                                                   this},
                                   AlwaysEnabled};
    FloatSettingItem mPulseWidth = {" - PulseWidth",
                                    StandardRangeSpecs::gFloat_0_1,
                                    Property<float>{[](void *cap) {
                                                        auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                        return pThis->GetBinding().mPulseWidth;
                                                    },
                                                    [](void *cap, const float &v) {
                                                        auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                        pThis->GetBinding().mPulseWidth = v;
                                                    },
                                                    this},
                                    AlwaysEnabled};

    ISettingItem *mArray[8] = {
        &mWaveform,
        &mGain,
        &mPortamentoTime,
        &mFreqMul,
        &mFreqOffset,
        &mPitchSemis,
        &mPitchFine,
        &mPulseWidth,
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

    SynthPatchMenuApp(CCDisplay &d) : SettingsMenuApp(d) //, mDisplay(d)
    {
    }

    // AppSettings &GetAppSettings() { return *mDisplay.mAppSettings; }
    SynthPreset &GetBinding()
    {
        return GetAppSettings()->mSynthSettings.mPresets[GetAppSettings()->mGlobalSynthPreset];
    }

    // LabelSettingItem(cc::function<String()>::ptr_t text, cc::function<bool()>::ptr_t isEnabled) :
    LabelSettingItem mBigSeparator = {Property<String>{[](void *) { return String("----"); }}, AlwaysEnabled};

    // NumericSettingItem(const String& name, T min_, T max_, const Property<T>& binding, typename
    // cc::function<bool()>::ptr_t isEnabled) : BoolSettingItem(const String& name, const String& trueCaption, const
    // String& falseCaption, const Property<bool>& binding, typename cc::function<bool()>::ptr_t isEnabled) :
    FloatSettingItem mDetune = {"Detune",
                                StandardRangeSpecs::gFloat_0_2,
                                Property<float>{[](void *cap) {
                                                    auto *pThis = (SynthPatchMenuApp *)cap;
                                                    return pThis->GetBinding().mDetune;
                                                },
                                                [](void *cap, const float &v) {
                                                    auto *pThis = (SynthPatchMenuApp *)cap;
                                                    pThis->GetBinding().mDetune = v;
                                                },
                                                this},
                                AlwaysEnabled};

    FloatSettingItem mPan = {"Pan",
                             StandardRangeSpecs::gFloat_N1_1,
                             Property<float>{[](void *cap) {
                                                 auto *pThis = (SynthPatchMenuApp *)cap;
                                                 return pThis->GetBinding().mPan;
                                             },
                                             [](void *cap, const float &v) {
                                                 auto *pThis = (SynthPatchMenuApp *)cap;
                                                 pThis->GetBinding().mPan = v;
                                             },
                                             this},
                             AlwaysEnabled};

    FloatSettingItem mVerbSend = {"Verb Send",
                                  StandardRangeSpecs::gFloat_0_1,
                                  Property<float>{[](void *cap) {
                                                      auto *pThis = (SynthPatchMenuApp *)cap;
                                                      return pThis->GetBinding().mVerbSend;
                                                  },
                                                  [](void *cap, const float &v) {
                                                      auto *pThis = (SynthPatchMenuApp *)cap;
                                                      pThis->GetBinding().mVerbSend = v;
                                                  },
                                                  this},
                                  AlwaysEnabled};

    FloatSettingItem mDelaySend = {"Dly Send",
                                   StandardRangeSpecs::gFloat_0_1,
                                   Property<float>{[](void *cap) {
                                                       auto *pThis = (SynthPatchMenuApp *)cap;
                                                       return pThis->GetBinding().mDelaySend;
                                                   },
                                                   [](void *cap, const float &v) {
                                                       auto *pThis = (SynthPatchMenuApp *)cap;
                                                       pThis->GetBinding().mDelaySend = v;
                                                   },
                                                   this},
                                   AlwaysEnabled};

    BoolSettingItem mSync = {"Sync",
                             "On",
                             "Off",
                             Property<bool>{[](void *cap) {
                                                auto *pThis = (SynthPatchMenuApp *)cap;
                                                return pThis->GetBinding().mSync;
                                            },
                                            [](void *cap, const bool &v) {
                                                auto *pThis = (SynthPatchMenuApp *)cap;
                                                pThis->GetBinding().mSync = v;
                                            },
                                            this},
                             AlwaysEnabled};

    FloatSettingItem mSyncMultMin = {" - mult min",
                                     NumericEditRangeSpec<float>{0.0f, 15.0f},
                                     Property<float>{[](void *cap) {
                                                         auto *pThis = (SynthPatchMenuApp *)cap;
                                                         return pThis->GetBinding().mSyncMultMin;
                                                     },
                                                     [](void *cap, const float &v) {
                                                         auto *pThis = (SynthPatchMenuApp *)cap;
                                                         pThis->GetBinding().mSyncMultMin = v;
                                                     },
                                                     this},
                                     Property<bool>{[](void *cap) {
                                                        auto *pThis = (SynthPatchMenuApp *)cap;
                                                        return pThis->GetBinding().mSync;
                                                    },
                                                    this}};

    FloatSettingItem mSyncMultMax = {" - mult max",
                                     NumericEditRangeSpec<float>{0.0f, 15.0f},
                                     Property<float>{[](void *cap) {
                                                         auto *pThis = (SynthPatchMenuApp *)cap;
                                                         return pThis->GetBinding().mSyncMultMax;
                                                     },
                                                     [](void *cap, const float &v) {
                                                         auto *pThis = (SynthPatchMenuApp *)cap;
                                                         pThis->GetBinding().mSyncMultMax = v;
                                                     },
                                                     this},
                                     Property<bool>{[](void *cap) {
                                                        auto *pThis = (SynthPatchMenuApp *)cap;
                                                        return pThis->GetBinding().mSync;
                                                    },
                                                    this}};

    // EnumSettingItem(const String& name, const EnumInfo<T>& enumInfo, const Property<T>& binding,
    // cc::function<bool()>::ptr_t isEnabled) :
    EnumSettingItem<ClarinoidFilterType> mBreathFiltType = {
        "Filter",
        gClarinoidFilterTypeInfo,
        Property<ClarinoidFilterType>{[](void *cap) {
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
                                     Property<float>{[](void *cap) {
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
                                              Property<float>{[](void *cap) {
                                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                                  return pThis->GetBinding().mFilterSaturation;
                                                              },
                                                              [](void *cap, const float &v) {
                                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                                  pThis->GetBinding().mFilterSaturation = v;
                                                              },
                                                              this},
                                              AlwaysEnabled};

    FloatSettingItem mBreathFiltMin = {" - filt min",
                                       NumericEditRangeSpec<float>{0.0f, 3000.0f},
                                       Property<float>{[](void *cap) {
                                                           auto *pThis = (SynthPatchMenuApp *)cap;
                                                           return pThis->GetBinding().mFilterMinFreq;
                                                       },
                                                       [](void *cap, const float &v) {
                                                           auto *pThis = (SynthPatchMenuApp *)cap;
                                                           pThis->GetBinding().mFilterMinFreq = v;
                                                       },
                                                       this},
                                       AlwaysEnabled};

    FloatSettingItem mBreathFiltMax = {" - filt max",
                                       NumericEditRangeSpec<float>{0.0f, 25000.0f},
                                       Property<float>{[](void *cap) {
                                                           auto *pThis = (SynthPatchMenuApp *)cap;
                                                           return pThis->GetBinding().mFilterMaxFreq;
                                                       },
                                                       [](void *cap, const float &v) {
                                                           auto *pThis = (SynthPatchMenuApp *)cap;
                                                           pThis->GetBinding().mFilterMaxFreq = v;
                                                       },
                                                       this},
                                       AlwaysEnabled};

    FloatSettingItem mBreathFiltKS = {" - filt KS",
                                      NumericEditRangeSpec<float>{0.0f, 2.0f, .2f, .1f, 0.01f},
                                      Property<float>{[](void *cap) {
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
                                  Property<bool>{[](void *cap) {
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
                                  Property<float>{[](void *cap) {
                                                      auto *pThis = (SynthPatchMenuApp *)cap;
                                                      return pThis->GetBinding().mDCFilterCutoff;
                                                  },
                                                  [](void *cap, const float &v) {
                                                      auto *pThis = (SynthPatchMenuApp *)cap;
                                                      pThis->GetBinding().mDCFilterCutoff = v;
                                                  },
                                                  this},
                                  Property<bool>{[](void *cap) {
                                                     auto *pThis = (SynthPatchMenuApp *)cap;
                                                     return pThis->GetBinding().mDCFilterEnabled;
                                                 },
                                                 this}};

    FunctionListSettingItem mCopyPreset = {
        "Copy to ...",
        SYNTH_PRESET_COUNT,
        [](void *cap, size_t i) { // itemNameGetter,
            auto *pThis = (SynthPatchMenuApp *)cap;
            return pThis->mDisplay.mAppSettings->mSynthSettings.mPresets[i].ToString(i);
        },
        [](void *cap, size_t i) { // cc::function<void(void*,size_t)>::ptr_t onClick,
            auto *pThis = (SynthPatchMenuApp *)cap;
            pThis->mDisplay.mAppSettings->mSynthSettings.mPresets[i] = pThis->GetBinding();
            pThis->mDisplay.ShowToast(
                String("Copied ") +
                pThis->GetAppSettings()->mSynthSettings.mPresets[pThis->GetAppSettings()->mGlobalSynthPreset].ToString(
                    pThis->GetAppSettings()->mGlobalSynthPreset) +
                "\nto\n" + pThis->GetAppSettings()->mSynthSettings.mPresets[i].ToString(i));
        },
        AlwaysEnabled,
        this};

    FloatSettingItem mLfo1Frequency = {"LFO1 Freq",
                                       StandardRangeSpecs::gLFOFrequency,
                                       Property<float>{[](void *cap) {
                                                           auto *pThis = (SynthPatchMenuApp *)cap;
                                                           return pThis->GetBinding().mLfo1Rate;
                                                       },
                                                       [](void *cap, const float &v) {
                                                           auto *pThis = (SynthPatchMenuApp *)cap;
                                                           pThis->GetBinding().mLfo1Rate = v;
                                                       },
                                                       this},
                                       AlwaysEnabled};

    EnumSettingItem<OscWaveformShape> mLfo1Waveform = {
        " >Wave",
        gOscWaveformShapeInfo,
        Property<OscWaveformShape>{[](void *cap) {
                                       auto *pThis = (SynthPatchMenuApp *)cap;
                                       return pThis->GetBinding().mLfo1Shape;
                                   },
                                   [](void *cap, const OscWaveformShape &v) {
                                       auto *pThis = (SynthPatchMenuApp *)cap;
                                       pThis->GetBinding().mLfo1Shape = v;
                                   },
                                   this},
        AlwaysEnabled};

    FloatSettingItem mLfo2Frequency = {"LFO2 Freq",
                                       StandardRangeSpecs::gLFOFrequency,
                                       Property<float>{[](void *cap) {
                                                           auto *pThis = (SynthPatchMenuApp *)cap;
                                                           return pThis->GetBinding().mLfo2Rate;
                                                       },
                                                       [](void *cap, const float &v) {
                                                           auto *pThis = (SynthPatchMenuApp *)cap;
                                                           pThis->GetBinding().mLfo2Rate = v;
                                                       },
                                                       this},
                                       AlwaysEnabled};

    EnumSettingItem<OscWaveformShape> mLfo2Waveform = {
        " >Wave",
        gOscWaveformShapeInfo,
        Property<OscWaveformShape>{[](void *cap) {
                                       auto *pThis = (SynthPatchMenuApp *)cap;
                                       return pThis->GetBinding().mLfo2Shape;
                                   },
                                   [](void *cap, const OscWaveformShape &v) {
                                       auto *pThis = (SynthPatchMenuApp *)cap;
                                       pThis->GetBinding().mLfo2Shape = v;
                                   },
                                   this},
        AlwaysEnabled};

    size_t mEditingModulationIndex = 0;
    SynthModulationSpec &GetModulationBinding()
    {
        return GetBinding().mModulations[mEditingModulationIndex];
    }

    EnumSettingItem<ModulationSource> mModSource = {
        "Source",
        gModulationSourceInfo,
        Property<ModulationSource>{[](void *cap) {
                                       auto *pThis = (SynthPatchMenuApp *)cap;
                                       return pThis->GetModulationBinding().mSource;
                                   },
                                   [](void *cap, const ModulationSource &v) {
                                       auto *pThis = (SynthPatchMenuApp *)cap;
                                       pThis->GetModulationBinding().mSource = v;
                                   },
                                   this},
        AlwaysEnabled};

    EnumSettingItem<ModulationDestination> mModDest = {
        "Dest",
        gModulationDestinationInfo,
        Property<ModulationDestination>{[](void *cap) {
                                            auto *pThis = (SynthPatchMenuApp *)cap;
                                            return pThis->GetModulationBinding().mDest;
                                        },
                                        [](void *cap, const ModulationDestination &v) {
                                            auto *pThis = (SynthPatchMenuApp *)cap;
                                            pThis->GetModulationBinding().mDest = v;
                                        },
                                        this},
        AlwaysEnabled};

    FloatSettingItem mModScale = {"Scale",
                                  StandardRangeSpecs::gFloat_N1_1,
                                  Property<float>{[](void *cap) {
                                                      auto *pThis = (SynthPatchMenuApp *)cap;
                                                      return pThis->GetModulationBinding().mScaleN11;
                                                  },
                                                  [](void *cap, const float &v) {
                                                      auto *pThis = (SynthPatchMenuApp *)cap;
                                                      pThis->GetModulationBinding().mScaleN11 = v;
                                                  },
                                                  this},
                                  AlwaysEnabled};

    ISettingItem *mModulationSubmenu[3] = {
        &mModSource,
        &mModDest,
        &mModScale,
    };
    SettingsList mModulationSubmenuList = {mModulationSubmenu};

    MultiSubmenuSettingItem mModulationsList = {[](void *cap) { return SYNTH_MODULATIONS_MAX; },
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
                                              [](void *cap) {
                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                  return pThis->mEnvEditor.GetSubmenuList(pThis->GetBinding().mEnv1);
                                              },
                                              AlwaysEnabled,
                                              this};

    SubmenuSettingItem mModEnv2SubmenuItem = {String("ENV2"),
                                              [](void *cap) {
                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                  return pThis->mEnvEditor.GetSubmenuList(pThis->GetBinding().mEnv2);
                                              },
                                              AlwaysEnabled,
                                              this};

    ISettingItem *mModulationsSubmenuArray[7] = {
        &mLfo1Frequency,
        &mLfo1Waveform,
        &mLfo2Frequency,
        &mLfo2Waveform,
        &mModEnv1SubmenuItem,
        &mModEnv2SubmenuItem,
        &mModulationsList,
    };
    SettingsList mModulationsSubmenuList = {mModulationsSubmenuArray};

    SubmenuSettingItem mModulationsSubmenuItem = {String("Modulations"), &mModulationsSubmenuList, AlwaysEnabled};

    SynthPatchOscillatorMenuStuff mOscSubmenuStuff;

    SubmenuSettingItem mOsc1SubmenuItem = {String("OSC1"),
                                           [](void *cap) {
                                               auto *pThis = (SynthPatchMenuApp *)cap;
                                               pThis->mOscSubmenuStuff.mpBinding = &pThis->GetBinding().mOsc[0];
                                               return &pThis->mOscSubmenuStuff.mSubmenuList;
                                           },
                                           AlwaysEnabled,
                                           this};

    SubmenuSettingItem mOsc2SubmenuItem = {String("OSC2"),
                                           [](void *cap) {
                                               auto *pThis = (SynthPatchMenuApp *)cap;
                                               pThis->mOscSubmenuStuff.mpBinding = &pThis->GetBinding().mOsc[1];
                                               return &pThis->mOscSubmenuStuff.mSubmenuList;
                                           },
                                           AlwaysEnabled,
                                           this};

    SubmenuSettingItem mOsc3SubmenuItem = {String("OSC3"),
                                           [](void *cap) {
                                               auto *pThis = (SynthPatchMenuApp *)cap;
                                               pThis->mOscSubmenuStuff.mpBinding = &pThis->GetBinding().mOsc[2];
                                               return &pThis->mOscSubmenuStuff.mSubmenuList;
                                           },
                                           AlwaysEnabled,
                                           this};

    ISettingItem *mBreathFilterSubmenuArray[6] = {
        &mBreathFiltType,
        &mBreathFiltMin,
        &mBreathFiltMax,
        &mBreathFiltKS,
        &mBreathFiltQ,
        &mBreathFiltSaturation,
    };
    SettingsList mBreathFilterSubmenuList = {mBreathFilterSubmenuArray};

    SubmenuSettingItem mBreathFilterSubmenuItem = {String("Breath Filter"), &mBreathFilterSubmenuList, AlwaysEnabled};

    ISettingItem *mArray[14] = {
        &mBreathFilterSubmenuItem,
        &mDetune,
        //&mPortamentoTime,
        &mPan,
        &mDelaySend,
        &mVerbSend,
        &mSync,
        &mSyncMultMin,
        &mSyncMultMax,

        &mOsc1SubmenuItem,
        &mOsc2SubmenuItem,
        &mOsc3SubmenuItem,

        &mModulationsSubmenuItem,
        &mCopyPreset,
        &mBigSeparator,
    };
    SettingsList mRootList = {mArray};

    SettingsList *Start(size_t iPatch)
    {
        this->DisplayAppInit(); // required to initialize stuff
        // log(String("Starting synth patch app"));
        // log(String("  appsettings: ") + (uint32_t)GetAppSettings());
        // log(String("  from ") + GetAppSettings()->mGlobalSynthPreset + " to " + iPatch);
        GetAppSettings()->mGlobalSynthPreset = iPatch;
        // mAppSettings->mGlobalSynthPreset = nv;
        // mEditingSynthPatch = iPatch;
        return &mRootList;
    }

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
        mDisplay.mDisplay.println(String("PATCH >"));
        mDisplay.mDisplay.println(
            GetAppSettings()->mSynthSettings.mPresets[GetAppSettings()->mGlobalSynthPreset].ToString(
                GetAppSettings()->mGlobalSynthPreset));
        SettingsMenuApp::RenderFrontPage();
    }
};

struct SynthSettingsApp : public SettingsMenuApp
{
    virtual const char *DisplayAppGetName() override
    {
        return "SynthSettingsApp";
    }

    SynthPatchMenuApp mSynthPatchSettingsApp;
    SynthSettings &GetSynthSettings()
    {
        return this->mAppSettings->mSynthSettings;
    }

    SynthSettingsApp(CCDisplay &d) : SettingsMenuApp(d), mSynthPatchSettingsApp(d)
    {
    }

    FloatSettingItem mMasterGain = {"Master gain",
                                    StandardRangeSpecs::gFloat_0_2,
                                    Property<float>{[](void *cap) {
                                                        auto *pThis = (SynthSettingsApp *)cap;
                                                        return pThis->GetSynthSettings().mMasterGain;
                                                    },
                                                    [](void *cap, const float &v) {
                                                        auto *pThis = (SynthSettingsApp *)cap;
                                                        pThis->GetSynthSettings().mMasterGain = v;
                                                    },
                                                    this},
                                    AlwaysEnabled};

    IntSettingItem mGlobalSynthPreset = {"Global synth P#",
                                         NumericEditRangeSpec<int>{0, SYNTH_PRESET_COUNT - 1},
                                         Property<int>{[](void *cap) {
                                                           auto *pThis = (SynthSettingsApp *)cap;
                                                           return (int)pThis->GetAppSettings()->mGlobalSynthPreset;
                                                       },
                                                       [](void *cap, const int &v) {
                                                           auto *pThis = (SynthSettingsApp *)cap;
                                                           pThis->GetAppSettings()->mGlobalSynthPreset = (uint16_t)v;
                                                       },
                                                       this},
                                         AlwaysEnabled};

    IntSettingItem mTranspose = {"Transpose",
                                 StandardRangeSpecs::gTransposeRange,
                                 Property<int>{[](void *cap) {
                                                   auto *pThis = (SynthSettingsApp *)cap;
                                                   return pThis->GetAppSettings()->mTranspose;
                                               },
                                               [](void *cap, const int &v) {
                                                   auto *pThis = (SynthSettingsApp *)cap;
                                                   pThis->GetAppSettings()->mTranspose = v;
                                               },
                                               this},
                                 AlwaysEnabled};

    // there's a bit of a conflict here because it's defined as float but this is int.
    IntSettingItem mPitchbendRange = {
        "Pitchbend range",
        NumericEditRangeSpec<int>{0, 12},
        Property<int>{[](void *cap) {
                          auto *pThis = (SynthSettingsApp *)cap;
                          return (int)pThis->GetAppSettings()->mSynthSettings.mPitchBendRange;
                      },
                      [](void *cap, const int &v) {
                          auto *pThis = (SynthSettingsApp *)cap;
                          pThis->GetAppSettings()->mSynthSettings.mPitchBendRange = (float)v;
                      },
                      this},
        AlwaysEnabled};

    MultiSubmenuSettingItem mPatches = {
        [](void *cap) { return SYNTH_PRESET_COUNT; },
        [](void *cap, size_t n) {
            auto *pThis = (SynthSettingsApp *)cap;
            if (pThis->GetSynthSettings().mPresets[n].mName.length() == 0)
            {
                return String(String("") + n + ": <init>");
            }
            return String(String("") + n + ":" + pThis->GetSynthSettings().mPresets[n].mName);
        }, // name
        [](void *cap, size_t n) {
            auto *pThis = (SynthSettingsApp *)cap;
            return pThis->mSynthPatchSettingsApp.Start(n);
        }, // get submenu list
        [](void *cap, size_t n) { return true; },
        (void *)this // capture
    };

    FloatSettingItem mReverbGain = {"Reverb gain",
                                    StandardRangeSpecs::gFloat_0_1,
                                    Property<float>{[](void *cap) {
                                                        auto *pThis = (SynthSettingsApp *)cap;
                                                        return pThis->GetSynthSettings().mReverbGain;
                                                    },
                                                    [](void *cap, const float &v) {
                                                        auto *pThis = (SynthSettingsApp *)cap;
                                                        pThis->GetSynthSettings().mReverbGain = v;
                                                    },
                                                    this},
                                    AlwaysEnabled};

    FloatSettingItem mReverbDamping = {"Reverb damp",
                                       StandardRangeSpecs::gFloat_0_1,
                                       Property<float>{[](void *cap) {
                                                           auto *pThis = (SynthSettingsApp *)cap;
                                                           return pThis->GetSynthSettings().mReverbDamping;
                                                       },
                                                       [](void *cap, const float &v) {
                                                           auto *pThis = (SynthSettingsApp *)cap;
                                                           pThis->GetSynthSettings().mReverbDamping = v;
                                                       },
                                                       this},
                                       AlwaysEnabled};

    FloatSettingItem mReverbSize = {"Reverb size",
                                    StandardRangeSpecs::gFloat_0_1,
                                    Property<float>{[](void *cap) {
                                                        auto *pThis = (SynthSettingsApp *)cap;
                                                        return pThis->GetSynthSettings().mReverbSize;
                                                    },
                                                    [](void *cap, const float &v) {
                                                        auto *pThis = (SynthSettingsApp *)cap;
                                                        pThis->GetSynthSettings().mReverbSize = v;
                                                    },
                                                    this},
                                    AlwaysEnabled};

    LabelSettingItem mSeparator = {Property<String>{[](void *) { return String("----"); }}, AlwaysEnabled};

    FloatSettingItem mDelayGain = {"Delay gain",
                                   StandardRangeSpecs::gFloat_0_1,
                                   Property<float>{[](void *cap) {
                                                       auto *pThis = (SynthSettingsApp *)cap;
                                                       return pThis->GetSynthSettings().mDelayGain;
                                                   },
                                                   [](void *cap, const float &v) {
                                                       auto *pThis = (SynthSettingsApp *)cap;
                                                       pThis->GetSynthSettings().mDelayGain = v;
                                                   },
                                                   this},
                                   AlwaysEnabled};

    FloatSettingItem mDelayTimeMS = {"Delay Time",
                                     NumericEditRangeSpec<float>(1, MAX_DELAY_MS),
                                     Property<float>{[](void *cap) {
                                                         auto *pThis = (SynthSettingsApp *)cap;
                                                         return pThis->GetSynthSettings().mDelayMS;
                                                     },
                                                     [](void *cap, const float &v) {
                                                         auto *pThis = (SynthSettingsApp *)cap;
                                                         pThis->GetSynthSettings().mDelayMS = v;
                                                     },
                                                     this},
                                     AlwaysEnabled};

    FloatSettingItem mDelayStereoSep = {" >Width",
                                        NumericEditRangeSpec<float>(1, 100),
                                        Property<float>{[](void *cap) {
                                                            auto *pThis = (SynthSettingsApp *)cap;
                                                            return pThis->GetSynthSettings().mDelayStereoSep;
                                                        },
                                                        [](void *cap, const float &v) {
                                                            auto *pThis = (SynthSettingsApp *)cap;
                                                            pThis->GetSynthSettings().mDelayStereoSep = v;
                                                        },
                                                        this},
                                        AlwaysEnabled};

    FloatSettingItem mDelayFeedbackLevel = {" >FB",
                                            StandardRangeSpecs::gFloat_0_1,
                                            Property<float>{[](void *cap) {
                                                                auto *pThis = (SynthSettingsApp *)cap;
                                                                return pThis->GetSynthSettings().mDelayFeedbackLevel;
                                                            },
                                                            [](void *cap, const float &v) {
                                                                auto *pThis = (SynthSettingsApp *)cap;
                                                                pThis->GetSynthSettings().mDelayFeedbackLevel = v;
                                                            },
                                                            this},
                                            AlwaysEnabled};

    EnumSettingItem<ClarinoidFilterType> mDelayFilterType = {
        " >Filter",
        gClarinoidFilterTypeInfo,
        Property<ClarinoidFilterType>{[](void *cap) {
                                          auto *pThis = (SynthSettingsApp *)cap;
                                          return pThis->GetSynthSettings().mDelayFilterType;
                                      },
                                      [](void *cap, const ClarinoidFilterType &v) {
                                          auto *pThis = (SynthSettingsApp *)cap;
                                          pThis->GetSynthSettings().mDelayFilterType = v;
                                      },
                                      this},
        AlwaysEnabled};

    FloatSettingItem mDelayCutoffFrequency = {
        " > >Freq",
        NumericEditRangeSpec<float>(0, 22050),
        Property<float>{[](void *cap) {
                            auto *pThis = (SynthSettingsApp *)cap;
                            return pThis->GetSynthSettings().mDelayCutoffFrequency;
                        },
                        [](void *cap, const float &v) {
                            auto *pThis = (SynthSettingsApp *)cap;
                            pThis->GetSynthSettings().mDelayCutoffFrequency = v;
                        },
                        this},
        AlwaysEnabled};

    FloatSettingItem mDelaySaturation = {" > >Sat",
                                         StandardRangeSpecs::gFloat_0_1,
                                         Property<float>{[](void *cap) {
                                                             auto *pThis = (SynthSettingsApp *)cap;
                                                             return pThis->GetSynthSettings().mDelaySaturation;
                                                         },
                                                         [](void *cap, const float &v) {
                                                             auto *pThis = (SynthSettingsApp *)cap;
                                                             pThis->GetSynthSettings().mDelaySaturation = v;
                                                         },
                                                         this},
                                         AlwaysEnabled};

    FloatSettingItem mDelayQ = {" > >Q",
                                StandardRangeSpecs::gFloat_0_1,
                                Property<float>{[](void *cap) {
                                                    auto *pThis = (SynthSettingsApp *)cap;
                                                    return pThis->GetSynthSettings().mDelayQ;
                                                },
                                                [](void *cap, const float &v) {
                                                    auto *pThis = (SynthSettingsApp *)cap;
                                                    pThis->GetSynthSettings().mDelayQ = v;
                                                },
                                                this},
                                AlwaysEnabled};

    // ClarinoidFilterType mDelayFilterType = ClarinoidFilterType::BP_Moog4;

    BoolSettingItem mMasterFXEnable = {"MasterFX Enable",
                                       "Yes",
                                       "No",
                                       Property<bool>{[](void *cap) {
                                                          auto *pThis = (SynthSettingsApp *)cap;
                                                          return pThis->GetSynthSettings().mMasterFXEnable;
                                                      },
                                                      [](void *cap, const bool &v) {
                                                          auto *pThis = (SynthSettingsApp *)cap;
                                                          pThis->GetSynthSettings().mMasterFXEnable = v;
                                                      },
                                                      this},
                                       AlwaysEnabled};

    ISettingItem *mMasterFXSubmenuItems[13] = {
        &mReverbGain,
        &mReverbDamping,
        &mReverbSize,
        &mSeparator,
        &mDelayGain,
        &mDelayTimeMS,
        &mDelayStereoSep,
        &mDelayFeedbackLevel,
        &mDelayFilterType,
        &mDelayCutoffFrequency,
        &mDelaySaturation,
        &mDelayQ,
        &mSeparator,
    };
    SettingsList mMasterFXList = {mMasterFXSubmenuItems};

    SubmenuSettingItem mMasterFX = {String("Master FX"), &mMasterFXList, AlwaysEnabled};

    ISettingItem *mArray[7] = {
        &mMasterGain,
        &mTranspose,
        &mPitchbendRange,
        //&mGlobalSynthPreset,
        &mMasterFXEnable,
        &mMasterFX,
        &mPatches,
        &mSeparator,
    };
    SettingsList mRootList = {mArray};

  public:
    virtual SettingsList *GetRootSettingsList()
    {
        return &mRootList;
    }

    virtual void RenderFrontPage()
    {
        // log(String("synth settings app settings = ") + (uint32_t)mAppSettings);
        mDisplay.ClearState();
        mDisplay.mDisplay.println(String("SYNTH > "));
        mDisplay.mDisplay.println(
            GetAppSettings()->mSynthSettings.mPresets[GetAppSettings()->mGlobalSynthPreset].ToString(
                GetAppSettings()->mGlobalSynthPreset));

        SettingsMenuApp::RenderFrontPage();
    }
};

} // namespace clarinoid
