
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

static constexpr float ReasonableOscillatorGain = 0.25f;

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
    // Osc1FB,
    // Osc2FB,
    // Osc3FB,
};

EnumItemInfo<ModulationSource> gModulationSourceItems[7] = {
    {ModulationSource::None, "None"}, // Gets special handling. see below.
    {ModulationSource::Breath, "Breath"},
    {ModulationSource::PitchStrip, "PitchBend"},
    {ModulationSource::LFO1, "LFO1"},
    {ModulationSource::LFO2, "LFO2"},
    {ModulationSource::ENV1, "ENV1"},
    {ModulationSource::ENV2, "ENV2"},
    // {ModulationSource::Osc1FB, "Osc1FB"},
    // {ModulationSource::Osc2FB, "Osc2FB"},
    // {ModulationSource::Osc3FB, "Osc3FB"},
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
    // Osc1Volume,
    Osc1PulseWidth,
    Osc2Frequency,
    Osc2PulseWidth,
    // Osc2Volume,
    Osc3Frequency,
    Osc3PulseWidth,
    // Osc3Volume,
};

EnumItemInfo<ModulationDestination> gModulationDestinationItems[7] = {
    {ModulationDestination::None, "None"},
    // {ModulationDestination::VoiceFilterCutoff, "VoiceFilterCutoff"},
    // {ModulationDestination::VoiceFilterQ, "VoiceFilterQ"},
    // {ModulationDestination::VoiceFilterSaturation, "VoiceFilterSaturation"},
    {ModulationDestination::Osc1Frequency, "Osc1Frequency"},
    //{ModulationDestination::Osc1Volume, "Osc1Volume"},
    {ModulationDestination::Osc1PulseWidth, "Osc1PulseWidth"},
    {ModulationDestination::Osc2Frequency, "Osc2Frequency"},
    {ModulationDestination::Osc2PulseWidth, "Osc2PulseWidth"},
    //{ModulationDestination::Osc2Volume, "Osc2Volume"},
    {ModulationDestination::Osc3Frequency, "Osc3Frequency"},
    {ModulationDestination::Osc3PulseWidth, "Osc3PulseWidth"},
    //{ModulationDestination::Osc3Volume, "Osc3Volume"},
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

struct SynthPreset
{
    String mName = "--";
    float mPortamentoTime = 0.005f;
    float mPan = 0;
    float mDelaySend = 0.12f;
    float mVerbSend = 0.12f;

    float mOsc1Gain = 0;
    float mOsc2Gain = ReasonableOscillatorGain;
    float mOsc3Gain = ReasonableOscillatorGain;

    // so frequency is calculated like,
    // frequencyof(midinote + pitchsemis + pitchfine) * multiplier + freqoffset

    float mOsc1FreqMultiplier = 1.0f; // midinotefreq * this
    float mOsc2FreqMultiplier = 1.0f;
    float mOsc3FreqMultiplier = 1.0f;

    float mOsc1FreqOffset = 0.0f; // midinotefreq * this
    float mOsc2FreqOffset = 0.0f;
    float mOsc3FreqOffset = 0.0f;

    int mOsc1PitchSemis = 0; // semis = integral, transposition. want to keep this integral because the menu system is
                             // not so great at being very precise.
    int mOsc2PitchSemis = 0;
    int mOsc3PitchSemis = 0;
    float mOsc1PitchFine = 0; // in semitones, just for detuning
    float mOsc2PitchFine = 0;
    float mOsc3PitchFine = 0;

    OscWaveformShape mOsc1Waveform = OscWaveformShape::VarTriangle; //  // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
    OscWaveformShape mOsc2Waveform = OscWaveformShape::SawSync;     //  // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
    OscWaveformShape mOsc3Waveform = OscWaveformShape::VarTriangle; //  // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
    float mOsc1PulseWidth = 0.5f;
    float mOsc2PulseWidth = 0.5f;
    float mOsc3PulseWidth = 0.5f;

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

    SynthModulationSpec mModulations[SYNTH_MODULATIONS_MAX];

    String ToString(uint8_t index) const {
        return String("") + index + ":" + mName;
    }
};

static constexpr auto synthpatchsize = sizeof(SynthPreset);

struct SynthSettings
{
    SynthPreset mPresets[SYNTH_PRESET_COUNT];

    float mMasterGain = 1.0f;
    float mPitchBendRange = 2.0f;

    float mReverbGain = 0.0f;
    float mReverbDamping = 0.6f;
    float mReverbSize = 0.6f;

    float mDelayMS = 300;
    float mDelayStereoSep = 30;
    float mDelayFeedbackLevel = 0.3f;
    ClarinoidFilterType mDelayFilterType = ClarinoidFilterType::BP_Moog2;
    float mDelayCutoffFrequency = 1000;
    float mDelaySaturation = 0.2f;
    float mDelayQ = 0.1f;

    static void InitClarinoid2Preset(SynthPreset &p,
                                     const char *name,
                                     ClarinoidFilterType filt,
                                     float filterKeyScaling,
                                     float q,
                                     float filterMaxFreq)
    {
        // detuned saw.
        p.mName = name;
        p.mOsc1Gain = 0.0f;
        p.mOsc3Gain = 0.0f;

        p.mOsc1Waveform = OscWaveformShape::SawSync;
        p.mOsc2Waveform = OscWaveformShape::SawSync;
        p.mOsc3Waveform = OscWaveformShape::SawSync;
        p.mOsc2Gain = ReasonableOscillatorGain;
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
        p.mOsc1Gain = 0.0f;
        p.mOsc3Gain = 0.0f;
        p.mSync = false;
        p.mDetune = 0.0f;

        p.mOsc2Gain = ReasonableOscillatorGain;
        p.mOsc2Waveform = shape;
        p.mOsc2PulseWidth = pulseWidth;

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

        p.mOsc1Gain = ReasonableOscillatorGain;
        p.mOsc1Waveform = shape;
        p.mOsc1PulseWidth = pulseWidth;

        p.mOsc2Gain = ReasonableOscillatorGain;
        p.mOsc2Waveform = shape;
        p.mOsc2PulseWidth = 1.0f - pulseWidth;

        p.mOsc3Gain = ReasonableOscillatorGain;
        p.mOsc3Waveform = shape;
        p.mOsc2PulseWidth = pulseWidth;

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

        p.mOsc1Gain = ReasonableOscillatorGain;
        p.mOsc1Waveform = OscWaveformShape::Pulse;
        p.mOsc1PulseWidth = 0.08f;
        p.mOsc1PitchSemis = -5;

        p.mOsc3Gain = 0.0f;

        p.mOsc2Gain = ReasonableOscillatorGain;
        p.mOsc2Waveform = OscWaveformShape::Pulse;
        p.mOsc2PulseWidth = 0.92f;
        p.mOsc2PitchSemis = -12;

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

        p.mOsc1Gain = ReasonableOscillatorGain;
        p.mOsc1Waveform = OscWaveformShape::Pulse;
        p.mOsc1PulseWidth = 0.4f;
        p.mOsc1PitchSemis = -5;

        p.mOsc3Gain = 0.0f;

        p.mOsc2Gain = ReasonableOscillatorGain;
        p.mOsc2Waveform = OscWaveformShape::Pulse;
        p.mOsc2PitchSemis = -12;

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
        p.mOsc1Gain = 0.0f;
        p.mOsc3Gain = 0.0f;

        p.mOsc2Gain = ReasonableOscillatorGain;
        p.mOsc3Gain = ReasonableOscillatorGain;

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
        p.mOsc1Gain = 0.0f;
        p.mOsc3Gain = 0.0f;

        p.mOsc2Waveform = OscWaveformShape::VarTriangle;
        p.mOsc2Gain = ReasonableOscillatorGain;
        p.mOsc2PulseWidth = 0.5f;
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
        p.mOsc1Gain = 0.0f;
        p.mOsc3Gain = 0.0f;

        p.mOsc2Waveform = OscWaveformShape::Pulse;
        p.mOsc2Gain = ReasonableOscillatorGain;
        p.mOsc2PulseWidth = 0.07f;
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
        p.mOsc1Gain = 0.0f;
        p.mOsc3Gain = 0.0f;

        p.mOsc2Waveform = OscWaveformShape::SawSync;
        p.mOsc2Gain = ReasonableOscillatorGain;
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
        p.mOsc1Gain = p.mOsc2Gain;
        p.mOsc1Waveform = OscWaveformShape::Pulse;
        p.mOsc1PulseWidth = 0.5f;
        p.mOsc1PitchSemis = -5;

        p.mOsc2PitchSemis = -12;

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

    static void InitCinematicTagPatch(SynthPreset &p)
    {
        p.mName = "Cinematic Tag";
        p.mSync = false;
        p.mDetune = 0.06f;

        p.mFilterType = ClarinoidFilterType::LP_Moog2;
        p.mFilterMaxFreq = 8750;
        p.mFilterSaturation = 0.20f;
        p.mFilterQ = 0.0f;

        p.mOsc1Waveform = OscWaveformShape::VarTriangle;
        p.mOsc1PulseWidth = 0;
        p.mOsc1PitchFine = 0.02f;
        p.mOsc1Gain = ReasonableOscillatorGain;

        p.mOsc2Waveform = OscWaveformShape::VarTriangle;
        p.mOsc2PulseWidth = 0;
        p.mOsc2PitchFine = 0.02f;
        p.mOsc2Gain = ReasonableOscillatorGain;

        p.mOsc3Waveform = OscWaveformShape::VarTriangle;
        p.mOsc3PulseWidth = 0;
        p.mOsc3PitchFine = -0.08f;
        p.mOsc3Gain = ReasonableOscillatorGain;
    }

    static void InitSpacecarPreset(SynthPreset &p)
    {
        InitBasicLeadPreset("SpaceGuitar", OscWaveformShape::Pulse, 0.40f, p);

        p.mLfo2Rate = 0.7f;

        p.mFilterKeytracking = 0;
        p.mFilterMaxFreq = 12000;
        p.mFilterQ = 0.2f;
        p.mFilterType = ClarinoidFilterType::LP_SEM12;

        // p.mOsc1Gain = p.mOsc2Gain; // set up osc1 but don't enable it yet.
        p.mOsc1Waveform = OscWaveformShape::Pulse;
        p.mOsc1PulseWidth = 0.5f;
        p.mOsc1PitchSemis = -5;

        p.mModulations[0].mSource = ModulationSource::LFO1;
        p.mModulations[0].mDest = ModulationDestination::Osc2PulseWidth;
        p.mModulations[0].mScaleN11 = 0.14f;

        p.mModulations[1].mSource = ModulationSource::LFO2;
        p.mModulations[1].mDest = ModulationDestination::Osc1PulseWidth;
        p.mModulations[1].mScaleN11 = 0.20f;
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

    static void InitPanFlutePreset(SynthPreset &p) {

        InitBasicLeadPreset("Pan Flute", OscWaveformShape::Pulse, 0.50f, p);
        // make osc1 and osc2 equal
        p.mOsc2Gain = p.mOsc1Gain = ReasonableOscillatorGain;
        p.mOsc2Waveform = p.mOsc1Waveform = OscWaveformShape::Pulse;
        p.mDetune = 0.04f;

        p.mEnv1.mDecayMS = 100;

        p.mModulations[1].mSource = ModulationSource::ENV1;
        p.mModulations[1].mDest = ModulationDestination::Osc2Frequency;
        p.mModulations[1].mScaleN11 = 0.05f;
    }

    static void InitFunkyLeadPreset(SynthPreset& p) {
        p.mName = "Funky";
        p.mSync = true;
        p.mSyncMultMax = 4.0f;
        p.mDetune = 0;
        p.mFilterQ = 0.40f;
        p.mFilterMaxFreq = 11500;
        p.mOsc3Waveform = OscWaveformShape::Pulse;
        p.mOsc3PulseWidth = 0.1f;
        p.mOsc3Gain = ReasonableOscillatorGain;
    }

    SynthSettings()
    {
        mPresets[0].mName = "Synccy Lead"; // default.
        mPresets[0].mModulations[0].mScaleN11 = 0.9f;
        mPresets[0].mModulations[0].mSource = ModulationSource::LFO1;
        mPresets[0].mModulations[0].mDest = ModulationDestination::Osc2Frequency;

        mPresets[0].mModulations[1].mScaleN11 = 0.015f;
        mPresets[0].mModulations[1].mSource = ModulationSource::LFO2;
        mPresets[0].mModulations[1].mDest = ModulationDestination::Osc1Frequency;

        size_t i = 1; // 0 = default = sync

        InitBasicLeadPreset("PWM 1", OscWaveformShape::Pulse, 0.40f, mPresets[i]);
        mPresets[i].mModulations[0].mSource = ModulationSource::LFO1;
        mPresets[i].mModulations[0].mDest = ModulationDestination::Osc2PulseWidth;
        mPresets[i].mModulations[0].mScaleN11 = 0.14f;
        ++i;

        InitCrystalFieldsPatch(mPresets[i++]);
        InitCinematicTagPatch(mPresets[i++]);
        InitSpacecarPreset(mPresets[i++]);
        InitBellycrawlPreset(mPresets[i++]);

        InitPanFlutePreset(mPresets[i++]);

        InitFunkyLeadPreset(mPresets[i++]);

        InitBasicLeadPreset("Saw Brass", OscWaveformShape::Pulse, 0.50f, mPresets[i]);
        // make osc1 and osc2 equal
        mPresets[i].mOsc3Gain = mPresets[i].mOsc2Gain = mPresets[i].mOsc1Gain = ReasonableOscillatorGain;
        mPresets[i].mOsc3Waveform = mPresets[i].mOsc2Waveform = mPresets[i].mOsc1Waveform = OscWaveformShape::SawSync;
        mPresets[i].mOsc1Waveform = OscWaveformShape::Pulse;
        mPresets[i].mOsc3FreqMultiplier = 2.0f;
        mPresets[i].mDetune = 0.14f;

        mPresets[i].mEnv1.mDecayMS = 100;

        mPresets[i].mModulations[1].mSource = ModulationSource::ENV1;
        mPresets[i].mModulations[1].mDest = ModulationDestination::Osc2Frequency;
        mPresets[i].mModulations[1].mScaleN11 = 0.16f;
        ++i;

        InitDetunedLeadPreset("Detuned pulse 08", OscWaveformShape::Pulse, 0.08f, mPresets[i++]);

        InitFifthLeadPresetA(mPresets[i++]);
        InitFifthLeadPresetB(mPresets[i++]);


        // harmonizer-friendly patches
        InitDetunedLeadPreset("Harm: Detsaws", OscWaveformShape::SawSync, 0.5f, mPresets[SynthPresetID_HarmDetunedSaws]);
        mPresets[SynthPresetID_HarmDetunedSaws].mOsc1Gain = 0.15f;
        mPresets[SynthPresetID_HarmDetunedSaws].mOsc2Gain = 0.15f;
        mPresets[SynthPresetID_HarmDetunedSaws].mOsc3Gain = 0.15f;
        mPresets[SynthPresetID_HarmDetunedSaws].mFilterQ = 0;
        mPresets[SynthPresetID_HarmDetunedSaws].mFilterType = ClarinoidFilterType::BP_Moog4;
        mPresets[SynthPresetID_HarmDetunedSaws].mFilterMaxFreq = 1800;

        InitHarmSyncLead(mPresets[SynthPresetID_HarmSync]);
        InitHarmTriLead(mPresets[SynthPresetID_HarmTri]);
        InitHarmPulseLead(mPresets[SynthPresetID_HarmPulse]);
        InitHarmSawLead(mPresets[SynthPresetID_HarmSaw]);

    }
};

} // namespace clarinoid
