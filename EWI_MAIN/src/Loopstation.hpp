#pragma once

#include <Shared_CCUtil.h>
#include "Harmonizer.hpp"

struct Looper
{
  // here you can record a loop or insert notes.
  // return the next item
  MusicalVoice* Update(const MusicalVoice* playingVoices, const MusicalVoice* playingEnd, MusicalVoice* outp, const MusicalVoice* outpEnd) {
    return outp;
  }
};
