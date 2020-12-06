

#pragma once


#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/basic/Music.hpp>

using clarinoid::RotateIntoRangeByte;
using clarinoid::EnharmonicDirection;

void TestScaleBasics()
{
  Test(RotateIntoRangeByte(-3, 3) == 0);
  Test(RotateIntoRangeByte(-2, 3) == 1);
  Test(RotateIntoRangeByte(-1, 3) == 2);
  Test(RotateIntoRangeByte(0, 3) == 0);
  Test(RotateIntoRangeByte(1, 3) == 1);
  Test(RotateIntoRangeByte(2, 3) == 2);
  Test(RotateIntoRangeByte(3, 3) == 0);
  Test(RotateIntoRangeByte(4, 3) == 1);
  Test(RotateIntoRangeByte(5, 3) == 2);
  Test(RotateIntoRangeByte(6, 3) == 0);
  Test(RotateIntoRangeByte(7, 3) == 1);
}

void TestScaleFlavors()
{
  // check intervals.
  auto& sfmaj = clarinoid::gScaleFlavors[(int)ScaleFlavorIndex::Major];
  Test(sfmaj.NormalizeScaleDegree(-1) == 6);
  Test(sfmaj.NormalizeScaleDegree(-7) == 0);
  Test(sfmaj.NormalizeScaleDegree(-8) == 6);
  Test(sfmaj.NormalizeScaleDegree(0) == 0);
  Test(sfmaj.NormalizeScaleDegree(2) == 2);
  Test(sfmaj.NormalizeScaleDegree(6) == 6);
  Test(sfmaj.NormalizeScaleDegree(7) == 0);

  auto ctx = sfmaj.RelativeChrNoteToContext(2, EnharmonicDirection::Sharp);
  Test(ctx.mEnharmonic == 0);
  Test(ctx.mScaleDegree == 1);

  ctx = sfmaj.RelativeChrNoteToContext(1, EnharmonicDirection::Sharp);
  Test(ctx.mEnharmonic == 1);
  Test(ctx.mScaleDegree == 0);

  ctx = sfmaj.RelativeChrNoteToContext(1, EnharmonicDirection::Flat);
  Test(ctx.mEnharmonic == -1);
  Test(ctx.mScaleDegree == 1);

  ctx = sfmaj.RelativeChrNoteToContext(0, EnharmonicDirection::Flat);
  Test(ctx.mEnharmonic == 0);
  Test(ctx.mScaleDegree == 0);

  ctx = sfmaj.RelativeChrNoteToContext(11, EnharmonicDirection::Sharp);
  Test(ctx.mEnharmonic == 0);
  Test(ctx.mScaleDegree == 6);

  ctx = sfmaj.RelativeChrNoteToContext(10, EnharmonicDirection::Sharp);
  Test(ctx.mEnharmonic == 1);
  Test(ctx.mScaleDegree == 5);

  ctx = sfmaj.RelativeChrNoteToContext(12, EnharmonicDirection::Sharp);
  Test(ctx.mEnharmonic == 0);
  Test(ctx.mScaleDegree == 0);

  auto& sfhmin = clarinoid::gScaleFlavors[(int)ScaleFlavorIndex::HarmonicMinor];
  ctx = sfhmin.RelativeChrNoteToContext(10, EnharmonicDirection::Sharp);
  Test(ctx.mEnharmonic == 2);
  Test(ctx.mScaleDegree == 5);

  ctx = sfhmin.RelativeChrNoteToContext(9, EnharmonicDirection::Flat);
  Test(ctx.mEnharmonic == -2);
  Test(ctx.mScaleDegree == 6);

  int8_t offsetAdj  = 0;
  Test(sfhmin.ContextToChrRelativeNote(clarinoid::NoteInScaleFlavorContext{ 2, -1 }, offsetAdj) == 2); // scale degree 2 = Eb, flat = D = 2.
  Test(sfhmin.ContextToChrRelativeNote(clarinoid::NoteInScaleFlavorContext{ 1, -1 }, offsetAdj) == 1); // scale degree 1 = D, flat = Db = 1.
  uint8_t n = sfhmin.ContextToChrRelativeNote(clarinoid::NoteInScaleFlavorContext{ 0, -1 }, offsetAdj);
  Test(n == 11); // scale degree 0 = C, flat = B = 11.

  // test some oddballs. unison first.
  auto& uni = clarinoid::gScaleFlavors[(int)ScaleFlavorIndex::Unison];
  ctx = uni.RelativeChrNoteToContext(0, EnharmonicDirection::Sharp);
  Test(ctx.mEnharmonic == 0 && ctx.mScaleDegree == 0);
  ctx = uni.RelativeChrNoteToContext(1, EnharmonicDirection::Sharp);
  Test(ctx.mEnharmonic == 1 && ctx.mScaleDegree == 0);
  ctx = uni.RelativeChrNoteToContext(11, EnharmonicDirection::Sharp);
  Test(ctx.mEnharmonic == 11 && ctx.mScaleDegree == 0);
  ctx = uni.RelativeChrNoteToContext(12, EnharmonicDirection::Sharp);
  Test(ctx.mEnharmonic == 0 && ctx.mScaleDegree == 0);
  ctx = uni.RelativeChrNoteToContext(13, EnharmonicDirection::Sharp);
  Test(ctx.mEnharmonic == 1 && ctx.mScaleDegree == 0);
  ctx = uni.RelativeChrNoteToContext(6, EnharmonicDirection::Sharp);
  Test(ctx.mEnharmonic == 6 && ctx.mScaleDegree == 0);

  ctx = uni.RelativeChrNoteToContext(0, EnharmonicDirection::Flat);
  Test(ctx.mEnharmonic == 0 && ctx.mScaleDegree == 0);
  ctx = uni.RelativeChrNoteToContext(1, EnharmonicDirection::Flat);
  Test(ctx.mEnharmonic == -11 && ctx.mScaleDegree == 0);
  ctx = uni.RelativeChrNoteToContext(11, EnharmonicDirection::Flat);
  Test(ctx.mEnharmonic == -1 && ctx.mScaleDegree == 0);
  ctx = uni.RelativeChrNoteToContext(12, EnharmonicDirection::Flat);
  Test(ctx.mEnharmonic == 0 && ctx.mScaleDegree == 0);
  ctx = uni.RelativeChrNoteToContext(13, EnharmonicDirection::Flat);
  Test(ctx.mEnharmonic == -11 && ctx.mScaleDegree == 0);
  ctx = uni.RelativeChrNoteToContext(6, EnharmonicDirection::Flat);
  Test(ctx.mEnharmonic == -6 && ctx.mScaleDegree == 0);

  // test chromatic. enharmonic always 0.
  auto& chr = clarinoid::gScaleFlavors[(int)ScaleFlavorIndex::Chromatic];
  ctx = chr.RelativeChrNoteToContext(0, EnharmonicDirection::Sharp);
  Test(ctx.mEnharmonic == 0 && ctx.mScaleDegree == 0);
  ctx = chr.RelativeChrNoteToContext(7, EnharmonicDirection::Sharp);
  Test(ctx.mEnharmonic == 0 && ctx.mScaleDegree == 7);
  ctx = chr.RelativeChrNoteToContext(11, EnharmonicDirection::Sharp);
  Test(ctx.mEnharmonic == 0 && ctx.mScaleDegree == 11);
  ctx = chr.RelativeChrNoteToContext(12, EnharmonicDirection::Sharp);
  Test(ctx.mEnharmonic == 0 && ctx.mScaleDegree == 0);
}

void TestScales()
{
  TestScaleBasics();
  TestScaleFlavors();

  Scale s = { 2, ScaleFlavorIndex::Major }; // D major
  MidiNote m = { 3, 4 }; // oct3, E
  MidiNote m2 = s.AdjustNoteByInterval(m.GetMidiValue(), 1, EnharmonicDirection::Sharp); // should go to F#
  Test(m2.GetOctave() == 3);
  Test(m2.GetNoteIndex() == 6); // F#

  m2 = s.AdjustNoteByInterval(m.GetMidiValue(), -3, EnharmonicDirection::Sharp); // should go to F#
  Test(m2.GetOctave() == 2);
  Test(m2.GetNoteIndex() == 11); // B
}

