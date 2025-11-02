
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

// static constexpr size_t SynthPresetID_FunkyCave = SYNTH_PRESET_COUNT - 17;
// static constexpr size_t SynthPresetID_CrystalSync = SYNTH_PRESET_COUNT - 16;
// static constexpr size_t SynthPresetID_CinematicTagAlt = SYNTH_PRESET_COUNT - 15;
// static constexpr size_t SynthPresetID_SynccyLead = SYNTH_PRESET_COUNT - 14;
//  static constexpr size_t SynthPresetID_PWMMono = SYNTH_PRESET_COUNT - 13;
//  static constexpr size_t SynthPresetID_Crystal = SYNTH_PRESET_COUNT - 12;
//  static constexpr size_t SynthPresetID_CinematicTag = SYNTH_PRESET_COUNT - 11;
//  static constexpr size_t SynthPresetID_Fluvial = SYNTH_PRESET_COUNT - 10;
//  static constexpr size_t SynthPresetID_PanFlute = SYNTH_PRESET_COUNT - 9;
//  static constexpr size_t SynthPresetID_SynthTrumpetDoubler = SYNTH_PRESET_COUNT - 8;
//  static constexpr size_t SynthPresetID_HarmSync = SYNTH_PRESET_COUNT - 5;
//  static constexpr size_t SynthPresetID_HarmPulse = SYNTH_PRESET_COUNT - 4;
// static constexpr size_t SynthPresetID_HarmSaw = SYNTH_PRESET_COUNT - 2;

static constexpr size_t SynthPresetID_Fluvial = 0;
static constexpr size_t SynthPresetID_Funky = 1;
static constexpr size_t SynthPresetID_BrakerSolo = 2;
static constexpr size_t SynthPresetID_PWMLead2 = 3;
static constexpr size_t SynthPresetID_PanFlute = 4;
static constexpr size_t SynthPresetID_Trumpet = 5;
static constexpr size_t SynthPresetID_FMTest = 6;
static constexpr size_t SynthPresetID_WobblyCat = 7;
static constexpr size_t SynthPresetID_PulseSync = 8;
static constexpr size_t SynthPresetID_Supersaw = 9;

static constexpr size_t SynthPresetID_MoogBass = SYNTH_PRESET_COUNT - 11;
static constexpr size_t SynthPresetID_Bassoonoid = SYNTH_PRESET_COUNT - 10;

static constexpr size_t SynthPresetID_HarmDetunedPWM = SYNTH_PRESET_COUNT - 4;
static constexpr size_t SynthPresetID_HarmDetunedSaws = SYNTH_PRESET_COUNT - 3;
static constexpr size_t SynthPresetID_HarmTri = SYNTH_PRESET_COUNT - 2;
static constexpr size_t SynthPresetID_HarmFMFB = SYNTH_PRESET_COUNT - 1;

static constexpr size_t HarmPresetID_WorldPeaceGlobal = 0;
static constexpr size_t HarmPresetID_Quintal = 1;
static constexpr size_t HarmPresetID_N3N2 = 2;
// static constexpr size_t HarmPresetID_WorldPeace = HARM_PRESET_COUNT - 1;
// static constexpr size_t HarmPresetID_Road = HARM_PRESET_COUNT - 2;
//  static constexpr size_t HarmPresetID_WorldPeace_Bb = HARM_PRESET_COUNT - 2;
//  static constexpr size_t HarmPresetID_WorldPeace_F = HARM_PRESET_COUNT - 3;
//  static constexpr size_t HarmPresetID_WorldPeace_Db = HARM_PRESET_COUNT - 4;
//  static constexpr size_t HarmPresetID_WorldPeace_Gb = HARM_PRESET_COUNT - 5;

enum class HarmScaleRefType : uint8_t
{
    Global,
    Preset,
    Voice,
};

EnumItemInfo<HarmScaleRefType> gHarmScaleRefTypeItems[3] = {
    {HarmScaleRefType::Global, "Global"},
    {HarmScaleRefType::Preset, "Preset"},
    {HarmScaleRefType::Voice, "Voice"},
};

EnumInfo<HarmScaleRefType> gHarmScaleRefTypeInfo("HarmScaleRefType", gHarmScaleRefTypeItems);

