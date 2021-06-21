#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"
#include "NumericSettingItem.hpp"
#include "EnumSettingItem.hpp"
#include "FunctionListSettingItem.hpp"

namespace clarinoid
{

    // acts as a standalone "current patch" editor app,
    // as well as sub-app of general synth settings. so you can change params of patches without them even being selected.
    struct SynthPatchMenuApp : public SettingsMenuApp
    {
        virtual const char *DisplayAppGetName() override { return "SynthPatchMenuApp"; }

        SynthPatchMenuApp(CCDisplay &d) : SettingsMenuApp(d)//, mDisplay(d)
        {
        }

        //AppSettings &GetAppSettings() { return *mDisplay.mAppSettings; }
        SynthPreset &GetBinding() {
            return GetAppSettings()->mSynthSettings.mPresets[GetAppSettings()->mGlobalSynthPreset];
        }

        // LabelSettingItem(cc::function<String()>::ptr_t text, cc::function<bool()>::ptr_t isEnabled) :
        LabelSettingItem mBigSeparator = {
            Property<String> { [](void*){ return String("----"); } },
            AlwaysEnabled };

        // NumericSettingItem(const String& name, T min_, T max_, const Property<T>& binding, typename cc::function<bool()>::ptr_t isEnabled) :
        // BoolSettingItem(const String& name, const String& trueCaption, const String& falseCaption, const Property<bool>& binding, typename cc::function<bool()>::ptr_t isEnabled) :
        FloatSettingItem mDetune = {"Detune", StandardRangeSpecs::gFloat_0_2,
                                    Property<float>{
                                        [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mDetune; },
                                        [](void *cap, const float &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mDetune = v; },
                                        this},
                                    AlwaysEnabled};

        FloatSettingItem mPortamentoTime = {"Portamento", StandardRangeSpecs::gPortamentoRange,
                                            Property<float>{
                                                [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mPortamentoTime; },
                                                [](void *cap, const float &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mPortamentoTime = v; },
                                                this},
                                            AlwaysEnabled};

        BoolSettingItem mSync = {"Sync", "On", "Off",
                                 Property<bool>{
                                     [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mSync; },
                                     [](void *cap, const bool &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mSync = v; },
                                     this},
                                 AlwaysEnabled};

        FloatSettingItem mSyncMultMin = {" - mult min", NumericEditRangeSpec<float> { 0.0f, 15.0f },
                                            Property<float>{
                                                [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mSyncMultMin; },
                                                [](void *cap, const float &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mSyncMultMin = v; },
                                                this},
                                            Property<bool> { [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mSync; }, this }
                                            };

        FloatSettingItem mSyncMultMax = {" - mult max", NumericEditRangeSpec<float> { 0.0f, 15.0f },
                                            Property<float>{
                                                [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mSyncMultMax; },
                                                [](void *cap, const float &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mSyncMultMax = v; },
                                                this},
                                            Property<bool> { [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mSync; }, this }
                                            };


        // EnumSettingItem(const String& name, const EnumInfo<T>& enumInfo, const Property<T>& binding, cc::function<bool()>::ptr_t isEnabled) :
        EnumSettingItem<ClarinoidFilterType> mBreathFiltType = {"Filter", gClarinoidFilterTypeInfo,
                Property<ClarinoidFilterType>{
                    [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mFilterType; },
                    [](void *cap, const ClarinoidFilterType &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mFilterType = v; },
                    this},
                AlwaysEnabled};

        FloatSettingItem mBreathFiltQ = {" - Reso", NumericEditRangeSpec<float> { 0.0f, 1.0f, .1f, .05f, 0.1f },
                                            Property<float>{
                                                [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mFilterQ; },
                                                [](void *cap, const float &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mFilterQ = v; },
                                                this},
                                            AlwaysEnabled};

        FloatSettingItem mBreathFiltSaturation = {" - saturation", NumericEditRangeSpec<float> { 0.0f, 1.0f },
                                            Property<float>{
                                                [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mFilterSaturation; },
                                                [](void *cap, const float &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mFilterSaturation = v; },
                                                this},
                                            AlwaysEnabled};

        FloatSettingItem mBreathFiltMin = {" - filt min", NumericEditRangeSpec<float> { 0.0f, 3000.0f },
                                            Property<float>{
                                                [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mFilterMinFreq; },
                                                [](void *cap, const float &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mFilterMinFreq = v; },
                                                this},
                                            AlwaysEnabled};

