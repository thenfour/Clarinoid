
#pragma once

#include <clarinoid/basic/Basic.hpp>

#include "HarmonizerSettings.hpp"
#include "SynthSettings.hpp"
#include "LoopstationSettings.hpp"
#include "ControlMapping.hpp"

namespace clarinoid
{

static constexpr int kMaxBeatsPerBar = 8;
static constexpr int kMaxSubdivisionsPerBeat = 8;

enum class PerfDisplayStyle : uint8_t
{
    Cube,
    CubeTessellated,
    Tetrahedron,
    Icosahedron,
    Geodesic,
    Metronome,
};

EnumItemInfo<PerfDisplayStyle> gPerfDisplayStyleItems[6] = {
    {PerfDisplayStyle::Cube, "Cube"},
    {PerfDisplayStyle::CubeTessellated, "Tessellated Cube"},
    {PerfDisplayStyle::Tetrahedron, "Tetrahedron"},
    {PerfDisplayStyle::Icosahedron, "Icosahedron"},
    {PerfDisplayStyle::Geodesic, "Geodesic"},
    {PerfDisplayStyle::Metronome, "Metronome"},
};

EnumInfo<PerfDisplayStyle> gPerfDisplayStyleInfo("PerfDisplayStyle", gPerfDisplayStyleItems);

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

    PerfDisplayStyle mPerfDisplayStyle = PerfDisplayStyle::Geodesic;

    float mBPM = 104.0f;
    int8_t mBeatsPerBar = 4;
    int8_t mBeatSubdivisions = 2;

    GlobalScaleRefType mGlobalScaleRef = GlobalScaleRefType::Chosen;
    Scale mGlobalScale = Scale{Note::C, ScaleFlavorIndex::MajorPentatonic};  // you can set this in menus
    Scale mDeducedScale = Scale{Note::C, ScaleFlavorIndex::MajorPentatonic}; // this is automatically populated always

    int8_t mTranspose = DEFAULT_TRANSPOSE;

    int16_t mSynthPresetA = 0;
    float mSynthAGain = 1.0f;
    int16_t mSynthATranspose = 0;

    int16_t mSynthPresetB = -1; // -1 = mute, no patch.
    int16_t mSynthBTranspose = 0;
    float mSynthBGain = 1.0f;
    int16_t mHarmPreset = 0;
    float mHarmGain = 1.0f;

    float mSynthStereoSpread = 0.35f; // -1 to 1
    float mDetuneSemis = 0.1f;        // detunes both synths away from each other by this amount.

    float mMasterGain = 1.0f;

    float mMasterFXGain = 1.0f;

    bool mSynthAEnabled = true;
    bool mSynthBEnabled = true;
    bool mHarmEnabled = false;
    bool mMasterFXEnable = true;
    bool mReverbEnabled = true;
    bool mDelayEnabled = true;

    float mReverbGain = DecibelsToLinear(-3.0f);
    float mReverbDamping = 0.6f;
    float mReverbSize = 0.6f;
    float mDelayGain = DecibelsToLinear(-3.0f);

    float mDelayModulationDepth = 0.003f; // todo: document what unit this is.
    float mDelayModulationRateHz = 0.5f;

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

    bool mDisplayDim = false;

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

    // static void InitSoaringGuitarPerf(PerformancePatch &p) // originally thicc
    // {
    //     p.mName = "/k Spacecar";
    //     p.mMasterGain = DecibelsToLinear(-10);
    //     p.mMasterFXEnable = false;
    //     p.mSynthPresetA = SynthPresetID_PWMMono;
    //     p.mSynthAGain = DecibelsToLinear(-3);
    //     p.mSynthPresetB = SynthPresetID_SynthTrumpetDoubler;
    //     p.mSynthBGain = DecibelsToLinear(-12);
    //     p.mHarmPreset = HarmPresetID_WorldPeace_Bb;
    //     p.mHarmEnabled = false;
    // }

    // static void InitRoadPerf(PerformancePatch &p) // originally thicc
    // {
    //     p.mName = "/k Road";
    //     p.mMasterGain = DecibelsToLinear(-10);
    //     p.mMasterFXEnable = false;
    //     p.mSynthPresetA = SynthPresetID_PWMMono;
    //     p.mSynthAGain = DecibelsToLinear(-3);
    //     p.mSynthPresetB = SynthPresetID_SynthTrumpetDoubler;
    //     p.mSynthBGain = DecibelsToLinear(-12);
    //     p.mHarmPreset = HarmPresetID_Road;
    //     p.mHarmGain = DecibelsToLinear(-6);
    //     p.mHarmEnabled = false;
    // }

