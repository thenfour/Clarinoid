
#pragma once

#include <clarinoid/basic/Basic.hpp>

#include "HarmonizerSettings.hpp"
#include "SynthSettings.hpp"
#include "LoopstationSettings.hpp"
#include "ControlMapping.hpp"

namespace clarinoid
{

enum class GlobalScaleRefType : uint8_t
{
    Chosen,
    Deduced,
};

EnumItemInfo<GlobalScaleRefType> gGlobalScaleRefTypeItems[2] = {
    {GlobalScaleRefType::Chosen, "Chosen"},
    {GlobalScaleRefType::Deduced, "Deduced"},
};

EnumInfo<GlobalScaleRefType> gGlobalScaleRefTypeInfo("GlobalScaleRefType", gGlobalScaleRefTypeItems);

struct PerformancePatch
{
    String mName = "--";

    float mBPM = 104.0f;

    int8_t mTranspose = DEFAULT_TRANSPOSE;

    GlobalScaleRefType mGlobalScaleRef = GlobalScaleRefType::Deduced;
    Scale mGlobalScale = Scale{Note::E, ScaleFlavorIndex::MajorPentatonic};  // you can set this in menus
    Scale mDeducedScale = Scale{Note::C, ScaleFlavorIndex::MajorPentatonic}; // this is automatically populated always

    int16_t mSynthPresetA = 0;
    bool mSynthAEnabled = true;
    float mSynthAGain = 1.0f;

    int16_t mSynthPresetB = -1; // -1 = mute, no patch.
    bool mSynthBEnabled = true;
    float mSynthBGain = 1.0f;

    int16_t mHarmPreset = 0;
    bool mHarmEnabled = true;
    float mHarmGain = 1.0f;

    float mSynthStereoSpread = 0.35f; // -1 to 1

    float mMasterGain = 1.0f;

    float mMasterFXGain = 1.0f;
    bool mMasterFXEnable = true;

    float mReverbGain = DecibelsToLinear(-3.0f);
    float mReverbDamping = 0.6f;
    float mReverbSize = 0.6f;
    bool mReverbEnabled = true;

    bool mDelayEnabled = true;
    float mDelayGain = DecibelsToLinear(-3.0f);

    TimeWithBasis mDelayTime;

    float mDelayStereoSep = 30;
    float mDelayFeedbackLevel = 0.3f;
    ClarinoidFilterType mDelayFilterType = ClarinoidFilterType::BP_Moog2;
    float mDelayCutoffFrequency = 1000;
    float mDelaySaturation = 0.2f;
    float mDelayQ = 0.1f;

    PerformancePatch()
    {
        mDelayTime.mBasis = TimeBasis::Milliseconds;
        mDelayTime.mTimeMS = 225;
    }

    String ToString(int index) const
    {
        return String("") + index + ":" + mName;
    }
};

static constexpr auto aosenuthaoesuth = sizeof(PerformancePatch);

struct AppSettings
{
    ControlMapping mControlMappings[MAX_CONTROL_MAPPINGS];

    UnipolarMapping mPitchUpMapping;
    UnipolarMapping mPitchDownMapping;

    int mNoteChangeSmoothingFrames = 3;
    // the idea here is that the bigger the interval, the bigger the delay required to lock in the note
    float mNoteChangeSmoothingIntervalFrameFactor = 0.30f;

    bool mDisplayDim = true;

    bool mMetronomeSoundOn = false;
    float mMetronomeGain = 0.34f;
    int mMetronomeNote = 80;
    int mMetronomeDecayMS = 15;

    bool mMetronomeLED = false;
    int mMetronomeBrightness = 255;
    float mMetronomeLEDDecay = 0.1f;

    HarmSettings mHarmSettings;
    LooperSettings mLooperSettings;
    SynthSettings mSynthSettings;

    PerformancePatch mPerformancePatches[PERFORMANCE_PATCH_COUNT];
    uint16_t mCurrentPerformancePatch = 0;

