#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"
#include "NumericSettingItem.hpp"
#include "EnumSettingItem.hpp"
#include "FunctionListSettingItem.hpp"
#include "GainSettingItem.hpp"

namespace clarinoid
{

struct LFOMenuApp
{
    EnumSettingItem<OscWaveformShape> mWaveform = {"Waveform",
                                                   gOscWaveformShapeInfo,
                                                   Property<OscWaveformShape>{[](void *cap) FLASHMEM {
                                                                                  auto *pThis = (LFOMenuApp *)cap;
                                                                                  return pThis->mBinding->mWaveShape;
                                                                              },
                                                                              [](void *cap, const OscWaveformShape &v)
                                                                                  FLASHMEM {
                                                                                      auto *pThis = (LFOMenuApp *)cap;
                                                                                      pThis->mBinding->mWaveShape = v;
                                                                                  },
                                                                              this},
                                                   AlwaysEnabled};

    FloatSettingItem mFrequency = {"Freq Hz",
                                   StandardRangeSpecs::gLFOFrequency,
                                   Property<float>{[](void *cap) FLASHMEM {
                                                       auto *pThis = (LFOMenuApp *)cap;
                                                       return pThis->mBinding->mTime.mHz;
                                                   },
                                                   [](void *cap, const float &v) FLASHMEM {
                                                       auto *pThis = (LFOMenuApp *)cap;
                                                       pThis->mBinding->mTime.SetFrequency(v);
                                                   },
                                                   this},
                                   AlwaysEnabled};

    BoolSettingItem mRetrig = {"Retrig",
                                     "On",
                                     "Off",
                                     Property<bool>{[](void *cap) FLASHMEM {
                                                        auto *pThis = (LFOMenuApp *)cap;
                                                        return pThis->mBinding->mPhaseRestart;
                                                    },
                                                    [](void *cap, const bool &v) FLASHMEM {
                                                        auto *pThis = (LFOMenuApp *)cap;
                                                        pThis->mBinding->mPhaseRestart = v;
                                                    },
                                                    this},
                                     AlwaysEnabled};

    FloatSettingItem mPulseWidth = {"Pulse Width",
                                StandardRangeSpecs::gFloat_N1_1,
                                Property<float>{[](void *cap) FLASHMEM {
                                                    auto *pThis = (LFOMenuApp *)cap;
                                                    return pThis->mBinding->mPulseWidth;
                                                },
                                                [](void *cap, const float &v) FLASHMEM {
                                                    auto *pThis = (LFOMenuApp *)cap;
                                                    pThis->mBinding->mPulseWidth = v;
                                                },
                                                this},
                                AlwaysEnabled};

    ISettingItem *mArray[4] = {
        &mWaveform,
        &mFrequency,
        &mRetrig,
        &mPulseWidth,
    };


    SettingsList mRootList = {mArray};

    LFOSpec *mBinding = nullptr;

