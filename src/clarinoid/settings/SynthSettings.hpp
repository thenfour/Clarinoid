
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{
static constexpr size_t POLYBLEP_OSC_COUNT = 3;

static constexpr float ReasonableOscillatorGain = 0.25f;
static constexpr float ReasonableOscillatorGainForHarm = 0.19f;

enum class OscWaveformShape
{
    Sine = 0,
    VarTriangle = 1,
    Pulse = 2,
    SawSync = 3 // not available in osc 1 (only 2 & 3)
};

EnumItemInfo<OscWaveformShape> gOscWaveformShapeItems[4] = {
    {OscWaveformShape::Sine, "Sine"},
    {OscWaveformShape::VarTriangle, "VarTriangle"},
    {OscWaveformShape::Pulse, "Pulse"},
    {OscWaveformShape::SawSync, "SawSync"},
};

EnumInfo<OscWaveformShape> gOscWaveformShapeInfo("OscWaveformShape", gOscWaveformShapeItems);

enum class ClarinoidFilterType : uint8_t
{
    LP_OnePole = 0,
    LP_SEM12,
    LP_Diode,
    LP_K35,
    LP_Moog2,
    LP_Moog4,
    BP_Moog2,
    BP_Moog4,

    HP_OnePole, // HP filters are no good for our purpose
    HP_K35,
    HP_Moog2,
    HP_Moog4,
};

EnumItemInfo<ClarinoidFilterType> gClarinoidFilterTypeItems[8] = {
    {ClarinoidFilterType::LP_OnePole, "LP_OnePole"},
    {ClarinoidFilterType::LP_SEM12, "LP_SEM12"},
    {ClarinoidFilterType::LP_Diode, "LP_Diode"},
    {ClarinoidFilterType::LP_K35, "LP_K35"},
    {ClarinoidFilterType::LP_Moog2, "LP_Moog2"},
    {ClarinoidFilterType::LP_Moog4, "LP_Moog4"},
    {ClarinoidFilterType::BP_Moog2, "BP_Moog2"},
    {ClarinoidFilterType::BP_Moog4, "BP_Moog4"},
};

EnumInfo<ClarinoidFilterType> gClarinoidFilterTypeInfo("FilterType", gClarinoidFilterTypeItems);

enum class ModulationSource : uint8_t
{
    None, // Gets special handling. see below.
    Breath,
    PitchStrip,
    LFO1,
    LFO2,
    ENV1,
    ENV2,
    Osc1FB,
    Osc2FB,
    Osc3FB,
};

EnumItemInfo<ModulationSource> gModulationSourceItems[10] = {
    {ModulationSource::None, "None"}, // Gets special handling. see below.
    {ModulationSource::Breath, "Breath"},
    {ModulationSource::PitchStrip, "PitchBend"},
    {ModulationSource::LFO1, "LFO1"},
    {ModulationSource::LFO2, "LFO2"},
    {ModulationSource::ENV1, "ENV1"},
    {ModulationSource::ENV2, "ENV2"},
    {ModulationSource::Osc1FB, "Osc1FB"},
    {ModulationSource::Osc2FB, "Osc2FB"},
    {ModulationSource::Osc3FB, "Osc3FB"},
};
static constexpr size_t ModulationSourceSkip = 1; // 1 for the None value.
static constexpr size_t ModulationSourceViableCount =
    SizeofStaticArray(gModulationSourceItems) -
    ModulationSourceSkip; // -1 because "none" is not actually a valid value.
static constexpr bool ModulationSourceToIndex(ModulationSource s, size_t &i)
{
    i = (size_t)s;
    if (i < ModulationSourceSkip)
        return false;
    i -= ModulationSourceSkip;
    if (i >= ModulationSourceViableCount)
        return false;
    return true;
}
static constexpr size_t ModulationSourceToIndex(ModulationSource s)
{
    size_t i = 0;
    ModulationSourceToIndex(s, i);
    return i;
}

EnumInfo<ModulationSource> gModulationSourceInfo("ModSource", gModulationSourceItems);

enum class ModulationDestination : uint8_t
{
    None, // Gets special handling. see below.
    // VoiceFilterCutoff,
    // VoiceFilterQ,
    // VoiceFilterSaturation,
    Osc1Frequency,
    Osc1Amplitude,
    Osc1PulseWidth,
    Osc1Phase,
    Osc2Frequency,
    Osc2PulseWidth,
    Osc2Phase,
    Osc2Amplitude,
    Osc3Frequency,
    Osc3PulseWidth,
    Osc3Phase,
    Osc3Amplitude,
};

EnumItemInfo<ModulationDestination> gModulationDestinationItems[13] = {
    {ModulationDestination::None, "None"},
    // {ModulationDestination::VoiceFilterCutoff, "VoiceFilterCutoff"},
    // {ModulationDestination::VoiceFilterQ, "VoiceFilterQ"},
    // {ModulationDestination::VoiceFilterSaturation, "VoiceFilterSaturation"},
    {ModulationDestination::Osc1Frequency, "Osc1Frequency"},
    {ModulationDestination::Osc1Amplitude, "Osc1Amplitude"},
    {ModulationDestination::Osc1PulseWidth, "Osc1PulseWidth"},
    {ModulationDestination::Osc1Phase, "Osc1Phase"},
    {ModulationDestination::Osc2Frequency, "Osc2Frequency"},
    {ModulationDestination::Osc2PulseWidth, "Osc2PulseWidth"},
    {ModulationDestination::Osc2Phase, "Osc2Phase"},
    {ModulationDestination::Osc2Amplitude, "Osc2Amplitude"},
    {ModulationDestination::Osc3Frequency, "Osc3Frequency"},
    {ModulationDestination::Osc3PulseWidth, "Osc3PulseWidth"},
    {ModulationDestination::Osc3Phase, "Osc3Phase"},
    {ModulationDestination::Osc3Amplitude, "Osc3Amplitude"},
};
static constexpr size_t ModulationDestinationSkip = 1; // 1 for the None value.
static constexpr size_t ModulationDestinationViableCount =
    SizeofStaticArray(gModulationDestinationItems) -
    ModulationDestinationSkip; // -1 because "none" is not actually a valid value.
static constexpr bool ModulationDestinationToIndex(ModulationDestination d, size_t &i)
{
    i = (size_t)d;
    if (i < ModulationDestinationSkip)
        return false;
    i -= ModulationDestinationSkip;
    if (i >= ModulationDestinationViableCount)
        return false;
    return true;
}
static constexpr size_t ModulationDestinationToIndex(ModulationDestination d)
{
    size_t i = 0;
    ModulationDestinationToIndex(d, i);
    return i;
}

EnumInfo<ModulationDestination> gModulationDestinationInfo("ModDest", gModulationDestinationItems);

struct SynthModulationSpec
{
    ModulationSource mSource = ModulationSource::None;
    ModulationDestination mDest = ModulationDestination::None;
    float mScaleN11 = 0.5f; // -1 to 1

    String ToString() const
    {
        if (mSource == ModulationSource::None)
            return "--";
        if (mDest == ModulationDestination::None)
            return "--";
        return String(gModulationSourceInfo.GetValueString(mSource)) + ">" +
               gModulationDestinationInfo.GetValueString(mDest);
    }
};

struct EnvelopeSpec
{
    float mDelayMS = 0.0f;
    float mAttackMS = 4.0f;
    float mDecayMS = 500.0f;
    float mSustainLevel = 0.0f;
    float mReleaseMS = 100.0f;
};

enum class FMAlgo : uint8_t
{
    c1c2c3_NoFM,        // [1][2][3]
    c1m2c3_FM12_NoFM3,  // [1<2][3]
    m1c2c3_FM21_NoFM3,  // [1>2][3]
    c1c2m3_FM32_NoFM1,  // [1][2<3]
    c1m2c3_FM23_NoFM1,  // [1][2>3]
    c1m2m3_Chain,       // [1<2<3]
    c1m23,              // [1<(2&3)]
    c2m2c3_FM13_Split2, // [1<2][2>3]
};

EnumItemInfo<FMAlgo> gFMAlgoItems[8] = {
    {FMAlgo::c1c2c3_NoFM, "[1][2][3]"},
    {FMAlgo::c1m2c3_FM12_NoFM3, "[1<2][3]"},
    {FMAlgo::m1c2c3_FM21_NoFM3, "[1>2][3]"},
    {FMAlgo::c1c2m3_FM32_NoFM1, "[1][2<3]"},
    {FMAlgo::c1m2c3_FM23_NoFM1, "[1][2>3]"},
    {FMAlgo::c1m2m3_Chain, "[1<2<3]"},
    {FMAlgo::c1m23, "[1<(2+3)]"},
    {FMAlgo::c2m2c3_FM13_Split2, "[1<2][2>3]"},
};
EnumInfo<FMAlgo> gFMAlgoInfo("FMAlgo", gFMAlgoItems);

struct SynthOscillatorSettings
{
    float mGain = 0;
    float mPortamentoTime = 0.0f;
    float mFreqMultiplier = 1.0f; // midinotefreq * this
    float mFreqOffset = 0.0f;

    float mPitchBendRange = 2.0f;
    float mPitchBendSnap = 0; // 0= no snap, 1=total snap

    int mPitchSemis = 0;  // semis = integral, transposition. want to keep this integral because the menu system is
                          // not so great at being very precise.
    float mPitchFine = 0; // in semitones, just for detuning
    float mPan = 0;

    bool mPhaseRestart = false;
    float mPhase01 = 0.0f;

    OscWaveformShape mWaveform = OscWaveformShape::VarTriangle;
    float mPulseWidth = 0.5f;

    float mFMFeedbackGain = 0.0f;
    float mAMMinimumGain = 0.0f; // in order to allow amplitude modulations to be non-zero
};

struct SynthPreset
{
    SynthOscillatorSettings mOsc[POLYBLEP_OSC_COUNT];

    String mName = "--";
    float mPan = 0;
    float mDelaySend = 0.08f;
    float mVerbSend = 0.08f;
    float mStereoSpread = 0.15f;

    EnvelopeSpec mEnv1;
    EnvelopeSpec mEnv2;

    bool mSync = true;
    float mSyncMultMin = 1.4f;
    float mSyncMultMax = 3.0f;

    OscWaveformShape mLfo1Shape = OscWaveformShape::Sine;
    float mLfo1Rate = 0.8f;
    OscWaveformShape mLfo2Shape = OscWaveformShape::Sine;
    float mLfo2Rate = 3.5f;

    float mDetune = 0;

    bool mDCFilterEnabled = true;
    float mDCFilterCutoff = 10.0f;

    ClarinoidFilterType mFilterType = ClarinoidFilterType::LP_Moog4;
    float mFilterQ = 0.02f;
    float mFilterMaxFreq = 16000.0f;
    float mFilterMinFreq = 0.0f;
    float mFilterSaturation = 0.2f;
    float mFilterKeytracking = 0.0f; // 0 = no keytracking affect. 1.0 = full effect applied, -1.0 = negative effect
                                     // applied (low notes get higher freq cutoff)

    FMAlgo mFMAlgo = FMAlgo::c1c2c3_NoFM;
    float mFMStrength = 0.1f;

    SynthModulationSpec mModulations[SYNTH_MODULATIONS_MAX];

    SynthPreset()
    {
        mOsc[0].mGain = 0;
        mOsc[1].mGain = ReasonableOscillatorGain;
        mOsc[2].mGain = ReasonableOscillatorGain;

        mOsc[0].mWaveform = OscWaveformShape::VarTriangle; //  // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
        mOsc[1].mWaveform = OscWaveformShape::SawSync;     //  // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
        mOsc[2].mWaveform = OscWaveformShape::VarTriangle; //  // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
    }

    String ToString(uint8_t index) const
    {
        return String("") + index + ":" + mName;
    }
};

static constexpr auto synthpatchsize = sizeof(SynthPreset);

struct SynthSettings
{
    SynthPreset mPresets[SYNTH_PRESET_COUNT];

    static void InitClarinoid2Preset(SynthPreset &p,
                                     const char *name,
                                     ClarinoidFilterType filt,
                                     float filterKeyScaling,
                                     float q,
                                     float filterMaxFreq)
    {
        // detuned saw.
        p.mName = name;
        p.mOsc[0].mGain = 0.0f;
        p.mOsc[2].mGain = 0.0f;

        p.mOsc[0].mWaveform = OscWaveformShape::SawSync;
        p.mOsc[1].mWaveform = OscWaveformShape::SawSync;
        p.mOsc[2].mWaveform = OscWaveformShape::SawSync;
        p.mOsc[1].mGain = ReasonableOscillatorGain;
        p.mSync = false;
        p.mDetune = 0.0f;

        p.mFilterType = filt;
        p.mFilterMinFreq = 0.0f;
        p.mFilterMaxFreq = filterMaxFreq;
        p.mFilterSaturation = 0;
        p.mFilterQ = q;
        p.mFilterKeytracking = filterKeyScaling;
    }

    static void InitBasicLeadPreset(const char *name, OscWaveformShape shape, float pulseWidth, SynthPreset &p)
    {
        p.mName = name;
        p.mOsc[0].mGain = 0.0f;
        p.mOsc[2].mGain = 0.0f;
        p.mSync = false;
        p.mDetune = 0.0f;

        p.mOsc[1].mGain = ReasonableOscillatorGain;
        p.mOsc[1].mWaveform = shape;
        p.mOsc[1].mPulseWidth = pulseWidth;

        p.mFilterType = ClarinoidFilterType::LP_Moog4;
        p.mFilterMinFreq = 0.0f;
        p.mFilterMaxFreq = 16000;
        p.mFilterSaturation = 0.30f;
        p.mFilterQ = 0.25f;
        p.mFilterKeytracking = 0.0f;
    }

    static void InitDetunedLeadPreset(const char *name, OscWaveformShape shape, float pulseWidth, SynthPreset &p)
    {
        p.mName = name;
        p.mDetune = 0.1f;
        p.mSync = false;

        p.mOsc[0].mGain = ReasonableOscillatorGain;
        p.mOsc[0].mWaveform = shape;
        p.mOsc[0].mPulseWidth = pulseWidth;

        p.mOsc[1].mGain = ReasonableOscillatorGain;
        p.mOsc[1].mWaveform = shape;
        p.mOsc[1].mPulseWidth = 1.0f - pulseWidth;

        p.mOsc[2].mGain = ReasonableOscillatorGain;
        p.mOsc[2].mWaveform = shape;
        p.mOsc[1].mPulseWidth = pulseWidth;

        p.mFilterType = ClarinoidFilterType::LP_Moog4;
        p.mFilterMinFreq = 0.0f;
        p.mFilterMaxFreq = 16000;
        p.mFilterSaturation = 0.30f;
        p.mFilterQ = 0.25f;
        p.mFilterKeytracking = 0.0f;
    }

    static void InitFifthLeadPresetA(SynthPreset &p)
    {
        p.mName = "5th lead A";
        p.mSync = false;
        p.mDetune = 0.0f;

        p.mOsc[0].mGain = ReasonableOscillatorGain;
        p.mOsc[0].mWaveform = OscWaveformShape::Pulse;
        p.mOsc[0].mPulseWidth = 0.08f;
        p.mOsc[0].mPitchSemis = -5;

        p.mOsc[2].mGain = 0.0f;

        p.mOsc[1].mGain = ReasonableOscillatorGain;
        p.mOsc[1].mWaveform = OscWaveformShape::Pulse;
        p.mOsc[1].mPulseWidth = 0.92f;
        p.mOsc[1].mPitchSemis = -12;

        p.mFilterType = ClarinoidFilterType::LP_Moog4;
        p.mFilterMinFreq = 0.0f;
        p.mFilterMaxFreq = 18000;
        p.mFilterSaturation = 0.0f;
        p.mFilterQ = 0.1f;
        p.mFilterKeytracking = 0.0f;
    }

    static void InitFifthLeadPresetB(SynthPreset &p)
    {
        p.mName = "5th lead B";
        p.mSync = false;
        p.mDetune = 0.0f;

        p.mOsc[0].mGain = ReasonableOscillatorGain;
        p.mOsc[0].mWaveform = OscWaveformShape::Pulse;
        p.mOsc[0].mPulseWidth = 0.4f;
        p.mOsc[0].mPitchSemis = -5;

        p.mOsc[2].mGain = 0.0f;

        p.mOsc[1].mGain = ReasonableOscillatorGain;
        p.mOsc[1].mWaveform = OscWaveformShape::Pulse;
        p.mOsc[1].mPitchSemis = -12;

        p.mFilterType = ClarinoidFilterType::LP_Moog4;
        p.mFilterMinFreq = 0.0f;
        p.mFilterMaxFreq = 16000;
        p.mFilterSaturation = 0.0f;
        p.mFilterQ = 0.0f;
        p.mFilterKeytracking = 0.0f;
    }

    static void InitHarmSyncLead(SynthPreset &p)
    {
        p.mName = "Sync for Harm";
        p.mOsc[0].mGain = 0.0f;
        p.mOsc[2].mGain = 0.0f;

        p.mOsc[1].mGain = ReasonableOscillatorGainForHarm;
        p.mOsc[2].mGain = ReasonableOscillatorGainForHarm;

        p.mFilterType = ClarinoidFilterType::LP_Moog4;
        p.mFilterMinFreq = 0.0f;
        p.mFilterMaxFreq = 15000.0f;
        p.mFilterSaturation = 0;
        p.mFilterQ = 0.25f;
        p.mFilterKeytracking = 0.0f;
    }

    static void InitHarmTriLead(SynthPreset &p)
    {
        p.mName = "Tri for Harm";
        p.mOsc[0].mGain = 0.0f;
        p.mOsc[2].mGain = 0.0f;

        p.mOsc[1].mWaveform = OscWaveformShape::VarTriangle;
        p.mOsc[1].mGain = ReasonableOscillatorGainForHarm;
        p.mOsc[1].mPulseWidth = 0.5f;
        p.mSync = false;
        p.mDetune = 0.0f;

        p.mFilterType = ClarinoidFilterType::LP_Moog4;
        p.mFilterMinFreq = 0.0f;
        p.mFilterMaxFreq = 15000.0f;
        p.mFilterSaturation = 0;
        p.mFilterQ = 0.1f;
        p.mFilterKeytracking = 0.0f;

        p.mModulations[0].mDest = ModulationDestination::Osc2Frequency;
        p.mModulations[0].mSource = ModulationSource::LFO2;
        p.mModulations[0].mScaleN11 = 0.02f;
    }

    static void InitHarmPulseLead(SynthPreset &p)
    {
        p.mName = "Pulse for Harm";
        p.mOsc[0].mGain = 0.0f;
        p.mOsc[2].mGain = 0.0f;

        p.mOsc[1].mWaveform = OscWaveformShape::Pulse;
        p.mOsc[1].mGain = ReasonableOscillatorGainForHarm;
        p.mOsc[1].mPulseWidth = 0.07f;
        p.mSync = false;
        p.mDetune = 0.0f;

        p.mFilterType = ClarinoidFilterType::LP_Moog4;
        p.mFilterMinFreq = 0.0f;
        p.mFilterMaxFreq = 15000.0f;
        p.mFilterSaturation = 0;
        p.mFilterQ = 0.1f;
        p.mFilterKeytracking = 0.0f;
    }

    static void InitHarmSawLead(SynthPreset &p)
    {
        p.mName = "Saw for Harm";
        p.mOsc[0].mGain = 0.0f;
        p.mOsc[2].mGain = 0.0f;

        p.mOsc[1].mWaveform = OscWaveformShape::SawSync;
        p.mOsc[1].mGain = ReasonableOscillatorGainForHarm;
        p.mSync = false;
        p.mDetune = 0.0f;

        p.mFilterType = ClarinoidFilterType::LP_Moog4;
        p.mFilterMinFreq = 0.0f;
        p.mFilterMaxFreq = 15000.0f;
        p.mFilterSaturation = 0;
        p.mFilterQ = 0.0f;
        p.mFilterKeytracking = 0.0f;
    }

    static void InitCrystalFieldsPatch(SynthPreset &p)
    {
        InitBasicLeadPreset("CrystalFields", OscWaveformShape::Pulse, 0.40f, p);
        p.mOsc[0].mGain = p.mOsc[1].mGain;
        p.mOsc[0].mWaveform = OscWaveformShape::Pulse;
        p.mOsc[0].mPulseWidth = 0.5f;
        p.mOsc[0].mPitchSemis = -5;

        p.mOsc[1].mPitchSemis = -12;

        p.mLfo2Rate = 0.7f;

        p.mFilterKeytracking = 0;
        p.mFilterMaxFreq = 12000;
        p.mFilterQ = 0.2f;
        p.mFilterType = ClarinoidFilterType::LP_SEM12;

        p.mModulations[0].mSource = ModulationSource::LFO1;
        p.mModulations[0].mDest = ModulationDestination::Osc2PulseWidth;
        p.mModulations[0].mScaleN11 = 0.14f;

        p.mModulations[1].mSource = ModulationSource::LFO2;
        p.mModulations[1].mDest = ModulationDestination::Osc1PulseWidth;
        p.mModulations[1].mScaleN11 = 0.20f;
    }

    static void InitCinematicTagPatch(SynthPreset &p, float gain)
    {
        p.mName = "Cinematic Tag";
        p.mSync = false;
        p.mDetune = 0.06f;

        p.mFilterType = ClarinoidFilterType::LP_Moog2;
        p.mFilterMaxFreq = 8750;
        p.mFilterSaturation = 0.20f;
        p.mFilterQ = 0.0f;

        p.mOsc[0].mWaveform = OscWaveformShape::VarTriangle;
        p.mOsc[0].mPulseWidth = 0;
        p.mOsc[0].mPitchFine = 0.02f;
        p.mOsc[0].mGain = ReasonableOscillatorGain * gain;

        p.mOsc[1].mWaveform = OscWaveformShape::VarTriangle;
        p.mOsc[1].mPulseWidth = 0;
        p.mOsc[1].mPitchFine = 0.02f;
        p.mOsc[1].mGain = ReasonableOscillatorGain * gain;

        p.mOsc[2].mWaveform = OscWaveformShape::VarTriangle;
        p.mOsc[2].mPulseWidth = 0;
        p.mOsc[2].mPitchFine = -0.08f;
        p.mOsc[2].mGain = ReasonableOscillatorGain * gain;
    }

    static void InitBellycrawlPreset(SynthPreset &p)
    {
        // start with defaults
        p.mName = "Bellycrawl";
        p.mModulations[0].mScaleN11 = 0.9f;
        p.mModulations[0].mSource = ModulationSource::LFO1;
        p.mModulations[0].mDest = ModulationDestination::Osc2Frequency;

        p.mModulations[1].mScaleN11 = 0.015f;
        p.mModulations[1].mSource = ModulationSource::LFO2;
        p.mModulations[1].mDest = ModulationDestination::Osc1Frequency;
    }

    static void InitPanFlutePreset(SynthPreset &p, float gain)
    {
        InitBasicLeadPreset("Pan Flute", OscWaveformShape::Pulse, 0.50f, p);
        // make osc1 and osc2 equal
        p.mOsc[1].mGain = p.mOsc[0].mGain = ReasonableOscillatorGain * 0.75f * gain;
        p.mOsc[1].mWaveform = p.mOsc[0].mWaveform = OscWaveformShape::Pulse;
        p.mDetune = 0.04f;

        p.mFilterType = ClarinoidFilterType::BP_Moog4;
        p.mFilterMaxFreq = 7000;
        p.mFilterKeytracking = 1.0f;
        p.mFilterQ = 0.1f;
        p.mFilterSaturation = 0.3f;

        p.mEnv1.mDecayMS = 100;

        p.mModulations[1].mSource = ModulationSource::ENV1;
        p.mModulations[1].mDest = ModulationDestination::Osc2Frequency;
        p.mModulations[1].mScaleN11 = 0.05f;
    }

    static void InitFunkyLeadPreset(SynthPreset &p)
    {
        p.mName = "Funky";
        p.mSync = true;
        p.mSyncMultMax = 4.0f;
        p.mDetune = 0;
        p.mFilterQ = 0.40f;
        p.mFilterMaxFreq = 11500;
        p.mOsc[2].mWaveform = OscWaveformShape::Pulse;
        p.mOsc[2].mPulseWidth = 0.1f;
        p.mOsc[2].mGain = ReasonableOscillatorGain;
    }

    static void InitDetunePWMLead(SynthPreset &p)
    {
        p.mName = "Detune PWM";
        p.mDetune = 0.09f;
        p.mStereoSpread = 0.5f;
        p.mSync = false;

        p.mFilterSaturation = 0.1f;
        p.mFilterQ = 0.12f;
        // lp moog 4 16k

        p.mOsc[0].mWaveform = OscWaveformShape::Pulse;
        p.mOsc[0].mPulseWidth = 0.3f;
        p.mOsc[0].mGain = ReasonableOscillatorGain / 1.5f;

        p.mOsc[1].mWaveform = OscWaveformShape::Pulse;
        p.mOsc[1].mPulseWidth = 0.3f;
        p.mOsc[1].mGain = ReasonableOscillatorGain / 1.5f;

        p.mOsc[2].mWaveform = OscWaveformShape::Pulse;
        p.mOsc[2].mPulseWidth = 0.3f;
        p.mOsc[2].mGain = ReasonableOscillatorGain / 1.5f;

        p.mLfo2Rate = p.mLfo1Rate;

        p.mModulations[0].mDest = ModulationDestination::Osc1PulseWidth;
        p.mModulations[0].mSource = ModulationSource::LFO1;
        p.mModulations[0].mScaleN11 = 0.2f;
        p.mModulations[1].mDest = ModulationDestination::Osc1PulseWidth;
        p.mModulations[1].mSource = ModulationSource::LFO2;
        p.mModulations[1].mScaleN11 = 0.2f;
        p.mModulations[2].mDest = ModulationDestination::Osc1PulseWidth;
        p.mModulations[2].mSource = ModulationSource::LFO1;
        p.mModulations[2].mScaleN11 = 0.2f;

        p.mModulations[3].mDest = ModulationDestination::Osc3Frequency;
        p.mModulations[3].mSource = ModulationSource::Breath;
        p.mModulations[3].mScaleN11 = -0.03f;

        p.mModulations[4].mDest = ModulationDestination::Osc1Frequency;
        p.mModulations[4].mSource = ModulationSource::ENV1;
        p.mModulations[4].mScaleN11 = 0.03f;
    }

    static void InitBassoonoidPreset(SynthPreset &p,
                                     const char *name,
                                     ClarinoidFilterType filt,
                                     float filterKeyScaling,
                                     float q,
                                     float filterMaxFreq)
    {
        p.mName = name;
        p.mSync = false;
        p.mDetune = 0.0f;
        p.mStereoSpread = 0;
        p.mVerbSend = 0;
        p.mDelaySend = 0;

        p.mOsc[0].mWaveform = OscWaveformShape::Sine;
        p.mOsc[1].mWaveform = OscWaveformShape::SawSync;
        p.mOsc[2].mWaveform = OscWaveformShape::SawSync;
        p.mOsc[0].mPulseWidth = 0.0f;
        p.mOsc[1].mPulseWidth = 0.0f;
        p.mOsc[2].mPulseWidth = 0.0f;
        p.mOsc[0].mGain = 1.0f;
        p.mOsc[1].mGain = 1.0f;
        p.mOsc[2].mGain = 1.0f;

        p.mFilterType = filt;
        p.mFilterMinFreq = 0.0f;
        p.mFilterMaxFreq = filterMaxFreq;
        p.mFilterSaturation = 0;
        p.mFilterQ = q;
        p.mFilterKeytracking = filterKeyScaling;
    };

    static void InitCloudsStars(SynthPreset &p)
    {
        p.mName = "Clouds+Stars";
        p.mDetune = 0.09f;
        p.mVerbSend = .50f;
        p.mDelaySend = .50f;
        p.mSync = false;

        p.mLfo2Rate = 2.0f;

        p.mFilterType = ClarinoidFilterType::LP_Moog2;
        p.mFilterSaturation = 0.16f;
        p.mFilterQ = 0.12f;

        p.mOsc[0].mWaveform = OscWaveformShape::VarTriangle;
        p.mOsc[0].mPulseWidth = 0.0f;
        p.mOsc[0].mGain = ReasonableOscillatorGain / 1.5f;

        p.mOsc[1].mWaveform = OscWaveformShape::VarTriangle;
        p.mOsc[1].mPulseWidth = 0.0f;
        p.mOsc[1].mFreqMultiplier = 0.501f;
        p.mOsc[1].mGain = ReasonableOscillatorGain / 1.5f;

        p.mOsc[2].mWaveform = OscWaveformShape::VarTriangle;
        p.mOsc[2].mPulseWidth = 0.0f;
        p.mOsc[2].mGain = ReasonableOscillatorGain / 1.5f;

        p.mModulations[0].mDest = ModulationDestination::Osc1PulseWidth;
        p.mModulations[0].mSource = ModulationSource::LFO1;
        p.mModulations[0].mScaleN11 = 0.2f;
        p.mModulations[1].mDest = ModulationDestination::Osc1PulseWidth;
        p.mModulations[1].mSource = ModulationSource::LFO2;
        p.mModulations[1].mScaleN11 = 0.2f;
        p.mModulations[2].mDest = ModulationDestination::Osc1PulseWidth;
        p.mModulations[2].mSource = ModulationSource::LFO1;
        p.mModulations[2].mScaleN11 = 0.2f;

        p.mModulations[3].mDest = ModulationDestination::Osc3Frequency;
        p.mModulations[3].mSource = ModulationSource::Breath;
        p.mModulations[3].mScaleN11 = -0.03f;

        p.mModulations[4].mDest = ModulationDestination::Osc1Frequency;
        p.mModulations[4].mSource = ModulationSource::ENV1;
        p.mModulations[4].mScaleN11 = 0.03f;
    }

    static void InitSynccyLead(SynthPreset &p)
    {
        p.mName = "Synccy Lead"; // default.
        p.mModulations[0].mScaleN11 = 0.9f;
        p.mModulations[0].mSource = ModulationSource::LFO1;
        p.mModulations[0].mDest = ModulationDestination::Osc2Frequency;

        p.mModulations[1].mScaleN11 = 0.015f;
        p.mModulations[1].mSource = ModulationSource::LFO2;
        p.mModulations[1].mDest = ModulationDestination::Osc1Frequency;
    }

    static void InitPWMLead2(SynthPreset &p)
    {
        InitBasicLeadPreset("PWM Mono Lead", OscWaveformShape::Pulse, 0.50f, p);
        p.mFilterMaxFreq = 12000;
        p.mFilterType = ClarinoidFilterType::LP_SEM12;
        p.mModulations[0].mSource = ModulationSource::LFO1;
        p.mModulations[0].mDest = ModulationDestination::Osc2PulseWidth;
        p.mModulations[0].mScaleN11 = 0.20f;
        p.mModulations[1].mSource = ModulationSource::Breath;
        p.mModulations[1].mDest = ModulationDestination::Osc2PulseWidth;
        p.mModulations[1].mScaleN11 = 0.20f;
    }

    static void InitPWMLeadStack(SynthPreset &p)
    {
        InitBasicLeadPreset("PWM Lead Stack", OscWaveformShape::Pulse, 0.50f, p);
        p.mFilterMaxFreq = 12000;
        p.mFilterType = ClarinoidFilterType::LP_SEM12;
        p.mLfo2Rate = 2.0f;
        p.mOsc[2].mFreqMultiplier = 4.0f;
        p.mOsc[2].mWaveform = OscWaveformShape::Pulse;
        p.mOsc[2].mPulseWidth = 0.5f;
        p.mOsc[2].mGain = .15f;
        p.mModulations[0].mSource = ModulationSource::LFO1;
        p.mModulations[0].mDest = ModulationDestination::Osc2PulseWidth;
        p.mModulations[0].mScaleN11 = 0.20f;
        p.mModulations[1].mSource = ModulationSource::Breath;
        p.mModulations[1].mDest = ModulationDestination::Osc2PulseWidth;
        p.mModulations[1].mScaleN11 = 0.20f;
        p.mModulations[2].mSource = ModulationSource::LFO2;
        p.mModulations[2].mDest = ModulationDestination::Osc3PulseWidth;
        p.mModulations[2].mScaleN11 = -0.08f;
        p.mModulations[3].mSource = ModulationSource::Breath;
        p.mModulations[3].mDest = ModulationDestination::Osc3Frequency;
        p.mModulations[3].mScaleN11 = 0.04f;
    }

    static void InitFluvial(SynthPreset &p, float gain)
    {
        p.mName = "Fluvial";
        p.mOsc[0].mGain = 0.0f;
        p.mSync = true;
        p.mSyncMultMin = 0.15f;
        p.mSyncMultMax = 1.95f;
        p.mDetune = 0.0f;
        p.mVerbSend = 0.1f;
        p.mDelaySend = 0.1f;

        p.mOsc[1].mGain = 0.15f * gain;
        p.mOsc[1].mWaveform = OscWaveformShape::SawSync;

        p.mOsc[2].mGain = 0.15f * gain;
        p.mOsc[2].mPulseWidth = 0.5f;
        p.mOsc[2].mWaveform = OscWaveformShape::Pulse;

        p.mFilterType = ClarinoidFilterType::LP_K35;
        p.mFilterMinFreq = 0.0f;
        p.mFilterMaxFreq = 22000;
        p.mFilterSaturation = 0.60f;
        p.mFilterQ = 0.25f;
        p.mFilterKeytracking = 1.0f;

        p.mModulations[0].mSource = ModulationSource::Breath;
        p.mModulations[0].mDest = ModulationDestination::Osc3Frequency;
        p.mModulations[0].mScaleN11 = -0.02f;
        p.mModulations[1].mSource = ModulationSource::Breath;
        p.mModulations[1].mDest = ModulationDestination::Osc3PulseWidth;
        p.mModulations[1].mScaleN11 = -0.3f;
    }

    static void InitSynthTrumpetPreset(SynthPreset &p, float gain)
    {
        p.mName = "Trumpet";
        p.mSync = false;
        p.mStereoSpread = 0.15f;
        p.mDetune = 0.04f;
        p.mVerbSend = 0.07f;
        p.mDelaySend = 0.07f;

        p.mFilterType = ClarinoidFilterType::LP_Moog2;
        p.mFilterMinFreq = 0.0f;
        p.mFilterMaxFreq = 15000;
        p.mFilterSaturation = 0.80f;
        p.mFilterQ = 0.15f;
        p.mFilterKeytracking = 0;

        p.mOsc[0].mGain = p.mOsc[1].mGain = p.mOsc[2].mGain =
            (ReasonableOscillatorGain / 2.5f) * DecibelsToLinear(gain);
        p.mOsc[0].mWaveform = p.mOsc[1].mWaveform = p.mOsc[2].mWaveform = OscWaveformShape::VarTriangle;
        p.mOsc[0].mPulseWidth = p.mOsc[1].mPulseWidth = p.mOsc[2].mPulseWidth = 0.05f;

        p.mModulations[0].mSource = ModulationSource::ENV1;
        p.mModulations[0].mDest = ModulationDestination::Osc1Frequency;
        p.mModulations[0].mScaleN11 = 0.05f;

        p.mModulations[1].mSource = ModulationSource::ENV2;
        p.mModulations[1].mDest = ModulationDestination::Osc3Frequency;
        p.mModulations[1].mScaleN11 = -0.02f;

        p.mEnv1.mDelayMS = 0;
        p.mEnv1.mAttackMS = 0;
        p.mEnv1.mDecayMS = 100;
        p.mEnv1.mSustainLevel = 0;
        p.mEnv1.mReleaseMS = 100;

        p.mEnv2.mDelayMS = 0;
        p.mEnv2.mAttackMS = 0;
        p.mEnv2.mDecayMS = 500;
        p.mEnv2.mSustainLevel = 0;
        p.mEnv2.mReleaseMS = 500;
    }

    // static void InitFMPreset(SynthPreset &p)
    // {
    //     p.mName = "FM test";
    //     p.mSync = false;
    //     p.mDetune = 0.0f;
    //     p.mVerbSend = 0.07f;
    //     p.mDelaySend = 0.07f;
    //     p.mFilterType = ClarinoidFilterType::LP_Moog2;
    //     p.mFilterMinFreq = 0.0f;
    //     p.mFilterMaxFreq = 15000;
    //     p.mFilterSaturation = 0.0f;
    //     p.mFilterQ = 0.15f;
    //     p.mFilterKeytracking = 0;

    //     p.mOsc[0].mGain = p.mOsc[1].mGain = DecibelsToLinear(-6.0f);
    //     p.mOsc[2].mGain = 0;
    //     p.mOsc[0].mWaveform = p.mOsc[1].mWaveform = p.mOsc[2].mWaveform = OscWaveformShape::Sine;
    // }

    SynthSettings()
    {
        size_t i = 0;

        // InitFMPreset(mPresets[i++]);
        InitFluvial(mPresets[i++], 1.0f);
        InitSynccyLead(mPresets[i++]);
        InitPWMLead2(mPresets[i++]);
        InitPWMLeadStack(mPresets[i++]);
        InitDetunePWMLead(mPresets[i++]);
        InitCloudsStars(mPresets[i++]);
        InitCrystalFieldsPatch(mPresets[i++]);
        InitCinematicTagPatch(mPresets[i++], 1.0f);
        InitPanFlutePreset(mPresets[i++], 1.0f);
        InitSynthTrumpetPreset(mPresets[i++], 0);
        InitFunkyLeadPreset(mPresets[i++]);
        InitDetunedLeadPreset("Detuned pulse 08", OscWaveformShape::Pulse, 0.08f, mPresets[i++]);

        InitFifthLeadPresetA(mPresets[i++]);
        InitFifthLeadPresetB(mPresets[i++]);

        // harmonizer-friendly patches

        i = SynthPresetID_MoogBass;
        InitBasicLeadPreset("Moog bass", OscWaveformShape::Pulse, 0.50f, mPresets[i]);
        // make osc1 and osc2 equal
        mPresets[i].mOsc[2].mGain = mPresets[i].mOsc[1].mGain = mPresets[i].mOsc[0].mGain = ReasonableOscillatorGain;
        mPresets[i].mOsc[2].mWaveform = mPresets[i].mOsc[1].mWaveform = mPresets[i].mOsc[0].mWaveform =
            OscWaveformShape::SawSync;
        mPresets[i].mOsc[0].mWaveform = OscWaveformShape::Pulse;
        mPresets[i].mOsc[0].mFreqMultiplier = .5f;
        mPresets[i].mOsc[1].mFreqMultiplier = .5f;
        mPresets[i].mOsc[2].mFreqMultiplier = 1.0f;
        mPresets[i].mDetune = 0.14f;

        mPresets[i].mEnv1.mDecayMS = 100;

        mPresets[i].mModulations[1].mSource = ModulationSource::ENV1;
        mPresets[i].mModulations[1].mDest = ModulationDestination::Osc2Frequency;
        mPresets[i].mModulations[1].mScaleN11 = 0.16f;
        ++i;

        InitDetunedLeadPreset(
            "Harm: Detsaws", OscWaveformShape::SawSync, 0.5f, mPresets[SynthPresetID_HarmDetunedSaws]);
        mPresets[SynthPresetID_HarmDetunedSaws].mOsc[0].mGain = ReasonableOscillatorGainForHarm;
        mPresets[SynthPresetID_HarmDetunedSaws].mOsc[1].mGain = ReasonableOscillatorGainForHarm;
        mPresets[SynthPresetID_HarmDetunedSaws].mOsc[2].mGain = ReasonableOscillatorGainForHarm;
        mPresets[SynthPresetID_HarmDetunedSaws].mFilterQ = 0;
        mPresets[SynthPresetID_HarmDetunedSaws].mFilterType = ClarinoidFilterType::BP_Moog4;
        mPresets[SynthPresetID_HarmDetunedSaws].mFilterMaxFreq = 1800;

        InitBassoonoidPreset(
            mPresets[SynthPresetID_Bassoonoid], "Diode-ks7-q15", ClarinoidFilterType::LP_Diode, 0.7f, 0.15f, 15000);

        InitCinematicTagPatch(mPresets[SynthPresetID_CinematicTag], DecibelsToLinear(-6.04f));
        InitFluvial(mPresets[SynthPresetID_Fluvial], DecibelsToLinear(-6.04f));
        InitHarmSyncLead(mPresets[SynthPresetID_HarmSync]);
        InitHarmTriLead(mPresets[SynthPresetID_HarmTri]);
        InitHarmPulseLead(mPresets[SynthPresetID_HarmPulse]);
        InitHarmSawLead(mPresets[SynthPresetID_HarmSaw]);
        InitSynthTrumpetPreset(mPresets[SynthPresetID_SynthTrumpetDoubler], DecibelsToLinear(-6.04f));
        InitPanFlutePreset(mPresets[SynthPresetID_PanFlute], DecibelsToLinear(-6.04f));
    }
};

} // namespace clarinoid
