
#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/scale_follower/ScaleFollower.hpp>
#include <clarinoid/harmonizer/harmonizer.hpp>


using event_delay_t = uint8_t;
static constexpr size_t LOOP_EVENT_DELAY_BITS = sizeof(event_delay_t) * 8;
static constexpr uint16_t LOOP_EVENT_DELAY_MAX = std::numeric_limits<event_delay_t>::max();


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

// event params have a 2-byte header (sorta; part can be used for params)
// delay 8 bits
// type 4 bits
// additional 4 bits. some events this is thrown away, some it's the high-ish bits of a 12-bit param.
void LoopEvent_ReadHeader(Ptr& ptr, event_delay_t& delayMS, LoopEventType& type, uint8_t& byte2)
{
  ptr.Read(delayMS);
  ptr.Read(byte2);
  type = (LoopEventType)(byte2 >> 4);// high 4 bits = type
}
// after reading the header, if you need the 12-bit param, this does it.
void LoopEvent_Construct12BitParam(Ptr& ptr, uint8_t byte2, uint16_t& param)
{
  uint8_t byte3;
  ptr.Read(byte3);
  param = (uint16_t)byte3 | (((uint16_t)byte2 & 0xf) << 8); // 2222xxxxxxxxxxxx
}
void LoopEvent_WriteHeaderAnd12BitParam(Ptr& ptr, event_delay_t delayMS, LoopEventType type, uint16_t param)
{
  CCASSERT(!(param & 0xf000));
  ptr.Write(delayMS);
  uint8_t byte2 = ((uint8_t)type << 4) | (uint8_t)(param >> 8); // high 4 bits of param into low 4 bits of byte2.
  uint8_t byte3 = (uint8_t)(param & 0xff);
  ptr.Write(byte2);
  ptr.Write(byte3);
}
void LoopEvent_WriteTwo12BitValues(Ptr& p, uint16_t val1, uint16_t val2)
{
  CCASSERT(!(val1 & 0xf000));
  CCASSERT(!(val2 & 0xf000));
  uint8_t bytes[3] = {
    (uint8_t)((val1 & (uint16_t)0x0ff0) >> 4), // high 8-bits of val1
    (uint8_t)((uint8_t)(((val1 & 0xf) << 4)) | ((uint8_t)((val2 & (uint16_t)0xf00) >> 8))), // low 4 bits of val1 + high 4 bits of val2
    (uint8_t)(val2 & (uint16_t)0x00ff)// low 8 bits of val2
  };
  p.WriteArray(bytes);
}
void LoopEvent_ReadTwo12BitValues(Ptr& p, uint16_t& val1, uint16_t& val2)
{
  uint8_t bytes[3];
  p.ReadArray(bytes);
  val1 = (((uint16_t)bytes[0]) << 4) | ((bytes[1] & 0xf0) >> 4);
  val2 = (((uint16_t)(bytes[1] & 0x0f)) << 8) | bytes[2];
}
// a header with no params has 4 bytes dangling. No params fills them with 0s.
void LoopEvent_WriteHeaderNoParams(Ptr& ptr, event_delay_t delayMS, LoopEventType type)
{
  ptr.Write(delayMS);
  uint8_t byte2 = ((uint8_t)type << 4);
  ptr.Write(byte2);
}
void LoopEvent_SurgicallyWriteDelayInPlace(const Ptr& ptr, event_delay_t delayMS)
{
  uint8_t* p = (uint8_t*)ptr.mP;
  p[0] = delayMS;// thankfully this is simple.
}
event_delay_t LoopEvent_PeekDelay(const Ptr& p) {
  return p.Peek<uint8_t>();
}


// THESE ARE NOT STREAM-PACKED! Reading & writing are done byte-wise.
// we put delay first in the packet so the 4-bit event type butts against another param. reduces # of bitsplits
struct LoopEvent_Nop
{
  static constexpr size_t PayloadSizeBytes = 2;
  static constexpr LoopEventType EventType = LoopEventType::Nop;
  static constexpr const char *Name = "Nop";

  static void Write(Ptr& p, event_delay_t delay) {
    LoopEvent_WriteHeaderNoParams(p, delay, LoopEventType::Nop);
  }
  String ToString() { return String(); }
};

struct LoopEvent_NoteOff
{
  static constexpr size_t PayloadSizeBytes = 2;
  static constexpr LoopEventType EventType = LoopEventType::NoteOff;
  static constexpr const char *Name = "NoteOff";

  LoopEvent_NoteOff() = default;

