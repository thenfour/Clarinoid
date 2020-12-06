

#pragma once

#include <clarinoid/scale_follower/ScaleFollower.hpp>
#include "Test.hpp"

using MusicalVoice = clarinoid::MusicalVoice;
using Note = clarinoid::Note;
using MidiNote = clarinoid::MidiNote;
using Scale = clarinoid::Scale;
using ScaleFlavor = clarinoid::ScaleFlavor;
using ScaleFlavorIndex = clarinoid::ScaleFlavorIndex;

MusicalVoice NoteOn(Note note, uint8_t octave, uint16_t voiceID)
{
  clarinoid::log("NOTE ON v:%d [ %s ]", (int)voiceID, MidiNote(octave, note).ToString());
  MusicalVoice ret;
  ret.mVelocity = 30;
  ret.mVoiceId = voiceID;
  ret.mMidiNote = MidiNote(octave, note).GetMidiValue();
  return ret;
}

void NoteOff(MusicalVoice& mv)
{
  clarinoid::log("NOTE OFF v:%d [ %s ]", (int)mv.mVoiceId, MidiNote(mv.mMidiNote).ToString());
  mv.mVelocity = 0;
}

bool ltint(const int& lhs, const int& rhs)
{
  return lhs < rhs;
}

void TestSortedArray()
{
  clarinoid::SortedArray<int, 4, decltype(&ltint)> a(&ltint);
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
  clarinoid::ScaleFollower sf;
  MusicalVoice lv[8];

  // test transient note gets forgotten.
  clarinoid::log("\r\n");
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
  clarinoid::log("\r\n");
  SetTestClockMillis(1000);
  sf = {};
  SetTestClockMillis(1000); // play Eb+Bb poly
  lv[0] = NoteOn(Note::Eb, 3, 1);
  lv[1] = NoteOn(Note::Bb, 3, 2);
  s = sf.Update(lv, 2);
  Test(s == Scale(Note::Bb, ScaleFlavorIndex::Major));

  SetTestClockMillis(1500); // the Eb + Bb are not transient notes.
  lv[0] = NoteOn(Note::Db, 3, 1); // move Eb to Db
  s = sf.Update(lv, 1); // this should mark Eb and Bb as non-transient, no longer playing, and Db now transient & playing.
  Test(s == Scale(Note::C, ScaleFlavorIndex::HalfWholeDiminished));

  SetTestClockMillis(2200); // kinda erase the Db
  lv[0] = NoteOn(Note::D, 3, 1);
  s = sf.Update(lv, 1);
  // it's unclear what the LUT should produce here. But it's especially ambiguous because
  // we don't care about importance.
  // better would be to ignore the non-playing notes.
  // a mitigating factor is that the non-important notes will definitely get pushed out soon, 
  // but better would be to have 2 tiers of importance.
  // at first i was concerned about LUT size, but i need to just restructure it because it's 
  // very inefficient. it accounst for all combinations of 12 notes playing together.
  // but considering we never analyze more than 4 notes at a time, it eliminates a LOT of permutations.


  // more normalish scenarios. Let's transition to F minor, then Ab minor
  clarinoid::log("\r\n");
  SetTestClockMillis(1000);
  sf = {};
  SetTestClockMillis(1000); // play Eb+Bb poly
  lv[0] = NoteOn(Note::Eb, 3, 1);
  lv[1] = NoteOn(Note::Bb, 3, 2);
  s = sf.Update(lv, 2);
  Test(s == Scale(Note::Bb, ScaleFlavorIndex::Major));

  SetTestClockMillis(1500); // the Eb + Bb are not transient notes.
  lv[0] = NoteOn(Note::Ab, 3, 1); // move Eb to Ab
  s = sf.Update(lv, 1); // this should mark Eb and Bb as non-transient, no longer playing, and Ab now transient & playing.
  Test(s == Scale(Note::Eb, ScaleFlavorIndex::Major));

  delay(300);
  lv[0] = NoteOn(Note::G, 3, 1); // move Ab to G
  s = sf.Update(lv, 1);
  Test(s == Scale(Note::Eb, ScaleFlavorIndex::Major));

  delay(300);
  lv[0] = NoteOn(Note::Bb, 3, 1);
  s = sf.Update(lv, 1);
  Test(s == Scale(Note::Eb, ScaleFlavorIndex::Major));

  delay(300);
  lv[0] = NoteOn(Note::B, 3, 1);
  s = sf.Update(lv, 1);
  Test(s == Scale(Note::Ab, ScaleFlavorIndex::MelodicMinor));

  // test note duration increase importance
  // test important notes, test transientness
  // test LUT

}