        FloatSettingItem mBreathFiltMax = {" - filt max", NumericEditRangeSpec<float> { 0.0f, 25000.0f },
                                            Property<float>{
                                                [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mFilterMaxFreq; },
                                                [](void *cap, const float &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mFilterMaxFreq = v; },
                                                this},
                                            AlwaysEnabled};

        FloatSettingItem mBreathFiltKS = {" - filt KS", NumericEditRangeSpec<float> { 0.0f, 2.0f, .2f, .1f, 0.01f },
                                            Property<float>{
                                                [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mFilterKeytracking; },
                                                [](void *cap, const float &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mFilterKeytracking = v; },
                                                this},
                                            AlwaysEnabled};


        BoolSettingItem mDCEnabled = {"DC filter", "On", "Off",
                                 Property<bool>{
                                     [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mDCFilterEnabled; },
                                     [](void *cap, const bool &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mDCFilterEnabled = v; },
                                     this},
                                 AlwaysEnabled};

        FloatSettingItem mDCCutoff = {" - cutoff", NumericEditRangeSpec<float> { 0.01f, 100.0f },
                                            Property<float>{
                                                [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mDCFilterCutoff; },
                                                [](void *cap, const float &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mDCFilterCutoff = v; },
                                                this},
                                            Property<bool> { [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mDCFilterEnabled; }, this }
                                            };




        // EnumSettingItem(const String& name, const EnumInfo<T>& enumInfo, const Property<T>& binding, cc::function<bool()>::ptr_t isEnabled) :
        EnumSettingItem<OscWaveformShape> mOsc1Waveform = {"Osc1-Waveform", gOscWaveformShapeInfo,
                                                           Property<OscWaveformShape>{
                                                               [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mOsc1Waveform; },
                                                               [](void *cap, const OscWaveformShape &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mOsc1Waveform = v; },
                                                               this},
                                                           AlwaysEnabled};

        FloatSettingItem mOsc1Gain = {" - Gain", StandardRangeSpecs::gFloat_0_1,
                                      Property<float>{
                                          [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mOsc1Gain; },
                                          [](void *cap, const float &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mOsc1Gain = v; },
                                          this},
                                      AlwaysEnabled};

        IntSettingItem mOsc1PitchSemis = {" - PitchSemis", StandardRangeSpecs::gTransposeRange,
                                          Property<int>{
                                              [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mOsc1PitchSemis; },
                                              [](void *cap, const int &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mOsc1PitchSemis = v; },
                                              this},
                                          AlwaysEnabled};
        FloatSettingItem mOsc1PitchFine = {" - PitchFine", StandardRangeSpecs::gFloat_N1_1,
                                           Property<float>{
                                               [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mOsc1PitchFine; },
                                               [](void *cap, const float &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mOsc1PitchFine = v; },
                                               this},
                                           AlwaysEnabled};
        FloatSettingItem mOsc1PulseWidth = {" - PulseWidth", StandardRangeSpecs::gFloat_0_1,
                                            Property<float>{
                                                [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mOsc1PulseWidth; },
                                                [](void *cap, const float &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mOsc1PulseWidth = v; },
                                                this},
                                            AlwaysEnabled};

        EnumSettingItem<OscWaveformShape> mOsc2Waveform = {"Osc2-Waveform", gOscWaveformShapeInfo,
                                                           Property<OscWaveformShape>{
                                                               [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mOsc2Waveform; },
                                                               [](void *cap, const OscWaveformShape &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mOsc2Waveform = v; },
                                                               this},
                                                           AlwaysEnabled};
        FloatSettingItem mOsc2Gain = {" - Gain", StandardRangeSpecs::gFloat_0_1,
                                      Property<float>{
                                          [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mOsc2Gain; },
                                          [](void *cap, const float &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mOsc2Gain = v; },
                                          this},
                                      AlwaysEnabled};
        IntSettingItem mOsc2PitchSemis = {" - PitchSemis", StandardRangeSpecs::gTransposeRange,
                                          Property<int>{
                                              [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mOsc2PitchSemis; },
                                              [](void *cap, const int &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mOsc2PitchSemis = v; },
                                              this},
                                          AlwaysEnabled};
        FloatSettingItem mOsc2PitchFine = {" - PitchFine", StandardRangeSpecs::gFloat_N1_1,
                                           Property<float>{
                                               [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mOsc2PitchFine; },
                                               [](void *cap, const float &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mOsc2PitchFine = v; },
                                               this},
                                           AlwaysEnabled};
        FloatSettingItem mOsc2PulseWidth = {" - PulseWidth", StandardRangeSpecs::gFloat_0_1,
                                            Property<float>{
                                                [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mOsc2PulseWidth; },
                                                [](void *cap, const float &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mOsc2PulseWidth = v; },
                                                this},
                                            AlwaysEnabled};

