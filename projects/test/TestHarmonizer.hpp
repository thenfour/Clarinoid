
#pragma once

#include "Test.hpp"
#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/basic/Music.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include <clarinoid/harmonizer/harmonizer.hpp>

using clarinoid::GlobalScaleRefType;

void TestHarmonizer()
{
  clarinoid::AppSettings appSettings;
  clarinoid::Harmonizer harm(&appSettings);
  MusicalVoice mv[clarinoid::HARM_VOICES + 1];
  MusicalVoice& lv(mv[0]);
  clarinoid::MusicalVoiceTransitionEvents te;
  appSettings.mGlobalScaleRef = GlobalScaleRefType::Chosen;
  appSettings.mGlobalScale = Scale{ Note::E, ScaleFlavorIndex::MajorPentatonic };
  //gAppSettings.mGlobalHarmPreset = 1;
  lv.mHarmPatch = 1;

  // use preset #1.
  auto& preset = appSettings.mHarmSettings.mPresets[1];

  // no voices, don't emit live note.
  preset = clarinoid::HarmPreset();
  preset.mEmitLiveNote = false;
  lv = MusicalVoice();
  lv.mMidiNote = MidiNote(4, Note::Gb).GetMidiValue();
  lv.mVelocity = 50;
  lv.mHarmPatch = 1;
  size_t vc = harm.Harmonize(2, &lv, te, mv + 1, EndPtr(mv), clarinoid::Harmonizer::VoiceFilterOptions::AllExceptDeducedVoices);
  Test(vc == 1);
  Test(lv.mIsNoteCurrentlyMuted = true);
  Test(lv.mVoiceId == clarinoid::MakeMusicalVoiceID(2, 0));

  // emit live voice (default behavior)
  preset = clarinoid::HarmPreset();
  lv = MusicalVoice();
  lv.mMidiNote = MidiNote(4, Note::Gb).GetMidiValue();
  lv.mVelocity = 50;
  lv.mHarmPatch = 1;
  vc = harm.Harmonize(2, &lv, te, mv + 1, EndPtr(mv), clarinoid::Harmonizer::VoiceFilterOptions::AllExceptDeducedVoices);
  Test(vc == 1);
  Test(lv.mIsNoteCurrentlyMuted == false);
  Test(lv.mMidiNote == MidiNote(4, Note::Gb).GetMidiValue());
  Test(lv.mVelocity == 50);
  Test(lv.mVoiceId == clarinoid::MakeMusicalVoiceID(2, 0));

  // add a 5th chromatic voice.
  preset = clarinoid::HarmPreset();
  preset.mVoiceSettings[0].mScaleRef = clarinoid::HarmScaleRefType::Voice;
  preset.mVoiceSettings[0].mLocalScale = Scale{ Note::C, ScaleFlavorIndex::Chromatic };
  preset.mVoiceSettings[0].mSequence[0] = 7;
  preset.mVoiceSettings[0].mSequenceLength = 1;
  lv = MusicalVoice();
  lv.mMidiNote = MidiNote(4, Note::Gb).GetMidiValue();
  lv.mVelocity = 50;
  lv.mHarmPatch = 1;
  vc = harm.Harmonize(2, &lv, te, mv + 1, EndPtr(mv), clarinoid::Harmonizer::VoiceFilterOptions::AllExceptDeducedVoices);
  Test(vc == 2);
  Test(lv.mIsNoteCurrentlyMuted == false);
  Test(lv.mMidiNote == MidiNote(4, Note::Gb).GetMidiValue());
  Test(lv.mVelocity == 50);
  Test(lv.mVoiceId == clarinoid::MakeMusicalVoiceID(2, 0));

  Test(mv[1].mIsNoteCurrentlyMuted == false);
  Test(mv[1].mMidiNote == MidiNote(5, Note::Db).GetMidiValue());
  Test(mv[1].mVelocity == 50);
  Test(mv[1].mVoiceId == clarinoid::MakeMusicalVoiceID(2, 1));

  // global scale usage { E major pentatonic -2} 
  preset = clarinoid::HarmPreset();
  preset.mVoiceSettings[0].mScaleRef = clarinoid::HarmScaleRefType::Global;
  preset.mVoiceSettings[0].mSequence[0] = -2;
  preset.mVoiceSettings[0].mSequenceLength = 1;

  lv = MusicalVoice();
  lv.mMidiNote = MidiNote(4, Note::Gb).GetMidiValue();
  lv.mVelocity = 50;
  lv.mHarmPatch = 1;
  vc = harm.Harmonize(2, &lv, te, mv + 1, EndPtr(mv), clarinoid::Harmonizer::VoiceFilterOptions::AllExceptDeducedVoices);
  Test(vc == 2);
  Test(mv[1].mIsNoteCurrentlyMuted == false);
  Test(mv[1].mMidiNote == MidiNote(4, Note::Db).GetMidiValue());
  Test(mv[1].mVelocity == 50);
  Test(mv[1].mVoiceId == clarinoid::MakeMusicalVoiceID(2, 1));


  // { chromatic +perfect5th ; E major -2} 
  preset = clarinoid::HarmPreset();
  preset.mVoiceSettings[0].mScaleRef = clarinoid::HarmScaleRefType::Voice;
  preset.mVoiceSettings[0].mLocalScale = Scale{ Note::C, ScaleFlavorIndex::Chromatic };
  preset.mVoiceSettings[0].mSequence[0] = 7;
  preset.mVoiceSettings[0].mSequenceLength = 1;

  preset.mVoiceSettings[1].mScaleRef = clarinoid::HarmScaleRefType::Global;
  preset.mVoiceSettings[1].mSequence[0] = -3;
  preset.mVoiceSettings[1].mSequenceLength = 1;

  lv = MusicalVoice();
  lv.mMidiNote = MidiNote(4, Note::Gb).GetMidiValue();
  lv.mVelocity = 50;
  lv.mHarmPatch = 1;
  vc = harm.Harmonize(2, &lv, te, mv + 1, EndPtr(mv), clarinoid::Harmonizer::VoiceFilterOptions::AllExceptDeducedVoices);
  Test(vc == 3);
  Test(lv.mIsNoteCurrentlyMuted == false);
  Test(lv.mMidiNote == MidiNote(4, Note::Gb).GetMidiValue());
  Test(lv.mVelocity == 50);
  Test(lv.mVoiceId == clarinoid::MakeMusicalVoiceID(2, 0));

  Test(mv[1].mIsNoteCurrentlyMuted == false);
  Test(mv[1].mMidiNote == MidiNote(5, Note::Db).GetMidiValue());
  Test(mv[1].mVelocity == 50);
  Test(mv[1].mVoiceId == clarinoid::MakeMusicalVoiceID(2, 1));

  Test(mv[2].mIsNoteCurrentlyMuted == false);
  Test(mv[2].mMidiNote == MidiNote(3, Note::B).GetMidiValue());
  Test(mv[2].mVelocity == 50);
  Test(mv[2].mVoiceId == clarinoid::MakeMusicalVoiceID(2, 2));

  // test sequence (note on required)
  // test deduced voice
  // test OOB note
  // test velocity scaling
  // test nondiatonic behavior
  // test synth patch mapping
}

