// todo: when reading state back, track note ons, duration, etc.
// todo: pack this shit. we really should be able to get optimal space by just
// bit packing to make the header like 2 bytes long. 12-bit time (1 event = 4 secs)
// and 4-bit event type.
// then with 2-byte breath, it's only 4 bytes for a breath event. and heck, honestly
// breath is only 12-bits resolution anyway so if you want to shift bits further,
// 4=type, 12=breath, 8=delay
// that's 12 bits of header and delays of 255 millisec. for like 10 minutes of recording
// that's only 4k of delays, not bad.
// the complication is that the header is no longer an even byte boundary, everything needs 
// to be adapted to support this.

#pragma once
#pragma pack(push,1)

#ifndef EWI_UNIT_TESTS
#include <Shared_CCUtil.h>
#include "ScaleFollower.hpp"
#else
#include <vector>
static const size_t LOOP_LAYERS = 3;
#endif // EWI_UNIT_TESTS
#include "Harmonizer.hpp"

static const size_t LOOPER_MEMORY_TOTAL_BYTES = 131072;
static const size_t LOOPER_TEMP_BUFFER_BYTES = 8192;// a smaller buffer that's just used for intermediate copy ops
using LOOP_EVENT_DELAY = uint16_t;
static constexpr size_t LOOP_EVENT_DELAY_BITS = sizeof(LOOP_EVENT_DELAY) * 8;
static constexpr uint16_t LOOP_EVENT_DELAY_MAX = std::numeric_limits<LOOP_EVENT_DELAY>::max();
static const uint32_t LOOP_BREATH_PITCH_RESOLUTION_MS = 5; // record only every N milliseconds max. This should probably be coordinated with the similar throttler in MusicalState, to make sure it plays well together
static const uint32_t LOOP_MIN_DURATION = 100;

template<size_t divBits, typename Tval, typename Tremainder>
void DivRemBitwise(Tval val, size_t& wholeParts, Tremainder& remainder)
{
  wholeParts = val >> divBits;
  remainder = val & (1 << divBits) - 1;
}

static inline size_t PointerDistanceBytes(const void* a_, const void* b_)
{
  const uint8_t* a = (const uint8_t*)a_;
  const uint8_t* b = (const uint8_t*)b_;
  if (a > b) return a - b;
  return b - a;
}

template<typename T>
static T* AddPointerBytes(T* p_, size_t bytes)
{
  uint8_t* p = (uint8_t*)p_;
  return (T*)(p + bytes);
}

// for moving left, we can go forward order.
// for moving right, we should go backwards.
static void OrderedMemcpy(void* dest_, void const *src_, size_t bytes)
{
  uint8_t *dest = (uint8_t*)dest_;
  const uint8_t *src = (const uint8_t*)src_;
  if (dest == src)
    return;
  if (dest < src) {
    for (size_t i = 0; i < bytes; ++i) {
      dest[i] = src[i];
    }
    return;
  }
  for (int32_t i = bytes - 1; i >= 0; --i) {
    dest[i] = src[i];
  }
}

static void SwapMem(uint8_t* begin, uint8_t* end, uint8_t* dest)
{
  if (begin == dest)
    return;
  uint8_t temp;
  size_t bytes = end - begin;
  for (size_t i = 0; i < bytes; ++i) {
    temp = begin[i];
    begin[i] = dest[i];
    dest[i] = temp;
  }
}

// copies memory from p1 to p2, but then puts memory from p2 to p3.
// |AaaaaBbb--aaa|
//            ^ p1
//       ^ p2 
//          ^ p3
static void MemCpyTriple(uint8_t *p1begin, uint8_t *p1end, uint8_t *p2begin, uint8_t *p2SourceEnd, uint8_t *p3)
{
  // p3 cannot be inside p1 or p2.
  //CCASSERT(p3 >= p1end);
  CCASSERT(p3 <= p1begin); // you cannot output into unread source territory.
  CCASSERT(p3 >= p2SourceEnd);
  CCASSERT(p2begin < p3); // you will overwrite your dest with other dest.
  //for (size_t i = 0; i < n; ++i) {
  uint8_t *p1 = p1begin;
  uint8_t *p2 = p2begin;
  while (true) {
    uint8_t temp = 0;
    if (p2 < p2SourceEnd) {
      temp = *p2;
    }
    else if (p1 > p1end) {
      // both past end; bail.
      return;
    }
    if (p1 < p1end) {
      *p2 = *p1;
    }
    if (p2 < p2SourceEnd) {
      *p3 = temp;
    }
    ++p1;
    ++p2;
    ++p3;
  }
}

#ifdef LOOP_TRACK_CALL_DEPTH
size_t gDepth = 0;
size_t gMaxDepth = 0;
struct DepthCheck
{
  DepthCheck() {
    ++gDepth;
    gMaxDepth = std::max(gMaxDepth, gDepth);
  }
  ~DepthCheck() {
    --gDepth;
  }
};
#else
using DepthCheck = int;
#endif // LOOP_TRACK_CALL_DEPTH

