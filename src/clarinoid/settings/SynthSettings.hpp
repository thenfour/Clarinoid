
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{
static constexpr size_t POLYBLEP_OSC_COUNT = 3;

enum class TimeBasis : uint8_t
{
    Milliseconds,
    Hertz,
    Half,
    Quarter,
    Eighth,
    Sixteenth,
    ThirtySecond,
    DottedHalf,
    DottedQuarter,
    DottedEighth,
    DottedSixteenth,
    TripletHalf,
    TripletQuarter,
    TripletEighth,
    TripletSixteenth,
};

EnumItemInfo<TimeBasis> gTimeBasisItems[15] = {
    {TimeBasis::Milliseconds, "Milliseconds"},
    {TimeBasis::Hertz, "Hertz"},
    //
    {TimeBasis::Half, "Half"},
    {TimeBasis::Quarter, "Quarter"},
    {TimeBasis::Eighth, "Eighth"},
    {TimeBasis::Sixteenth, "Sixteenth"},
    {TimeBasis::ThirtySecond, "ThirtySecond"},
    //
    {TimeBasis::DottedHalf, "DottedHalf"},
    {TimeBasis::DottedQuarter, "DottedQuarter"},
    {TimeBasis::DottedEighth, "DottedEighth"},
    {TimeBasis::DottedSixteenth, "DottedSixteenth"},
    //
    {TimeBasis::TripletHalf, "TripletHalf"},
    {TimeBasis::TripletQuarter, "TripletQuarter"},
    {TimeBasis::TripletEighth, "TripletEighth"},
    {TimeBasis::TripletSixteenth, "TripletSixteenth"},
};

EnumInfo<TimeBasis> gTimeBasisInfo("TimeBasis", gTimeBasisItems);

// this is necessary to allow "rich" beat/frequency settings to beat-sync LFO for example
struct TimeWithBasis
{
    TimeBasis mBasis = TimeBasis::Milliseconds;
    float mTimeMS = 100;
    float mHz = 6;
    float mTimeBeats = 1;

    void SetFrequency(float hz)
    {
        mBasis = TimeBasis::Hertz;
        mHz = hz;
    }
    float ToHertz(float bpmIfNeeded) const
    {
        switch (mBasis)
        {
        default:
        case TimeBasis::Milliseconds:
            return CycleMSToHertz(mTimeMS);
        case TimeBasis::Hertz:
            return mHz;
        case TimeBasis::Half:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 2, 1)); // 2
        case TimeBasis::Quarter:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 1, 1)); // 1
        case TimeBasis::Eighth:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 1, 2)); // 0.5
        case TimeBasis::Sixteenth:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 1, 4));
        case TimeBasis::ThirtySecond:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 1, 8));
            //
        case TimeBasis::DottedHalf:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 3, 1)); // 3
        case TimeBasis::DottedQuarter:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 3, 2)); // 1.5
        case TimeBasis::DottedEighth:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 3, 4)); // 0.75
        case TimeBasis::DottedSixteenth:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 3, 8));
            //
        case TimeBasis::TripletHalf:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 4, 3)); // 1.33
        case TimeBasis::TripletQuarter:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 4, 6)); //
        case TimeBasis::TripletEighth:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 4, 12));
        case TimeBasis::TripletSixteenth:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 4, 24));
        }
    }
    float ToMS(float bpmIfNeeded) const
    {
        switch (mBasis)
        {
        default:
        case TimeBasis::Milliseconds:
            return mTimeMS;
        case TimeBasis::Hertz:
            return HertzToCycleMS(mHz);
        case TimeBasis::Half:
            return NoteLengthToMS(bpmIfNeeded, 2, 1); // 2
        case TimeBasis::Quarter:
            return NoteLengthToMS(bpmIfNeeded, 1, 1); // 1
        case TimeBasis::Eighth:
            return NoteLengthToMS(bpmIfNeeded, 1, 2); // 0.5
        case TimeBasis::Sixteenth:
            return NoteLengthToMS(bpmIfNeeded, 1, 4);
        case TimeBasis::ThirtySecond:
            return NoteLengthToMS(bpmIfNeeded, 1, 8);
            //
        case TimeBasis::DottedHalf:
            return NoteLengthToMS(bpmIfNeeded, 3, 1); // 3
        case TimeBasis::DottedQuarter:
            return NoteLengthToMS(bpmIfNeeded, 3, 2); // 1.5
        case TimeBasis::DottedEighth:
            return NoteLengthToMS(bpmIfNeeded, 3, 4); // 0.75
        case TimeBasis::DottedSixteenth:
            return NoteLengthToMS(bpmIfNeeded, 3, 8);
            //
        case TimeBasis::TripletHalf:
            return NoteLengthToMS(bpmIfNeeded, 4, 3); // 1.33
        case TimeBasis::TripletQuarter:
            return NoteLengthToMS(bpmIfNeeded, 4, 6); //
        case TimeBasis::TripletEighth:
            return NoteLengthToMS(bpmIfNeeded, 4, 12);
        case TimeBasis::TripletSixteenth:
            return NoteLengthToMS(bpmIfNeeded, 4, 24);
        }
    }
};

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
};

EnumItemInfo<OscWaveformShape> gOscWaveformShapeItems[4] = {
    {OscWaveformShape::Sine, "Sine"},
    {OscWaveformShape::VarTriangle, "Tri-Saw"},
    {OscWaveformShape::Pulse, "Pulse"},
    {OscWaveformShape::SawSync, "Sync Saw"},
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
    {ClarinoidFilterType::Disabled, "Disabled"},
    {ClarinoidFilterType::LP_OnePole, "LP_OnePole"},
    {ClarinoidFilterType::LP_SEM12, "LP_SEM12"},
    {ClarinoidFilterType::LP_Diode, "LP_Diode"},
    {ClarinoidFilterType::LP_K35, "LP_K35"},
    {ClarinoidFilterType::LP_Moog2, "LP_Moog2"},
    {ClarinoidFilterType::LP_Moog4, "LP_Moog4"},
    {ClarinoidFilterType::BP_Moog2, "BP_Moog2"},
    {ClarinoidFilterType::BP_Moog4, "BP_Moog4"},
    {ClarinoidFilterType::HP_OnePole, "HP_OnePole"},
    {ClarinoidFilterType::HP_K35, "HP_K35"},
    {ClarinoidFilterType::HP_Moog2, "HP_Moog2"},
    {ClarinoidFilterType::HP_Moog4, "HP_Moog4"},
    // butterworth 4
    // butterworth 8
};

