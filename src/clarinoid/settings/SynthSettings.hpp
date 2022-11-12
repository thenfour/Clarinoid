
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
    {ARateModulationDestination::Osc1PulseWidth, "Osc1PulseWidth"},
    {ARateModulationDestination::Osc1Phase, "Osc1Phase"},

    {ARateModulationDestination::Osc2PulseWidth, "Osc2PulseWidth"},
    {ARateModulationDestination::Osc2Phase, "Osc2Phase"},

    {ARateModulationDestination::Osc3PulseWidth, "Osc3PulseWidth"},
    {ARateModulationDestination::Osc3Phase, "Osc3Phase"},
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct SynthModulationSpec
{
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
        return String(gAnyModulationSourceInfo.GetValueDisplayName(mSource.GetValue())) + ">" +
               gAnyModulationDestinationInfo.GetValueDisplayName(mDest.GetValue());
    }

    bool SerializableObject_ToJSON(JsonVariant doc) const
    {
        bool ret = true;
        ret = ret && mSource.SerializableObject_ToJSON(doc.createNestedObject("src"));
        ret = ret && mDest.SerializableObject_ToJSON(doc.createNestedObject("dst"));
        ret = ret && mSourcePolarity.SerializableObject_ToJSON(doc.createNestedObject("pol"));
        ret = ret && mCurveShape.SerializableObject_ToJSON(doc.createNestedObject("crv"));
        ret = ret && mScaleN11.SerializableObject_ToJSON(doc.createNestedObject("amt"));
        ret = ret && mAuxSource.SerializableObject_ToJSON(doc.createNestedObject("Xsrc"));
        ret = ret && mAuxPolarity.SerializableObject_ToJSON(doc.createNestedObject("Xpol"));
        ret = ret && mAuxEnabled.SerializableObject_ToJSON(doc.createNestedObject("Xen"));
        ret = ret && mAuxCurveShape.SerializableObject_ToJSON(doc.createNestedObject("Xcrv"));
        ret = ret && mAuxAmount.SerializableObject_ToJSON(doc.createNestedObject("Xamt"));
        return ret;
    }

    Result SerializableObject_Deserialize(JsonVariant obj)
    {
        if (!obj.is<JsonObject>())
        {
            return Result::Failure("expected object");
        }
        Result ret = Result::Success();

        ret.AndRequires(mSource.SerializableObject_Deserialize(obj["src"]), "src");
        ret.AndRequires(mDest.SerializableObject_Deserialize(obj["dst"]), "dst");
        ret.AndRequires(mSourcePolarity.SerializableObject_Deserialize(obj["pol"]), "pol");
        ret.AndRequires(mCurveShape.SerializableObject_Deserialize(obj["crv"]), "crv");
        ret.AndRequires(mScaleN11.SerializableObject_Deserialize(obj["amt"]), "amt");
        ret.AndRequires(mAuxSource.SerializableObject_Deserialize(obj["Xsrc"]), "Xsrc");
        ret.AndRequires(mAuxPolarity.SerializableObject_Deserialize(obj["Xpol"]), "Xpol");
        ret.AndRequires(mAuxEnabled.SerializableObject_Deserialize(obj["Xen"]), "Xen");
        ret.AndRequires(mAuxCurveShape.SerializableObject_Deserialize(obj["Xcrv"]), "Xcrv");
        ret.AndRequires(mAuxAmount.SerializableObject_Deserialize(obj["Xamt"]), "Xamt");

        return ret;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct EnvelopeSpec
{
    EnvTimeParamValue mDelayTime{0.0f};
    EnvTimeParamValue mAttackTime{0.0f};
    CurveLUTParamValue mAttackCurve{0.0f};
    EnvTimeParamValue mHoldTime{0.0f};
    EnvTimeParamValue mDecayTime{0.5f};
    CurveLUTParamValue mDecayCurve{0};
    FloatParam mSustainLevel{0};
    EnvTimeParamValue mReleaseTime{0.6f};
    CurveLUTParamValue mReleaseCurve{0};
    BoolParam mLegatoRestart{false};

    bool SerializableObject_ToJSON(JsonVariant doc) const
    {
        bool ret = true;
        ret = ret && mDelayTime.SerializableObject_ToJSON(doc.createNestedObject("L"));
        ret = ret && mAttackTime.SerializableObject_ToJSON(doc.createNestedObject("A"));
        ret = ret && mAttackCurve.SerializableObject_ToJSON(doc.createNestedObject("Ac"));
        ret = ret && mHoldTime.SerializableObject_ToJSON(doc.createNestedObject("H"));
        ret = ret && mDecayTime.SerializableObject_ToJSON(doc.createNestedObject("D"));
        ret = ret && mDecayCurve.SerializableObject_ToJSON(doc.createNestedObject("Dc"));
        ret = ret && mSustainLevel.SerializableObject_ToJSON(doc.createNestedObject("S"));
        ret = ret && mReleaseTime.SerializableObject_ToJSON(doc.createNestedObject("R"));
        ret = ret && mReleaseCurve.SerializableObject_ToJSON(doc.createNestedObject("Rc"));
        ret = ret && mLegatoRestart.SerializableObject_ToJSON(doc.createNestedObject("Rst"));
        return ret;
    }

    Result SerializableObject_Deserialize(JsonVariant obj)
    {
        if (!obj.is<JsonObject>())
        {
            return Result::Failure("expected object");
        }
        Result ret = Result::Success();

        ret.AndRequires(mDelayTime.SerializableObject_Deserialize(obj["L"]), "L");
        ret.AndRequires(mAttackTime.SerializableObject_Deserialize(obj["A"]), "A");
        ret.AndRequires(mAttackCurve.SerializableObject_Deserialize(obj["Ac"]), "Ac");
        ret.AndRequires(mHoldTime.SerializableObject_Deserialize(obj["H"]), "H");
        ret.AndRequires(mDecayTime.SerializableObject_Deserialize(obj["D"]), "D");
        ret.AndRequires(mDecayCurve.SerializableObject_Deserialize(obj["Dc"]), "Dc");
        ret.AndRequires(mSustainLevel.SerializableObject_Deserialize(obj["S"]), "S");
        ret.AndRequires(mReleaseTime.SerializableObject_Deserialize(obj["R"]), "R");
        ret.AndRequires(mReleaseCurve.SerializableObject_Deserialize(obj["Rc"]), "Rc");
        ret.AndRequires(mLegatoRestart.SerializableObject_Deserialize(obj["Rst"]), "Rst");

        return ret;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct SynthOscillatorSettings
{
    void CopyFrom(const SynthOscillatorSettings &rhs)
    {
        *this = rhs;
    }

    VolumeParamValue mVolume = VolumeParamValue::FromParamValue(0.4f);
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

    // these are good for modulation, and for sync frequency.
    FrequencyParamValue mFreqParam{0.3f, 1.0f}; // param, kt amt

    BoolParam mHardSyncEnabled{false};
    FrequencyParamValue mSyncFreqParam{0.4f, 1.0f}; // param, kt amt

    // these are good for musical stuff like transposition.
    IntParam<int> mPitchSemis{0}; // semis = integral, transposition. want to keep this integral because the menu system
                                  // is not so great at being very precise.
    FloatParam mPitchFine{0}; // in semitones, just for detuning

    FloatParam mPitchBendRangePositive{2.0f};
    FloatParam mPitchBendRangeNegative{-2.0f};

    FloatParam mPan{0};

    BoolParam mPhaseRestart{false};
    FloatParam mPhase01{0};

    EnumParam<OscWaveformShape> mWaveform{gOscWaveformShapeInfo, OscWaveformShape::VarTriangle};
    FloatParam mPulseWidth{0.5f};

    FloatParam mFMFeedbackGain{0.0f}; // 0 to 1

    bool SerializableObject_ToJSON(JsonVariant rhs) const
    {
        bool r = true;
        r = r && mEnabled.SerializableObject_ToJSON(rhs.createNestedObject("on"));
        r = r && mWaveform.SerializableObject_ToJSON(rhs.createNestedObject("wav"));
        r = r && mVolume.SerializableObject_ToJSON(rhs.createNestedObject("vol"));
        r = r && mPortamentoTimeMS.SerializableObject_ToJSON(rhs.createNestedObject("port"));
        r = r && mFreqMultiplier.SerializableObject_ToJSON(rhs.createNestedObject("Fmul"));
        r = r && mFreqOffsetHz.SerializableObject_ToJSON(rhs.createNestedObject("Foff"));
        r = r && mFreqParam.SerializableObject_ToJSON(rhs.createNestedObject("Freq"));
        r = r && mHardSyncEnabled.SerializableObject_ToJSON(rhs.createNestedObject("sync"));
        r = r && mSyncFreqParam.SerializableObject_ToJSON(rhs.createNestedObject("syncFreq"));
        r = r && mPitchSemis.SerializableObject_ToJSON(rhs.createNestedObject("transp"));
        r = r && mPitchFine.SerializableObject_ToJSON(rhs.createNestedObject("fine"));
        r = r && mPitchBendRangePositive.SerializableObject_ToJSON(rhs.createNestedObject("pbPos"));
        r = r && mPitchBendRangeNegative.SerializableObject_ToJSON(rhs.createNestedObject("pbNeg"));
        r = r && mPan.SerializableObject_ToJSON(rhs.createNestedObject("pan"));
        r = r && mPhaseRestart.SerializableObject_ToJSON(rhs.createNestedObject("phtrig"));
        r = r && mPhase01.SerializableObject_ToJSON(rhs.createNestedObject("phOff"));
        r = r && mPulseWidth.SerializableObject_ToJSON(rhs.createNestedObject("pw"));
        r = r && mFMFeedbackGain.SerializableObject_ToJSON(rhs.createNestedObject("fb"));
        return r;
    }

    Result SerializableObject_Deserialize(JsonVariant obj)
    {
        if (!obj.is<JsonObject>())
        {
            return Result::Failure("must be object");
        }

        Result ret = Result::Success();
        ret.AndRequires(mEnabled.SerializableObject_Deserialize(obj["on"]), "on");
        ret.AndRequires(mWaveform.SerializableObject_Deserialize(obj["wav"]), "wav");
        ret.AndRequires(mVolume.SerializableObject_Deserialize(obj["vol"]), "vol");
        ret.AndRequires(mPortamentoTimeMS.SerializableObject_Deserialize(obj["port"]), "port");
        ret.AndRequires(mFreqMultiplier.SerializableObject_Deserialize(obj["Fmul"]), "Fmul");
        ret.AndRequires(mFreqOffsetHz.SerializableObject_Deserialize(obj["Foff"]), "Foff");
        ret.AndRequires(mFreqParam.SerializableObject_Deserialize(obj["Freq"]), "Freq");
        ret.AndRequires(mHardSyncEnabled.SerializableObject_Deserialize(obj["sync"]), "sync");
        ret.AndRequires(mSyncFreqParam.SerializableObject_Deserialize(obj["syncFreq"]), "syncFreq");
        ret.AndRequires(mPitchSemis.SerializableObject_Deserialize(obj["transp"]), "transp");
        ret.AndRequires(mPitchFine.SerializableObject_Deserialize(obj["fine"]), "fine");
        ret.AndRequires(mPitchBendRangePositive.SerializableObject_Deserialize(obj["pbPos"]), "pbPos");
        ret.AndRequires(mPitchBendRangeNegative.SerializableObject_Deserialize(obj["pbNeg"]), "pbNeg");
        ret.AndRequires(mPan.SerializableObject_Deserialize(obj["pan"]), "pan");
        ret.AndRequires(mPhaseRestart.SerializableObject_Deserialize(obj["phtrig"]), "phtrig");
        ret.AndRequires(mPhase01.SerializableObject_Deserialize(obj["phOff"]), "phOff");
        ret.AndRequires(mPulseWidth.SerializableObject_Deserialize(obj["pw"]), "pw");
        ret.AndRequires(mFMFeedbackGain.SerializableObject_Deserialize(obj["fb"]), "fb");
        return ret;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct LFOSpec //: SerializableDictionary
{
    EnumParam<OscWaveformShape> mWaveShape{gOscWaveformShapeInfo, OscWaveformShape::Sine};
    FloatParam mPulseWidth{0.5f};
    TimeWithBasisParam mSpeed{TimeBasis::Hertz, 1.0f};

    bool SerializableObject_ToJSON(JsonVariant doc) const
    {
        bool ret = true;
        ret = ret && mWaveShape.SerializableObject_ToJSON(doc.createNestedObject("wav"));
        ret = ret && mPulseWidth.SerializableObject_ToJSON(doc.createNestedObject("PW"));
        ret = ret && mSpeed.SerializableObject_ToJSON(doc.createNestedObject("speed"));
        return ret;
    }

    Result SerializableObject_Deserialize(JsonVariant obj)
    {
        if (!obj.is<JsonObject>())
        {
            return Result::Failure("expected object");
        }
        Result ret = Result::Success();

        ret.AndRequires(mWaveShape.SerializableObject_Deserialize(obj["wav"]), "wav");
        ret.AndRequires(mPulseWidth.SerializableObject_Deserialize(obj["PW"]), "PW");
        ret.AndRequires(mSpeed.SerializableObject_Deserialize(obj["speed"]), "speed");

        return ret;
    }

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
struct FilterSettings
{
    EnumParam<ClarinoidFilterType> mType{gClarinoidFilterTypeInfo, ClarinoidFilterType::LP_Moog4};
    FloatParam mQ{0.02f};
    FloatParam mSaturation{0.2f};
    FrequencyParamValue mFrequency{0.3f, 0.0f}; // param, kt amt

    bool SerializableObject_ToJSON(JsonVariant rhs) const
    {
        bool r = true;
        r = r && mType.SerializableObject_ToJSON(rhs.createNestedObject("type"));
        r = r && mQ.SerializableObject_ToJSON(rhs.createNestedObject("q"));
        r = r && mSaturation.SerializableObject_ToJSON(rhs.createNestedObject("sat"));
        r = r && mFrequency.SerializableObject_ToJSON(rhs.createNestedObject("freq"));
        return r;
    }

    Result SerializableObject_Deserialize(JsonVariant obj)
    {
        if (!obj.is<JsonObject>())
        {
            return Result::Failure("must be object");
        }

        Result ret = Result::Success();
        ret.AndRequires(mType.SerializableObject_Deserialize(obj["type"]), "type");
        ret.AndRequires(mQ.SerializableObject_Deserialize(obj["q"]), "q");
        ret.AndRequires(mSaturation.SerializableObject_Deserialize(obj["sat"]), "sat");
        ret.AndRequires(mFrequency.SerializableObject_Deserialize(obj["freq"]), "freq");
        return ret;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct SynthPatch
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

    bool SerializableObject_ToJSON(JsonVariant doc) const
    {
        bool ret = true;
        ret = ret && mName.SerializableObject_ToJSON(doc.createNestedObject("name"));
        ret = ret && mMasterVolume.SerializableObject_ToJSON(doc.createNestedObject("vol"));
        ret = ret && mPan.SerializableObject_ToJSON(doc.createNestedObject("pan"));
        ret = ret && mDelayMix.SerializableObject_ToJSON(doc.createNestedObject("dly"));
        ret = ret && mVerbMix.SerializableObject_ToJSON(doc.createNestedObject("verb"));
        ret = ret && mStereoSpread.SerializableObject_ToJSON(doc.createNestedObject("spread"));
        ret = ret && mDetune.SerializableObject_ToJSON(doc.createNestedObject("det"));
        ret = ret && mFilter.SerializableObject_ToJSON(doc.createNestedObject("flt"));
        ret = ret && mFMStrength2To1.SerializableObject_ToJSON(doc.createNestedObject("fm21"));
        ret = ret && mFMStrength3To1.SerializableObject_ToJSON(doc.createNestedObject("fm31"));
        ret = ret && mFMStrength1To2.SerializableObject_ToJSON(doc.createNestedObject("fm12"));
        ret = ret && mFMStrength3To2.SerializableObject_ToJSON(doc.createNestedObject("fm32"));
        ret = ret && mFMStrength1To3.SerializableObject_ToJSON(doc.createNestedObject("fm13"));
        ret = ret && mFMStrength2To3.SerializableObject_ToJSON(doc.createNestedObject("fm23"));
        ret = ret && mOverallFMStrength.SerializableObject_ToJSON(doc.createNestedObject("fmAll"));
        ret = ret && mVoicingMode.SerializableObject_ToJSON(doc.createNestedObject("vm"));
        ret = ret && SerializeArrayToJSON(doc.createNestedObject("mod"), mModulations);
        ret = ret && SerializeArrayToJSON(doc.createNestedObject("osc"), mOsc);
        ret = ret && SerializeArrayToJSON(doc.createNestedObject("env"), mEnvelopes);
        ret = ret && SerializeArrayToJSON(doc.createNestedObject("lfo"), mLFOs);
        return ret;
    }

    Result SerializableObject_Deserialize(JsonVariant obj)
    {
        if (!obj.is<JsonObject>())
        {
            return Result::Failure("expected object");
        }
        Result ret = Result::Success();

        ret.AndRequires(mName.SerializableObject_Deserialize(obj["name"]), "name");
        ret.AndRequires(mMasterVolume.SerializableObject_Deserialize(obj["vol"]), "vol");
        ret.AndRequires(mPan.SerializableObject_Deserialize(obj["pan"]), "pan");
        ret.AndRequires(mDelayMix.SerializableObject_Deserialize(obj["dly"]), "dly");
        ret.AndRequires(mVerbMix.SerializableObject_Deserialize(obj["verb"]), "verb");
        ret.AndRequires(mStereoSpread.SerializableObject_Deserialize(obj["spread"]), "spread");
        ret.AndRequires(mDetune.SerializableObject_Deserialize(obj["det"]), "det");
        ret.AndRequires(mFilter.SerializableObject_Deserialize(obj["flt"]), "flt");
        ret.AndRequires(mFMStrength2To1.SerializableObject_Deserialize(obj["fm21"]), "fm21");
        ret.AndRequires(mFMStrength3To1.SerializableObject_Deserialize(obj["fm31"]), "fm31");
        ret.AndRequires(mFMStrength1To2.SerializableObject_Deserialize(obj["fm12"]), "fm12");
        ret.AndRequires(mFMStrength3To2.SerializableObject_Deserialize(obj["fm32"]), "fm32");
        ret.AndRequires(mFMStrength1To3.SerializableObject_Deserialize(obj["fm13"]), "fm13");
        ret.AndRequires(mFMStrength2To3.SerializableObject_Deserialize(obj["fm23"]), "fm23");
        ret.AndRequires(mOverallFMStrength.SerializableObject_Deserialize(obj["fmAll"]), "fmAll");
        ret.AndRequires(mVoicingMode.SerializableObject_Deserialize(obj["vm"]), "vm");
        return ret;
    }
};

static constexpr auto synthpatchsize = sizeof(SynthPatch);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct SynthSettings
{
    std::array<SynthPatch, SYNTH_PRESET_COUNT> mPatches{
        initialize_array_with_indices<SynthPatch, SYNTH_PRESET_COUNT>()};

    static void InitBommanoidPreset(SynthPatch &p, const char *name)
    {
        p.mName.SetValue(name);
        p.mFilter.mType.SetValue(ClarinoidFilterType::Disabled);

        p.mOsc[0].mWaveform.SetValue(OscWaveformShape::SawSync);

        p.mFilter.mFrequency.SetParamValue(0);
        p.mFilter.mFrequency.SetKTParamValue(1.0f);

        p.mModulations[0].mDest.SetValue(AnyModulationDestination::MasterVolume);
        p.mModulations[0].mSource.SetValue(AnyModulationSource::ENV1);
        p.mModulations[0].mAuxEnabled.SetValue(false);
        p.mModulations[0].mScaleN11.SetValue(1.0f);
    };

    bool SerializableObject_ToJSON(JsonVariant rhs) const
    {
        return SerializeArrayToJSON(rhs.createNestedArray("patches"), mPatches);
    }

    Result SerializableObject_Deserialize(JsonVariant obj)
    {
        if (!obj.is<JsonObject>())
        {
            return Result::Failure("expected object");
        }

        return DeserializeArray(obj["patches"], mPatches);
    }

    SynthSettings()
    {
        InitBommanoidPreset(mPatches[0], "default");
        InitBommanoidPreset(mPatches[SynthPresetID_Bommanoid], "Bommanoid");
    }
};

} // namespace clarinoid
