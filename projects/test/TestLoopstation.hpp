
#pragma once

#include "Test.hpp"

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/loopstation/Loopstation.hpp>
#include <clarinoid/loopstation/LooperHarmonizer.hpp>
#include <clarinoid/synth/Synth.hpp>


// write a loop, read it back.
void TestHappyFlow()
{
  MusicalVoice mv;

  uint8_t buf[10000];
  LoopEventStream stream;
  LoopStatus status;

  status.mState = LooperState::StartSet;
  status.mLoopDurationMS = 0;

  // some events at 0, 100ms, loop len 1 second.
  status.mCurrentLoopTimeMS = 0;
  stream.StartRecording(status, mv, Ptr(buf), Ptr(EndPtr(buf)));
  mv.mBreath01 = 0.5f;
  stream.Write(mv);
  stream.Dump();
  Test(stream.DebugGetStream().size() == 2);

  status.mCurrentLoopTimeMS = 100;
  mv.mBreath01 = 0.0f;
  mv.mSynthPatch = 5;
  mv.mNeedsNoteOn = true;
  mv.mMidiNote = 20;
  stream.Write(mv);
  stream.Dump();
  Test(stream.DebugGetStream().size() == 5);

  // simulate pressing LoopIt again at 1sec.
  status.mState = LooperState::DurationSet;
  status.mLoopDurationMS = 1000;
  status.mCurrentLoopTimeMS = 0;

  stream.WrapUpRecording();
  stream.Dump();

  //Test(stream.DebugGetStream().size() == 7);

  status.mCurrentLoopTimeMS = 0;
  MusicalVoice mvout;
  stream.Dump();
  stream.ReadUntilLoopTime(mvout);
  stream.Dump();
  Test(mvout.mBreath01.GetFloatVal() == 0.5f); // ON the cursor, we should get the breath change applied.
  stream.ReadUntilLoopTime(mvout);
  stream.Dump();
  Test(mvout.mBreath01.GetFloatVal() == 0.5f); // reading again at the same place should not change anything.
  status.mCurrentLoopTimeMS = 90;
  stream.ReadUntilLoopTime(mvout); // advancing to before the next event should also not change anything
  stream.Dump();
  Test(mvout.mBreath01.GetFloatVal() == 0.5f);

  status.mCurrentLoopTimeMS = 110; // advancing to AFTER the event should change things.
  stream.ReadUntilLoopTime(mvout);
  Test(mvout.mBreath01.GetFloatVal() == 0.0f);
  Test(mvout.mSynthPatch == 5);
  Test(mvout.mMidiNote == 20);

  status.mCurrentLoopTimeMS = 10; // wrapping the loop. we should go back to original state.
  stream.ReadUntilLoopTime(mvout);
  Test(mvout.mBreath01.GetFloatVal() == 0.5f);
}



// write a loop, read it back.
void TestConsumeMultipleEvents()
{
  MusicalVoice mv;
  MusicalVoice mvout;
  uint8_t buf[10000];
  LoopEventStream stream;
  LoopStatus status;

  status.mState = LooperState::DurationSet;
  status.mLoopDurationMS = 1000;
  stream.StartRecording(status, mv, Ptr(buf), Ptr(EndPtr(buf)));
  stream.Dump();

  // some events at 10ms, 20ms, 50ms, loop len 1 second.
  status.mCurrentLoopTimeMS = 10;
  mv.mBreath01.Deserialize12Bit(1);
  stream.Write(mv);
  stream.Dump();
  Test(stream.DebugGetStream().size() == 2);

  status.mCurrentLoopTimeMS = 20;
  mv.mPitchBendN11.Deserialize12Bit(2);
  stream.Write(mv);
  cc::log("---");
  stream.Dump();
  Test(stream.DebugGetStream().size() == 3);

  status.mCurrentLoopTimeMS = 50;
  mv.mHarmPatch = 5;
  mv.mSynthPatch = 5;
  mv.mBreath01.Deserialize12Bit(5);
  stream.Write(mv);
  cc::log("---");
  stream.Dump();
  Test(stream.DebugGetStream().size() == 6);

  // simulate pressing LoopIt again at 1sec.
  status.mCurrentLoopTimeMS = 0;
  stream.WrapUpRecording();
  stream.Dump();

  //Test(stream.DebugGetStream().size() == 8);

  status.mCurrentLoopTimeMS = 0;
  stream.ReadUntilLoopTime(mvout);

  status.mCurrentLoopTimeMS = 100;
  stream.ReadUntilLoopTime(mvout);
  stream.Dump();

  Test(mvout.mBreath01.Serialize12Bit() == 5);
  Test(mvout.mHarmPatch == 5);
  Test(mvout.mSynthPatch == 5);
  Test(mvout.mPitchBendN11.Serialize12Bit() == 2);
}




