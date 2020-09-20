
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

  // called each frame to add harmonizer voices to the output, given the live playing voice.
  // liveVoice is considered a part of the output. It will be muted or unmuted whether it should be part of playback
  // returns the number of voices added (including live voice)
  // layerID is needed in order to create the voiceID
  size_t Harmonize(uint8_t loopLayerID, MusicalVoice* liveVoice, MusicalVoice* outp, MusicalVoice* end, VoiceFilterOptions voiceFilter) {
    //HarmPreset& preset = liveVoice->GetHarmPreset();

    switch (voiceFilter) {
    case Harmonizer::VoiceFilterOptions::OnlyDeducedVoices:
      return 0;
    case Harmonizer::VoiceFilterOptions::ExcludeDeducedVoices:
      break;
    }

    // harmonizing should always output the live note; if it's not part of the real harmonized output,
    // then mark it as muted. it needs to be there so the scale deducer can use it.
    liveVoice->mIsNoteCurrentlyMuted = false;
    liveVoice->mVoiceId = MakeMusicalVoiceID(loopLayerID, 0);

    if (outp >= end) {
      return 1;
    }
    if (!gAppSettings.mHarmSettings.mIsEnabled) {
      return 1;
    }

    // todo: support intervals that are more complex. like "at least a m3rd away" which helps keep consistent voices between scales
    // of various densities
    // between

    // TODO: arpeggiator mode where instead of playing polyphonic, it cycles between notes with breath.

    outp[0].CloneForHarmonizationStream(*liveVoice);
    outp[0].mMidiNote += 7;
    outp[0].mVoiceId = MakeMusicalVoiceID(loopLayerID, 1);
    
    return 2;
  }
};
