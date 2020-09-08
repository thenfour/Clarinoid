#pragma once

#include <Shared_CCUtil.h>
#include "AppSettings.h"

// musical voices have unique IDs which are a bitfield of
// low 8 bits = 
using MusicalVoiceID_t = uint16_t;

static inline MusicalVoiceID_t MakeMusicalVoiceID(uint8_t layerID, uint8_t harmVoice)
{
  static_assert(HARM_VOICES < 256, "harmonizer voice ids must fit into a byte");
  static_assert(LOOP_LAYERS < 256, "loop layer ids must fit into a byte");
  return layerID << 8 | harmVoice;
}

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
  
  MusicalVoiceID_t mVoiceId = -1;
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
  size_t Update(uint8_t layerID, const MusicalVoice& liveVoice, MusicalVoice* outp, const MusicalVoice* end) {
    
    outp[0] = liveVoice;
    outp[0].mVoiceId = MakeMusicalVoiceID(layerID, 0);

    outp[1] = liveVoice;
    outp[1].mNote += 7;
    outp[1].mVoiceId = MakeMusicalVoiceID(layerID, 1);
    
    return 2;
  }
};
