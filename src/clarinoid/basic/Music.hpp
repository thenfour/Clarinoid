
#pragma once

#include "Enum.hpp"



// don't use a LUT because we want to support pitch bend and glides and stuff. using a LUT + interpolation would be asinine.
inline float MIDINoteToFreq(float x) {
  float a = 440;
  return (a / 32.0f) * powf(2.0f, (((float)x - 9.0f) / 12.0f));
}



////////////////////////////////////////////////////
struct NoteDesc
{
  uint8_t mIndex;
  const char *mName;
};

NoteDesc const gNotes[12] = {
  {0, "C "},
  {1, "C#"},
  {2, "D "},
  {3, "D#"},
  {4, "E "},
  {5, "F "},
  {6, "F#"},
  {7, "G "},
  {8, "G#"},
  {9, "A "},
  {10, "A#"},
  {11, "B "},
};

enum class Note : uint8_t
{
  C,
  Db,
  D,
  Eb,
  E,
  F,
  Gb,
  G,
  Ab,
  A,
  Bb,
  B
};

////////////////////////////////////////////////////
class MidiNote
{
  uint8_t mValue = 0; // 0-127 midi note value
  uint8_t mNoteIndex = 0;// 0-11 gNote index
  uint8_t mOctave = 0;

public:
  MidiNote() = default;
  MidiNote(uint8_t val) :
    mValue(val)
  {
    // for reference, 36 = C2
    // but MIDI "octaves" are like, not very handy because A0 is val 21.
    // which makes C1 = 24
    // which makens C0 = 12
    // so... what's below that???
    // for mathematical simplicity
    DivRem<uint8_t, 12>(val, mOctave, mNoteIndex);
  }

  MidiNote(uint8_t oct, uint8_t note) :
    mValue(oct * 12 + note),
    mNoteIndex(note),
    mOctave(oct)
  {
  }

  MidiNote(uint8_t oct, Note note) :
    mValue(oct * 12 + (uint8_t)note),
    mNoteIndex((uint8_t)note),
    mOctave(oct)
  {
  }

  uint8_t GetMidiValue() const { return mValue; }
  uint8_t GetNoteIndex() const { return mNoteIndex; }
  const NoteDesc& GetNoteDesc() const { return gNotes[mNoteIndex]; }
  uint8_t GetOctave() const { return mOctave; }
};


////////////////////////////////////////////////////
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
};

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
  const char *mName;
  ScaleFlavorOptions mOptions;

  // list of intervals in the scale?
  uint8_t mIntervalCount;
  int8_t mIntervals[13]; // signed because we have -1 magic numbers.
  NoteInScaleFlavorContext mNoteToScaleDegreeLUT_Sharps[12]; // convert chromatic note to scale degree+enharmonic.
  NoteInScaleFlavorContext mNoteToScaleDegreeLUT_Flats[12]; // convert chromatic note to scale degree+enharmonic.
  uint8_t mScaleDegreeToChromaticRelNoteLUT[12]; // convert scale degree to chromatic relative note (0=scale root, 1=scale root +1 semitone)

  ScaleFlavor(ScaleFlavorIndex id, const char *name, ScaleFlavorOptions options,
    int8_t i1, int8_t i2 = -1, int8_t i3 = -1, int8_t i4 = -1,
    int8_t i5 = -1, int8_t i6 = -1, int8_t i7 = -1, int8_t i8 = -1,
    int8_t i9 = -1, int8_t i10 = -1, int8_t i11 = -1, int8_t i12 = -1,
    int8_t i13 = -1) :
    mID(id),
    mName(name),
    mOptions(options)
  {
    mIntervals[0] = i1;
    mIntervals[1] = i2;
    mIntervals[2] = i3;
    mIntervals[3] = i4;
    mIntervals[4] = i5;
    mIntervals[5] = i6;
    mIntervals[6] = i7;
    mIntervals[7] = i8;
    mIntervals[8] = i9;
    mIntervals[9] = i10;
    mIntervals[10] = i11;
    mIntervals[11] = i12;
    mIntervals[12] = i13;
    uint8_t span = 0;
    for (int8_t i = 0; i < (int8_t)SizeofStaticArray(mIntervals); ++ i) {
      if (mIntervals[i] == -1) {
        mIntervalCount = i;
        break;
      }
      span += mIntervals[i];
    }
    CCASSERT(span == 12);

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
      if (enh_sharps >= mIntervals[iScaleDeg_sharps]) {
        enh_sharps = 0;
        iScaleDeg_sharps++;
        mScaleDegreeToChromaticRelNoteLUT[iScaleDeg_sharps] = (ich + 1) % 12;
      }
    }

    // fill the flat version. could be done in the previous loop but this is clearer.
    // we want to start at chromatic interval 12 (0+octave), in order to simplify processing by starting from a known fixed position.
    int8_t iScaleDeg_flats = 0;
    int8_t enh_flats = 0;
    for (uint8_t ich = 12; ich > 0; --ich)
    {
      mNoteToScaleDegreeLUT_Flats[ich % 12].mScaleDegree = iScaleDeg_flats;
      mNoteToScaleDegreeLUT_Flats[ich % 12].mEnharmonic = -enh_flats;
      enh_flats++;
      uint8_t intMinus1 = RotateIntoRange(iScaleDeg_flats - 1, mIntervalCount);
      if (enh_flats >= mIntervals[intMinus1]) {
        enh_flats = 0;
        iScaleDeg_flats = intMinus1;
      }
    }
  }

  NoteInScaleFlavorContext RelativeChrNoteToContext(int8_t relativeNoteIndex, EnharmonicDirection ed) const
  {
    relativeNoteIndex = RotateIntoRangeByte(relativeNoteIndex, 12);
    switch (ed) {
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
    return RotateIntoRangeByte(s, mIntervalCount);
  }

  // takes any scale degree and brings it into range of 0-
  uint8_t NormalizeScaleDegree(int8_t s, int8_t& octaveTransposition) const
  {
    return RotateIntoRangeByte(s, mIntervalCount, octaveTransposition);
  }

  // if ctx octave had to be normalized, offsetAdj will be adjusted to understand how to preserve octave info.
  uint8_t ContextToChrRelativeNote(const NoteInScaleFlavorContext& ctx, int8_t& octaveTransposition) const
  {
    uint8_t nsd = NormalizeScaleDegree(ctx.mScaleDegree, octaveTransposition);
    int8_t chr = (int8_t)mScaleDegreeToChromaticRelNoteLUT[nsd];
    return RotateIntoRangeByte(chr + ctx.mEnharmonic, 12);
  }
};

