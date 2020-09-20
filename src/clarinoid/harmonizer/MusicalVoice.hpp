
#pragma once

#include <clarinoid/basic/Basic.hpp>
#include "AnalogValue.hpp"

// musical voices have unique IDs which are a bitfield of
// low 8 bits = 
using MusicalVoiceID_t = uint16_t;

static inline MusicalVoiceID_t MakeMusicalVoiceID(uint8_t loopLayerID, uint8_t harmVoice)
{
  static_assert(HARM_VOICES < 256, "harmonizer voice ids must fit into a byte");
  static_assert(LOOP_LAYERS < 256, "loop layer ids must fit into a byte");
  return loopLayerID << 8 | harmVoice;
}

struct MusicalVoice
{
  MusicalVoice() = default;
  MusicalVoice(const MusicalVoice& rhs) = delete; // no copy ctor.
  MusicalVoice(MusicalVoice&&) = delete;// no move.
  MusicalVoice& operator =(const MusicalVoice& rhs) = delete;
  MusicalVoice& operator =(MusicalVoice&&) = delete; // no move assignment because i want these objects to stay static.

  void AssignFromLoopStream(const MusicalVoice& rhs)
  {
    CloneForHarmonizationStream(rhs);
  }

  void CloneForHarmonizationStream(const MusicalVoice& rhs)
  {
    // don't change own voice ID!
    mIsNoteCurrentlyOn = rhs.mIsNoteCurrentlyOn;
    mIsNoteCurrentlyMuted = rhs.mIsNoteCurrentlyMuted;
    mMidiNote = rhs.mMidiNote;
    mBreath01 = rhs.mBreath01;
    mPitchBendN11 = rhs.mPitchBendN11;
    mVelocity = rhs.mVelocity;
    mSynthPatch = rhs.mSynthPatch;
    mHarmPatch = rhs.mHarmPatch;

    mDuration = rhs.mDuration;

    // these are not possible to know actually.
    mNeedsNoteOn = rhs.mNeedsNoteOn;
    mNeedsNoteOff = rhs.mNeedsNoteOff;
    mNoteOffNote = rhs.mNoteOffNote;
  }

  void Reset() {
    mIsNoteCurrentlyOn = false;
    mIsNoteCurrentlyMuted = false;
    mMidiNote = 0;
    mBreath01.SetFloat(0.0f);
    mPitchBendN11.SetFloat(0.0f);
    //mDuration.Restart(); not needed
    mVelocity = 0;
    mSynthPatch = 0;
    mHarmPatch = 0;
    mNeedsNoteOn = false;
    mNeedsNoteOff = false;
  }

  void ResetOneFrameState() {
    mNeedsNoteOn = false;
    mNeedsNoteOff = false;
    mNoteOffNote = 0;
  }

  // HarmPreset& GetHarmPreset()
  // {
  //   return FindHarmPreset(mHarmPatch != -1, mHarmPatch);
  // }

  MusicalVoiceID_t mVoiceId = -1;

  bool mIsNoteCurrentlyOn = false;
  bool mIsNoteCurrentlyMuted = false; // this is needed when this is the "live" voice that has been physically played, but the harmonizer demands we not output it.
  uint8_t mMidiNote = 0;
  AnalogValue01<> mBreath01;
  AnalogValueN11<> mPitchBendN11;
  uint8_t mVelocity = 0;
  int16_t mSynthPatch = 0;
  int16_t mHarmPatch = 0;

  Stopwatch mDuration;

  // { valid for 1 frame only, useful for converting stream state into events for looper or midi.
  bool mNeedsNoteOn = false;
  bool mNeedsNoteOff = false;
  uint8_t mNoteOffNote = 0; // ignore this if needsNoteOff is false.
  // } valid for 1 frame only.
};