// shift a split circular buffer into place without using some temp buffer. avoids OOM.
// returns the total size of the resulting buffer which will start at segmentBBegin.
// |Bbbb----Aaaa|  => |AaaaBbbb----|
template<size_t tempBufferBytes = 1>
static size_t UnifyCircularBuffer_Left(void* segmentABegin_, void* segmentAEnd_, void* segmentBBegin_, void* segmentBEnd_)
{
  DepthCheck dc;
  uint8_t* segmentABegin = (uint8_t*)segmentABegin_;
  uint8_t* segmentAEnd = (uint8_t*)segmentAEnd_;
  uint8_t* segmentBBegin = (uint8_t*)segmentBBegin_;
  uint8_t* segmentBEnd = (uint8_t*)segmentBEnd_;

  CCASSERT(segmentBEnd >= segmentBBegin);
  CCASSERT(segmentABegin >= segmentBEnd);
  CCASSERT(segmentAEnd >= segmentABegin);

  // |Bbb--Aaaaaaaa| => |AaaaaaaaBbb--|
  // |Bbbbbbbb--Aaa| => |AaaBbbbbbbb--|

  size_t sizeA = segmentAEnd - segmentABegin;
  size_t sizeB = segmentBEnd - segmentBBegin;

  if (sizeA == 0) {
    // nothing to do; B already in place and A doesn't exist.
    return sizeB;
  }

  if (sizeB == 0)
  {
    // slide A into place
    OrderedMemcpy(segmentBBegin, segmentABegin, sizeA);
    return sizeA;
  }

  size_t len = segmentABegin - segmentBBegin;

  if (sizeA >= sizeB) {
    if (len >= sizeA) {
      // there's enough empty space to copy all of A in one shot.
      // |Bbb------Aaaaaaaa|
      // =>
      // |AaaaaaaaBbb------|
      MemCpyTriple(segmentABegin, segmentAEnd, segmentBBegin, segmentBEnd, segmentBBegin + sizeA);
      return sizeA + sizeB;
    }

    // |Bbb--Aaaaaaaaaaa|

    // without a temp buffer, this scenario can get messy fast, recursing for every len bytes, and len could be 1.
    if (tempBufferBytes > len) {
      CCASSERT(len >= sizeB);
      // so the temp buffer can hold entire B.
      uint8_t tempBuffer[tempBufferBytes];
      memcpy(tempBuffer, segmentBBegin, sizeB);
      // shift A over
      OrderedMemcpy(segmentBBegin, segmentABegin, sizeA);
      // and place B.
      memcpy(segmentBBegin + sizeA, tempBuffer, sizeB);
      return sizeA + sizeB;
    }

    // copy a len-sized chunk of A to the front.
    // |Bbb--Aaaaaaaaaaa|
    // =>
    // |AaaaaBbb--aaaaaa|
    SwapMem(segmentABegin, segmentABegin + len, segmentBBegin);

    // recurse because what we have left is a mini version of this buffer.
    // |AaaaaBbb--aaaaaa|
    //      |Bbb--aaaaaa|
    // =>   |aaaaaaBbb--|
    UnifyCircularBuffer_Left<tempBufferBytes>(segmentABegin + len, segmentAEnd, segmentBBegin + len, segmentBBegin + len + sizeB);

    return sizeA + sizeB;
  }

  if (len >= (sizeA + sizeB)) {
    // enough space to make sure nothing overlaps while writing.
    //    |Bbbbbbbb------Aaa|
    // => |AaaBbbbbbbb------|
    OrderedMemcpy(segmentBBegin + sizeA, segmentBBegin, sizeB); // move B right
    OrderedMemcpy(segmentBBegin, segmentABegin, sizeA);// move A into place.
    return sizeA + sizeB;
  }

  // here we have to swap chunks using our own buffer as a temp buffer
  //    |Bbbbbbbb--Aaa|
  // first just get A into place.
  SwapMem(segmentABegin, segmentAEnd, segmentBBegin);

  // => |Aaabbbbb--Bbb|
  // now we need to piece back together B. in fact it's just a mini-version of the big buffer. recurse.
  UnifyCircularBuffer_Left<tempBufferBytes>(segmentABegin, segmentAEnd, segmentBBegin + sizeA, segmentBEnd);
  return sizeA + sizeB;
}

enum class LoopEventType : uint8_t
{
  Nop = 0, // {  } supports long time rests
  NoteOff = 1, // { }
  NoteOn = 2, // { uint8_t note, uint8_t velocity }
  Breath = 3, // { breath }
  Pitch = 4, // { pitch }
  BreathAndPitch = 5, // { breath, pitch }
  SynthPatchChange = 6, // { uint8_t patchid }
  HarmPatchChange = 7, // { uint8_t patchid }
  FullState = 8,
  // reserve some additional CC here.
  // - expr
  // - modwheel
  // - pedal
  // we could also have a couple special flags to mark that we don't need timing info. for example
  // we sample breath / pitch / nop at known intervals. they will always be the same, so don't bother specifying.
};

static const size_t LOOP_EVENT_TYPE_COUNT = 9;

#ifdef EWI_LOOP_USE_MARKER
static const uint8_t LOOP_EVENT_MARKER = 0x6D;
#endif // EWI_LOOP_USE_MARKER

struct LoopEventHeader
{
#ifdef EWI_LOOP_USE_MARKER
  uint8_t mMarker = LOOP_EVENT_MARKER;
#endif // EWI_LOOP_USE_MARKER
  LoopEventType mEventType;// : LOOPEVENTTYPE_BITS;
  LOOP_EVENT_DELAY mTimeSinceLastEventMS;// : LOOPEVENTTIME_BITS;
};

//static constexpr size_t LOOP_EVENT_HEADER_SIZE = sizeof(LoopEventHeader);

struct LoopEvent_NoteOnParams
{
  uint8_t mMidiNote;
  uint8_t mVelocity;
};

struct LoopEvent_BreathParams
{
  uint16_t mBreath01;
};

struct LoopEvent_PitchParams
{
  uint16_t mPitchN11;
};

struct LoopEvent_BreathAndPitchParams
{
  uint16_t mBreath01;
  uint16_t mPitchN11;
};

struct LoopEvent_SynthPatchChangeParams
{
  int16_t mSynthPatchId;
};

