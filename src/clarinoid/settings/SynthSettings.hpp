
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{
static constexpr size_t POLYBLEP_OSC_COUNT = 3;
static constexpr size_t ENVELOPE_COUNT = 3;
static constexpr size_t LFO_COUNT = 3;

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
    // SyncPulse = 4,   //                 pulsewidth   syncfreq
};

EnumItemInfo<OscWaveformShape> gOscWaveformShapeItems[4] = {
    {OscWaveformShape::Sine, "Sine", "sin"},
    {OscWaveformShape::VarTriangle, "Tri-Saw", "tri"},
    {OscWaveformShape::Pulse, "Pulse", "sq"},
    {OscWaveformShape::SawSync, "HQ Sawtooth", "saw"},
};

EnumInfo<OscWaveformShape> gOscWaveformShapeInfo("OscWaveformShape", gOscWaveformShapeItems);

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
    {KRateModulationSource::Breath, "Breath"},
    {KRateModulationSource::PitchStrip, "PitchBend"},
    {KRateModulationSource::Velocity, "Velocity"},
    {KRateModulationSource::NoteValue, "NoteValue"},
    {KRateModulationSource::RandomTrigger, "RandomTrigger"},
    {KRateModulationSource::ModWheel, "ModWheel"},
    {KRateModulationSource::Macro1, "Macro1"},
    {KRateModulationSource::Macro2, "Macro2"},
    {KRateModulationSource::Macro3, "Macro3"},
    {KRateModulationSource::Macro4, "Macro4"},
    {KRateModulationSource::Pedal, "Pedal"},
};
static constexpr size_t gKRateModulationSourceCount = SizeofStaticArray(gKRateModulationSourceItems);

EnumInfo<KRateModulationSource> gKRateModulationSourceInfo("KRateModSource", gKRateModulationSourceItems);

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
    {AnyModulationSource::None, "None"},
    {AnyModulationSource::LFO1, "LFO1"},
    {AnyModulationSource::LFO2, "LFO2"},
    {AnyModulationSource::LFO3, "LFO3"},
    {AnyModulationSource::ENV1, "ENV1"},
    {AnyModulationSource::ENV2, "ENV2"},
    {AnyModulationSource::ENV3, "ENV3"},
    {AnyModulationSource::Osc1FB, "Osc1FB"},
    {AnyModulationSource::Osc2FB, "Osc2FB"},
    {AnyModulationSource::Osc3FB, "Osc3FB"},
    {AnyModulationSource::Breath, "Breath"},
    {AnyModulationSource::PitchStrip, "PitchBend"},
    {AnyModulationSource::Velocity, "Velocity"},
    {AnyModulationSource::NoteValue, "NoteValue"},
    {AnyModulationSource::RandomTrigger, "RandomTrigger"},
    {AnyModulationSource::ModWheel, "ModWheel"},
    {AnyModulationSource::Macro1, "Macro1"},
    {AnyModulationSource::Macro2, "Macro2"},
    {AnyModulationSource::Macro3, "Macro3"},
    {AnyModulationSource::Macro4, "Macro4"},
    {AnyModulationSource::Pedal, "Pedal"},
};

static constexpr size_t gAnyModulationSourceCount = SizeofStaticArray(gAnyModulationSourceItems);

EnumInfo<AnyModulationSource> gAnyModulationSourceInfo("AnyModSource", gAnyModulationSourceItems);

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
    {ARateModulationDestination::Osc1PulseWidth, "Osc1PulseWidth"},
    {ARateModulationDestination::Osc1Phase, "Osc1Phase"},

    {ARateModulationDestination::Osc2PulseWidth, "Osc2PulseWidth"},
    {ARateModulationDestination::Osc2Phase, "Osc2Phase"},

    {ARateModulationDestination::Osc3PulseWidth, "Osc3PulseWidth"},
    {ARateModulationDestination::Osc3Phase, "Osc3Phase"},
};

static constexpr size_t gARateModulationDestinationCount = SizeofStaticArray(gARateModulationDestinationItems);

