
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

static constexpr size_t SynthPresetID_Bommanoid = SYNTH_PRESET_COUNT - 18;
// static constexpr size_t SynthPresetID_FunkyCave = SYNTH_PRESET_COUNT - 17;
// static constexpr size_t SynthPresetID_CrystalSync = SYNTH_PRESET_COUNT - 16;
// static constexpr size_t SynthPresetID_CinematicTagAlt = SYNTH_PRESET_COUNT - 15;
// static constexpr size_t SynthPresetID_SynccyLead = SYNTH_PRESET_COUNT - 14;
// static constexpr size_t SynthPresetID_PWMMono = SYNTH_PRESET_COUNT - 13;
// static constexpr size_t SynthPresetID_Crystal = SYNTH_PRESET_COUNT - 12;
// static constexpr size_t SynthPresetID_CinematicTag = SYNTH_PRESET_COUNT - 11;
// static constexpr size_t SynthPresetID_Fluvial = SYNTH_PRESET_COUNT - 10;
// static constexpr size_t SynthPresetID_PanFlute = SYNTH_PRESET_COUNT - 9;
// static constexpr size_t SynthPresetID_SynthTrumpetDoubler = SYNTH_PRESET_COUNT - 8;
// static constexpr size_t SynthPresetID_MoogBass = SYNTH_PRESET_COUNT - 7;
// static constexpr size_t SynthPresetID_Bassoonoid = SYNTH_PRESET_COUNT - 6;
// static constexpr size_t SynthPresetID_HarmSync = SYNTH_PRESET_COUNT - 5;
// static constexpr size_t SynthPresetID_HarmPulse = SYNTH_PRESET_COUNT - 4;
// static constexpr size_t SynthPresetID_HarmDetunedSaws = SYNTH_PRESET_COUNT - 3;
// static constexpr size_t SynthPresetID_HarmSaw = SYNTH_PRESET_COUNT - 2;
// static constexpr size_t SynthPresetID_HarmTri = SYNTH_PRESET_COUNT - 1;

// static constexpr size_t HarmPresetID_WorldPeace_Eb = HARM_PRESET_COUNT - 1;
// static constexpr size_t HarmPresetID_WorldPeace_Bb = HARM_PRESET_COUNT - 2;
// static constexpr size_t HarmPresetID_WorldPeace_F = HARM_PRESET_COUNT - 3;
// static constexpr size_t HarmPresetID_WorldPeace_Db = HARM_PRESET_COUNT - 4;
// static constexpr size_t HarmPresetID_WorldPeace_Gb = HARM_PRESET_COUNT - 5;
// static constexpr size_t HarmPresetID_Road = HARM_PRESET_COUNT - 6;

enum class HarmScaleRefType : uint8_t
{
    Global,
    Preset,
    Voice,
};

EnumItemInfo<HarmScaleRefType> gHarmScaleRefTypeItems[3] = {
    {HarmScaleRefType::Global, "Global", "g"},
    {HarmScaleRefType::Preset, "Preset", "p"},
    {HarmScaleRefType::Voice, "Voice", "v"},
};

EnumInfo<HarmScaleRefType> gHarmScaleRefTypeInfo("HarmScaleRefType", gHarmScaleRefTypeItems);

////////////////////////////////////////////////////
// enum class NonDiatonicBehavior : uint8_t
// {
//     NextDiatonicNote,
//     PrevDiatonicNote,
//     PreferStay, // keep playing the same note, if it's within range. otherwise change to the next.
//     PreferMove, // change to the next note (think cantaloupe comp that passing B)
//     Drop,       // just don't play this note.
//     // FollowMelodyFromBelow, // so this voice plays a nondiatonic note too, based on distance from lower note
//     // FollowMelodyFromAbove, // so this voice plays a nondiatonic note too, based on distance from upper note
//     TryAlternateScale, // could be interesting to have a list of alternative scales to try. need to have a LUT of
//                        // alternative scales or maybe even just use the scale follower's LUT?
// };

