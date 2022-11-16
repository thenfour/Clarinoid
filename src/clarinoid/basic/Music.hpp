
#pragma once

#include <initializer_list>
#include "Enum.hpp"

namespace clarinoid
{

// don't use a LUT because we want to support pitch bend and glides and stuff. using a LUT + interpolation would be
// asinine.
inline float MIDINoteToFreq(float x)
{
    // float a = 440;
    // return (a / 32.0f) * fast::pow(2.0f, (((float)x - 9.0f) / 12.0f));
    return 440 * fast::pow(2.0f, ((x - 69) / 12));
}

// let MidiNoteToFrequency = function(midiNote) {
//   return 440 * Math.pow(2, (midiNote - 69) / 12);
// };
// let FrequencyToMidiNote = (hz) => {
//   return 12.0 * Math.log2(Math.max(8, hz) / 440) + 69;
// };

inline float FrequencyToMIDINote(float hz)
{
    float ret = 12.0f * fast::log2(std::max(8.0f, hz) / 440) + 69;
    return ret;
}

// https://www.paulcecchettimusic.com/full-list-of-midi-cc-numbers/
enum class MidiCCValue
{
    BankSelect = 0,
    ModWheel = 1,
    Breath = 2,
    Expression = 11,
    DamperPedal = 64,
    AllNotesOff = 123,
};

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
    {Note::Db, "C#"},
    {Note::D, "D"},
    {Note::Eb, "D#"},
    {Note::E, "E"},
    {Note::F_, "F"},
    {Note::Gb, "F#"},
    {Note::G, "G"},
    {Note::Ab, "G#"},
    {Note::A, "A"},
    {Note::Bb, "A#"},
    {Note::B, "B"},
};

EnumInfo<Note> gNoteInfo("Note", gNoteItems);

////////////////////////////////////////////////////
class MidiNote
{
    uint8_t mValue = 0;     // 0-127 midi note value
    // uint8_t mNoteIndex = 0; // 0-11 gNote index
    // uint8_t mOctave = 0;

  public:
    MidiNote() = default;
    explicit MidiNote(int midiValue) : mValue((uint8_t)midiValue)
    {
        // for reference, 36 = C2
        // but MIDI "octaves" are like, not very handy because A0 is val 21.
        // which makes C1 = 24
        // which makens C0 = 12
        // so... what's below that???
        // for mathematical simplicity
    }

    uint8_t GetNoteIndex() const {
        return mValue % 12;
    // remainder = val % divisor;

    //     uint8_t noteIndex, octave;
    //     DivRem<uint8_t, 12>(mValue, octave, noteIndex);
    //     return noteIndex;
    }

    uint8_t GetOctave() const {
        return mValue / 12;
        // uint8_t noteIndex, octave;
        // DivRem<uint8_t, 12>(mValue, octave, noteIndex);
        // return octave;
    }

    static MidiNote MinimumValue()
    {
        return MidiNote{0};
    }
    static MidiNote MaximumValue()
    {
        return MidiNote{127};
    }

    // avoid ambiguity
    // MidiNote(uint8_t oct, uint8_t note) : mValue(oct * 12 + note), mNoteIndex(note), mOctave(oct)
    // {
    // }

    MidiNote(int oct, Note note) : mValue(oct * 12 + (uint8_t)note)//, mNoteIndex((uint8_t)note), mOctave(oct)
    {
    }

    bool operator==(const MidiNote &rhs) const
    {
        return this->mValue == rhs.mValue;
    }
    bool operator!=(const MidiNote &rhs) const
    {
        return !(*this == rhs);
    }
    bool operator<(const MidiNote &rhs) const
    {
        return this->mValue < rhs.mValue;
    }
    bool operator<=(const MidiNote &rhs) const
    {
        return this->mValue <= rhs.mValue;
    }
    bool operator>(const MidiNote &rhs) const
    {
        return this->mValue > rhs.mValue;
    }
    bool operator>=(const MidiNote &rhs) const
    {
        return this->mValue >= rhs.mValue;
    }