EnumInfo<ARateModulationDestination> gARateModulationDestinationInfo("ARateModDest", gARateModulationDestinationItems);

enum class KRateModulationDestination : uint8_t
{
    // these are INDICES used by synthvoice / modulationmatrix
    FilterCutoff = 0, // k-rate
    MasterVolume,
    Detune,

    Osc1Volume, // k-rate
    Osc2Volume, // k-rate
    Osc3Volume, // k-rate

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

EnumItemInfo<KRateModulationDestination> gKRateModulationDestinationItems[49] = {
    {KRateModulationDestination::FilterCutoff, "FilterCutoff"},
    {KRateModulationDestination::MasterVolume, "MasterVolume"},
    {KRateModulationDestination::Detune, "Detune"},
    {KRateModulationDestination::Osc1Volume, "Osc1Volume"},
    {KRateModulationDestination::Osc2Volume, "Osc2Volume"},
    {KRateModulationDestination::Osc3Volume, "Osc3Volume"},
    {KRateModulationDestination::Osc1FMFeedback, "Osc1FMFeedback"},
    {KRateModulationDestination::Osc2FMFeedback, "Osc2FMFeedback"},
    {KRateModulationDestination::Osc3FMFeedback, "Osc3FMFeedback"},
    {KRateModulationDestination::OverallFMStrength, "OverallFMStrength"},
    {KRateModulationDestination::FMStrength2To1, "FMStrength2To1"},
    {KRateModulationDestination::FMStrength3To1, "FMStrength3To1"},
    {KRateModulationDestination::FMStrength1To2, "FMStrength1To2"},
    {KRateModulationDestination::FMStrength3To2, "FMStrength3To2"},
    {KRateModulationDestination::FMStrength1To3, "FMStrength1To3"},
    {KRateModulationDestination::FMStrength2To3, "FMStrength2To3"},
    {KRateModulationDestination::Osc1FrequencyParam, "Osc1FrequencyP"},
    {KRateModulationDestination::Osc2FrequencyParam, "Osc2FrequencyP"},
    {KRateModulationDestination::Osc3FrequencyParam, "Osc3FrequencyP"},
    {KRateModulationDestination::Osc1SyncFrequencyParam, "Osc1SyncFrequency"},
    {KRateModulationDestination::Osc2SyncFrequencyParam, "Osc2SyncFrequency"},
    {KRateModulationDestination::Osc3SyncFrequencyParam, "Osc3SyncFrequency"},

    {KRateModulationDestination::Env1DelayTime, "Env1DelayTime"},
    {KRateModulationDestination::Env1AttackTime, "Env1AttackTime"},
    {KRateModulationDestination::Env1AttackCurve, "Env1AttackCurve"},
    {KRateModulationDestination::Env1HoldTime, "Env1HoldTime"},
    {KRateModulationDestination::Env1DecayTime, "Env1DecayTime"},
    {KRateModulationDestination::Env1DecayCurve, "Env1DecayCurve"},
    {KRateModulationDestination::Env1SustainLevel, "Env1SustainLevel"},
    {KRateModulationDestination::Env1ReleaseTime, "Env1ReleaseTime"},
    {KRateModulationDestination::Env1ReleaseCurve, "Env1ReleaseCurve"},

    {KRateModulationDestination::Env2DelayTime, "Env2DelayTime"},
    {KRateModulationDestination::Env2AttackTime, "Env2AttackTime"},
    {KRateModulationDestination::Env2AttackCurve, "Env2AttackCurve"},
    {KRateModulationDestination::Env2HoldTime, "Env2HoldTime"},
    {KRateModulationDestination::Env2DecayTime, "Env2DecayTime"},
    {KRateModulationDestination::Env2DecayCurve, "Env2DecayCurve"},
    {KRateModulationDestination::Env2SustainLevel, "Env2SustainLevel"},
    {KRateModulationDestination::Env2ReleaseTime, "Env2ReleaseTime"},
    {KRateModulationDestination::Env2ReleaseCurve, "Env2ReleaseCurve"},

    {KRateModulationDestination::Env3DelayTime, "Env3DelayTime"},
    {KRateModulationDestination::Env3AttackTime, "Env3AttackTime"},
    {KRateModulationDestination::Env3AttackCurve, "Env3AttackCurve"},
    {KRateModulationDestination::Env3HoldTime, "Env3HoldTime"},
    {KRateModulationDestination::Env3DecayTime, "Env3DecayTime"},
    {KRateModulationDestination::Env3DecayCurve, "Env3DecayCurve"},
    {KRateModulationDestination::Env3SustainLevel, "Env3SustainLevel"},
    {KRateModulationDestination::Env3ReleaseTime, "Env3ReleaseTime"},
    {KRateModulationDestination::Env3ReleaseCurve, "Env3ReleaseCurve"},
};

static constexpr size_t gKRateModulationDestinationCount = SizeofStaticArray(gKRateModulationDestinationItems);

EnumInfo<KRateModulationDestination> gKRateModulationDestinationInfo("KRateModDest", gKRateModulationDestinationItems);

// Lets create a way to get modulation enum values by oscillator index
struct OscillatorModulationInfo
{
    KRateModulationDestination KRateDestination_FMFeedback;
    KRateModulationDestination KRateDestination_Frequency;
    KRateModulationDestination KRateDestination_SyncFrequency;
    KRateModulationDestination KRateDestination_Volume;
};

static OscillatorModulationInfo gModValuesByOscillator[POLYBLEP_OSC_COUNT] = {
    {KRateModulationDestination::Osc1FMFeedback,
     KRateModulationDestination::Osc1FrequencyParam,
     KRateModulationDestination::Osc1SyncFrequencyParam,
     KRateModulationDestination::Osc1Volume},

    {KRateModulationDestination::Osc2FMFeedback,
     KRateModulationDestination::Osc2FrequencyParam,
     KRateModulationDestination::Osc1SyncFrequencyParam,
     KRateModulationDestination::Osc2Volume},

    {KRateModulationDestination::Osc3FMFeedback,
     KRateModulationDestination::Osc3FrequencyParam,
     KRateModulationDestination::Osc1SyncFrequencyParam,
     KRateModulationDestination::Osc3Volume},
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
    {AnyModulationDestination::None, "None"},
    {AnyModulationDestination::Osc1PulseWidth, "Osc1PulseWidth"},
    {AnyModulationDestination::Osc1Phase, "Osc1Phase"},
    {AnyModulationDestination::Osc2PulseWidth, "Osc2PulseWidth"},
    {AnyModulationDestination::Osc2Phase, "Osc2Phase"},
    {AnyModulationDestination::Osc3PulseWidth, "Osc3PulseWidth"},
    {AnyModulationDestination::Osc3Phase, "Osc3Phase"}, // A-rate

    // K-rates:
    {AnyModulationDestination::FilterCutoff, "FilterCutoff"},
    {AnyModulationDestination::MasterVolume, "MasterVolume"},
    {AnyModulationDestination::Detune, "Detune"},
    {AnyModulationDestination::Osc1Volume, "Osc1Volume"},
    {AnyModulationDestination::Osc2Volume, "Osc2Volume"},
    {AnyModulationDestination::Osc3Volume, "Osc3Volume"},
    {AnyModulationDestination::Osc1FMFeedback, "Osc1FMFeedback"},
    {AnyModulationDestination::Osc2FMFeedback, "Osc2FMFeedback"},
    {AnyModulationDestination::Osc3FMFeedback, "Osc3FMFeedback"},
    {AnyModulationDestination::OverallFMStrength, "OverallFMStrength"},
    {AnyModulationDestination::FMStrength2To1, "FMStrength2To1"},
    {AnyModulationDestination::FMStrength3To1, "FMStrength3To1"},
    {AnyModulationDestination::FMStrength1To2, "FMStrength1To2"},
    {AnyModulationDestination::FMStrength3To2, "FMStrength3To2"},
    {AnyModulationDestination::FMStrength1To3, "FMStrength1To3"},
    {AnyModulationDestination::FMStrength2To3, "FMStrength2To3"},
    {AnyModulationDestination::Osc1FrequencyParam, "Osc1Frequency"},
    {AnyModulationDestination::Osc2FrequencyParam, "Osc2Frequency"},
    {AnyModulationDestination::Osc3FrequencyParam, "Osc3Frequency"},
    {AnyModulationDestination::Osc1SyncFrequencyParam, "Osc1SyncFrequency"},
    {AnyModulationDestination::Osc2SyncFrequencyParam, "Osc2SyncFrequency"},
    {AnyModulationDestination::Osc3SyncFrequencyParam, "Osc3SyncFrequency"},

    {AnyModulationDestination::Env1DelayTime, "Env1DelayTime"},
    {AnyModulationDestination::Env1AttackTime, "Env1AttackTime"},
    {AnyModulationDestination::Env1AttackCurve, "Env1AttackCurve"},
    {AnyModulationDestination::Env1HoldTime, "Env1HoldTime"},
    {AnyModulationDestination::Env1DecayTime, "Env1DecayTime"},
    {AnyModulationDestination::Env1DecayCurve, "Env1DecayCurve"},
    {AnyModulationDestination::Env1SustainLevel, "Env1SustainLevel"},
    {AnyModulationDestination::Env1ReleaseTime, "Env1ReleaseTime"},
    {AnyModulationDestination::Env1ReleaseCurve, "Env1ReleaseCurve"},

    {AnyModulationDestination::Env2DelayTime, "Env2DelayTime"},
    {AnyModulationDestination::Env2AttackTime, "Env2AttackTime"},
    {AnyModulationDestination::Env2AttackCurve, "Env2AttackCurve"},
    {AnyModulationDestination::Env2HoldTime, "Env2HoldTime"},
    {AnyModulationDestination::Env2DecayTime, "Env2DecayTime"},
    {AnyModulationDestination::Env2DecayCurve, "Env2DecayCurve"},
    {AnyModulationDestination::Env2SustainLevel, "Env2SustainLevel"},
    {AnyModulationDestination::Env2ReleaseTime, "Env2ReleaseTime"},
    {AnyModulationDestination::Env2ReleaseCurve, "Env2ReleaseCurve"},

    {AnyModulationDestination::Env3DelayTime, "Env3DelayTime"},
    {AnyModulationDestination::Env3AttackTime, "Env3AttackTime"},
    {AnyModulationDestination::Env3AttackCurve, "Env3AttackCurve"},
    {AnyModulationDestination::Env3HoldTime, "Env3HoldTime"},
    {AnyModulationDestination::Env3DecayTime, "Env3DecayTime"},
    {AnyModulationDestination::Env3DecayCurve, "Env3DecayCurve"},
    {AnyModulationDestination::Env3SustainLevel, "Env3SustainLevel"},
    {AnyModulationDestination::Env3ReleaseTime, "Env3ReleaseTime"},
    {AnyModulationDestination::Env3ReleaseCurve, "Env3ReleaseCurve"},

    // LFO
};

static constexpr size_t gAnyModulationDestinationCount = SizeofStaticArray(gAnyModulationDestinationItems);

EnumInfo<AnyModulationDestination> gAnyModulationDestinationInfo("AnyModDest", gAnyModulationDestinationItems);

enum class ModulationPolarityTreatment : uint8_t
{
    AsPositive01,         // force it to 0-1 positive polarity
    AsPositive01Inverted, // same as Positive01, but signal is inverted so 0,1 translates 1,0. (1-x)
    AsBipolar,            // force it to -1,1 bi-polarity
    AsBipolarInverted,    // same as AsBipolar, but signal is inverted so -1,1 translates 1,-1 (-x)
    Default,              // do not change the source signal at all.
};

EnumItemInfo<ModulationPolarityTreatment> gModulationPolarityTreatmentItems[5] = {
    {ModulationPolarityTreatment::AsPositive01, "Map to [0,1]"},
    {ModulationPolarityTreatment::AsPositive01Inverted, "Map to [1,0]"},
    {ModulationPolarityTreatment::AsBipolar, "Map to [-1,1]"},
    {ModulationPolarityTreatment::AsBipolarInverted, "Map to [1,-1]"},
    {ModulationPolarityTreatment::Default, "Default"},
};
static constexpr size_t gModulationPolarityTreatmentCount = SizeofStaticArray(gAnyModulationDestinationItems);

EnumInfo<ModulationPolarityTreatment> gModulationPolarityTreatmentInfo("ModulationPolarity",
                                                                       gModulationPolarityTreatmentItems);

EnumItemInfo<ModulationPolarityTreatment> gModulationAuxPolarityTreatmentItems[2] = {
    {ModulationPolarityTreatment::AsPositive01, "Map to [0,1]"},
    {ModulationPolarityTreatment::AsPositive01Inverted, "Map to [1,0]"},
};

static constexpr size_t gModulationAuxPolarityTreatmentCount = SizeofStaticArray(gModulationAuxPolarityTreatmentItems);

EnumInfo<ModulationPolarityTreatment> gModulationAuxPolarityTreatmentInfo("ModulationAuxPolarity",
                                                                          gModulationAuxPolarityTreatmentItems);

struct SynthModulationSpec
{
    AnyModulationSource mSource = AnyModulationSource::None;
    AnyModulationDestination mDest = AnyModulationDestination::None;
    ModulationPolarityTreatment mSourcePolarity = ModulationPolarityTreatment::Default;
    CurveLUTParamValue mCurveShape = 0;
    float mScaleN11 = 0.5f;

