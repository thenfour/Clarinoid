
#pragma once

#include <clarinoid/basic/Basic.hpp>
//#include <clarinoid/application/MusicalState.hpp>


struct Voice
{
  //int16_t mMusicalVoiceId = MAGIC_VOICE_ID_UNASSIGNED;
  //bool mTouched = false;

  MusicalVoice mRunningVoice;

  void EnsurePatchConnections()
  {
  }
  
  void Update(const MusicalVoice& mv)
  {
    mRunningVoice = mv;
  }

  bool IsPlaying() const {
    return mRunningVoice.IsPlaying();
  }
  void Unassign() {
    mRunningVoice.mVoiceId = MAGIC_VOICE_ID_UNASSIGNED;
  }
};

Voice gVoices[MAX_SYNTH_VOICES];


struct SynthGraphControl
{
  CCThrottlerT<500> mMetronomeTimer;

  void Setup()
  {
  }

  void SetGain(float f) {
  }

  void BeginUpdate() {
  }

  void EndUpdate() {
  }

  void UpdatePostFx() {
  }
};

SynthGraphControl gSynthGraphControl;