  void Write(Ptr& p, event_delay_t delay) const {
    LoopEvent_WriteHeaderNoParams(p, delay, LoopEventType::NoteOff);
  }
  void ApplyToVoice(MusicalVoice& mv) const
  {
    mv.mMidiNote = 0;
    mv.mVelocity = 0;
  }
  String ToString() { return String(); }
};
//  8 bit delay 4 bit type + 4 bit ignored + 8-bit note + 8-bit velocity
struct LoopEvent_NoteOn
{
  static constexpr size_t PayloadSizeBytes = 4;
  static constexpr LoopEventType EventType = LoopEventType::NoteOn;
  static constexpr const char *Name = "NoteOn";

  uint8_t mMidiNote;
  uint8_t mVelocity;

  LoopEvent_NoteOn() = default;

  explicit LoopEvent_NoteOn(Ptr& p)
  {
    p.Read(mMidiNote);
    p.Read(mVelocity);
  }
  explicit LoopEvent_NoteOn(const MusicalVoice& mv) :
    mMidiNote(mv.mMidiNote),
    mVelocity(mv.mVelocity)
  {}
  void Write(Ptr& p, event_delay_t delay) const {
    LoopEvent_WriteHeaderNoParams(p, delay, LoopEventType::NoteOn);
    p.Write(mMidiNote);
    p.Write(mVelocity);
  }
  void ApplyToVoice(MusicalVoice& mv) const
  {
    mv.mMidiNote = mMidiNote;
    mv.mVelocity = mVelocity;
  }
  String ToString() const { return String("note:") + (int)mMidiNote + "v:" + (int)mVelocity; }
};

struct LoopEvent_Breath
{
  static constexpr size_t PayloadSizeBytes = 3;
  static constexpr LoopEventType EventType = LoopEventType::Breath;
  static constexpr const char *Name = "Breath";
  uint16_t mBreath01; // 12 bits

  LoopEvent_Breath() = default;

  explicit LoopEvent_Breath(Ptr& p, uint8_t byte2)
  {
    LoopEvent_Construct12BitParam(p, byte2, mBreath01);
  }
  explicit LoopEvent_Breath(const MusicalVoice& mv) :
    mBreath01(mv.mBreath01.Serialize12Bit())
  {}
  void Write(Ptr& p, event_delay_t delay) const {
    LoopEvent_WriteHeaderAnd12BitParam(p, delay, LoopEventType::Breath, mBreath01);
  }
  void ApplyToVoice(MusicalVoice& mv) const
  {
    mv.mBreath01.Deserialize12Bit(mBreath01);
  }
  String ToString() const { return String("") + (int)mBreath01; }
};

struct LoopEvent_Pitch
{
  static constexpr size_t PayloadSizeBytes = 3;
  static constexpr LoopEventType EventType = LoopEventType::Pitch;
  static constexpr const char *Name = "Pitch";

  uint16_t mPitchN11; // 12 bits

  LoopEvent_Pitch() = default;

  explicit LoopEvent_Pitch(Ptr& p, uint8_t byte2)
  {
    LoopEvent_Construct12BitParam(p, byte2, mPitchN11);
  }
  explicit LoopEvent_Pitch(const MusicalVoice& mv) :
    mPitchN11(mv.mPitchBendN11.Serialize12Bit())
  {}
  void Write(Ptr& p, event_delay_t delay) const {
    LoopEvent_WriteHeaderAnd12BitParam(p, delay, LoopEventType::Pitch, mPitchN11);
  }
  void ApplyToVoice(MusicalVoice& mv) const
  {
    mv.mPitchBendN11.Deserialize12Bit(mPitchN11);
  }
  String ToString() const { return String("") + (int)mPitchN11; }
};

struct LoopEvent_BreathAndPitch
{
  static constexpr size_t PayloadSizeBytes = 5;
  static constexpr LoopEventType EventType = LoopEventType::BreathAndPitch;
  static constexpr const char *Name = "BreathAndPitch";

  uint16_t mBreath01; // 12 bits
  uint16_t mPitchN11; // 16 bits

  LoopEvent_BreathAndPitch() = default;

  explicit LoopEvent_BreathAndPitch(Ptr& p, uint8_t byte2)
  {
    LoopEvent_Construct12BitParam(p, byte2, mBreath01);
    p.Read(mPitchN11);
  }
  explicit LoopEvent_BreathAndPitch(const MusicalVoice& mv) :
    mBreath01(mv.mBreath01.Serialize12Bit()),
    mPitchN11(mv.mPitchBendN11.Serialize12Bit())
  {}
  void Write(Ptr& p, event_delay_t delay) const {
    LoopEvent_WriteHeaderAnd12BitParam(p, delay, LoopEventType::BreathAndPitch, mBreath01);
    p.Write(mPitchN11);
  }
  void ApplyToVoice(MusicalVoice& mv) const
  {
    mv.mBreath01.Deserialize12Bit(mBreath01);
    mv.mPitchBendN11.Deserialize12Bit(mPitchN11);
  }
  String ToString() { return String("b:") + mBreath01 + ", p:" + mPitchN11; }
};