// EnumItemInfo<NonDiatonicBehavior> gNonDiatonicBehaviorItems[6] = {
//     {NonDiatonicBehavior::NextDiatonicNote, "NextDiatonicNote"},
//     {NonDiatonicBehavior::PrevDiatonicNote, "PrevDiatonicNote"},
//     {NonDiatonicBehavior::PreferStay, "PreferStay"},
//     {NonDiatonicBehavior::PreferMove, "PreferMove"},
//     {NonDiatonicBehavior::Drop, "Drop"},
//     {NonDiatonicBehavior::TryAlternateScale, "TryAlternateScale"},
// };

// EnumInfo<NonDiatonicBehavior> gNonDiatonicBehaviorInfo("NonDiatonicBehavior", gNonDiatonicBehaviorItems);

////////////////////////////////////////////////////
enum class NoteOOBBehavior : uint8_t
{
    Mute,
    TransposeOctave
    // transposeoctave_but_drop_if_it_crosses_live
    // keep below live
    // keep above live
};

EnumItemInfo<NoteOOBBehavior> gNoteOOBBehaviorItems[2] = {
    {NoteOOBBehavior::Mute, "Mute", "off"},
    {NoteOOBBehavior::TransposeOctave, "TransposeOctave", "oct"},
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
    {HarmSynthPresetRefType::GlobalA, "GlobalA", "A"},
    {HarmSynthPresetRefType::GlobalB, "GlobalB", "B"},
    {HarmSynthPresetRefType::Preset1, "Preset1", "p1"},
    {HarmSynthPresetRefType::Preset2, "Preset2", "p2"},
    {HarmSynthPresetRefType::Preset3, "Preset3", "p3"},
    {HarmSynthPresetRefType::Preset4, "Preset4", "p4"},
    {HarmSynthPresetRefType::Voice, "Voice", "v"},
};

EnumInfo<HarmSynthPresetRefType> gHarmSynthPresetRefTypeInfo("HarmSynthPresetRefType", gHarmSynthPresetRefTypeItems);

////////////////////////////////////////////////////
// enum class PitchBendParticipation : uint8_t
// {
//     Same,
//     Invert,
//     Off,
// };

// EnumItemInfo<PitchBendParticipation> gPitchBendParticipationItems[3] = {
//     {PitchBendParticipation::Same, "Same"},
//     {PitchBendParticipation::Invert, "Invert"},
//     {PitchBendParticipation::Off, "Off"},
// };

// EnumInfo<PitchBendParticipation> gPitchBendParticipationInfo("PitchBendParticipation", gPitchBendParticipationItems);


////////////////////////////////////////////////////
struct HarmVoiceSettings : ISerializationObjectMap<7>
{
    HarmSequence mSequence;
    EnumParam<HarmSynthPresetRefType> mSynthPatchRef{gHarmSynthPresetRefTypeInfo, HarmSynthPresetRefType::GlobalA};
    IntParam<uint16_t> mSynthPatch{0};

    EnumParam<HarmScaleRefType> mScaleRef{gHarmScaleRefTypeInfo, HarmScaleRefType::Global};
    ScaleParam mLocalScale{{0, ScaleFlavorIndex::Chromatic}};
    MidiNoteRangeParam mOutpRange;

    EnumParam<NoteOOBBehavior> mNoteOOBBehavior{gNoteOOBBehaviorInfo, NoteOOBBehavior::TransposeOctave};

    // EnumParam<NonDiatonicBehavior> mNonDiatonicBehavior = {"NonDiatonicBehavior", gNonDiatonicBehaviorInfo,
    // NonDiatonicBehavior::NextDiatonicNote }; EnumParam<PitchBendParticipation> mPitchBendParticipation =
    // {"PitchBendParticipation", gPitchBendParticipationInfo, PitchBendParticipation::Same };

    virtual SerializationObjectMapArray GetSerializationObjectMap() override
    {
        return {{
            CreateSerializationMapping(mSynthPatchRef, "pR"),
            CreateSerializationMapping(mSynthPatch, "p"),
            CreateSerializationMapping(mScaleRef, "scR"),
            CreateSerializationMapping(mLocalScale, "lS"),
            CreateSerializationMapping(mNoteOOBBehavior, "oob"),
            CreateSerializationMapping(mOutpRange, "rng"),
            CreateSerializationMapping(mSequence, "seq"),
        }};
    }