    AnyModulationSource mAuxSource = AnyModulationSource::None;
    ModulationPolarityTreatment mAuxPolarity = ModulationPolarityTreatment::AsPositive01;
    bool mAuxEnabled = true; // just allows bypassing without removing the aux source
    CurveLUTParamValue mAuxCurveShape = 0;
    float mAuxAmount = 0.0f; // amount of attenuation

    // to mimic old behavior with 0 offset and just a scale.
    void SetScaleN11_Legacy(float scaleN11)
    {
        mScaleN11 = scaleN11;
    }

    String ToDisplayString() const
    {
        if (mSource == AnyModulationSource::None)
            return "--";
        if (mDest == AnyModulationDestination::None)
            return "--";
        return String(gAnyModulationSourceInfo.GetValueDisplayName(mSource)) + ">" +
               gAnyModulationDestinationInfo.GetValueDisplayName(mDest);
    }
};

struct EnvelopeSpec
{
    EnvTimeParamValue mDelayTime = 0.0f;
    EnvTimeParamValue mAttackTime = 0.0f;
    CurveLUTParamValue mAttackCurve = 0.0f;
    EnvTimeParamValue mHoldTime = 0.0f;
    EnvTimeParamValue mDecayTime = 0.5f;
    CurveLUTParamValue mDecayCurve = 0;
    float mSustainLevel = 0.5f;
    EnvTimeParamValue mReleaseTime = 0.6f;
    CurveLUTParamValue mReleaseCurve = 0;
    bool mLegatoRestart = false;

