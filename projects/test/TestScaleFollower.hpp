

#pragma once

#include <clarinoid/scale_follower/ScaleFollower.hpp>
#include "Test.hpp"

MusicalVoice NoteOn(Note note, uint8_t octave)
{
  MusicalVoice ret;
  ret.mVelocity = 30;
  ret.mMidiNote = MidiNote(octave, note).GetMidiValue();
  return ret;
}

bool ltint(const int& lhs, const int& rhs)
{
  return lhs < rhs;
}

void TestSortedArray()
{
  SortedArray<int, 4, decltype(&ltint)> a(&ltint);
  Test(a.mSize == 0);
  a.Insert(1); // 1 .
  Test(a.mSize == 1);
  Test(a.mArray[0] == 1);
  a.Insert(2); // 2 1 .
  a.Insert(3); // 3 2 1 .
  a.Insert(4); // 4 3 2 1 .
  Test(a.mSize == 4);
  Test(a.mArray[0] == 4);

  a.Clear();
  a.mArray[0] = 0;
  a.mArray[1] = 0;
  a.mArray[2] = 0;
  a.mArray[3] = 0;
  Test(a.mSize == 0);
  a.Insert(4); // 4 .
  a.Insert(3); // 4 3 .
  a.Insert(2); // 4 3 2 .
  a.Insert(1); // 4 3 2 1 .
  Test(a.mSize == 4);
  Test(a.mArray[0] == 4);
  Test(a.mArray[1] == 3);
  Test(a.mArray[2] == 2);
  Test(a.mArray[3] == 1);
  a.Insert(1); // 4 3 2 1 .
  Test(a.mSize == 4);
  Test(a.mArray[3] == 1);
  a.Insert(5); // 4 3 2 1 .
  Test(a.mSize == 4);
  Test(a.mArray[0] == 5);
  Test(a.mArray[1] == 4);
  Test(a.mArray[2] == 3);
  Test(a.mArray[3] == 2);

  a.Clear();
  a.mArray[0] = 0;
  a.mArray[1] = 0;
  a.mArray[2] = 0;
  a.mArray[3] = 0;

  a.Insert(6); // 4 .
  a.Insert(1); // 4 3 .
  a.Insert(5); // 4 3 2 .
  a.Insert(2); // 4 3 2 1 .
  a.Insert(3); // 4 3 2 1 .
  a.Insert(4); // 4 3 2 1 .
  Test(a.mSize == 4);
  Test(a.mArray[0] == 6);
  Test(a.mArray[1] == 5);
  Test(a.mArray[2] == 4);
  Test(a.mArray[3] == 3);


}

void TestScaleFollower()
{
  TestSortedArray();

  Scale s = Scale(Note::C, ScaleFlavorIndex::Major);
  ScaleFollower sf;
  MusicalVoice lv[8];

  SetTestClockMillis(1000);
  lv[0] = NoteOn(Note::C, 3);
  s = sf.Update(lv, 1);
  SetTestClockMillis(1050);
  lv[0] = NoteOn(Note::Eb, 3);
  s = sf.Update(lv, 1);

  SetTestClockMillis(1010); // 10 milliseconds later you release the Eb. we should act like it never happened.
  lv[0] = {};
  s = sf.Update(lv, 1);

}