    PerformancePatch &GetCurrentPerformancePatch()
    {
        return mPerformancePatches[mCurrentPerformancePatch];
    }

    String GetSynthPatchName(int16_t id)
    {
        if (id < 0 || (size_t)id >= SYNTH_PRESET_COUNT)
            return String("<none>");
        return mSynthSettings.mPresets[id].ToString(id);
    }

    String GetHarmPatchName(int16_t id)
    {
        if (id < 0 || (size_t)id >= HARM_PRESET_COUNT)
            return String("<none>");
        return mHarmSettings.mPresets[id].ToString(id);
    }

    String GetPerfPatchName(int16_t id)
    {
        if (id < 0 || (size_t)id >= PERFORMANCE_PATCH_COUNT)
            return String("<none>");
        return mPerformancePatches[id].ToString(id);
    }

    HarmPreset &FindHarmPreset(int16_t id)
    {
        id = RotateIntoRange(id, HARM_PRESET_COUNT);
        return mHarmSettings.mPresets[id];
    }

    SynthPreset &FindSynthPreset(int16_t id)
    {
        id = RotateIntoRange(id, SYNTH_PRESET_COUNT);
        return mSynthSettings.mPresets[id];
    }

    static void InitSpacecarPerf(PerformancePatch &p) // originally thicc
    {
        p.mName = "Spacecar";
        // p.mSynthStereoSpread = 0.35f;
        p.mMasterGain = DecibelsToLinear(0);
        p.mMasterFXGain = DecibelsToLinear(-8);
        p.mMasterFXEnable = false;
        p.mSynthPresetA = SynthPresetID_Fluvial;
        p.mSynthAGain = DecibelsToLinear(-3);
        p.mSynthPresetB = SynthPresetID_SynthTrumpetDoubler;
        p.mSynthBGain = DecibelsToLinear(-3);
    }

    static void InitCrystalPerf(PerformancePatch &p) // originally thicc
    {
        p.mName = "Crystal";
        // p.mSynthStereoSpread = 0.35f;
        p.mMasterGain = DecibelsToLinear(0);
        p.mMasterFXGain = DecibelsToLinear(-8);
        p.mMasterFXEnable = false;
        p.mSynthPresetA = SynthPresetID_Crystal;
        p.mSynthAGain = DecibelsToLinear(-3);
        p.mSynthPresetB = SynthPresetID_SynccyLead;
        p.mSynthBGain = DecibelsToLinear(-3);
    }

    static void InitSyntheticPerf(PerformancePatch &p) // originally thicc
    {
        p.mName = "SyntheticBeach";
        // p.mSynthStereoSpread = 0.35f;
        p.mMasterGain = DecibelsToLinear(0);
        p.mMasterFXGain = DecibelsToLinear(-8);
        p.mMasterFXEnable = false;
        p.mSynthPresetA = SynthPresetID_Fluvial;
        p.mSynthAGain = DecibelsToLinear(-3);
        p.mSynthPresetB = SynthPresetID_SynthTrumpetDoubler;
        p.mSynthBGain = DecibelsToLinear(-3);
    }

    static void InitClarinoidSoloPerf(PerformancePatch &p)
    {
        p.mName = "ClarinoidSolo";
        // p.mSynthStereoSpread = 0.35f;
        p.mMasterGain = DecibelsToLinear(0);
        p.mMasterFXGain = DecibelsToLinear(-8);
        p.mMasterFXEnable = true;
        p.mSynthPresetA = SynthPresetID_Fluvial;
        p.mSynthAGain = DecibelsToLinear(-3);
        p.mSynthPresetB = 4; // detuned PWM lead
        p.mSynthBGain = DecibelsToLinear(-3);
        p.mHarmPreset = 1; // world peace Eb -> col bass -> quintal -> fusion
        p.mHarmEnabled = false;
        p.mHarmGain = DecibelsToLinear(-3);
    }