////////////////////////////////////////////////////
enum class NonDiatonicBehavior : uint8_t
{
    // implementing the stateless ones.
    NearestDiatonic, // imagines that the source note snaps to the nearest diatonic note, and uses that as basis for the
                     // harmony voice
    ChromaticFromAbove, // same as NearestDiatonic, but the resulting note gets transposed down the same as the distance
                        // from the playing note from its next higher diatonic note.
    ChromaticFromBelow, // same as NearestDiatonic, but the resulting note gets transposed up the same as the distance
                        // from the playing note from its next lower diatonic note.
    UseScaleFollower, // use the scale follower for an alternate scale; if it's still nondiatonic for that (weird), use
                      // NearestDiatonic behavior.
    Drop,             // just don't play this note.

    // stateful modes could be useful like "keep playing the same note", but let's measure need first.
};

EnumItemInfo<NonDiatonicBehavior> gNonDiatonicBehaviorItems[5] = {
    {NonDiatonicBehavior::NearestDiatonic, "NearestDiatonic"},
    {NonDiatonicBehavior::ChromaticFromAbove, "ChromaticFromAbove"},
    {NonDiatonicBehavior::ChromaticFromBelow, "ChromaticFromBelow"},
    {NonDiatonicBehavior::UseScaleFollower, "UseScaleFollower"},
    {NonDiatonicBehavior::Drop, "Drop"},
};

EnumInfo<NonDiatonicBehavior> gNonDiatonicBehaviorInfo("NonDiatonicBehavior", gNonDiatonicBehaviorItems);

////////////////////////////////////////////////////
enum class NoteOOBBehavior : uint8_t
{
    Mute,
    RotateIntoRange,
    RotateBelowLive, // rotate into range; mute if >= live
    RotateAboveLive, // rotate into range; mute if <= live
};

EnumItemInfo<NoteOOBBehavior> gNoteOOBBehaviorItems[4] = {
    {NoteOOBBehavior::Mute, "Mute"},
    {NoteOOBBehavior::RotateIntoRange, "RotateIntoRange"},
    {NoteOOBBehavior::RotateBelowLive, "RotateBelowLive"},
    {NoteOOBBehavior::RotateAboveLive, "RotateAboveLive"},
};

EnumInfo<NoteOOBBehavior> gNoteOOBBehaviorInfo("NoteOOBBehavior", gNoteOOBBehaviorItems);

////////////////////////////////////////////////////
enum class HarmSynthPresetRefType : uint8_t
{
    GlobalA,
    GlobalB,
    Preset1, // at the preset level i can imagine setting a bass, comp, fx synth presets. they can be used for multiple
             // layers then.
    Preset2,
    Preset3,
    Preset4,
    Voice
};

EnumItemInfo<HarmSynthPresetRefType> gHarmSynthPresetRefTypeItems[7] = {
    {HarmSynthPresetRefType::GlobalA, "GlobalA"},
    {HarmSynthPresetRefType::GlobalB, "GlobalB"},
    {HarmSynthPresetRefType::Preset1, "Preset1"},
    {HarmSynthPresetRefType::Preset2, "Preset2"},
    {HarmSynthPresetRefType::Preset3, "Preset3"},
    {HarmSynthPresetRefType::Preset4, "Preset4"},
    {HarmSynthPresetRefType::Voice, "Voice"},
};

EnumInfo<HarmSynthPresetRefType> gHarmSynthPresetRefTypeInfo("HarmSynthPresetRefType", gHarmSynthPresetRefTypeItems);

////////////////////////////////////////////////////
enum class PitchBendParticipation : uint8_t
{
    Same,
    Invert,
    Off,
};

EnumItemInfo<PitchBendParticipation> gPitchBendParticipationItems[3] = {
    {PitchBendParticipation::Same, "Same"},
    {PitchBendParticipation::Invert, "Invert"},
    {PitchBendParticipation::Off, "Off"},
};

EnumInfo<PitchBendParticipation> gPitchBendParticipationInfo("PitchBendParticipation", gPitchBendParticipationItems);

////////////////////////////////////////////////////
struct HarmVoiceSettings
{
    int8_t mSequence[HARM_SEQUENCE_LEN] = {0};
    uint8_t mSequenceLength = 0;

    HarmSynthPresetRefType mSynthPresetRef = HarmSynthPresetRefType::GlobalA;
    uint16_t mVoiceSynthPreset = 0;

