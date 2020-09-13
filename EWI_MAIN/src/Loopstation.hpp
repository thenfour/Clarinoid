// todo: resolve ambiguity with:
// eventsbegin time?? why is this needed?
// eventscursor is just cursor.

#pragma once

#ifndef EWI_UNIT_TESTS
#include <Shared_CCUtil.h>
#include "ScaleFollower.hpp"
#else
#include <vector>
static const size_t LOOP_LAYERS = 3;
#endif // EWI_UNIT_TESTS
#include "Harmonizer.hpp"

static const size_t LOOPER_MEMORY_TOTAL_BYTES = 10000;
static const size_t LOOPEVENTTYPE_BITS = 4;
static const size_t LOOPEVENTTIME_BITS = 12; // 12 bits of milliseconds is 4 seconds; pretty reasonable
static const uint32_t LOOPEVENTTIME_MAX = (1 << LOOPEVENTTIME_BITS) - 1;
static const uint32_t LOOP_BREATH_PITCH_RESOLUTION_MS = 5; // record only every N milliseconds max. This should probably be coordinated with the similar throttler in MusicalState, to make sure it plays well together
static const float LOOP_BREATH_PITCH_EPSILON = 1.0f / 1024; // resolution to track.
static const uint32_t LOOP_MIN_DURATION = 100;

static void OrderedMemcpy(uint8_t* dest, uint8_t *src, size_t bytes)
{
  for (size_t i = 0; i < bytes; ++i) {
    dest[i] = src[i];
  }
}

static void SwapMem(uint8_t* begin, uint8_t* end, uint8_t* dest)
{
  uint8_t temp;
  size_t bytes = end - begin;
  for (size_t i = 0; i < bytes; ++i) {
    temp = begin[i];
    begin[i] = dest[i];
    dest[i] = temp;
  }
}

// shift a split circular buffer into place without using some temp buffer. avoids OOM.
// returns the total size of the resulting buffer which will start at segmentBBegin.
// |Bbbb----Aaaa|  => |AaaaBbbb----|
static size_t UnifyCircularBuffer(uint8_t* segmentABegin, uint8_t* segmentAEnd, uint8_t* segmentBBegin, uint8_t* segmentBEnd)
{
  CCASSERT(segmentBEnd > segmentBBegin);
  CCASSERT(segmentABegin > segmentBEnd);
  CCASSERT(segmentAEnd > segmentABegin);

  // if A is bigger than B.
  size_t sizeA = segmentAEnd - segmentABegin;
  size_t sizeB = segmentBEnd - segmentBBegin;

  if (sizeA > sizeB) {
    // 1. swap
    // |Bb----Aaaa|  => |Aaaa--Bb--|
    SwapMem(segmentABegin, segmentAEnd, segmentBBegin);
    // 2. slide B into position.
    // |Aaaa--Bb--|  => |AaaaBb----|
    OrderedMemcpy(segmentBBegin + sizeA, segmentABegin + sizeB, sizeB);
  }
  else {
    // 1. swap
    // |Bbbb----Aa|  => |Aabb----Bb|
    SwapMem(segmentABegin, segmentAEnd, segmentBBegin);
    // 2. swap the end and begin segments of B
    // |Aabb----Bb|  => |AaBb----bb|
    SwapMem(segmentBBegin + sizeA, segmentBEnd, segmentABegin);
    // 3. slide the second part of B into place.
    // |AaBb----bb|  => |AaBbbb----|
    OrderedMemcpy(segmentBBegin + sizeA, segmentABegin, sizeB - sizeA);
  }
  return sizeA + sizeB;
}


enum class LoopEventType
{
  Nop = 0, // {  } supports long time rests
  NoteOff = 1, // { }
  NoteOn = 2, // { uint8_t note, uint8_t velocity }
  Breath = 3, // { float breath }
  Pitch = 4, // { float pitch }
  BreathAndPitch = 5, // { float breath, float pitch }
  SynthPatchChange = 6, // { uint8_t patchid }
  HarmPatchChange = 7, // { uint8_t patchid }
  LOOP_EVENT_TYPE_COUNT_ = 8
  // reserve some additional CC here.
  // - expr
  // - modwheel
  // - pedal
  // we could also have a couple special flags to mark that we don't need timing info. for example
  // we sample breath / pitch / nop at known intervals. they will always be the same, so don't bother specifying.
};
static const size_t LOOP_EVENT_TYPE_COUNT = (size_t)LoopEventType::LOOP_EVENT_TYPE_COUNT_;

#ifdef EWI_UNIT_TESTS
static const uint8_t LOOP_EVENT_MARKER = 0x6D;
#endif // EWI_UNIT_TESTS

struct LoopEventHeader
{
#ifdef EWI_UNIT_TESTS
  uint8_t mMarker = LOOP_EVENT_MARKER;
#endif // EWI_UNIT_TESTS
  LoopEventType mEventType;// : LOOPEVENTTYPE_BITS;
  uint16_t mTimeSinceLastEventMS;// : LOOPEVENTTIME_BITS;
};

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
  int16_t mSynthPatchId;
};

struct LoopEvent_HarmPatchChangeParams
{
  int16_t mHarmPatchId;
};

// NOT stream-friendly layout.
struct LoopEvent_Unified
{
  LoopEventHeader mHeader;
  uint32_t mLoopTimeMS; // debugging convenience
  uint8_t *mP; // debugging convenience
  union {
    LoopEvent_NoteOnParams mNoteOnParams;
    LoopEvent_BreathParams mBreathParams;
    LoopEvent_PitchParams mPitchParams;
    LoopEvent_BreathAndPitchParams mBreathAndPitchParams;
    LoopEvent_SynthPatchChangeParams mSynthPatchChangeParams;
    LoopEvent_HarmPatchChangeParams mHarmPatchChangeParams;
  } mParams;
};

