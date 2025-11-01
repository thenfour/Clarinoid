
#pragma once

#include <initializer_list>
#include "Enum.hpp"

namespace clarinoid
{

// don't use a LUT because we want to support pitch bend and glides and stuff. using a LUT + interpolation would be
// asinine.
inline float MIDINoteToFreq(float x)
{
    float a = 440;
    constexpr float kOneTwelfth = 1.0f / 12.0f;
    constexpr float kOneThirtySecond = 1.0f / 32.0f;
    return (a * kOneThirtySecond) * fast::pow(2.0f, (((float)x - 9.0f) * kOneTwelfth));
}

////////////////////////////////////////////////////
struct NoteDesc
{
    uint8_t mIndex;
    const char *mName;
    const char *mNameWithCustomGlyphs;
};

NoteDesc const gNotes[12] = {
    {0, "C ", "C "},
    {1, "C#", "C" CHARSTR_SHARP},
    {2, "D ", "D "},
    {3, "D#", "D" CHARSTR_SHARP},
    {4, "E ", "E "},
    {5, "F ", "F "},
    {6, "F#", "F" CHARSTR_SHARP},
    {7, "G ", "G "},
    {8, "G#", "G" CHARSTR_SHARP},
    {9, "A ", "A "},
    {10, "A#", "A" CHARSTR_SHARP},
    {11, "B ", "B "},
};

enum class Note : uint8_t
{
    C = 0,
    Db = 1,
    D = 2,
    Eb = 3,
    E = 4,
    F_ = 5, // damned arduino.h getting in the way.
    Gb = 6,
    G = 7,
    Ab = 8,
    A = 9,
    Bb = 10,
    B = 11
};

EnumItemInfo<Note> gNoteItems[12] = {
    {Note::C, "C"},
    {Note::Db, "Db"},
    {Note::D, "D"},
    {Note::Eb, "Eb"},
    {Note::E, "E"},
    {Note::F_, "F_"},
    {Note::Gb, "Gb"},
    {Note::G, "G"},
    {Note::Ab, "Ab"},
    {Note::A, "A"},
    {Note::Bb, "Bb"},
    {Note::B, "B"},
};

EnumInfo<Note> gNoteInfo("Note", gNoteItems);

////////////////////////////////////////////////////
class MidiNote
{
    uint8_t mValue = 0;     // 0-127 midi note value
    uint8_t mNoteIndex = 0; // 0-11 gNote index (aka pitch class)
    uint8_t mOctave = 0;

  public:
    MidiNote() = default;
    MidiNote(uint8_t val) : mValue(val)
    {
        // for reference, 36 = C2
        // but MIDI "octaves" are like, not very handy because A0 is val 21.
        // which makes C1 = 24
        // which makens C0 = 12
        // so... what's below that???
        // for mathematical simplicity
        DivRem<uint8_t, 12>(val, mOctave, mNoteIndex);
    }

    MidiNote(uint8_t oct, uint8_t note) : mValue(oct * 12 + note), mNoteIndex(note), mOctave(oct)
    {
    }

    MidiNote(uint8_t oct, Note note) : mValue(oct * 12 + (uint8_t)note), mNoteIndex((uint8_t)note), mOctave(oct)
    {
    }

    const char *ToString() const
    {
        return gNotes[mNoteIndex].mNameWithCustomGlyphs;
    }
    String ToStringWithOctave() const
    {
        return String(gNotes[mNoteIndex].mNameWithCustomGlyphs) + mOctave;
    }