struct LoopEvent_HarmPatchChangeParams
{
  int16_t mHarmPatchId;
};

struct LoopEvent_FullStateParams
{
  int16_t mBreath01;
  int16_t mPitchN11;
  //bool mIsNoteCurrentlyOn; <-- implicit from note/velocity.
  int16_t mSynthPatchId;
  int16_t mHarmPatchId;
  uint8_t mMidiNote;
  uint8_t mVelocity;

  LoopEvent_FullStateParams() = default;

  LoopEvent_FullStateParams(const MusicalVoice& mv) :
    mBreath01(mv.mBreath01.GetIntVal()),
    mPitchN11(mv.mPitchBendN11.GetIntVal()),
    //mIsNoteCurrentlyOn(mv.mIsNoteCurrentlyOn),
    mSynthPatchId(mv.mSynthPatch),
    //mIsNoteCurrentlyMuted(mv.mIsNoteCurrentlyMuted),
    mHarmPatchId(mv.mHarmPatch),
    mMidiNote(mv.mMidiNote),
    mVelocity(mv.mVelocity)
  {
    if (mv.mIsNoteCurrentlyMuted || !mv.mIsNoteCurrentlyOn) {
      mMidiNote = 0;
      mVelocity = 0;
    }
  }
  String ToString() const
  {
    return String(
      "b:") + mBreath01 +
      ", p:" + mPitchN11 +
      ", s:" + mSynthPatchId +
      ", h:" + mHarmPatchId +
      ", note:" + mMidiNote + ",v:" + mVelocity
      ;
  }
  void ApplyToVoice(MusicalVoice& mv) const
  {
    mv.mBreath01.SetInt(mBreath01);
    mv.mPitchBendN11.SetInt(mPitchN11);
    mv.mSynthPatch = mSynthPatchId;
    mv.mHarmPatch = mHarmPatchId;
    mv.mIsNoteCurrentlyOn = !!mMidiNote && !!mVelocity;
    mv.mIsNoteCurrentlyMuted = false;// mIsNoteCurrentlyMuted;
    mv.mMidiNote = mMidiNote;
    mv.mVelocity = mVelocity;
  }
};

// NOT 100% stream-friendly layout.
struct LoopEvent_Unified
{
  LoopEventHeader mHeader;
  union {
    LoopEvent_NoteOnParams mNoteOnParams;
    LoopEvent_BreathParams mBreathParams;
    LoopEvent_PitchParams mPitchParams;
    LoopEvent_BreathAndPitchParams mBreathAndPitchParams;
    LoopEvent_SynthPatchChangeParams mSynthPatchChangeParams;
    LoopEvent_HarmPatchChangeParams mHarmPatchChangeParams;
    LoopEvent_FullStateParams mFullStateParams;
  } mParams;

  uint32_t mLoopTimeMS; // debugging convenience
  LoopEventHeader *mP; // debugging convenience
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
String LoopBreathAndPitchParamToString(const LoopEvent_Unified& e) { return String("b:") + e.mParams.mBreathAndPitchParams.mBreath01 + ", p:" + e.mParams.mBreathAndPitchParams.mPitchN11; }
String LoopSynthPatchChangeParamToString(const LoopEvent_Unified& e) { return String(e.mParams.mSynthPatchChangeParams.mSynthPatchId); }
String LoopHarmPatchChangeParamToString(const LoopEvent_Unified& e) { return String(e.mParams.mHarmPatchChangeParams.mHarmPatchId); }
String LoopFullStateParamToString(const LoopEvent_Unified& e) { return e.mParams.mFullStateParams.ToString(); }

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
  { LoopEventType::FullState, "FullState", sizeof(LoopEvent_FullStateParams), LoopFullStateParamToString },
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

// |---PxxxxxxxxxE (pe)
// |xxE---Pxxxxxxx (ep)
enum class LayoutSituation
{
  PE,
  EP,
};

struct LoopCursor
{
  LoopEventHeader* mP = nullptr;
  // the time AT the cursor. the cursor points to an event which has a delay
  // while PeekLoopTime() tells you when the current event would happen
  // careful to use these correctly.
  uint32_t mLoopTimeMS = 0;
  MusicalVoice mRunningVoice;

  void Set(void *p, uint32_t loopTime, const MusicalVoice& mv) {
    mP = (LoopEventHeader*)p;
    mLoopTimeMS = loopTime;
    mRunningVoice.AssignFromLoopStream(mv);
  }

  void Assign(const LoopCursor& rhs) {
    mP = rhs.mP;
    mLoopTimeMS = rhs.mLoopTimeMS;
    mRunningVoice.AssignFromLoopStream(rhs.mRunningVoice);
  }

  void SetNull() {
    mP = nullptr;
    mLoopTimeMS = 0;
  }

  // returns the absolute loop time that the currently-pointed-to-event should occur.
  // NOTE: can be past the loop duration!
  uint32_t PeekLoopTimeNotNormalized()
  {
#ifdef EWI_LOOP_USE_MARKER
    CCASSERT(mP->mMarker == LOOP_EVENT_MARKER);
#endif // EWI_LOOP_USE_MARKER
    return mLoopTimeMS + mP->mTimeSinceLastEventMS;
  }

  // you can have an event at the end of the buffer which has a delay which would loop back the loop time.
  // this will account for this and return a value within the loop.
  uint32_t PeekLoopTimeWrapSensitive(const LoopStatus& status)
  {
    CCASSERT(status.mState == LooperState::DurationSet);
    uint32_t ret = PeekLoopTimeNotNormalized();
    while (ret >= status.mLoopDurationMS)
      ret -= status.mLoopDurationMS;
    return ret;
  }