    // Result SerializableObject_ToJSON(JsonVariant rhs) const
    // {
    //     Result ret = Result::Success();
    //     ret.AndRequires(mSynthPatchRef.SerializableObject_ToJSON(rhs.createNestedObject("pref")), "pref");
    //     ret.AndRequires(mSynthPatch.SerializableObject_ToJSON(rhs.createNestedObject("patch")), "patch");
    //     ret.AndRequires(mScaleRef.SerializableObject_ToJSON(rhs.createNestedObject("sref")), "sref");
    //     ret.AndRequires(mLocalScale.SerializableObject_ToJSON(rhs.createNestedObject("scale")), "scale");
    //     ret.AndRequires(mNoteOOBBehavior.SerializableObject_ToJSON(rhs.createNestedObject("oob")), "oob");
    //     ret.AndRequires(SerializeMidiNoteRange(mMinOutpNote, mMaxOutpNote, rhs.createNestedObject("range")),
    //     "range"); ret.AndRequires(SerializeHarmVoiceNoteSequence(mSequence, mSequenceLength,
    //     rhs.createNestedObject("seq")), "seq"); return ret;
    // }

    String GetMenuDetailString() const
    {
        if (mSequence.mLength < 1)
            return "<off>";
        String ret = "[";
        for (size_t i = 0; i < (size_t)mSequence.mLength - 1; ++i)
        {
            ret += mSequence[i];
            ret += ",";
        }
        ret += mSequence[mSequence.mLength - 1];
        ret += "]";
        return ret;
    }
};

struct HarmPatch : ISerializationObjectMap<10>
{
    StringParam mName{"--"};
    BoolParam mEmitLiveNote{true};
    FloatParam mStereoSeparation{0.1f}; // spreads stereo signal of the voices.
    ScaleParam mPatchScale{{0, ScaleFlavorIndex::Chromatic}};
    std::array<HarmVoiceSettings, HARM_VOICES> mVoiceSettings;

    IntParam<uint32_t> mMinRotationTimeMS{70};
    IntParam<uint16_t> mSynthPatch1{(uint16_t)SynthPresetID_Bommanoid};
    IntParam<uint16_t> mSynthPatch2{(uint16_t)SynthPresetID_Bommanoid};
    IntParam<uint16_t> mSynthPatch3{(uint16_t)SynthPresetID_Bommanoid};
    IntParam<uint16_t> mSynthPatch4{(uint16_t)SynthPresetID_Bommanoid};

    String ToString(int index) const
    {
        return String("") + index + ":" + mName.GetValue();
    }

    void CopyFrom(const HarmPatch &rhs)
    {
        *this = rhs;
    }

    virtual SerializationObjectMapArray GetSerializationObjectMap() override
    {
        return {{
            CreateSerializationMapping(mName, "Name"),
            CreateSerializationMapping(mEmitLiveNote, "el"),
            CreateSerializationMapping(mStereoSeparation, "ster"),
            CreateSerializationMapping(mPatchScale, "psc"),
            CreateSerializationMapping(mMinRotationTimeMS, "minrot"),
            CreateSerializationMapping(mSynthPatch1, "p1"),
            CreateSerializationMapping(mSynthPatch2, "p2"),
            CreateSerializationMapping(mSynthPatch3, "p3"),
            CreateSerializationMapping(mSynthPatch4, "p4"),
            CreateSerializationMapping(mVoiceSettings, "vox"),
        }};
    }
};

struct HarmSettings : ISerializationObjectMap<1>
{
    virtual SerializationObjectMapArray GetSerializationObjectMap() override
    {
        return {{
            CreateSerializationMapping(mPatches, "Patches"),
        }};
    }

    std::array<HarmPatch, HARM_PRESET_COUNT> mPatches;

    // static void InitSlumsHarmPreset(HarmPreset &p)
    // {
    //     p.mName = "Slums Dm";
    //     p.mPresetScale.mRootNoteIndex = Note::D;
    //     p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::Minor;
    //     p.mStereoSeparation = 0.5f;

    //     p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
    //     p.mVoiceSettings[0].mSequenceLength = 1;
    //     p.mVoiceSettings[0].mSequence[0] = -5;
    //     p.mVoiceSettings[0].mNonDiatonicBehavior = NonDiatonicBehavior::PrevDiatonicNote;

