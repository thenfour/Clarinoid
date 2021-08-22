
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

static constexpr auto aosenuthaoesuth = sizeof (PerformancePatch);

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

    static void InitThiccPerf(PerformancePatch &p)
    {
        p.mName = "Thicc";
        p.mSynthStereoSpread = 0.35f;
        p.mSynthPresetB = SynthPresetID_SynthTrumpetDoubler;
    }

    static void InitSilkSuspendersPerf(PerformancePatch &p)
    {
        p.mName = "Silk Susp";
        p.mSynthPresetA = SynthPresetID_PanFlute;
        p.mSynthPresetB = SynthPresetID_CinematicTag;
        p.mBPM = 110;
        p.mReverbGain = DecibelsToLinear(-12.0f);
        p.mDelayGain = DecibelsToLinear(-12.0f);
    }

    static void InitSoaringGuitarPerf(PerformancePatch &p)
    {
        p.mName = "Soaring Guitar";
        p.mSynthPresetA = SynthPresetID_Fluvial;
        p.mSynthPresetB = SynthPresetID_SynthTrumpetDoubler;
    }

    AppSettings()
    {
        size_t i = 1;
        InitThiccPerf(mPerformancePatches[i++]);
        InitSilkSuspendersPerf(mPerformancePatches[i++]);
        InitSoaringGuitarPerf(mPerformancePatches[i++]);
    }
};

static constexpr auto appsettingssize = sizeof(AppSettings);
static constexpr auto appsettingssize67 = sizeof(AppSettings::mHarmSettings);
static constexpr auto appsettingssize55 = sizeof(AppSettings::mPerformancePatches);
static constexpr auto appsettingssize44 = sizeof(AppSettings::mSynthSettings);
static constexpr auto rththth = sizeof(AppSettings::mControlMappings);

} // namespace clarinoid