void TestMuted()
{
  MusicalVoice mv;
  MusicalVoice mvout;
  uint8_t buf[10000];
  LoopEventStream stream;
  LoopStatus status;

  status.mState = LooperState::DurationSet;
  status.mLoopDurationMS = 1000;
  stream.StartRecording(status, mv, Ptr(buf), Ptr(EndPtr(buf)));

  // some events at 10ms, 20ms, 50ms, loop len 1 second.
  status.mCurrentLoopTimeMS = 10;
  mv.mBreath01.Deserialize12Bit(1);
  stream.Write(mv);
  cc::log("---");
  stream.Dump();
  Test(stream.DebugGetStream().size() == 2);

  // simulate pressing LoopIt again at 1sec.
  status.mCurrentLoopTimeMS = 0;
  stream.WrapUpRecording();
  stream.mIsPlaying = false;
  stream.ReadUntilLoopTime(mvout);
  Test(mvout.mBreath01 != 1);
}


void TestReadingAfterLooped()
{
  MusicalVoice mv;
  MusicalVoice mvout;
  uint8_t buf[10000];
  LoopEventStream stream;
  LoopStatus status;

  status.mState = LooperState::DurationSet;
  status.mLoopDurationMS = 1000;
  stream.StartRecording(status, mv, Ptr(buf), Ptr(EndPtr(buf)));

  // some events at 10ms, 20ms, 50ms, loop len 1 second.
  status.mCurrentLoopTimeMS = 10;
  mv.mBreath01.Deserialize12Bit(1);
  stream.Write(mv);
  stream.Dump();
  Test(stream.DebugGetStream().size() == 2);

  status.mCurrentLoopTimeMS = 20;
  mv.mPitchBendN11.Deserialize12Bit(2);
  stream.Write(mv);
  stream.Dump();
  Test(stream.DebugGetStream().size() == 3);

  status.mCurrentLoopTimeMS = 50;
  mv.mHarmPatch = 5;
  mv.mSynthPatch = 5;
  mv.mBreath01.Deserialize12Bit(5);
  stream.Write(mv);
  stream.Dump();
  Test(stream.DebugGetStream().size() == 6);

  // simulate pressing LoopIt again at 1sec.
  stream.WrapUpRecording();

  status.mCurrentLoopTimeMS = 60;
  stream.ReadUntilLoopTime(mvout);
  Test(mvout.mBreath01.Serialize12Bit() == 5);

  status.mCurrentLoopTimeMS = 15;
  stream.ReadUntilLoopTime(mvout);
  Test(mvout.mBreath01.Serialize12Bit() == 1);

  status.mCurrentLoopTimeMS = 14; // should loop all the way around.
  stream.ReadUntilLoopTime(mvout);
  Test(mvout.mBreath01.Serialize12Bit() == 1);
}


void TestReadingEmptyBuffer()
{
  MusicalVoice mv;
  MusicalVoice mvout;
  uint8_t buf[10000];
  LoopEventStream stream;
  LoopStatus status;

  status.mState = LooperState::DurationSet;
  status.mLoopDurationMS = 1000;
  stream.StartRecording(status, mv, Ptr(buf), Ptr(EndPtr(buf)));
  stream.Dump();
  stream.WrapUpRecording();

  status.mCurrentLoopTimeMS = 60;
  stream.ReadUntilLoopTime(mvout);

  status.mCurrentLoopTimeMS = 15;
  stream.ReadUntilLoopTime(mvout);

  status.mCurrentLoopTimeMS = 14; // should loop all the way around.
  stream.ReadUntilLoopTime(mvout);
}