    //     p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
    //     p.mVoiceSettings[1].mSequenceLength = 1;
    //     p.mVoiceSettings[1].mSequence[0] = -3;
    //     p.mVoiceSettings[1].mNonDiatonicBehavior = NonDiatonicBehavior::NextDiatonicNote;
    // }

    // static void InitBotanicalHarmPreset(HarmPreset &p)
    // {
    //     p.mName = "Botanical F#m";
    //     p.mPresetScale.mRootNoteIndex = Note::Gb;
    //     p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::HarmonicMinor;
    //     p.mStereoSeparation = 0.5f;
    //     p.mSynthPreset2 = SynthPresetID_HarmDetunedSaws;

    //     p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
    //     p.mVoiceSettings[0].mSequenceLength = 1;
    //     p.mVoiceSettings[0].mSequence[0] = -5;
    //     p.mVoiceSettings[0].mMaxOutpNote = 80;
    //     p.mVoiceSettings[0].mNonDiatonicBehavior = NonDiatonicBehavior::PrevDiatonicNote;

    //     p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
    //     p.mVoiceSettings[1].mSequenceLength = 1;
    //     p.mVoiceSettings[1].mSequence[0] = -3;
    //     p.mVoiceSettings[1].mMaxOutpNote = 80;
    //     p.mVoiceSettings[1].mNonDiatonicBehavior = NonDiatonicBehavior::NextDiatonicNote;
    // }

    // static void InitCrystalFieldsHarmPreset(HarmPreset &p)
    // {
    //     p.mName = "Crystal F#HW";
    //     p.mPresetScale.mRootNoteIndex = Note::Gb;
    //     p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::HalfWholeDiminished;
    //     p.mStereoSeparation = 0.5f;
    //     p.mSynthPreset2 = SynthPresetID_HarmDetunedSaws;

    //     p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
    //     p.mVoiceSettings[0].mSequenceLength = 3;
    //     p.mVoiceSettings[0].mSequence[0] = -1;
    //     p.mVoiceSettings[0].mSequence[1] = -2;
    //     p.mVoiceSettings[0].mSequence[2] = -3;
    //     p.mVoiceSettings[0].mMaxOutpNote = 80;
    //     p.mVoiceSettings[0].mNonDiatonicBehavior = NonDiatonicBehavior::PrevDiatonicNote;

    //     p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
    //     p.mVoiceSettings[1].mSequenceLength = 4;
    //     p.mVoiceSettings[1].mSequence[0] = -4;
    //     p.mVoiceSettings[1].mSequence[1] = -4;
    //     p.mVoiceSettings[1].mSequence[2] = -5;
    //     p.mVoiceSettings[1].mSequence[3] = -5;
    //     p.mVoiceSettings[1].mMaxOutpNote = 80;
    //     p.mVoiceSettings[1].mNonDiatonicBehavior = NonDiatonicBehavior::NextDiatonicNote;
    // }

    // static void InitBellycrawlHarmPreset(HarmPreset &p)
    // {
    //     p.mName = "Bellycrawl Gm";
    //     p.mPresetScale.mRootNoteIndex = Note::G;
    //     p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::HarmonicMinor;
    //     p.mStereoSeparation = 0.5f;
    //     p.mSynthPreset2 = SynthPresetID_HarmDetunedSaws;

    //     p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
    //     p.mVoiceSettings[0].mSequenceLength = 1;
    //     p.mVoiceSettings[0].mSequence[0] = -2;
    //     p.mVoiceSettings[0].mNonDiatonicBehavior = NonDiatonicBehavior::NextDiatonicNote;

    //     p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
    //     p.mVoiceSettings[1].mSequenceLength = 1;
    //     p.mVoiceSettings[1].mSequence[0] = -4;
    //     p.mVoiceSettings[1].mNonDiatonicBehavior = NonDiatonicBehavior::NextDiatonicNote;

    //     p.mVoiceSettings[2].mScaleRef = HarmScaleRefType::Voice;
    //     p.mVoiceSettings[2].mLocalScale.mFlavorIndex = ScaleFlavorIndex::Chromatic;
    //     p.mVoiceSettings[2].mLocalScale.mRootNoteIndex = Note::C;
    //     p.mVoiceSettings[2].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
    //     p.mVoiceSettings[2].mSequenceLength = 1;
    //     p.mVoiceSettings[2].mSequence[0] = -11;
    // }

