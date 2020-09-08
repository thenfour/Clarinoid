#pragma once

#include <Shared_CCUtil.h>
#include "Harmonizer.hpp"

// transpose -> looperharmonizer.

struct LoopLayer
{
  // when a layer is committed, save the pointer & length here from a master buffer. too much fragmentation if we gave each layer its own.
  struct MusicalEvent* mEvents;
  size_t mEventCount = 0;
  // harmonizer settings reference.
  //int mHarmonizerPreset;
  bool mHarmonizerEnabled;
  bool mIsConcrete;
  // event memory
};

struct LooperAndHarmonizer
{
  LoopLayer mLayers[LOOP_LAYERS];
  Harmonizer mHarmonizer;

  // here you can record a loop or insert notes.
  // return the # of notes recorded.
  size_t Update(const MusicalVoice& liveVoice, MusicalVoice* outp, const MusicalVoice* outpEnd) {

    // fill out the loop layers, which allows the scale follower to operate.
    // you want the scale follower to see stuff in all other layers. so like,
    // any layers which are concrete can just go for it.
    // then we hit the scale follower, and then fill the remaining layers.
    /*
    
    fill concrete layers, feeding the scale follower.
    now given concrete layers, fill remaining voices.

    the trick will be keeping voice allocation consistent. when a note on happens, it will get assigned a voice and can keep it
    the synth already handles whatever voiceID you give it and makes sure similar voice ID will get the same synthID.
    so I'm thinking voiceIDs could be a bitfield of layer_id:harmvoice_id

    it means musical voice id should be assigned as soon as its filled in, here in this function.
    
    */
    if (gAppSettings.mHarmSettings.mIsEnabled) {
      return mHarmonizer.Update(0, liveVoice, outp, outpEnd);
    } else {
      outp[0] = liveVoice;
      outp[0].mVoiceId = MakeMusicalVoiceID(0, 0);
      return 1;
    }


  }
};
