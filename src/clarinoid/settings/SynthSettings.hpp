
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{
static constexpr size_t POLYBLEP_OSC_COUNT = 3;

static constexpr float ReasonableOscillatorGain = 0.25f;
static constexpr float ReasonableOscillatorGainForHarm = 0.19f;

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
    {OscWaveformShape::SawSync, "HQ Saw"},
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

EnumItemInfo<ClarinoidFilterType> gClarinoidFilterTypeItems[12] = {
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
};

EnumInfo<ClarinoidFilterType> gClarinoidFilterTypeInfo("FilterType", gClarinoidFilterTypeItems);

enum class ARateModulationSource : uint8_t
{
    // NB: IF you change something here, keep ModulationMatrixNode in sync. (ModulationSourceInfo)
    // these are INDICES used by synthvoice / modulationmatrix
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
    // these are INDICES used by synthvoice / modulationmatrix
    Breath = 0, // K-rate
    PitchStrip, // K-rate
};

EnumItemInfo<KRateModulationSource> gKRateModulationSourceItems[2] = {
    {KRateModulationSource::Breath, "Breath"},
    {KRateModulationSource::PitchStrip, "PitchBend"},
};
static constexpr size_t gKRateModulationSourceCount = SizeofStaticArray(gKRateModulationSourceItems);

EnumInfo<KRateModulationSource> gKRateModulationSourceInfo("KRateModSource", gKRateModulationSourceItems);

enum class AnyModulationSource : uint8_t
{
    None = 0,
    LFO1,       // a-rate
    LFO2,       // a-rate
    ENV1,       // a-rate
    ENV2,       // a-rate
    Osc1FB,     // a-rate
    Osc2FB,     // a-rate
    Osc3FB,     // a-rate
    Breath,     // K-rate
    PitchStrip, // K-rate
    // random trigger
    // keytracking
    // macro1, 2, 3, 4
};

EnumItemInfo<AnyModulationSource> gAnyModulationSourceItems[10] = {
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
    VoiceFilterCutoff = 0, // k-rate
    Osc1Frequency,         // k-rate
    Osc1Amplitude,         // k-rate
    Osc2Frequency,         // k-rate
    Osc2Amplitude,         // k-rate
    Osc3Frequency,         // k-rate
    Osc3Amplitude,         // k-rate
};

EnumItemInfo<KRateModulationDestination> gKRateModulationDestinationItems[7] = {
    {KRateModulationDestination::VoiceFilterCutoff, "VoiceFilterCutoff"},
    {KRateModulationDestination::Osc1Frequency, "Osc1Frequency"},
    {KRateModulationDestination::Osc1Amplitude, "Osc1Amplitude"},
    {KRateModulationDestination::Osc2Frequency, "Osc2Frequency"},
    {KRateModulationDestination::Osc2Amplitude, "Osc2Amplitude"},
    {KRateModulationDestination::Osc3Frequency, "Osc3Frequency"},
    {KRateModulationDestination::Osc3Amplitude, "Osc3Amplitude"},
};

static constexpr size_t gKRateModulationDestinationCount = SizeofStaticArray(gKRateModulationDestinationItems);

EnumInfo<KRateModulationDestination> gKRateModulationDestinationInfo("KRateModDest", gKRateModulationDestinationItems);

enum class AnyModulationDestination : uint8_t
{
    None = 0,
    Osc1PulseWidth, // a-rate
    Osc1Phase,      // a-rate
    Osc2PulseWidth, // a-rate
    Osc2Phase,      // a-rate
    Osc3PulseWidth, // a-rate
    Osc3Phase,      // a-rate

    VoiceFilterCutoff, // k-rate
    Osc1Frequency,     // k-rate
    Osc1Amplitude,     // k-rate  // OUTPUT amplitude
    Osc2Frequency,     // k-rate
    Osc2Amplitude,     // k-rate  // OUTPUT amplitude
    Osc3Frequency,     // k-rate
    Osc3Amplitude,     // k-rate  // OUTPUT amplitude

    // osc FM feedback
    // osc waveform morph (now replaces SYNC freq)
    // osc pan
    // patch verb mix
    // patch delay send
    // patch detune
    // patch spread
    // FM brightness
    // FM strength Osc2 => Osc1
    // FM strength Osc3 => Osc1
    // FM strength Osc1 => Osc2
    // FM strength Osc3 => Osc2
    // FM strength Osc1 => Osc3
    // FM strength Osc2 => Osc3
    // filter stereo spread
    // filter saturation
    // filter Q
    // lfo rate
    // lfo phase
    // lfo wave morph
    // lfo pulse width
    // env delay
    // env attack
    // env hold
    // env decay
    // env sustain
    // env release
};

EnumItemInfo<AnyModulationDestination> gAnyModulationDestinationItems[14] = {
    {AnyModulationDestination::None, "None"},
    {AnyModulationDestination::Osc1PulseWidth, "Osc1PulseWidth"},
    {AnyModulationDestination::Osc1Phase, "Osc1Phase"},
    {AnyModulationDestination::Osc2PulseWidth, "Osc2PulseWidth"},
    {AnyModulationDestination::Osc2Phase, "Osc2Phase"},
    {AnyModulationDestination::Osc3PulseWidth, "Osc3PulseWidth"},
    {AnyModulationDestination::Osc3Phase, "Osc3Phase"},

    {AnyModulationDestination::VoiceFilterCutoff, "VoiceFilterCutoff"},
    {AnyModulationDestination::Osc1Frequency, "Osc1Frequency"},
    {AnyModulationDestination::Osc1Amplitude, "Osc1Amplitude"},
    {AnyModulationDestination::Osc2Frequency, "Osc2Frequency"},
    {AnyModulationDestination::Osc2Amplitude, "Osc2Amplitude"},
    {AnyModulationDestination::Osc3Frequency, "Osc3Frequency"},
    {AnyModulationDestination::Osc3Amplitude, "Osc3Amplitude"},
};

static constexpr size_t gAnyModulationDestinationCount = SizeofStaticArray(gAnyModulationDestinationItems);

EnumInfo<AnyModulationDestination> gAnyModulationDestinationInfo("AnyModDest", gAnyModulationDestinationItems);

enum class ModulationPolarityTreatment : uint8_t
{
    AsPositive01,         // force it to 0-1 positive polarity
    AsBipolar,            // force it to -1,1 bi-polarity
    AsPositive01Inverted, // same as Positive01, but signal is inverted so 0,1 translates 1,0. (1-x)
    AsBipolarInverted,    // same as AsBipolar, but signal is inverted so -1,1 translates 1,-1 (-x)
};

struct SynthModulationSpec
{
    AnyModulationSource mSource = AnyModulationSource::None;
    AnyModulationDestination mDest = AnyModulationDestination::None;
    ModulationPolarityTreatment mSourcePolarity = ModulationPolarityTreatment::AsPositive01;
    int16_t mCurveShape = gModCurveLUT.LinearYIndex; // integral, because we don't interpolate; mod curves are actually
                                                     // discrete. it also simplifies the "0" case where it's linear.
    float mScaleN11 = 0.5f;

    AnyModulationSource mAuxSource = AnyModulationSource::None;
    ModulationPolarityTreatment mAuxPolarity = ModulationPolarityTreatment::AsPositive01;
    bool mAuxEnabled = true; // just allows bypassing without removing the aux source
    int16_t mAuxCurveShape = gModCurveLUT.LinearYIndex;
    float mAuxAmount01 = 0.0f; // amount of attenuation

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
    float mReleaseNoteOnMS = 0.0f;
};

struct SynthOscillatorSettings
{
    float mGain = 0;
    float mPortamentoTime = 0.0f;

    float mFreqMultiplier = 1.0f; // midinotefreq * this
    float mFreqOffset = 0.0f;
    int mPitchSemis = 0;  // semis = integral, transposition. want to keep this integral because the menu system is
                          // not so great at being very precise.
    float mPitchFine = 0; // in semitones, just for detuning

    float mPitchBendRangePositive = 2.0f;
    float mPitchBendRangeNegative = -2.0f;
    float mPitchBendSnap = 0; // 0= no snap, 1=total snap

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

    LFOSpec mLFO1;
    LFOSpec mLFO2;

    float mDetune = 0;

    float mSyncMultMin = 1.4f;
    float mSyncMultMax = 3.0f;
    bool mSync = true;

    bool mDCFilterEnabled = true;
    float mDCFilterCutoff = 10.0f;

    ClarinoidFilterType mFilterType = ClarinoidFilterType::LP_Moog4;
    float mFilterQ = 0.02f;
    float mFilterMaxFreq = 16000.0f;
    float mFilterMinFreq = 0.0f;
    float mFilterSaturation = 0.2f;
    float mFilterKeytracking = 0.0f; // 0 = no keytracking affect. 1.0 = full effect applied, -1.0 = negative effect
                                     // applied (low notes get higher freq cutoff)

    float mFMStrength2To1 = 0;
    float mFMStrength3To1 = 0;
    float mFMStrength1To2 = 0;
    float mFMStrength3To2 = 0;
    float mFMStrength1To3 = 0;
    float mFMStrength2To3 = 0;

    float mOverallFMStrength = 0.1f;

    SynthModulationSpec mModulations[SYNTH_MODULATIONS_MAX];

    SynthPreset()
    {
        mLFO1.mTime.SetFrequency(0.8f);
        mLFO2.mTime.SetFrequency(3.5f);

        mOsc[0].mGain = 0;
        mOsc[1].mGain = ReasonableOscillatorGain;
        mOsc[2].mGain = ReasonableOscillatorGain;

        mOsc[0].mWaveform = OscWaveformShape::VarTriangle; //  // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
        mOsc[1].mWaveform = OscWaveformShape::SawSync;     //  // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
        mOsc[2].mWaveform = OscWaveformShape::VarTriangle; //  // 0 = sine, 1 = var tria, 2 = pwm, 3 = saw sync
    }

    String ToString(int index) const
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

        p.mModulations[0].mDest = AnyModulationDestination::Osc2Frequency;
        p.mModulations[0].mSource = AnyModulationSource::LFO2;
        p.mModulations[0].SetScaleN11_Legacy(0.02f);
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
        p.mOsc[0].mGain *= DecibelsToLinear(-9);
        p.mOsc[0].mWaveform = OscWaveformShape::Pulse;
        p.mOsc[0].mPulseWidth = 0.5f;
        p.mOsc[0].mPitchSemis = -5;

        p.mOsc[1].mPitchSemis = -12;

        p.mLFO2.mTime.SetFrequency(0.7f);

        p.mFilterKeytracking = 0;
        p.mFilterMaxFreq = 12000;
        p.mFilterQ = 0.2f;
        p.mFilterType = ClarinoidFilterType::LP_SEM12;

        p.mModulations[0].mSource = AnyModulationSource::LFO1;
        p.mModulations[0].mDest = AnyModulationDestination::Osc2PulseWidth;
        p.mModulations[0].SetScaleN11_Legacy(0.14f);

        p.mModulations[1].mSource = AnyModulationSource::LFO2;
        p.mModulations[1].mDest = AnyModulationDestination::Osc1PulseWidth;
        p.mModulations[1].SetScaleN11_Legacy(0.20f);

        p.mModulations[2].mSource = AnyModulationSource::LFO1; // helps perturb for overdrive
        p.mModulations[2].mDest = AnyModulationDestination::Osc1Frequency;
        p.mModulations[2].SetScaleN11_Legacy(0.01f);
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

        p.mOsc[0].mWaveform = OscWaveformShape::VarTriangle;
        p.mOsc[0].mPulseWidth = 0;
        p.mOsc[0].mPitchFine = 0.02f;
        p.mOsc[0].mGain = ReasonableOscillatorGain;

        p.mOsc[1].mWaveform = OscWaveformShape::VarTriangle;
        p.mOsc[1].mPulseWidth = 0;
        p.mOsc[1].mPitchFine = 0.02f;
        p.mOsc[1].mGain = ReasonableOscillatorGain;

        p.mOsc[2].mWaveform = OscWaveformShape::VarTriangle;
        p.mOsc[2].mPulseWidth = 0;
        p.mOsc[2].mPitchFine = -0.08f;
        p.mOsc[2].mGain = ReasonableOscillatorGain;
    }

    static void InitBellycrawlPreset(SynthPreset &p)
    {
        // start with defaults
        p.mName = "Bellycrawl";
        p.mModulations[0].SetScaleN11_Legacy(0.9f);
        p.mModulations[0].mSource = AnyModulationSource::LFO1;
        p.mModulations[0].mDest = AnyModulationDestination::Osc2Frequency;

        p.mModulations[1].SetScaleN11_Legacy(0.015f);
        p.mModulations[1].mSource = AnyModulationSource::LFO2;
        p.mModulations[1].mDest = AnyModulationDestination::Osc1Frequency;
    }

    static void InitPanFlutePreset(SynthPreset &p)
    {
        InitBasicLeadPreset("Pan Flute", OscWaveformShape::Pulse, 0.50f, p);
        // make osc1 and osc2 equal
        p.mOsc[1].mGain = p.mOsc[0].mGain = ReasonableOscillatorGain * 0.75f;
        p.mOsc[1].mWaveform = p.mOsc[0].mWaveform = OscWaveformShape::Pulse;
        p.mDetune = 0.04f;

        p.mFilterType = ClarinoidFilterType::BP_Moog4;
        p.mFilterMaxFreq = 11000.0f;
        p.mFilterKeytracking = 1.0f;
        p.mFilterQ = 0.0f;
        p.mFilterSaturation = 0.3f;

        p.mEnv1.mDecayMS = 100;

        p.mModulations[1].mSource = AnyModulationSource::ENV1;
        p.mModulations[1].mDest = AnyModulationDestination::Osc2Frequency;
        p.mModulations[1].SetScaleN11_Legacy(0.05f);
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

        p.mLFO2.mTime = p.mLFO1.mTime;

        p.mModulations[0].mDest = AnyModulationDestination::Osc1PulseWidth;
        p.mModulations[0].mSource = AnyModulationSource::LFO1;
        p.mModulations[0].SetScaleN11_Legacy(0.2f);
        p.mModulations[1].mDest = AnyModulationDestination::Osc1PulseWidth;
        p.mModulations[1].mSource = AnyModulationSource::LFO2;
        p.mModulations[1].SetScaleN11_Legacy(0.2f);
        p.mModulations[2].mDest = AnyModulationDestination::Osc1PulseWidth;
        p.mModulations[2].mSource = AnyModulationSource::LFO1;
        p.mModulations[2].SetScaleN11_Legacy(0.2f);

        p.mModulations[3].mDest = AnyModulationDestination::Osc3Frequency;
        p.mModulations[3].mSource = AnyModulationSource::Breath;
        p.mModulations[3].SetScaleN11_Legacy(-0.03f);

        p.mModulations[4].mDest = AnyModulationDestination::Osc1Frequency;
        p.mModulations[4].mSource = AnyModulationSource::ENV1;
        p.mModulations[4].SetScaleN11_Legacy(0.03f);
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

        p.mLFO2.mTime.SetFrequency(2.0f);

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

        p.mModulations[0].mDest = AnyModulationDestination::Osc1PulseWidth;
        p.mModulations[0].mSource = AnyModulationSource::LFO1;
        p.mModulations[0].SetScaleN11_Legacy(0.2f);
        p.mModulations[1].mDest = AnyModulationDestination::Osc1PulseWidth;
        p.mModulations[1].mSource = AnyModulationSource::LFO2;
        p.mModulations[1].SetScaleN11_Legacy(0.2f);
        p.mModulations[2].mDest = AnyModulationDestination::Osc1PulseWidth;
        p.mModulations[2].mSource = AnyModulationSource::LFO1;
        p.mModulations[2].SetScaleN11_Legacy(0.2f);

        p.mModulations[3].mDest = AnyModulationDestination::Osc3Frequency;
        p.mModulations[3].mSource = AnyModulationSource::Breath;
        p.mModulations[3].SetScaleN11_Legacy(-0.03f);

        p.mModulations[4].mDest = AnyModulationDestination::Osc1Frequency;
        p.mModulations[4].mSource = AnyModulationSource::ENV1;
        p.mModulations[4].SetScaleN11_Legacy(0.03f);
    }

    static void InitSynccyLead(SynthPreset &p)
    {
        p.mName = "Synccy Lead"; // default.
        p.mModulations[0].SetScaleN11_Legacy(0.9f);
        p.mModulations[0].mSource = AnyModulationSource::LFO1;
        p.mModulations[0].mDest = AnyModulationDestination::Osc2Frequency;

        p.mModulations[1].SetScaleN11_Legacy(0.015f);
        p.mModulations[1].mSource = AnyModulationSource::LFO2;
        p.mModulations[1].mDest = AnyModulationDestination::Osc1Frequency;
    }

    static void InitPWMLead2(SynthPreset &p)
    {
        InitBasicLeadPreset("PWM Mono Lead", OscWaveformShape::Pulse, 0.50f, p);
        p.mFilterMaxFreq = 12000;
        p.mFilterType = ClarinoidFilterType::LP_SEM12;
        p.mModulations[0].mSource = AnyModulationSource::LFO1;
        p.mModulations[0].mDest = AnyModulationDestination::Osc2PulseWidth;
        p.mModulations[0].SetScaleN11_Legacy(0.20f);
        p.mModulations[1].mSource = AnyModulationSource::Breath;
        p.mModulations[1].mDest = AnyModulationDestination::Osc2PulseWidth;
        p.mModulations[1].SetScaleN11_Legacy(0.20f);
    }

    static void InitPWMLeadStack(SynthPreset &p)
    {
        InitBasicLeadPreset("PWM Lead Stack", OscWaveformShape::Pulse, 0.50f, p);
        p.mFilterMaxFreq = 12000;
        p.mFilterType = ClarinoidFilterType::LP_SEM12;
        p.mLFO2.mTime.SetFrequency(2.0f);
        p.mOsc[2].mFreqMultiplier = 4.0f;
        p.mOsc[2].mWaveform = OscWaveformShape::Pulse;
        p.mOsc[2].mPulseWidth = 0.5f;
        p.mOsc[2].mGain = .15f;
        p.mModulations[0].mSource = AnyModulationSource::LFO1;
        p.mModulations[0].mDest = AnyModulationDestination::Osc2PulseWidth;
        p.mModulations[0].SetScaleN11_Legacy(0.20f);
        p.mModulations[1].mSource = AnyModulationSource::Breath;
        p.mModulations[1].mDest = AnyModulationDestination::Osc2PulseWidth;
        p.mModulations[1].SetScaleN11_Legacy(0.20f);
        p.mModulations[2].mSource = AnyModulationSource::LFO2;
        p.mModulations[2].mDest = AnyModulationDestination::Osc3PulseWidth;
        p.mModulations[2].SetScaleN11_Legacy(-0.08f);
        p.mModulations[3].mSource = AnyModulationSource::Breath;
        p.mModulations[3].mDest = AnyModulationDestination::Osc3Frequency;
        p.mModulations[3].SetScaleN11_Legacy(0.04f);
    }

    static void InitFluvial(SynthPreset &p)
    {
        p.mName = "Fluvial";
        p.mOsc[0].mGain = 0.0f;
        p.mSync = true;
        p.mSyncMultMin = 0.15f;
        p.mSyncMultMax = 1.95f;
        p.mDetune = 0.0f;
        p.mVerbSend = 0.1f;
        p.mDelaySend = 0.1f;

        p.mOsc[1].mGain = 0.15f; // * gain;
        p.mOsc[1].mWaveform = OscWaveformShape::SawSync;

        p.mOsc[2].mGain = 0.15f; // * gain;
        p.mOsc[2].mPulseWidth = 0.5f;
        p.mOsc[2].mWaveform = OscWaveformShape::Pulse;

        p.mFilterType = ClarinoidFilterType::LP_K35;
        p.mFilterMinFreq = 0.0f;
        p.mFilterMaxFreq = 22000;
        p.mFilterSaturation = 0.60f;
        p.mFilterQ = 0.25f;
        p.mFilterKeytracking = 1.0f;

        p.mModulations[0].mSource = AnyModulationSource::Breath;
        p.mModulations[0].mDest = AnyModulationDestination::Osc3Frequency;
        p.mModulations[0].SetScaleN11_Legacy(-0.02f);
        p.mModulations[1].mSource = AnyModulationSource::Breath;
        p.mModulations[1].mDest = AnyModulationDestination::Osc3PulseWidth;
        p.mModulations[1].SetScaleN11_Legacy(-0.3f);
    }

    static void InitSynthTrumpetPreset(SynthPreset &p)
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
            (ReasonableOscillatorGain / 2.5f); // * DecibelsToLinear(gain);
        p.mOsc[0].mWaveform = p.mOsc[1].mWaveform = p.mOsc[2].mWaveform = OscWaveformShape::VarTriangle;
        p.mOsc[0].mPulseWidth = p.mOsc[1].mPulseWidth = p.mOsc[2].mPulseWidth = 0.05f;

        p.mModulations[0].mSource = AnyModulationSource::ENV1;
        p.mModulations[0].mDest = AnyModulationDestination::Osc1Frequency;
        p.mModulations[0].SetScaleN11_Legacy(0.05f);

        p.mModulations[1].mSource = AnyModulationSource::ENV2;
        p.mModulations[1].mDest = AnyModulationDestination::Osc3Frequency;
        p.mModulations[1].SetScaleN11_Legacy(-0.02f);

        p.mEnv1.mDelayMS = 0;
        p.mEnv1.mAttackMS = 0;
        p.mEnv1.mDecayMS = 100;
        p.mEnv1.mSustainLevel = 0;
        p.mEnv1.mReleaseMS = p.mEnv1.mReleaseNoteOnMS = 0;

        p.mEnv2.mDelayMS = 0;
        p.mEnv2.mAttackMS = 0;
        p.mEnv2.mDecayMS = 500;
        p.mEnv2.mSustainLevel = 0;
        p.mEnv2.mReleaseMS = p.mEnv2.mReleaseNoteOnMS = 0;
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
        InitFluvial(mPresets[i++]);
        InitSynccyLead(mPresets[i++]);
        InitPWMLead2(mPresets[i++]);
        InitPWMLeadStack(mPresets[i++]);
        InitDetunePWMLead(mPresets[i++]);
        InitCloudsStars(mPresets[i++]);
        InitCrystalFieldsPatch(mPresets[i++]);
        InitCinematicTagPatch(mPresets[i++]);
        InitPanFlutePreset(mPresets[i++]);
        InitSynthTrumpetPreset(mPresets[i++]);
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

        mPresets[i].mModulations[1].mSource = AnyModulationSource::ENV1;
        mPresets[i].mModulations[1].mDest = AnyModulationDestination::Osc2Frequency;
        mPresets[i].mModulations[1].SetScaleN11_Legacy(0.16f);
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

        InitPWMLead2(mPresets[SynthPresetID_PWMMono]);
        InitSynccyLead(mPresets[SynthPresetID_SynccyLead]);
        InitCinematicTagPatch(mPresets[SynthPresetID_CinematicTag]);
        InitFluvial(mPresets[SynthPresetID_Fluvial]);
        InitHarmSyncLead(mPresets[SynthPresetID_HarmSync]);
        InitHarmTriLead(mPresets[SynthPresetID_HarmTri]);
        InitHarmPulseLead(mPresets[SynthPresetID_HarmPulse]);
        InitHarmSawLead(mPresets[SynthPresetID_HarmSaw]);
        InitSynthTrumpetPreset(mPresets[SynthPresetID_SynthTrumpetDoubler]);
        InitPanFlutePreset(mPresets[SynthPresetID_PanFlute]);
        InitCrystalFieldsPatch(mPresets[SynthPresetID_Crystal]);
    }
};

} // namespace clarinoid