    // // static void InitSoaringGuitarPerf(PerformancePatch &p)
    // {
    //     p.mName = "Fluvial Guitar";
    //     p.mMasterGain = DecibelsToLinear(0);
    //     p.mMasterFXGain = DecibelsToLinear(-12);
    //     p.mSynthPresetA = SynthPresetID_Fluvial;
    //     p.mSynthAGain = DecibelsToLinear(-3);
    //     p.mSynthPresetB = SynthPresetID_SynthTrumpetDoubler;
    //     p.mSynthBGain = DecibelsToLinear(-3);
    // }

    // static void InitCrystalPerf(PerformancePatch &p) // originally thicc
    // {
    //     p.mName = "/k Crystal";
    //     p.mMasterGain = DecibelsToLinear(-8.7f + 0.6f);
    //     p.mMasterFXEnable = false;
    //     p.mSynthPresetA = SynthPresetID_Crystal;
    //     p.mSynthAGain = DecibelsToLinear(-4.5f);
    //     p.mSynthPresetB = SynthPresetID_CrystalSync;
    //     p.mSynthBGain = DecibelsToLinear(-8);
    // }

    // // static void InitSyntheticPerf(PerformancePatch &p) // originally thicc
    // {
    //     p.mName = "SyntheticBeach";
    //     // p.mSynthStereoSpread = 0.35f;
    //     p.mMasterGain = DecibelsToLinear(0);
    //     p.mMasterFXGain = DecibelsToLinear(-8);
    //     p.mMasterFXEnable = false;
    //     p.mSynthPresetA = SynthPresetID_Fluvial;
    //     p.mSynthAGain = DecibelsToLinear(-3);
    //     p.mSynthPresetB = SynthPresetID_SynthTrumpetDoubler;
    //     p.mSynthBGain = DecibelsToLinear(-3);
    // }

    // static void InitHightechPerf(PerformancePatch &p) // originally thicc
    // {
    //     p.mName = "Hightech";
    //     // p.mSynthStereoSpread = 0.35f;
    //     p.mMasterGain = DecibelsToLinear(0);
    //     p.mMasterFXGain = DecibelsToLinear(-8);
    //     p.mMasterFXEnable = false;
    //     p.mSynthPresetA = SynthPresetID_Fluvial;
    //     p.mSynthAGain = DecibelsToLinear(-3);
    //     p.mSynthPresetB = SynthPresetID_SynthTrumpetDoubler;
    //     p.mSynthBGain = DecibelsToLinear(-3);
    // }

    // static void InitBellyPerf(PerformancePatch &p) // originally thicc
    // {
    //     p.mName = "BellyCrawl";
    //     // p.mSynthStereoSpread = 0.35f;
    //     p.mMasterGain = DecibelsToLinear(0);
    //     p.mMasterFXGain = DecibelsToLinear(-8);
    //     p.mMasterFXEnable = false;
    //     p.mSynthPresetA = SynthPresetID_Fluvial;
    //     p.mSynthAGain = DecibelsToLinear(-3);
    //     p.mSynthPresetB = SynthPresetID_SynthTrumpetDoubler;
    //     p.mSynthBGain = DecibelsToLinear(-3);
    // }

    // static void InitClarinoidSoloPerf(PerformancePatch &p)
    // {
    //     p.mName = "ClarinoidSolo";
    //     // p.mSynthStereoSpread = 0.35f;
    //     p.mMasterGain = DecibelsToLinear(0);
    //     p.mMasterFXGain = DecibelsToLinear(-8);
    //     p.mMasterFXEnable = true;
    //     p.mSynthPresetA = SynthPresetID_Fluvial;
    //     p.mSynthAGain = DecibelsToLinear(-3);
    //     p.mSynthPresetB = 4; // detuned PWM lead
    //     p.mSynthBGain = DecibelsToLinear(-3);
    //     p.mHarmPreset = HarmPresetID_WorldPeace_Eb; // world peace Eb -> col bass -> quintal -> fusion
    //     p.mHarmEnabled = false;
    //     p.mHarmGain = DecibelsToLinear(-3);
    // }