    uint8_t GetMidiValue() const
    {
        return mValue;
    }
    uint8_t GetNoteIndex() const // returns chromatic note index 0-11
    {
        return mNoteIndex;
    }
    const NoteDesc &GetNoteDesc() const
    {
        return gNotes[mNoteIndex];
    }
    uint8_t GetOctave() const
    {
        return mOctave;
    }
};

////////////////////////////////////////////////////
// NOTE: only want 16 (4 bits) max of these which participate in scale follower to help save space with the scale
// follower LUTs put the scale follower ones first for this reason.
enum class ScaleFlavorIndex : uint8_t // match index to gScaleFlavors
{
    Chromatic,
    Major, // also minor
    Minor,
    MelodicMinor, // also altered
    HarmonicMinor,
    MajorPentatonic,
    MinorPentatonic,
    WholeTone,
    HalfWholeDiminished,
    WholeHalfDiminished,
    Altered,
    Blues,
    Unison, // used by scale follower when there's no info
    Power,
    ScaleFlavorCount
    // unisono
    // 10 chord: 9
    // 11 chord: maj13 (maj minus 4th)
};
static constexpr size_t ScaleFlavorCount = (size_t)ScaleFlavorIndex::ScaleFlavorCount;

// abbreviated.
EnumItemInfo<ScaleFlavorIndex> gScaleFlavorIndexItems[ScaleFlavorCount] = {
    {ScaleFlavorIndex::Chromatic, "Chrom"},
    {ScaleFlavorIndex::Major, "Maj"},
    {ScaleFlavorIndex::Minor, "Min"},
    {ScaleFlavorIndex::MelodicMinor, "MelMin"},
    {ScaleFlavorIndex::HarmonicMinor, "HarmMin"},
    {ScaleFlavorIndex::MajorPentatonic, "MajPent"},
    {ScaleFlavorIndex::MinorPentatonic, "MinPent"},
    {ScaleFlavorIndex::WholeTone, "Whole"},
    {ScaleFlavorIndex::HalfWholeDiminished, "HWDim"},
    {ScaleFlavorIndex::WholeHalfDiminished, "WHDim"},
    {ScaleFlavorIndex::Altered, "Alt"},
    {ScaleFlavorIndex::Blues, "Blues"},
    {ScaleFlavorIndex::Unison, "Unison"},
    {ScaleFlavorIndex::Power, "Power"},
};

EnumInfo<ScaleFlavorIndex> gScaleFlavorIndexInfo("ScaleFlavorIndex", gScaleFlavorIndexItems);

////////////////////////////////////////////////////
// using this + Scale flavor allows you to construct an absolute MIDI note value.
struct NoteInScaleFlavorContext
{
    int8_t mScaleDegree;
    // +1 = sharp, -1 = flat, and greater alterations possible.
    // So Db in C major is scale degree 1, enharmonic 1, OR degree 2, enharmonic -1.
    int8_t mEnharmonic;
};

enum class EnharmonicDirection : uint8_t
{
    Sharp,
    Flat,
    // other possibilities:
    // nearest-or-sharp
    // nearest-or-flat
};

// FLAGS
enum class ScaleFlavorOptions : uint8_t
{
    AllowInMenus = 2,
    AllowEverywhere = 3,
};

struct ScaleFlavor
{
    ScaleFlavorIndex mID;
    const char *mShortName;
    const char *mLongName;
    ScaleFlavorOptions mOptions;

    bool IsAllowedInMenus() const
    {
        return (uint8_t)mOptions & (uint8_t)ScaleFlavorOptions::AllowInMenus;
    }

    // list of intervals in the scale?
    uint8_t mSymmetry =
        0; // symmetric scales mean there are not 12 unique ones. chromatic = symmetry of 1. whole tone = symmetry 2.

    int8_t mIntervals[12]; // signed because we have -1 magic numbers.
    size_t mIntervalCount;

    int8_t mDegreeCharacteristicStrengths[12];
    size_t mDegreeCharacteristicStrengthsCount;

    uint8_t mTotalCharacteristicStrength = 0;
    NoteInScaleFlavorContext mNoteToScaleDegreeLUT_Sharps[12]; // convert chromatic note to scale degree+enharmonic.
    NoteInScaleFlavorContext mNoteToScaleDegreeLUT_Flats[12];  // convert chromatic note to scale degree+enharmonic.
    uint8_t mScaleDegreeToChromaticRelNoteLUT[12]; // convert scale degree to chromatic relative note (0=scale root,
                                                   // 1=scale root +1 semitone)

    float mChromaticFitnessFits[12]; // how well each chromatic note fits in the scale.
    float howCommonInScaleDetector;

