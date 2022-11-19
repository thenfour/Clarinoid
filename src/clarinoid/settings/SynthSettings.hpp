
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{
static constexpr size_t POLYBLEP_OSC_COUNT = 3;
static constexpr size_t ENVELOPE_COUNT = 3;
static constexpr size_t LFO_COUNT = 3;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// some notes about the selection of waveforms:
// - Sync is available in all oscillators.
//   - we want it to be prominent in settings.
//   - it's something that can be added to any oscillator
// - Then, sawsync doesn't really feel necessary because of vartriangle doubling as a saw, and also supports sync.
//   so that's a bit weird but we'll take it. I'll just label it "HQ saw".
// - We need 2 "shaping" params. We could probably get away with only 1, but we have GUI space for 2, and it allows
//   more experimenting (curve? morph?) and room for future things
enum class OscWaveformShape : uint8_t
{
    //                                  param1       param2
    //                                 --------     --------
    Sine = 0,        // [todo:sin-tri]  fb amount    curve
    VarTriangle = 1, // [aka Saw-Tri]   shape
    Pulse = 2,       //                 pulsewidth
    SawSync = 3,     //                 ??           curve
    // smooth square (for FM)
};

EnumItemInfo<OscWaveformShape> gOscWaveformShapeItems[4] = {
    {OscWaveformShape::Sine, "Sine", "sin"},
    {OscWaveformShape::VarTriangle, "Tri-Saw", "tri"},
    {OscWaveformShape::Pulse, "Pulse", "sq"},
    {OscWaveformShape::SawSync, "HQ Sawtooth", "saw"},
};

EnumInfo<OscWaveformShape> gOscWaveformShapeInfo("OscWaveformShape", gOscWaveformShapeItems);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class ClarinoidFilterType : uint8_t
{
    Disabled = 0,
    LP_OnePole,
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

EnumItemInfo<ClarinoidFilterType> gClarinoidFilterTypeItems[13] = {
    {ClarinoidFilterType::Disabled, "Disabled", "nil"},
    {ClarinoidFilterType::LP_OnePole, "LP_OnePole", "lp1"},
    {ClarinoidFilterType::LP_SEM12, "LP_SEM12", "lpsem"},
    {ClarinoidFilterType::LP_Diode, "LP_Diode", "lpd"},
    {ClarinoidFilterType::LP_K35, "LP_K35", "lpk35"},
    {ClarinoidFilterType::LP_Moog2, "LP_Moog2", "lpm2"},
    {ClarinoidFilterType::LP_Moog4, "LP_Moog4", "lpm4"},
    {ClarinoidFilterType::BP_Moog2, "BP_Moog2", "bpm2"},
    {ClarinoidFilterType::BP_Moog4, "BP_Moog4", "bpm4"},
    {ClarinoidFilterType::HP_OnePole, "HP_OnePole", "hp1"},
    {ClarinoidFilterType::HP_K35, "HP_K35", "hpk35"},
    {ClarinoidFilterType::HP_Moog2, "HP_Moog2", "hpm2"},
    {ClarinoidFilterType::HP_Moog4, "HP_Moog4", "hpm4"},
    // butterworth 4
    // butterworth 8
    // notches
};

EnumInfo<ClarinoidFilterType> gClarinoidFilterTypeInfo("FilterType", gClarinoidFilterTypeItems);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class ARateModulationSource : uint8_t
{
    // NB: IF you change something here, keep ModulationMatrixNode in sync. (ModulationSourceInfo)
    // these are INDICES used by synthvoice / modulationmatrix. MUST be 0-based, sequential, index-like.
    LFO1 = 0, // a-rate
    LFO2,     // a-rate
    LFO3,     // a-rate
    ENV1,     // a-rate
    ENV2,     // a-rate
    ENV3,     // a-rate
    Osc1FB,   // a-rate
    Osc2FB,   // a-rate
    Osc3FB,   // a-rate
};

EnumItemInfo<ARateModulationSource> gARateModulationSourceItems[9] = {
    {ARateModulationSource::LFO1, "LFO1"},
    {ARateModulationSource::LFO2, "LFO2"},
    {ARateModulationSource::LFO3, "LFO3"},
    {ARateModulationSource::ENV1, "ENV1"},
    {ARateModulationSource::ENV2, "ENV2"},
    {ARateModulationSource::ENV3, "ENV3"},
    {ARateModulationSource::Osc1FB, "Osc1FB"},
    {ARateModulationSource::Osc2FB, "Osc2FB"},
    {ARateModulationSource::Osc3FB, "Osc3FB"},
};
static constexpr size_t gARateModulationSourceCount = SizeofStaticArray(gARateModulationSourceItems);

EnumInfo<ARateModulationSource> gARateModulationSourceInfo("ARateModSource", gARateModulationSourceItems);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class KRateModulationSource : uint8_t
{
    // NB: IF you change something here, keep ModulationMatrixNode in sync. (ModulationSourceInfo)
    // these are INDICES used by synthvoice / modulationmatrix. MUST be 0-based, sequential, index-like.
    Breath = 0,    // K-rate
    PitchStrip,    // K-rate
    Velocity,      // K-rate
    NoteValue,     // K-rate
    RandomTrigger, // K-rate
    ModWheel,      // K-rate
    Macro1,        // K-rate
    Macro2,        // K-rate
    Macro3,        // K-rate
    Macro4,        // K-rate
    Pedal,         // K-rate
};

EnumItemInfo<KRateModulationSource> gKRateModulationSourceItems[11] = {
    {KRateModulationSource::Breath, "Breath", "br"},
    {KRateModulationSource::PitchStrip, "PitchBend", "pb"},
    {KRateModulationSource::Velocity, "Velocity", "vel"},
    {KRateModulationSource::NoteValue, "NoteValue", "note"},
    {KRateModulationSource::RandomTrigger, "RandomTrigger", "rng"},
    {KRateModulationSource::ModWheel, "ModWheel", "mod"},
    {KRateModulationSource::Macro1, "Macro1", "M1"},
    {KRateModulationSource::Macro2, "Macro2", "M2"},
    {KRateModulationSource::Macro3, "Macro3", "M3"},
    {KRateModulationSource::Macro4, "Macro4", "M4"},
    {KRateModulationSource::Pedal, "Pedal", "ped"},
};
static constexpr size_t gKRateModulationSourceCount = SizeofStaticArray(gKRateModulationSourceItems);

EnumInfo<KRateModulationSource> gKRateModulationSourceInfo("KRateModSource", gKRateModulationSourceItems);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class AnyModulationSource : uint8_t
{
    None = 0,
    LFO1,   // a-rate
    LFO2,   // a-rate
    LFO3,   // a-rate
    ENV1,   // a-rate
    ENV2,   // a-rate
    ENV3,   // a-rate
    Osc1FB, // a-rate
    Osc2FB, // a-rate
    Osc3FB, // a-rate

    Breath,        // K-rate
    PitchStrip,    // K-rate
    Velocity,      // K-rate
    NoteValue,     // K-rate
    RandomTrigger, // K-rate
    ModWheel,      // K-rate
    Macro1,        // K-rate
    Macro2,        // K-rate
    Macro3,        // K-rate
    Macro4,        // K-rate
    Pedal,         // K-rate
};

EnumItemInfo<AnyModulationSource> gAnyModulationSourceItems[21] = {
    {AnyModulationSource::None, "None", "-"},
    {AnyModulationSource::LFO1, "LFO1"},
    {AnyModulationSource::LFO2, "LFO2"},
    {AnyModulationSource::LFO3, "LFO3"},
    {AnyModulationSource::ENV1, "ENV1"},
    {AnyModulationSource::ENV2, "ENV2"},
    {AnyModulationSource::ENV3, "ENV3"},
    {AnyModulationSource::Osc1FB, "Osc1FB"},
    {AnyModulationSource::Osc2FB, "Osc2FB"},
    {AnyModulationSource::Osc3FB, "Osc3FB"},
    {AnyModulationSource::Breath, "Breath", "br"},
    {AnyModulationSource::PitchStrip, "PitchBend", "pb"},
    {AnyModulationSource::Velocity, "Velocity", "vel"},
    {AnyModulationSource::NoteValue, "NoteValue", "note"},
    {AnyModulationSource::RandomTrigger, "RandomTrigger", "rnd"}, // todo: random voice, random etc
    {AnyModulationSource::ModWheel, "ModWheel", "mod"},
    {AnyModulationSource::Macro1, "Macro1", "m1"},
    {AnyModulationSource::Macro2, "Macro2", "m2"},
    {AnyModulationSource::Macro3, "Macro3", "m3"},
    {AnyModulationSource::Macro4, "Macro4", "m4"},
    {AnyModulationSource::Pedal, "Pedal"},
};

static constexpr size_t gAnyModulationSourceCount = SizeofStaticArray(gAnyModulationSourceItems);

EnumInfo<AnyModulationSource> gAnyModulationSourceInfo("AnyModSource", gAnyModulationSourceItems);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class ARateModulationDestination : uint8_t
{
    // these are INDICES used by synthvoice / modulationmatrix
    Osc1PulseWidth = 0, // a-rate
    Osc1Phase,          // a-rate
    Osc2PulseWidth,     // a-rate
    Osc2Phase,          // a-rate
    Osc3PulseWidth,     // a-rate
    Osc3Phase,          // a-rate
};

EnumItemInfo<ARateModulationDestination> gARateModulationDestinationItems[6] = {
    {ARateModulationDestination::Osc1PulseWidth, "Osc1PulseWidth", "osc1pw"},
    {ARateModulationDestination::Osc1Phase, "Osc1Phase", "osc1ph"},
    {ARateModulationDestination::Osc2PulseWidth, "Osc2PulseWidth", "osc2pw"},
    {ARateModulationDestination::Osc2Phase, "Osc2Phase", "osc2ph"},
    {ARateModulationDestination::Osc3PulseWidth, "Osc3PulseWidth", "osc3pw"},
    {ARateModulationDestination::Osc3Phase, "Osc3Phase", "osc3ph"},
};

static constexpr size_t gARateModulationDestinationCount = SizeofStaticArray(gARateModulationDestinationItems);

EnumInfo<ARateModulationDestination> gARateModulationDestinationInfo("ARateModDest", gARateModulationDestinationItems);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class KRateModulationDestination : uint8_t
{
    // these are INDICES used by synthvoice / modulationmatrix
    FilterCutoff = 0, // k-rate
    MasterVolume,
    Detune,

    Osc1Volume, // k-rate
    Osc2Volume, // k-rate
    Osc3Volume, // k-rate

    Osc1RingModAmt, // k-rate
    Osc2RingModAmt, // k-rate
    Osc3RingModAmt, // k-rate

    Osc1FMFeedback, // k-rate
    Osc2FMFeedback, // k-rate
    Osc3FMFeedback, // k-rate

    OverallFMStrength, // k-rate
    FMStrength2To1,    // k-rate
    FMStrength3To1,    // k-rate
    FMStrength1To2,    // k-rate
    FMStrength3To2,    // k-rate
    FMStrength1To3,    // k-rate
    FMStrength2To3,    // k-rate

    Osc1FrequencyParam, // k-rate
    Osc2FrequencyParam, // k-rate
    Osc3FrequencyParam, // k-rate

    Osc1SyncFrequencyParam, // k-rate
    Osc2SyncFrequencyParam, // k-rate
    Osc3SyncFrequencyParam, // k-rate

    Env1DelayTime,    // k-rate
    Env1AttackTime,   // k-rate
    Env1AttackCurve,  // k-rate
    Env1HoldTime,     // k-rate
    Env1DecayTime,    // k-rate
    Env1DecayCurve,   // k-rate
    Env1SustainLevel, // k-rate
    Env1ReleaseTime,  // k-rate
    Env1ReleaseCurve, // k-rate

    Env2DelayTime,    // k-rate
    Env2AttackTime,   // k-rate
    Env2AttackCurve,  // k-rate
    Env2HoldTime,     // k-rate
    Env2DecayTime,    // k-rate
    Env2DecayCurve,   // k-rate
    Env2SustainLevel, // k-rate
    Env2ReleaseTime,  // k-rate
    Env2ReleaseCurve, // k-rate

    Env3DelayTime,    // k-rate
    Env3AttackTime,   // k-rate
    Env3AttackCurve,  // k-rate
    Env3HoldTime,     // k-rate
    Env3DecayTime,    // k-rate
    Env3DecayCurve,   // k-rate
    Env3SustainLevel, // k-rate
    Env3ReleaseTime,  // k-rate
    Env3ReleaseCurve, // k-rate
};

EnumItemInfo<KRateModulationDestination> gKRateModulationDestinationItems[52] = {
    {KRateModulationDestination::FilterCutoff, "FilterCutoff", "flt"},
    {KRateModulationDestination::MasterVolume, "MasterVolume", "mstvol"},
    {KRateModulationDestination::Detune, "Detune", "det"},
    {KRateModulationDestination::Osc1Volume, "Osc1Volume", "osc1vol"},
    {KRateModulationDestination::Osc2Volume, "Osc2Volume", "osc2vol"},
    {KRateModulationDestination::Osc3Volume, "Osc3Volume", "osc3vol"},
    {KRateModulationDestination::Osc1RingModAmt, "Osc1RingModAmt", "Osc1ring"},
    {KRateModulationDestination::Osc2RingModAmt, "Osc2RingModAmt", "Osc2ring"},
    {KRateModulationDestination::Osc3RingModAmt, "Osc3RingModAmt", "Osc3ring"},
    {KRateModulationDestination::Osc1FMFeedback, "Osc1FMFeedback", "Osc1fb"},
    {KRateModulationDestination::Osc2FMFeedback, "Osc2FMFeedback", "Osc2fb"},
    {KRateModulationDestination::Osc3FMFeedback, "Osc3FMFeedback", "Osc3fb"},
    {KRateModulationDestination::OverallFMStrength, "OverallFMStrength", "fmstr"},
    {KRateModulationDestination::FMStrength2To1, "FMStrength2To1", "FM21"},
    {KRateModulationDestination::FMStrength3To1, "FMStrength3To1", "FM31"},
    {KRateModulationDestination::FMStrength1To2, "FMStrength1To2", "FM12"},
    {KRateModulationDestination::FMStrength3To2, "FMStrength3To2", "FM32"},
    {KRateModulationDestination::FMStrength1To3, "FMStrength1To3", "FM13"},
    {KRateModulationDestination::FMStrength2To3, "FMStrength2To3", "FM23"},
    {KRateModulationDestination::Osc1FrequencyParam, "Osc1FrequencyP", "Osc1Freq"},
    {KRateModulationDestination::Osc2FrequencyParam, "Osc2FrequencyP", "Osc2Freq"},
    {KRateModulationDestination::Osc3FrequencyParam, "Osc3FrequencyP", "Osc3Freq"},
    {KRateModulationDestination::Osc1SyncFrequencyParam, "Osc1SyncFrequency", "Osc1SyncFreq"},
    {KRateModulationDestination::Osc2SyncFrequencyParam, "Osc2SyncFrequency", "Osc2SyncFreq"},
    {KRateModulationDestination::Osc3SyncFrequencyParam, "Osc3SyncFrequency", "Osc3SyncFreq"},

    {KRateModulationDestination::Env1DelayTime, "Env1DelayTime", "Env1Dly"},
    {KRateModulationDestination::Env1AttackTime, "Env1AttackTime", "Env1At"},
    {KRateModulationDestination::Env1AttackCurve, "Env1AttackCurve", "Env1Ac"},
    {KRateModulationDestination::Env1HoldTime, "Env1HoldTime", "Env1Ht"},
    {KRateModulationDestination::Env1DecayTime, "Env1DecayTime", "Env1Dt"},
    {KRateModulationDestination::Env1DecayCurve, "Env1DecayCurve", "Env1Dc"},
    {KRateModulationDestination::Env1SustainLevel, "Env1SustainLevel", "Env1S"},
    {KRateModulationDestination::Env1ReleaseTime, "Env1ReleaseTime", "Env1Rt"},
    {KRateModulationDestination::Env1ReleaseCurve, "Env1ReleaseCurve", "Env1Rc"},

    {KRateModulationDestination::Env2DelayTime, "Env2DelayTime", "Env2Dly"},
    {KRateModulationDestination::Env2AttackTime, "Env2AttackTime", "Env2At"},
    {KRateModulationDestination::Env2AttackCurve, "Env2AttackCurve", "Env2Ac"},
    {KRateModulationDestination::Env2HoldTime, "Env2HoldTime", "Env2Ht"},
    {KRateModulationDestination::Env2DecayTime, "Env2DecayTime", "Env2Dt"},
    {KRateModulationDestination::Env2DecayCurve, "Env2DecayCurve", "Env2Dc"},
    {KRateModulationDestination::Env2SustainLevel, "Env2SustainLevel", "Env2S"},
    {KRateModulationDestination::Env2ReleaseTime, "Env2ReleaseTime", "Env2Rt"},
    {KRateModulationDestination::Env2ReleaseCurve, "Env2ReleaseCurve", "Env2Rc"},

    {KRateModulationDestination::Env3DelayTime, "Env3DelayTime", "Env3Dly"},
    {KRateModulationDestination::Env3AttackTime, "Env3AttackTime", "En31At"},
    {KRateModulationDestination::Env3AttackCurve, "Env3AttackCurve", "En31Ac"},
    {KRateModulationDestination::Env3HoldTime, "Env3HoldTime", "En31Ht"},
    {KRateModulationDestination::Env3DecayTime, "Env3DecayTime", "En31Dt"},
    {KRateModulationDestination::Env3DecayCurve, "Env3DecayCurve", "En31Dc"},
    {KRateModulationDestination::Env3SustainLevel, "Env3SustainLevel", "E3v1S"},
    {KRateModulationDestination::Env3ReleaseTime, "Env3ReleaseTime", "En31Rt"},
    {KRateModulationDestination::Env3ReleaseCurve, "Env3ReleaseCurve", "En31Rc"},
};

static constexpr size_t gKRateModulationDestinationCount = SizeofStaticArray(gKRateModulationDestinationItems);

EnumInfo<KRateModulationDestination> gKRateModulationDestinationInfo("KRateModDest", gKRateModulationDestinationItems);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Lets create a way to get modulation enum values by oscillator index
struct OscillatorModulationInfo
{
    KRateModulationDestination KRateDestination_FMFeedback;
    KRateModulationDestination KRateDestination_Frequency;
    KRateModulationDestination KRateDestination_SyncFrequency;
    KRateModulationDestination KRateDestination_Volume;
    KRateModulationDestination KRateDestination_RingModAmt;
};

static OscillatorModulationInfo gModValuesByOscillator[POLYBLEP_OSC_COUNT] = {
    {
        KRateModulationDestination::Osc1FMFeedback,
        KRateModulationDestination::Osc1FrequencyParam,
        KRateModulationDestination::Osc1SyncFrequencyParam,
        KRateModulationDestination::Osc1Volume,
        KRateModulationDestination::Osc1RingModAmt,
    },

    {
        KRateModulationDestination::Osc2FMFeedback,
        KRateModulationDestination::Osc2FrequencyParam,
        KRateModulationDestination::Osc1SyncFrequencyParam,
        KRateModulationDestination::Osc2Volume,
        KRateModulationDestination::Osc2RingModAmt,
    },

    {
        KRateModulationDestination::Osc3FMFeedback,
        KRateModulationDestination::Osc3FrequencyParam,
        KRateModulationDestination::Osc1SyncFrequencyParam,
        KRateModulationDestination::Osc3Volume,
        KRateModulationDestination::Osc3RingModAmt,
    },
};

struct EnvelopeModulationInfo
{
    KRateModulationDestination KRateDestination_DelayTime;
    KRateModulationDestination KRateDestination_AttackTime;
    KRateModulationDestination KRateDestination_AttackCurve;
    KRateModulationDestination KRateDestination_HoldTime;
    KRateModulationDestination KRateDestination_DecayTime;
    KRateModulationDestination KRateDestination_DecayCurve;
    KRateModulationDestination KRateDestination_SustainLevel;
    KRateModulationDestination KRateDestination_ReleaseTime;
    KRateModulationDestination KRateDestination_ReleaseCurve;
};

static EnvelopeModulationInfo gModValuesByEnvelope[ENVELOPE_COUNT] = {
    {KRateModulationDestination::Env1DelayTime,
     KRateModulationDestination::Env1AttackTime,
     KRateModulationDestination::Env1AttackCurve,
     KRateModulationDestination::Env1HoldTime,
     KRateModulationDestination::Env1DecayTime,
     KRateModulationDestination::Env1DecayCurve,
     KRateModulationDestination::Env1SustainLevel,
     KRateModulationDestination::Env1ReleaseTime,
     KRateModulationDestination::Env1ReleaseCurve},

    {KRateModulationDestination::Env2DelayTime,
     KRateModulationDestination::Env2AttackTime,
     KRateModulationDestination::Env2AttackCurve,
     KRateModulationDestination::Env2HoldTime,
     KRateModulationDestination::Env2DecayTime,
     KRateModulationDestination::Env2DecayCurve,
     KRateModulationDestination::Env2SustainLevel,
     KRateModulationDestination::Env2ReleaseTime,
     KRateModulationDestination::Env2ReleaseCurve},

    {KRateModulationDestination::Env3DelayTime,
     KRateModulationDestination::Env3AttackTime,
     KRateModulationDestination::Env3AttackCurve,
     KRateModulationDestination::Env3HoldTime,
     KRateModulationDestination::Env3DecayTime,
     KRateModulationDestination::Env3DecayCurve,
     KRateModulationDestination::Env3SustainLevel,
     KRateModulationDestination::Env3ReleaseTime,
     KRateModulationDestination::Env3ReleaseCurve},
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class AnyModulationDestination : uint8_t
{
    None = 0,
    Osc1PulseWidth, // a-rate
    Osc1Phase,      // a-rate
    Osc2PulseWidth, // a-rate
    Osc2Phase,      // a-rate
    Osc3PulseWidth, // a-rate
    Osc3Phase,      // a-rate

    FilterCutoff,       // k-rate
    MasterVolume,       // k-rate
    Detune,             // k-rate
    Osc1Volume,         // k-rate
    Osc2Volume,         // k-rate
    Osc3Volume,         // k-rate
    Osc1RingModAmt,     // k-rate
    Osc2RingModAmt,     // k-rate
    Osc3RingModAmt,     // k-rate
    Osc1FMFeedback,     // k-rate
    Osc2FMFeedback,     // k-rate
    Osc3FMFeedback,     // k-rate
    OverallFMStrength,  // k-rate
    FMStrength2To1,     // k-rate
    FMStrength3To1,     // k-rate
    FMStrength1To2,     // k-rate
    FMStrength3To2,     // k-rate
    FMStrength1To3,     // k-rate
    FMStrength2To3,     // k-rate
    Osc1FrequencyParam, // k-rate
    Osc2FrequencyParam, // k-rate
    Osc3FrequencyParam, // k-rate

    Osc1SyncFrequencyParam, // k-rate
    Osc2SyncFrequencyParam, // k-rate
    Osc3SyncFrequencyParam, // k-rate

    Env1DelayTime,    // k-rate
    Env1AttackTime,   // k-rate
    Env1AttackCurve,  // k-rate
    Env1HoldTime,     // k-rate
    Env1DecayTime,    // k-rate
    Env1DecayCurve,   // k-rate
    Env1SustainLevel, // k-rate
    Env1ReleaseTime,  // k-rate
    Env1ReleaseCurve, // k-rate

    Env2DelayTime,    // k-rate
    Env2AttackTime,   // k-rate
    Env2AttackCurve,  // k-rate
    Env2HoldTime,     // k-rate
    Env2DecayTime,    // k-rate
    Env2DecayCurve,   // k-rate
    Env2SustainLevel, // k-rate
    Env2ReleaseTime,  // k-rate
    Env2ReleaseCurve, // k-rate

    Env3DelayTime,    // k-rate
    Env3AttackTime,   // k-rate
    Env3AttackCurve,  // k-rate
    Env3HoldTime,     // k-rate
    Env3DecayTime,    // k-rate
    Env3DecayCurve,   // k-rate
    Env3SustainLevel, // k-rate
    Env3ReleaseTime,  // k-rate
    Env3ReleaseCurve, // k-rate

    // patch spread
    // filter saturation
    // filter Q
    // lfo stuff
};

EnumItemInfo<AnyModulationDestination> gAnyModulationDestinationItems[1 /* none */ + gARateModulationDestinationCount +
                                                                      gKRateModulationDestinationCount] = {
    {AnyModulationDestination::None, "None", "-"},
    {AnyModulationDestination::Osc1PulseWidth, "Osc1PulseWidth", "osc1pw"},
    {AnyModulationDestination::Osc1Phase, "Osc1Phase", "osc1ph"},
    {AnyModulationDestination::Osc2PulseWidth, "Osc2PulseWidth", "osc2pw"},
    {AnyModulationDestination::Osc2Phase, "Osc2Phase", "osc2ph"},
    {AnyModulationDestination::Osc3PulseWidth, "Osc3PulseWidth", "osc3pw"},
    {AnyModulationDestination::Osc3Phase, "Osc3Phase", "osc3ph"}, // A-rate

    // K-rates:
    {AnyModulationDestination::FilterCutoff, "FilterCutoff", "flt"},
    {AnyModulationDestination::MasterVolume, "MasterVolume", "mstvol"},
    {AnyModulationDestination::Detune, "Detune", "det"},
    {AnyModulationDestination::Osc1Volume, "Osc1Volume", "osc1vol"},
    {AnyModulationDestination::Osc2Volume, "Osc2Volume", "osc2vol"},
    {AnyModulationDestination::Osc3Volume, "Osc3Volume", "osc3vol"},
    {AnyModulationDestination::Osc1RingModAmt, "Osc1RingModAmt", "Osc1ring"},
    {AnyModulationDestination::Osc2RingModAmt, "Osc2RingModAmt", "Osc2ring"},
    {AnyModulationDestination::Osc3RingModAmt, "Osc3RingModAmt", "Osc3ring"},
    {AnyModulationDestination::Osc1FMFeedback, "Osc1FMFeedback", "Osc1fb"},
    {AnyModulationDestination::Osc2FMFeedback, "Osc2FMFeedback", "Osc2fb"},
    {AnyModulationDestination::Osc3FMFeedback, "Osc3FMFeedback", "Osc3fb"},
    {AnyModulationDestination::OverallFMStrength, "OverallFMStrength", "fmstr"},
    {AnyModulationDestination::FMStrength2To1, "FMStrength2To1", "FM21"},
    {AnyModulationDestination::FMStrength3To1, "FMStrength3To1", "FM31"},
    {AnyModulationDestination::FMStrength1To2, "FMStrength1To2", "FM12"},
    {AnyModulationDestination::FMStrength3To2, "FMStrength3To2", "FM32"},
    {AnyModulationDestination::FMStrength1To3, "FMStrength1To3", "FM13"},
    {AnyModulationDestination::FMStrength2To3, "FMStrength2To3", "FM23"},
    {AnyModulationDestination::Osc1FrequencyParam, "Osc1Frequency", "Osc1Freq"},
    {AnyModulationDestination::Osc2FrequencyParam, "Osc2Frequency", "Osc2Freq"},
    {AnyModulationDestination::Osc3FrequencyParam, "Osc3Frequency", "Osc3Freq"},
    {AnyModulationDestination::Osc1SyncFrequencyParam, "Osc1SyncFrequency", "Osc1SyncFreq"},
    {AnyModulationDestination::Osc2SyncFrequencyParam, "Osc2SyncFrequency", "Osc2SyncFreq"},
    {AnyModulationDestination::Osc3SyncFrequencyParam, "Osc3SyncFrequency", "Osc3SyncFreq"},

    {AnyModulationDestination::Env1DelayTime, "Env1DelayTime", "Env1Dly"},
    {AnyModulationDestination::Env1AttackTime, "Env1AttackTime", "Env1At"},
    {AnyModulationDestination::Env1AttackCurve, "Env1AttackCurve", "Env1Ac"},
    {AnyModulationDestination::Env1HoldTime, "Env1HoldTime", "Env1Ht"},
    {AnyModulationDestination::Env1DecayTime, "Env1DecayTime", "Env1Dt"},
    {AnyModulationDestination::Env1DecayCurve, "Env1DecayCurve", "Env1Dc"},
    {AnyModulationDestination::Env1SustainLevel, "Env1SustainLevel", "Env1S"},
    {AnyModulationDestination::Env1ReleaseTime, "Env1ReleaseTime", "Env1Rt"},
    {AnyModulationDestination::Env1ReleaseCurve, "Env1ReleaseCurve", "Env1Rc"},

    {AnyModulationDestination::Env2DelayTime, "Env2DelayTime", "Env2Dly"},
    {AnyModulationDestination::Env2AttackTime, "Env2AttackTime", "Env2At"},
    {AnyModulationDestination::Env2AttackCurve, "Env2AttackCurve", "Env2Ac"},
    {AnyModulationDestination::Env2HoldTime, "Env2HoldTime", "Env2Ht"},
    {AnyModulationDestination::Env2DecayTime, "Env2DecayTime", "Env2Dt"},
    {AnyModulationDestination::Env2DecayCurve, "Env2DecayCurve", "Env2Dc"},
    {AnyModulationDestination::Env2SustainLevel, "Env2SustainLevel", "Env2S"},
    {AnyModulationDestination::Env2ReleaseTime, "Env2ReleaseTime", "Env2Rt"},
    {AnyModulationDestination::Env2ReleaseCurve, "Env2ReleaseCurve", "Env2Rc"},

    {AnyModulationDestination::Env3DelayTime, "Env3DelayTime", "Env3Dly"},
    {AnyModulationDestination::Env3AttackTime, "Env3AttackTime", "En31At"},
    {AnyModulationDestination::Env3AttackCurve, "Env3AttackCurve", "En31Ac"},
    {AnyModulationDestination::Env3HoldTime, "Env3HoldTime", "En31Ht"},
    {AnyModulationDestination::Env3DecayTime, "Env3DecayTime", "En31Dt"},
    {AnyModulationDestination::Env3DecayCurve, "Env3DecayCurve", "En31Dc"},
    {AnyModulationDestination::Env3SustainLevel, "Env3SustainLevel", "E3v1S"},
    {AnyModulationDestination::Env3ReleaseTime, "Env3ReleaseTime", "En31Rt"},
    {AnyModulationDestination::Env3ReleaseCurve, "Env3ReleaseCurve", "En31Rc"},

    // LFO
};

static constexpr size_t gAnyModulationDestinationCount = SizeofStaticArray(gAnyModulationDestinationItems);

EnumInfo<AnyModulationDestination> gAnyModulationDestinationInfo("AnyModDest", gAnyModulationDestinationItems);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class ModulationPolarityTreatment : uint8_t
{
    AsPositive01,         // force it to 0-1 positive polarity
    AsPositive01Inverted, // same as Positive01, but signal is inverted so 0,1 translates 1,0. (1-x)
    AsBipolar,            // force it to -1,1 bi-polarity
    AsBipolarInverted,    // same as AsBipolar, but signal is inverted so -1,1 translates 1,-1 (-x)
    Default,              // do not change the source signal at all.
};

EnumItemInfo<ModulationPolarityTreatment> gModulationPolarityTreatmentItems[5] = {
    {ModulationPolarityTreatment::AsPositive01, "Map to [0,1]", "01"},
    {ModulationPolarityTreatment::AsPositive01Inverted, "Map to [1,0]", "10"},
    {ModulationPolarityTreatment::AsBipolar, "Map to [-1,1]", "N11"},
    {ModulationPolarityTreatment::AsBipolarInverted, "Map to [1,-1]", "1N1"},
    {ModulationPolarityTreatment::Default, "Default", "-"},
};
static constexpr size_t gModulationPolarityTreatmentCount = SizeofStaticArray(gAnyModulationDestinationItems);

EnumInfo<ModulationPolarityTreatment> gModulationPolarityTreatmentInfo("ModulationPolarity",
                                                                       gModulationPolarityTreatmentItems);

EnumItemInfo<ModulationPolarityTreatment> gModulationAuxPolarityTreatmentItems[2] = {
    {ModulationPolarityTreatment::AsPositive01, "Map to [0,1]", "01"},
    {ModulationPolarityTreatment::AsPositive01Inverted, "Map to [1,0]", "10"},
};

static constexpr size_t gModulationAuxPolarityTreatmentCount = SizeofStaticArray(gModulationAuxPolarityTreatmentItems);

EnumInfo<ModulationPolarityTreatment> gModulationAuxPolarityTreatmentInfo("ModulationAuxPolarity",
                                                                          gModulationAuxPolarityTreatmentItems);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct SynthModulationSpec : ISerializationObjectMap<11>
{
    BoolParam mEnabled{true}; // allows bypassing
    EnumParam<AnyModulationSource> mSource{gAnyModulationSourceInfo, AnyModulationSource::None};
    EnumParam<AnyModulationDestination> mDest{gAnyModulationDestinationInfo, AnyModulationDestination::None};
    EnumParam<ModulationPolarityTreatment> mSourcePolarity{gModulationPolarityTreatmentInfo,
                                                           ModulationPolarityTreatment::Default};
    CurveLUTParamValue mCurveShape{0};
    FloatParam mScaleN11{0.5f};

    EnumParam<AnyModulationSource> mAuxSource{gAnyModulationSourceInfo, AnyModulationSource::None};
    EnumParam<ModulationPolarityTreatment> mAuxPolarity{gModulationPolarityTreatmentInfo,
                                                        ModulationPolarityTreatment::AsPositive01};
    BoolParam mAuxEnabled{true}; // just allows bypassing without removing the aux source
    CurveLUTParamValue mAuxCurveShape{0};
    FloatParam mAuxAmount{0.0f}; // amount of attenuation

    String ToDisplayString() const
    {
        if (mSource.GetValue() == AnyModulationSource::None)
            return "--";
        if (mDest.GetValue() == AnyModulationDestination::None)
            return "--";
        String ret;
        if (!mEnabled.GetValue()) {
            ret = "<mute>";
        }
        return ret + String(gAnyModulationSourceInfo.GetValueDisplayName(mSource.GetValue())) + ">" +
               gAnyModulationDestinationInfo.GetValueDisplayName(mDest.GetValue());
    }

    virtual SerializationObjectMapArray GetSerializationObjectMap()  override
    {
        return {{
            CreateSerializationMapping(mEnabled, "E"),
            CreateSerializationMapping(mSource, "S"),
            CreateSerializationMapping(mDest, "D"),
            CreateSerializationMapping(mSourcePolarity, "P"),
            CreateSerializationMapping(mCurveShape, "C"),
            CreateSerializationMapping(mScaleN11, "A"),
            CreateSerializationMapping(mAuxSource, "s"),
            CreateSerializationMapping(mAuxPolarity, "p"),
            CreateSerializationMapping(mAuxEnabled, "e"),
            CreateSerializationMapping(mAuxCurveShape, "c"),
            CreateSerializationMapping(mAuxAmount, "a"),
        }};
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct EnvelopeSpec : ISerializationObjectMap<10>
{
    EnvTimeParamValue mDelayTime{0.0f};
    EnvTimeParamValue mAttackTime{0.0f};
    CurveLUTParamValue mAttackCurve{0.0f};
    EnvTimeParamValue mHoldTime{0.0f};
    EnvTimeParamValue mDecayTime{0.5f};
    CurveLUTParamValue mDecayCurve{0};
    FloatParam mSustainLevel{0.4f};
    EnvTimeParamValue mReleaseTime{0.2f};
    CurveLUTParamValue mReleaseCurve{0};
    BoolParam mLegatoRestart{false};

    virtual SerializationObjectMapArray GetSerializationObjectMap() override
    {
        return {{
            CreateSerializationMapping(mDelayTime, "Dly"),
            CreateSerializationMapping(mAttackTime, "A"),
            CreateSerializationMapping(mAttackCurve, "Ac"),
            CreateSerializationMapping(mHoldTime, "H"),
            CreateSerializationMapping(mDecayTime, "D"),
            CreateSerializationMapping(mDecayCurve, "Dc"),
            CreateSerializationMapping(mSustainLevel, "S"),
            CreateSerializationMapping(mReleaseTime, "R"),
            CreateSerializationMapping(mReleaseCurve, "Rc"),
            CreateSerializationMapping(mLegatoRestart, "LR"),
        }};
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct SynthOscillatorSettings : ISerializationObjectMap<18>
{
    void CopyFrom(const SynthOscillatorSettings &rhs)
    {
        *this = rhs;
    }

    VolumeParamValue mVolume = VolumeParamValue::FromParamValue(0.0f);
    // this is not redundant with volume, because
    // - it enables quick muting
    // - it helps know how to display things
    // - it helps know how detune / stereo sep will operate.
    BoolParam mEnabled{true};
    IntParam<int> mPortamentoTimeMS{0};

    // for pitch, it's even hard to know which kind of params are needed.
    // 1. for FM, ratio & offset(hz) are absolutely necessary.
    // 2. for sync, param + KT amt are ideal, to behave like filter.
    // 3. for transposition & detune, semis & fine.

    // these are good for FM
    FloatParam mFreqMultiplier{1.0f}; // midinotefreq * this
    FloatParam mFreqOffsetHz{0.0f};

    float mRingModStrengthN11 = 1.0f;
    std::array<BoolParam, POLYBLEP_OSC_COUNT - 1> mRingModOtherOsc;

    // these are good for modulation, and for sync frequency.
    FrequencyParamValue mFreqParam{0.3f, 1.0f}; // param, kt amt

    BoolParam mHardSyncEnabled{false};
    FrequencyParamValue mSyncFreqParam{0.4f, 1.0f}; // param, kt amt

    // these are good for musical stuff like transposition.
    IntParam<int> mPitchSemis{0}; // semis = integral, transposition. want to keep this integral because the menu system
                                  // is not so great at being very precise.
    FloatParam mPitchFine{0};     // in semitones, just for detuning

    FloatParam mPitchBendRangePositive{2.0f};
    FloatParam mPitchBendRangeNegative{-2.0f};

    FloatParam mPan{0};

    BoolParam mPhaseRestart{false};
    FloatParam mPhase01{0};

    EnumParam<OscWaveformShape> mWaveform{gOscWaveformShapeInfo, OscWaveformShape::VarTriangle};
    FloatParam mPulseWidth{0.5f};

    FloatParam mFMFeedbackGain{0.0f}; // 0 to 1

    virtual SerializationObjectMapArray GetSerializationObjectMap() override
    {
        return {{
            CreateSerializationMapping(mEnabled, "on"),
            CreateSerializationMapping(mWaveform, "w"),
            CreateSerializationMapping(mVolume, "v"),
            CreateSerializationMapping(mPortamentoTimeMS, "p"),
            CreateSerializationMapping(mFreqMultiplier, "fM"),
            CreateSerializationMapping(mFreqOffsetHz, "fO"),
            CreateSerializationMapping(mFreqParam, "fP"),
            CreateSerializationMapping(mHardSyncEnabled, "sE"),
            CreateSerializationMapping(mSyncFreqParam, "sF"),
            CreateSerializationMapping(mPitchSemis, "tS"),
            CreateSerializationMapping(mPitchFine, "tF"),
            CreateSerializationMapping(mPitchBendRangePositive, "pbP"),
            CreateSerializationMapping(mPitchBendRangeNegative, "pbN"),
            CreateSerializationMapping(mPan, "Pan"),
            CreateSerializationMapping(mPhaseRestart, "rst"),
            CreateSerializationMapping(mPhase01, "Ph"),
            CreateSerializationMapping(mPulseWidth, "PW"),
            CreateSerializationMapping(mFMFeedbackGain, "Fb"),
        }};
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct LFOSpec : ISerializationObjectMap<3>
{
    EnumParam<OscWaveformShape> mWaveShape{gOscWaveformShapeInfo, OscWaveformShape::Sine};
    FloatParam mPulseWidth{0.5f};
    TimeWithBasisParam mSpeed{TimeBasis::Hertz, 1.0f};

    virtual SerializationObjectMapArray GetSerializationObjectMap() override
    {
        return {{
            CreateSerializationMapping(mWaveShape, "w"),
            CreateSerializationMapping(mPulseWidth, "pw"),
            CreateSerializationMapping(mSpeed, "s"),
        }};
    }

    // Result SerializableObject_ToJSON(JsonVariant doc) const
    // {
    //     Result ret = Result::Success();
    //     ret.AndRequires(mWaveShape.SerializableObject_ToJSON(doc.createNestedObject("wav")), "wav");
    //     ret.AndRequires(mPulseWidth.SerializableObject_ToJSON(doc.createNestedObject("PW")), "PW");
    //     ret.AndRequires(mSpeed.SerializableObject_ToJSON(doc.createNestedObject("speed")), "speed");
    //     return ret;
    // }

    // Result SerializableObject_Deserialize(JsonVariant obj)
    // {
    //     if (!obj.is<JsonObject>())
    //     {
    //         return Result::Failure("expected object");
    //     }
    //     Result ret = Result::Success();

    //     ret.AndRequires(mWaveShape.SerializableObject_Deserialize(obj["wav"]), "wav");
    //     ret.AndRequires(mPulseWidth.SerializableObject_Deserialize(obj["PW"]), "PW");
    //     ret.AndRequires(mSpeed.SerializableObject_Deserialize(obj["speed"]), "speed");

    //     return ret;
    // }

    // BoolParam mPhaseRestart = {"PhaseRestart", false};

    // SerializableObject *mSerializableChildObjects[4] = {
    //     &mWaveShape,
    //     &mPulseWidth,
    //     &mSpeed,
    //     &mPhaseRestart,
    // };

    // const size_t mMyIndex;

    // explicit LFOSpec(size_t myIndex)
    //     : // SerializableDictionary("LFO", mSerializableChildObjects), //
    //       mMyIndex(myIndex)
    // {
    // }

    // when 0, all voices use the same LFO phase
    // when 1, phase is distributed/staggered among voices
    // float mVoicePhaseDistribution01 = 1.0f;

    // when 1, phase offset is "randomized"
    // float mPhaseDistributionRandomization01 = 1.0f;
    // float mRandomSeed01 = 0; // due to float precision, actual seed will snap this so there are only like 100 values.
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class VoicingMode : uint8_t
{
    Monophonic,
    Polyphonic,
};

EnumItemInfo<VoicingMode> gVoicingModeItems[2] = {
    {VoicingMode::Monophonic, "Mono"},
    {VoicingMode::Polyphonic, "Poly"},
};

EnumInfo<VoicingMode> gVoicingModeInfo("VoicingMode", gVoicingModeItems);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct FilterSettings : ISerializationObjectMap<4>
{
    EnumParam<ClarinoidFilterType> mType{gClarinoidFilterTypeInfo, ClarinoidFilterType::LP_Moog4};
    FloatParam mQ{0.02f};
    FloatParam mSaturation{0.2f};
    FrequencyParamValue mFrequency{0.3f, 0.0f}; // param, kt amt

    virtual SerializationObjectMapArray GetSerializationObjectMap() override
    {
        return {{
            CreateSerializationMapping(mType, "t"),
            CreateSerializationMapping(mQ, "Q"),
            CreateSerializationMapping(mSaturation, "S"),
            CreateSerializationMapping(mFrequency, "F"),
        }};
    }

    // Result SerializableObject_ToJSON(JsonVariant rhs) const
    // {
    //     Result ret = Result::Success();
    //     ret.AndRequires(mType.SerializableObject_ToJSON(rhs.createNestedObject("type")), "type");
    //     ret.AndRequires(mQ.SerializableObject_ToJSON(rhs.createNestedObject("q")), "q");
    //     ret.AndRequires(mSaturation.SerializableObject_ToJSON(rhs.createNestedObject("sat")), "sat");
    //     ret.AndRequires(mFrequency.SerializableObject_ToJSON(rhs.createNestedObject("freq")), "freq");
    //     return ret;
    // }

    // Result SerializableObject_Deserialize(JsonVariant obj)
    // {
    //     if (!obj.is<JsonObject>())
    //     {
    //         return Result::Failure("must be object");
    //     }

    //     Result ret = Result::Success();
    //     ret.AndRequires(mType.SerializableObject_Deserialize(obj["type"]), "type");
    //     ret.AndRequires(mQ.SerializableObject_Deserialize(obj["q"]), "q");
    //     ret.AndRequires(mSaturation.SerializableObject_Deserialize(obj["sat"]), "sat");
    //     ret.AndRequires(mFrequency.SerializableObject_Deserialize(obj["freq"]), "freq");
    //     return ret;
    // }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct SynthPatch : ISerializationObjectMap<20>
{
  private:
    SynthPatch &operator=(const SynthPatch &rhs) = default;

  public:
    void CopyFrom(const SynthPatch &rhs)
    {
        size_t myIndex = mMyIndex;
        *this = rhs;
        mMyIndex = myIndex;
    }

    std::array<SynthOscillatorSettings, POLYBLEP_OSC_COUNT> mOsc;

    StringParam mName{"--"};
    VolumeParamValue mMasterVolume;
    FloatParam mPan{0};
    FloatParam mDelayMix{0.08f};
    FloatParam mVerbMix{0.08f};
    FloatParam mStereoSpread{0.15f};

    std::array<EnvelopeSpec, ENVELOPE_COUNT>
        mEnvelopes; //{initialize_array_with_indices<EnvelopeSpec, ENVELOPE_COUNT>()};

    EnumParam<VoicingMode> mVoicingMode{gVoicingModeInfo, VoicingMode::Polyphonic};

    std::array<LFOSpec, LFO_COUNT> mLFOs; //{initialize_array_with_indices<LFOSpec, LFO_COUNT>()};

    FloatParam mDetune{0};

    // BoolParam mDCFilterEnabled{true};
    // FloatParam mDCFilterCutoff{10.0f};

    FilterSettings mFilter;

    // for FM modulation matrix.
    FloatParam mFMStrength2To1{0};
    FloatParam mFMStrength3To1{0};
    FloatParam mFMStrength1To2{0};
    FloatParam mFMStrength3To2{0};
    FloatParam mFMStrength1To3{0};
    FloatParam mFMStrength2To3{0};

    FloatParam mOverallFMStrength{1.00f};

    std::array<SynthModulationSpec, SYNTH_MODULATIONS_MAX> mModulations;

    size_t mMyIndex;
    explicit SynthPatch(size_t myIndex) : mMyIndex(myIndex)
    {
        mLFOs[0].mSpeed.SetFrequency(1.1f);
        mLFOs[1].mSpeed.SetFrequency(3.5f);

        // mOsc[0].mVolume.SetValue(0.4f);
        // mOsc[1].mVolume.SetValue(0);
        // mOsc[2].mVolume.SetValue(0);

        // mOsc[0].mWaveform = OscWaveformShape::VarTriangle;
        // mOsc[1].mWaveform = OscWaveformShape::SawSync;
        // mOsc[2].mWaveform = OscWaveformShape::VarTriangle;
    }

    String ToString() const
    {
        return String(mMyIndex) + ":" + mName.GetValue();
    }

    String OscillatorToString(size_t i) const
    {
        if (mOsc[i].mVolume.IsSilent())
        {
            // any modulations on gain?
            static constexpr AnyModulationDestination oscGainMods[3] = {
                AnyModulationDestination::Osc1Volume,
                AnyModulationDestination::Osc2Volume,
                AnyModulationDestination::Osc3Volume,
            };

            auto dest = oscGainMods[i];
            if (!Any(this->mModulations, [&](const SynthModulationSpec &m) { return m.mDest.GetValue() == dest; }))
            {
                if (!mOsc[i].mEnabled.GetValue()) // hm kinda ugly logic; maybe one day this whole fn will be improved?
                {
                    return "<mute>";
                }
                return "<silent>";
            }
        }
        // not silent / disabled / whatever. show some info.
        // TRI <mute>
        String ret = gOscWaveformShapeInfo.GetValueDisplayName(mOsc[i].mWaveform.GetValue());
        if (!mOsc[i].mEnabled.GetValue())
        {
            ret += " <mute>";
        }
        return ret;
    }

    virtual SerializationObjectMapArray GetSerializationObjectMap() override
    {
        return {{
            CreateSerializationMapping(mName, "Name"),
            CreateSerializationMapping(mMasterVolume, "vol"),
            CreateSerializationMapping(mPan, "p"),
            CreateSerializationMapping(mDelayMix, "dX"),
            CreateSerializationMapping(mVerbMix, "vX"),
            CreateSerializationMapping(mStereoSpread, "st"),
            CreateSerializationMapping(mDetune, "det"),
            CreateSerializationMapping(mFilter, "flt"),
            CreateSerializationMapping(mFMStrength2To1, "FM21"),
            CreateSerializationMapping(mFMStrength3To1, "FM31"),
            CreateSerializationMapping(mFMStrength1To2, "FM12"),
            CreateSerializationMapping(mFMStrength3To2, "FM32"),
            CreateSerializationMapping(mFMStrength1To3, "FM13"),
            CreateSerializationMapping(mFMStrength2To3, "FM23"),
            CreateSerializationMapping(mOverallFMStrength, "fmstr"),
            CreateSerializationMapping(mVoicingMode, "voxx"),
            CreateSerializationMapping(mModulations, "mod"),
            CreateSerializationMapping(mOsc, "Osc"),
            CreateSerializationMapping(mEnvelopes, "Env"),
            CreateSerializationMapping(mLFOs, "LFOs"),
        }};
    }

    // Result SerializableObject_ToJSON(JsonVariant doc) const
    // {
    //     Result ret = Result::Success();
    //     ret.AndRequires(mName.SerializableObject_ToJSON(doc.createNestedObject("name")), "name");
    //     ret.AndRequires(mMasterVolume.SerializableObject_ToJSON(doc.createNestedObject("vol")), "vol");
    //     ret.AndRequires(mPan.SerializableObject_ToJSON(doc.createNestedObject("pan")), "pan");
    //     ret.AndRequires(mDelayMix.SerializableObject_ToJSON(doc.createNestedObject("dly")), "dly");
    //     ret.AndRequires(mVerbMix.SerializableObject_ToJSON(doc.createNestedObject("verb")), "verb");
    //     ret.AndRequires(mStereoSpread.SerializableObject_ToJSON(doc.createNestedObject("spread")), "spread");
    //     ret.AndRequires(mDetune.SerializableObject_ToJSON(doc.createNestedObject("det")), "det");
    //     ret.AndRequires(mFilter.SerializableObject_ToJSON(doc.createNestedObject("flt")), "flt");
    //     ret.AndRequires(mFMStrength2To1.SerializableObject_ToJSON(doc.createNestedObject("fm21")), "fm21");
    //     ret.AndRequires(mFMStrength3To1.SerializableObject_ToJSON(doc.createNestedObject("fm31")), "fm31");
    //     ret.AndRequires(mFMStrength1To2.SerializableObject_ToJSON(doc.createNestedObject("fm12")), "fm12");
    //     ret.AndRequires(mFMStrength3To2.SerializableObject_ToJSON(doc.createNestedObject("fm32")), "fm32");
    //     ret.AndRequires(mFMStrength1To3.SerializableObject_ToJSON(doc.createNestedObject("fm13")), "fm13");
    //     ret.AndRequires(mFMStrength2To3.SerializableObject_ToJSON(doc.createNestedObject("fm23")), "fm23");
    //     ret.AndRequires(mOverallFMStrength.SerializableObject_ToJSON(doc.createNestedObject("fmAll")), "fmAll");
    //     ret.AndRequires(mVoicingMode.SerializableObject_ToJSON(doc.createNestedObject("vm")), "vm");
    //     ret.AndRequires(SerializeArrayToJSON(doc.createNestedObject("mod"), mModulations), "mod");
    //     ret.AndRequires(SerializeArrayToJSON(doc.createNestedObject("osc"), mOsc), "osc");
    //     ret.AndRequires(SerializeArrayToJSON(doc.createNestedObject("env"), mEnvelopes), "env");
    //     ret.AndRequires(SerializeArrayToJSON(doc.createNestedObject("lfo"), mLFOs), "lfo");
    //     return ret;
    // }

    // Result SerializableObject_Deserialize(JsonVariant obj)
    // {
    //     if (!obj.is<JsonObject>())
    //     {
    //         return Result::Failure("expected object");
    //     }
    //     Result ret = Result::Success();

    //     ret.AndRequires(mName.SerializableObject_Deserialize(obj["name"]), "name");
    //     ret.AndRequires(mMasterVolume.SerializableObject_Deserialize(obj["vol"]), "vol");
    //     ret.AndRequires(mPan.SerializableObject_Deserialize(obj["pan"]), "pan");
    //     ret.AndRequires(mDelayMix.SerializableObject_Deserialize(obj["dly"]), "dly");
    //     ret.AndRequires(mVerbMix.SerializableObject_Deserialize(obj["verb"]), "verb");
    //     ret.AndRequires(mStereoSpread.SerializableObject_Deserialize(obj["spread"]), "spread");
    //     ret.AndRequires(mDetune.SerializableObject_Deserialize(obj["det"]), "det");
    //     ret.AndRequires(mFilter.SerializableObject_Deserialize(obj["flt"]), "flt");
    //     ret.AndRequires(mFMStrength2To1.SerializableObject_Deserialize(obj["fm21"]), "fm21");
    //     ret.AndRequires(mFMStrength3To1.SerializableObject_Deserialize(obj["fm31"]), "fm31");
    //     ret.AndRequires(mFMStrength1To2.SerializableObject_Deserialize(obj["fm12"]), "fm12");
    //     ret.AndRequires(mFMStrength3To2.SerializableObject_Deserialize(obj["fm32"]), "fm32");
    //     ret.AndRequires(mFMStrength1To3.SerializableObject_Deserialize(obj["fm13"]), "fm13");
    //     ret.AndRequires(mFMStrength2To3.SerializableObject_Deserialize(obj["fm23"]), "fm23");
    //     ret.AndRequires(mOverallFMStrength.SerializableObject_Deserialize(obj["fmAll"]), "fmAll");
    //     ret.AndRequires(mVoicingMode.SerializableObject_Deserialize(obj["vm"]), "vm");

    //     ret.AndRequires(DeserializeArray(obj["mod"], mModulations), "mod");
    //     ret.AndRequires(DeserializeArray(obj["osc"], mOsc), "osc");
    //     ret.AndRequires(DeserializeArray(obj["env"], mEnvelopes), "env");
    //     ret.AndRequires(DeserializeArray(obj["lfo"], mLFOs), "lfo");

    //     return ret;
    // }
};

static constexpr auto synthpatchsize = sizeof(SynthPatch);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct SynthSettings : ISerializationObjectMap<1>
{
    std::array<SynthPatch, SYNTH_PRESET_COUNT> mPatches{
        initialize_array_with_indices<SynthPatch, SYNTH_PRESET_COUNT>()};

    virtual SerializationObjectMapArray GetSerializationObjectMap() override
    {
        return {{
            CreateSerializationMapping(mPatches, "Patches"),
        }};
    }

    static void InitBommanoidPreset(SynthPatch &p, const char *name)
    {
        p.mName.SetValue(name);
        p.mFilter.mType.SetValue(ClarinoidFilterType::Disabled);

        p.mOsc[0].mWaveform.SetValue(OscWaveformShape::SawSync);
        p.mOsc[0].mVolume.SetValue(0.4f);

        p.mFilter.mFrequency.SetParamValue(0);
        p.mFilter.mFrequency.SetKTParamValue(1.0f);

        p.mModulations[0].mDest.SetValue(AnyModulationDestination::MasterVolume);
        p.mModulations[0].mSource.SetValue(AnyModulationSource::ENV1);
        p.mModulations[0].mAuxEnabled.SetValue(false);
        p.mModulations[0].mScaleN11.SetValue(1.0f);
    };

    // Result SerializableObject_ToJSON(JsonVariant rhs) const
    // {
    //     return SerializeArrayToJSON(rhs.createNestedArray("patches"), mPatches);
    // }

    // Result SerializableObject_Deserialize(JsonVariant obj)
    // {
    //     if (!obj.is<JsonObject>())
    //     {
    //         return Result::Failure("expected object");
    //     }

    //     return DeserializeArray(obj["patches"], mPatches);
    // }

    SynthSettings()
    {
        InitBommanoidPreset(mPatches[0], "default");
        InitBommanoidPreset(mPatches[SynthPresetID_Bommanoid], "Bommanoid");
    }
};

} // namespace clarinoid