    // static void InitChameleon(PerformancePatch &p)
    // {
    //     p.mName = "Chameleon";
    //     p.mMasterGain = DecibelsToLinear(0);
    //     p.mMasterFXGain = DecibelsToLinear(-8);
    //     p.mMasterFXEnable = false;
    //     p.mSynthPresetA = 10; // funky
    //     p.mSynthAGain = DecibelsToLinear(-3);
    //     p.mSynthPresetB = 4; // detuned PWM lead
    //     p.mSynthBGain = DecibelsToLinear(-10);
    //     p.mHarmPreset = HarmPresetID_WorldPeace_Db;
    //     p.mHarmEnabled = false;
    //     p.mHarmGain = DecibelsToLinear(-3);
    // }

    // static void InitCaveBouncer(PerformancePatch &p)
    // {
    //     p.mName = "CaveBouncer";
    //     p.mMasterGain = DecibelsToLinear(-2.5f);
    //     p.mMasterFXGain = DecibelsToLinear(-5);
    //     p.mMasterFXEnable = false;
    //     p.mSynthPresetA = SynthPresetID_FunkyCave; // funky
    //     p.mSynthAGain = DecibelsToLinear(-1);
    //     p.mSynthPresetB = SynthPresetID_CinematicTag; // detuned PWM lead
    //     p.mSynthBGain = DecibelsToLinear(-24);
    //     p.mHarmPreset = HarmPresetID_WorldPeace_F;
    //     p.mHarmEnabled = false;
    //     p.mHarmGain = DecibelsToLinear(-12);
    // }

    // static void InitSilkSuspendersPerf(PerformancePatch &p)
    // {
    //     p.mName = "/k Pan Flute";
    //     p.mMasterGain = DecibelsToLinear(-8.0f + 3.3f - 0.5f);
    //     p.mSynthPresetA = SynthPresetID_PanFlute;
    //     p.mSynthAGain = DecibelsToLinear(-4);
    //     p.mSynthPresetB = SynthPresetID_CinematicTag;
    //     p.mSynthBGain = DecibelsToLinear(-7.5f);
    //     p.mMasterFXEnable = false;
    //     p.mMasterFXGain = DecibelsToLinear(-10);
    //     p.mBPM = 110;
    //     p.mHarmEnabled = false;
    //     p.mHarmPreset = HarmPresetID_WorldPeace_F;
    // }

    // // a mellow synthwave style lead
    // static void InitSynthwavePerf(PerformancePatch &p)
    // {
    //     p.mName = "/k Synthwave";
    //     p.mMasterGain = DecibelsToLinear(-5.5f);
    //     p.mSynthStereoSpread = 0.8f;
    //     p.mSynthPresetA = SynthPresetID_CinematicTag;
    //     p.mSynthAGain = DecibelsToLinear(-6);
    //     p.mSynthPresetB = SynthPresetID_CinematicTagAlt;
    //     p.mSynthBGain = DecibelsToLinear(-6);
    //     p.mMasterFXEnable = false;
    //     p.mMasterFXGain = DecibelsToLinear(-10);
    //     p.mHarmEnabled = false;
    //     p.mHarmPreset = HarmPresetID_WorldPeace_Gb;
    //     p.mHarmGain = DecibelsToLinear(-6);
    // }

    // static void InitSyncyPerf(PerformancePatch &p)
    // {
    //     p.mName = "/k Sync";
    //     p.mMasterFXEnable = false;
    //     p.mMasterGain = DecibelsToLinear(-10.0f + 1.4f);
    //     p.mSynthAGain = DecibelsToLinear(-4.5f);
    //     p.mMasterFXGain = DecibelsToLinear(-10);
    // }

    // SPACE CAR (+ cinematic)
    //     - reset transpose
    //     - fluvial
    //     - braker solo
    static void InitSpaceCarPerf(PerformancePatch &p)
    {
        p.mName = "Space Car";
        p.mMasterFXEnable = false;
        p.mPerfDisplayStyle = PerfDisplayStyle::Cube;

        p.mTranspose = 0;
        p.mBPM = 140;

        p.mSynthPresetA = SynthPresetID_Fluvial;
        p.mSynthAGain = DecibelsToLinear(-3);
        p.mSynthPresetB = SynthPresetID_BrakerSolo;
        p.mSynthBGain = DecibelsToLinear(-3);

        p.mHarmPreset = HarmPresetID_Quintal;
        p.mGlobalScaleRef = GlobalScaleRefType::Chosen;
        p.mGlobalScale = Scale{Note::F_, ScaleFlavorIndex::Major};
    }