    HarmScaleRefType mScaleRef = HarmScaleRefType::Global;
    Scale mLocalScale = {0, ScaleFlavorIndex::Chromatic};
    int8_t mOctaveTranspose = 0;
    uint8_t mMinOutpNote = 0;
    uint8_t mMaxOutpNote = 127;
    NoteOOBBehavior mNoteOOBBehavior = NoteOOBBehavior::RotateIntoRange;
    NonDiatonicBehavior mNonDiatonicBehavior = NonDiatonicBehavior::ChromaticFromAbove;
    PitchBendParticipation mPitchBendParticipation = PitchBendParticipation::Same;

    String GetMenuDetailString() const
    {
        if (mSequenceLength < 1)
            return "<off>";
        String ret = "[";
        for (size_t i = 0; i < (size_t)mSequenceLength - 1; ++i)
        {
            ret += mSequence[i];
            ret += ",";
        }
        ret += mSequence[mSequenceLength - 1];
        ret += "]";
        return ret;
    }
};

struct HarmPreset
{
    String mName = "--";
    bool mEmitLiveNote = true;
    float mStereoSeparation = 0.1f; // spreads stereo signal of the voices.
    Scale mPresetScale = {0, ScaleFlavorIndex::Chromatic};
    HarmVoiceSettings mVoiceSettings[HARM_VOICES];
    uint32_t mMinRotationTimeMS = 70;
    uint16_t mSynthPreset1 = SynthPresetID_HarmTri;
    uint16_t mSynthPreset2 = SynthPresetID_HarmFMFB;
    uint16_t mSynthPreset3 = SynthPresetID_HarmDetunedSaws;
    uint16_t mSynthPreset4 = SynthPresetID_HarmDetunedPWM;

    String ToString(int index) const
    {
        return String("") + index + ":" + mName;
    }
};

struct HarmSettings
{
    HarmPreset mPresets[HARM_PRESET_COUNT];

    static void InitSlumsHarmPreset(HarmPreset &p)
    {
        p.mName = "Slums";
        // p.mPresetScale.mRootNoteIndex = Note::D;
        // p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::Minor;
        p.mStereoSeparation = 0.5f;

        p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Global;
        p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
        p.mVoiceSettings[0].mSequenceLength = 1;
        p.mVoiceSettings[0].mSequence[0] = -5;
        p.mVoiceSettings[0].mNonDiatonicBehavior = NonDiatonicBehavior::ChromaticFromAbove;

        p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Global;
        p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
        p.mVoiceSettings[1].mSequenceLength = 1;
        p.mVoiceSettings[1].mSequence[0] = -3;
        p.mVoiceSettings[1].mNonDiatonicBehavior = NonDiatonicBehavior::ChromaticFromAbove;
    }

    static void InitBotanicalHarmPreset(HarmPreset &p)
    {
        p.mName = "Botanical F#m";
        p.mPresetScale.mRootNoteIndex = Note::Gb;
        p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::HarmonicMinor;
        p.mStereoSeparation = 0.5f;
        p.mSynthPreset2 = SynthPresetID_HarmDetunedSaws;

        p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
        p.mVoiceSettings[0].mSequenceLength = 1;
        p.mVoiceSettings[0].mSequence[0] = -5;
        p.mVoiceSettings[0].mMaxOutpNote = 80;
        p.mVoiceSettings[0].mNonDiatonicBehavior = NonDiatonicBehavior::ChromaticFromAbove;

        p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
        p.mVoiceSettings[1].mSequenceLength = 1;
        p.mVoiceSettings[1].mSequence[0] = -3;
        p.mVoiceSettings[1].mMaxOutpNote = 80;
        p.mVoiceSettings[1].mNonDiatonicBehavior = NonDiatonicBehavior::ChromaticFromAbove;
    }

