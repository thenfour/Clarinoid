#pragma once

#ifndef EWI_UNIT_TESTS
#include <Shared_CCUtil.h>
#include "AppSettings.h"
#else
static const size_t HARM_VOICES = 6;
static const size_t ANALOG_RESOLUTION_BITS = 12;
#endif // EWI_UNIT_TESTS
#include "../../Shared/EWI/Basic.hpp"

// musical voices have unique IDs which are a bitfield of
// low 8 bits = 
using MusicalVoiceID_t = uint16_t;

static inline MusicalVoiceID_t MakeMusicalVoiceID(uint8_t loopLayerID, uint8_t harmVoice)
{
  static_assert(HARM_VOICES < 256, "harmonizer voice ids must fit into a byte");
  static_assert(LOOP_LAYERS < 256, "loop layer ids must fit into a byte");
  return loopLayerID << 8 | harmVoice;
}

template<size_t BitsResolution = ANALOG_RESOLUTION_BITS>
struct AnalogValue01
{
  using this_t = AnalogValue01<BitsResolution>;
  void SetFloat(float v)
  {
    mFloatVal = v;
    if (v >= 1.0f) {
      mIntVal = (1 << BitsResolution) - 1;
    }
    else if (v <= 0) {
      mIntVal = 0;
    }
    else {
      mIntVal = (uint16_t)(v * (1 << BitsResolution));
    }
  }
  void Deserialize12Bit(uint16_t v) {
    mIntVal = v;
    mFloatVal = (float)v / (1 << BitsResolution);
  }
  uint16_t Serialize12Bit() const { return mIntVal; }
  float GetFloatVal() const { return mFloatVal; }
  bool operator ==(const this_t& rhs) const {
    return mIntVal == rhs.mIntVal;
  }
  //bool operator ==(float rhs) const {
  //  return mFloatVal == rhs;
  //}
  //bool operator ==(int rhs) const {
  //  return mIntVal == rhs;
  //}
  bool operator !=(const this_t& rhs) const {
    return !(*this == rhs);
  }
  bool operator !=(float rhs) const {
    return mFloatVal != rhs;
  }
  //bool operator !=(int rhs) const {
  //  return mIntVal != rhs;
  //}
  //this_t& operator =(int v) { SetInt(v); return *this; }
  this_t& operator =(float v) { SetFloat(v); return *this; }
private:
  uint16_t mIntVal = 0;
  float mFloatVal = 0;
};

struct MusicalVoice
{
  MusicalVoice() = default;
  MusicalVoice(const MusicalVoice& rhs) = delete; // no copy ctor.
  MusicalVoice(MusicalVoice&&) = delete;// no move.
  MusicalVoice& operator =(const MusicalVoice& rhs) = delete;
  MusicalVoice& operator =(MusicalVoice&&) = delete; // no move assignment because i want these objects to stay static.

  void AssignFromLoopStream(const MusicalVoice& rhs)
  {
    CloneForHarmonizationStream(rhs);
  }

  void CloneForHarmonizationStream(const MusicalVoice& rhs)
  {
    // don't change own voice ID!
    mIsNoteCurrentlyOn = rhs.mIsNoteCurrentlyOn;
    mIsNoteCurrentlyMuted = rhs.mIsNoteCurrentlyMuted;
    mMidiNote = rhs.mMidiNote;
    mBreath01 = rhs.mBreath01;
    mPitchBendN11 = rhs.mPitchBendN11;
    mVelocity = rhs.mVelocity;
    mSynthPatch = rhs.mSynthPatch;
    mHarmPatch = rhs.mHarmPatch;

    mDuration = rhs.mDuration;

    // these are not possible to know actually.
    mNeedsNoteOn = rhs.mNeedsNoteOn;
    mNeedsNoteOff = rhs.mNeedsNoteOff;
    mNoteOffNote = rhs.mNoteOffNote;
  }

  void Reset() {
    mIsNoteCurrentlyOn = false;
    mIsNoteCurrentlyMuted = false;
    mMidiNote = 0;
    mBreath01.SetFloat(0.0f);
    mPitchBendN11.SetFloat(0.0f);
    //mDuration.Restart(); not needed
    mVelocity = 0;
    mSynthPatch = 0;
    mHarmPatch = 0;
    mNeedsNoteOn = false;
    mNeedsNoteOff = false;
  }

#ifndef EWI_UNIT_TESTS
  HarmPreset& GetHarmPreset()
  {
    return FindHarmPreset(mHarmPatch != -1, mHarmPatch);
  }
#endif // EWI_UNIT_TESTS

  MusicalVoiceID_t mVoiceId = -1;

  bool mIsNoteCurrentlyOn = false;
  bool mIsNoteCurrentlyMuted = false; // this is needed when this is the "live" voice that has been physically played, but the harmonizer demands we not output it.
  uint8_t mMidiNote = 0;
  AnalogValue01<> mBreath01;
  AnalogValue01<> mPitchBendN11;
  uint8_t mVelocity = 0;
  int16_t mSynthPatch = 0;
  int16_t mHarmPatch = 0;

  Stopwatch mDuration;

  // { valid for 1 frame only, useful for converting stream state into events for looper or midi.
  bool mNeedsNoteOn = false;
  bool mNeedsNoteOff = false;
  uint8_t mNoteOffNote = 0; // ignore this if needsNoteOff is false.
  // } valid for 1 frame only.
};


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

    // harmonizing should always output the live note; if it's not part of the real harmonized output,
    // then mark it as muted. it needs to be there so the scale deducer can use it.
    liveVoice->mIsNoteCurrentlyMuted = false;
    liveVoice->mVoiceId = MakeMusicalVoiceID(loopLayerID, 0);

    if (outp >= end) {
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