    EnvelopeSpec(size_t myIndex)
    {
    }

    EnvelopeSpec() = default;
};

struct SynthOscillatorSettings
{
    void CopyFrom(const SynthOscillatorSettings &rhs)
    {
        Serial.println("TODO: Harm patch copy");
#pragma message "TODO: Harm patch copy"
    }

    VolumeParamValue mVolume = VolumeParamValue::FromParamValue(0.4f);
    // this is not redundant with volume, because
    // - it enables quick muting
    // - it helps know how to display things
    // - it helps know how detune / stereo sep will operate.
    bool mEnabled = true;
    int mPortamentoTimeMS = 0;

    // for pitch, it's even hard to know which kind of params are needed.
    // 1. for FM, ratio & offset(hz) are absolutely necessary.
    // 2. for sync, param + KT amt are ideal, to behave like filter.
    // 3. for transposition & detune, semis & fine.

    // these are good for FM
    float mFreqMultiplier = 1.0f; // midinotefreq * this
    float mFreqOffsetHz = 0.0f;

    // these are good for modulation, and for sync frequency.
    FrequencyParamValue mFreqParam = {"Frequency", 0.3f, 1.0f}; // param, kt amt

    bool mHardSyncEnabled = false;
    FrequencyParamValue mSyncFreqParam = {"SyncFrequency", 0.4f, 1.0f}; // param, kt amt