struct LoopEvent_SynthPatchChange
{
  static constexpr size_t PayloadSizeBytes = 3;
  static constexpr LoopEventType EventType = LoopEventType::SynthPatchChange;
  static constexpr const char *Name = "SynthPatch";

  uint16_t mSynthPatchId;// 12 bits

  LoopEvent_SynthPatchChange() = default;

  explicit LoopEvent_SynthPatchChange(Ptr& p, uint8_t byte2)
  {
    LoopEvent_Construct12BitParam(p, byte2, mSynthPatchId);
  }
  explicit LoopEvent_SynthPatchChange(const MusicalVoice& mv) :
    mSynthPatchId(mv.mSynthPatch)
  {}
  void Write(Ptr& p, event_delay_t delay) const {
    LoopEvent_WriteHeaderAnd12BitParam(p, delay, LoopEventType::SynthPatchChange, mSynthPatchId);
  }
  void ApplyToVoice(MusicalVoice& mv) const
  {
    mv.mSynthPatch = mSynthPatchId;
  }
  String ToString() const { return String("") + (int)mSynthPatchId; }
};

struct LoopEvent_HarmPatchChange
{
  static constexpr size_t PayloadSizeBytes = 3;
  static constexpr LoopEventType EventType = LoopEventType::HarmPatchChange;
  static constexpr const char *Name = "HarmPatch";

  uint16_t mHarmPatchId;// 12 bits

  LoopEvent_HarmPatchChange() = default;

  LoopEvent_HarmPatchChange(Ptr& p, uint8_t byte2)
  {
    LoopEvent_Construct12BitParam(p, byte2, mHarmPatchId);
  }
  explicit LoopEvent_HarmPatchChange(const MusicalVoice& mv) :
    mHarmPatchId(mv.mHarmPatch)
  {}
  void Write(Ptr& p, event_delay_t delay) const {
    LoopEvent_WriteHeaderAnd12BitParam(p, delay, LoopEventType::HarmPatchChange, mHarmPatchId);
  }
  void ApplyToVoice(MusicalVoice& mv) const
  {
    mv.mHarmPatch = mHarmPatchId;
  }
  String ToString() const { return String("") + (int)mHarmPatchId; }
};

struct LoopEvent_FullState
{
  static constexpr size_t PayloadSizeBytes = 10;
  static constexpr LoopEventType EventType = LoopEventType::FullState;
  static constexpr const char *Name = "FullState";

  uint16_t mBreath01; // 12 // 3
  uint16_t mPitchN11; // 16
  uint16_t mSynthPatchId; // 12
  uint16_t mHarmPatchId; // 12
  uint8_t mMidiNote; // 8 
  uint8_t mVelocity; // 8

  LoopEvent_FullState() = default;

  explicit LoopEvent_FullState(const MusicalVoice& mv) :
    mBreath01(mv.mBreath01.Serialize12Bit()),
    mPitchN11(mv.mPitchBendN11.Serialize12Bit()),
    mSynthPatchId(mv.mSynthPatch),
    mHarmPatchId(mv.mHarmPatch),
    mMidiNote(mv.mMidiNote),
    mVelocity(mv.mVelocity)
  {
  }

  LoopEvent_FullState(Ptr& p, uint8_t byte2)
  {
    LoopEvent_Construct12BitParam(p, byte2, mBreath01);
    p.Read(mPitchN11);
    LoopEvent_ReadTwo12BitValues(p, mSynthPatchId, mHarmPatchId);
    p.Read(mMidiNote);
    p.Read(mVelocity);
  }
  void Write(Ptr& p, event_delay_t delay) const {
    LoopEvent_WriteHeaderAnd12BitParam(p, delay, LoopEventType::FullState, mBreath01);
    p.Write(mPitchN11);
    LoopEvent_WriteTwo12BitValues(p, mSynthPatchId, mHarmPatchId);
    p.Write(mMidiNote);
    p.Write(mVelocity);
  }

  String ToString() const
  {
    return String(
      "b:") + (int)mBreath01 +
      ", p:" + (int)mPitchN11 +
      ", s:" + (int)mSynthPatchId +
      ", h:" + (int)mHarmPatchId +
      ", note:" + (int)mMidiNote + ",v:" + (int)mVelocity
      ;
  }
  void ApplyToVoice(MusicalVoice& mv) const
  {
    mv.mBreath01.Deserialize12Bit(mBreath01);
    mv.mPitchBendN11.Deserialize12Bit(mPitchN11);
    mv.mSynthPatch = mSynthPatchId;
    mv.mHarmPatch = mHarmPatchId;
    mv.mIsNoteCurrentlyMuted = false;
    mv.mMidiNote = mMidiNote;
    mv.mVelocity = mVelocity;
  }
};

