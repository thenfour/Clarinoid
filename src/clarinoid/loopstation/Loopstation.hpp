// optimizing for material rather than silence. in other words, i prefer short
// datatypes for durations, which means longer payloads for silence, but optimal
// for many consecutive events.

// todo: post-processing: pruning out
// - breath & pitch during no note on
// - transient notes

#pragma once

#ifdef CLARINOID_MODULE_TEST
#include <vector>
#endif // CLARINOID_MODULE_TEST

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/scale_follower/ScaleFollower.hpp>
#include <clarinoid/harmonizer/harmonizer.hpp>

namespace clarinoid
{
static const uint32_t LOOP_BREATH_PITCH_RESOLUTION_MS = 5; // record only every N milliseconds max. This should probably be coordinated with the similar throttler in MusicalState, to make sure it plays well together
static const uint32_t LOOP_MIN_DURATION = 100; // minimum length in MS of a loop layer
}

#include "LoopstationMemory.hpp"
#include "LoopstationEvents.hpp"

namespace clarinoid
{

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

// |---PxxxxxxxxxE (pe)
// |xxE---Pxxxxxxx (ep)
enum class LayoutSituation
{
  PE,
  EP,
};

struct LoopCursor
{
  Ptr mP;
  // the time AT the cursor. the cursor points to an event which has a delay
  // while PeekLoopTime() tells you when the current event would happen
  // careful to use these correctly.
  uint32_t mLoopTimeMS = 0;
  MusicalVoice mRunningVoice;

  void Set(Ptr p, uint32_t loopTime, const MusicalVoice& mv) {
    mP = p;
    mLoopTimeMS = loopTime;
    mRunningVoice = mv;
  }

  void Assign(const LoopCursor& rhs) {
    mP = rhs.mP;
    mLoopTimeMS = rhs.mLoopTimeMS;
    mRunningVoice = rhs.mRunningVoice;
  }

  void SetNull() {
    mP = 0;
    mLoopTimeMS = 0;
  }

  // you can have an event at the end of the buffer which has a delay which would loop back the loop time.
  // this will account for this and return a value within the loop.
  uint32_t PeekLoopTimeWrapSensitive(const LoopStatus& status)
  {
    CCASSERT(status.mState == LooperState::DurationSet);
    event_delay_t delay = LoopEvent_PeekDelay(mP);
    uint32_t ret = mLoopTimeMS + delay;
    while (ret >= status.mLoopDurationMS)
      ret -= status.mLoopDurationMS;
    return ret;
  }

  // does this event contain the given loop time? if our event starts at L-50 with duration 150, then 0 is contained for example.
  bool TouchesTime(const LoopStatus& status, uint32_t r) {
    event_delay_t delay = LoopEvent_PeekDelay(mP);

    uint32_t dnEventTimeWithDelay = mLoopTimeMS + delay;
    if (dnEventTimeWithDelay < status.mLoopDurationMS) {
      // simple case where no boundaries are crossed.
      if (r < mLoopTimeMS)
        return false;
      if (r > dnEventTimeWithDelay)
        return false;
      return true;
    }
    uint32_t normEventTimeWithDelay = dnEventTimeWithDelay - status.mLoopDurationMS;
    // this event crosses zero boundary.
    // 0ms|-------E=r=Z|===D---|
    // 0ms|---r---E===Z|===D---|
    // 0ms|-------E===Z|===D-r-|
    if (r >= mLoopTimeMS)
      return true;
    if (r <= normEventTimeWithDelay)
      return true;
    return false;
  }