    ScaleFlavor(ScaleFlavorIndex id,
                const char *shortName,
                const char *longName,
                ScaleFlavorOptions options,
                uint8_t symmetry,
                std::initializer_list<int8_t> intervals,
                std::initializer_list<int8_t> degreeCharacteristicStrengths,
                float howCommonInScaleDetector, // 1 = most common, 0 = NEVER.
                std::initializer_list<float> chromaticFitnessFits)
        : mID(id), mShortName(shortName), mLongName(longName), mOptions(options), mSymmetry(symmetry),
          howCommonInScaleDetector(howCommonInScaleDetector)
    {
        size_t i = 0;
        for (auto it = intervals.begin(); it != intervals.end(); ++it, ++i)
        {
            mIntervals[i] = *it;
        }
        mIntervals[intervals.size()] = -1;
        mIntervalCount = intervals.size();

        i = 0;
        for (auto it = degreeCharacteristicStrengths.begin(); it != degreeCharacteristicStrengths.end(); ++it, ++i)
        {
            mDegreeCharacteristicStrengths[i] = *it;
        }
        mDegreeCharacteristicStrengths[degreeCharacteristicStrengths.size()] = -1;
        mDegreeCharacteristicStrengthsCount = degreeCharacteristicStrengths.size();

        i = 0;
        float sumOfPositiveFitnesses = 0;
        for (auto it = chromaticFitnessFits.begin(); it != chromaticFitnessFits.end(); ++it, ++i)
        {
            float score = *it;
            mChromaticFitnessFits[i] = score;
            if (score > 0)
            {
                sumOfPositiveFitnesses += score;
            }
        }

        // we need to find a way to normalize fitness score so every scale flavor will return a similar value for
        // similar fitness.
        float factor = 1.0f / (std::max(1.0f, sumOfPositiveFitnesses));
        for (i = 0; i < 12; ++i)
        {
            // if (mChromaticFitnessFits[i] > 0)
            {
                mChromaticFitnessFits[i] *= factor;
            }
        }

        uint8_t span = 0;
        for (auto i : mIntervals)
        {
            if (i < 0)
                break;
            span += i;
        }
        if (span != 12)
        {
            Serial.println(String("no 12 in ") + longName + "; mIntervalCount=" + mIntervalCount);
        }
        CCASSERT(span == 12);

        for (auto i : mDegreeCharacteristicStrengths)
        {
            if (i < 0)
                break;
            mTotalCharacteristicStrength += i;
        }

        // fill LUTs.

        // mNoteToScaleDegreeLUT and
        // mScaleDegreeToChromaticRelNoteLUT
        uint8_t iScaleDeg_sharps = 0;
        int8_t enh_sharps = 0;
        mScaleDegreeToChromaticRelNoteLUT[0] = 0;
        for (uint8_t ich = 0; ich < 12; ++ich)
        {
            CCASSERT(iScaleDeg_sharps < 12);
            mNoteToScaleDegreeLUT_Sharps[ich].mScaleDegree = iScaleDeg_sharps;
            mNoteToScaleDegreeLUT_Sharps[ich].mEnharmonic = enh_sharps;
            enh_sharps++;
            if (enh_sharps >= mIntervals[iScaleDeg_sharps])
            {
                enh_sharps = 0;
                iScaleDeg_sharps++;
                mScaleDegreeToChromaticRelNoteLUT[iScaleDeg_sharps] = (ich + 1) % 12;
            }
        }

        // fill the flat version. could be done in the previous loop but this is clearer.
        // we want to start at chromatic interval 12 (0+octave), in order to simplify processing by starting from a
        // known fixed position.
        int8_t iScaleDeg_flats = 0;
        int8_t enh_flats = 0;
        for (uint8_t ich = 12; ich > 0; --ich)
        {
            mNoteToScaleDegreeLUT_Flats[ich % 12].mScaleDegree = iScaleDeg_flats;
            mNoteToScaleDegreeLUT_Flats[ich % 12].mEnharmonic = -enh_flats;
            enh_flats++;
            uint8_t intMinus1 = RotateIntoRange(iScaleDeg_flats - 1, mIntervalCount);
            if (enh_flats >= mIntervals[intMinus1])
            {
                enh_flats = 0;
                iScaleDeg_flats = intMinus1;
            }
        }
    }

    // takes a chromatic relative note (0=scale root, 1=scale root +1 semitone, etc) and returns scale degree +
    // enharmonic. caller must specify whether they want sharp or flat enharmonic preference.
    NoteInScaleFlavorContext RelativeChrNoteToContext(int8_t relativeNoteIndex, EnharmonicDirection ed) const
    {
        relativeNoteIndex = RotateIntoRangeByte(relativeNoteIndex, 12);
        switch (ed)
        {
        case EnharmonicDirection::Sharp:
            return mNoteToScaleDegreeLUT_Sharps[relativeNoteIndex];
        default:
        case EnharmonicDirection::Flat:
            return mNoteToScaleDegreeLUT_Flats[relativeNoteIndex];
        }
    }

    // takes any scale degree and brings it into range of 0-
    uint8_t NormalizeScaleDegree(int8_t s) const
    {
        return RotateIntoRangeByte(s, (uint8_t)mIntervalCount);
    }