    // static void InitFunkyHarmPreset(HarmPreset &p)
    // {
    //     p.mName = "Funky D blues";
    //     p.mPresetScale.mRootNoteIndex = Note::D;
    //     p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::Blues;
    //     p.mSynthPreset1 = SynthPresetID_HarmDetunedSaws;

    //     p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
    //     p.mVoiceSettings[0].mSequenceLength = 1;
    //     p.mVoiceSettings[0].mSequence[0] = -3;

    //     p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
    //     p.mVoiceSettings[1].mSequenceLength = 1;
    //     p.mVoiceSettings[1].mSequence[0] = -4;

    //     p.mVoiceSettings[2].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[2].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
    //     p.mVoiceSettings[2].mSequenceLength = 2;
    //     p.mVoiceSettings[2].mSequence[0] = -5;
    //     p.mVoiceSettings[2].mSequence[1] = -6;
    // }

    // static void InitQuartalHarmPreset1(HarmPreset &p)
    // {
    //     p.mName = "Quartals 1";
    //     p.mPresetScale.mRootNoteIndex = Note::C;
    //     p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::Chromatic;
    //     p.mSynthPreset1 = SynthPresetID_HarmDetunedSaws;

    //     p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
    //     p.mVoiceSettings[0].mSequenceLength = 3;
    //     p.mVoiceSettings[0].mSequence[0] = -21; // Eb
    //     p.mVoiceSettings[0].mSequence[1] = -20; // E
    //     p.mVoiceSettings[0].mSequence[2] = -19; // F

    //     p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
    //     p.mVoiceSettings[1].mSequenceLength = 3;
    //     p.mVoiceSettings[1].mSequence[0] = -16; // Ab
    //     p.mVoiceSettings[1].mSequence[1] = -15; // A
    //     p.mVoiceSettings[1].mSequence[2] = -14; // Bb

    //     p.mVoiceSettings[2].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[2].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
    //     p.mVoiceSettings[2].mSequenceLength = 3;
    //     p.mVoiceSettings[2].mSequence[0] = -11; // Db
    //     p.mVoiceSettings[2].mSequence[1] = -10; // D
    //     p.mVoiceSettings[2].mSequence[2] = -9;  // Eb
    // }

    static void InitQuartalHarmPreset2(HarmPatch &p)
    {
        p.mName.SetValue("Quartal Madness");
        p.mPatchScale.mValue.mRootNoteIndex = Note::C;
        p.mPatchScale.mValue.mFlavorIndex = ScaleFlavorIndex::Chromatic;
        // p.mSynthPreset1 = SynthPresetID_HarmDetunedSaws;

        p.mVoiceSettings[0].mScaleRef.SetValue(HarmScaleRefType::Preset);
        p.mVoiceSettings[0].mSynthPatchRef.SetValue(HarmSynthPresetRefType::Preset1);
        p.mVoiceSettings[0].mSequence.mLength = 3;
        p.mVoiceSettings[0].mSequence[0] = -16; // Ab
        p.mVoiceSettings[0].mSequence[1] = -15; // A
        p.mVoiceSettings[0].mSequence[2] = -14; // Bb

        p.mVoiceSettings[1].mScaleRef.SetValue(HarmScaleRefType::Preset);
        p.mVoiceSettings[1].mSynthPatchRef.SetValue(HarmSynthPresetRefType::Preset1);
        p.mVoiceSettings[1].mSequence.mLength = 3;
        p.mVoiceSettings[1].mSequence[0] = -11; // Db
        p.mVoiceSettings[1].mSequence[1] = -10; // D
        p.mVoiceSettings[1].mSequence[2] = -9;  // Eb

        p.mVoiceSettings[2].mScaleRef.SetValue(HarmScaleRefType::Preset);
        p.mVoiceSettings[2].mSynthPatchRef.SetValue(HarmSynthPresetRefType::Preset1);
        p.mVoiceSettings[2].mSequence.mLength = 3;
        p.mVoiceSettings[2].mSequence[0] = -6; // Gb
        p.mVoiceSettings[2].mSequence[1] = -5; // G
        p.mVoiceSettings[2].mSequence[2] = -4; // Ab
    }

    // static void InitFuzionPreset(HarmPreset &p)
    // {
    //     if (HARM_SEQUENCE_LEN < 7)
    //         return;

