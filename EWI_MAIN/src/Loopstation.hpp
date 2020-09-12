
#pragma once

#include <Shared_CCUtil.h>
#include "Harmonizer.hpp"
#include "ScaleFollower.hpp"

static const size_t LOOPER_MEMORY_TOTAL_BYTES = 100000;
//static const size_t LOOPER_WRITEBUFFER_TOTAL_BYTES = 50000;
static const size_t LOOPEVENTTYPE_BITS = 4;
static const size_t LOOPEVENTTIME_BITS = 12; // 12 bits of milliseconds is 4 seconds; pretty reasonable
static const uint32_t LOOPEVENTTIME_MAX = (1 << LOOPEVENTTIME_BITS) - 1;
static const uint32_t LOOP_BREATH_PITCH_RESOLUTION_MS = 5; // record only every N milliseconds max. This should probably be coordinated with the similar throttler in MusicalState, to make sure it plays well together
static const float LOOP_BREATH_PITCH_EPSILON = 1.0f / 1024; // resolution to track.

enum class LoopEventType
{
  Nop, // {  } supports long time rests
  NoteOff, // { }
  NoteOn, // { uint8_t note, uint8_t velocity }
  Breath, // { float breath }
  Pitch, // { float pitch }
  BreathAndPitch, // { float breath, float pitch }
  SynthPatchChange, // { uint8_t patchid }
  HarmPatchChange, // { uint8_t patchid }
  // reserve some additional CC here.
  // - expr
  // - modwheel
  // - pedal
  // we could also have a couple special flags to mark that we don't need timing info. for example
  // we sample breath / pitch / nop at known intervals. they will always be the same, so don't bother specifying.
};

struct LoopEventHeader
{
  LoopEventType mEventType;// : LOOPEVENTTYPE_BITS;
  uint16_t mTimeSinceLastEventMS;// : LOOPEVENTTIME_BITS;
};

// nop and note off have no additional params...

struct LoopEvent_NoteOnParams
{
  uint8_t mMidiNote;
  uint8_t mVelocity;
};

struct LoopEvent_BreathParams
{
  float mBreath01;
};

struct LoopEvent_PitchParams
{
  float mPitchN11;
};

struct LoopEvent_BreathAndPitchParams
{
  float mBreath01;
  float mPitchN11;
};

struct LoopEvent_SynthPatchChangeParams
{
  uint8_t mSynthPatchId;
};

struct LoopEvent_HarmPatchChangeParams
{
  uint8_t mHarmPatchId;
};