    static void InitCrystalFieldsHarmPreset(HarmPreset &p)
    {
        p.mName = "Crystal F#HW";
        p.mPresetScale.mRootNoteIndex = Note::Gb;
        p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::HalfWholeDiminished;
        p.mStereoSeparation = 0.5f;
        p.mSynthPreset2 = SynthPresetID_HarmDetunedSaws;

        p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
        p.mVoiceSettings[0].mSequenceLength = 3;
        p.mVoiceSettings[0].mSequence[0] = -1;
        p.mVoiceSettings[0].mSequence[1] = -2;
        p.mVoiceSettings[0].mSequence[2] = -3;
        p.mVoiceSettings[0].mMaxOutpNote = 80;
        p.mVoiceSettings[0].mNonDiatonicBehavior = NonDiatonicBehavior::NearestDiatonic;

        p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
        p.mVoiceSettings[1].mSequenceLength = 4;
        p.mVoiceSettings[1].mSequence[0] = -4;
        p.mVoiceSettings[1].mSequence[1] = -4;
        p.mVoiceSettings[1].mSequence[2] = -5;
        p.mVoiceSettings[1].mSequence[3] = -5;
        p.mVoiceSettings[1].mMaxOutpNote = 80;
        p.mVoiceSettings[1].mNonDiatonicBehavior = NonDiatonicBehavior::NearestDiatonic;
    }

    static void InitBellycrawlHarmPreset(HarmPreset &p)
    {
        p.mName = "Bellycrawl Gm";
        p.mPresetScale.mRootNoteIndex = Note::G;
        p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::HarmonicMinor;
        p.mStereoSeparation = 0.5f;
        p.mSynthPreset2 = SynthPresetID_HarmDetunedSaws;

        p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
        p.mVoiceSettings[0].mSequenceLength = 1;
        p.mVoiceSettings[0].mSequence[0] = -2;
        p.mVoiceSettings[0].mNonDiatonicBehavior = NonDiatonicBehavior::NearestDiatonic;

        p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
        p.mVoiceSettings[1].mSequenceLength = 1;
        p.mVoiceSettings[1].mSequence[0] = -4;
        p.mVoiceSettings[1].mNonDiatonicBehavior = NonDiatonicBehavior::NearestDiatonic;

        p.mVoiceSettings[2].mScaleRef = HarmScaleRefType::Voice;
        p.mVoiceSettings[2].mLocalScale.mFlavorIndex = ScaleFlavorIndex::Chromatic;
        p.mVoiceSettings[2].mLocalScale.mRootNoteIndex = Note::C;
        p.mVoiceSettings[2].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
        p.mVoiceSettings[2].mSequenceLength = 1;
        p.mVoiceSettings[2].mSequence[0] = -11;
    }

    static void InitFunkyHarmPreset(HarmPreset &p)
    {
        p.mName = "Funky blues";
        p.mPresetScale.mRootNoteIndex = Note::D;
        p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::Blues;
        p.mSynthPreset1 = SynthPresetID_HarmDetunedSaws;

        p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Global;
        p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[0].mSequenceLength = 1;
        p.mVoiceSettings[0].mSequence[0] = -3;

        p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Global;
        p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[1].mSequenceLength = 1;
        p.mVoiceSettings[1].mSequence[0] = -4;

        p.mVoiceSettings[2].mScaleRef = HarmScaleRefType::Global;
        p.mVoiceSettings[2].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[2].mSequenceLength = 2;
        p.mVoiceSettings[2].mSequence[0] = -5;
        p.mVoiceSettings[2].mSequence[1] = -6;
    }

    static void InitFunky2(HarmPreset &p)
    {
        p.mName = "Funky2";
        p.mSynthPreset1 = SynthPresetID_HarmFMFB;

        p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Global;
        p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[0].mSequenceLength = 1;
        p.mVoiceSettings[0].mSequence[0] = 2;

        p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Global;
        p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[1].mSequenceLength = 1;
        p.mVoiceSettings[1].mSequence[0] = 6;
    }

    static void InitFunky3(HarmPreset &p)
    {
        p.mName = "Funky3";
        p.mSynthPreset1 = SynthPresetID_HarmFMFB;

        p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Global;
        p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[0].mSequenceLength = 1;
        p.mVoiceSettings[0].mSequence[0] = -1;

        p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Global;
        p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[1].mSequenceLength = 1;
        p.mVoiceSettings[1].mSequence[0] = 2;
    }