  // does this event contain the given loop time? if our event starts at L-50 with duration 150, then 0 is contained for example.
  bool TouchesTime(const LoopStatus& status, uint32_t r) {
    uint32_t dnEventTimeWithDelay = mLoopTimeMS + mP->mTimeSinceLastEventMS;
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

  // return true if we wrapped in the buffer (not time though!).
  bool MoveNextInternal(const LoopStatus& status, const LoopCursor& bufferBegin, void* bufferEnd)
  {
    mLoopTimeMS += mP->mTimeSinceLastEventMS;
    mP = AddPointerBytes(mP, sizeof(LoopEventHeader) + GetLoopEventTypeItemInfo(mP->mEventType).mParamsSize);
    if (status.mState == LooperState::DurationSet) {
      if (mLoopTimeMS >= status.mLoopDurationMS)
        mLoopTimeMS -= status.mLoopDurationMS;
    }
    if (mP >= bufferEnd) {
      Assign(bufferBegin);
      return true;
    }
    return false;
  }

  // read next event in the stream and integrate to outp.
  // advances cursors.
  // returns true if the cursor wrapped.
  bool ConsumeSingleEvent(const LoopStatus& status, const LoopCursor& bufferBegin, void* bufferEnd, MusicalVoice* additionalVoice = nullptr)
  {
    LoopEvent_Unified& event = *((LoopEvent_Unified*)mP);

    // take various actions based on stream, and advance cursor again if needed.
    switch (event.mHeader.mEventType) {
    case LoopEventType::Nop: // {  } supports long time rests
      break;
    case LoopEventType::NoteOff: // { }
      mRunningVoice.mNeedsNoteOff = true;
      mRunningVoice.mIsNoteCurrentlyOn = false;
      mRunningVoice.mNoteOffNote = mRunningVoice.mNoteOffNote;
      if (additionalVoice) {
        additionalVoice->mNeedsNoteOff = true;
        additionalVoice->mIsNoteCurrentlyOn = false;
        additionalVoice->mNoteOffNote = mRunningVoice.mNoteOffNote;
      }
      break;
    case LoopEventType::NoteOn: // { uint8_t note, uint8_t velocity }
      mRunningVoice.mNeedsNoteOn = true;
      mRunningVoice.mMidiNote = event.mParams.mNoteOnParams.mMidiNote;
      mRunningVoice.mVelocity = event.mParams.mNoteOnParams.mVelocity;
      if (additionalVoice) {
        additionalVoice->mNeedsNoteOn = true;
        additionalVoice->mMidiNote = event.mParams.mNoteOnParams.mMidiNote;
        additionalVoice->mVelocity = event.mParams.mNoteOnParams.mVelocity;
      }
      break;
    case LoopEventType::Breath: // { breath }
      mRunningVoice.mBreath01.SetInt(event.mParams.mBreathParams.mBreath01);
      if (additionalVoice) {
        additionalVoice->mBreath01.SetInt(event.mParams.mBreathParams.mBreath01);
      }
      break;
    case LoopEventType::Pitch: // { float pitch }
      mRunningVoice.mPitchBendN11.SetInt(event.mParams.mPitchParams.mPitchN11);
      if (additionalVoice) {
        additionalVoice->mPitchBendN11.SetInt(event.mParams.mPitchParams.mPitchN11);
      }
      break;
    case LoopEventType::BreathAndPitch: // { float breath, float pitch }
      mRunningVoice.mBreath01.SetInt(event.mParams.mBreathAndPitchParams.mBreath01);
      mRunningVoice.mPitchBendN11.SetInt(event.mParams.mBreathAndPitchParams.mPitchN11);
      if (additionalVoice) {
        additionalVoice->mBreath01.SetInt(event.mParams.mBreathAndPitchParams.mBreath01);
        additionalVoice->mPitchBendN11.SetInt(event.mParams.mBreathAndPitchParams.mPitchN11);
      }
      break;
    case LoopEventType::SynthPatchChange: // { uint8_t patchid }
      mRunningVoice.mSynthPatch = event.mParams.mSynthPatchChangeParams.mSynthPatchId;
      if (additionalVoice) {
        additionalVoice->mSynthPatch = event.mParams.mSynthPatchChangeParams.mSynthPatchId;
      }
      break;
    case LoopEventType::HarmPatchChange: // { uint8_t patchid }
      mRunningVoice.mHarmPatch = event.mParams.mHarmPatchChangeParams.mHarmPatchId;
      if (additionalVoice) {
        additionalVoice->mHarmPatch = event.mParams.mHarmPatchChangeParams.mHarmPatchId;
      }
      break;
    case LoopEventType::FullState:
      event.mParams.mFullStateParams.ApplyToVoice(mRunningVoice);
      if (additionalVoice) {
        event.mParams.mFullStateParams.ApplyToVoice(*additionalVoice);
      }
      break;
    default:
      CCASSERT(false);
      break;
    }
    return MoveNextInternal(status, bufferBegin, bufferEnd);
  }

  LoopEvent_Unified DebugPeekEvent() {
#ifdef EWI_LOOP_USE_MARKER
    CCASSERT(mP->mMarker == LOOP_EVENT_MARKER);
#endif // EWI_LOOP_USE_MARKER
    LoopEvent_Unified ret;// = { 0 };
    memcpy(&ret, mP, sizeof(LoopEventHeader) + GetLoopEventTypeItemInfo(mP->mEventType).mParamsSize);
    ret.mP = mP;
    ret.mLoopTimeMS = mLoopTimeMS;
    return ret;
  }

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct LoopEventStream
{
  bool mIsPlaying = false;// Mute function
  bool mOOM = false; // Out of memory condition. Set this flag to stop recording.
  bool mIsRecording = false; // necessary for checking cursor conditions & states.

  LoopCursor mBufferBegin;
  void* mBufferEnd = nullptr;
  void* mEventsValidEnd = nullptr;// when writing, this can be before the end of the raw buffer.

  // for writing, points to free area for writing (don't read).
  // for reading, points to the next unread event
  LoopCursor mCursor;

  // if you're writing into the stream, and you pass the loop length, then we start tracking a complete loop worth of events.
  // this way when we stop recording, we can accurately piece together a complete loop of events in sequence.
  LoopCursor mPrevCursor;

#ifndef EWI_UNIT_TESTS
  CCThrottlerT<LOOP_BREATH_PITCH_RESOLUTION_MS> mBreathPitchThrottler;
#endif // EWI_UNIT_TESTS

  const LoopStatus* mpStatus = nullptr;
  const LoopStatus& GetStatus() const { return *mpStatus; }

  bool IsEmpty() const {
    return mBufferBegin.mP == mEventsValidEnd;
  }

  uint32_t Duration(uint32_t begin, uint32_t end) {
    if ((GetStatus().mState == LooperState::DurationSet) && (end < begin)) {
      end += GetStatus().mLoopDurationMS;
    }
    CCASSERT(end >= begin);
    return end - begin;
  }

  // returns # of events read.
  size_t ReadUntilLoopTime(MusicalVoice& outp)
  {
    if (mOOM)
      return 0; // there are ways to sorta work with a OOM situation, but it's not necessary to add the complexity for a shoddy solution.
    if (!mIsPlaying)
      return 0; // muted.
    CCASSERT(!!mCursor.mP);
    if (mBufferBegin.mP == mEventsValidEnd)
      return 0;

    size_t ret = 0;

    // assumption: current cursor is AT or BEFORE the requested time.
    // so if the next event would be TOO far, stop reading. otherwise just go.
    uint32_t reqTime = GetStatus().mCurrentLoopTimeMS;

    // there are no gaps. so we should *encounter* the cursor time.
    while (!mCursor.TouchesTime(GetStatus(), reqTime)) {
      mCursor.ConsumeSingleEvent(GetStatus(), mBufferBegin, mEventsValidEnd, &outp);
      ++ret;
    }

    // if this event is after reqtime, bail. we know it's touching reqtime too, so
    // the only passing condition is if r is exactly equal to the event time post-delay
    // r-------| <-- only pass if dly=0
    // |---r---| <-- dly
    // |-------r <-- r is exactly on the event time. also pass.
    uint32_t t = mCursor.PeekLoopTimeWrapSensitive(GetStatus());
    if (t == reqTime) {
      while (mCursor.PeekLoopTimeWrapSensitive(GetStatus()) == t) {
        mCursor.ConsumeSingleEvent(GetStatus(), mBufferBegin, mEventsValidEnd, &outp);
        ++ret;
      }
    }


    return ret;
  }

  void StartRecording(const LoopStatus& status, const MusicalVoice& musicalStatus, void* const begin, void* const end)
  {
    mIsPlaying = false;
    mIsRecording = true;
    mpStatus = &status;
    mBufferBegin.Set(begin, status.mCurrentLoopTimeMS, musicalStatus);
    mEventsValidEnd = begin;
    mPrevCursor.Assign(mBufferBegin);
    mBufferEnd = end;
    mCursor.Assign(mBufferBegin);

    LoopEvent_FullStateParams params(musicalStatus);
    WriteEventRaw(LoopEventType::FullState, params);
  }

  LayoutSituation GetLayoutSituation() const
  {
    if (mPrevCursor.mP <= mCursor.mP) {
      return LayoutSituation::PE;
    }
    return LayoutSituation::EP;
  }

#ifdef EWI_UNIT_TESTS

  void Dump()
  {
    cc::log("----");
    auto events = DebugGetStream();
    auto layout = mIsRecording ? GetLayoutSituation() : LayoutSituation::PE;

    if (mIsRecording) {
      switch (layout) {
      case LayoutSituation::PE:// |---PxxxxZxxxxE (pze)
        cc::log("|---PxxxxxxxxxE (PE)");
        break;
      case LayoutSituation::EP:// |xxE---PxxxxZxx (epz)
        cc::log("|xxE---Pxxxxxxx (EP)");
        break;
      }

      if (layout == LayoutSituation::PE) {
        cc::log("   > { %p pe padding %d bytes }", mBufferBegin.mP, PointerDistanceBytes(mPrevCursor.mP, mBufferBegin.mP));
      }
    }
    else {
      cc::log("not recording; layout not relevant.");
    }

    bool encounteredPadding = false;
    for (auto& e : events) {
      int loopTime = e.mLoopTimeMS;

      if (mIsRecording && !encounteredPadding && (e.mP >= mCursor.mP)) {
        switch (layout) {
        case LayoutSituation::EP:
          encounteredPadding = true;
          cc::log(" E > { %p t=%d EP padding %d bytes }", mCursor.mP, mCursor.mLoopTimeMS, PointerDistanceBytes(mPrevCursor.mP, mCursor.mP));
          break;
        }
      }

#ifdef EWI_LOOP_USE_MARKER
      CCASSERT(e.mHeader.mMarker == LOOP_EVENT_MARKER);
#endif // EWI_LOOP_USE_MARKER
      String sParams = GetLoopEventTypeItemInfo(e.mHeader.mEventType).mParamsToString(e);
      cc::log("%s%s%s [%p (+%d) t=%d: dly=%d, type=%s, params=%s]",
        e.mP == mBufferBegin.mP ? "B" : " ",
        e.mP == mCursor.mP ? "E" : " ",
        e.mP == mPrevCursor.mP ? "P" : " ",
        e.mP,
        PointerDistanceBytes(e.mP, mBufferBegin.mP),
        loopTime,
        (int)e.mHeader.mTimeSinceLastEventMS,
        GetLoopEventTypeItemInfo(e.mHeader.mEventType).mName,
        sParams.mStr.str().c_str());
    } // for()

    if (mCursor.mP == mEventsValidEnd) {
      cc::log(" E  [%p (+%d) t=%d <eof> ]",
        mCursor.mP,
        PointerDistanceBytes(mCursor.mP, mBufferBegin.mP),
        mCursor.mLoopTimeMS
        );
    }
    cc::log("   > { %p VE padding %d bytes } <", mEventsValidEnd, PointerDistanceBytes(mEventsValidEnd, mBufferEnd));
    cc::log("   > { %p end } <", mBufferEnd);
    cc::log("begin-valid: %d, event count: %d %s",
      PointerDistanceBytes(mEventsValidEnd, mBufferBegin.mP),
      events.size(),
      mOOM ? "OOM!" : ""
      );
    if (mBufferBegin.mP) {
      cc::log("B state {%s}", LoopEvent_FullStateParams(mBufferBegin.mRunningVoice).ToString().mStr.str().data());
    }
    if (mPrevCursor.mP) {
      cc::log("P state {%s}", LoopEvent_FullStateParams(mPrevCursor.mRunningVoice).ToString().mStr.str().data());
    }
    if (mCursor.mP) {
      cc::log("E state {%s}", LoopEvent_FullStateParams(mCursor.mRunningVoice).ToString().mStr.str().data());
    }
  }

  // even here we must be sensitive to scenarios.
  std::vector<LoopEvent_Unified> DebugGetStream() const
  {
    std::vector<LoopEvent_Unified> ret;
    if (!mIsRecording) {
      LoopCursor c; c.Assign(mBufferBegin);
      while (c.mP < mEventsValidEnd) {
        ret.push_back(c.DebugPeekEvent());
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
        ret.push_back(c.DebugPeekEvent());
        if (c.ConsumeSingleEvent(GetStatus(), mBufferBegin, mBufferEnd))
          break;
      };
      return ret;
    }
    case LayoutSituation::EP:// |xxE---PxxxxZxx (ep)
    {
      LoopCursor c; c.Assign(mBufferBegin);
      while (c.mP < mCursor.mP) {
        ret.push_back(c.DebugPeekEvent());
        if (c.ConsumeSingleEvent(GetStatus(), mBufferBegin, mBufferEnd))
          break;
      };
      c.Assign(mPrevCursor);
      while (c.mP < mEventsValidEnd) {
        ret.push_back(c.DebugPeekEvent());
        if (c.ConsumeSingleEvent(GetStatus(), mBufferBegin, mBufferEnd))
          break;
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
  void* WriteSeamNops(void* p_, uint32_t duration) {
    uint8_t *p = (uint8_t*)p_;
    if (mOOM)
      return p;
    size_t fullNops;
    LOOP_EVENT_DELAY remainderTime;
    DivRemBitwise<LOOP_EVENT_DELAY_BITS>(duration, fullNops, remainderTime);
    size_t bytesNeeded = fullNops + (remainderTime ? 1 : 0);
    bytesNeeded *= sizeof(LoopEventHeader);
    if (p + bytesNeeded >= mBufferEnd) {
      mOOM = true;
      return p; // OOM
    }
    LoopEventHeader* ph = (LoopEventHeader*)p;
    for (size_t i = 0; i < fullNops; ++i) {
#ifdef EWI_LOOP_USE_MARKER
      ph->mMarker = LOOP_EVENT_MARKER;
#endif // EWI_LOOP_USE_MARKER
      ph->mEventType = LoopEventType::Nop;
      ph->mTimeSinceLastEventMS = LOOP_EVENT_DELAY_MAX;
      ++ph;
    }
    if (remainderTime) {
#ifdef EWI_LOOP_USE_MARKER
      ph->mMarker = LOOP_EVENT_MARKER;
#endif // EWI_LOOP_USE_MARKER
      ph->mEventType = LoopEventType::Nop;
      ph->mTimeSinceLastEventMS = remainderTime;
      ++ph;
    }

    CCASSERT(GetStatus().mState == LooperState::DurationSet);
    return (uint8_t*)ph;
  }

  // return the end of the used buffer.
  // point cursor at the time in status.
  // we want the resulting buffer to start at the earliest known material, and end at mCursor.
  void* WrapUpRecording() {
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
    void* segmentABegin = mPrevCursor.mP;
    void* segmentAEnd = mEventsValidEnd;
    void* segmentBBegin = mBufferBegin.mP;
    void* segmentBEnd = mBufferBegin.mP;// |---PxxxxZxxxxE (pe)

    size_t mainBufferValidBytes = PointerDistanceBytes(segmentAEnd, segmentABegin);

    if (GetLayoutSituation() == LayoutSituation::EP) {// |xxE---PxxxxZxx (ep)
      segmentBEnd = mCursor.mP;
      mainBufferValidBytes += PointerDistanceBytes(segmentBEnd, segmentBBegin);
    }

    //if (mPrevCursor.mP->mEventType != LoopEventType::FullState)
    LoopEventHeader hdr;
    LoopEvent_FullStateParams params(mPrevCursor.mRunningVoice);

    // check for OOM.
    size_t bytesNeeded = sizeof(hdr) + sizeof(params);
    if (mainBufferValidBytes + bytesNeeded > PointerDistanceBytes(mBufferEnd, mBufferBegin.mP))
    {
      mOOM = true;
      mIsPlaying = true;
      mIsRecording = false;
      mPrevCursor.SetNull(); // no longer valid.
      mEventsValidEnd = mBufferBegin.mP;
      mBufferEnd = mEventsValidEnd;
      return mEventsValidEnd;
    }

    hdr.mEventType = LoopEventType::FullState;
#ifdef EWI_LOOP_USE_MARKER
    hdr.mMarker = LOOP_EVENT_MARKER;
#endif // EWI_LOOP_USE_MARKER
    hdr.mTimeSinceLastEventMS = mPrevCursor.mP->mTimeSinceLastEventMS; // easy to insert an event; just steal the other time and set its to 0.
    mPrevCursor.mP->mTimeSinceLastEventMS = 0;

    mBufferBegin.mLoopTimeMS = mPrevCursor.mLoopTimeMS;
    mBufferBegin.mRunningVoice.AssignFromLoopStream(mPrevCursor.mRunningVoice);

    // get our buffer conjoined.
    UnifyCircularBuffer_Left<LOOPER_TEMP_BUFFER_BYTES>(segmentABegin, segmentAEnd, segmentBBegin, segmentBEnd);
    // slide it over to make room
    OrderedMemcpy(AddPointerBytes(mBufferBegin.mP, bytesNeeded), segmentBBegin, mainBufferValidBytes);
    // write our full state
    LoopEventHeader* phdr = mBufferBegin.mP;
    *phdr = hdr;
    ++phdr;
    LoopEvent_FullStateParams* pparams = (LoopEvent_FullStateParams*)phdr;
    *pparams = params;
    ++pparams;

    mEventsValidEnd = AddPointerBytes(pparams, mainBufferValidBytes);

    // write a "seam" at the end of the buffer to make our recorded material exactly 1 loop in duration.
    mEventsValidEnd = WriteSeamNops(mEventsValidEnd, GetStatus().mLoopDurationMS - recordedDuration);

    mBufferEnd = mEventsValidEnd;
    mPrevCursor.SetNull(); // no longer valid.
    mIsPlaying = true;
    mIsRecording = false;
    mCursor.Assign(mBufferBegin);
    return mEventsValidEnd;
  }

  // return true on success.
  bool FindMemoryOrOOM(size_t bytesNeeded)
  {
    // ZE, PZE check for end of buffer
    // EPZ and ZEP check f or P
    switch (GetLayoutSituation()) {
    case LayoutSituation::PE:// |---PxxxxZxxxxE (pze)
      if (AddPointerBytes(mCursor.mP, bytesNeeded) < mBufferEnd) {
        return true;// we're already set.
      }
      // uh oh. wrap buffer?
      if (AddPointerBytes(mBufferBegin.mP, bytesNeeded) >= mPrevCursor.mP) {
        // too small; we would eat into our own tail. OOM.
        mOOM = true;
        return false;
      }
      // safe to use beginning of buffer, transition to EPZ
      mCursor.mP = mBufferBegin.mP;
      mBufferBegin.mLoopTimeMS = mCursor.mLoopTimeMS;
      Dump();
      return true;
    case LayoutSituation::EP:// |xxE---PxxxxZxx (epz)
      if (AddPointerBytes(mCursor.mP, bytesNeeded) < mPrevCursor.mP) {
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
  bool WriteEventRaw(LoopEventType eventType, const Tparams& eventParams)
  {
    const void* params = &eventParams;
    if (mOOM)
      return false;
    const auto& eventInfo = GetLoopEventTypeItemInfo(eventType);

    if (GetStatus().mState == LooperState::DurationSet) {
      CCASSERT(GetStatus().mCurrentLoopTimeMS < GetStatus().mLoopDurationMS);
    }

    // not "has looped". we want to know if you've passed rec start time.
    uint32_t loopTimeElapsedSinceLastWrite = Duration(mCursor.mLoopTimeMS, GetStatus().mCurrentLoopTimeMS);

    // this is a useful calculation to do NOW, because we know it's always less than 1 loop duration in length which simplifies math.
    int32_t peMS = (int32_t)Duration(mPrevCursor.mLoopTimeMS, mCursor.mLoopTimeMS);
    peMS += loopTimeElapsedSinceLastWrite; // and now it can be longer than a loop. handle this scenario later.

    // calculate a boundary to rec time, not 0.
    uint32_t delayTime = loopTimeElapsedSinceLastWrite;// -perfectLoopFillerTime;
    size_t fullNops;// = delayTime / LOOP_EVENT_DELAY_MAX;
    LOOP_EVENT_DELAY eventDelayTime;// = delayTime - (fullNops * LOOP_EVENT_DELAY_MAX);
    DivRemBitwise<LOOP_EVENT_DELAY_BITS>(delayTime, fullNops, eventDelayTime);
    if (eventDelayTime == 0 && fullNops > 0) {
      --fullNops;
      eventDelayTime = LOOP_EVENT_DELAY_MAX;
    }
    size_t bytesNeeded = (sizeof(LoopEventHeader) * (fullNops + 1)) + eventInfo.mParamsSize;// +1 because THIS event has a header too.

    // CHECK for OOM, deal with wrapping buffer.
    if (!FindMemoryOrOOM(bytesNeeded)) {
      return false;
    }

    // Write nops.
    LoopEventHeader* ph = (LoopEventHeader*)(mCursor.mP);

#ifdef EWI_LOOP_USE_MARKER
#define WRITE_LOOP_EVENT_HEADER_MARKER ph->mMarker = LOOP_EVENT_MARKER
#else
#define WRITE_LOOP_EVENT_HEADER_MARKER
#endif // EWI_LOOP_USE_MARKER

    for (size_t i = 0; i < fullNops; ++i) {
      WRITE_LOOP_EVENT_HEADER_MARKER;
      ph->mEventType = LoopEventType::Nop;
      ph->mTimeSinceLastEventMS = LOOP_EVENT_DELAY_MAX;
      ++ph;
    }

    WRITE_LOOP_EVENT_HEADER_MARKER;
    ph->mEventType = eventType;
    ph->mTimeSinceLastEventMS = eventDelayTime;
    ++ph;
    uint8_t* pp = (uint8_t*)ph;
    if (eventInfo.mParamsSize) {
      memcpy(pp, params, eventInfo.mParamsSize);
      pp += eventInfo.mParamsSize;
    }

    mCursor.mLoopTimeMS = GetStatus().mCurrentLoopTimeMS;
    mCursor.mP = (LoopEventHeader*)pp;
    mEventsValidEnd = std::max(mEventsValidEnd, (void*)mCursor.mP); // one of the only times you actually need to cast to void*

    // advance the "prev" write cursor if it exists. do this before checking the OOB conditions below, to make a bit of room and squeeze out a few more bytes.
    if (GetStatus().mState == LooperState::DurationSet) {
      // fast-forward P so it's just under 1 loop's worth of material.
      // it's not necessary to support a full loop because you'd end up with 2 versions of "state" at the loop point (beginning + end).
      while (peMS >= (int32_t)GetStatus().mLoopDurationMS) {
        uint32_t less = mPrevCursor.mP->mTimeSinceLastEventMS;
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
    CCASSERT(!!mBufferBegin.mP);

#ifndef EWI_UNIT_TESTS
    if (mBreathPitchThrottler.IsReady()) {
#else
    if (true) {
#endif // EWI_UNIT_TESTS
      bool breathChanged = mCursor.mRunningVoice.mBreath01 != liveVoice.mBreath01;// abs(mCursor.mRunningVoice.mBreath01 - liveVoice.mBreath01) > LOOP_BREATH_PITCH_EPSILON;
      bool pitchChanged = mCursor.mRunningVoice.mPitchBendN11 != liveVoice.mPitchBendN11;// abs(mCursor.mRunningVoice.mPitchBendN11 - liveVoice.mPitchBendN11) > LOOP_BREATH_PITCH_EPSILON;
      if (breathChanged && pitchChanged) {
        mCursor.mRunningVoice.mBreath01 = liveVoice.mBreath01;
        mCursor.mRunningVoice.mPitchBendN11 = liveVoice.mPitchBendN11;
        WriteEventRaw(LoopEventType::BreathAndPitch, LoopEvent_BreathAndPitchParams{ mCursor.mRunningVoice.mBreath01.GetIntVal(), mCursor.mRunningVoice.mPitchBendN11.GetIntVal() });
      }
      else if (breathChanged) {
        mCursor.mRunningVoice.mBreath01 = liveVoice.mBreath01;
        WriteEventRaw(LoopEventType::Breath, LoopEvent_BreathParams{ mCursor.mRunningVoice.mBreath01.GetIntVal() });
      }
      else if (pitchChanged) {
        mCursor.mRunningVoice.mPitchBendN11 = liveVoice.mPitchBendN11;
        WriteEventRaw(LoopEventType::Pitch, LoopEvent_PitchParams{ mCursor.mRunningVoice.mPitchBendN11.GetIntVal() });
      }
    }

    if (liveVoice.mNeedsNoteOn) {
      mCursor.mRunningVoice.mIsNoteCurrentlyOn = true;
      mCursor.mRunningVoice.mMidiNote = liveVoice.mMidiNote;
      mCursor.mRunningVoice.mVelocity = liveVoice.mVelocity;
      WriteEventRaw(LoopEventType::NoteOn, LoopEvent_NoteOnParams{ liveVoice.mMidiNote, liveVoice.mVelocity });
    }

    if (liveVoice.mNeedsNoteOff) {
      mCursor.mRunningVoice.mIsNoteCurrentlyOn = false;
      WriteEventRaw(LoopEventType::NoteOff, nullptr);
    }

    // capture alteration events before note on
    if (liveVoice.mSynthPatch != mCursor.mRunningVoice.mSynthPatch) {
      mCursor.mRunningVoice.mSynthPatch = liveVoice.mSynthPatch;
      WriteEventRaw(LoopEventType::SynthPatchChange, LoopEvent_SynthPatchChangeParams{ mCursor.mRunningVoice.mSynthPatch });
    }

    if (liveVoice.mHarmPatch != mCursor.mRunningVoice.mHarmPatch) {
      mCursor.mRunningVoice.mHarmPatch = liveVoice.mHarmPatch;
      WriteEventRaw(LoopEventType::HarmPatchChange, LoopEvent_HarmPatchChangeParams{ mCursor.mRunningVoice.mHarmPatch });
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
  void LoopIt(const MusicalVoice& mv)
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
      mLayers[0].StartRecording(mStatus, mv, mBuffer, EndPtr(mBuffer));
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
        void* buf = mLayers[mCurrentlyWritingLayer].WrapUpRecording();
        mLayers[mCurrentlyWritingLayer].mIsPlaying = true;

        mCurrentlyWritingLayer++; // can go out of bounds!
        if (mCurrentlyWritingLayer < SizeofStaticArray(mLayers)) {
          mLayers[mCurrentlyWritingLayer].StartRecording(mStatus, mv, buf, EndPtr(mBuffer));// prepare the next layer for recording.
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

    int poly = (int)(pout - outp);
    cc::log("cv:%d state:%d poly:%d %s", mCurrentlyWritingLayer, (int)(this->mStatus.mState), poly, "oom");

    return pout - outp;
  }
};

#pragma pack(pop)
