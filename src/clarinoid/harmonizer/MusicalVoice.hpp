
#pragma once

#include <clarinoid/basic/Basic.hpp>
#include "AnalogValue.hpp"

// musical voices have unique IDs which are a bitfield of
// low 8 bits = 
using MusicalVoiceID_t = uint16_t;

static constexpr MusicalVoiceID_t MAGIC_VOICE_ID_UNASSIGNED = std::numeric_limits<MusicalVoiceID_t>::max(); // used as a voice ID for voices that aren't assigned to any musical voice.

static inline MusicalVoiceID_t MakeMusicalVoiceID(uint8_t loopLayerID, uint8_t harmVoice)
{
  static_assert(HARM_VOICES < 256, "harmonizer voice ids must fit into a byte");
  static_assert(LOOP_LAYERS < 256, "loop layer ids must fit into a byte");
  return loopLayerID << 8 | harmVoice;
}

struct MusicalVoice
{
  MusicalVoice() = default;
  MusicalVoice(const MusicalVoice& rhs) = default; // no copy ctor.
  MusicalVoice(MusicalVoice&&) = default;// no move.
  MusicalVoice& operator =(const MusicalVoice& rhs) = default;
  MusicalVoice& operator =(MusicalVoice&&) = default; // no move assignment because i want these objects to stay static.

  // void AssignExceptID(const MusicalVoice& rhs)
  // {
  //   // don't change own voice ID!
  //   //mIsNoteCurrentlyOn = rhs.mIsNoteCurrentlyOn;
  //   mIsNoteCurrentlyMuted = rhs.mIsNoteCurrentlyMuted;
  //   mMidiNote = rhs.mMidiNote;
  //   mBreath01 = rhs.mBreath01;
  //   mPitchBendN11 = rhs.mPitchBendN11;
  //   mVelocity = rhs.mVelocity;
  //   mSynthPatch = rhs.mSynthPatch;
  //   mHarmPatch = rhs.mHarmPatch;

  //   //mDuration = rhs.mDuration;

  //   // these are not possible to know actually.
  //   //mNeedsNoteOn = rhs.mNeedsNoteOn;
  //   //mNeedsNoteOff = rhs.mNeedsNoteOff;
  //   //mNoteOffNote = rhs.mNoteOffNote;
  // }

  void Reset() {
    //mIsNoteCurrentlyOn = false;
    mIsNoteCurrentlyMuted = false;
    mMidiNote = 0;
    mBreath01.SetFloat(0.0f);
    mPitchBendN11.SetFloat(0.0f);
    //mDuration.Restart(); not needed
    mVelocity = 0;
    mSynthPatch = 0;
    mHarmPatch = 0;
    //mNeedsNoteOn = false;
    //mNeedsNoteOff = false;
  }

  bool IsPlaying() const {
    return !!mMidiNote && !!mVelocity;
  }

  //void ResetOneFrameState() {
  //  mNeedsNoteOn = false;
  //  mNeedsNoteOff = false;
  //  mNoteOffNote = 0;
  //}

  // HarmPreset& GetHarmPreset()
  // {
  //   return FindHarmPreset(mHarmPatch != -1, mHarmPatch);
  // }

  MusicalVoiceID_t mVoiceId = MAGIC_VOICE_ID_UNASSIGNED; // this is really an outlier member; it's NOT musical state but useful to keep here anyway even if it makes thinsg confusing.

  //bool mIsNoteCurrentlyOn = false; // if true, then mNeedsNoteOff must be false.
  bool mIsNoteCurrentlyMuted = false; // this is needed when this is the "live" voice that has been physically played, but the harmonizer demands we not output it.
  uint8_t mMidiNote = 0;
  AnalogValue01<> mBreath01;
  AnalogValueN11<> mPitchBendN11;
  uint8_t mVelocity = 0;
  int16_t mSynthPatch = 0;
  int16_t mHarmPatch = 0;

  //Stopwatch mDuration;

  // { valid for 1 frame only, useful for converting stream state into events for looper or midi.
  //bool mNeedsNoteOn = false;
  //bool mNeedsNoteOff = false; // if true, mIsNoteCurrentlyOn must be false.
  //uint8_t mNoteOffNote = 0; // ignore this if needsNoteOff is false.
  // } valid for 1 frame only.
};

struct MusicalVoiceTransitionEvents
{
  bool mNeedsNoteOn = false;
  bool mNeedsNoteOff = false;
  uint8_t mNoteOffNote;
};

// computes the note on & note off events as the result of musical state.
MusicalVoiceTransitionEvents CalculateTransitionEvents(const MusicalVoice& a, const MusicalVoice& b)
{
  MusicalVoiceTransitionEvents ret;
  bool changedPlayingMidiNote = a.IsPlaying() && b.IsPlaying() && (a.mMidiNote != b.mMidiNote);
  ret.mNeedsNoteOn = (b.IsPlaying() && !a.IsPlaying()) || changedPlayingMidiNote;
  // send note off in these cases:
  // - you are not playing but were
  // - or, you are playing, but a different note than before.
  ret.mNeedsNoteOff = (!b.IsPlaying() && a.IsPlaying()) || changedPlayingMidiNote;
  ret.mNoteOffNote = a.mMidiNote;

  return ret;
}