    static void InitQuartalHarmPreset1(HarmPreset &p)
    {
        p.mName = "Quartals 1";
        p.mPresetScale.mRootNoteIndex = Note::C;
        p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::Chromatic;
        p.mSynthPreset1 = SynthPresetID_HarmDetunedSaws;

        p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[0].mSequenceLength = 3;
        p.mVoiceSettings[0].mSequence[0] = -21; // Eb
        p.mVoiceSettings[0].mSequence[1] = -20; // E
        p.mVoiceSettings[0].mSequence[2] = -19; // F

        p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[1].mSequenceLength = 3;
        p.mVoiceSettings[1].mSequence[0] = -16; // Ab
        p.mVoiceSettings[1].mSequence[1] = -15; // A
        p.mVoiceSettings[1].mSequence[2] = -14; // Bb

        p.mVoiceSettings[2].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[2].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[2].mSequenceLength = 3;
        p.mVoiceSettings[2].mSequence[0] = -11; // Db
        p.mVoiceSettings[2].mSequence[1] = -10; // D
        p.mVoiceSettings[2].mSequence[2] = -9;  // Eb
    }

    static void InitQuartalHarmPreset2(HarmPreset &p)
    {
        p.mName = "Quartal Madness";
        p.mPresetScale.mRootNoteIndex = Note::C;
        p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::Chromatic;
        p.mSynthPreset1 = SynthPresetID_HarmDetunedSaws;

        p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[0].mSequenceLength = 3;
        p.mVoiceSettings[0].mSequence[0] = -16; // Ab
        p.mVoiceSettings[0].mSequence[1] = -15; // A
        p.mVoiceSettings[0].mSequence[2] = -14; // Bb

        p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[1].mSequenceLength = 3;
        p.mVoiceSettings[1].mSequence[0] = -11; // Db
        p.mVoiceSettings[1].mSequence[1] = -10; // D
        p.mVoiceSettings[1].mSequence[2] = -9;  // Eb

        p.mVoiceSettings[2].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[2].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[2].mSequenceLength = 3;
        p.mVoiceSettings[2].mSequence[0] = -6; // Gb
        p.mVoiceSettings[2].mSequence[1] = -5; // G
        p.mVoiceSettings[2].mSequence[2] = -4; // Ab
    }

    static void InitFuzionPreset(HarmPreset &p)
    {
        if (HARM_SEQUENCE_LEN < 7)
            return;

        p.mName = "Fuzion";
        p.mPresetScale.mRootNoteIndex = Note::C;
        p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::Chromatic;
        p.mSynthPreset1 = SynthPresetID_HarmDetunedSaws;

        p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[0].mSequenceLength = 1;
        p.mVoiceSettings[0].mSequence[0] = -4; // Ab

        p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[1].mSequenceLength = 1;
        p.mVoiceSettings[1].mSequence[0] = -9; // Eb

        p.mVoiceSettings[2].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[2].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[2].mSequenceLength = 1;
        p.mVoiceSettings[2].mSequence[0] = -14; // Bb

        p.mVoiceSettings[3].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[3].mVoiceSynthPreset = SynthPresetID_HarmDetunedSaws;
        p.mVoiceSettings[3].mSequenceLength = 7;

        p.mVoiceSettings[3].mMaxOutpNote = 40;
        p.mVoiceSettings[3].mSequence[0] = 0;
        p.mVoiceSettings[3].mSequence[1] = 1;
        p.mVoiceSettings[3].mSequence[2] = 2;
        p.mVoiceSettings[3].mSequence[3] = 3;
        p.mVoiceSettings[3].mSequence[4] = 5;
        p.mVoiceSettings[3].mSequence[5] = 9;
        p.mVoiceSettings[3].mSequence[6] = 10;
    }

    static void InitQuartQuintHarmPreset(HarmPreset &p)
    {
        p.mName = "Quart-Quint";
        p.mPresetScale.mRootNoteIndex = Note::C;
        p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::Chromatic;
        p.mSynthPreset1 = SynthPresetID_HarmDetunedSaws;

        p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[0].mSequenceLength = 2;
        p.mVoiceSettings[0].mSequence[0] = -5; // G
        p.mVoiceSettings[0].mSequence[1] = -2; // F

        p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[1].mSequenceLength = 2;
        p.mVoiceSettings[1].mSequence[0] = -10; // D
        p.mVoiceSettings[1].mSequence[1] = -14; // Bb

        p.mVoiceSettings[3].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[3].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[3].mSequenceLength = 2;
        p.mVoiceSettings[3].mSequence[0] = -15; // A
        p.mVoiceSettings[3].mSequence[1] = -21; // Eb

        p.mVoiceSettings[4].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[4].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[4].mSequenceLength = 2;
        p.mVoiceSettings[4].mSequence[0] = -20; // E
        p.mVoiceSettings[4].mSequence[1] = -28; // Ab
    }