    static void InitChameleon(PerformancePatch &p)
    {
        p.mName = "Chameleon";
        p.mMasterGain = DecibelsToLinear(0);
        p.mMasterFXGain = DecibelsToLinear(-8);
        p.mMasterFXEnable = false;
        p.mSynthPresetA = 10; // funky
        p.mSynthAGain = DecibelsToLinear(-3);
        p.mSynthPresetB = 4; // detuned PWM lead
        p.mSynthBGain = DecibelsToLinear(-10);
        p.mHarmPreset = 5; // world peace Db
        p.mHarmEnabled = true;
        p.mHarmGain = DecibelsToLinear(-3);
    }

    

    static void InitHightechPerf(PerformancePatch &p) // originally thicc
    {
        p.mName = "Hightech";
        // p.mSynthStereoSpread = 0.35f;
        p.mMasterGain = DecibelsToLinear(0);
        p.mMasterFXGain = DecibelsToLinear(-8);
        p.mMasterFXEnable = false;
        p.mSynthPresetA = SynthPresetID_Fluvial;
        p.mSynthAGain = DecibelsToLinear(-3);
        p.mSynthPresetB = SynthPresetID_SynthTrumpetDoubler;
        p.mSynthBGain = DecibelsToLinear(-3);
    }

    static void InitBellyPerf(PerformancePatch &p) // originally thicc
    {
        p.mName = "BellyCrawl";
        // p.mSynthStereoSpread = 0.35f;
        p.mMasterGain = DecibelsToLinear(0);
        p.mMasterFXGain = DecibelsToLinear(-8);
        p.mMasterFXEnable = false;
        p.mSynthPresetA = SynthPresetID_Fluvial;
        p.mSynthAGain = DecibelsToLinear(-3);
        p.mSynthPresetB = SynthPresetID_SynthTrumpetDoubler;
        p.mSynthBGain = DecibelsToLinear(-3);
    }

    static void InitSilkSuspendersPerf(PerformancePatch &p)
    {
        p.mName = "Silk Susp";
        p.mMasterGain = DecibelsToLinear(0);
        p.mSynthPresetA = SynthPresetID_PanFlute;
        p.mSynthAGain = DecibelsToLinear(-3);
        p.mSynthPresetB = SynthPresetID_CinematicTag;
        p.mSynthBGain = DecibelsToLinear(-3);
        p.mMasterFXEnable = false;
        p.mMasterFXGain = DecibelsToLinear(-10);
        p.mBPM = 110;
    }

    static void InitSoaringGuitarPerf(PerformancePatch &p)
    {
        p.mName = "Soaring Guitar";
        p.mMasterGain = DecibelsToLinear(0);
        p.mSynthPresetA = SynthPresetID_Fluvial;
        p.mSynthAGain = DecibelsToLinear(-3);
        p.mSynthPresetB = SynthPresetID_SynthTrumpetDoubler;
        p.mSynthBGain = DecibelsToLinear(-3);
        p.mMasterFXGain = DecibelsToLinear(-12);
    }

    AppSettings()
    {
        size_t i = 1;                                // 0 = default, no performance
        InitSpacecarPerf(mPerformancePatches[i++]);  // spacecar
        InitCrystalPerf(mPerformancePatches[i++]);   // crystal
        InitSyntheticPerf(mPerformancePatches[i++]); // synthetic
        InitClarinoidSoloPerf(mPerformancePatches[i++]); // 
        InitHightechPerf(mPerformancePatches[i++]);  // hightech
        InitBellyPerf(mPerformancePatches[i++]);     // belly
        InitSilkSuspendersPerf(mPerformancePatches[i++]);
        InitChameleon(mPerformancePatches[i++]);

        InitSoaringGuitarPerf(mPerformancePatches[i++]);
    }
};

static constexpr auto appsettingssize = sizeof(AppSettings);
static constexpr auto appsettingssize67 = sizeof(AppSettings::mHarmSettings);
static constexpr auto appsettingssize55 = sizeof(AppSettings::mPerformancePatches);
static constexpr auto appsettingssize44 = sizeof(AppSettings::mSynthSettings);
static constexpr auto rththth = sizeof(AppSettings::mControlMappings);

} // namespace clarinoid