// always make sure the scale spans 1 octave exactly.
// !! NB: match indices to ScaleFlavorIndex!
const ScaleFlavor gScaleFlavors[14] = { 
  {ScaleFlavorIndex::Chromatic, "Chr", ScaleFlavorOptions::AllowEverywhere, 1,1,1,1, 1,1,1,1, 1,1,1,1},
  {ScaleFlavorIndex::Major, "Maj", ScaleFlavorOptions::AllowEverywhere, 2,2,1,2,2,2,1},
  {ScaleFlavorIndex::Minor, "Min", ScaleFlavorOptions::AllowEverywhere, 2,1,2,2,1,2,2},// not needed
  {ScaleFlavorIndex::MelodicMinor, "Mmi", ScaleFlavorOptions::AllowEverywhere, 2,1,2,2,2,2,1},
  {ScaleFlavorIndex::HarmonicMinor, "Hm", ScaleFlavorOptions::AllowEverywhere, 2,1,2,2,1,3,1},
  {ScaleFlavorIndex::MajorPentatonic, "MaP", ScaleFlavorOptions::AllowEverywhere, 2,2,3,2,3 },
  {ScaleFlavorIndex::MinorPentatonic, "miP", ScaleFlavorOptions::AllowEverywhere, 3,2,2,3,2},
  {ScaleFlavorIndex::WholeTone, "Who", ScaleFlavorOptions::AllowEverywhere, 2,2,2,2,2,2},
  {ScaleFlavorIndex::HalfWholeDiminished, "hwd", ScaleFlavorOptions::AllowEverywhere, 1,2,1,2,1,2,1,2},
  {ScaleFlavorIndex::WholeHalfDiminished, "whd", ScaleFlavorOptions::AllowEverywhere, 2,1,2,1,2,1,2,1},
  {ScaleFlavorIndex::Altered, "Alt", ScaleFlavorOptions::AllowEverywhere, 1,2,1,2,2,2,2},
  {ScaleFlavorIndex::Blues, "Blu", ScaleFlavorOptions::AllowEverywhere, 3,2,1,1,3,2},
  {ScaleFlavorIndex::Unison, "Uni", ScaleFlavorOptions::AllowEverywhere, 12},
  {ScaleFlavorIndex::Power, "5th", ScaleFlavorOptions::AllowEverywhere, 7,5},
};

constexpr auto scaleFlavorSize = sizeof(gScaleFlavors);

////////////////////////////////////////////////////
struct Scale
{
  uint8_t mRootNoteIndex = 0; // note index gNotes
  ScaleFlavorIndex mFlavorIndex = (ScaleFlavorIndex)0; // index into gScaleFlavors

  Scale() = default;
  Scale(Scale&&) = default;
  Scale(const Scale&) = default;
  Scale(uint8_t root, ScaleFlavorIndex flavor) :
    mRootNoteIndex(root),
    mFlavorIndex(flavor)
  {}
  Scale(Note root, ScaleFlavorIndex flavor) :
    mRootNoteIndex((uint8_t)root),
    mFlavorIndex(flavor)
  {}

  Scale& operator =(const Scale& rhs)
  {
    mRootNoteIndex = rhs.mRootNoteIndex;
    mFlavorIndex = rhs.mFlavorIndex;
    return *this;
  }

  const ScaleFlavor& GetScaleFlavor() const {
    return gScaleFlavors[(size_t)mFlavorIndex];
  }

  NoteInScaleFlavorContext GetNoteInScaleContext(uint8_t midiNote, uint8_t& midiNoteOffset, EnharmonicDirection ed)
  {
    CCASSERT(midiNote <= 127);
    MidiNote chromaticRelToRoot = (int8_t)midiNote - mRootNoteIndex; // make relative to the root.
    
    midiNoteOffset = (chromaticRelToRoot.GetOctave() * 12) + mRootNoteIndex;
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
    NoteInScaleFlavorContext ctx = GetNoteInScaleContext(midiNote, /*out*/ midiNoteOffset, ed); // convert note to a manipulatable form
    ctx.mScaleDegree += interval; // manipulate
    uint8_t ret = GetMidiNoteFromContext(ctx, midiNoteOffset);
    return ret;
  }
};