    // these are good for musical stuff like transposition.
    int mPitchSemis = 0;  // semis = integral, transposition. want to keep this integral because the menu system is
                          // not so great at being very precise.
    float mPitchFine = 0; // in semitones, just for detuning

    float mPitchBendRangePositive = 2.0f;
    float mPitchBendRangeNegative = -2.0f;

    float mPan = 0;

    bool mPhaseRestart = false;
    float mPhase01 = 0.0f;

    OscWaveformShape mWaveform = OscWaveformShape::VarTriangle;
    float mPulseWidth = 0.5f;

    float mFMFeedbackGain = 0.0f; // 0 to 1

    const size_t mMyIndex;
    SynthOscillatorSettings(size_t myIndex) : mMyIndex(myIndex)
    {
        //
    }
};

struct LFOSpec //: SerializableDictionary
{
    EnumParam<OscWaveformShape> mWaveShape = {"Waveform", gOscWaveformShapeInfo, OscWaveformShape::Sine};
    FloatParam mPulseWidth = {"PulseWidth", 0.5f};
    TimeWithBasisParam mSpeed = {"Speed", TimeBasis::Hertz, 1.0f};

    // BoolParam mPhaseRestart = {"PhaseRestart", false};

    // SerializableObject *mSerializableChildObjects[4] = {
    //     &mWaveShape,
    //     &mPulseWidth,
    //     &mSpeed,
    //     &mPhaseRestart,
    // };