  // read next event in the stream and integrate to outp.
  // advances cursors.
  // returns true if the cursor wrapped.
  bool ConsumeSingleEvent(const LoopStatus& status, const LoopCursor& bufferBegin, const Ptr& bufferEnd)
  {
    event_delay_t delay;
    LoopEventType eventType;
    uint8_t byte2;
    LoopEvent_ReadHeader(mP, delay, eventType, byte2);

    mLoopTimeMS += delay;
    bool ret = false;
    if (status.mState == LooperState::DurationSet) {
      if (mLoopTimeMS >= status.mLoopDurationMS)
        mLoopTimeMS -= status.mLoopDurationMS;
    }
    if (mP >= bufferEnd) {
      Assign(bufferBegin);
      ret = true;
    }

    // take various actions based on stream, and advance cursor again if needed.
    switch (eventType) {
    case LoopEventType::Nop: // {  } 
      break;
    case LoopEventType::NoteOff: // { }
    {
      LoopEvent_NoteOff e;// = LoopEvent_NoteOff(mP);
      e.ApplyToVoice(mRunningVoice);
      break;
    }
    case LoopEventType::NoteOn: // { uint8_t note, uint8_t velocity }
    {
      LoopEvent_NoteOn e(mP);
      e.ApplyToVoice(mRunningVoice);
      break;
    }
    case LoopEventType::Breath: // { breath }
    {
      LoopEvent_Breath e(mP, byte2);
      e.ApplyToVoice(mRunningVoice);
      break;
    }
    case LoopEventType::Pitch: // { float pitch }
    {
      LoopEvent_Pitch e(mP, byte2);
      e.ApplyToVoice(mRunningVoice);
      break;
    }
    case LoopEventType::BreathAndPitch: // { float breath, float pitch }
    {
      LoopEvent_BreathAndPitch e(mP, byte2);
      e.ApplyToVoice(mRunningVoice);
      break;
    }
    case LoopEventType::SynthPatchChange: // { uint8_t patchid }
    {
      LoopEvent_SynthPatchChange e(mP, byte2);
      e.ApplyToVoice(mRunningVoice);
      break;
    }
    case LoopEventType::HarmPatchChange: // { uint8_t patchid }
    {
      LoopEvent_HarmPatchChange e(mP, byte2);
      e.ApplyToVoice(mRunningVoice);
      break;
    }
    case LoopEventType::FullState:
    {
      LoopEvent_FullState e(mP, byte2);
      e.ApplyToVoice(mRunningVoice);
      break;
    }
    default:
      CCASSERT(false);
      break;
    }
    return ret;
  }

};

// NOT stream-friendly.
struct LoopEvent_DebugUnified
{
  Ptr mP;
  uint32_t mLoopTimeMS;
  event_delay_t mDelayMS;
  LoopEventType mEventType;
  const char *mName;
  uint8_t mPayloadSize;
  String mParamString;

  LoopEvent_Nop mNop;
  LoopEvent_NoteOff mNoteOff;
  LoopEvent_NoteOn mNoteOnParams;
  LoopEvent_Breath mBreathParams;
  LoopEvent_Pitch mPitchParams;
  LoopEvent_BreathAndPitch mBreathAndPitchParams;
  LoopEvent_SynthPatchChange mSynthPatchChangeParams;
  LoopEvent_HarmPatchChange mHarmPatchChangeParams;
  LoopEvent_FullState mFullStateParams;

