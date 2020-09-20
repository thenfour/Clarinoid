
#pragma once

#include <clarinoid/basic/Basic.hpp>
//#include <clarinoid/application/MusicalState.hpp>

static constexpr float pitchBendRange = 0.0f;
static constexpr int16_t MAGIC_VOICE_ID_UNASSIGNED = -1; // used as a voice ID for voices that aren't assigned to any musical voice.

#ifdef CLARINOID_MODULE_TEST
#include "MockSynthVoice.hpp"
#else
#include "SynthVoice.hpp"
#endif


struct CCSynth : UpdateObjectT<ProfileObjectType::Synth>
{
  size_t mCurrentPolyphony = 0;
  size_t mCurrentRejected = 0;

  // returns a voice that's either already assigned to this voice, or the best one to free up for it.
  Voice* FindAssignedOrAvailable(int16_t musicalVoiceId) {
    Voice* firstFree = nullptr;
    for (auto& v : gVoices) {
      if (v.mMusicalVoiceId == musicalVoiceId) {
        return &v; // already assigned to this voice.
      }
      if (!firstFree && (v.mMusicalVoiceId == MAGIC_VOICE_ID_UNASSIGNED)) {
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

  // returns null if this voice isn't assigned.
  Voice* FindAssigned(int16_t musicalVoiceId) {
    for (auto& v : gVoices) {
      if (v.mMusicalVoiceId == musicalVoiceId) {
        return &v;
      }
    }
    return nullptr;
  }

  void SetVoicesUntouched() {
    for (auto& v : gVoices) {
      v.mTouched = false;
    }
  }
  void UnassignUntouchedVoices() {
    for (auto& v : gVoices) {
      v.mMusicalVoiceId = MAGIC_VOICE_ID_UNASSIGNED;
    }
  }

  virtual void setup() {
    gSynthGraphControl.Setup();
  }

  void SetGain(float f) {
    gSynthGraphControl.SetGain(f);
  }

  void Update(const MusicalVoice* pVoicesBegin, const MusicalVoice* pVoicesEnd) {
    gSynthGraphControl.BeginUpdate();
    mCurrentPolyphony = 0;
    mCurrentRejected = 0;

    SetVoicesUntouched();
    size_t voiceCount = pVoicesEnd - pVoicesBegin;
    //for (size_t imv = 0; imv < voiceCount; ++ imv) {
//      auto& mv = state.mMusicalVoices[imv];
    
    for (const MusicalVoice* pvoice = pVoicesBegin; pvoice != pVoicesEnd; ++ pvoice)
    {
      auto& mv = *pvoice;
      if (mv.mIsNoteCurrentlyOn || mv.mNeedsNoteOff) {
        mCurrentPolyphony ++;
        Voice* pv = FindAssignedOrAvailable(mv.mVoiceId);
        CCASSERT(!!pv);
        pv->UpdatePlaying(mv);
        pv->mTouched = true;
      } else {
        mCurrentRejected++;
        Voice* pv = FindAssigned(mv.mVoiceId);
        if (pv) {
          pv->UpdateStopped();
          pv->mTouched = true;
        }
      }
    }

    UnassignUntouchedVoices();

    gSynthGraphControl.UpdatePostFx();
    gSynthGraphControl.EndUpdate();
  }
};


