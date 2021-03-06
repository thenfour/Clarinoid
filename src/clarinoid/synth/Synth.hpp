
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{
  static constexpr float pitchBendRange = 2.0f;
} // namespace clarinoid

#ifdef CLARINOID_MODULE_TEST
#include "MockSynthVoice.hpp"
#else
#include "SynthVoice.hpp"
#endif

namespace clarinoid
{
  static float gPeak = 0;

  struct CCSynth
  {
    size_t mCurrentPolyphony = 0;
    AppSettings *mAppSettings;
    Metronome *mMetronome;

    void Init(AppSettings *appSettings, Metronome *metronome)
    {
      mAppSettings = appSettings;
      mMetronome = metronome;
      gSynthGraphControl.Setup(appSettings, metronome);
    }

    // returns a voice that's either already assigned to this voice, or the best one to free up for it.
    Voice *FindAssignedOrAvailable(int16_t musicalVoiceId)
    {
      Voice *firstFree = nullptr;
      for (auto &v : gVoices)
      {
        if (v.mRunningVoice.mVoiceId == musicalVoiceId)
        {
          return &v; // already assigned to this voice.
        }
        if (!firstFree && (v.mRunningVoice.mVoiceId == MAGIC_VOICE_ID_UNASSIGNED))
        {
          firstFree = &v;
        }
      }
      if (firstFree)
      {
        return firstFree;
      }
      // no free voices. in this case find the oldest.
      // TODO.
      return &gVoices[0];
    }

    // void SetGain(float f) {
    //   gSynthGraphControl.SetGain(f);
    // }

    // After musical state has been updated, call this to apply those changes to the synth state.
    void Update(const MusicalVoice *pVoicesBegin, const MusicalVoice *pVoicesEnd)
    {
      gSynthGraphControl.BeginUpdate();
      mCurrentPolyphony = 0;

      if (CCSynthGraph::peak1.available())
      {
        gPeak = CCSynthGraph::peak1.read();
      }

      for (const MusicalVoice *pvoice = pVoicesBegin; pvoice != pVoicesEnd; ++pvoice)
      {
        auto &mv = *pvoice;
        Voice *pv = FindAssignedOrAvailable(mv.mVoiceId);
        CCASSERT(!!pv);
        pv->Update(mv);
        if (mv.IsPlaying())
        {
          mCurrentPolyphony++;
        }
      }

      for (auto &v : gVoices)
      {
        if (!v.IsPlaying())
        {
          v.Unassign();
        }
      }

      gSynthGraphControl.UpdatePostFx();
      gSynthGraphControl.EndUpdate();
    }

    static float GetPeakLevel()
    {
      return gPeak;
    }
  };

} // namespace clarinoid