    MidiNote &operator+=(int8_t t)
    {
        mValue += t;
        //DivRem<uint8_t, 12>(mValue, mOctave, mNoteIndex);
        return *this;
    }

    MidiNote &operator-=(int8_t t)
    {
        return (*this) += (-t);
    }

    // in a serialize/deserializeable form
    String ToString() const
    {
        // C#8
        // force last char to be a single digit octave #.
        auto noteIndex = GetNoteIndex();
        auto octave = GetOctave();
        //return String(gNotes[noteIndex].mName) + std::min((uint8_t)9, octave);
        return String(gNoteInfo.GetValueShortName(gNoteInfo.ToValue(noteIndex))) + std::min((uint8_t)9, octave);
    }

    static Result FromString(const String &s, MidiNote &out)
    {
        if (s.length() < 2)
            return Result::Failure("too short");
        char oct = s[s.length() - 1];
        if (oct < '0' || oct > '9')
            return Result::Failure("bad octave");
        uint8_t nOct = oct - '0';
        Note note = Note::C;
        Result ret = gNoteInfo.ValueForShortName(s.substring(0, (unsigned int)(s.length() - 1)), note);
        if (ret.IsFailure())
            return ret;
        out = MidiNote{nOct, note};
        return ret;
    }

    String ToStringWithCustomGlyphs() const
    {
        auto noteIndex = GetNoteIndex();
        auto octave = GetOctave();
        return String(gNotes[noteIndex].mNameWithCustomGlyphs) + octave;
    }

    uint8_t GetMidiValue() const
    {
        return mValue;
    }
    // uint8_t GetNoteIndex() const
    // {
    //     return mNoteIndex;
    // }
    const NoteDesc &GetNoteDesc() const
    {
        return gNotes[GetNoteIndex()];
    }
    // uint8_t GetOctave() const
    // {
    //     return mOctave;
    // }
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
    AllowInScaleFollower = 1,
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
    bool IsAllowedInScaleFollower() const
    {
        return (uint8_t)mOptions & (uint8_t)ScaleFlavorOptions::AllowInScaleFollower;
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

    ScaleFlavor(ScaleFlavorIndex id,
                const char *shortName,
                const char *longName,
                ScaleFlavorOptions options,
                uint8_t symmetry,
                std::initializer_list<int8_t> intervals,
                std::initializer_list<int8_t> degreeCharacteristicStrengths)
        : mID(id), mShortName(shortName), mLongName(longName), mOptions(options), mSymmetry(symmetry)
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
            uint8_t intMinus1 = RotateIntoRange((int)(iScaleDeg_flats - 1), (int)mIntervalCount);
            if (enh_flats >= mIntervals[intMinus1])
            {
                enh_flats = 0;
                iScaleDeg_flats = intMinus1;
            }
        }
    }

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

