
#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include "AnalogValue.hpp"
#include "MusicalVoice.hpp"

namespace clarinoid
{

struct Harmonizer
{
  AppSettings* mAppSettings;

  explicit Harmonizer(AppSettings* appSettings) : mAppSettings(appSettings)
  {
  }

  // state & processing for harmonizer.

  enum class VoiceFilterOptions : uint8_t
  {
    AllExceptDeducedVoices,
    OnlyDeducedVoices,
  };

  size_t mSequencePos = 0;
  Stopwatch mRotationTriggerTimer;

  // called each frame to add harmonizer voices to the output, given the live playing voice.
  // liveVoice is considered a part of the output. It will be muted or unmuted whether it should be part of playback
  // returns the number of voices added (including live voice, even if muted)
  // layerID is needed in order to create the voiceID
  size_t Harmonize(uint8_t loopLayerID, MusicalVoice* liveVoice, const MusicalVoiceTransitionEvents& transitionEvents, MusicalVoice* outp, MusicalVoice* end, VoiceFilterOptions voiceFilter) {
    HarmPreset& preset = mAppSettings->FindHarmPreset(liveVoice->mHarmPatch);

    size_t ret = 0;

    // advance sequence pointer?
    if (transitionEvents.mNeedsNoteOn) {
      if (mRotationTriggerTimer.ElapsedTime().ElapsedMillisI() >= preset.mMinRotationTimeMS)
      {
        mRotationTriggerTimer.Restart();
        mSequencePos ++;
        //Serial.println(String("seq") + mSequencePos);
      }
    }

    // LIVE note:
    // harmonizing should always output the live note; if it's not part of the real harmonized output,
    // then mark it as muted. it needs to be there so the scale deducer can use it.
    liveVoice->mIsNoteCurrentlyMuted = !preset.mEmitLiveNote;
    liveVoice->mVoiceId = MakeMusicalVoiceID(loopLayerID, 0);
    if (voiceFilter == Harmonizer::VoiceFilterOptions::AllExceptDeducedVoices)
    {
      ++ ret; // live voice is a non-deduced voice.
    }

    MusicalVoice* pout = outp;
    
    bool globalDeduced = mAppSettings->mGlobalScaleRef == GlobalScaleRefType::Deduced;
    Scale globalScale = globalDeduced ? mAppSettings->mDeducedScale : mAppSettings->mGlobalScale;

            //CCPlot(String("globalScale:") + globalScale.ToString() + ", isdeduced=" + (globalDeduced ? "yes" : "no"));

    for (size_t nVoice = 0; nVoice < SizeofStaticArray(preset.mVoiceSettings); ++ nVoice)
    {
      auto& hv = preset.mVoiceSettings[nVoice];

      // can we skip straight away?
      if (hv.mSequenceLength == 0)
        continue;
      if (pout >= end) {
        return ret;
      }

      // is it a deduced voice? in other words, one that a scale follower selects? we may need to filter it.
      bool deduced = false;
      Scale scale;

      switch (hv.mScaleRef) {
      case HarmScaleRefType::Voice:
        scale = hv.mLocalScale;
        break;
      case HarmScaleRefType::Global:
        deduced = globalDeduced;
        scale = globalScale;
        break;
      } 

      bool wantDeduced = (voiceFilter == VoiceFilterOptions::OnlyDeducedVoices);
      //CCPlot(String("wantdeduced:") + (wantDeduced ? "yes" : "no") + "voiceFilter=" + (int)(voiceFilter));
      if (wantDeduced != deduced)
        continue;

      //if (deduced) {
      //      CCPlot(String("harmpreset:") + int(liveVoice->mHarmPatch));
      //}


      *pout = *liveVoice; // copy from live voice to get started.
      pout->mVoiceId = MakeMusicalVoiceID(loopLayerID, (uint8_t)(nVoice + 1)); // +1 because live voice is id 0.

      // todo: use hv.mNonDiatonicBehavior
      auto newNote = scale.AdjustNoteByInterval(pout->mMidiNote, hv.mSequence[mSequencePos % hv.mSequenceLength], EnharmonicDirection::Sharp);
      if (!newNote) {
        continue;
      }

      pout->mMidiNote = newNote;

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
        pout->mSynthPatch = mAppSettings->mGlobalSynthPreset;
        break;
      case HarmSynthPresetRefType::Preset1:
        pout->mSynthPatch = preset.mSynthPreset1;
        break;
      case HarmSynthPresetRefType::Preset2:
        pout->mSynthPatch = preset.mSynthPreset2;
        break;
      case HarmSynthPresetRefType::Preset3:
        pout->mSynthPatch = preset.mSynthPreset3;
        break;
      case HarmSynthPresetRefType::Voice:
        pout->mSynthPatch = hv.mVoiceSynthPreset;
        break;
      }

      ++ pout;
      ++ ret;
    } // for voice
    
    return ret;
  }
};

} // namespace clarinoid