    // takes any scale degree and brings it into range of 0-
    uint8_t NormalizeScaleDegree(int8_t s, int8_t &octaveTransposition) const
    {
        return RotateIntoRangeByte(s, (uint8_t)mIntervalCount, octaveTransposition);
    }

    // if ctx octave had to be normalized, offsetAdj will be adjusted to understand how to preserve octave info.
    uint8_t ContextToChrRelativeNote(const NoteInScaleFlavorContext &ctx, int8_t &octaveTransposition) const
    {
        uint8_t nsd = NormalizeScaleDegree(ctx.mScaleDegree, octaveTransposition);
        int8_t chr = (int8_t)mScaleDegreeToChromaticRelNoteLUT[nsd];
        return RotateIntoRangeByte(chr + ctx.mEnharmonic, 12);
    }
};

struct PitchFitnessClass
{
    static constexpr float NotInScale_HardBlock = -1;
    static constexpr float NotInScale_Unfit = -1;
    static constexpr float NotInScale_Agnostic = 0; // not in scale, but does not penalize. rare to use this; only place
                                                    // i could really think needs this is minor blues around the 7th.
    static constexpr float InScale = 1;
    static constexpr float Strong = 2;
};

struct HowCommonClasses
{
    static constexpr float Never = 0;
    // actually i think this is not used; basically the more rare the scale is the more penalties it will incur.
    static constexpr float Novelty = 0.7f;   // effect scales: chromatic, whole tone, diminished...
    static constexpr float Secondary = 0.8f; // secondary scales: pentatonic, blues, harm minor...
    static constexpr float Primary = 0.9f;   // primary: major, minor, ...
};

// always make sure the scale spans 1 octave exactly.
// !! NB: match indices to ScaleFlavorIndex!
const ScaleFlavor gScaleFlavors[14] = {
    {ScaleFlavorIndex::Chromatic,
     "Chrom",
     "Chromatic",
     ScaleFlavorOptions::AllowInMenus,
     1,
     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // characteristic strengths (not used anymore)
     HowCommonClasses::Novelty,
     {
         PitchFitnessClass::InScale, // C
         PitchFitnessClass::InScale, // C#
         PitchFitnessClass::InScale, // D
         PitchFitnessClass::InScale, // D#
         PitchFitnessClass::InScale, // E
         PitchFitnessClass::InScale, // F
         PitchFitnessClass::InScale, // F#
         PitchFitnessClass::InScale, // G
         PitchFitnessClass::InScale, // G#
         PitchFitnessClass::InScale, // A
         PitchFitnessClass::InScale, // A#
         PitchFitnessClass::InScale, // B
     }},

    {ScaleFlavorIndex::Major,
     "Major",
     "Major",
     ScaleFlavorOptions::AllowEverywhere,
     12,
     {2, 2, 1, 2, 2, 2, 1},
     {2, 1, 3, 1, 2, 1, 1}, // characteristic strengths (not used anymore)
     HowCommonClasses::Primary,
     {
         PitchFitnessClass::InScale,          // C
         PitchFitnessClass::NotInScale_Unfit, // C#
         PitchFitnessClass::InScale,          // D
         PitchFitnessClass::NotInScale_Unfit, // D#
         PitchFitnessClass::Strong,           // E
         PitchFitnessClass::InScale,          // F
         PitchFitnessClass::NotInScale_Unfit, // F#
         PitchFitnessClass::InScale,          // G
         PitchFitnessClass::NotInScale_Unfit, // G#
         PitchFitnessClass::InScale,          // A
         PitchFitnessClass::NotInScale_Unfit, // A#
         PitchFitnessClass::InScale,          // B
     }},

    {ScaleFlavorIndex::Minor,
     "Minor",
     "Minor",
     ScaleFlavorOptions::AllowInMenus,
     12,
     {2, 1, 2, 2, 1, 2, 2},
     {2, 1, 3, 1, 2, 1, 1}, // characteristic strengths (not used anymore)
     HowCommonClasses::Primary,
     {
         PitchFitnessClass::InScale,          // C
         PitchFitnessClass::NotInScale_Unfit, // C#
         PitchFitnessClass::InScale,          // D
         PitchFitnessClass::InScale,          // D#
         PitchFitnessClass::NotInScale_Unfit, // E
         PitchFitnessClass::InScale,          // F
         PitchFitnessClass::NotInScale_Unfit, // F#
         PitchFitnessClass::InScale,          // G
         PitchFitnessClass::InScale,          // G#
         PitchFitnessClass::NotInScale_Unfit, // A
         PitchFitnessClass::InScale,          // A#
         PitchFitnessClass::NotInScale_Unfit, // B
     }},

    {ScaleFlavorIndex::MelodicMinor,
     "MelMi",
     "Mel Min",
     ScaleFlavorOptions::AllowEverywhere,
     12,
     {2, 1, 2, 2, 2, 2, 1},
     {2, 1, 3, 1, 1, 1, 3}, // characteristic strengths (not used anymore)
     HowCommonClasses::Secondary,
     {
         PitchFitnessClass::InScale,              // C
         PitchFitnessClass::NotInScale_Unfit,     // C#
         PitchFitnessClass::InScale,              // D
         PitchFitnessClass::InScale,              // D#
         PitchFitnessClass::NotInScale_HardBlock, // E
         PitchFitnessClass::InScale,              // F
         PitchFitnessClass::NotInScale_Unfit,     // F#
         PitchFitnessClass::InScale,              // G
         PitchFitnessClass::NotInScale_HardBlock, // G#
         PitchFitnessClass::InScale,              // A
         PitchFitnessClass::NotInScale_Unfit,     // A#
         PitchFitnessClass::InScale,              // B
     }},

    {ScaleFlavorIndex::HarmonicMinor,
     "HrmMi",
     "Harm Min",
     ScaleFlavorOptions::AllowInMenus,
     12,
     {2, 1, 2, 2, 1, 3, 1},
     {2, 1, 3, 1, 1, 3, 3}, // characteristic strengths (not used anymore)
     HowCommonClasses::Secondary,
     {
         PitchFitnessClass::InScale,              // C
         PitchFitnessClass::NotInScale_Unfit,     // C#
         PitchFitnessClass::InScale,              // D
         PitchFitnessClass::InScale,              // D#
         PitchFitnessClass::NotInScale_Unfit,     // E
         PitchFitnessClass::InScale,              // F
         PitchFitnessClass::NotInScale_Unfit,     // F#
         PitchFitnessClass::InScale,              // G
         PitchFitnessClass::InScale,              // G#
         PitchFitnessClass::NotInScale_HardBlock, // A
         PitchFitnessClass::NotInScale_HardBlock, // A#
         PitchFitnessClass::InScale,              // B
     }},

    {ScaleFlavorIndex::MajorPentatonic,
     "MajPent",
     "Maj Pent",
     ScaleFlavorOptions::AllowEverywhere,
     12,
     {2, 2, 3, 2, 3},
     {2, 1, 3, 2, 1}, // characteristic strengths (not used anymore)
     HowCommonClasses::Secondary,
     {
         PitchFitnessClass::InScale,              // C
         PitchFitnessClass::NotInScale_Unfit,     // C#
         PitchFitnessClass::InScale,              // D
         PitchFitnessClass::NotInScale_HardBlock, // D#
         PitchFitnessClass::Strong,               // E
         PitchFitnessClass::NotInScale_Unfit,     // F agnostic candidate
         PitchFitnessClass::NotInScale_Unfit,     // F# agnostic candidate
         PitchFitnessClass::InScale,              // G
         PitchFitnessClass::NotInScale_Unfit,     // G#
         PitchFitnessClass::InScale,              // A
         PitchFitnessClass::NotInScale_Unfit,     // A# agnostic candidate
         PitchFitnessClass::NotInScale_Unfit,     // B agnostic candidate
     }},

    {ScaleFlavorIndex::MinorPentatonic,
     "MinPent",
     "Min Pent",
     ScaleFlavorOptions::AllowEverywhere,
     12,
     {3, 2, 2, 3, 2},
     {2, 3, 1, 1, 1}, // characteristic strengths (not used anymore)
     HowCommonClasses::Secondary,
     {
         PitchFitnessClass::InScale,          // C
         PitchFitnessClass::NotInScale_Unfit, // C#
         PitchFitnessClass::NotInScale_Unfit, // D // could be agnostic but this way it distinguishes from major/minor
                                              // scales
         PitchFitnessClass::Strong,           // D#
         PitchFitnessClass::NotInScale_HardBlock, // E
         PitchFitnessClass::InScale,              // F
         PitchFitnessClass::NotInScale_Unfit,     // F#
         PitchFitnessClass::InScale,              // G
         PitchFitnessClass::NotInScale_Unfit,     // G#
         PitchFitnessClass::NotInScale_Unfit,     // A // agnostic candidate
         PitchFitnessClass::InScale,              // A#
         PitchFitnessClass::NotInScale_Unfit,     // B
     }},

    {ScaleFlavorIndex::WholeTone,
     "WholeTone",
     "WholeTone",
     ScaleFlavorOptions::AllowInMenus,
     2,
     {2, 2, 2, 2, 2, 2},
     {1, 1, 1, 1, 1, 1}, // characteristic strengths (not used anymore)
     HowCommonClasses::Novelty,
     {
         PitchFitnessClass::InScale,              // C
         PitchFitnessClass::NotInScale_HardBlock, // C#
         PitchFitnessClass::InScale,              // D
         PitchFitnessClass::NotInScale_HardBlock, // D#
         PitchFitnessClass::InScale,              // E
         PitchFitnessClass::NotInScale_HardBlock, // F
         PitchFitnessClass::InScale,              // F#
         PitchFitnessClass::NotInScale_HardBlock, // G
         PitchFitnessClass::InScale,              // G#
         PitchFitnessClass::NotInScale_HardBlock, // A
         PitchFitnessClass::InScale,              // A#
         PitchFitnessClass::NotInScale_HardBlock, // B
     }},

    {ScaleFlavorIndex::HalfWholeDiminished,
     "HalfWhDim",
     "Half-Whole Dim",
     ScaleFlavorOptions::AllowEverywhere,
     3,
     {1, 2, 1, 2, 1, 2, 1, 2},
     {1, 1, 1, 1, 1, 1, 1, 1}, // characteristic strengths (not used anymore)
     HowCommonClasses::Novelty,
     {
         // need to distinguish this from blues scale.
         // think A C C#
         // this could definitely be both F# blues or C half-whole dim.
         // the presence of the B is the key.
         PitchFitnessClass::InScale,              // C
         PitchFitnessClass::InScale,              // C#
         PitchFitnessClass::NotInScale_HardBlock, // D
         PitchFitnessClass::InScale,              // D#
         PitchFitnessClass::InScale,              // E
         PitchFitnessClass::NotInScale_HardBlock, // F
         PitchFitnessClass::InScale,              // F#
         PitchFitnessClass::InScale,              // G
         PitchFitnessClass::NotInScale_HardBlock, // G#
         PitchFitnessClass::InScale,              // A
         PitchFitnessClass::InScale,              // A#
         PitchFitnessClass::NotInScale_HardBlock, // B
     }},

    {ScaleFlavorIndex::WholeHalfDiminished,
     "WhHalfDim",
     "Whole-Half Dim",
     ScaleFlavorOptions::AllowInMenus,
     3,
     {2, 1, 2, 1, 2, 1, 2, 1},
     {1, 1, 1, 1, 1, 1, 1, 1}, // characteristic strengths (not used anymore)
     HowCommonClasses::Novelty,
     {
         PitchFitnessClass::InScale,              // C
         PitchFitnessClass::NotInScale_HardBlock, // C#
         PitchFitnessClass::InScale,              // D
         PitchFitnessClass::InScale,              // D#
         PitchFitnessClass::NotInScale_HardBlock, // E
         PitchFitnessClass::InScale,              // F
         PitchFitnessClass::InScale,              // F#
         PitchFitnessClass::NotInScale_HardBlock, // G
         PitchFitnessClass::InScale,              // G#
         PitchFitnessClass::InScale,              // A
         PitchFitnessClass::NotInScale_HardBlock, // A#
         PitchFitnessClass::InScale,              // B
     }},

    {ScaleFlavorIndex::Altered,
     "Alt",
     "Altered",
     ScaleFlavorOptions::AllowInMenus,
     12,
     {1, 2, 1, 2, 2, 2, 2},
     {2, 2, 1, 2, 1, 1, 1}, // characteristic strengths (not used anymore)
     HowCommonClasses::Secondary,
     {
         // TODO: but because we don't include this in scale follower it's not needed now.
         PitchFitnessClass::InScale, // C
         PitchFitnessClass::InScale, // C#
         PitchFitnessClass::InScale, // D
         PitchFitnessClass::InScale, // D#
         PitchFitnessClass::InScale, // E
         PitchFitnessClass::InScale, // F
         PitchFitnessClass::InScale, // F#
         PitchFitnessClass::InScale, // G
         PitchFitnessClass::InScale, // G#
         PitchFitnessClass::InScale, // A
         PitchFitnessClass::InScale, // A#
         PitchFitnessClass::InScale, // B
     }},

    {ScaleFlavorIndex::Blues,
     "MinBlues",
     "MinBlues",
     ScaleFlavorOptions::AllowInMenus,
     12,
     {3, 2, 1, 1, 3, 2},
     {2, 3, 1, 3, 2, 1}, // characteristic strengths (not used anymore)
     HowCommonClasses::Secondary,
     {
         PitchFitnessClass::InScale,              // C
         PitchFitnessClass::NotInScale_Unfit,     // C#
         PitchFitnessClass::InScale,              // D
         PitchFitnessClass::InScale,              // D#
         PitchFitnessClass::NotInScale_HardBlock, // E
         PitchFitnessClass::InScale,              // F
         PitchFitnessClass::InScale,              // F#
         PitchFitnessClass::InScale,              // G
         PitchFitnessClass::NotInScale_HardBlock, // G#
         PitchFitnessClass::NotInScale_Agnostic,  // A
         PitchFitnessClass::InScale,              // A#
         PitchFitnessClass::NotInScale_Agnostic,  // B
     }},

    {ScaleFlavorIndex::Unison,
     "Uni",
     "Unison",
     ScaleFlavorOptions::AllowEverywhere,
     12,
     {12},
     {1}, // characteristic strengths (not used anymore)
     HowCommonClasses::Never,
     {
         // TODO: but because we don't include this in scale follower it's not needed now.
         PitchFitnessClass::InScale, // C
         PitchFitnessClass::InScale, // C#
         PitchFitnessClass::InScale, // D
         PitchFitnessClass::InScale, // D#
         PitchFitnessClass::InScale, // E
         PitchFitnessClass::InScale, // F
         PitchFitnessClass::InScale, // F#
         PitchFitnessClass::InScale, // G
         PitchFitnessClass::InScale, // G#
         PitchFitnessClass::InScale, // A
         PitchFitnessClass::InScale, // A#
         PitchFitnessClass::InScale, // B
     }},

    {ScaleFlavorIndex::Power,
     "5th",
     "Power",
     ScaleFlavorOptions::AllowEverywhere,
     12,
     {7, 5},
     {1, 1}, // characteristic strengths (not used anymore)
     HowCommonClasses::Never,
     {
         // TODO: but because we don't include this in scale follower it's not needed now.
         PitchFitnessClass::InScale, // C
         PitchFitnessClass::InScale, // C#
         PitchFitnessClass::InScale, // D
         PitchFitnessClass::InScale, // D#
         PitchFitnessClass::InScale, // E
         PitchFitnessClass::InScale, // F
         PitchFitnessClass::InScale, // F#
         PitchFitnessClass::InScale, // G
         PitchFitnessClass::InScale, // G#
         PitchFitnessClass::InScale, // A
         PitchFitnessClass::InScale, // A#
         PitchFitnessClass::InScale, // B
     }},
};

constexpr auto scaleFlavorSize = sizeof(gScaleFlavors);

////////////////////////////////////////////////////
struct Scale
{
    Note mRootNoteIndex = Note::C;                       // note index gNotes
    ScaleFlavorIndex mFlavorIndex = (ScaleFlavorIndex)0; // index into gScaleFlavors