  LoopEvent_DebugUnified(const LoopCursor& c) :
    mP(c.mP),
    mLoopTimeMS(c.mLoopTimeMS)
  {
    Ptr p = c.mP;
    uint8_t byte2;
    LoopEvent_ReadHeader(p, mDelayMS, mEventType, byte2);
    switch (mEventType) {
    case LoopEventType::Nop:
      mName = LoopEvent_Nop::Name;
      mPayloadSize = LoopEvent_Nop::PayloadSizeBytes;
      mNop = LoopEvent_Nop();
      mParamString = mNop.ToString();
      break;
    case LoopEventType::NoteOff:
      mName = LoopEvent_NoteOff::Name;
      mPayloadSize = LoopEvent_NoteOff::PayloadSizeBytes;
      mNoteOff = LoopEvent_NoteOff();
      mParamString = mNoteOff.ToString();
      break;
    case LoopEventType::NoteOn:
      mName = LoopEvent_NoteOn::Name;
      mPayloadSize = LoopEvent_NoteOn::PayloadSizeBytes;
      mNoteOnParams = LoopEvent_NoteOn(p);
      mParamString = mNoteOnParams.ToString();
      break;
    case LoopEventType::Breath:
      mName = LoopEvent_Breath::Name;
      mPayloadSize = LoopEvent_Breath::PayloadSizeBytes;
      mBreathParams = LoopEvent_Breath(p, byte2);
      mParamString = mBreathParams.ToString();
      break;
    case LoopEventType::Pitch:
      mName = LoopEvent_Pitch::Name;
      mPayloadSize = LoopEvent_Pitch::PayloadSizeBytes;
      mPitchParams = LoopEvent_Pitch(p, byte2);
      mParamString = mPitchParams.ToString();
      break;
    case LoopEventType::BreathAndPitch:
      mName = LoopEvent_BreathAndPitch::Name;
      mPayloadSize = LoopEvent_BreathAndPitch::PayloadSizeBytes;
      mBreathAndPitchParams = LoopEvent_BreathAndPitch(p, byte2);
      mParamString = mBreathAndPitchParams.ToString();
      break;
    case LoopEventType::SynthPatchChange:
      mName = LoopEvent_SynthPatchChange::Name;
      mPayloadSize = LoopEvent_SynthPatchChange::PayloadSizeBytes;
      mSynthPatchChangeParams = LoopEvent_SynthPatchChange(p, byte2);
      mParamString = mSynthPatchChangeParams.ToString();
      break;
    case LoopEventType::HarmPatchChange:
      mName = LoopEvent_HarmPatchChange::Name;
      mPayloadSize = LoopEvent_HarmPatchChange::PayloadSizeBytes;
      mHarmPatchChangeParams = LoopEvent_HarmPatchChange(p, byte2);
      mParamString = mHarmPatchChangeParams.ToString();
      break;
    case LoopEventType::FullState:
      mName = LoopEvent_FullState::Name;
      mPayloadSize = LoopEvent_FullState::PayloadSizeBytes;
      mFullStateParams = LoopEvent_FullState(p, byte2);
      mParamString = mFullStateParams.ToString();
      break;
    }
  }

};


// enum class LoopEventStreamState : uint8_t
// {
//   Inactive,
//   Recording,
//   Playing
// }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct LoopEventStream
{
  //LoopEventStreamState mStreamState = LoopEventStreamState::Inactive;
  bool mIsPlaying = false;// Mute function
  bool mIsRecording = false; // necessary for checking cursor conditions & states.

  bool mOOM = false; // Out of memory condition. Set this flag to stop recording.
  bool mNeedsNoteOff = false;

  LoopCursor mBufferBegin;
  Ptr mBufferEnd;
  Ptr mEventsValidEnd;// when writing, this can be before the end of the raw buffer.

  // for writing, points to free area for writing (don't read).
  // for reading, points to the next unread event
  LoopCursor mCursor;

  // if you're writing into the stream, and you pass the loop length, then we start tracking a complete loop worth of events.
  // this way when we stop recording, we can accurately piece together a complete loop of events in sequence.
  LoopCursor mPrevCursor;

  CCThrottlerT<LOOP_BREATH_PITCH_RESOLUTION_MS> mBreathPitchThrottler;

  // valid only when READING
  size_t GetMemoryUsage() const {
    if (!mpStatus)
      return 0;
    if (mpStatus->mState != LooperState::DurationSet)
      return 0;
    if (!mIsPlaying)
      return 0;
    return mEventsValidEnd.DistanceBytes(mBufferBegin.mP);
  }

  const char *GetStateString() const {
    if (mIsPlaying && mOOM) return "Playing but OOM!";
    if (mIsPlaying && !mOOM) return "Playing";
    if (mIsRecording && mOOM) return "Recording but OOM!";
    if (mIsRecording && !mOOM) return "Recording";
    if (mNeedsNoteOff) return "Needs note off";
    return "Idle";
  }

  const LoopStatus* mpStatus = nullptr;
  const LoopStatus& GetStatus() const { return *mpStatus; }

  // represents a recorded size of 0
  bool IsEmpty() const {
    return mBufferBegin.mP == mEventsValidEnd;
  }

  // basically if this returns false then nothing is lost by calling BeginRecording(). Is this voice not available?
  bool IsInUse() const
  {
    if (mIsPlaying) return true;
    if (mIsRecording) return true;
    if (mNeedsNoteOff) return true;
    return false;
  }

  uint32_t Duration(uint32_t begin, uint32_t end) {
    if ((GetStatus().mState == LooperState::DurationSet) && (end < begin)) {
      end += GetStatus().mLoopDurationMS;
    }
    CCASSERT(end >= begin);
    return end - begin;
  }

  // musical stop (if playing, it will emit a note off for example).
  void Stop()
  {
    if (mOOM)
      return; // there are ways to sorta work with a OOM situation, but it's not necessary to add the complexity for a shoddy solution.
    if (!mIsPlaying)
      return;
    if (mCursor.mRunningVoice.IsPlaying()) {
      mNeedsNoteOff = true;
    }
    mIsPlaying = false;
  }

  // returns true if this layer either:
  // 1. has a note playing currently,
  // 2. or, has any events applied/read. including note offs when the voice is not playing.
  // outputs a new copy of musical state to outp.
  bool ReadUntilLoopTime(MusicalVoice& outp)
  {
    if (mOOM)
      return false; // there are ways to sorta work with a OOM situation, but it's not necessary to add the complexity for a shoddy solution.

    if (mNeedsNoteOff) {
      LoopEvent_NoteOff e;
      e.ApplyToVoice(mCursor.mRunningVoice);
      outp = mCursor.mRunningVoice;
      mNeedsNoteOff = false;
      return true;
    }
    if (!mIsPlaying)
      return false; // muted.
    if (mBufferBegin.mP == mEventsValidEnd)
      return false;

    bool eventsApplied = false;

    // assumption: current cursor is AT or BEFORE the requested time.
    // so if the next event would be TOO far, stop reading. otherwise just go.
    uint32_t reqTime = GetStatus().mCurrentLoopTimeMS;

    // there are no gaps. so we should *encounter* the cursor time.
    while (!mCursor.TouchesTime(GetStatus(), reqTime)) {
      mCursor.ConsumeSingleEvent(GetStatus(), mBufferBegin, mEventsValidEnd);
      eventsApplied = true;
    }

    // if this event is after reqtime, bail. we know it's touching reqtime too, so
    // the only passing condition is if r is exactly equal to the event time post-delay
    // r-------| <-- only pass if dly=0
    // |---r---| <-- dly
    // |-------r <-- r is exactly on the event time. also pass.
    uint32_t t = mCursor.PeekLoopTimeWrapSensitive(GetStatus());
    if (t == reqTime) {
      while (mCursor.PeekLoopTimeWrapSensitive(GetStatus()) == t) {
        mCursor.ConsumeSingleEvent(GetStatus(), mBufferBegin, mEventsValidEnd);
        eventsApplied = true;
      }
    }

    outp = mCursor.mRunningVoice;
    return eventsApplied || mCursor.mRunningVoice.IsPlaying();
  }

  void StartRecording(const LoopStatus& status, const MusicalVoice& musicalStatus, const Ptr& begin, const Ptr& end)
  {
    mIsPlaying = false;
    mIsRecording = true;
    mpStatus = &status;
    mBufferBegin.Set(begin, status.mCurrentLoopTimeMS, musicalStatus);
    mEventsValidEnd = begin;
    mPrevCursor.Assign(mBufferBegin);
    mBufferEnd = end;
    mCursor.Assign(mBufferBegin);

    LoopEvent_FullState params(musicalStatus);
    WriteEventRaw(params);
  }

  LayoutSituation GetLayoutSituation() const
  {
    if (mPrevCursor.mP <= mCursor.mP) {
      return LayoutSituation::PE;
    }
    return LayoutSituation::EP;
  }

#ifdef CLARINOID_MODULE_TEST

  void Dump()
  {
    clarinoid::log("----");
    auto events = DebugGetStream();
    auto layout = mIsRecording ? GetLayoutSituation() : LayoutSituation::PE;

    if (mIsRecording) {
      switch (layout) {
      case LayoutSituation::PE:// |---PxxxxZxxxxE (pze)
        clarinoid::log("|---PxxxxxxxxxE (PE)");
        break;
      case LayoutSituation::EP:// |xxE---PxxxxZxx (epz)
        clarinoid::log("|xxE---Pxxxxxxx (EP)");
        break;
      }

      if (layout == LayoutSituation::PE) {
        clarinoid::log("   > { %p pe padding %d bytes }", mBufferBegin.mP, mPrevCursor.mP.DistanceBytes(mBufferBegin.mP));
      }
    }
    else {
      clarinoid::log("not recording; layout not relevant.");
    }

    bool encounteredPadding = false;
    for (auto& e : events) {
      int loopTime = e.mLoopTimeMS;

      if (mIsRecording && !encounteredPadding && (e.mP >= mCursor.mP)) {
        switch (layout) {
        case LayoutSituation::EP:
          encounteredPadding = true;
          clarinoid::log(" E > { %p t=%d EP padding %d bytes }", mCursor.mP, mCursor.mLoopTimeMS, mPrevCursor.mP.DistanceBytes(mCursor.mP));
          break;
        }
      }

      clarinoid::log("%s%s%s [%p (+%d) t=%d: dly=%d <looptime=%d> type=%s, params=%s]",
        e.mP == mBufferBegin.mP ? "B" : " ",
        e.mP == mCursor.mP ? "E" : " ",
        e.mP == mPrevCursor.mP ? "P" : " ",
        e.mP,
        e.mP.DistanceBytes(mBufferBegin.mP),
        loopTime,
        (int)e.mDelayMS,
        loopTime + (int)e.mDelayMS,
        e.mName,
        e.mParamString.mStr.str().c_str());
    } // for()

    if (mCursor.mP == mEventsValidEnd) {
      clarinoid::log(" E  [%p (+%d) t=%d <eof> ]",
        mCursor.mP,
        mCursor.mP.DistanceBytes(mBufferBegin.mP),
        mCursor.mLoopTimeMS
        );
    }
    clarinoid::log("   > { %p VE padding %d bytes } <", mEventsValidEnd, mEventsValidEnd.DistanceBytes(mBufferEnd));
    clarinoid::log("   > { %p end } <", mBufferEnd);
    clarinoid::log("begin-valid: %d, event count: %d %s",
      mEventsValidEnd.DistanceBytes(mBufferBegin.mP),
      events.size(),
      mOOM ? "OOM!" : ""
      );
    if (mBufferBegin.mP) {
      clarinoid::log("B state {%s}", LoopEvent_FullState(mBufferBegin.mRunningVoice).ToString().mStr.str().data());
    }
    if (mPrevCursor.mP) {
      clarinoid::log("P state {%s}", LoopEvent_FullState(mPrevCursor.mRunningVoice).ToString().mStr.str().data());
    }
    if (mCursor.mP) {
      clarinoid::log("E state {%s}", LoopEvent_FullState(mCursor.mRunningVoice).ToString().mStr.str().data());
    }
  }

  // even here we must be sensitive to scenarios.
  std::vector<LoopEvent_DebugUnified> DebugGetStream() const
  {
    std::vector<LoopEvent_DebugUnified> ret;
    if (!mIsRecording) {
      LoopCursor c; c.Assign(mBufferBegin);
      while (c.mP < mEventsValidEnd) {
        ret.push_back(LoopEvent_DebugUnified(c));
        if (c.ConsumeSingleEvent(GetStatus(), mBufferBegin, mBufferEnd))
          break;
      };
      return ret;
    }
    switch (GetLayoutSituation()) {
    case LayoutSituation::PE:// |---PxxxxxxxxE (pe)
    {
      LoopCursor c; c.Assign(mPrevCursor);
      while (c.mP < mEventsValidEnd) {
        ret.push_back(LoopEvent_DebugUnified(c));
        if (c.ConsumeSingleEvent(GetStatus(), mBufferBegin, mBufferEnd))
          break;
      };
      return ret;
    }
    case LayoutSituation::EP:// |xxE---PxxxxZxx (ep)
    {
      LoopCursor c; c.Assign(mBufferBegin);
      while (c.mP < mCursor.mP) {
        ret.push_back(LoopEvent_DebugUnified(c));
        if (c.ConsumeSingleEvent(GetStatus(), mBufferBegin, mBufferEnd))
          break;
      };
      c.Assign(mPrevCursor);
      while (c.mP < mEventsValidEnd) {
        ret.push_back(LoopEvent_DebugUnified(c));
        if (c.ConsumeSingleEvent(GetStatus(), mBufferBegin, mBufferEnd))
          break;
      };
      return ret;
    }
    }

    return ret;
  }
#endif // CLARINOID_MODULE_TEST

  // specialized for writing at the end of the buffer, just before the buffer
  // changes from writing to reading.
  // will not wrap the buffer. returns new end ptr.
  // on OOM just does nothing (loop will be slightly too short)
  void WriteSeamNops(Ptr& p, uint32_t duration) {
    if (mOOM)
      return;
    size_t fullNops;
    event_delay_t remainderTime;
    DivRemBitwise<LOOP_EVENT_DELAY_BITS>(duration, fullNops, remainderTime);
    size_t bytesNeeded = fullNops + (remainderTime ? 1 : 0);
    bytesNeeded *= LoopEvent_Nop::PayloadSizeBytes;
    if (p.PlusBytes(bytesNeeded) >= mBufferEnd) {
      mOOM = true;
      return;
    }

    for (size_t i = 0; i < fullNops; ++i) {
      LoopEvent_Nop::Write(p, LOOP_EVENT_DELAY_MAX);
    }
    if (remainderTime) {
      LoopEvent_Nop::Write(p, remainderTime);
    }
  }

  // return the end of the used buffer.
  // point cursor at the time in status.
  // we want the resulting buffer to start at the earliest known material, and end at mCursor.
  Ptr WrapUpRecording() {
    // if we didn't record any events, this buffer is basically transparent; nothing needs to be done at all. nothing will be played.
    if (IsEmpty() || mOOM) {
      mIsPlaying = true;
      mIsRecording = false;
      mPrevCursor.SetNull(); // no longer valid.
      mEventsValidEnd = mBufferBegin.mP;
      mBufferEnd = mEventsValidEnd;
      return mEventsValidEnd;
    }

    uint32_t recordedDuration = Duration(mPrevCursor.mLoopTimeMS, mCursor.mLoopTimeMS);// pzMS + zeMS;

    // Get the buffer arranged with hopefully room at the end for seam.
    Ptr segmentABegin = mPrevCursor.mP;
    Ptr segmentAEnd = mEventsValidEnd;
    Ptr segmentBBegin = mBufferBegin.mP;
    Ptr segmentBEnd = mBufferBegin.mP;// |---PxxxxZxxxxE (pe)

    size_t mainBufferValidBytes = segmentAEnd.DistanceBytes(segmentABegin);

    if (GetLayoutSituation() == LayoutSituation::EP) {// |xxE---PxxxxZxx (ep)
      segmentBEnd = mCursor.mP;
      mainBufferValidBytes += segmentBEnd.DistanceBytes(segmentBBegin);
    }

    // check for OOM.
    size_t bytesNeeded = LoopEvent_FullState::PayloadSizeBytes;
    if (mainBufferValidBytes + bytesNeeded > mBufferEnd.DistanceBytes(mBufferBegin.mP))
    {
      mOOM = true;
      mIsPlaying = true;
      mIsRecording = false;
      mPrevCursor.SetNull(); // no longer valid.
      mEventsValidEnd = mBufferBegin.mP;
      mBufferEnd = mEventsValidEnd;
      return mEventsValidEnd;
    }

    event_delay_t firstEventDelay;
    LoopEventType firstEventType;
    uint8_t throwaway;
    Ptr tempCursor(mPrevCursor.mP);
    LoopEvent_ReadHeader(tempCursor, firstEventDelay, firstEventType, throwaway);
    bool shouldWriteFullState = firstEventType != LoopEventType::FullState;
    Ptr fsCursor = mBufferBegin.mP;
    UnifyCircularBuffer_Left(segmentABegin, segmentAEnd, segmentBBegin, segmentBEnd, LOOPSTATION_TEMP_BUFFER);

    if (shouldWriteFullState) {
      LoopEvent_SurgicallyWriteDelayInPlace(Ptr(segmentBBegin), 0);
      mBufferBegin.mLoopTimeMS = mPrevCursor.mLoopTimeMS;
      mBufferBegin.mRunningVoice = mPrevCursor.mRunningVoice;

      // slide it over to make room
      OrderedMemcpy(mBufferBegin.mP.PlusBytes(bytesNeeded), segmentBBegin, mainBufferValidBytes);

      // write our full state
      LoopEvent_FullState fullStateEvent(mPrevCursor.mRunningVoice);
      fullStateEvent.Write(fsCursor, firstEventDelay);
    }

    mEventsValidEnd = fsCursor.PlusBytes(mainBufferValidBytes);

    // write a "seam" at the end of the buffer to make our recorded material exactly 1 loop in duration.
    WriteSeamNops(mEventsValidEnd, GetStatus().mLoopDurationMS - recordedDuration);

    mBufferEnd = mEventsValidEnd;
    mPrevCursor.SetNull(); // no longer valid.
    mIsPlaying = true;
    mIsRecording = false;
    mCursor.Assign(mBufferBegin);
    return mEventsValidEnd;
  }

  // sets mCursor to a place with enough bytes.
  // return true on success.
  bool FindMemoryOrOOM(size_t bytesNeeded)
  {
    // ZE, PZE check for end of buffer
    // EPZ and ZEP check f or P
    switch (GetLayoutSituation()) {
    case LayoutSituation::PE:// |---PxxxxZxxxxE (pze)
      if (mCursor.mP.PlusBytes(bytesNeeded) < mBufferEnd) {
        return true;// we're already set.
      }
      // uh oh. wrap buffer?
      if (mBufferBegin.mP.PlusBytes(bytesNeeded) >= mPrevCursor.mP) {
        // too small; we would eat into our own tail. OOM.
        mOOM = true;
        return false;
      }
      // safe to use beginning of buffer, transition to EPZ
      mCursor.mP = mBufferBegin.mP;
      mBufferBegin.mLoopTimeMS = mCursor.mLoopTimeMS;
      return true;
    case LayoutSituation::EP:// |xxE---PxxxxZxx (epz)
      if (mCursor.mP.PlusBytes(bytesNeeded) < mPrevCursor.mP) {
        return true;// we're already set.
      }
      // too small; we would eat into our own tail. OOM.
      mOOM = true;
      return false;
    }
    CCASSERT(false);
    return false;
  }

  template<typename Tparams>
  bool WriteEventRaw(const Tparams& e)
  {
    if (mOOM)
      return false;

    if (GetStatus().mState == LooperState::DurationSet) {
      CCASSERT(GetStatus().mCurrentLoopTimeMS < GetStatus().mLoopDurationMS);
    }

    // not "has looped". we want to know if you've passed rec start time.
    uint32_t loopTimeElapsedSinceLastWrite = Duration(mCursor.mLoopTimeMS, GetStatus().mCurrentLoopTimeMS);

    // this is a useful calculation to do NOW, because we know it's always less than 1 loop duration in length which simplifies math.
    int32_t peMS = (int32_t)Duration(mPrevCursor.mLoopTimeMS, mCursor.mLoopTimeMS);
    peMS += loopTimeElapsedSinceLastWrite; // and now it can be longer than a loop. handle this scenario later.

    // calculate a boundary to rec time, not 0.
    uint32_t delayTime = loopTimeElapsedSinceLastWrite;
    size_t fullNops;
    event_delay_t eventDelayTime;
    DivRemBitwise<LOOP_EVENT_DELAY_BITS>(delayTime, fullNops, eventDelayTime);
    if (eventDelayTime == 0 && fullNops > 0) {
      --fullNops;
      eventDelayTime = LOOP_EVENT_DELAY_MAX;
    }
    size_t bytesNeeded = (LoopEvent_Nop::PayloadSizeBytes * fullNops) + Tparams::PayloadSizeBytes;

    // CHECK for OOM, deal with wrapping buffer.
    if (!FindMemoryOrOOM(bytesNeeded)) {
      return false;
    }

    // Write nops.
    for (size_t i = 0; i < fullNops; ++i) {
      LoopEvent_Nop::Write(mCursor.mP, LOOP_EVENT_DELAY_MAX);
    }

    e.Write(mCursor.mP, eventDelayTime);

    mCursor.mLoopTimeMS = GetStatus().mCurrentLoopTimeMS;
    mEventsValidEnd = std::max(mEventsValidEnd, mCursor.mP);

    // advance the "prev" write cursor if it exists. do this before checking the OOB conditions below, to make a bit of room and squeeze out a few more bytes.
    if (GetStatus().mState == LooperState::DurationSet) {
      // fast-forward P so it's just under 1 loop's worth of material.
      // it's not necessary to support a full loop because you'd end up with 2 versions of "state" at the loop point (beginning + end).
      while (peMS >= (int32_t)GetStatus().mLoopDurationMS) {
        uint32_t less = LoopEvent_PeekDelay(mPrevCursor.mP);// ->mTimeSinceLastEventMS;
        if (mPrevCursor.ConsumeSingleEvent(GetStatus(), mBufferBegin, mEventsValidEnd)) {
          // in order to maintain correct mEventsValidEnd, when P wraps, we need to reset it to E.
          // (transition from ZEP to PZE)
          mEventsValidEnd = mCursor.mP;
        }
        peMS -= less;
      }
    }

    return true;
  }

  void Write(const MusicalVoice& liveVoice)
  {
#ifndef CLARINOID_MODULE_TEST
    if (mBreathPitchThrottler.IsReady()) {
#else
    if (true) {
#endif // CLARINOID_MODULE_TEST
      bool breathChanged = mCursor.mRunningVoice.mBreath01 != liveVoice.mBreath01;
      bool pitchChanged = mCursor.mRunningVoice.mPitchBendN11 != liveVoice.mPitchBendN11;
      if (breathChanged && pitchChanged) {
        mCursor.mRunningVoice.mBreath01 = liveVoice.mBreath01;
        mCursor.mRunningVoice.mPitchBendN11 = liveVoice.mPitchBendN11;
        WriteEventRaw(LoopEvent_BreathAndPitch(liveVoice));
      }
      else if (breathChanged) {
        mCursor.mRunningVoice.mBreath01 = liveVoice.mBreath01;
        WriteEventRaw(LoopEvent_Breath(liveVoice));
      }
      else if (pitchChanged) {
        mCursor.mRunningVoice.mPitchBendN11 = liveVoice.mPitchBendN11;
        WriteEventRaw(LoopEvent_Pitch(liveVoice));
      }
    }

    MusicalVoiceTransitionEvents transitionState = CalculateTransitionEvents(mCursor.mRunningVoice, liveVoice);
    if (transitionState.mNeedsNoteOn) {
      mCursor.mRunningVoice.mMidiNote = liveVoice.mMidiNote;
      mCursor.mRunningVoice.mVelocity = liveVoice.mVelocity;
      WriteEventRaw(LoopEvent_NoteOn(liveVoice));
    } else if (transitionState.mNeedsNoteOff) {
      // ELSE because for legato notes, you'll get note on + note off at the same time.
      // if we actually write the note off in this case, you'll note off the new note. we don't store the note off note so just skip that.
      mCursor.mRunningVoice.mMidiNote = liveVoice.mMidiNote;
      mCursor.mRunningVoice.mVelocity = liveVoice.mVelocity;
      WriteEventRaw(LoopEvent_NoteOff());
    }

    // capture alteration events before note on
    if (liveVoice.mSynthPatch != mCursor.mRunningVoice.mSynthPatch) {
      mCursor.mRunningVoice.mSynthPatch = liveVoice.mSynthPatch;
      WriteEventRaw(LoopEvent_SynthPatchChange(liveVoice));
    }

    if (liveVoice.mHarmPatch != mCursor.mRunningVoice.mHarmPatch) {
      mCursor.mRunningVoice.mHarmPatch = liveVoice.mHarmPatch;
      WriteEventRaw(LoopEvent_HarmPatchChange(liveVoice));
    }
  }
};

} // namespace clarinoid