struct LoopEventTypeItemInfo
{
  LoopEventType mValue;
  const char *mName;
  size_t mParamsSize;
  String(*mParamsToString)(const LoopEvent_Unified&);
};

String LoopBlankParamToString(const LoopEvent_Unified&) { return String(); }
String LoopNoteOnParamToString(const LoopEvent_Unified& e) { return String(e.mParams.mNoteOnParams.mMidiNote); }
String LoopBreathParamToString(const LoopEvent_Unified& e) { return String(e.mParams.mBreathParams.mBreath01); }
String LoopPitchParamToString(const LoopEvent_Unified& e) { return String(e.mParams.mPitchParams.mPitchN11); }
String LoopBreathAndPitchParamToString(const LoopEvent_Unified& e) { return String(e.mParams.mBreathAndPitchParams.mBreath01); }
String LoopSynthPatchChangeParamToString(const LoopEvent_Unified& e) { return String(e.mParams.mSynthPatchChangeParams.mSynthPatchId); }
String LoopHarmPatchChangeParamToString(const LoopEvent_Unified& e) { return String(e.mParams.mHarmPatchChangeParams.mHarmPatchId); }

const LoopEventTypeItemInfo gLoopEventTypeItemInfo_[LOOP_EVENT_TYPE_COUNT] =
{
  { LoopEventType::Nop, "Nop", 0, LoopBlankParamToString },
  { LoopEventType::NoteOff, "NoteOff", 0, LoopBlankParamToString },
  { LoopEventType::NoteOn, "NoteOn", sizeof(LoopEvent_NoteOnParams), LoopNoteOnParamToString },
  { LoopEventType::Breath, "Breath", sizeof(LoopEvent_BreathParams), LoopBreathParamToString },
  { LoopEventType::Pitch, "Pitch", sizeof(LoopEvent_PitchParams), LoopPitchParamToString },
  { LoopEventType::BreathAndPitch, "BreathAndPitch", sizeof(LoopEvent_BreathAndPitchParams), LoopBreathAndPitchParamToString },
  { LoopEventType::SynthPatchChange, "SynthPatchChange", sizeof(LoopEvent_SynthPatchChangeParams), LoopSynthPatchChangeParamToString },
  { LoopEventType::HarmPatchChange, "HarmPatchChange", sizeof(LoopEvent_HarmPatchChangeParams), LoopHarmPatchChangeParamToString },
};

const LoopEventTypeItemInfo& GetLoopEventTypeItemInfo(size_t i) {
  CCASSERT(i >= 0 && i < LOOP_EVENT_TYPE_COUNT);
  return gLoopEventTypeItemInfo_[i];
}