    const size_t mMyIndex;

    LFOSpec(size_t myIndex)
        : // SerializableDictionary("LFO", mSerializableChildObjects), //
          mMyIndex(myIndex)
    {
    }

    // when 0, all voices use the same LFO phase
    // when 1, phase is distributed/staggered among voices
    // float mVoicePhaseDistribution01 = 1.0f;

    // when 1, phase offset is "randomized"
    // float mPhaseDistributionRandomization01 = 1.0f;
    // float mRandomSeed01 = 0; // due to float precision, actual seed will snap this so there are only like 100 values.
};

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

struct FilterSettings //: SerializableDictionary
{
    FilterSettings(const char *fieldName) //: SerializableDictionary(fieldName, mSerializableChildObjects)
    {
    }

    EnumParam<ClarinoidFilterType> mType = {"Type", gClarinoidFilterTypeInfo, ClarinoidFilterType::LP_Moog4};
    FloatParam mQ = {"Q", 0.02f};
    FloatParam mSaturation = {"Saturation", 0.2f};
    FrequencyParamValue mFrequency = {"Frequency", 0.3f, 0.0f}; // param, kt amt

    bool SerializableObject_ToJSON(JsonVariant rhs) const
    {
        bool r = true;
        r = r && mType.SerializableObject_ToJSON(rhs.createNestedObject("type"));
        r = r && mQ.SerializableObject_ToJSON(rhs.createNestedObject("q"));
        r = r && mSaturation.SerializableObject_ToJSON(rhs.createNestedObject("sat"));
        r = r && mFrequency.SerializableObject_ToJSON(rhs.createNestedObject("freq"));
        return r;
    }
    // SerializableObject *mSerializableChildObjects[4] = {
    //     &mType,
    //     &mQ,
    //     &mSaturation,
    //     &mFrequency,
    // };
};

struct SynthPreset
{
    std::array<SynthOscillatorSettings, POLYBLEP_OSC_COUNT> mOsc{
        initialize_array_with_indices<SynthOscillatorSettings, POLYBLEP_OSC_COUNT>()};

    void CopyFrom(const SynthPreset &rhs)
    {
        Serial.println("TODO: Harm patch copy");
#pragma message "TODO: Harm patch copy"
    }