        EnumSettingItem<OscWaveformShape> mOsc3Waveform = {"Osc3-Waveform", gOscWaveformShapeInfo,
                                                           Property<OscWaveformShape>{
                                                               [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mOsc3Waveform; },
                                                               [](void *cap, const OscWaveformShape &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mOsc3Waveform = v; },
                                                               this},
                                                           AlwaysEnabled};
        FloatSettingItem mOsc3Gain = {" - Gain", StandardRangeSpecs::gFloat_0_1,
                                      Property<float>{
                                          [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mOsc3Gain; },
                                          [](void *cap, const float &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mOsc3Gain = v; },
                                          this},
                                      AlwaysEnabled};
        IntSettingItem mOsc3PitchSemis = {" - PitchSemis", StandardRangeSpecs::gTransposeRange,
                                          Property<int>{
                                              [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mOsc3PitchSemis; },
                                              [](void *cap, const int &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mOsc3PitchSemis = v; },
                                              this},
                                          AlwaysEnabled};
        FloatSettingItem mOsc3PitchFine = {" - PitchFine", StandardRangeSpecs::gFloat_N1_1,
                                           Property<float>{
                                               [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mOsc3PitchFine; },
                                               [](void *cap, const float &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mOsc3PitchFine = v; },
                                               this},
                                           AlwaysEnabled};
        FloatSettingItem mOsc3PulseWidth = {" - PulseWidth", StandardRangeSpecs::gFloat_0_1,
                                            Property<float>{
                                                [](void *cap) { auto* pThis = (SynthPatchMenuApp*)cap; return pThis->GetBinding().mOsc3PulseWidth; },
                                                [](void *cap, const float &v) { auto* pThis = (SynthPatchMenuApp*)cap; pThis->GetBinding().mOsc3PulseWidth = v; },
                                                this},
                                            AlwaysEnabled};


  FunctionListSettingItem mCopyPreset = {
    "Copy to ...",
    SYNTH_PRESET_COUNT,
    [](void* cap, size_t i) {//itemNameGetter,
        auto *pThis = (SynthPatchMenuApp *)cap;
      return String(String("") + i + ":" + pThis->mDisplay.mAppSettings->mSynthSettings.mPresets[i].mName);
    },
    [](void* cap, size_t i) {//cc::function<void(void*,size_t)>::ptr_t onClick,
      auto *pThis = (SynthPatchMenuApp *)cap;
      pThis->mDisplay.mAppSettings->mSynthSettings.mPresets[i] = pThis->GetBinding();
      pThis->mDisplay.ShowToast(String("Copied ") +
        pThis->GetAppSettings()->mGlobalSynthPreset + ":" + pThis->GetAppSettings()->mSynthSettings.mPresets[pThis->GetAppSettings()->mGlobalSynthPreset].mName
        + "\r\nto\r\n"
        + i + ":" + pThis->mDisplay.mAppSettings->mSynthSettings.mPresets[i].mName);
    },
    AlwaysEnabled,
    this
  };


        ISettingItem *mArray[30] =
            {
                &mBigSeparator,
                &mBreathFiltType,
                &mBreathFiltMin,
                &mBreathFiltMax,
                &mBreathFiltKS,
                &mBreathFiltQ,
                &mBreathFiltSaturation,
                &mDCEnabled,
                &mDCCutoff,
                &mDetune,
                &mPortamentoTime,
                &mSync,
                &mSyncMultMin,
                &mSyncMultMax,
                &mOsc1Waveform,
                &mOsc1Gain,
                &mOsc1PitchSemis,
                &mOsc1PitchFine,
                &mOsc1PulseWidth,
                &mOsc2Waveform,
                &mOsc2Gain,
                &mOsc2PitchSemis,
                &mOsc2PitchFine,
                &mOsc2PulseWidth,
                &mOsc3Waveform,
                &mOsc3Gain,
                &mOsc3PitchSemis,
                &mOsc3PitchFine,
                &mOsc3PulseWidth,
                &mCopyPreset,
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
            //log("synth patch app GetRootSettingsList");
            // this will only be called whne this app is standalone. that means it should always be working on the "current" synth patch.
            return &mRootList;
        }

        virtual void RenderFrontPage()
        {
            mDisplay.ClearState();
            mDisplay.mDisplay.println(String("PATCH->"));
            mDisplay.mDisplay.println(String("") + GetAppSettings()->mGlobalSynthPreset);
            SettingsMenuApp::RenderFrontPage();
        }
    };


