
#pragma once

#include <clarinoid/basic/Basic.hpp>
//#include <clarinoid/application/MusicalState.hpp>

static constexpr float pitchBendRange = 0.0f;

#ifdef CLARINOID_MODULE_TEST
#include "MockSynthVoice.hpp"
#else
#include "SynthVoice.hpp"
#endif


struct CCSynth : UpdateObjectT<ProfileObjectType::Synth>
{
  size_t mCurrentPolyphony = 0;
  //size_t mCurrentRejected = 0;

  // returns a voice that's either already assigned to this voice, or the best one to free up for it.
  Voice* FindAssignedOrAvailable(int16_t musicalVoiceId) {
    Voice* firstFree = nullptr;
    for (auto& v : gVoices) {
      if (v.mRunningVoice.mVoiceId == musicalVoiceId) {
        return &v; // already assigned to this voice.
      }
      if (!firstFree && (v.mRunningVoice.mVoiceId == MAGIC_VOICE_ID_UNASSIGNED)) {
        firstFree = &v;
      }
    }
    if (firstFree) {
      return firstFree;
    }
    // no free voices. in this case find the oldest.
    // TODO.
    return &gVoices[0];
  }

  virtual void setup() {
    gSynthGraphControl.Setup();
  }

  void SetGain(float f) {
    gSynthGraphControl.SetGain(f);
  }

  // After musical state has been updated, call this to apply those changes to the synth state.
  void Update(const MusicalVoice* pVoicesBegin, const MusicalVoice* pVoicesEnd) {
    gSynthGraphControl.BeginUpdate();
    mCurrentPolyphony = 0;
    //mCurrentRejected = 0;

    //SetVoicesUntouched();
    //size_t voiceCount = pVoicesEnd - pVoicesBegin;
    //for (size_t imv = 0; imv < voiceCount; ++ imv) {
//      auto& mv = state.mMusicalVoices[imv];
    
    for (const MusicalVoice* pvoice = pVoicesBegin; pvoice != pVoicesEnd; ++ pvoice)
    {
      auto& mv = *pvoice;
      Voice* pv = FindAssignedOrAvailable(mv.mVoiceId);
      CCASSERT(!!pv);
      pv->Update(mv);
      if (mv.IsPlaying()) {
        mCurrentPolyphony ++;
      }
    }

    for (auto& v : gVoices) {
      if (!v.IsPlaying()) {
        v.Unassign();
      }
    }

    //UnassignUntouchedVoices();

    gSynthGraphControl.UpdatePostFx();
    gSynthGraphControl.EndUpdate();
  }
};