    String mName = "--";
    VolumeParamValue mMasterVolume;
    float mPan = 0;
    float mDelayMix = 0.08f;
    float mVerbMix = 0.08f;
    float mStereoSpread = 0.15f;

    std::array<EnvelopeSpec, ENVELOPE_COUNT> mEnvelopes{initialize_array_with_indices<EnvelopeSpec, ENVELOPE_COUNT>()};

    VoicingMode mVoicingMode = VoicingMode::Polyphonic;

    std::array<LFOSpec, LFO_COUNT> mLFOs{initialize_array_with_indices<LFOSpec, LFO_COUNT>()};

    float mDetune = 0;

    // bool mSync = true;

    bool mDCFilterEnabled = true;
    float mDCFilterCutoff = 10.0f;

    FilterSettings mFilter = {"Filter1"};
    // ClarinoidFilterType mFilterType = ClarinoidFilterType::LP_Moog4;
    // float mFilterQ = 0.02f;
    // float mFilterSaturation = 0.2f;
    // FrequencyParamValue mFilterFreqParam = {0.3f, 0.0f}; // param, kt amt

    // for FM modulation matrix.
    float mFMStrength2To1 = 0;
    float mFMStrength3To1 = 0;
    float mFMStrength1To2 = 0;
    float mFMStrength3To2 = 0;
    float mFMStrength1To3 = 0;
    float mFMStrength2To3 = 0;

    float mOverallFMStrength = 1.00f;

    SynthModulationSpec mModulations[SYNTH_MODULATIONS_MAX];

    const size_t mMyIndex;
    SynthPreset(size_t myIndex) : mMyIndex(myIndex)
    {
        mLFOs[0].mSpeed.SetFrequency(1.1f);
        mLFOs[1].mSpeed.SetFrequency(3.5f);

        mOsc[0].mVolume.SetValue(0.4f);
        mOsc[1].mVolume.SetValue(0);
        mOsc[2].mVolume.SetValue(0);

        mOsc[0].mWaveform = OscWaveformShape::VarTriangle;
        mOsc[1].mWaveform = OscWaveformShape::SawSync;
        mOsc[2].mWaveform = OscWaveformShape::VarTriangle;
    }

    String ToString(int index) const
    {
        return String("") + index + ":" + mName;
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
            if (!Any(this->mModulations, [&](const SynthModulationSpec &m) { return m.mDest == dest; }))
            {
                if (!mOsc[i].mEnabled) // hm kinda ugly logic; maybe one day this whole fn will be improved?
                {
                    return "<mute>";
                }
                return "<silent>";
            }
        }
        // not silent / disabled / whatever. show some info.
        // TRI <mute>
        String ret = gOscWaveformShapeInfo.GetValueDisplayName(mOsc[i].mWaveform);
        if (!mOsc[i].mEnabled)
        {
            ret += " <mute>";
        }
        return ret;
    }
};

static constexpr auto synthpatchsize = sizeof(SynthPreset);

struct SynthSettings
{
    std::array<SynthPreset, SYNTH_PRESET_COUNT> mPresets{
        initialize_array_with_indices<SynthPreset, SYNTH_PRESET_COUNT>()};

    static void InitBommanoidPreset(SynthPreset &p, const char *name)
    {
        p.mName = name;
        p.mDetune = 0.0f;
        p.mStereoSpread = 0;
        p.mFilter.mType.SetValue(ClarinoidFilterType::Disabled);

        p.mOsc[0].mWaveform = OscWaveformShape::SawSync;

        p.mFilter.mFrequency.SetParamValue(0);
        p.mFilter.mFrequency.SetKTParamValue(1.0f);

        p.mModulations[0].mDest = AnyModulationDestination::MasterVolume;
        p.mModulations[0].mSource = AnyModulationSource::ENV1;
        p.mModulations[0].mAuxEnabled = false;
        p.mModulations[0].mScaleN11 = 1.0f;
    };

    SynthSettings()
    {
        InitBommanoidPreset(mPresets[0], "default");
        InitBommanoidPreset(mPresets[SynthPresetID_Bommanoid], "Bommanoid");
    }
};

} // namespace clarinoid