    static void InitBigPreset(HarmPreset &p)
    {
        p.mName = "Big";
        p.mPresetScale.mRootNoteIndex = Note::C;
        p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::Chromatic;
        p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[2].mScaleRef = HarmScaleRefType::Preset;

        p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
        p.mVoiceSettings[0].mSequenceLength = 1;
        p.mVoiceSettings[0].mSequence[0] = -7;
        p.mVoiceSettings[0].mMaxOutpNote = 80;
        p.mVoiceSettings[0].mMinOutpNote = 40;

        p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
        p.mVoiceSettings[1].mSequenceLength = 1;
        p.mVoiceSettings[1].mSequence[0] = -11;
        p.mVoiceSettings[1].mMaxOutpNote = 80;
        p.mVoiceSettings[1].mMinOutpNote = 40;

        p.mVoiceSettings[2].mSynthPresetRef = HarmSynthPresetRefType::Preset4;
        p.mVoiceSettings[2].mSequenceLength = 2;
        p.mVoiceSettings[2].mMaxOutpNote = 40;
        p.mVoiceSettings[2].mSequence[0] = -4;
        p.mVoiceSettings[2].mSequence[0] = -9;
    }

    static void InitMajInv2Preset(HarmPreset &p)
    {
        p.mName = "maj inv2";
        p.mVoiceSettings[0].mSequenceLength = 1;
        p.mVoiceSettings[0].mSequence[0] = -3;

        p.mVoiceSettings[1].mSequenceLength = 1;
        p.mVoiceSettings[1].mSequence[0] = -5;
    }

    static void InitMin6Preset(HarmPreset &p)
    {
        p.mName = "Min6/9";
        p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
        p.mVoiceSettings[0].mSequenceLength = 1;
        p.mVoiceSettings[0].mSequence[0] = -2; // Bb
        p.mVoiceSettings[0].mMaxOutpNote = 80;
        p.mVoiceSettings[0].mMinOutpNote = 40;

        p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset3;
        p.mVoiceSettings[1].mSequenceLength = 4;
        p.mVoiceSettings[1].mSequence[0] = -6; // Gb
        p.mVoiceSettings[1].mSequence[1] = -6;
        p.mVoiceSettings[1].mSequence[2] = -5; // G
        p.mVoiceSettings[1].mSequence[3] = -5;
        p.mVoiceSettings[1].mMaxOutpNote = 80;
        p.mVoiceSettings[1].mMinOutpNote = 40;

        p.mVoiceSettings[2].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
        p.mVoiceSettings[2].mSequenceLength = 4;
        p.mVoiceSettings[2].mSequence[0] = -11; // Db
        p.mVoiceSettings[2].mSequence[1] = -11;
        p.mVoiceSettings[2].mSequence[2] = -9; // Eb
        p.mVoiceSettings[2].mSequence[3] = -9;
        p.mVoiceSettings[2].mMaxOutpNote = 80;
        p.mVoiceSettings[2].mMinOutpNote = 40;

        p.mVoiceSettings[3].mSynthPresetRef = HarmSynthPresetRefType::Preset4;
        p.mVoiceSettings[3].mSequenceLength = 3;
        p.mVoiceSettings[3].mMaxOutpNote = 40;
        p.mVoiceSettings[3].mSequence[0] = -9; // Eb
        p.mVoiceSettings[3].mSequence[1] = -3; // A
        p.mVoiceSettings[3].mSequence[2] = -9; // C
    }