// test recording when you record more than a full loop of material in the buffer,
// and the recorded material is all in 1 continuous block. when recording ends, the buffer
// is simply moved left (PZE situation)
void TestEndRecordingWithFullLoopSimple()
{
  MusicalVoice mv;
  MusicalVoice mvout;
  uint8_t buf[10000];
  LoopEventStream stream;

  LoopStatus status;
  status.mState = LooperState::DurationSet;
  status.mLoopDurationMS = 1000;
  status.mCurrentLoopTimeMS = 0;

  stream.StartRecording(status, mv, Ptr(buf), Ptr(EndPtr(buf)));

  status.mCurrentLoopTimeMS = 0;
  mv.mBreath01.Deserialize12Bit(0);
  stream.Write(mv);

  status.mCurrentLoopTimeMS = 300;
  mv.mBreath01.Deserialize12Bit(1);
  stream.Write(mv);

  status.mCurrentLoopTimeMS = 600;
  mv.mBreath01.Deserialize12Bit(2);
  stream.Write(mv);

  status.mCurrentLoopTimeMS = 950;
  mv.mBreath01.Deserialize12Bit(3);
  stream.Write(mv);

  status.mCurrentLoopTimeMS = 100;
  mv.mBreath01.Deserialize12Bit(4);
  //Test(stream.mPrevCursor.mP == nullptr);
  //Test(stream.mRecStartCursor.mP == stream.mBufferBegin.mP);
  stream.Dump(); // you should see a NOP event of 50ms next to the breath event of 100ms, to sit directly on the loop boundary.
  stream.Write(mv); // this one makes the recording longer than 1 loop.
  stream.Dump(); // you should see a NOP event of 50ms next to the breath event of 100ms, to sit directly on the loop boundary.
  Test(!!stream.mPrevCursor.mP);

  stream.WrapUpRecording();
  stream.Dump();

  // read it back. 
  status.mCurrentLoopTimeMS = 0;
  size_t ec = stream.ReadUntilLoopTime(mvout);
  stream.Dump();
  status.mCurrentLoopTimeMS = 100;
  ec = stream.ReadUntilLoopTime(mvout);
  stream.Dump();
  Test(ec == 1);// breath is changed at 100.
  Test(mvout.mBreath01.Serialize12Bit() == 4);
}


// when duration is set, and you're recording a new layer, we run out of memory
// before you're even done recording this layer. we just stop recording.
void TestScenarioOOM_PartialLoop()
{
  // |xxE---PxxxxZxx (epz)
  MusicalVoice mv;
  MusicalVoice mvout;
  uint8_t buf[100];
  LoopEventStream stream;

  LoopStatus status;
  status.mState = LooperState::DurationSet;
  status.mLoopDurationMS = 40000;

  stream.StartRecording(status, mv, Ptr(buf), Ptr(buf + 50));

  status.mCurrentLoopTimeMS = 0;
  for (int i = 0; i < 10; ++i) {
    status.mCurrentLoopTimeMS += 5000;
    status.mCurrentLoopTimeMS %= status.mLoopDurationMS;
    mv.mBreath01.Deserialize12Bit(i + 1);
    stream.Write(mv);
    stream.Dump();
  }

  // Note: if we did not LOCK out recording, then some events could make it into
  // the buffer if they're small enough.
  //Test(stream.DebugGetStream().size() == 5);
  Test(stream.mOOM);
}

// also tests that we can start writing from beginning again by wrapping, if we have
// enough space in the buffer.
void TestScenarioEP()
{
  // |xxE---PxxxxZxx (epz)
  MusicalVoice mv;
  MusicalVoice mvout;
  uint8_t buf[300];
  LoopEventStream stream;

  LoopStatus status;
  status.mState = LooperState::DurationSet;
  status.mLoopDurationMS = 1000; // we want each event to be 5 secs long,
  // and in order to actually trigger this scenario we need to capture prev
  // cursor. so we must cross a loop within the buffer. so make loop small enough.

  stream.StartRecording(status, mv, Ptr(buf), Ptr(EndPtr(buf)));

  status.mCurrentLoopTimeMS = 0;
  for (int i = 0; i < 100; ++i) {
    status.mCurrentLoopTimeMS += 125;
    status.mCurrentLoopTimeMS %= status.mLoopDurationMS;
    mv.mBreath01.Deserialize12Bit(i + 1);
    stream.Write(mv);
    stream.Dump();
    // if we hit EPZ break out.
    if (stream.GetLayoutSituation() == LayoutSituation::EP) {
      break;
    }
  }

  stream.Dump();
  stream.WrapUpRecording();
  stream.Dump();

  status.mCurrentLoopTimeMS = 600;
  size_t ec = stream.ReadUntilLoopTime(mvout);
  stream.Dump();
  status.mCurrentLoopTimeMS = 250;
  ec = stream.ReadUntilLoopTime(mvout);
  stream.Dump();
  // we have to read the events above
  Test(mvout.mBreath01.Serialize12Bit() == 97);
  status.mCurrentLoopTimeMS = 0;
  ec = stream.ReadUntilLoopTime(mvout);
  stream.Dump();
  // we have to read the events above
  Test(ec == 7);
  Test(mvout.mBreath01.Serialize12Bit() == 96);
}