    // RIPPLE BOOGIE
    //     +3 transp
    //     - funky + fluvial
    static void InitRippleBoogiePerf(PerformancePatch &p)
    {
        p.mName = "Ripple Boogie";
        p.mPerfDisplayStyle = PerfDisplayStyle::CubeTessellated;
        p.mMasterFXEnable = false;
        p.mMasterFXGain = DecibelsToLinear(-2);

        p.mTranspose = 3;
        p.mBPM = 110;

        p.mSynthPresetA = SynthPresetID_Funky;
        p.mSynthAGain = DecibelsToLinear(-3);
        p.mSynthPresetB = SynthPresetID_Fluvial;
        p.mSynthBGain = DecibelsToLinear(-3);

        p.mHarmPreset = HarmPresetID_Quintal;
        p.mGlobalScaleRef = GlobalScaleRefType::Chosen;
        p.mGlobalScale = Scale{Note::D, ScaleFlavorIndex::Minor};
    }

    // CLOUDS AND STARS
    //     - fluvial + pwm
    static void InitCloudsAndStarsPerf(PerformancePatch &p, const char *name, int16_t harmPresetId)
    {
        p.mName = name;
        p.mMasterGain = DecibelsToLinear(+2);
        p.mMasterFXEnable = false;
        p.mBPM = 70;
        p.mBeatsPerBar = 2;
        p.mBeatSubdivisions = 3;
        p.mPerfDisplayStyle = PerfDisplayStyle::Metronome;

        p.mSynthPresetA = SynthPresetID_SupersawSoft;
        p.mSynthAGain = DecibelsToLinear(-6);
        p.mSynthPresetB = SynthPresetID_SupersawSoft;
        p.mSynthBGain = DecibelsToLinear(-6);

        p.mHarmGain = DecibelsToLinear(-9);
        p.mHarmPreset = harmPresetId;

        p.mGlobalScale = Scale{Note::A, ScaleFlavorIndex::MajorPentatonic};
    }

    // FULL SCALE
    //     - fluvial + trumpet
    //     - harm: pent -3 -2
    //     - F major
    static void InitFullScalePerf(PerformancePatch &p)
    {
        p.mName = "Full Scale";
        p.mBPM = 120;
        p.mPerfDisplayStyle = PerfDisplayStyle::Icosahedron;
        p.mMasterFXEnable = false;

        p.mSynthPresetA = SynthPresetID_Fluvial;
        p.mSynthAGain = DecibelsToLinear(-3);
        p.mSynthPresetB = SynthPresetID_Trumpet;
        p.mSynthBGain = DecibelsToLinear(-3);

        p.mHarmPreset = HarmPresetID_FullScale;
        p.mGlobalScaleRef = GlobalScaleRefType::Chosen;
        p.mGlobalScale = Scale{Note::F_, ScaleFlavorIndex::MajorNo4th};

        p.mHarmGain = DecibelsToLinear(-6);
    }

    static void InitWobblyCatPerf(PerformancePatch &p)
    {
        p.mName = "Wobbly Cat";
        p.mBPM = 120;
        p.mMasterFXEnable = false;
        p.mPerfDisplayStyle = PerfDisplayStyle::Tetrahedron;

        p.mSynthPresetA = SynthPresetID_WobblyCat;
        p.mSynthAGain = DecibelsToLinear(-3);
        p.mSynthPresetB = SynthPresetID_WobblyCat;
        p.mSynthBGain = DecibelsToLinear(-3);

        p.mHarmPreset = HarmPresetID_N3N2;
        p.mGlobalScaleRef = GlobalScaleRefType::Chosen;
        p.mGlobalScale = Scale{Note::F_, ScaleFlavorIndex::Major};
    }

    // SILK SUSP PERF
    //     - transpose +12
    //     - pan flute
    //     - trumpet -12
    //     - C major
    static void InitSilkSuspendersPerf(PerformancePatch &p)
    {
        p.mName = "Silk Suspenders";
        p.mBPM = 110;
        p.mMasterFXEnable = false;
        p.mPerfDisplayStyle = PerfDisplayStyle::Geodesic;

        p.mTranspose = 12;

        p.mSynthPresetA = SynthPresetID_PanFlute;
        p.mSynthAGain = DecibelsToLinear(-3);
        p.mSynthPresetB = SynthPresetID_Trumpet;
        p.mSynthBTranspose = -12;
        p.mSynthBGain = DecibelsToLinear(-3);

        p.mGlobalScaleRef = GlobalScaleRefType::Chosen;
        p.mGlobalScale = Scale{Note::C, ScaleFlavorIndex::Major};
    }