    //     p.mName = "Fuzion";
    //     p.mPresetScale.mRootNoteIndex = Note::C;
    //     p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::Chromatic;
    //     p.mSynthPreset1 = SynthPresetID_HarmDetunedSaws;

    //     p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
    //     p.mVoiceSettings[0].mSequenceLength = 1;
    //     p.mVoiceSettings[0].mSequence[0] = -4; // Ab

    //     p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
    //     p.mVoiceSettings[1].mSequenceLength = 1;
    //     p.mVoiceSettings[1].mSequence[0] = -9; // Eb

    //     p.mVoiceSettings[2].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[2].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
    //     p.mVoiceSettings[2].mSequenceLength = 1;
    //     p.mVoiceSettings[2].mSequence[0] = -14; // Bb

    //     p.mVoiceSettings[3].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[3].mVoiceSynthPreset = SynthPresetID_HarmSaw;
    //     p.mVoiceSettings[3].mSequenceLength = 7;

    //     p.mVoiceSettings[3].mMaxOutpNote = 40;
    //     p.mVoiceSettings[3].mSequence[0] = 0;
    //     p.mVoiceSettings[3].mSequence[1] = 1;
    //     p.mVoiceSettings[3].mSequence[2] = 2;
    //     p.mVoiceSettings[3].mSequence[3] = 3;
    //     p.mVoiceSettings[3].mSequence[4] = 5;
    //     p.mVoiceSettings[3].mSequence[5] = 9;
    //     p.mVoiceSettings[3].mSequence[6] = 10;
    // }

    // static void InitQuartQuintHarmPreset(HarmPreset &p)
    // {
    //     p.mName = "Quart-Quint";
    //     p.mPresetScale.mRootNoteIndex = Note::C;
    //     p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::Chromatic;
    //     p.mSynthPreset1 = SynthPresetID_HarmDetunedSaws;

    //     p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
    //     p.mVoiceSettings[0].mSequenceLength = 2;
    //     p.mVoiceSettings[0].mSequence[0] = -5; // G
    //     p.mVoiceSettings[0].mSequence[1] = -2; // F

    //     p.mVoiceSettings[1].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
    //     p.mVoiceSettings[1].mSequenceLength = 2;
    //     p.mVoiceSettings[1].mSequence[0] = -10; // D
    //     p.mVoiceSettings[1].mSequence[1] = -14; // Bb

    //     p.mVoiceSettings[3].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[3].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
    //     p.mVoiceSettings[3].mSequenceLength = 2;
    //     p.mVoiceSettings[3].mSequence[0] = -15; // A
    //     p.mVoiceSettings[3].mSequence[1] = -21; // Eb

    //     p.mVoiceSettings[4].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[4].mSynthPresetRef = HarmSynthPresetRefType::Preset1;
    //     p.mVoiceSettings[4].mSequenceLength = 2;
    //     p.mVoiceSettings[4].mSequence[0] = -20; // E
    //     p.mVoiceSettings[4].mSequence[1] = -28; // Ab
    // }

    // static void InitBigPreset(HarmPreset &p)
    // {
    //     p.mName = "Big";
    //     p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
    //     p.mVoiceSettings[0].mSequenceLength = 1;
    //     p.mVoiceSettings[0].mSequence[0] = -7;
    //     p.mVoiceSettings[0].mMaxOutpNote = 80;
    //     p.mVoiceSettings[0].mMinOutpNote = 40;

    //     p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
    //     p.mVoiceSettings[1].mSequenceLength = 1;
    //     p.mVoiceSettings[1].mSequence[0] = -11;
    //     p.mVoiceSettings[1].mMaxOutpNote = 80;
    //     p.mVoiceSettings[1].mMinOutpNote = 40;

    //     p.mVoiceSettings[2].mSynthPresetRef = HarmSynthPresetRefType::Preset4;
    //     p.mVoiceSettings[2].mSequenceLength = 2;
    //     p.mVoiceSettings[2].mMaxOutpNote = 40;
    //     p.mVoiceSettings[2].mSequence[0] = -4;
    //     p.mVoiceSettings[2].mSequence[0] = -9;
    // }