    struct SynthSettingsApp : public SettingsMenuApp
    {
        virtual const char *DisplayAppGetName() override { return "SynthSettingsApp"; }

        SynthPatchMenuApp mSynthPatchSettingsApp;
        SynthSettings &GetSynthSettings() { return this->mAppSettings->mSynthSettings; }

        SynthSettingsApp(CCDisplay &d) : SettingsMenuApp(d), mSynthPatchSettingsApp(d)
        {
        }

        FloatSettingItem mMasterGain = {"Master gain", StandardRangeSpecs::gFloat_0_2,
                                        Property<float>{
                                            [](void *cap) { auto *pThis = (SynthSettingsApp *)cap; return pThis->GetSynthSettings().mMasterGain; },
                                            [](void *cap, const float &v) { auto *pThis = (SynthSettingsApp *)cap; pThis->GetSynthSettings().mMasterGain = v; },
                                            this},
                                        AlwaysEnabled};

        IntSettingItem mGlobalSynthPreset = {"Global synth P#", NumericEditRangeSpec<int> { 0, SYNTH_PRESET_COUNT - 1 },
                                             Property<int>{
                                                 [](void *cap) { auto *pThis = (SynthSettingsApp *)cap; return (int)pThis->GetAppSettings()->mGlobalSynthPreset; },
                                                 [](void *cap, const int &v) { auto *pThis = (SynthSettingsApp *)cap; pThis->GetAppSettings()->mGlobalSynthPreset = (uint16_t)v; },
                                                 this},
                                             AlwaysEnabled};

        IntSettingItem mTranspose = {"Transpose", StandardRangeSpecs::gTransposeRange,
                                     Property<int>{
                                         [](void *cap) { auto *pThis = (SynthSettingsApp *)cap; return pThis->GetAppSettings()->mTranspose; },
                                         [](void *cap, const int &v) { auto *pThis = (SynthSettingsApp *)cap; pThis->GetAppSettings()->mTranspose = v; },
                                         this},
                                     AlwaysEnabled};

        // there's a bit of a conflict here because it's defined as float but this is int.
        IntSettingItem mPitchbendRange = {"Pitchbend range", NumericEditRangeSpec<int> { 0, 12},
                                     Property<int>{
                                         [](void *cap) { auto *pThis = (SynthSettingsApp *)cap; return (int)pThis->GetAppSettings()->mSynthSettings.mPitchBendRange; },
                                         [](void *cap, const int &v) { auto *pThis = (SynthSettingsApp *)cap; pThis->GetAppSettings()->mSynthSettings.mPitchBendRange = (float)v; },
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
            },                           // name
            [](void *cap, size_t n) { auto* pThis = (SynthSettingsApp*)cap; return pThis->mSynthPatchSettingsApp.Start(n); }, // get submenu list
            [](void *cap, size_t n) { return true; },
            (void *)this // capture
        };




        FloatSettingItem mReverbGain = {"Reverb gain", StandardRangeSpecs::gFloat_0_1,
                                        Property<float>{
                                            [](void *cap) { auto *pThis = (SynthSettingsApp *)cap; return pThis->GetSynthSettings().mReverbGain; },
                                            [](void *cap, const float &v) { auto *pThis = (SynthSettingsApp *)cap; pThis->GetSynthSettings().mReverbGain = v; },
                                            this},
                                        AlwaysEnabled};

        FloatSettingItem mReverbDamping = {"Reverb damp", StandardRangeSpecs::gFloat_0_1,
                                        Property<float>{
                                            [](void *cap) { auto *pThis = (SynthSettingsApp *)cap; return pThis->GetSynthSettings().mReverbDamping; },
                                            [](void *cap, const float &v) { auto *pThis = (SynthSettingsApp *)cap; pThis->GetSynthSettings().mReverbDamping = v; },
                                            this},
                                        AlwaysEnabled};

        FloatSettingItem mReverbSize = {"Reverb size", StandardRangeSpecs::gFloat_0_1,
                                        Property<float>{
                                            [](void *cap) { auto *pThis = (SynthSettingsApp *)cap; return pThis->GetSynthSettings().mReverbSize; },
                                            [](void *cap, const float &v) { auto *pThis = (SynthSettingsApp *)cap; pThis->GetSynthSettings().mReverbSize = v; },
                                            this},
                                        AlwaysEnabled};

        LabelSettingItem mSeparator = {
                    Property<String> { [](void*){ return String("----"); } },
                    AlwaysEnabled };