// always make sure the scale spans 1 octave exactly.
// !! NB: match indices to ScaleFlavorIndex!
const ScaleFlavor gScaleFlavors[14] = {
    {ScaleFlavorIndex::Chromatic,
     "Chrom",
     "Chromatic",
     ScaleFlavorOptions::AllowInMenus,
     1,
     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},
    {ScaleFlavorIndex::Major,
     "Major",
     "Major",
     ScaleFlavorOptions::AllowEverywhere,
     12,
     {2, 2, 1, 2, 2, 2, 1},
     {2, 1, 3, 1, 2, 1, 1}},
    {ScaleFlavorIndex::Minor,
     "Minor",
     "Minor",
     ScaleFlavorOptions::AllowInMenus,
     12,
     {2, 1, 2, 2, 1, 2, 2},
     {2, 1, 3, 1, 2, 1, 1}},
    {ScaleFlavorIndex::MelodicMinor,
     "MelMi",
     "Mel Min",
     ScaleFlavorOptions::AllowEverywhere,
     12,
     {2, 1, 2, 2, 2, 2, 1},
     {2, 1, 3, 1, 1, 1, 3}},
    {ScaleFlavorIndex::HarmonicMinor,
     "HrmMi",
     "Harm Min",
     ScaleFlavorOptions::AllowInMenus,
     12,
     {2, 1, 2, 2, 1, 3, 1},
     {2, 1, 3, 1, 1, 3, 3}},
    {ScaleFlavorIndex::MajorPentatonic,
     "MajPent",
     "Maj Pent",
     ScaleFlavorOptions::AllowEverywhere,
     12,
     {2, 2, 3, 2, 3},
     {2, 1, 3, 2, 1}},
    {ScaleFlavorIndex::MinorPentatonic,
     "MinPent",
     "Min Pent",
     ScaleFlavorOptions::AllowEverywhere,
     12,
     {3, 2, 2, 3, 2},
     {2, 3, 1, 1, 1}},
    {ScaleFlavorIndex::WholeTone,
     "WholeTone",
     "WholeTone",
     ScaleFlavorOptions::AllowInMenus,
     2,
     {2, 2, 2, 2, 2, 2},
     {1, 1, 1, 1, 1, 1}},
    {ScaleFlavorIndex::HalfWholeDiminished,
     "HalfWhDim",
     "Half-Whole Dim",
     ScaleFlavorOptions::AllowEverywhere,
     3,
     {1, 2, 1, 2, 1, 2, 1, 2},
     {1, 1, 1, 1, 1, 1, 1, 1}},
    {ScaleFlavorIndex::WholeHalfDiminished,
     "WhHalfDim",
     "Whole-Half Dim",
     ScaleFlavorOptions::AllowInMenus,
     3,
     {2, 1, 2, 1, 2, 1, 2, 1},
     {1, 1, 1, 1, 1, 1, 1, 1}},
    {ScaleFlavorIndex::Altered,
     "Alt",
     "Altered",
     ScaleFlavorOptions::AllowInMenus,
     12,
     {1, 2, 1, 2, 2, 2, 2},
     {2, 2, 1, 2, 1, 1, 1}},
    {ScaleFlavorIndex::Blues,
     "Blues",
     "Blues",
     ScaleFlavorOptions::AllowInMenus,
     12,
     {3, 2, 1, 1, 3, 2},
     {2, 3, 1, 3, 2, 1}},
    {ScaleFlavorIndex::Unison, "Uni", "Unison", ScaleFlavorOptions::AllowEverywhere, 12, {12}, {1}},
    {ScaleFlavorIndex::Power, "5th", "Power", ScaleFlavorOptions::AllowEverywhere, 12, {7, 5}, {1, 1}},
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

    String ToSerializableString() const
    {
        return String(gNoteInfo.GetValueShortName(mRootNoteIndex)) + " " +
               gScaleFlavorIndexInfo.GetValueShortName(mFlavorIndex);
    }

    Result DeserializeFromString(const String &s)
    {
        String copy = s;
        copy.trim();
        int sep = copy.indexOf(' ');
        if (sep == -1)
            return Result::Failure("Separator not found");
        Result ret = gNoteInfo.ValueForShortName(copy.substring(0, sep), mRootNoteIndex);
        ret.AndRequires(gScaleFlavorIndexInfo.ValueForShortName(copy.substring(sep + 1), mFlavorIndex), "flav");
        return ret;
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

    NoteInScaleFlavorContext GetNoteInScaleContext(uint8_t midiNote,
                                                   uint8_t &midiNoteOffset,
                                                   EnharmonicDirection ed) const
    {
        CCASSERT(midiNote <= 127);
        MidiNote chromaticRelToRoot =
            MidiNote{(int)midiNote - (int)mRootNoteIndex}; // make relative to the root.

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

    // bool IsNoteDiatonic(Note note) const {
    //  uint8_t temp = 0;
    //  auto ctx = GetNoteInScaleContext(MidiNote(1, note).GetMidiValue(), temp, EnharmonicDirection::Flat);
    //  return ctx.mEnharmonic == 0;
    //}

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
};

constexpr size_t scalesize = sizeof(Scale);

} // namespace clarinoid
