#pragma once

#include <Shared_CCUtil.h>

static constexpr size_t MAX_MUSICAL_VOICES = LOOP_LAYERS * HARM_VOICES;

struct MusicalVoice
{
  MusicalVoice() = default;
  MusicalVoice(const MusicalVoice& rhs) = delete; // no copy ctor.
  MusicalVoice(MusicalVoice&&) = delete;// no move.
  MusicalVoice& operator =(const MusicalVoice& rhs) // assignment is fine for an existing voice obj.
  {
    mIsNoteCurrentlyOn = rhs.mIsNoteCurrentlyOn;
    mNote = rhs.mNote;
    mVelocity = rhs.mVelocity;
    mSynthPatch = rhs.mSynthPatch;
    return *this;
  }
  MusicalVoice& operator =(MusicalVoice&&) = delete; // no move assignment because i want these objects to stay static.
  
  int16_t mVoiceId = -1;
  bool mIsNoteCurrentlyOn = false;
  uint8_t mNote = 0;
  uint8_t mVelocity = 0;
  int16_t mSynthPatch = -1;
};



struct Harmonizer
{
  // state & processing for harmonizer.

  // called each frame to add harmonizer voices to the output, given the live playing voice.
  // return the next available voice.
  MusicalVoice* Update(const MusicalVoice& liveVoice, MusicalVoice* outp, const MusicalVoice* end) {
    *outp = liveVoice;
    outp ++;
    *outp = liveVoice;
    outp->mNote += 7;
    return outp;
  }
};