        FloatSettingItem mDelayTimeMS = {"Delay Time", NumericEditRangeSpec<float>(1, 300),
                                        Property<float>{
                                            [](void *cap) { auto *pThis = (SynthSettingsApp *)cap; return pThis->GetSynthSettings().mDelayMS; },
                                            [](void *cap, const float &v) { auto *pThis = (SynthSettingsApp *)cap; pThis->GetSynthSettings().mDelayMS = v; },
                                            this},
                                        AlwaysEnabled};

        FloatSettingItem mDelayStereoSep = {" >Width", NumericEditRangeSpec<float>(1, 100),
                                        Property<float>{
                                            [](void *cap) { auto *pThis = (SynthSettingsApp *)cap; return pThis->GetSynthSettings().mDelayStereoSep; },
                                            [](void *cap, const float &v) { auto *pThis = (SynthSettingsApp *)cap; pThis->GetSynthSettings().mDelayStereoSep = v; },
                                            this},
                                        AlwaysEnabled};

        FloatSettingItem mDelayFeedbackLevel = {" >FB", StandardRangeSpecs::gFloat_0_1,
                                        Property<float>{
                                            [](void *cap) { auto *pThis = (SynthSettingsApp *)cap; return pThis->GetSynthSettings().mDelayFeedbackLevel; },
                                            [](void *cap, const float &v) { auto *pThis = (SynthSettingsApp *)cap; pThis->GetSynthSettings().mDelayFeedbackLevel = v; },
                                            this},
                                        AlwaysEnabled};


        EnumSettingItem<ClarinoidFilterType> mDelayFilterType = {" >Filter", gClarinoidFilterTypeInfo,
                Property<ClarinoidFilterType>{
                    [](void *cap) { auto* pThis = (SynthSettingsApp*)cap; return pThis->GetSynthSettings().mDelayFilterType; },
                    [](void *cap, const ClarinoidFilterType &v) { auto* pThis = (SynthSettingsApp*)cap; pThis->GetSynthSettings().mDelayFilterType = v; },
                    this},
                AlwaysEnabled};


        FloatSettingItem mDelayCutoffFrequency = {" > >Freq", NumericEditRangeSpec<float>(0, 22050),
                                        Property<float>{
                                            [](void *cap) { auto *pThis = (SynthSettingsApp *)cap; return pThis->GetSynthSettings().mDelayCutoffFrequency; },
                                            [](void *cap, const float &v) { auto *pThis = (SynthSettingsApp *)cap; pThis->GetSynthSettings().mDelayCutoffFrequency = v; },
                                            this},
                                        AlwaysEnabled};

        FloatSettingItem mDelaySaturation = {" > >Sat", StandardRangeSpecs::gFloat_0_1,
                                        Property<float>{
                                            [](void *cap) { auto *pThis = (SynthSettingsApp *)cap; return pThis->GetSynthSettings().mDelaySaturation; },
                                            [](void *cap, const float &v) { auto *pThis = (SynthSettingsApp *)cap; pThis->GetSynthSettings().mDelaySaturation = v; },
                                            this},
                                        AlwaysEnabled};

        FloatSettingItem mDelayQ = {" > >Q", StandardRangeSpecs::gFloat_0_1,
                                        Property<float>{
                                            [](void *cap) { auto *pThis = (SynthSettingsApp *)cap; return pThis->GetSynthSettings().mDelayQ; },
                                            [](void *cap, const float &v) { auto *pThis = (SynthSettingsApp *)cap; pThis->GetSynthSettings().mDelayQ = v; },
                                            this},
                                        AlwaysEnabled};

    // ClarinoidFilterType mDelayFilterType = ClarinoidFilterType::BP_Moog4;

        ISettingItem *mMasterFXSubmenuItems[12] =
            {
                &mReverbGain,
                &mReverbDamping,
                &mReverbSize,
                &mSeparator,
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

        SubmenuSettingItem mMasterFX = {
            String("Master FX"),
            &mMasterFXList,
            AlwaysEnabled
        };
//  SubmenuSettingItem(const String& name, SettingsList* pSubmenu, typename cc::function<bool()>::ptr_t isEnabled) :

        ISettingItem *mArray[7] =
            {
                &mMasterGain,
                &mTranspose,
                &mPitchbendRange,
                &mGlobalSynthPreset,
                &mMasterFX,
                //&mReverbGain,
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
            //log(String("synth settings app settings = ") + (uint32_t)mAppSettings);
            mDisplay.ClearState();
            mDisplay.mDisplay.println(String("SYNTH > "));
            mDisplay.mDisplay.println(String("#: ") + GetAppSettings()->mGlobalSynthPreset);

            SettingsMenuApp::RenderFrontPage();
        }
    };



} // namespace clarinoid