    static void InitBigBandPreset(HarmPreset &p, const char *name)
    {
        p.mName = name; //"World Peace";
        // p.mPresetScale.mRootNoteIndex = scaleRoot;//Note::Eb;
        // p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::MajorPentatonic;
        p.mStereoSeparation = 0.7f;

        p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Global;
        p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
        p.mVoiceSettings[0].mSequenceLength = 2;
        p.mVoiceSettings[0].mSequence[0] = -1; // Bb
        p.mVoiceSettings[0].mSequence[1] = -2; // G

        p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Global;
        p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset3;
        p.mVoiceSettings[1].mSequenceLength = 2;
        p.mVoiceSettings[1].mSequence[0] = -2; // G
        p.mVoiceSettings[1].mSequence[1] = -3; // F

        p.mVoiceSettings[2].mScaleRef = HarmScaleRefType::Global;
        p.mVoiceSettings[2].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
        p.mVoiceSettings[2].mSequenceLength = 1;
        p.mVoiceSettings[2].mSequence[0] = -4; // Eb

        p.mVoiceSettings[3].mScaleRef = HarmScaleRefType::Global;
        p.mVoiceSettings[3].mSynthPresetRef = HarmSynthPresetRefType::Preset4;
        p.mVoiceSettings[3].mSequenceLength = 2;
        p.mVoiceSettings[3].mSequence[0] = -5; // D
        p.mVoiceSettings[3].mSequence[1] = -7; // Bb
    }

    static void InitDiatonicPreset(HarmPreset &p, const char *name, int interval1, int interval2)
    {
        p.mName = name;
        p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Global;
        p.mVoiceSettings[0].mNonDiatonicBehavior = NonDiatonicBehavior::NearestDiatonic;
        p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[0].mSequenceLength = 1;
        p.mVoiceSettings[0].mSequence[0] = interval1;

        p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Global;
        p.mVoiceSettings[1].mNonDiatonicBehavior = NonDiatonicBehavior::NearestDiatonic;
        p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
        p.mVoiceSettings[1].mSequenceLength = 1;
        p.mVoiceSettings[1].mSequence[0] = interval2;
    }

    // static void InitRoadPreset(HarmPreset &p)
    // {
    //     p.mName = "Road Ephryg";
    //     p.mPresetScale.mRootNoteIndex = Note::F_;
    //     p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::MajorPentatonic;
    //     p.mStereoSeparation = 0.7f;

    //     p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
    //     p.mVoiceSettings[0].mSequenceLength = 1;
    //     p.mVoiceSettings[0].mSequence[0] = -1; // A

    //     p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset3;
    //     p.mVoiceSettings[1].mSequenceLength = 1;
    //     p.mVoiceSettings[1].mSequence[0] = -3; // F

    //     p.mVoiceSettings[2].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[2].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
    //     p.mVoiceSettings[2].mSequenceLength = 1;
    //     p.mVoiceSettings[2].mSequence[0] = -4; // E

    //     // p.mVoiceSettings[3].mScaleRef = HarmScaleRefType::Preset;
    //     // p.mVoiceSettings[3].mSynthPresetRef = HarmSynthPresetRefType::Preset4;
    //     // p.mVoiceSettings[3].mSequenceLength = 1;
    //     // p.mVoiceSettings[3].mSequence[0] = -5; // D
    // }

    void InitOctDownPreset(HarmPreset &p)
    {
        p.mName = "Oct down";
        p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::Chromatic;
        p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::GlobalA;
        p.mVoiceSettings[0].mSequenceLength = 1;
        p.mVoiceSettings[0].mSequence[0] = -12;
    }

    void InitColBassPreset(HarmPreset &p)
    {
        p.mName = "Col Bass";
        p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::Chromatic;
        p.mVoiceSettings[0].mMaxOutpNote = 36;
        p.mVoiceSettings[0].mMaxOutpNote = 50;
        p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Voice;
        p.mVoiceSettings[0].mVoiceSynthPreset = SynthPresetID_MoogBass;
        p.mVoiceSettings[0].mSequenceLength = 1;
        p.mVoiceSettings[0].mSequence[0] = -12;
    }

    void InitCol8vaPreset(HarmPreset &p)
    {
        p.mName = "Col 8va";
        p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::Chromatic;
        // p.mVoiceSettings[0].mMaxOutpNote = 36;
        // p.mVoiceSettings[0].mMaxOutpNote = 50;
        p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::GlobalA;
        // p.mVoiceSettings[0].mVoiceSynthPreset = SynthPresetID_MoogBass;
        p.mVoiceSettings[0].mSequenceLength = 1;
        p.mVoiceSettings[0].mSequence[0] = 12;
    }

    // void InitColDetSawsPreset(HarmPreset &p)
    // {
    //     p.mName = "Thiccc";
    //     p.mStereoSeparation = 0.5f;
    //     p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::Chromatic;
    //     p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Voice;
    //     p.mVoiceSettings[0].mVoiceSynthPreset = SynthPresetID_HarmDetunedSaws;
    //     p.mVoiceSettings[0].mSequenceLength = 1;
    //     p.mVoiceSettings[0].mSequence[0] = 0;
    // }

