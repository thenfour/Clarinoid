
#pragma once

#include <Shared_CCUtil.h>

struct NoteDesc
{
  uint8_t mIndex;
  const char *mName;
};

NoteDesc gNotes[12] = {
  {0, "C"},
  {1, "C#"},
  {2, "D"},
  {3, "D#"},
  {4, "E"},
  {5, "F"},
  {6, "F#"},
  {7, "G"},
  {8, "G#"},
  {9, "A"},
  {10, "A#"},
  {11, "B"},
};

enum class ScaleFlavorIndex : uint8_t // match index to gScaleFlavors
{
    Major = 0,
    WholeTone = 1,
    Chromatic = 2,
    HalfWholeDiminished = 3,
    Altered = 4,
};

struct ScaleFlavor
{
  ScaleFlavor(ScaleFlavorIndex id, const char *name, int8_t i1, int8_t i2 = -1, int8_t i3 = -1, int8_t i4 = -1, int8_t i5 = -1, int8_t i6 = -1, int8_t i7 = -1) :
    mID(id),
    mName(name)
  {
    mIntervals[0] = i1;
    mIntervals[1] = i2;
    mIntervals[2] = i3;
    mIntervals[3] = i4;
    mIntervals[4] = i5;
    mIntervals[5] = i6;
    mIntervals[6] = i7;
    for (size_t i = 0; i < SizeofStaticArray(mIntervals); ++ i) {
      if (mIntervals[i] == -1) {
        mIntervalCount = i;
        break;
      }
    }
  }
  ScaleFlavorIndex mID;
  const char *mName;
  // list of intervals in the scale?
  uint8_t mIntervalCount;
  uint8_t mIntervals[7];
};

ScaleFlavor gScaleFlavors[5] = {
  {ScaleFlavorIndex::Major, "Maj", 2,2,1,2,2,2,1},
  {ScaleFlavorIndex::WholeTone, "Who", 2},
  {ScaleFlavorIndex::Chromatic, "Chr", 1},
  {ScaleFlavorIndex::HalfWholeDiminished, "hwd", 1,2},
  {ScaleFlavorIndex::Altered, "Alt", 1,2,1,2,2,2,2},
};

struct Scale
{
  ScaleFlavorIndex mFlavorIndex = (ScaleFlavorIndex)0; // index into gScaleFlavors
  uint8_t mRootNoteIndex = 0; // note index gNotes
};

// don't use a LUT because we want to support pitch bend and glides and stuff. using a LUT + interpolation would be asinine.
inline float MIDINoteToFreq(float x) {
    float a = 440;
    return (a / 32.0f) * powf(2.0f, (((float)x - 9.0f) / 12.0f));
}