    // static void InitMajInv2Preset(HarmPreset &p)
    // {
    //     p.mName = "maj inv2";
    //     p.mVoiceSettings[0].mSequenceLength = 1;
    //     p.mVoiceSettings[0].mSequence[0] = -3;

    //     p.mVoiceSettings[1].mSequenceLength = 1;
    //     p.mVoiceSettings[1].mSequence[0] = -5;
    // }

    // static void InitMin6Preset(HarmPreset &p)
    // {
    //     p.mName = "Min6/9";
    //     p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
    //     p.mVoiceSettings[0].mSequenceLength = 1;
    //     p.mVoiceSettings[0].mSequence[0] = -2; // Bb
    //     p.mVoiceSettings[0].mMaxOutpNote = 80;
    //     p.mVoiceSettings[0].mMinOutpNote = 40;

    //     p.mVoiceSettings[1].mSynthPresetRef = HarmSynthPresetRefType::Preset3;
    //     p.mVoiceSettings[1].mSequenceLength = 4;
    //     p.mVoiceSettings[1].mSequence[0] = -6; // Gb
    //     p.mVoiceSettings[1].mSequence[1] = -6;
    //     p.mVoiceSettings[1].mSequence[2] = -5; // G
    //     p.mVoiceSettings[1].mSequence[3] = -5;
    //     p.mVoiceSettings[1].mMaxOutpNote = 80;
    //     p.mVoiceSettings[1].mMinOutpNote = 40;

    //     p.mVoiceSettings[2].mSynthPresetRef = HarmSynthPresetRefType::Preset2;
    //     p.mVoiceSettings[2].mSequenceLength = 4;
    //     p.mVoiceSettings[2].mSequence[0] = -11; // Db
    //     p.mVoiceSettings[2].mSequence[1] = -11;
    //     p.mVoiceSettings[2].mSequence[2] = -9; // Eb
    //     p.mVoiceSettings[2].mSequence[3] = -9;
    //     p.mVoiceSettings[2].mMaxOutpNote = 80;
    //     p.mVoiceSettings[2].mMinOutpNote = 40;

    //     p.mVoiceSettings[3].mSynthPresetRef = HarmSynthPresetRefType::Preset4;
    //     p.mVoiceSettings[3].mSequenceLength = 3;
    //     p.mVoiceSettings[3].mMaxOutpNote = 40;
    //     p.mVoiceSettings[3].mSequence[0] = -9; // Eb
    //     p.mVoiceSettings[3].mSequence[1] = -3; // A
    //     p.mVoiceSettings[3].mSequence[2] = -9; // C
    // }