    Scale() = default;
    Scale(Scale &&) = default;
    Scale(const Scale &) = default;
    Scale(uint8_t root, ScaleFlavorIndex flavor) : mRootNoteIndex((Note)root), mFlavorIndex(flavor)
    {
        CCASSERT(root < 12);
    }
    Scale(Note root, ScaleFlavorIndex flavor) : mRootNoteIndex(root), mFlavorIndex(flavor)
    {
    }

    Scale &operator=(const Scale &rhs)
    {
        mRootNoteIndex = rhs.mRootNoteIndex;
        mFlavorIndex = rhs.mFlavorIndex;
        return *this;
    }

    bool operator==(const Scale &rhs) const
    {
        return (mRootNoteIndex == rhs.mRootNoteIndex) && (mFlavorIndex == rhs.mFlavorIndex);
    }
    bool operator!=(const Scale &rhs) const
    {
        return !((*this) == rhs);
    }

    const ScaleFlavor &GetScaleFlavor() const
    {
        return gScaleFlavors[(size_t)mFlavorIndex];
    }

    // midiNoteOffset returns the root of the scale, in the same octave as midiNote.
    //  - that's useful for passing into GetMidiNoteFromContext to reconstruct the midi note.
    // enharmonicDirection indicates whether to prefer sharps or flats when converting to scale degree + enharmonic.
    NoteInScaleFlavorContext GetNoteInScaleContext(uint8_t midiNote,
                                                   uint8_t &midiNoteOffset,
                                                   EnharmonicDirection ed) const
    {
        CCASSERT(midiNote <= 127);
        MidiNote chromaticRelToRoot =
            (int8_t)midiNote - (uint8_t)mRootNoteIndex; // make relative to the root; retains octave, positive.

        midiNoteOffset = (chromaticRelToRoot.GetOctave() * 12) + (uint8_t)mRootNoteIndex;
        // now convert note to scale degree & enharmonic.
        return GetScaleFlavor().RelativeChrNoteToContext(chromaticRelToRoot.GetNoteIndex(), ed);
    }