const LoopEventTypeItemInfo& GetLoopEventTypeItemInfo(LoopEventType t) {
  return GetLoopEventTypeItemInfo((size_t)t);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class LooperState
{
  Idle,
  StartSet, // when true, loop timer is reset so we are measuring duration.
  DurationSet, // when true, it means at least 1 loop is active (i.e. mCurrentlyLiveLayer > 0), and mLoopDurationMS is valid.
};

struct LoopStatus
{
  LooperState mState = LooperState::Idle;
  uint32_t mCurrentLoopTimeMS = 0;
  uint32_t mLoopDurationMS = 0;
};

struct LoopCursor
{
  uint8_t *mP = nullptr;
  // the time AT the cursor. the cursor points to an event which has a delay
  // while PeekLoopTime() tells you when the current event would happen
  // careful to use these correctly.
  uint32_t mLoopTimeMS = 0;

  void Set(uint8_t *p, uint32_t loopTime) {
    mP = p;
    mLoopTimeMS = loopTime;
  }

  void Reset() {
    Set(nullptr, 0);
  }

  // returns the absolute loop time that the currently-pointed-to-event should occur.
  // NOTE: can be past the loop duration!
  uint32_t PeekLoopTime()
  {
    LoopEventHeader *phdr = (LoopEventHeader*)mP;
#ifdef EWI_UNIT_TESTS
    CCASSERT(phdr->mMarker == LOOP_EVENT_MARKER);
#endif // EWI_UNIT_TESTS
    return mLoopTimeMS + phdr->mTimeSinceLastEventMS;
  }

  // you can have an event at the end of the buffer which has a delay which would loop back the loop time.
  // this will account for this and return a value within the loop.
  uint32_t PeekLoopTimeWrapSensitive(const LoopStatus& status)
  {
    CCASSERT(status.mState == LooperState::DurationSet);
    return PeekLoopTime() % status.mLoopDurationMS;
  }

  LoopEvent_Unified PeekEvent() {
    LoopEventHeader *phdr = (LoopEventHeader*)mP;
#ifdef EWI_UNIT_TESTS
    CCASSERT(phdr->mMarker == LOOP_EVENT_MARKER);
#endif // EWI_UNIT_TESTS
    LoopEvent_Unified ret;
    ret.mHeader = *phdr;
    ret.mP = (uint8_t*)phdr;
    ret.mLoopTimeMS = mLoopTimeMS;
    memcpy(&(ret.mParams), phdr + 1, GetLoopEventTypeItemInfo(phdr->mEventType).mParamsSize);
    return ret;
  }

  void MoveNextNaive(const LoopStatus& status)
  {
    LoopEventHeader *phdr = (LoopEventHeader*)mP;
    mP = (uint8_t*)(phdr + 1);
    mLoopTimeMS += phdr->mTimeSinceLastEventMS;
    mP += GetLoopEventTypeItemInfo(phdr->mEventType).mParamsSize;
    if (status.mState == LooperState::DurationSet) {
      if (mLoopTimeMS >= status.mLoopDurationMS)
        mLoopTimeMS -= status.mLoopDurationMS;
    }
  }

  // return true if we wrapped in the buffer (not time though!).
  bool MoveNext(const LoopStatus& status, const LoopCursor& bufferBegin, uint8_t* bufferEnd)
  {
    MoveNextNaive(status);
    if (mP >= bufferEnd) {
      *this = bufferBegin;
      return true;
    }
    return false;
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct LoopEventStream
{
  bool mIsPlaying = false;// Mute function
  bool mOOM = false; // Out of memory condition. Set this flag to stop recording.
  bool mIsRecording = false; // necessary for checking cursor conditions & states.

  LoopCursor mEventsBegin;
  uint8_t* mEventsEnd = nullptr;

  uint8_t* mEventsValidEnd = nullptr;// when writing, this can be before the end of the raw buffer.

  // for writing,  generally points to free area for writing (don't read).
  // for reading, points to the next unread event
  LoopCursor mEventCursor;

  // if you're writing into the stream, and you pass the loop length, then we start tracking a complete loop worth of events.
  // this way when we stop recording, we can accurately piece together a complete loop of events in sequence.
  LoopCursor mWritePrevCursor;

  // also track the last loop crossing in the buffer. helps us to know easily how to transition from writing to reading.
  uint8_t *mWriteZeroTimeCursor = nullptr;

  //LoopStreamStatistics mStats;

  MusicalVoice mRunningVoice;
#ifndef EWI_UNIT_TESTS
  CCThrottlerT<LOOP_BREATH_PITCH_RESOLUTION_MS> mBreathPitchThrottler;
#endif // EWI_UNIT_TESTS

  const LoopStatus* mpStatus = nullptr;
  const LoopStatus& GetStatus() const { return *mpStatus; }

  // read next event in the stream and integrate to outp.
  // advances cursors.
  // returns true if the cursor wrapped.
  bool ConsumeSingleEvent(MusicalVoice& outp)
  {
    LoopEvent_Unified event = mEventCursor.PeekEvent();

    // take various actions based on stream, and advance cursor again if needed.
    switch (event.mHeader.mEventType) {
    case LoopEventType::Nop: // {  } supports long time rests
      break;
    case LoopEventType::NoteOff: // { }
      outp.mNeedsNoteOff = true;
      outp.mIsNoteCurrentlyOn = false;
      outp.mNoteOffNote = mRunningVoice.mNoteOffNote;
      break;
    case LoopEventType::NoteOn: // { uint8_t note, uint8_t velocity }
      outp.mNeedsNoteOn = true;
      outp.mMidiNote = event.mParams.mNoteOnParams.mMidiNote;
      outp.mVelocity = event.mParams.mNoteOnParams.mVelocity;
      break;
    case LoopEventType::Breath: // { float breath }
      outp.mBreath01 = event.mParams.mBreathParams.mBreath01;
      break;
    case LoopEventType::Pitch: // { float pitch }
      outp.mPitchBendN11 = event.mParams.mPitchParams.mPitchN11;
      break;
    case LoopEventType::BreathAndPitch: // { float breath, float pitch }
      outp.mBreath01 = event.mParams.mBreathAndPitchParams.mBreath01;
      outp.mPitchBendN11 = event.mParams.mBreathAndPitchParams.mPitchN11;
      break;
    case LoopEventType::SynthPatchChange: // { uint8_t patchid }
      outp.mSynthPatch = event.mParams.mSynthPatchChangeParams.mSynthPatchId;
      break;
    case LoopEventType::HarmPatchChange: // { uint8_t patchid }
      outp.mHarmPatch = event.mParams.mHarmPatchChangeParams.mHarmPatchId;
      break;
    default:
      CCASSERT(false);
      break;
    }
    return mEventCursor.MoveNext(GetStatus(), mEventsBegin, mEventsValidEnd);
  }

  // returns # of events read.
  size_t ReadUntilLoopTime(MusicalVoice& outp)
  {
    if (!mIsPlaying)
      return 0;
    CCASSERT(!!mEventCursor.mP);
    if (mEventsBegin.mP == mEventsValidEnd)
      return 0;

    size_t ret = 0;

    bool hasLooped = GetStatus().mCurrentLoopTimeMS < mEventCursor.mLoopTimeMS;
    if (hasLooped) {
      // if the loop has wrapped, we need to first read until WE loop, to ensure our time is <= requested time.
      while (!ConsumeSingleEvent(outp))
        ++ret;
    }

    while (mEventCursor.PeekLoopTimeWrapSensitive(GetStatus()) <= GetStatus().mCurrentLoopTimeMS) {
      ++ret;
      if (ConsumeSingleEvent(outp)) {
        // if we wrapped, break out. basically we just don't have events this far into the loop.
        break;
      }
    }
    return ret;
  }

  void ResetBufferForRecording(const LoopStatus& status, uint8_t* const begin, uint8_t* const end)
  {
    mIsPlaying = false;
    mIsRecording = true;
    mpStatus = &status;
    //mStats.Reset();
    mEventsBegin.Set(begin, status.mCurrentLoopTimeMS);
    mEventsValidEnd = begin;
    mWriteZeroTimeCursor = begin;
    mEventsEnd = end;
    mEventCursor = mEventsBegin;
    mWritePrevCursor.Reset();
    mRunningVoice.Reset();
  }

  // |ZxxxxE-------- (ze)
  // |---PxxxxZxxxxE (pze)
  // |xxE---PxxxxZxx (epz)
  // |xxZxxxxE---Pxx (zep)
  enum class LayoutSituation
  {
    ZE,
    PZE,
    EPZ,
    ZEP
  };

  LayoutSituation GetLayoutSituation() const
  {
    if (mWritePrevCursor.mP == nullptr) {
      CCASSERT(mWriteZeroTimeCursor == mEventsBegin.mP);
      CCASSERT(mWriteZeroTimeCursor <= mEventCursor.mP);
      return LayoutSituation::ZE;
    }
    if (mWritePrevCursor.mP <= mEventCursor.mP) {
      CCASSERT(mWritePrevCursor.mP < mEventCursor.mP);
      CCASSERT(mWritePrevCursor.mP <= mWriteZeroTimeCursor);
      return LayoutSituation::PZE;
    }
    if (mEventCursor.mP <= mWriteZeroTimeCursor) {
      CCASSERT(mEventCursor.mP < mWritePrevCursor.mP);
      CCASSERT(mWritePrevCursor.mP <= mWriteZeroTimeCursor);
      return LayoutSituation::EPZ;
    }
    CCASSERT(mWriteZeroTimeCursor < mEventCursor.mP);
    CCASSERT(mEventCursor.mP < mWritePrevCursor.mP);
    return LayoutSituation::ZEP;
  }

#ifdef EWI_UNIT_TESTS

  void Dump()
  {
    cc::log("----");
    auto events = DebugGetStream();
    auto layout = mIsRecording ? GetLayoutSituation() : LayoutSituation::ZE;

    if (mIsRecording) {
      switch (layout) {
      case LayoutSituation::ZE:// |ZxxxxE-------- (ze)
        cc::log("|ZxxxxE-------- (ZE)");
        break;
      case LayoutSituation::PZE:// |---PxxxxZxxxxE (pze)
        cc::log("|---PxxxxZxxxxE (PZE)");
        break;
      case LayoutSituation::EPZ:// |xxE---PxxxxZxx (epz)
        cc::log("|xxE---PxxxxZxx (EPZ)");
        break;
      case LayoutSituation::ZEP:// |xxZxxxxE---Pxx (zep)
        cc::log("|xxZxxxxE---Pxx (ZEP)");
        break;
      }

      if (layout == LayoutSituation::PZE) {
        cc::log("    -- [ %p pze padding %d bytes ] --", mEventsBegin.mP, mWritePrevCursor.mP - mEventsBegin.mP);
      }
    }
    else {
      cc::log("not recording; no layout is really relevant.");
    }

    bool encounteredPadding = false;
    for (auto& e : events) {
      int loopTime = e.mLoopTimeMS;

      if (mIsRecording && !encounteredPadding && (e.mP >= mEventCursor.mP)) {
        switch (layout) {
        case LayoutSituation::EPZ:
          encounteredPadding = true;
          cc::log(" E  -- [ %p EPZ padding %d bytes ] --", mEventCursor.mP, mWritePrevCursor.mP - mEventCursor.mP);
          break;
        case LayoutSituation::ZEP:
          encounteredPadding = true;
          cc::log(" E  -- [ %p ZEP padding %d bytes ] --", mEventCursor.mP, mWritePrevCursor.mP - mEventCursor.mP);
          break;
        }
      }

      CCASSERT(e.mHeader.mMarker == LOOP_EVENT_MARKER);
      String sParams = GetLoopEventTypeItemInfo(e.mHeader.mEventType).mParamsToString(e);
      cc::log("%s%s%s [%p (+%d) t=%d: dly=%d, type=%s, params=%s]",
        e.mP == mWriteZeroTimeCursor ? "Z" : " ",
        e.mP == mEventCursor.mP ? "E" : " ",
        e.mP == mWritePrevCursor.mP ? "P" : " ",
        e.mP,
        e.mP - mEventsBegin.mP,
        loopTime,
        (int)e.mHeader.mTimeSinceLastEventMS,
        GetLoopEventTypeItemInfo(e.mHeader.mEventType).mName,
        sParams.mStr.str().c_str());
    } // for()

    if (mEventCursor.mP == mEventsValidEnd) {
      cc::log(" E  [%p (+%d) <eof> ]",
        mEventCursor.mP,
        mEventCursor.mP - mEventsBegin.mP
        );
    }
    cc::log("    buf used: %d, event count: %d %s, validEnd:%p, bufend:%p",
      mEventsValidEnd - mEventsBegin.mP, events.size(),
      mOOM ? "OOM!" : "",
      mEventsValidEnd,
      mEventsEnd
      );
  }

  // even here we must be sensitive to scenarios.
  std::vector<LoopEvent_Unified> DebugGetStream() const
  {
    std::vector<LoopEvent_Unified> ret;
    if (!mIsRecording) {
      // assume sequential layout
      LoopCursor c = mEventsBegin;
      while (c.mP < mEventsValidEnd) {
        ret.push_back(c.PeekEvent());
        c.MoveNextNaive(GetStatus());
      };
      return ret;
    }
    switch (GetLayoutSituation()) {
    case LayoutSituation::ZE:// |ZxxxxE-------- (ze)
    {
      LoopCursor c = mEventsBegin;
      while (c.mP < mEventsValidEnd) {
        ret.push_back(c.PeekEvent());
        c.MoveNextNaive(GetStatus());
      };
      return ret;
    }
    case LayoutSituation::PZE:// |---PxxxxZxxxxE (pze)
    {
      LoopCursor c = mWritePrevCursor;
      while (c.mP < mEventsValidEnd) {
        ret.push_back(c.PeekEvent());
        c.MoveNextNaive(GetStatus());
      };
      return ret;
    }
    case LayoutSituation::EPZ:// |xxE---PxxxxZxx (epz)
    case LayoutSituation::ZEP:// |xxZxxxxE---Pxx (zep)
    {
      LoopCursor c = mEventsBegin;
      while (c.mP < mEventCursor.mP) {
        ret.push_back(c.PeekEvent());
        c.MoveNextNaive(GetStatus());
      };
      c = mWritePrevCursor;
      while (c.mP < mEventsValidEnd) {
        ret.push_back(c.PeekEvent());
        c.MoveNextNaive(GetStatus());
      };
      return ret;
    }
    }

    return ret;
  }
#endif // EWI_UNIT_TESTS

  // specialized for writing at the end of the buffer, just before the buffer
  // changes from writing to reading.
  // will not wrap the buffer. returns new end ptr.
  // on OOM just does nothing (loop will be slightly too short)
  uint8_t* WriteSeamNops(uint8_t* p, uint32_t duration) {
    if (mOOM)
      return p;
    size_t fullNops = duration / LOOPEVENTTIME_MAX;
    uint32_t remainderTime = duration - (fullNops * LOOPEVENTTIME_MAX);
    size_t bytesNeeded = fullNops + (remainderTime ? 1 : 0);
    if (p + bytesNeeded >= mEventsEnd) {
      return p; // OOM
    }
    LoopEventHeader* ph = (LoopEventHeader*)p;
    for (size_t i = 0; i < fullNops; ++i) {
      ph->mMarker = LOOP_EVENT_MARKER;
      ph->mEventType = LoopEventType::Nop;
      ph->mTimeSinceLastEventMS = LOOPEVENTTIME_MAX;
      ++ph;
    }
    if (remainderTime) {
      ph->mMarker = LOOP_EVENT_MARKER;
      ph->mEventType = LoopEventType::Nop;
      ph->mTimeSinceLastEventMS = remainderTime;
      ++ph;
    }

    CCASSERT(GetStatus().mState == LooperState::DurationSet);
    return (uint8_t*)ph;
  }

  // seek so we're ready to read at loop time.
  void SeekToLoopTime()
  {
    CCASSERT(GetStatus().mState == LooperState::DurationSet);
    mEventCursor.mP = mWriteZeroTimeCursor;
    mEventCursor.mLoopTimeMS = 0;
    LoopCursor c = mEventCursor;
    while (c.PeekLoopTime() < GetStatus().mCurrentLoopTimeMS) {
      if (mEventCursor.MoveNext(GetStatus(), mEventsBegin, mEventsValidEnd)) {
        CCASSERT(false); // should not be possible to 
      }
    }
  }

  // return the end of the used buffer.
  // point cursor at the time in status.
  uint8_t* WrapUpRecording() {
    // |ZxxxxE-------- (ze)
    // |---PxxxxZxxxxE (pze)
    // |xxE---PxxxxZxx (epz)
    // |xxZxxxxE---Pxx (zep)

    switch (GetLayoutSituation()) {// |ZxxxxE-------- (ze)
      case LayoutSituation::ZE:
        // so you haven't yet seen a loop crossing; no blitting necessary.
        mEventsValidEnd = mEventCursor.mP;
        mEventsEnd = mEventsValidEnd;
        mEventCursor = mEventsBegin;
        mIsPlaying = true;
        mIsRecording = false;
        return mEventsValidEnd;

      case LayoutSituation::PZE:// |---PxxxxZxxxxE (pze)
      {
        uint32_t pzMS = GetStatus().mLoopDurationMS - mWritePrevCursor.mLoopTimeMS;
        uint32_t zeMS = mEventCursor.mLoopTimeMS;
        uint32_t recordedDuration = pzMS + zeMS;
        CCASSERT(recordedDuration <= GetStatus().mLoopDurationMS);

        size_t pzBytes = mWriteZeroTimeCursor - mWritePrevCursor.mP;
        size_t zeBytes = mEventCursor.mP - mWriteZeroTimeCursor;
        size_t peBytes = pzBytes + zeBytes;

        OrderedMemcpy(mEventsBegin.mP, mWritePrevCursor.mP, peBytes);

        mEventsValidEnd = mEventsBegin.mP + peBytes;
        mEventsEnd = mEventsValidEnd;
        mEventsBegin.mLoopTimeMS = mWritePrevCursor.mLoopTimeMS;
        mWriteZeroTimeCursor = mEventsBegin.mP + pzBytes;
        mWritePrevCursor.Reset(); // no longer valid.

        // write a "seam" at the end of the buffer.
        mEventsValidEnd = WriteSeamNops(mEventsValidEnd, GetStatus().mLoopDurationMS - recordedDuration);
        SeekToLoopTime();
        mIsPlaying = true;
        mIsRecording = false;
        return mEventsValidEnd;
      }

      case LayoutSituation::EPZ:// |xxE---PxxxxZxx (epz)
      {
        // Scenario EPZ
        uint32_t pzMS = GetStatus().mLoopDurationMS - mWritePrevCursor.mLoopTimeMS;
        uint32_t zeMS = mEventCursor.mLoopTimeMS;
        uint32_t recordedDuration = pzMS + zeMS;
        CCASSERT(recordedDuration <= GetStatus().mLoopDurationMS);

        uint8_t *segmentABegin = mWritePrevCursor.mP;
        uint8_t *segmentAEnd = mEventsValidEnd;
        uint8_t *segmentBBegin = mEventsBegin.mP;
        uint8_t *segmentBEnd = mEventCursor.mP;
        size_t buflen = UnifyCircularBuffer(segmentABegin, segmentAEnd, segmentBBegin, segmentBEnd);
        // now the buffer is
        // |PxxxxZxxxxE---

        mEventsValidEnd = mEventsBegin.mP + buflen;
        mEventsEnd = mEventsValidEnd;
        mEventsBegin.mLoopTimeMS = mWritePrevCursor.mLoopTimeMS;
        size_t pzBytes = mWriteZeroTimeCursor - mWritePrevCursor.mP;
        mWriteZeroTimeCursor = mEventsBegin.mP + pzBytes;
        mWritePrevCursor.Reset(); // no longer valid.

        // write a "seam" at the end of the buffer.
        mEventsValidEnd = WriteSeamNops(mEventsValidEnd, GetStatus().mLoopDurationMS - recordedDuration);
        SeekToLoopTime();
        mIsPlaying = true;
        mIsRecording = false;
        return mEventsValidEnd;
      }

      case LayoutSituation::ZEP:// |xxZxxxxE---Pxx (zep)
      {
        break;
      }
    }
    return 0;
  }

  uint32_t CalcDuration(const LoopStatus& status, uint32_t loopTimeBegin, uint32_t loopTimeEnd) {
    CCASSERT(status.mState == LooperState::DurationSet);
    if (loopTimeEnd < loopTimeBegin) {
      loopTimeEnd += status.mLoopDurationMS;
    }
    return loopTimeEnd - loopTimeBegin;
  }

  // return true on success.
  bool FindMemoryOrOOM(size_t bytesNeeded)
  {
    // ZE, PZE check for end of buffer
    // EPZ and ZEP check f or P
    switch (GetLayoutSituation()) {
    case LayoutSituation::ZE:// |ZxxxxE-------- (ze)
      if (mEventCursor.mP + bytesNeeded < mEventsEnd) {
        return true;// we're already set.
      }
      if (GetStatus().mState == LooperState::StartSet) {
        // - No duration set (1st loop layer), but you're recording. so you're just trying to make a loop layer too big.
        mOOM = true;
        return false;
      }
      if (GetStatus().mState == LooperState::DurationSet && mWritePrevCursor.mP == nullptr) {
        // - Duration set, but we haven't recorded a full loop. So you're recording a 2nd layer that just doesn't have enough mem left to complete.
        mOOM = true;
        return false;
      }
      CCASSERT(false); // not possible to land here
      return false;
    case LayoutSituation::PZE:// |---PxxxxZxxxxE (pze)
      if (mEventCursor.mP + bytesNeeded < mEventsEnd) {
        return true;// we're already set.
      }
      // uh oh. wrap buffer?
      if (mEventsBegin.mP + bytesNeeded >= mWritePrevCursor.mP) {
        // too small; we would eat into our own tail. OOM.
        mOOM = true;
        return false;
      }
      // safe to use beginning of buffer, transition to EPZ
      //mEventsValidEnd = mEventCursor.mP;
      mEventCursor.mP = mEventsBegin.mP;
      mEventsBegin.mLoopTimeMS = mEventCursor.mLoopTimeMS;
      cc::log("Wrapping buffer, entering EPZ layout.");
      Dump();
      cc::log("}]]]]]]");
      return true;
    case LayoutSituation::EPZ:// |xxE---PxxxxZxx (epz)
    case LayoutSituation::ZEP:// |xxZxxxxE---Pxx (zep)
      if (mEventCursor.mP + bytesNeeded < mWritePrevCursor.mP) {
        return true;// we're already set.
      }
      // too small; we would eat into our own tail. OOM.
      mOOM = true;
      return false;
    }
    CCASSERT(false);
    return false;
  }

  // measures from P to E. optimizing this would be possible but complex because
  // of all the changing cursors and intermediate states.
  uint32_t MeasureRecordedDurationPE()
  {
    Dump();
    LoopCursor p = mWritePrevCursor;
    uint32_t ret = 0;
    while (p.mP != mEventCursor.mP) {
      ret += p.PeekEvent().mHeader.mTimeSinceLastEventMS;
      if (p.MoveNext(GetStatus(), mEventsBegin, mEventsValidEnd)) {// <-- this would wrap when E is the end of the buffer. so don't use it; need to detect loop condition.
        // we wrapped. this is a case where we could miss E.
        if (mEventCursor.mP == mEventsValidEnd)
        {
          break;
        }
      }
    }
    return ret;
  }

  bool WriteEventRaw(LoopEventType eventType, const void* params)
  {
    cc::log("WRITE EVENT");
    if (mOOM)
      return false;
    const auto& eventInfo = GetLoopEventTypeItemInfo(eventType);
    uint32_t currentLoopTimeMSExtra = GetStatus().mCurrentLoopTimeMS; // loop time but guaranteed to be longer than our time so relative time is positive.
    uint32_t endOfLoopFillerTime = 0;

    if (GetStatus().mState == LooperState::DurationSet) {
      CCASSERT(GetStatus().mCurrentLoopTimeMS < GetStatus().mLoopDurationMS);
    }

    bool hasLooped = GetStatus().mCurrentLoopTimeMS < mEventCursor.mLoopTimeMS;
    if (GetStatus().mState == LooperState::DurationSet && hasLooped) {
      currentLoopTimeMSExtra += GetStatus().mLoopDurationMS;
      if (mWritePrevCursor.mP == nullptr) {
        // because we've looped back, and we haven't yet tracked a full loop of material, it means this is the point to prepare the "prev write" cursor for tracking.
        // one loop ago was exactly at the beginning of the buffer.
        mWritePrevCursor = mEventsBegin;
        CCASSERT(mEventsBegin.mLoopTimeMS == 0);
      }
      // in order to keep perfect loop boundaries, NOPs are written to complete the loop timing.
      endOfLoopFillerTime = GetStatus().mLoopDurationMS - mEventCursor.mLoopTimeMS;
    }

    // relative time AFTER end-of-loop filler.
    uint32_t loopTimeRel = currentLoopTimeMSExtra - mEventCursor.mLoopTimeMS - endOfLoopFillerTime;
    size_t fullNopsEOL = endOfLoopFillerTime / LOOPEVENTTIME_MAX;
    uint32_t EOLremainderNopTime = endOfLoopFillerTime - (fullNopsEOL * LOOPEVENTTIME_MAX);// how much time is needed in a partial NOP to fill out the loop
    size_t fullNops = loopTimeRel / LOOPEVENTTIME_MAX;
    uint32_t remainderTime = loopTimeRel - (fullNops * LOOPEVENTTIME_MAX);
    size_t bytesNeeded = sizeof(LoopEventHeader) * (fullNops + (EOLremainderNopTime ? 1 : 0) + fullNopsEOL + 1) + eventInfo.mParamsSize; // +1 because THIS event has a header too.

    // CHECK for OOM, deal with wrapping buffer.
    if (!FindMemoryOrOOM(bytesNeeded)) {
      return false;
    }

    // Write nops.
    LoopEventHeader* ph = (LoopEventHeader*)(mEventCursor.mP);

#ifdef EWI_UNIT_TESTS
#define WRITE_LOOP_EVENT_HEADER_MARKER ph->mMarker = LOOP_EVENT_MARKER
#else
#define WRITE_LOOP_EVENT_HEADER_MARKER
#endif // EWI_UNIT_TESTS

    // fill out remainder of loop
    for (size_t i = 0; i < fullNopsEOL; ++i) {
      WRITE_LOOP_EVENT_HEADER_MARKER;
      ph->mEventType = LoopEventType::Nop;
      ph->mTimeSinceLastEventMS = LOOPEVENTTIME_MAX;
      ++ph;
    }
    if (EOLremainderNopTime) {
      WRITE_LOOP_EVENT_HEADER_MARKER;
      ph->mEventType = LoopEventType::Nop;
      ph->mTimeSinceLastEventMS = EOLremainderNopTime;
      ++ph;
    }

    if (hasLooped) {
      // this right here is exactly the point where loop time is 0.
      mWriteZeroTimeCursor = (uint8_t*)ph;
    }

    for (size_t i = 0; i < fullNops; ++i) {
      WRITE_LOOP_EVENT_HEADER_MARKER;
      ph->mEventType = LoopEventType::Nop;
      ph->mTimeSinceLastEventMS = LOOPEVENTTIME_MAX;
      ++ph;
    }

    WRITE_LOOP_EVENT_HEADER_MARKER;
    ph->mEventType = eventType;
    ph->mTimeSinceLastEventMS = remainderTime;
    ++ph;
    uint8_t* pp = (uint8_t*)ph;
    if (eventInfo.mParamsSize) {
      memcpy(pp, params, eventInfo.mParamsSize);
      pp += eventInfo.mParamsSize;
    }

    mEventCursor.mLoopTimeMS = GetStatus().mCurrentLoopTimeMS;
    mEventCursor.mP = pp;
    mEventsValidEnd = std::max(mEventsValidEnd, mEventCursor.mP);

    // advance the "prev" write cursor if it exists. do this before checking the OOB conditions below, to make a bit of room and squeeze out a few more bytes.
    if (mWritePrevCursor.mP) {
      // fast-forward so it's just under 1 loop's worth of material.
      uint32_t recordedDuration = MeasureRecordedDurationPE();
      while (recordedDuration > GetStatus().mLoopDurationMS) {
        // advance & shrink duration
        uint32_t less = mWritePrevCursor.PeekEvent().mHeader.mTimeSinceLastEventMS;
        Dump();
        cc::log("Advancing P");
        if (mWritePrevCursor.MoveNext(GetStatus(), mEventsBegin, mEventsValidEnd)) {
          // in order to maintain correct mEventsValidEnd, when P wraps, we need to reset it to E.
          // (transition from ZEP to PZE)
          mEventsValidEnd = mEventCursor.mP;
        }
        if (less > recordedDuration) {
          break; // we bit off even more than we had to; bail.
        }
        recordedDuration -= less;
      }
      Dump();
      cc::log("^ that's after i advanced P.");
    }

    return true;
  }

  template<typename Tparams>
  bool WriteEvent(LoopEventType eventType, const Tparams& eventParams)
  {
    return WriteEventRaw(eventType, &eventParams);
  }

  void Write(const MusicalVoice& liveVoice)
  {
    CCASSERT(!!mEventsBegin.mP);

    // convert livevoice to an event and write the event.
    if (liveVoice.mNeedsNoteOff) {
      mRunningVoice.mIsNoteCurrentlyOn = false;
      WriteEvent(LoopEventType::NoteOff, nullptr);
    }

    // capture alteration events before note on
    if (liveVoice.mSynthPatch != mRunningVoice.mSynthPatch) {
      mRunningVoice.mSynthPatch = liveVoice.mSynthPatch;
      WriteEvent(LoopEventType::SynthPatchChange, LoopEvent_SynthPatchChangeParams{ mRunningVoice.mSynthPatch });
    }

    if (liveVoice.mHarmPatch != mRunningVoice.mHarmPatch) {
      mRunningVoice.mHarmPatch = liveVoice.mHarmPatch;
      WriteEvent(LoopEventType::HarmPatchChange, LoopEvent_HarmPatchChangeParams{ mRunningVoice.mHarmPatch });
    }

#ifndef EWI_UNIT_TESTS
    if (mBreathPitchThrottler.IsReady()) {
#else
    if (true) {
#endif // EWI_UNIT_TESTS
      bool breathChanged = abs(mRunningVoice.mBreath01 - liveVoice.mBreath01) > LOOP_BREATH_PITCH_EPSILON;
      bool pitchChanged = abs(mRunningVoice.mPitchBendN11 - liveVoice.mPitchBendN11) > LOOP_BREATH_PITCH_EPSILON;
      if (breathChanged && pitchChanged) {
        mRunningVoice.mBreath01 = liveVoice.mBreath01;
        mRunningVoice.mPitchBendN11 = liveVoice.mPitchBendN11;
        WriteEvent(LoopEventType::BreathAndPitch, LoopEvent_BreathAndPitchParams{ mRunningVoice.mBreath01, mRunningVoice.mPitchBendN11 });
      }
      else if (breathChanged) {
        mRunningVoice.mBreath01 = liveVoice.mBreath01;
        WriteEvent(LoopEventType::Breath, LoopEvent_BreathParams{ mRunningVoice.mBreath01 });
      }
      else if (pitchChanged) {
        mRunningVoice.mPitchBendN11 = liveVoice.mPitchBendN11;
        WriteEvent(LoopEventType::Pitch, LoopEvent_PitchParams{ mRunningVoice.mPitchBendN11 });
      }
    }

    if (liveVoice.mNeedsNoteOn) {
      mRunningVoice.mIsNoteCurrentlyOn = true;
      mRunningVoice.mMidiNote = liveVoice.mMidiNote;
      mRunningVoice.mVelocity = liveVoice.mVelocity;
      WriteEvent(LoopEventType::NoteOn, LoopEvent_NoteOnParams{ liveVoice.mMidiNote, liveVoice.mVelocity });
    }
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct LooperAndHarmonizer
{
  uint8_t mCurrentlyWritingLayer = 0;
  uint8_t mBuffer[LOOPER_MEMORY_TOTAL_BYTES];
  LoopEventStream mLayers[LOOP_LAYERS];

  LoopStatus mStatus;
  Stopwatch mLoopTimer;

  Harmonizer mHarmonizer;

  // UI actions.
  void LoopIt()
  {
    // basically this is a one-button loop function.
    // if the loop beginning is not set, begin measuring loop length.
    // if loop length is not set, set the length and commit to the next layer.
    // if the loop length is set, just commit to next layer.
    switch (mStatus.mState) {
    case LooperState::Idle:
      // reset the loop start, set loop time now.
      mLoopTimer.Restart();
      mCurrentlyWritingLayer = 0;
      mStatus.mState = LooperState::StartSet;
      mStatus.mCurrentLoopTimeMS = 0;
      mLayers[0].ResetBufferForRecording(mStatus, mBuffer, EndPtr(mBuffer));
      break;
    case LooperState::StartSet:
      // set loop duration, set up next loop layer.
      if (mStatus.mCurrentLoopTimeMS < LOOP_MIN_DURATION) {
        break;
      }
      UpdateCurrentLoopTimeMS();
      mStatus.mLoopDurationMS = mStatus.mCurrentLoopTimeMS;
      mStatus.mState = LooperState::DurationSet;
      mStatus.mCurrentLoopTimeMS = 0;
      // FALL-THROUGH
    case LooperState::DurationSet:
      // tell the currently-writing layer it's over.
      if (mCurrentlyWritingLayer < SizeofStaticArray(mLayers)) {
        uint8_t* buf = mLayers[mCurrentlyWritingLayer].WrapUpRecording();
        mLayers[mCurrentlyWritingLayer].mIsPlaying = true;

        mCurrentlyWritingLayer++; // can go out of bounds!
        if (mCurrentlyWritingLayer < SizeofStaticArray(mLayers)) {
          mLayers[mCurrentlyWritingLayer].ResetBufferForRecording(mStatus, buf, EndPtr(mBuffer));// prepare the next layer for recording.
        }
      }

      break;
    }
  }

  void Clear()
  {
    mCurrentlyWritingLayer = 0;
    mStatus.mState = LooperState::Idle;
    mLoopTimer.Restart();
    for (auto& l : mLayers) {
      l.mIsPlaying = false;
    }
  }

  void UpdateCurrentLoopTimeMS() {
    uint32_t ret = (uint32_t)(mLoopTimer.ElapsedMicros() / 1000); // theoretical error for very long loop times.
    if (mStatus.mState == LooperState::DurationSet) {
      // handle wrapping.
      ret %= mStatus.mLoopDurationMS;
    }
    mStatus.mCurrentLoopTimeMS = ret;
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
    UpdateCurrentLoopTimeMS();

    // if you have exhausted layers, don't write.
    if (mStatus.mState != LooperState::Idle && (mCurrentlyWritingLayer < SizeofStaticArray(mLayers))) {
      mLayers[mCurrentlyWritingLayer].Write(liveVoice);
    }

    // in order to feed the scale follower with a single buffer for both "live" (may not be played!) and harmonized voices,
    // go through all looper layers & harmonizer voices, and if we know the note then put it. if there are 0 notes output,
    // then just put the "live" note but mark it as muted.

    MusicalVoice* pout = outp;
    MusicalVoice* pLiveVoices[LOOP_LAYERS]; // keep track of where i read layer state into, for later when deduced voices are filled in.

    // output the live actually-playing voice.
    if (mCurrentlyWritingLayer < SizeofStaticArray(mLayers)) {
      pout->AssignFromLoopStream(liveVoice);
      pout += mHarmonizer.Harmonize(mCurrentlyWritingLayer, pout, pout + 1, outpEnd, Harmonizer::VoiceFilterOptions::ExcludeDeducedVoices);
    }

    // do the same for other layers; they're read from stream.
    for (uint8_t iLayer = 0; iLayer < SizeofStaticArray(mLayers); ++iLayer) {
      if (iLayer == mCurrentlyWritingLayer)
        continue;
      auto& l = mLayers[iLayer];
      if (l.mIsPlaying) {
        l.ReadUntilLoopTime(*pout);
        pLiveVoices[iLayer] = pout;
        pout += mHarmonizer.Harmonize(iLayer, pout, pout + 1, outpEnd, Harmonizer::VoiceFilterOptions::ExcludeDeducedVoices);
      }
    }

#ifndef EWI_UNIT_TESTS
    gScaleFollower.Update(outp, pout - outp);
#endif // EWI_UNIT_TESTS

    // go through and fill in all deduced voices. logically, the "live voices" will all remain untouched so voices just get filled in.
    for (uint8_t iLayer = 0; iLayer < SizeofStaticArray(mLayers); ++iLayer) {
      auto& l = mLayers[iLayer];
      if (l.mIsPlaying) {
        pout += mHarmonizer.Harmonize(iLayer, pLiveVoices[iLayer], pout, outpEnd, Harmonizer::VoiceFilterOptions::OnlyDeducedVoices);
      }
    }

    return pout - outp;
  }
};