    SettingsList *GetSubmenuList(LFOSpec &binding)
    {
        mBinding = &binding;
        return &mRootList;
    }
};

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
                                 StandardRangeSpecs::gFloat_0_1,
                                 Property<float>{[](void *cap) FLASHMEM {
                                                     auto *pThis = (EnvelopeMenuApp *)cap;
                                                     return pThis->mBinding->mDelayTime.GetValue();
                                                 },
                                                 [](void *cap, const float &v) FLASHMEM {
                                                     auto *pThis = (EnvelopeMenuApp *)cap;
                                                     pThis->mBinding->mDelayTime.SetValue(v);
                                                 },
                                                 this},
                                 AlwaysEnabled};

    FloatSettingItem mAttackMS = {"Attack",
                                  StandardRangeSpecs::gFloat_0_1,
                                  Property<float>{[](void *cap) FLASHMEM {
                                                      auto *pThis = (EnvelopeMenuApp *)cap;
                                                      return pThis->mBinding->mAttackTime.GetValue();
                                                  },
                                                  [](void *cap, const float &v) FLASHMEM {
                                                      auto *pThis = (EnvelopeMenuApp *)cap;
                                                      pThis->mBinding->mAttackTime.SetValue(v);
                                                  },
                                                  this},
                                  AlwaysEnabled};

    FloatSettingItem mAttackCurve = {" - Curve",
                                StandardRangeSpecs::gFloat_N1_1,
                                Property<float>{[](void *cap) FLASHMEM {
                                                      auto *pThis = (EnvelopeMenuApp *)cap;
                                                      return pThis->mBinding->mAttackCurve.GetParamValue();
                                              },
                                              [](void *cap, const float &v) FLASHMEM {
                                                      auto *pThis = (EnvelopeMenuApp *)cap;
                                                      pThis->mBinding->mAttackCurve.SetParamValue(v);
                                              },
                                              this},
                                AlwaysEnabled};

    FloatSettingItem mHoldMS = {"Hold",
                                StandardRangeSpecs::gFloat_0_1,
                                Property<float>{[](void *cap) FLASHMEM {
                                                    auto *pThis = (EnvelopeMenuApp *)cap;
                                                    return pThis->mBinding->mHoldTime.GetValue();
                                                },
                                                [](void *cap, const float &v) FLASHMEM {
                                                    auto *pThis = (EnvelopeMenuApp *)cap;
                                                    pThis->mBinding->mHoldTime.SetValue(v);
                                                },
                                                this},
                                AlwaysEnabled};

    FloatSettingItem mDecayMS = {"Decay",
                                 StandardRangeSpecs::gFloat_0_1,
                                 Property<float>{[](void *cap) FLASHMEM {
                                                     auto *pThis = (EnvelopeMenuApp *)cap;
                                                     return pThis->mBinding->mDecayTime.GetValue();
                                                 },
                                                 [](void *cap, const float &v) FLASHMEM {
                                                     auto *pThis = (EnvelopeMenuApp *)cap;
                                                     pThis->mBinding->mDecayTime.SetValue(v);
                                                 },
                                                 this},
                                 AlwaysEnabled};

    FloatSettingItem mDecayCurve = {" - Curve",
                                StandardRangeSpecs::gFloat_N1_1,
                                Property<float>{[](void *cap) FLASHMEM {
                                                      auto *pThis = (EnvelopeMenuApp *)cap;
                                                      return pThis->mBinding->mDecayCurve.GetParamValue();
                                              },
                                              [](void *cap, const float &v) FLASHMEM {
                                                      auto *pThis = (EnvelopeMenuApp *)cap;
                                                      pThis->mBinding->mDecayCurve.SetParamValue(v);
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
                                   StandardRangeSpecs::gFloat_0_1,
                                   Property<float>{[](void *cap) FLASHMEM {
                                                       auto *pThis = (EnvelopeMenuApp *)cap;
                                                       return pThis->mBinding->mReleaseTime.GetValue();
                                                   },
                                                   [](void *cap, const float &v) FLASHMEM {
                                                       auto *pThis = (EnvelopeMenuApp *)cap;
                                                       pThis->mBinding->mReleaseTime.SetValue(v);
                                                   },
                                                   this},
                                   AlwaysEnabled};

    FloatSettingItem mReleaseCurve = {" - Curve",
                                StandardRangeSpecs::gFloat_N1_1,
                                Property<float>{[](void *cap) FLASHMEM {
                                                      auto *pThis = (EnvelopeMenuApp *)cap;
                                                      return pThis->mBinding->mReleaseCurve.GetParamValue();
                                              },
                                              [](void *cap, const float &v) FLASHMEM {
                                                      auto *pThis = (EnvelopeMenuApp *)cap;
                                                      pThis->mBinding->mReleaseCurve.SetParamValue(v);
                                              },
                                              this},
                                AlwaysEnabled};

    ISettingItem *mArray[10] = {
        &mLegatoRetrig,
        &mDelayMS,
        &mAttackMS,
        &mAttackCurve,
        &mHoldMS,
        &mDecayMS,
        &mDecayCurve,
        &mSustainLevel,
        &mReleaseMS,
        &mReleaseCurve,
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
  private:
    SynthOscillatorSettings *mpBinding = nullptr;
    SynthPreset *mpPresetBinding = nullptr;
    int mOscillatorIndex = -1;
    IDisplay *mpDisplay;

  public:
    void SetBindings(SynthPreset *patch, int oscIndex, IDisplay *pdisplay)
    {
        mOscillatorIndex = oscIndex;
        mpPresetBinding = patch;
        mpBinding = &patch->mOsc[oscIndex];
        mpDisplay = pdisplay;
    }
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
                                                       return pThis->GetBinding().mFreqParam.GetParamValue();
                                                   },
                                                   [](void *cap, const float &v) FLASHMEM {
                                                       auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                       pThis->GetBinding().mFreqParam.SetParamValue(v);
                                                   },
                                                   this},
                                   AlwaysEnabled};

    FloatSettingItem mFreqParamKT = {"FreqParamKT",
                                     StandardRangeSpecs::gFloat_0_1,
                                     Property<float>{[](void *cap) FLASHMEM {
                                                         auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                         return pThis->GetBinding().mFreqParam.GetKTParamValue();
                                                     },
                                                     [](void *cap, const float &v) FLASHMEM {
                                                         auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                         pThis->GetBinding().mFreqParam.SetKTParamValue(v);
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


    FloatSettingItem mSyncFreqParam = {"Sync Freq",
                                   StandardRangeSpecs::gFloat_0_1,
                                   Property<float>{[](void *cap) FLASHMEM {
                                                       auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                       return pThis->GetBinding().mSyncFreqParam.GetParamValue();
                                                   },
                                                   [](void *cap, const float &v) FLASHMEM {
                                                       auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                       pThis->GetBinding().mSyncFreqParam.SetParamValue(v);
                                                   },
                                                   this},
                                       Property<bool>{[](void *cap) FLASHMEM -> bool { // enabled only for sync
                                                         auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                         return IsSyncWaveform(pThis->GetBinding().mWaveform);
                                                      },
                                                      this}};

    FloatSettingItem mSyncFreqParamKT = {" - KT",
                                     StandardRangeSpecs::gFloat_N1_1,
                                     Property<float>{[](void *cap) FLASHMEM {
                                                         auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                         return pThis->GetBinding().mSyncFreqParam.GetKTParamValue();
                                                     },
                                                     [](void *cap, const float &v) FLASHMEM {
                                                         auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                         pThis->GetBinding().mSyncFreqParam.SetKTParamValue(v);
                                                     },
                                                     this},
                                       Property<bool>{[](void *cap) FLASHMEM -> bool { // enabled only for sync
                                                         auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                         return IsSyncWaveform(pThis->GetBinding().mWaveform);
                                                      },
                                                      this}};


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
                                       Property<bool>{[](void *cap) FLASHMEM -> bool { // enabled only for sync
                                                         auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
                                                         return (pThis->GetBinding().mWaveform == OscWaveformShape::Pulse)
                                                         || (pThis->GetBinding().mWaveform == OscWaveformShape::SyncPulse)
                                                         || (pThis->GetBinding().mWaveform == OscWaveformShape::VarTriangle)
                                                         ;
                                                      },
                                                      this}};

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

    // returns info about any OTHER oscillator than the one in focus. iosc is 0-(POLYBLEP_OSC_COUNT - 2).
    int GetOtherOscillatorIndex(int iosc)
    {
        // if i am oscillator 2, and you demand #2, then add 1.
        if (iosc < mOscillatorIndex)
            return iosc;
        return iosc + 1;
    }

    FunctionListSettingItem mCopyToOsc = {
        "Copy to osc...",
        POLYBLEP_OSC_COUNT - 1,                      // minus one because you can't copy to self.
        [](void *cap, size_t i) FLASHMEM -> String { // itemNameGetter,
            auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
            int iOtherOsc = pThis->GetOtherOscillatorIndex(i);
            return String("OSC") + (iOtherOsc + 1) + " " + pThis->mpPresetBinding->OscillatorToString(iOtherOsc);
        },
        [](void *cap, size_t i) FLASHMEM -> void { // cc::function<void(void*,size_t)>::ptr_t onClick,
            auto *pThis = (SynthPatchOscillatorMenuStuff *)cap;
            int iOtherOsc = pThis->GetOtherOscillatorIndex(i);

            pThis->mpPresetBinding->mOsc[iOtherOsc] = pThis->GetBinding();

            auto fromName = String("OSC") + (pThis->mOscillatorIndex + 1) + " " +
                            pThis->mpPresetBinding->OscillatorToString(pThis->mOscillatorIndex);
            auto toName = String("OSC") + (iOtherOsc + 1) + " " + pThis->mpPresetBinding->OscillatorToString(iOtherOsc);

            pThis->mpDisplay->ShowToast(String("Copied ") + fromName + "\nto\n" + toName);
        },
        AlwaysEnabled,
        this};

    ISettingItem *mArray[18] = {
        &mEnabled,
        &mWaveform,
        &mGain,
        &mFMFeedback,
        &mPan,
        &mPortamentoTimeMS,
        &mFreqMul,
        &mFreqOffset,
        &mFreqParam,
        &mFreqParamKT,
        &mSyncFreqParam,
        &mSyncFreqParamKT,
        &mPitchSemis,
        &mPitchFine,
        &mPulseWidth,
        &mPhaseRestart,
        &mPhaseOffset,
        &mCopyToOsc,
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

    FloatSettingItem mMasterVolume = {"MasterVol",
                                      StandardRangeSpecs::gFloat_0_1,
                                      Property<float>{[](void *cap) FLASHMEM {
                                                          auto *pThis = (SynthPatchMenuApp *)cap;
                                                          return pThis->GetBinding().mMasterVolume.GetParamValue();
                                                      },
                                                      [](void *cap, const float &v) FLASHMEM {
                                                          auto *pThis = (SynthPatchMenuApp *)cap;
                                                          pThis->GetBinding().mMasterVolume.SetValue(v);
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


    // EnumSettingItem(const String& name, const EnumInfo<T>& enumInfo, const Property<T>& binding,
    // cc::function<bool()>::ptr_t isEnabled) :
    EnumSettingItem<VoicingMode> mVoicingMode = {"Voicing",
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

    FloatSettingItem mBreathFiltMax = {" - freq",
                                       StandardRangeSpecs::gFloat_0_1_Fine,
                                       Property<float>{[](void *cap) FLASHMEM {
                                                           auto *pThis = (SynthPatchMenuApp *)cap;
                                                           return pThis->GetBinding().mFilterFreqParam.GetParamValue();
                                                       },
                                                       [](void *cap, const float &v) {
                                                           auto *pThis = (SynthPatchMenuApp *)cap;
                                                           pThis->GetBinding().mFilterFreqParam.SetParamValue(v);
                                                       },
                                                       this},
                                       Property<bool>{[](void *cap) FLASHMEM -> bool {
                                                          // auto *pThis = (SynthPatchMenuApp *)cap;
                                                          //  TODO: check filter capabilities.
                                                          return true;
                                                      },
                                                      this}};

    FloatSettingItem mBreathFiltKS = {" - filt KT",
                                      NumericEditRangeSpec<float>{0.0f, 2.0f, .2f, .1f, 0.01f},
                                      Property<float>{[](void *cap) FLASHMEM {
                                                          auto *pThis = (SynthPatchMenuApp *)cap;
                                                          return pThis->GetBinding().mFilterFreqParam.GetKTParamValue();
                                                      },
                                                      [](void *cap, const float &v) {
                                                          auto *pThis = (SynthPatchMenuApp *)cap;
                                                          pThis->GetBinding().mFilterFreqParam.SetKTParamValue(v);
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

    FloatSettingItem mModCurve = {"Curve",
                                StandardRangeSpecs::gFloat_N1_1,
                                Property<float>{[](void *cap) FLASHMEM {
                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                  return pThis->GetModulationBinding().mCurveShape.GetParamValue();
                                              },
                                              [](void *cap, const float &v) FLASHMEM {
                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                  pThis->GetModulationBinding().mCurveShape.SetParamValue(v);
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
                                                          return pThis->GetModulationBinding().mAuxAmount;
                                                      },
                                                      [](void *cap, const float &v) {
                                                          auto *pThis = (SynthPatchMenuApp *)cap;
                                                          pThis->GetModulationBinding().mAuxAmount = v;
                                                      },
                                                      this},
                                      AlwaysEnabled};

    EnumSettingItem<ModulationPolarityTreatment> mModAuxPolarity = {
        " - Aux polarity",
        gModulationAuxPolarityTreatmentInfo,
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

    FloatSettingItem mModAuxCurve = {" - Aux Curve",
                                   StandardRangeSpecs::gFloat_N1_1,
                                   Property<float>{[](void *cap) FLASHMEM {
                                                     auto *pThis = (SynthPatchMenuApp *)cap;
                                                     return pThis->GetModulationBinding().mAuxCurveShape.GetParamValue();
                                                 },
                                                 [](void *cap, const float &v) FLASHMEM {
                                                     auto *pThis = (SynthPatchMenuApp *)cap;
                                                     pThis->GetModulationBinding().mAuxCurveShape.SetParamValue(v);
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

    // TODO: use a multi submenu
    SubmenuSettingItem mModEnv1SubmenuItem = {String("ENV1 (voice)"),
                                              [](void *cap) FLASHMEM {
                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                  return pThis->mEnvEditor.GetSubmenuList(pThis->GetBinding().mEnvelopes[0]);
                                              },
                                              AlwaysEnabled,
                                              this};

    SubmenuSettingItem mModEnv2SubmenuItem = {String("ENV2"),
                                              [](void *cap) FLASHMEM {
                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                  return pThis->mEnvEditor.GetSubmenuList(pThis->GetBinding().mEnvelopes[1]);
                                              },
                                              AlwaysEnabled,
                                              this};

    SubmenuSettingItem mModEnv3SubmenuItem = {String("ENV3"),
                                              [](void *cap) FLASHMEM {
                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                  return pThis->mEnvEditor.GetSubmenuList(pThis->GetBinding().mEnvelopes[2]);
                                              },
                                              AlwaysEnabled,
                                              this};

    LFOMenuApp mLFOEditor;

    // TODO: use a multi submenu
    SubmenuSettingItem mModLFO1SubmenuItem = {String("LFO1"),
                                              [](void *cap) FLASHMEM {
                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                  return pThis->mLFOEditor.GetSubmenuList(pThis->GetBinding().mLFOs[0]);
                                              },
                                              AlwaysEnabled,
                                              this};

    SubmenuSettingItem mModLFO2SubmenuItem = {String("LFO2"),
                                              [](void *cap) FLASHMEM {
                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                  return pThis->mLFOEditor.GetSubmenuList(pThis->GetBinding().mLFOs[1]);
                                              },
                                              AlwaysEnabled,
                                              this};

    SubmenuSettingItem mModLFO3SubmenuItem = {String("LFO3"),
                                              [](void *cap) FLASHMEM {
                                                  auto *pThis = (SynthPatchMenuApp *)cap;
                                                  return pThis->mLFOEditor.GetSubmenuList(pThis->GetBinding().mLFOs[2]);
                                              },
                                              AlwaysEnabled,
                                              this};

    ISettingItem *mModulationsSubmenuArray[7] = {
        &mModLFO1SubmenuItem,
        &mModLFO2SubmenuItem,
        &mModLFO3SubmenuItem,
        &mModEnv1SubmenuItem,
        &mModEnv2SubmenuItem,
        &mModEnv3SubmenuItem,
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
                                               pThis->mOscSubmenuStuff.SetBindings(
                                                   &pThis->GetBinding(), 0, &pThis->mDisplay);
                                               // pThis->mOscSubmenuStuff.mpBinding = &pThis->GetBinding().mOsc[0];
                                               // pThis->mOscSubmenuStuff.mOscillatorIndex = 0;
                                               return &pThis->mOscSubmenuStuff.mSubmenuList;
                                           },
                                           AlwaysEnabled,
                                           this};

    SubmenuSettingItem mOsc2SubmenuItem = {[](void *cap) FLASHMEM -> String {
                                               auto *pThis = (SynthPatchMenuApp *)cap;
                                               return String("OSC2 ") + pThis->GetBinding().OscillatorToString(1);
                                           },
                                           [](void *cap) FLASHMEM {
                                               auto *pThis = (SynthPatchMenuApp *)cap;
                                               pThis->mOscSubmenuStuff.SetBindings(
                                                   &pThis->GetBinding(), 1, &pThis->mDisplay);
                                               //    pThis->mOscSubmenuStuff.mpBinding = &pThis->GetBinding().mOsc[1];
                                               //    pThis->mOscSubmenuStuff.mOscillatorIndex = 1;
                                               return &pThis->mOscSubmenuStuff.mSubmenuList;
                                           },
                                           AlwaysEnabled,
                                           this};

    SubmenuSettingItem mOsc3SubmenuItem = {[](void *cap) FLASHMEM -> String {
                                               auto *pThis = (SynthPatchMenuApp *)cap;
                                               return String("OSC3 ") + pThis->GetBinding().OscillatorToString(2);
                                           },
                                           [](void *cap) FLASHMEM {
                                               auto *pThis = (SynthPatchMenuApp *)cap;
                                               pThis->mOscSubmenuStuff.SetBindings(
                                                   &pThis->GetBinding(), 2, &pThis->mDisplay);
                                               //    pThis->mOscSubmenuStuff.mpBinding = &pThis->GetBinding().mOsc[2];
                                               //    pThis->mOscSubmenuStuff.mOscillatorIndex = 2;
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

    SubmenuSettingItem mBreathFilterSubmenuItem = {String("Filter"), &mBreathFilterSubmenuList, AlwaysEnabled};

    ISettingItem *mArray[20] = {
        &mMasterVolume,
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
