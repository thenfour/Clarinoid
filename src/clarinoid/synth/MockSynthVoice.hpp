
#pragma once

#include <clarinoid/basic/Basic.hpp>
//#include <clarinoid/application/MusicalState.hpp>


struct Voice
{
  int16_t mMusicalVoiceId = MAGIC_VOICE_ID_UNASSIGNED;
  bool mTouched = false;

  void EnsurePatchConnections()
  {
  }
  
  void UpdateStopped()
  {
  }
  
  void UpdatePlaying(const MusicalVoice& mv)
  {
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