    uint8_t GetMidiNoteFromContext(NoteInScaleFlavorContext ctx, int8_t midiNoteOffset)
    {
        int8_t octaveOffset;
        uint8_t note = GetScaleFlavor().ContextToChrRelativeNote(ctx, octaveOffset);
        note += midiNoteOffset + (octaveOffset * 12);
        return note;
    }

    // return 0 if result is invalid
    uint8_t AdjustNoteByInterval(uint8_t midiNote, int8_t interval, EnharmonicDirection ed)
    {
        uint8_t midiNoteOffset;
        NoteInScaleFlavorContext ctx =
            GetNoteInScaleContext(midiNote, /*out*/ midiNoteOffset, ed); // convert note to a manipulatable form
        ctx.mScaleDegree += interval;                                    // manipulate
        uint8_t ret = GetMidiNoteFromContext(ctx, midiNoteOffset);
        return ret;
    }

    // takes 0-127 style midi note
    // and returns 0 to 11, semitones relative to root
    // so if we are C and you pass C# , return = C-C# = 0 - 1 =  -1, +12 to normalize = 11
    uint8_t MidiToChromaticRelativeToRoot(uint8_t midiNote) const
    {
        MidiNote m = MidiNote(midiNote);
        int8_t ret = m.GetNoteIndex();
        CCASSERT(ret >= 0 && ret <= 11);
        ret -= (uint8_t)this->mRootNoteIndex;
        if (ret < 0)
            ret += 12;
        return ret;
    }