    static void InitBigBandPreset(HarmPatch &p, const char *name, Note scaleRoot)
    {
        p.mName.SetValue(name);                          //"World Peace Eb";
        p.mPatchScale.mValue.mRootNoteIndex = scaleRoot; // Note::Eb;
        p.mPatchScale.mValue.mFlavorIndex = ScaleFlavorIndex::MajorPentatonic;
        p.mStereoSeparation.SetValue(0.7f);

        p.mVoiceSettings[0].mScaleRef.SetValue(HarmScaleRefType::Preset);
        p.mVoiceSettings[0].mSynthPatchRef.SetValue(HarmSynthPresetRefType::Preset2);
        p.mVoiceSettings[0].mSequence.mLength = 2;
        p.mVoiceSettings[0].mSequence[0] = -1; // Bb
        p.mVoiceSettings[0].mSequence[1] = -2; // G

        p.mVoiceSettings[1].mScaleRef.SetValue(HarmScaleRefType::Preset);
        p.mVoiceSettings[1].mSynthPatchRef.SetValue(HarmSynthPresetRefType::Preset3);
        p.mVoiceSettings[1].mSequence.mLength = 2;
        p.mVoiceSettings[1].mSequence[0] = -2; // G
        p.mVoiceSettings[1].mSequence[1] = -3; // F

        p.mVoiceSettings[2].mScaleRef.SetValue(HarmScaleRefType::Preset);
        p.mVoiceSettings[2].mSynthPatchRef.SetValue(HarmSynthPresetRefType::Preset2);
        p.mVoiceSettings[2].mSequence.mLength = 1;
        p.mVoiceSettings[2].mSequence[0] = -4; // Eb

        p.mVoiceSettings[3].mScaleRef.SetValue(HarmScaleRefType::Preset);
        p.mVoiceSettings[3].mSynthPatchRef.SetValue(HarmSynthPresetRefType::Preset4);
        p.mVoiceSettings[3].mSequence.mLength = 2;
        p.mVoiceSettings[3].mSequence[0] = -5; // D
        p.mVoiceSettings[3].mSequence[1] = -7; // Bb
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
    // }

    // void InitOctDownPreset(HarmPreset &p)
    // {
    //     p.mName = "Oct down";
    //     p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::Chromatic;
    //     p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::GlobalA;
    //     p.mVoiceSettings[0].mSequenceLength = 1;
    //     p.mVoiceSettings[0].mSequence[0] = -12;
    // }

    void InitColBassPreset(HarmPatch &p)
    {
        p.mName.SetValue("Col Bass");
        p.mPatchScale.mValue.mFlavorIndex = ScaleFlavorIndex::Chromatic;
        p.mVoiceSettings[0].mOutpRange.mMin = MidiNote{36};
        p.mVoiceSettings[0].mOutpRange.mMax = MidiNote{50};
        p.mVoiceSettings[0].mScaleRef.SetValue(HarmScaleRefType::Preset);
        p.mVoiceSettings[0].mSynthPatchRef.SetValue(HarmSynthPresetRefType::GlobalA);
        // p.mVoiceSettings[0].mVoiceSynthPreset = SynthPresetID_MoogBass;
        p.mVoiceSettings[0].mSequence.mLength = 1;
        p.mVoiceSettings[0].mSequence[0] = -12;
    }

    // void InitSpicePreset(HarmPreset &p)
    // {
    //     p.mName = "Spicy";
    //     p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::Chromatic;
    //     p.mSynthPreset4 = SynthPresetID_HarmSync;
    //     p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
    //     p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset3;
    //     p.mVoiceSettings[0].mSequenceLength = 1;
    //     p.mVoiceSettings[0].mSequence[0] = 24;
    // }
    // void Init5thPreset(HarmPreset &p)
    // {
    //     p.mName = "5th";
    //     p.mPresetScale.mFlavorIndex = ScaleFlavorIndex::Chromatic;
    //     p.mVoiceSettings[0].mScaleRef = HarmScaleRefType::Preset;
    //     p.mSynthPreset4 = SynthPresetID_HarmDetunedSaws;
    //     p.mVoiceSettings[0].mSynthPresetRef = HarmSynthPresetRefType::Preset4;
    //     p.mVoiceSettings[0].mSequenceLength = 1;
    //     p.mVoiceSettings[0].mSequence[0] = 7;
    // }

    // SerializableObject *mSerializableChildObjects[1] = {
    //     &mPatchSerializer,
    // };

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

    HarmSettings()
    {
        size_t iPreset = 1;

        InitBigBandPreset(mPatches[iPreset++], "World Peace Eb", Note::Eb);
        InitColBassPreset(mPatches[iPreset++]);
        InitQuartalHarmPreset2(mPatches[iPreset++]);

        // InitFuzionPreset(mPresets[iPreset++]);

        // InitFunkyHarmPreset(mPresets[iPreset++]);
        // InitMajInv2Preset(mPresets[iPreset++]);
        // InitBigPreset(mPresets[iPreset++]);

        // InitMin6Preset(mPresets[iPreset++]);
        // InitQuartQuintHarmPreset(mPresets[iPreset++]);

        // InitBigBandPreset(mPresets[HarmPresetID_WorldPeace_Db], "World Peace Db", Note::Db);
        // InitBigBandPreset(mPresets[HarmPresetID_WorldPeace_F], "World Peace F", Note::F_);
        // InitBigBandPreset(mPresets[HarmPresetID_WorldPeace_Bb], "World Peace Bb", Note::Bb);
        // InitBigBandPreset(mPresets[HarmPresetID_WorldPeace_Eb], "World Peace Eb", Note::Eb);
        // InitBigBandPreset(mPresets[HarmPresetID_WorldPeace_Gb], "World Peace Gb", Note::Gb);
        // InitRoadPreset(mPresets[HarmPresetID_Road]);
    }
};

// static constexpr auto harmsettingssize = sizeof(HarmSettings);

} // namespace clarinoid