// struct LoopEvent_Unified
// {
//   LoopEventHeader mHeader;
//   //uint32_t mAbsLoopTimeMS; // convenience for reader
//   union {
//     LoopEvent_NoteOnParams mNoteOnParams;
//     LoopEvent_BreathParams mBreathParams;
//     LoopEvent_PitchParams mPitchParams;
//     LoopEvent_BreathAndPitchParams mBreathAndPitchParams;
//     LoopEvent_SynthPatchChangeParams mSynthPatchChangeParams;
//     LoopEvent_HarmPatchChangeParams mHarmPatchChangeParams;
//   } mParams;
// };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct LoopEventStream
{
  uint8_t* mEventsBegin = nullptr;
  uint8_t* mEventsEnd = nullptr;

  // for writing,  generally points to free area for writing (don't read).
  // for reading, points to the next unread event
  uint8_t* mEventCursor = nullptr;
  // the time relative to the loop start, in milliseconds, of the event pointed to by mEventCursor.
  // NB: it means when the cursor is at the beginning, this is 0.
  // Helps for keeping a running time, because events are relative time.
  uint32_t mEventCursorLoopTimeMS = 0;

  MusicalVoice mRunningVoice;
  CCThrottlerT<LOOP_BREATH_PITCH_RESOLUTION_MS> mBreathPitchThrottler;

  uint32_t PeekNextEventLoopTime() {
    return mEventCursorLoopTimeMS + (((LoopEventHeader*)mEventCursor)->mTimeSinceLastEventMS);
  }

  // read next event in the stream and integrate to outp.
  // advances cursors.
  void ConsumeEvent(MusicalVoice& outp)
  {
    // "Read" header
    LoopEventHeader* phdr = (LoopEventHeader*)mEventCursor;

    // advance our cursor
    mEventCursorLoopTimeMS += phdr->mTimeSinceLastEventMS;
    mEventCursor = (uint8_t*)(phdr + 1);

    // take various actions based on stream, and advance cursor again if needed.
    switch (phdr->mEventType) {
      case LoopEventType::Nop: // {  } supports long time rests
        break;
      case LoopEventType::NoteOff: // { }
      {
        outp.mNeedsNoteOff = true;
        outp.mIsNoteCurrentlyOn = false;
        outp.mNoteOffNote = mRunningVoice.mNoteOffNote;
        break;
      }
      case LoopEventType::NoteOn: // { uint8_t note, uint8_t velocity }
      {
        outp.mNeedsNoteOn = true;
        LoopEvent_NoteOnParams* p = (LoopEvent_NoteOnParams*)mEventCursor;
        mEventCursor = (uint8_t*)(p + 1);
        outp.mMidiNote = p->mMidiNote;
        outp.mVelocity = p->mVelocity;
        break;
      }
      case LoopEventType::Breath: // { float breath }
      {
        LoopEvent_BreathParams* p = (LoopEvent_BreathParams*)mEventCursor;
        mEventCursor = (uint8_t*)(p + 1);
        outp.mBreath01 = p->mBreath01;
        break;
      }
      case LoopEventType::Pitch: // { float pitch }
      {
        LoopEvent_PitchParams* p = (LoopEvent_PitchParams*)mEventCursor;
        mEventCursor = (uint8_t*)(p + 1);
        outp.mPitchBendN11 = p->mPitchN11;
        break;
      }
      case LoopEventType::BreathAndPitch: // { float breath, float pitch }
      {
        LoopEvent_BreathAndPitchParams* p = (LoopEvent_BreathAndPitchParams*)mEventCursor;
        mEventCursor = (uint8_t*)(p + 1);
        outp.mBreath01 = p->mBreath01;
        outp.mPitchBendN11 = p->mPitchN11;
        break;
      }
      case LoopEventType::SynthPatchChange: // { uint8_t patchid }
      {
        LoopEvent_SynthPatchChangeParams* p = (LoopEvent_SynthPatchChangeParams*)mEventCursor;
        mEventCursor = (uint8_t*)(p + 1);
        outp.mSynthPatch = p->mSynthPatchId;
        break;
      }
      case LoopEventType::HarmPatchChange: // { uint8_t patchid }
      {
        LoopEvent_HarmPatchChangeParams* p = (LoopEvent_HarmPatchChangeParams*)mEventCursor;
        mEventCursor = (uint8_t*)(p + 1);
        outp.mHarmPatch = p->mHarmPatchId;
        break;
      }
      default:
        CCASSERT(false);
        break;
    }
  }

  void Read(uint32_t currentLoopTimeMS, MusicalVoice& outp)
  {
    CCASSERT(mEventCursor);
    // if the CURRENT loop time is after what's being requested, it means the loop has.. looped, and we need to read till the end and wrap.
    bool hasLooped = currentLoopTimeMS < mEventCursorLoopTimeMS;
    // initialize outp
    outp.AssignFromLoopStream(mRunningVoice);
    if (hasLooped) {
      // read till end of buffer
      while (mEventCursor < mEventsEnd) {
        ConsumeEvent(outp);
      }
      mEventCursor = mEventsBegin;
      mEventCursorLoopTimeMS = 0;
    }
    // feed outp events until and including currentloop time.
    while(PeekNextEventLoopTime() <= currentLoopTimeMS) {
      ConsumeEvent(outp);
    }
  }

  void ResetBufferForRecording(uint8_t* const begin, uint8_t* const end)
  {
    mEventsBegin = begin;
    mEventCursor = begin;
    mEventsEnd = end;
    mEventCursorLoopTimeMS = 0;
    mRunningVoice.Reset();
  }

  // return the end of the used buffer.
  uint8_t *WrapUpRecording() {
    mEventsEnd = mEventCursor;
    // Q: do we need to adjust anything here like mRunningVoice?
    return mEventsEnd;
  }

  void WriteEvent(uint32_t currentLoopTimeMS, LoopEventType eventType, const void* params = nullptr, size_t paramsSize = 0)
  {
    // do we have enough space for this event?
    uint32_t loopTimeRel = currentLoopTimeMS - mEventCursorLoopTimeMS;
    size_t fullNops = loopTimeRel / LOOPEVENTTIME_MAX;
    uint32_t remainderTime = loopTimeRel - (fullNops * LOOPEVENTTIME_MAX);
    size_t bytesNeeded = sizeof(LoopEventHeader) * (fullNops + 1) + paramsSize; // +1 because THIS event has a header too.

    if (mEventCursor + bytesNeeded >= mEventsEnd) {
      // so this is a tough one. you're recording a loop, and we run out of memory.
      // one could imagine different ways to handle this, but the simplest is to just stop recording.
      return;
    }

    // Write the nops.
    LoopEventHeader* ph = (LoopEventHeader*)mEventCursor;
    for (size_t i = 0; i < fullNops; ++ i){
      ph->mEventType = LoopEventType::Nop;
      ph->mTimeSinceLastEventMS = LOOPEVENTTIME_MAX;
      ++ ph;
    }
    ph->mEventType = eventType;
    ph->mTimeSinceLastEventMS = remainderTime;
    ++ ph;

    uint8_t* pp = (uint8_t*)ph;
    if (paramsSize) {
      memcpy(pp, params, paramsSize);
      pp += paramsSize;
    }

    mEventCursorLoopTimeMS = currentLoopTimeMS;
    mEventCursor = pp;
  }

  template<typename Tparams>
  void WriteEvent(uint32_t currentLoopTimeMS, LoopEventType eventType, const Tparams& eventParams)
  {
    WriteEvent(currentLoopTimeMS, eventType, &eventParams, sizeof(Tparams));
  }

  void WriteNoteOn(uint32_t currentLoopTimeMS, const LoopEvent_NoteOnParams& p) {
    WriteEvent(currentLoopTimeMS, LoopEventType::NoteOn, p);
  }
  void WriteNoteOff(uint32_t currentLoopTimeMS) {
    WriteEvent(currentLoopTimeMS, LoopEventType::NoteOff);
  }
  void WriteBreath(uint32_t currentLoopTimeMS, float p) {
    WriteEvent(currentLoopTimeMS, LoopEventType::Breath, LoopEvent_BreathParams { p });
  }
  void WritePitch(uint32_t currentLoopTimeMS, float p) {
    WriteEvent(currentLoopTimeMS, LoopEventType::Pitch, LoopEvent_PitchParams { p });
  }
  void WriteBreathAndPitch(uint32_t currentLoopTimeMS, float b, float p) {
    WriteEvent(currentLoopTimeMS, LoopEventType::BreathAndPitch, LoopEvent_BreathAndPitchParams { b, p });
  }
  void WriteSynthPatchChange(uint32_t currentLoopTimeMS, uint8_t p) {
    WriteEvent(currentLoopTimeMS, LoopEventType::SynthPatchChange, LoopEvent_SynthPatchChangeParams { p });
  }
  void WriteHarmPatchChange(uint32_t currentLoopTimeMS, uint8_t p) {
    WriteEvent(currentLoopTimeMS, LoopEventType::HarmPatchChange, LoopEvent_HarmPatchChangeParams { p });
  }
  void Write(uint32_t currentLoopTimeMS, const MusicalVoice& liveVoice)
  {
    // convert livevoice to an event and write the event.
    if (liveVoice.mNeedsNoteOff) {
      mRunningVoice.mIsNoteCurrentlyOn = false;
      WriteNoteOff(currentLoopTimeMS);
    }

    // capture alteration events before note on
    if (liveVoice.mSynthPatch != mRunningVoice.mSynthPatch) {
      mRunningVoice.mSynthPatch = liveVoice.mSynthPatch;
      WriteSynthPatchChange(currentLoopTimeMS, mRunningVoice.mSynthPatch);
    }

    if (liveVoice.mHarmPatch != mRunningVoice.mHarmPatch) {
      mRunningVoice.mHarmPatch = liveVoice.mHarmPatch;
      WriteHarmPatchChange(currentLoopTimeMS, mRunningVoice.mHarmPatch);
    }

    if (mBreathPitchThrottler.IsReady()) {
      bool breathChanged = abs(mRunningVoice.mBreath01 - liveVoice.mBreath01) < LOOP_BREATH_PITCH_EPSILON;
      bool pitchChanged = abs(mRunningVoice.mPitchBendN11 - liveVoice.mPitchBendN11) < LOOP_BREATH_PITCH_EPSILON;
      if (breathChanged && pitchChanged) {
        mRunningVoice.mBreath01 = liveVoice.mBreath01;
        mRunningVoice.mPitchBendN11 = liveVoice.mPitchBendN11;
        WriteBreathAndPitch(currentLoopTimeMS, mRunningVoice.mBreath01, mRunningVoice.mPitchBendN11);
      }
      else if (breathChanged) {
        mRunningVoice.mBreath01 = liveVoice.mBreath01;
        WriteBreath(currentLoopTimeMS, mRunningVoice.mBreath01);
      }
      else if (pitchChanged) {
        mRunningVoice.mPitchBendN11 = liveVoice.mPitchBendN11;
        WritePitch(currentLoopTimeMS, mRunningVoice.mPitchBendN11);
      }
    }

    if (liveVoice.mNeedsNoteOn) {
      mRunningVoice.mIsNoteCurrentlyOn = true;
      mRunningVoice.mMidiNote = liveVoice.mMidiNote;
      mRunningVoice.mVelocity = liveVoice.mVelocity;
      WriteNoteOn(currentLoopTimeMS, LoopEvent_NoteOnParams { liveVoice.mMidiNote, liveVoice.mVelocity });
    }
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct LoopLayer
{
  LoopEventStream mStream;
  //int mHarmonizerPreset = 0;
  //bool mHarmonizerEnabled = false;

  bool mIsPlaying = false;// Mute function

  void ResetBufferForRecording(uint8_t* streamBegin, uint8_t* streamEnd)
  {
    mStream.ResetBufferForRecording(streamBegin, streamEnd);
    //mHarmonizerPreset = 0;
    //mHarmonizerEnabled = false;
    mIsPlaying = false;
  }

  uint8_t * WrapUpRecording() { return mStream.WrapUpRecording(); }

  void Write(uint32_t currentLoopTimeMS, const MusicalVoice& liveVoice)
  {
    mStream.Write(currentLoopTimeMS, liveVoice);
  }

  // return false if nothing output (mute)
  bool Read(uint32_t currentLoopTimeMS, MusicalVoice& outp)
  {
    if (!mIsPlaying)
      return false;
    mStream.Read(currentLoopTimeMS, outp);
    return true;
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct LooperAndHarmonizer
{
  enum class LooperState
  {
    Idle,
    StartSet, // when true, loop timer is reset so we are measuring duration.
    DurationSet, // when true, it means at least 1 loop is active (i.e. mCurrentlyLiveLayer > 0), and mLoopDurationMS is valid.
  };

  size_t mCurrentlyWritingLayer = 0;
  uint8_t mBuffer[LOOPER_MEMORY_TOTAL_BYTES];
  LoopLayer mLayers[LOOP_LAYERS];

  LooperState mState = LooperState::Idle;
  Stopwatch mLoopTimer; 
  uint32_t mLoopDurationMS = 0;

  Harmonizer mHarmonizer;

  // UI actions.
  void LoopIt()
  {
    // basically this is a one-button loop function.
    // if the loop beginning is not set, begin measuring loop length.
    // if loop length is not set, set the length and commit to the next layer.
    // if the loop length is set, just commit to next layer.
    switch (mState) {
    case LooperState::Idle:
      // reset the loop start, set loop time now.
      mLoopTimer.Restart();
      mCurrentlyWritingLayer = 0;
      mLayers[mCurrentlyWritingLayer].ResetBufferForRecording(mBuffer, EndPtr(mBuffer));
      mState = LooperState::StartSet;
      break;
    case LooperState::StartSet:
      // set loop duration, set up next loop layer.
      mLoopDurationMS = GetCurrentLoopTimeMS();
      mState = LooperState::DurationSet;
      // FALL-THROUGH
    case LooperState::DurationSet:
      // tell the currently-writing layer it's over.
      uint8_t* buf = mLayers[mCurrentlyWritingLayer].WrapUpRecording();

      mCurrentlyWritingLayer ++; // can go out of bounds!
      if (mCurrentlyWritingLayer < SizeofStaticArray(mLayers)) {
        mLayers[mCurrentlyWritingLayer].ResetBufferForRecording(buf, EndPtr(mBuffer));// prepare the next layer for recording.
      }
      break;
    }
  }

  void Clear() 
  {
    mCurrentlyWritingLayer = 0;
    mState = LooperState::Idle;
    mLoopTimer.Restart();
    for (auto& l : mLayers) {
      l.mIsPlaying = false;
    }
  }

  uint32_t GetCurrentLoopTimeMS() {
    uint32_t ret = mLoopTimer.ElapsedMicros() / 1000;
    if (mState == LooperState::DurationSet) {
      // handle wrapping.
      ret %= mLoopDurationMS;
    }
    return ret;
  }

  void ClearLayer(size_t n) {
    // when you clear a layer, things get ugly. i'm tempted to not even support it.
    // scenario: you have layers 0 1 2 playing, recording into 3.
    // you clear 1.
    // i should scoot memory over so it's 0 2 and now you're recording into 1
    // hm it's not the end of the world really. but you might get confused what you're recording.
    // you kinda want to be able to select the layer you're recording. maybe just handle that at a higher level.
  }

  // here you can record a loop or insert notes.
  // return the # of notes recorded.
  size_t Update(const MusicalVoice& liveVoice, MusicalVoice* outp, MusicalVoice* outpEnd) {
    /*
    1. record state
    2. for each layer, each harmonizer layer, if it can already be filled in, do it.
    3. feed scale follower with the current stuff AND ALL "live" notes.
    4. repeat step 2 for remaining voices.

      live notes are important @ #3, because you can be playing:
      - a comping layer of just Eb, with harm voices of -3 and -5 intervals
      - static bass line of C.
      the bass line is concrete; will automatically be in context.
      the comping layer will be delayed because it's dynamic
      the performer expects that Eb is considered here, and the scale is deduced as C minor.
      without live notes considered, it would probably guess C major becasue it doesn't know
      about Eb.

    - synth engine should not read anything that could be written here
    - synth engine should read all musical state from voices, not from ewimusicalstate.

    */

    // Calculate the loop time for this event. Don't repeat millis() calls.
    uint32_t currentLoopTimeMS = GetCurrentLoopTimeMS();

    // if you have exhausted layers, don't write.
    if (mState != LooperState::Idle && (mCurrentlyWritingLayer < SizeofStaticArray(mLayers))) {
      mLayers[mCurrentlyWritingLayer].Write(currentLoopTimeMS, liveVoice);
    }

    // in order to feed the scale follower with a single buffer for both "live" (may not be played!) and harmonized voices,
    // go through all looper layers & harmonizer voices, and if we know the note then put it. if there are 0 notes output,
    // then just put the "live" note but mark it as muted.

    MusicalVoice* pout = outp;
    MusicalVoice* pLiveVoices [LOOP_LAYERS]; // keep track of where i read layer state into, for later when deduced voices are filled in.

    // output the live actually-playing voice.
    pout->AssignFromLoopStream(liveVoice);
    pout += mHarmonizer.Harmonize(mCurrentlyWritingLayer, pout, pout + 1, outpEnd, Harmonizer::VoiceFilterOptions::ExcludeDeducedVoices);

    // do the same for other layers; they're read from stream.
    for (size_t iLayer = 0; iLayer < SizeofStaticArray(mLayers); ++ iLayer) {
      if (iLayer == mCurrentlyWritingLayer)
        continue;
      auto& l = mLayers[iLayer];
      if (l.Read(currentLoopTimeMS, *pout)) {
        pLiveVoices[iLayer] = pout;
        pout += mHarmonizer.Harmonize(iLayer, pout, pout + 1, outpEnd, Harmonizer::VoiceFilterOptions::ExcludeDeducedVoices);
      }
    }

    gScaleFollower.Update(outp, pout - outp);

    // go through and fill in all deduced voices. logically, the "live voices" will all remain untouched so voices just get filled in.
    for (size_t iLayer = 0; iLayer < SizeofStaticArray(mLayers); ++ iLayer) {
      auto& l = mLayers[iLayer];
      if (l.mIsPlaying) {
        pout += mHarmonizer.Harmonize(iLayer, pLiveVoices[iLayer], pout, outpEnd, Harmonizer::VoiceFilterOptions::OnlyDeducedVoices);
      }
    }

    return pout - outp;
  }
};