    bool IsNoteInScale(Note note) const
    {
        uint8_t temp = 0;
        auto ctx = GetNoteInScaleContext(MidiNote(1, note).GetMidiValue(), temp, EnharmonicDirection::Flat);
        return ctx.mEnharmonic == 0;
    }

    float GetFitnessForNote(Note note) const
    {
        // get chromatic pitch relative to root
        uint8_t chr = MidiToChromaticRelativeToRoot(MidiNote(1, note).GetMidiValue());
        return GetScaleFlavor().mChromaticFitnessFits[chr];
    }

#ifdef CLARINOID_MODULE_TEST // because this is using std::vector, and is not optimized
    std::vector<std::pair<Note, int>> GetDiatonicNotesAndCharacter() const
    {
        std::vector<std::pair<Note, int>> ret;
        for (Note n = (Note)0; (int)n < 12; n = (Note)((int)n + 1))
        {
            uint8_t temp = 0;
            auto ctx = GetNoteInScaleContext(MidiNote(1, n).GetMidiValue(), temp, EnharmonicDirection::Flat);
            if (ctx.mEnharmonic == 0)
            {
                CCASSERT(ctx.mScaleDegree >= 0 &&
                         ctx.mScaleDegree < (int8_t)this->GetScaleFlavor().mDegreeCharacteristicStrengthsCount);
                ret.push_back(
                    std::make_pair(n, this->GetScaleFlavor().mDegreeCharacteristicStrengths[ctx.mScaleDegree]));
            }
        }
        return ret;
    }
#endif

    String ToString() const
    {
        return String(gNotes[(uint8_t)mRootNoteIndex].mName) + " " + GetScaleFlavor().mLongName;
    }
    String ToShortString() const
    {
        return String(gNotes[(uint8_t)mRootNoteIndex].mName) + " " + GetScaleFlavor().mShortName;
    }
};

constexpr size_t scalesize = sizeof(Scale);

} // namespace clarinoid