EnumInfo<ClarinoidFilterType> gClarinoidFilterTypeInfo("FilterType", gClarinoidFilterTypeItems);

enum class ARateModulationSource : uint8_t
{
    // NB: IF you change something here, keep ModulationMatrixNode in sync. (ModulationSourceInfo)
    // these are INDICES used by synthvoice / modulationmatrix. MUST be 0-based, sequential, index-like.
    LFO1 = 0, // a-rate
    LFO2,     // a-rate
    ENV1,     // a-rate
    ENV2,     // a-rate
    Osc1FB,   // a-rate
    Osc2FB,   // a-rate
    Osc3FB,   // a-rate
};

EnumItemInfo<ARateModulationSource> gARateModulationSourceItems[7] = {
    {ARateModulationSource::LFO1, "LFO1"},
    {ARateModulationSource::LFO2, "LFO2"},
    {ARateModulationSource::ENV1, "ENV1"},
    {ARateModulationSource::ENV2, "ENV2"},
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
    ENV1,   // a-rate
    ENV2,   // a-rate
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

EnumItemInfo<AnyModulationSource> gAnyModulationSourceItems[19] = {
    {AnyModulationSource::None, "None"},
    {AnyModulationSource::LFO1, "LFO1"},
    {AnyModulationSource::LFO2, "LFO2"},
    {AnyModulationSource::ENV1, "ENV1"},
    {AnyModulationSource::ENV2, "ENV2"},
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
};

EnumItemInfo<KRateModulationDestination> gKRateModulationDestinationItems[22] = {
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

    // patch detune
    // patch spread
    // filter saturation
    // filter Q
    // lfo rate
    // lfo phase
    // env delay
    // env attack
    // env hold
    // env decay
    // env sustain
    // env release
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
    int16_t mCurveShape = gModCurveLUT.LinearYIndex; // integral, because we don't interpolate; mod curves are actually
                                                     // discrete. it also simplifies the "0" case where it's linear.
    float mScaleN11 = 0.5f;

    AnyModulationSource mAuxSource = AnyModulationSource::None;
    ModulationPolarityTreatment mAuxPolarity = ModulationPolarityTreatment::AsPositive01;
    bool mAuxEnabled = true; // just allows bypassing without removing the aux source
    int16_t mAuxCurveShape = gModCurveLUT.LinearYIndex;
    float mAuxAmount = 0.0f; // amount of attenuation

    // to mimic old behavior with 0 offset and just a scale.
    void SetScaleN11_Legacy(float scaleN11)
    {
        mScaleN11 = scaleN11;
    }

    String ToString() const
    {
        if (mSource == AnyModulationSource::None)
            return "--";
        if (mDest == AnyModulationDestination::None)
            return "--";
        return String(gAnyModulationSourceInfo.GetValueString(mSource)) + ">" +
               gAnyModulationDestinationInfo.GetValueString(mDest);
    }
};

struct EnvelopeSpec
{
    float mDelayMS = 0.0f;
    float mAttackMS = 4.0f;
    float mHoldMS = 0.0f;
    float mDecayMS = 500.0f;
    float mSustainLevel = 0.0f;
    float mReleaseMS = 100.0f;
    bool mLegatoRestart = false;
};

struct SynthOscillatorSettings
{
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
    FrequencyParamValue mFreqParam = {0.3f, 1.0f}; // param, kt amt

    FrequencyParamValue mSyncFreqParam = {0.4f, 1.0f}; // param, kt amt

    // float mFreqParam =
    //     0.3f; // same as filter frequency calc. 0.3 = unity, and each 0.1 param value = 1 octave transposition, when
    //     KT
    //           // = 1. when KT = 0, 0.5 = 1khz, and each 0.1 param value = +/- octave.
    // float mFreqParamKT = 1.0f; // keytracking

    // // these are good for modulation, and for sync frequency.
    // float mSyncFreqParam = 0.3f;
    // float mSyncFreqParamKT = 1.0f; // keytracking

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

    // this depends on the type of waveform (eventually)
    // currently it only controls "curve", where 0.5 is linear.
    float mWaveformMorph01 = 0.5f;
};

struct LFOSpec
{
    OscWaveformShape mWaveShape = OscWaveformShape::Sine;
    float mPulseWidth = 0.5f;
    TimeWithBasis mTime;
    bool mPhaseRestart = false;
    float mWaveformMorph01 = 0.0f;
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

struct SynthPreset
{
    SynthOscillatorSettings mOsc[POLYBLEP_OSC_COUNT];

    String mName = "--";
    VolumeParamValue mMasterVolume;
    float mPan = 0;
    float mDelayMix = 0.08f;
    float mVerbMix = 0.08f;
    float mStereoSpread = 0.15f;

    EnvelopeSpec mEnv1;
    EnvelopeSpec mEnv2;

    VoicingMode mVoicingMode = VoicingMode::Polyphonic;

    LFOSpec mLFO1;
    LFOSpec mLFO2;

    float mDetune = 0;

    // bool mSync = true;

    bool mDCFilterEnabled = true;
    float mDCFilterCutoff = 10.0f;

    ClarinoidFilterType mFilterType = ClarinoidFilterType::LP_Moog4;
    float mFilterQ = 0.02f;
    float mFilterSaturation = 0.2f;
    FrequencyParamValue mFilterFreqParam = {0.3f, 0.0f}; // param, kt amt

    // for FM modulation matrix.
    float mFMStrength2To1 = 0;
    float mFMStrength3To1 = 0;
    float mFMStrength1To2 = 0;
    float mFMStrength3To2 = 0;
    float mFMStrength1To3 = 0;
    float mFMStrength2To3 = 0;

    float mOverallFMStrength = 1.00f;

    SynthModulationSpec mModulations[SYNTH_MODULATIONS_MAX];

    SynthPreset()
    {
        mLFO1.mTime.SetFrequency(0.8f);
        mLFO2.mTime.SetFrequency(3.5f);

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
        String ret = gOscWaveformShapeInfo.GetValueString(mOsc[i].mWaveform);
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
    SynthPreset mPresets[SYNTH_PRESET_COUNT];

    static void InitBommanoidPreset(SynthPreset &p, const char *name)
    {
        p.mName = name;
        p.mDetune = 0.0f;
        p.mStereoSpread = 0;
        p.mFilterType = ClarinoidFilterType::Disabled;

        p.mOsc[0].mWaveform = OscWaveformShape::SawSync;
        // p.mOsc[0].mPortamentoTimeMS = 100;

        // p.mVoicingMode = VoicingMode::Monophonic;
        p.mFilterFreqParam = { 0.0f, 1.0f };

        p.mEnv1.mSustainLevel = 0.2f;
        p.mEnv1.mDecayMS = 1200;
        p.mEnv1.mReleaseMS = 300;
        // p.mEnv1.mReleaseNoteOnMS = 1000;

        p.mModulations[0].mDest = AnyModulationDestination::MasterVolume;
        p.mModulations[0].mSource = AnyModulationSource::ENV1;
        p.mModulations[0].mAuxEnabled = false;
        p.mModulations[0].mScaleN11 = 1.0f;
    };

    // static void InitClarinoid2Preset(SynthPreset &p,
    //                                  const char *name,
    //                                  ClarinoidFilterType filt,
    //                                  float filterKeyScaling,
    //                                  float q,
    //                                  float filterMaxFreq)
    // {
    //     // detuned saw.
    //     p.mName = name;
    //     p.mOsc[0].mGain = 0.0f;
    //     p.mOsc[2].mGain = 0.0f;

    //     p.mOsc[0].mWaveform = OscWaveformShape::SawSync;
    //     p.mOsc[1].mWaveform = OscWaveformShape::SawSync;
    //     p.mOsc[2].mWaveform = OscWaveformShape::SawSync;
    //     p.mOsc[1].mGain = ReasonableOscillatorGain;
    //     p.mSync = false;
    //     p.mDetune = 0.0f;

    //     p.mFilterType = filt;
    //     p.mFilterSaturation = 0;
    //     p.mFilterQ = q;
    //     p.mFilterKeytracking = filterKeyScaling;
    // }

    // static void InitBasicLeadPreset(const char *name, OscWaveformShape shape, float pulseWidth, SynthPreset &p)
    // {
    //     p.mName = name;
    //     p.mOsc[0].mGain = 0.0f;
    //     p.mOsc[2].mGain = 0.0f;
    //     p.mSync = false;
    //     p.mDetune = 0.0f;

    //     p.mOsc[1].mGain = ReasonableOscillatorGain;
    //     p.mOsc[1].mWaveform = shape;
    //     p.mOsc[1].mPulseWidth = pulseWidth;

    //     p.mFilterType = ClarinoidFilterType::LP_Moog4;
    //     p.mFilterSaturation = 0.30f;
    //     p.mFilterQ = 0.25f;
    //     p.mFilterKeytracking = 0.0f;
    // }

    // static void InitDetunedLeadPreset(const char *name, OscWaveformShape shape, float pulseWidth, SynthPreset &p)
    // {
    //     p.mName = name;
    //     p.mDetune = 0.1f;
    //     p.mSync = false;

    //     p.mOsc[0].mGain = ReasonableOscillatorGain;
    //     p.mOsc[0].mWaveform = shape;
    //     p.mOsc[0].mPulseWidth = pulseWidth;

    //     p.mOsc[1].mGain = ReasonableOscillatorGain;
    //     p.mOsc[1].mWaveform = shape;
    //     p.mOsc[1].mPulseWidth = 1.0f - pulseWidth;

    //     p.mOsc[2].mGain = ReasonableOscillatorGain;
    //     p.mOsc[2].mWaveform = shape;
    //     p.mOsc[1].mPulseWidth = pulseWidth;

    //     p.mFilterType = ClarinoidFilterType::LP_Moog4;
    //     p.mFilterSaturation = 0.30f;
    //     p.mFilterQ = 0.25f;
    //     p.mFilterKeytracking = 0.0f;
    // }

    // static void InitFifthLeadPresetA(SynthPreset &p)
    // {
    //     p.mName = "5th lead A";
    //     p.mSync = false;
    //     p.mDetune = 0.0f;

    //     p.mOsc[0].mGain = ReasonableOscillatorGain;
    //     p.mOsc[0].mWaveform = OscWaveformShape::Pulse;
    //     p.mOsc[0].mPulseWidth = 0.08f;
    //     p.mOsc[0].mPitchSemis = -5;

    //     p.mOsc[2].mGain = 0.0f;

    //     p.mOsc[1].mGain = ReasonableOscillatorGain;
    //     p.mOsc[1].mWaveform = OscWaveformShape::Pulse;
    //     p.mOsc[1].mPulseWidth = 0.92f;
    //     p.mOsc[1].mPitchSemis = -12;

    //     p.mFilterType = ClarinoidFilterType::LP_Moog4;
    //     p.mFilterSaturation = 0.0f;
    //     p.mFilterQ = 0.1f;
    //     p.mFilterKeytracking = 0.0f;
    // }

    // static void InitFifthLeadPresetB(SynthPreset &p)
    // {
    //     p.mName = "5th lead B";
    //     p.mSync = false;
    //     p.mDetune = 0.0f;

    //     p.mOsc[0].mGain = ReasonableOscillatorGain;
    //     p.mOsc[0].mWaveform = OscWaveformShape::Pulse;
    //     p.mOsc[0].mPulseWidth = 0.4f;
    //     p.mOsc[0].mPitchSemis = -5;

    //     p.mOsc[2].mGain = 0.0f;

    //     p.mOsc[1].mGain = ReasonableOscillatorGain;
    //     p.mOsc[1].mWaveform = OscWaveformShape::Pulse;
    //     p.mOsc[1].mPitchSemis = -12;

    //     p.mFilterType = ClarinoidFilterType::LP_Moog4;
    //     p.mFilterSaturation = 0.0f;
    //     p.mFilterQ = 0.0f;
    //     p.mFilterKeytracking = 0.0f;
    // }

    // static void InitHarmSyncLead(SynthPreset &p)
    // {
    //     p.mName = "Sync for Harm";
    //     p.mOsc[0].mGain = 0.0f;
    //     p.mOsc[2].mGain = 0.0f;

    //     p.mOsc[1].mGain = ReasonableOscillatorGainForHarm;
    //     p.mOsc[2].mGain = ReasonableOscillatorGainForHarm;

    //     p.mFilterType = ClarinoidFilterType::LP_Moog4;
    //     p.mFilterSaturation = 0;
    //     p.mFilterQ = 0.25f;
    //     p.mFilterKeytracking = 0.0f;
    // }

    // static void InitHarmTriLead(SynthPreset &p)
    // {
    //     p.mName = "Tri for Harm";
    //     p.mOsc[0].mGain = 0.0f;
    //     p.mOsc[2].mGain = 0.0f;

    //     p.mOsc[1].mWaveform = OscWaveformShape::VarTriangle;
    //     p.mOsc[1].mGain = ReasonableOscillatorGainForHarm;
    //     p.mOsc[1].mPulseWidth = 0.5f;
    //     p.mSync = false;
    //     p.mDetune = 0.0f;

    //     p.mFilterType = ClarinoidFilterType::LP_Moog4;
    //     p.mFilterSaturation = 0;
    //     p.mFilterQ = 0.1f;
    //     p.mFilterKeytracking = 0.0f;

    //     p.mModulations[0].mDest = AnyModulationDestination::Osc2Frequency;
    //     p.mModulations[0].mSource = AnyModulationSource::LFO2;
    //     p.mModulations[0].SetScaleN11_Legacy(0.02f);
    // }

    // static void InitHarmPulseLead(SynthPreset &p)
    // {
    //     p.mName = "Pulse for Harm";
    //     p.mOsc[0].mGain = 0.0f;
    //     p.mOsc[2].mGain = 0.0f;

    //     p.mOsc[1].mWaveform = OscWaveformShape::Pulse;
    //     p.mOsc[1].mGain = ReasonableOscillatorGainForHarm;
    //     p.mOsc[1].mPulseWidth = 0.07f;
    //     p.mSync = false;
    //     p.mDetune = 0.0f;

    //     p.mFilterType = ClarinoidFilterType::LP_Moog4;
    //     p.mFilterSaturation = 0;
    //     p.mFilterQ = 0.1f;
    //     p.mFilterKeytracking = 0.0f;
    // }

    // static void InitHarmSawLead(SynthPreset &p)
    // {
    //     p.mName = "Saw for Harm";
    //     p.mOsc[0].mGain = 0.0f;
    //     p.mOsc[2].mGain = 0.0f;

    //     p.mOsc[1].mWaveform = OscWaveformShape::SawSync;
    //     p.mOsc[1].mGain = ReasonableOscillatorGainForHarm;
    //     p.mSync = false;
    //     p.mDetune = 0.0f;

    //     p.mFilterType = ClarinoidFilterType::LP_Moog4;
    //     p.mFilterSaturation = 0;
    //     p.mFilterQ = 0.0f;
    //     p.mFilterKeytracking = 0.0f;
    // }

    // static void InitCrystalFieldsPatch(SynthPreset &p)
    // {
    //     InitBasicLeadPreset("CrystalFields", OscWaveformShape::Pulse, 0.40f, p);
    //     p.mOsc[0].mGain = p.mOsc[1].mGain;
    //     p.mOsc[0].mGain *= DecibelsToLinear(-9);
    //     p.mOsc[0].mWaveform = OscWaveformShape::Pulse;
    //     p.mOsc[0].mPulseWidth = 0.5f;
    //     p.mOsc[0].mPitchSemis = -5;

    //     p.mOsc[1].mPitchSemis = -12;

    //     p.mLFO2.mTime.SetFrequency(0.7f);

    //     p.mFilterKeytracking = 0;
    //     p.mFilterQ = 0.2f;
    //     p.mFilterType = ClarinoidFilterType::LP_SEM12;

    //     p.mModulations[0].mSource = AnyModulationSource::LFO1;
    //     p.mModulations[0].mDest = AnyModulationDestination::Osc2PulseWidth;
    //     p.mModulations[0].SetScaleN11_Legacy(0.14f);

    //     p.mModulations[1].mSource = AnyModulationSource::LFO2;
    //     p.mModulations[1].mDest = AnyModulationDestination::Osc1PulseWidth;
    //     p.mModulations[1].SetScaleN11_Legacy(0.20f);

    //     p.mModulations[2].mSource = AnyModulationSource::LFO1; // helps perturb for overdrive
    //     p.mModulations[2].mDest = AnyModulationDestination::Osc1Frequency;
    //     p.mModulations[2].SetScaleN11_Legacy(0.01f);
    // }

    // static void InitCinematicTagPatch(SynthPreset &p, const char *name, float detuneAmt, float pitchA, float pitchB)
    // {
    //     p.mName = name; //"Cinematic Tag";
    //     p.mSync = false;
    //     p.mDetune = detuneAmt;
    //     p.mFilterKeytracking = 0.8f;

    //     p.mFilterType = ClarinoidFilterType::LP_Moog2;
    //     p.mFilterSaturation = 0.0f;
    //     p.mFilterQ = 0.0f;

    //     p.mOsc[0].mWaveform = OscWaveformShape::VarTriangle;
    //     p.mOsc[0].mPulseWidth = 0;
    //     p.mOsc[0].mPitchFine = pitchA;
    //     p.mOsc[0].mGain = ReasonableOscillatorGain;

    //     p.mOsc[1].mWaveform = OscWaveformShape::VarTriangle;
    //     p.mOsc[1].mPulseWidth = 0;
    //     p.mOsc[1].mPitchFine = pitchA;
    //     p.mOsc[1].mGain = ReasonableOscillatorGain;

    //     p.mOsc[2].mWaveform = OscWaveformShape::VarTriangle;
    //     p.mOsc[2].mPulseWidth = 0;
    //     p.mOsc[2].mPitchFine = pitchB;
    //     p.mOsc[2].mGain = ReasonableOscillatorGain;
    // }

    // static void InitBellycrawlPreset(SynthPreset &p)
    // {
    //     // start with defaults
    //     p.mName = "Bellycrawl";
    //     p.mModulations[0].SetScaleN11_Legacy(0.9f);
    //     p.mModulations[0].mSource = AnyModulationSource::LFO1;
    //     p.mModulations[0].mDest = AnyModulationDestination::Osc2Frequency;

    //     p.mModulations[1].SetScaleN11_Legacy(0.015f);
    //     p.mModulations[1].mSource = AnyModulationSource::LFO2;
    //     p.mModulations[1].mDest = AnyModulationDestination::Osc1Frequency;
    // }

    // static void InitPanFlutePreset(SynthPreset &p)
    // {
    //     InitBasicLeadPreset("Pan Flute", OscWaveformShape::Pulse, 0.50f, p);
    //     // make osc1 and osc2 equal
    //     p.mOsc[1].mGain = p.mOsc[0].mGain = ReasonableOscillatorGain * 0.75f;
    //     p.mOsc[1].mWaveform = p.mOsc[0].mWaveform = OscWaveformShape::Pulse;
    //     p.mDetune = 0.04f;

    //     p.mFilterType = ClarinoidFilterType::BP_Moog4;
    //     p.mFilterKeytracking = 0.8f;
    //     p.mFilterQ = 0.0f;
    //     p.mFilterSaturation = 0.2f;

    //     p.mEnv1.mDecayMS = 115;

    //     p.mModulations[1].mSource = AnyModulationSource::ENV1;
    //     p.mModulations[1].mDest = AnyModulationDestination::Osc2Frequency;
    //     p.mModulations[1].SetScaleN11_Legacy(0.05f);
    // }

    // static void InitFunkyLeadPreset(SynthPreset &p)
    // {
    //     p.mName = "Funky";
    //     p.mSync = true;
    //     // p.mSyncMultMax = 4.0f;
    //     p.mDetune = 0;
    //     p.mFilterQ = 0.40f;

    //     // p.mOsc[0].mFreqMultiplier = 0.9995f;
    //     // p.mOsc[2].mFreqMultiplier = 1.003f;

    //     p.mOsc[2].mWaveform = OscWaveformShape::Pulse;
    //     p.mOsc[2].mPulseWidth = 0.1f;
    //     p.mOsc[2].mGain = ReasonableOscillatorGain;
    // }

    // static void InitDetunePWMLead(SynthPreset &p)
    // {
    //     p.mName = "Detune PWM";
    //     p.mDetune = 0.09f;
    //     p.mStereoSpread = 0.5f;
    //     p.mSync = false;

    //     p.mFilterSaturation = 0.1f;
    //     p.mFilterQ = 0.12f;
    //     // lp moog 4 16k

    //     p.mOsc[0].mWaveform = OscWaveformShape::Pulse;
    //     p.mOsc[0].mPulseWidth = 0.3f;
    //     p.mOsc[0].mGain = ReasonableOscillatorGain / 1.5f;

    //     p.mOsc[1].mWaveform = OscWaveformShape::Pulse;
    //     p.mOsc[1].mPulseWidth = 0.3f;
    //     p.mOsc[1].mGain = ReasonableOscillatorGain / 1.5f;

    //     p.mOsc[2].mWaveform = OscWaveformShape::Pulse;
    //     p.mOsc[2].mPulseWidth = 0.3f;
    //     p.mOsc[2].mGain = ReasonableOscillatorGain / 1.5f;

    //     p.mLFO2.mTime = p.mLFO1.mTime;

    //     p.mModulations[0].mDest = AnyModulationDestination::Osc1PulseWidth;
    //     p.mModulations[0].mSource = AnyModulationSource::LFO1;
    //     p.mModulations[0].SetScaleN11_Legacy(0.2f);
    //     p.mModulations[1].mDest = AnyModulationDestination::Osc1PulseWidth;
    //     p.mModulations[1].mSource = AnyModulationSource::LFO2;
    //     p.mModulations[1].SetScaleN11_Legacy(0.2f);
    //     p.mModulations[2].mDest = AnyModulationDestination::Osc1PulseWidth;
    //     p.mModulations[2].mSource = AnyModulationSource::LFO1;
    //     p.mModulations[2].SetScaleN11_Legacy(0.2f);

    //     p.mModulations[3].mDest = AnyModulationDestination::Osc3Frequency;
    //     p.mModulations[3].mSource = AnyModulationSource::Breath;
    //     p.mModulations[3].SetScaleN11_Legacy(-0.03f);

    //     p.mModulations[4].mDest = AnyModulationDestination::Osc1Frequency;
    //     p.mModulations[4].mSource = AnyModulationSource::ENV1;
    //     p.mModulations[4].SetScaleN11_Legacy(0.03f);
    // }

    // static void InitBassoonoidPreset(SynthPreset &p,
    //                                  const char *name,
    //                                  ClarinoidFilterType filt,
    //                                  float filterKeyScaling,
    //                                  float q,
    //                                  float filterMaxFreq)
    // {
    //     p.mName = name;
    //     p.mSync = false;
    //     p.mDetune = 0.0f;
    //     p.mStereoSpread = 0;
    //     p.mVerbSend = 0;
    //     p.mDelaySend = 0;

    //     p.mOsc[0].mWaveform = OscWaveformShape::Sine;
    //     p.mOsc[1].mWaveform = OscWaveformShape::SawSync;
    //     p.mOsc[2].mWaveform = OscWaveformShape::SawSync;
    //     p.mOsc[0].mPulseWidth = 0.0f;
    //     p.mOsc[1].mPulseWidth = 0.0f;
    //     p.mOsc[2].mPulseWidth = 0.0f;
    //     p.mOsc[0].mGain = 1.0f;
    //     p.mOsc[1].mGain = 1.0f;
    //     p.mOsc[2].mGain = 1.0f;

    //     p.mFilterType = filt;
    //     p.mFilterSaturation = 0;
    //     p.mFilterQ = q;
    //     p.mFilterKeytracking = filterKeyScaling;
    // };

    // static void InitCloudsStars(SynthPreset &p)
    // {
    //     p.mName = "Clouds+Stars";
    //     p.mDetune = 0.09f;
    //     p.mVerbSend = .50f;
    //     p.mDelaySend = .50f;
    //     p.mSync = false;

    //     p.mLFO2.mTime.SetFrequency(2.0f);

    //     p.mFilterType = ClarinoidFilterType::LP_Moog2;
    //     p.mFilterSaturation = 0.16f;
    //     p.mFilterQ = 0.12f;

    //     p.mOsc[0].mWaveform = OscWaveformShape::VarTriangle;
    //     p.mOsc[0].mPulseWidth = 0.0f;
    //     p.mOsc[0].mGain = ReasonableOscillatorGain / 1.5f;

    //     p.mOsc[1].mWaveform = OscWaveformShape::VarTriangle;
    //     p.mOsc[1].mPulseWidth = 0.0f;
    //     p.mOsc[1].mFreqMultiplier = 0.501f;
    //     p.mOsc[1].mGain = ReasonableOscillatorGain / 1.5f;

    //     p.mOsc[2].mWaveform = OscWaveformShape::VarTriangle;
    //     p.mOsc[2].mPulseWidth = 0.0f;
    //     p.mOsc[2].mGain = ReasonableOscillatorGain / 1.5f;

    //     p.mModulations[0].mDest = AnyModulationDestination::Osc1PulseWidth;
    //     p.mModulations[0].mSource = AnyModulationSource::LFO1;
    //     p.mModulations[0].SetScaleN11_Legacy(0.2f);
    //     p.mModulations[1].mDest = AnyModulationDestination::Osc1PulseWidth;
    //     p.mModulations[1].mSource = AnyModulationSource::LFO2;
    //     p.mModulations[1].SetScaleN11_Legacy(0.2f);
    //     p.mModulations[2].mDest = AnyModulationDestination::Osc1PulseWidth;
    //     p.mModulations[2].mSource = AnyModulationSource::LFO1;
    //     p.mModulations[2].SetScaleN11_Legacy(0.2f);

    //     p.mModulations[3].mDest = AnyModulationDestination::Osc3Frequency;
    //     p.mModulations[3].mSource = AnyModulationSource::Breath;
    //     p.mModulations[3].SetScaleN11_Legacy(-0.03f);

    //     p.mModulations[4].mDest = AnyModulationDestination::Osc1Frequency;
    //     p.mModulations[4].mSource = AnyModulationSource::ENV1;
    //     p.mModulations[4].SetScaleN11_Legacy(0.03f);
    // }

    // static void InitSynccyLead(SynthPreset &p)
    // {
    //     p.mName = "Synccy Lead"; // default.
    //     p.mFilterKeytracking = 0.8f;
    //     p.mModulations[0].SetScaleN11_Legacy(0.9f);
    //     p.mModulations[0].mSource = AnyModulationSource::LFO1;
    //     p.mModulations[0].mDest = AnyModulationDestination::Osc2Frequency;

    //     p.mModulations[1].SetScaleN11_Legacy(0.015f);
    //     p.mModulations[1].mSource = AnyModulationSource::LFO2;
    //     p.mModulations[1].mDest = AnyModulationDestination::Osc1Frequency;
    // }

    // static void InitFluvial(SynthPreset &p)
    // {
    //     p.mName = "Fluvial";
    //     p.mOsc[0].mGain = 0.0f;
    //     p.mSync = true;
    //     // p.mSyncMultMin = 0.15f;
    //     // p.mSyncMultMax = 1.95f;
    //     p.mDetune = 0.0f;
    //     p.mVerbSend = 0.1f;
    //     p.mDelaySend = 0.1f;

    //     p.mOsc[1].mGain = 0.15f; // * gain;
    //     p.mOsc[1].mWaveform = OscWaveformShape::SawSync;

    //     p.mOsc[2].mGain = 0.15f; // * gain;
    //     p.mOsc[2].mPulseWidth = 0.5f;
    //     p.mOsc[2].mWaveform = OscWaveformShape::Pulse;

    //     p.mFilterType = ClarinoidFilterType::LP_K35;
    //     p.mFilterSaturation = 0.60f;
    //     p.mFilterQ = 0.25f;
    //     p.mFilterKeytracking = 0.8f;

    //     p.mModulations[0].mSource = AnyModulationSource::Breath;
    //     p.mModulations[0].mDest = AnyModulationDestination::Osc3Frequency;
    //     p.mModulations[0].SetScaleN11_Legacy(-0.02f);
    //     p.mModulations[1].mSource = AnyModulationSource::Breath;
    //     p.mModulations[1].mDest = AnyModulationDestination::Osc3PulseWidth;
    //     p.mModulations[1].SetScaleN11_Legacy(-0.3f);
    // }

    // static void InitCrystalSyncLead(SynthPreset &p)
    // {
    //     p.mName = "CrystalSync"; // default.
    //     p.mModulations[0].SetScaleN11_Legacy(0.9f);
    //     p.mModulations[0].mSource = AnyModulationSource::LFO1;
    //     p.mModulations[0].mDest = AnyModulationDestination::Osc2Frequency;

    //     p.mModulations[1].SetScaleN11_Legacy(0.015f);
    //     p.mModulations[1].mSource = AnyModulationSource::LFO2;
    //     p.mModulations[1].mDest = AnyModulationDestination::Osc1Frequency;

    //     // p.mSyncMultMin = 2.5f;
    //     // p.mSyncMultMax = 0.7f;
    // }

    // static void InitPWMLead2(SynthPreset &p)
    // {
    //     InitBasicLeadPreset("PWM Mono Lead", OscWaveformShape::Pulse, 0.50f, p);
    //     p.mFilterType = ClarinoidFilterType::LP_SEM12;
    //     p.mModulations[0].mSource = AnyModulationSource::LFO1;
    //     p.mModulations[0].mDest = AnyModulationDestination::Osc2PulseWidth;
    //     p.mModulations[0].SetScaleN11_Legacy(0.20f);
    //     p.mModulations[1].mSource = AnyModulationSource::Breath;
    //     p.mModulations[1].mDest = AnyModulationDestination::Osc2PulseWidth;
    //     p.mModulations[1].SetScaleN11_Legacy(0.20f);
    // }

    // static void InitPWMLeadStack(SynthPreset &p)
    // {
    //     InitBasicLeadPreset("PWM Lead Stack", OscWaveformShape::Pulse, 0.50f, p);
    //     p.mFilterType = ClarinoidFilterType::LP_SEM12;
    //     p.mLFO2.mTime.SetFrequency(2.0f);
    //     p.mOsc[2].mFreqMultiplier = 4.0f;
    //     p.mOsc[2].mWaveform = OscWaveformShape::Pulse;
    //     p.mOsc[2].mPulseWidth = 0.5f;
    //     p.mOsc[2].mGain = .15f;
    //     p.mModulations[0].mSource = AnyModulationSource::LFO1;
    //     p.mModulations[0].mDest = AnyModulationDestination::Osc2PulseWidth;
    //     p.mModulations[0].SetScaleN11_Legacy(0.20f);
    //     p.mModulations[1].mSource = AnyModulationSource::Breath;
    //     p.mModulations[1].mDest = AnyModulationDestination::Osc2PulseWidth;
    //     p.mModulations[1].SetScaleN11_Legacy(0.20f);
    //     p.mModulations[2].mSource = AnyModulationSource::LFO2;
    //     p.mModulations[2].mDest = AnyModulationDestination::Osc3PulseWidth;
    //     p.mModulations[2].SetScaleN11_Legacy(-0.08f);
    //     p.mModulations[3].mSource = AnyModulationSource::Breath;
    //     p.mModulations[3].mDest = AnyModulationDestination::Osc3Frequency;
    //     p.mModulations[3].SetScaleN11_Legacy(0.04f);
    // }

    // static void InitSynthTrumpetPreset(SynthPreset &p)
    // {
    //     p.mName = "Trumpet";
    //     p.mSync = false;
    //     p.mStereoSpread = 0.15f;
    //     p.mDetune = 0.04f;
    //     p.mVerbSend = 0.07f;
    //     p.mDelaySend = 0.07f;

    //     p.mFilterType = ClarinoidFilterType::LP_Moog2;
    //     p.mFilterSaturation = 0.80f;
    //     p.mFilterQ = 0.15f;
    //     p.mFilterKeytracking = 0;

    //     p.mOsc[0].mGain = p.mOsc[1].mGain = p.mOsc[2].mGain =
    //         (ReasonableOscillatorGain / 2.5f); // * DecibelsToLinear(gain);
    //     p.mOsc[0].mWaveform = p.mOsc[1].mWaveform = p.mOsc[2].mWaveform = OscWaveformShape::VarTriangle;
    //     p.mOsc[0].mPulseWidth = p.mOsc[1].mPulseWidth = p.mOsc[2].mPulseWidth = 0.05f;

    //     p.mModulations[0].mSource = AnyModulationSource::ENV1;
    //     p.mModulations[0].mDest = AnyModulationDestination::Osc1Frequency;
    //     p.mModulations[0].SetScaleN11_Legacy(0.05f);

    //     p.mModulations[1].mSource = AnyModulationSource::ENV2;
    //     p.mModulations[1].mDest = AnyModulationDestination::Osc3Frequency;
    //     p.mModulations[1].SetScaleN11_Legacy(-0.02f);

    //     p.mEnv1.mDelayMS = 0;
    //     p.mEnv1.mAttackMS = 0;
    //     p.mEnv1.mDecayMS = 100;
    //     p.mEnv1.mSustainLevel = 0;
    //     p.mEnv1.mReleaseMS = 0; // p.mEnv1.mReleaseNoteOnMS = 0;

    //     p.mEnv2.mDelayMS = 0;
    //     p.mEnv2.mAttackMS = 0;
    //     p.mEnv2.mDecayMS = 500;
    //     p.mEnv2.mSustainLevel = 0;
    //     p.mEnv2.mReleaseMS = 0; // p.mEnv2.mReleaseNoteOnMS = 0;
    // }

    // static void InitBraker(SynthPreset &p)
    // {
    //     p.mName = "Braker Solo";
    //     p.mSync = true;
    //     // p.mSyncMultMax = 0.70f;
    //     // p.mSyncMultMin = 0.17f;
    //     p.mDetune = 0;
    //     p.mFilterQ = 0.35f;
    //     p.mFilterSaturation = 0.2f;
    //     p.mFilterType = ClarinoidFilterType::LP_Diode;

    //     p.mOsc[0].mFreqMultiplier = 0.998f;

    //     p.mOsc[2].mFreqMultiplier = 1.003f;

    //     p.mOsc[2].mWaveform = OscWaveformShape::Pulse;
    //     p.mOsc[2].mPulseWidth = 0.47f;
    //     p.mOsc[2].mGain = ReasonableOscillatorGain;

    //     p.mModulations[0].mSource = AnyModulationSource::LFO1;
    //     p.mModulations[0].mDest = AnyModulationDestination::Osc1Frequency;
    //     p.mModulations[0].SetScaleN11_Legacy(0.017f);
    // }

    // static void InitBasicSine(SynthPreset &p)
    // {
    //     p.mName = "INIT SINE";
    //     p.mSync = false;
    //     p.mDetune = 0;
    //     p.mStereoSpread = 0;
    //     p.mFilterQ = 0;
    //     p.mFilterSaturation = 0;
    //     p.mFilterKeytracking = 0;
    //     p.mOsc[0].mWaveform = OscWaveformShape::Sine;
    //     p.mOsc[0].mGain = DecibelsToLinear(-6.0f);
    //     p.mOsc[1].mWaveform = OscWaveformShape::Sine;
    //     p.mOsc[2].mWaveform = OscWaveformShape::Sine;
    //     p.mOsc[1].mGain = 0;
    //     p.mOsc[2].mGain = 0;
    // }

    // static void InitFMTest(SynthPreset &p)
    // {
    //     p.mName = "Braker Solo";
    //     p.mSync = false;
    //     p.mDetune = 0;
    //     p.mStereoSpread = 0;
    //     p.mFMStrength1To2 = 0.50f;
    //     p.mOverallFMStrength = 0.1f;
    //     p.mOsc[0].mWaveform = OscWaveformShape::Sine;
    //     p.mOsc[0].mFreqMultiplier = .5;
    //     // p.mOsc[0].mFreqOffset = 1;
    //     p.mOsc[0].mGain = DecibelsToLinear(-6.0f);

    //     p.mOsc[1].mWaveform = OscWaveformShape::Sine;
    // }

    SynthSettings()
    {
        // size_t i = 0;

        // InitFluvial(mPresets[i++]);

        // // InitFMTest(mPresets[i++]);
        // InitBasicSine(mPresets[i++]);

        // // InitFMPreset(mPresets[i++]);
        // InitSynccyLead(mPresets[i++]);
        // InitPWMLead2(mPresets[i++]);
        // InitPWMLeadStack(mPresets[i++]);
        // InitDetunePWMLead(mPresets[i++]);
        // InitCloudsStars(mPresets[i++]);
        // InitCrystalFieldsPatch(mPresets[i++]);
        // InitCinematicTagPatch(mPresets[i++], "Cinematic", 0.06f, 0.02f, -0.08f);
        // InitPanFlutePreset(mPresets[i++]);
        // InitSynthTrumpetPreset(mPresets[i++]);
        // InitFunkyLeadPreset(mPresets[i++]);
        // InitBraker(mPresets[i++]);
        // InitDetunedLeadPreset("Detuned pulse 08", OscWaveformShape::Pulse, 0.08f, mPresets[i++]);

        // InitFifthLeadPresetA(mPresets[i++]);
        // InitFifthLeadPresetB(mPresets[i++]);

        // // harmonizer-friendly patches

        // i = SynthPresetID_MoogBass;
        // InitBasicLeadPreset("Moog bass", OscWaveformShape::Pulse, 0.50f, mPresets[i]);
        // // make osc1 and osc2 equal
        // mPresets[i].mOsc[2].mGain = mPresets[i].mOsc[1].mGain = mPresets[i].mOsc[0].mGain = ReasonableOscillatorGain;
        // mPresets[i].mOsc[2].mWaveform = mPresets[i].mOsc[1].mWaveform = mPresets[i].mOsc[0].mWaveform =
        //     OscWaveformShape::SawSync;
        // mPresets[i].mOsc[0].mWaveform = OscWaveformShape::Pulse;
        // mPresets[i].mOsc[0].mFreqMultiplier = .5f;
        // mPresets[i].mOsc[1].mFreqMultiplier = .5f;
        // mPresets[i].mOsc[2].mFreqMultiplier = 1.0f;
        // mPresets[i].mDetune = 0.14f;

        // mPresets[i].mEnv1.mDecayMS = 100;

        // mPresets[i].mModulations[1].mSource = AnyModulationSource::ENV1;
        // mPresets[i].mModulations[1].mDest = AnyModulationDestination::Osc2Frequency;
        // mPresets[i].mModulations[1].SetScaleN11_Legacy(0.16f);
        // ++i;

        // InitDetunedLeadPreset(
        //     "Harm: Detsaws", OscWaveformShape::SawSync, 0.5f, mPresets[SynthPresetID_HarmDetunedSaws]);
        // mPresets[SynthPresetID_HarmDetunedSaws].mOsc[0].mGain = ReasonableOscillatorGainForHarm;
        // mPresets[SynthPresetID_HarmDetunedSaws].mOsc[1].mGain = ReasonableOscillatorGainForHarm;
        // mPresets[SynthPresetID_HarmDetunedSaws].mOsc[2].mGain = ReasonableOscillatorGainForHarm;
        // mPresets[SynthPresetID_HarmDetunedSaws].mFilterQ = 0;
        // mPresets[SynthPresetID_HarmDetunedSaws].mFilterType = ClarinoidFilterType::BP_Moog4;

        // InitBassoonoidPreset(
        //     mPresets[SynthPresetID_Bassoonoid], "Diode-ks7-q15", ClarinoidFilterType::LP_Diode, 0.7f, 0.15f, 15000);

        InitBommanoidPreset(mPresets[SynthPresetID_Bommanoid], "Bommanoid");

        // InitPWMLead2(mPresets[SynthPresetID_PWMMono]);
        // InitCrystalSyncLead(mPresets[SynthPresetID_CrystalSync]);
        // InitSynccyLead(mPresets[SynthPresetID_SynccyLead]);
        // InitCinematicTagPatch(mPresets[SynthPresetID_CinematicTag], "SynthwaveB", 0.06f, 0.025f, -0.09f);
        // InitFluvial(mPresets[SynthPresetID_Fluvial]);
        // InitHarmSyncLead(mPresets[SynthPresetID_HarmSync]);
        // InitHarmTriLead(mPresets[SynthPresetID_HarmTri]);
        // InitHarmPulseLead(mPresets[SynthPresetID_HarmPulse]);
        // InitHarmSawLead(mPresets[SynthPresetID_HarmSaw]);
        // InitSynthTrumpetPreset(mPresets[SynthPresetID_SynthTrumpetDoubler]);
        // InitPanFlutePreset(mPresets[SynthPresetID_PanFlute]);
        // InitCrystalFieldsPatch(mPresets[SynthPresetID_Crystal]);
        // InitCinematicTagPatch(mPresets[SynthPresetID_CinematicTagAlt], "SynthwaveA", 0.1f, 0.015f, -0.07f);
        // InitBraker(mPresets[SynthPresetID_FunkyCave]);
    }
};

} // namespace clarinoid