void TestFullMusicalState1()
{
  // |xxE---PxxxxZxx (epz)
  MusicalVoice mv;
  MusicalVoice mvout;
  uint8_t buf[3000];
  LoopEventStream stream;

  LoopStatus status;
  status.mState = LooperState::DurationSet;
  status.mLoopDurationMS = 60000;

  // start with some state @ 1000ms.
  status.mCurrentLoopTimeMS = 1000;
  mv.mBreath01.Deserialize12Bit(1001);
  mv.mPitchBendN11.Deserialize12Bit(1002);
  mv.mSynthPatch = 1003;
  mv.mHarmPatch = 1004;

  stream.StartRecording(status, mv, Ptr(buf), Ptr(EndPtr(buf)));
  stream.Dump();

  // we somehow wrap all the way around to t=3 with totalyl new state.
  // |-n             nnn-----------------|
  status.mCurrentLoopTimeMS = 3;
  mv.mBreath01.Deserialize12Bit(1);
  mv.mPitchBendN11.Deserialize12Bit(2);
  mv.mSynthPatch = 3;
  mv.mHarmPatch = 4;
  stream.Write(mv);
  stream.Dump();

  // and at t=50ms with different state.
  // |-n---nn        nnn-----------------|
  status.mCurrentLoopTimeMS = 50;
  mv.mPitchBendN11.Deserialize12Bit(52);
  mv.mSynthPatch = 53;
  stream.Write(mv);
  stream.Dump();

  stream.WrapUpRecording();
  stream.Dump();

  // check musical state against what we wrote.
  status.mCurrentLoopTimeMS = 1000;
  stream.ReadUntilLoopTime(mvout);
  stream.Dump();
  Test(mvout.mBreath01.Serialize12Bit() == 1001);
  Test(mvout.mPitchBendN11.Serialize12Bit() == 1002);
  Test(mvout.mSynthPatch == 1003);
  Test(mvout.mHarmPatch == 1004);
}

void TestFullMusicalState2()
{
  // |xxE---PxxxxZxx (epz)
  MusicalVoice mv;
  MusicalVoice mvout;
  uint8_t buf[3000];
  LoopEventStream stream;

  LoopStatus status;
  status.mState = LooperState::DurationSet;
  status.mLoopDurationMS = 60000;

  // start with some state @ 1000ms.
  status.mCurrentLoopTimeMS = 1000;
  mv.mBreath01.Deserialize12Bit(1001);
  mv.mPitchBendN11.Deserialize12Bit(1002);
  mv.mSynthPatch = 1003;
  mv.mHarmPatch = 1004;

  stream.StartRecording(status, mv, Ptr(buf), Ptr(EndPtr(buf)));
  stream.Dump();

  // we somehow wrap all the way around to t=3 with totalyl new state.
  status.mCurrentLoopTimeMS = 3;
  mv.mBreath01.Deserialize12Bit(1);
  mv.mPitchBendN11.Deserialize12Bit(2);
  mv.mSynthPatch = 3;
  mv.mHarmPatch = 4;
  stream.Write(mv);
  stream.Dump();

  // and at t=50ms with different state.
  status.mCurrentLoopTimeMS = 50;
  mv.mPitchBendN11.Deserialize12Bit(52);
  mv.mSynthPatch = 53;
  stream.Write(mv);
  stream.Dump();

  // and then for fun
  status.mCurrentLoopTimeMS = 59000;
  mv.mBreath01.Deserialize12Bit(880);
  mv.mPitchBendN11.Deserialize12Bit(881);
  mv.mSynthPatch = 882;
  mv.mHarmPatch = 883;
  stream.Write(mv);
  stream.Dump();

  stream.WrapUpRecording();
  stream.Dump();

  // check musical state against what we wrote.
  status.mCurrentLoopTimeMS = 1000;
  stream.ReadUntilLoopTime(mvout);
  stream.Dump();
  Test(mvout.mBreath01.Serialize12Bit() == 1);
  Test(mvout.mPitchBendN11.Serialize12Bit() == 52);
  Test(mvout.mSynthPatch == 53);
  Test(mvout.mHarmPatch == 4);
}

