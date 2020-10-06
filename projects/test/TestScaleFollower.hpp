

#pragma once

#include <clarinoid/scale_follower/ScaleFollower.hpp>
#include "Test.hpp"

MusicalVoice NoteOn(Note note, uint8_t octave, uint16_t voiceID)
{
  cc::log("NOTE ON v:%d [ %s ]", (int)voiceID, MidiNote(octave, note).ToString());
  MusicalVoice ret;
  ret.mVelocity = 30;
  ret.mVoiceId = voiceID;
  ret.mMidiNote = MidiNote(octave, note).GetMidiValue();
  return ret;
}

void NoteOff(MusicalVoice& mv)
{
  cc::log("NOTE OFF v:%d [ %s ]", (int)mv.mVoiceId, MidiNote(mv.mMidiNote).ToString());
  mv.mVelocity = 0;
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

  // test transient note gets forgotten.
  cc::log("\r\n");
  SetTestClockMillis(1000);
  lv[0] = NoteOn(Note::C, 3, 1);
  s = sf.Update(lv, 1);
  Test(s == Scale(Note::C, ScaleFlavorIndex::Major));

  SetTestClockMillis(1050);
  lv[0] = NoteOn(Note::Eb, 3, 1);
  s = sf.Update(lv, 1);
  Test(s == Scale(Note::C, ScaleFlavorIndex::MelodicMinor));

  SetTestClockMillis(1010); // 10 milliseconds later you release the Eb. we should act like it never happened.
  NoteOff(lv[0]);
  s = sf.Update(lv, 1);
  Test(s == Scale(Note::C, ScaleFlavorIndex::Major));

  // test progressive following.
  cc::log("\r\n");
  SetTestClockMillis(1000);
  sf = {};
  SetTestClockMillis(1000); // play Eb+Bb poly
  lv[0] = NoteOn(Note::Eb, 3, 1);
  lv[1] = NoteOn(Note::Bb, 3, 2);
  s = sf.Update(lv, 2);
  Test(s == Scale(Note::Bb, ScaleFlavorIndex::Major));

  SetTestClockMillis(1500); // the Eb + Bb are not transient notes.
  // move Eb to Db
  lv[0] = NoteOn(Note::Db, 3, 1);
  s = sf.Update(lv, 1);
  Test(s == Scale(Note::Bb, ScaleFlavorIndex::Major));


  // test LUT

  // test scale following

}