    void InitSpicePreset(HarmPreset &p)
    {
        p.mName = "Spicy";
        p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::Chromatic;
        p.mSynthPreset4 = SynthPresetID_HarmDetunedPWM;
        p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
        p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset3;
        p.mVoiceSettings[0].mSequenceLength = 1;
        p.mVoiceSettings[0].mSequence[0] = 24;
    }
    void Init5thPreset(HarmPreset &p)
    {
        p.mName = "5th";
        p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::Chromatic;
        p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
        p.mSynthPreset4 = SynthPresetID_HarmDetunedSaws;
        p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset4;
        p.mVoiceSettings[0].mSequenceLength = 1;
        p.mVoiceSettings[0].mSequence[0] = 7;
    }

    HarmSettings()
    {
        size_t iPreset = 0;

        CCASSERT(iPreset == HarmPresetID_WorldPeaceGlobal);
        InitBigBandPreset(mPresets[iPreset++], "World Peace-glob");

        CCASSERT(iPreset == HarmPresetID_Quintal);
        InitQuartalHarmPreset2(mPresets[iPreset++]);

        CCASSERT(iPreset == HarmPresetID_N3N2);
        InitDiatonicPreset(mPresets[iPreset++], "Dia -3,-2", -3, -2);

        InitQuartQuintHarmPreset(mPresets[iPreset++]);
        InitBigPreset(mPresets[iPreset++]);
        InitFuzionPreset(mPresets[iPreset++]);
        // InitMin6Preset(mPresets[iPreset++]);
        // InitMajInv2Preset(mPresets[iPreset++]);
        // InitFunky3(mPresets[iPreset++]);
        // InitFunky2(mPresets[iPreset++]);
        // InitFunkyHarmPreset(mPresets[iPreset++]);

        // InitColBassPreset(mPresets[iPreset++]);
        InitCol8vaPreset(mPresets[iPreset++]);

        InitDiatonicPreset(mPresets[iPreset++], "Pentatonic -3,-1", -3, -1);
        InitDiatonicPreset(mPresets[iPreset++], "Pentatonic -2,-1", -2, -1);
        InitDiatonicPreset(mPresets[iPreset++], "Pentatonic -2,+1", -2, +1);
        InitDiatonicPreset(mPresets[iPreset++], "Pentatonic -1,+1", -1, +1);
        InitDiatonicPreset(mPresets[iPreset++], "Pentatonic -1,+2", -1, +2);
        InitDiatonicPreset(mPresets[iPreset++], "Pentatonic +1,+2", +1, +2);
        InitDiatonicPreset(mPresets[iPreset++], "Pentatonic +1,+3", +1, +3);

        // InitSpicePreset(mPresets[iPreset++]);

        // InitCrystalFieldsHarmPreset(mPresets[iPreset++]);
        // InitSlumsHarmPreset(mPresets[iPreset++]);
        // InitBotanicalHarmPreset(mPresets[iPreset++]);
        // InitBellycrawlHarmPreset(mPresets[iPreset++]);

        // InitOctDownPreset(mPresets[iPreset++]);
        // InitColDetSawsPreset(mPresets[iPreset++]);
        // Init5thPreset(mPresets[iPreset++]);

        // InitQuartalHarmPreset1(mPresets[iPreset++]); <-- it's nice, but too similar to the other quartal

        // InitBigBandPreset(mPresets[HarmPresetID_WorldPeace], "World Peace");
        //  InitBigBandPreset(mPresets[HarmPresetID_WorldPeace_F], "World Peace F", Note::F_);
        //  InitBigBandPreset(mPresets[HarmPresetID_WorldPeace_Bb], "World Peace Bb", Note::Bb);
        //  InitBigBandPreset(mPresets[HarmPresetID_WorldPeace_Eb], "World Peace Eb", Note::Eb);
        //  InitBigBandPreset(mPresets[HarmPresetID_WorldPeace_Gb], "World Peace Gb", Note::Gb);
        // InitRoadPreset(mPresets[HarmPresetID_Road]);
    }
};

// static constexpr auto harmsettingssize = sizeof(HarmSettings);

} // namespace clarinoid