void TestReadHeader()
{
  uint8_t buf[10];
  Ptr write(buf);
  LoopEvent_WriteHeaderNoParams(write, 55, LoopEventType::HarmPatchChange);
  write.Write((uint8_t)89);
  Ptr read(buf);
  uint8_t delay;
  LoopEventType eventType;
  uint8_t byte2;
  LoopEvent_ReadHeader(read, delay, eventType, byte2);
  uint8_t magic;
  read.Read(magic);
  Test(magic == 89);
  Test(read == write);
  Test(delay == 55);
  Test(eventType == LoopEventType::HarmPatchChange);

  write = Ptr(&buf[0]);
  read = write;
  LoopEvent_SurgicallyWriteDelayInPlace(write, 66);
  LoopEvent_ReadHeader(read, delay, eventType, byte2);
  Test(delay == 66);
}

void Test12BitParam()
{
  uint8_t buf[10];
  Ptr write(buf);
  const uint16_t param1 = 0x345;
  LoopEvent_WriteHeaderAnd12BitParam(write, 121, LoopEventType::SynthPatchChange, param1);
  write.Write((uint8_t)89);

  Ptr read(buf);
  uint8_t delay;
  LoopEventType eventType;
  uint8_t byte2;
  uint16_t param1r;
  LoopEvent_ReadHeader(read, delay, eventType, byte2);
  LoopEvent_Construct12BitParam(read, byte2, param1r);

  uint8_t magic;
  read.Read(magic);
  Test(read == write);
  Test(delay == 121);
  Test(eventType == LoopEventType::SynthPatchChange);
  Test(magic == 89);
  Test(param1r == param1);

  write = Ptr(&buf[0]);
  read = write;
  LoopEvent_SurgicallyWriteDelayInPlace(write, 66);
  LoopEvent_ReadHeader(read, delay, eventType, byte2);
  Test(delay == 66);
}


// test that voice IDs are constructed well
void TestVoiceID()
{
  LooperAndHarmonizer lh;
  MusicalVoice lv;

  SetTestClockMillis(1000);
  lv.mHarmPatch = 4;
  lv.mSynthPatch = 5;
  lv.mVoiceId = 0x666;
  lh.LoopIt(lv); // start recording.

  MusicalVoice outp[10];

  SetTestClockMillis(2000); // play note
  lv.mIsNoteCurrentlyOn = true;
  lv.mNeedsNoteOn = true;
  lv.mMidiNote = 30;
  lv.mVelocity = 31;

  size_t vc = lh.Update(lv, outp, EndPtr(outp));
  Test(vc == 1);
  Test(outp[0].mNeedsNoteOn);
  Test(outp[0].mMidiNote == 30);
  Test(outp[0].mHarmPatch == 4);
  Test(outp[0].mSynthPatch == 5);

  SetTestClockMillis(3000); // stop playing note.
  lv.ResetOneFrameState();
  lv.mNeedsNoteOff = true;
  lv.mNoteOffNote = 30;
  lv.mIsNoteCurrentlyOn = false;

  vc = lh.Update(lv, outp, EndPtr(outp));

  SetTestClockMillis(4000); // commit recording
  lv.ResetOneFrameState();
  lh.LoopIt(lv);

  SetTestClockMillis(5000); // test playback.
  vc = lh.Update(lv, outp, EndPtr(outp));
  Test(vc == 2);
  Test(lh.mStatus.mCurrentLoopTimeMS == 1000);
  Test(lh.mStatus.mLoopDurationMS == 3000);
  Test(lh.mStatus.mState == LooperState::DurationSet);
  Test(lh.mCurrentPolyphony == 2);
  Test(lh.mCurrentlyWritingLayer == 1);
  Test(outp[0].mVoiceId == MakeMusicalVoiceID(0, 0));
  Test(outp[0].mMidiNote == 30);
  Test(outp[1].mVoiceId == MakeMusicalVoiceID(1, 0));
}


// test that voice IDs are mapped to real synth output
void TestLoopstationSynth()
{
  //
}