    static void InitFMPerf(PerformancePatch &p)
    {
        p.mName = "FM test";
        p.mSynthStereoSpread = 0.7f;

        p.mSynthPresetA = SynthPresetID_FMTest;
        p.mSynthAGain = DecibelsToLinear(-3);

        p.mSynthPresetB = SynthPresetID_FMTest;
        p.mSynthBGain = DecibelsToLinear(-3);
    }

    static void InitSyncPerf(PerformancePatch &p)
    {
        p.mName = "Sync doubler";
        p.mSynthStereoSpread = 0.7f;

        p.mSynthPresetA = SynthPresetID_PulseSync;
        p.mSynthAGain = DecibelsToLinear(-3);

        p.mSynthPresetB = SynthPresetID_PulseSync;
        p.mSynthBGain = DecibelsToLinear(-3);
    }

    static void InitSupersawPerf(PerformancePatch &p)
    {
        p.mName = "Supersaw";
        p.mSynthStereoSpread = 0.7f;

        p.mSynthPresetA = SynthPresetID_Supersaw;
        p.mSynthAGain = DecibelsToLinear(-3);

        p.mSynthPresetB = SynthPresetID_Supersaw;
        p.mSynthBGain = DecibelsToLinear(-3);
    }

    static void InitTubularBellPerf(PerformancePatch &p)
    {
        p.mName = "Tubular Bell";
        p.mSynthStereoSpread = 0.6f;
        p.mDetuneSemis = 0.15f;
        p.mPerfDisplayStyle = PerfDisplayStyle::Geodesic;
        p.mBPM = 112;
        p.mMasterFXEnable = false;

        p.mSynthPresetA = SynthPresetID_TubularBell;
        p.mSynthAGain = DecibelsToLinear(-3);

        p.mSynthPresetB = SynthPresetID_TubularBell;
        p.mSynthBGain = DecibelsToLinear(-3);
    }

    AppSettings()
    {
        size_t i = 1; // 0 = default, no performance

        // SPACE CAR (+ cinematic)
        //     - reset transpose
        //     - fluvial
        //     - braker solo
        InitSpaceCarPerf(mPerformancePatches[i++]);

        // RIPPLE BOOGIE
        //     +3 transp
        //     - funky + fluvial
        InitRippleBoogiePerf(mPerformancePatches[i++]);

        // CLOUDS AND STARS
        //     - fluvial + pwm
        InitCloudsAndStarsPerf(mPerformancePatches[i++], "Clouds as Bass", HarmPresetID_SynthWaveAsBass); // as bass
        // InitCloudsAndStarsPerf(mPerformancePatches[i++], "Clouds-Lead", HarmPresetID_SynthWaveAsMelody); // as lead

        // FULL SCALE
        //     - fluvial + trumpet
        //     - harm: pent -3 -2
        //     - F major
        InitFullScalePerf(mPerformancePatches[i++]);
        InitWobblyCatPerf(mPerformancePatches[i++]);

        // SILK SUSP PERF
        //     - transpose +12
        //     - pan flute
        //     - trumpet -12
        //     - C major
        InitSilkSuspendersPerf(mPerformancePatches[i++]);
        InitTubularBellPerf(mPerformancePatches[i++]);

        InitFMPerf(mPerformancePatches[i++]);
        InitSyncPerf(mPerformancePatches[i++]);
        InitSupersawPerf(mPerformancePatches[i++]);

        // InitSoaringGuitarPerf(mPerformancePatches[i++]);
        // InitCaveBouncer(mPerformancePatches[i++]);

        // InitRoadPerf(mPerformancePatches[i++]);
        // InitSyncyPerf(mPerformancePatches[i++]);
        // InitSynthwavePerf(mPerformancePatches[i++]);
        // InitSilkSuspendersPerf(mPerformancePatches[i++]);
        // InitCrystalPerf(mPerformancePatches[i++]);   // crystal

        // InitChameleon(mPerformancePatches[i++]);
        // InitClarinoidSoloPerf(mPerformancePatches[i++]); //

        // at some point it was decided to disable fx by default for performances,
        // but it's more common to have them on, and during a cheeky restart during live performance, you'll need to
        // adjust other things anyway..
    }
};

static constexpr auto appsettingssize = sizeof(AppSettings);
static constexpr auto appsettingssize67 = sizeof(AppSettings::mHarmSettings);
static constexpr auto appsettingssize55 = sizeof(AppSettings::mPerformancePatches);
static constexpr auto appsettingssize44 = sizeof(AppSettings::mSynthSettings);
static constexpr auto rththth = sizeof(AppSettings::mControlMappings);

} // namespace clarinoid
