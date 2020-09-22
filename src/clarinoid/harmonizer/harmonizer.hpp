
#pragma once

#include <clarinoid/basic/Basic.hpp>
#include "AnalogValue.hpp"
#include "MusicalVoice.hpp"


struct Harmonizer
{
  // state & processing for harmonizer.

  enum VoiceFilterOptions
  {
    ExcludeDeducedVoices,
    OnlyDeducedVoices,
  };

  size_t mSequencePos = 0;
  Stopwatch mRotationTriggerTimer;

  // called each frame to add harmonizer voices to the output, given the live playing voice.
  // liveVoice is considered a part of the output. It will be muted or unmuted whether it should be part of playback
  // returns the number of voices added (including live voice)
  // layerID is needed in order to create the voiceID
  size_t Harmonize(uint8_t loopLayerID, MusicalVoice* liveVoice, const MusicalVoiceTransitionEvents& transitionEvents, MusicalVoice* outp, MusicalVoice* end, VoiceFilterOptions voiceFilter) {
    HarmPreset& preset = FindHarmPreset(liveVoice->mHarmPatch);

    size_t ret = 0;

    // advance sequence pointer?
    if (transitionEvents.mNeedsNoteOn) {
      if (mRotationTriggerTimer.ElapsedMillis() >= preset.mMinRotationTimeMS)
      {
        mRotationTriggerTimer.Restart();
        mSequencePos ++;
      }
    }

    // LIVE note:
    // harmonizing should always output the live note; if it's not part of the real harmonized output,
    // then mark it as muted. it needs to be there so the scale deducer can use it.
    liveVoice->mIsNoteCurrentlyMuted = !preset.mEmitLiveNote;
    liveVoice->mVoiceId = MakeMusicalVoiceID(loopLayerID, 0);
    if (voiceFilter == Harmonizer::VoiceFilterOptions::ExcludeDeducedVoices)
      ++ ret; // live voice is a non-deduced voice.

    if (!gAppSettings.mHarmSettings.mIsEnabled) {
      return ret;
    }

    MusicalVoice* pout = outp;
    
    bool globalDeduced = gAppSettings.mGlobalScaleRef == ScaleRefType::Deduced;
    Scale* pGlobalScale = globalDeduced ? &gAppSettings.mDeducedScale : &gAppSettings.mGlobalScale;

    for (size_t nVoice = 0; nVoice < SizeofStaticArray(preset.mVoiceSettings); ++ nVoice)
    {
      auto& hv = preset.mVoiceSettings[nVoice];

      // can we skip straight away?
      if (hv.mVoiceType == HarmVoiceType::Off)
        continue;
      if (hv.mSequence[0].mEnd || hv.mSequenceLength == 0)
        continue;
      if (pout >= end) {
        return ret;
      }

      // is it a deduced voice? in other words, one that a scale follower selects? we may need to filter it.
      bool deduced = false;
      Scale* pScale = nullptr;

      switch (hv.mScaleRef) {
      case ScaleRefType::Local:
        pScale = &hv.mLocalScale;
      case ScaleRefType::Global:
        deduced = globalDeduced;
        pScale = pGlobalScale;
        break;
      case ScaleRefType::Deduced:
        deduced = true;
        pScale = &gAppSettings.mDeducedScale;
        break;
      } 

      bool wantDeduced = (voiceFilter == VoiceFilterOptions::OnlyDeducedVoices);
      if (wantDeduced != deduced)
        continue;

      *pout = *liveVoice; // copy from live voice to get started.
      pout->mVoiceId = MakeMusicalVoiceID(loopLayerID, nVoice);

      CCASSERT(!hv.mSequence[mSequencePos % HARM_SEQUENCE_LEN].mEnd);
      if (!pScale->AdjustNoteByInterval(pout->mMidiNote, hv.mSequence[mSequencePos % HARM_SEQUENCE_LEN].mInterval, hv.mNonDiatonicBehavior)) {
        continue;
      }

      // corrective settings...
      switch (hv.mNoteOOBBehavior) {
      case NoteOOBBehavior::TransposeOctave:
        while (pout->mMidiNote < hv.mMinOutpNote)
          pout->mMidiNote += 12;
        while (pout->mMidiNote > hv.mMaxOutpNote)
          pout->mMidiNote -= 12;
        break;
      case NoteOOBBehavior::Drop:
        if (pout->mMidiNote < hv.mMinOutpNote)
          continue;
        if (pout->mMidiNote > hv.mMaxOutpNote)
          continue;
        break;
      }

      if (pout->mMidiNote < 1)
        continue;
      if (pout->mVelocity == 0)
        continue;

      switch (hv.mSynthPresetRef)
      {
      case HarmSynthPresetRefType::Global:
        pout->mSynthPatch = gAppSettings.mGlobalSynthPreset;
        break;
      case HarmSynthPresetRefType::Voice:
        pout->mSynthPatch = hv.mVoiceSynthPreset;
        break;
      }

      ++ pout;
      ++ ret;
    }

    
    return ret;
  }
};
