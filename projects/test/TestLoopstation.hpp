
#pragma once

#include "Test.hpp"

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/loopstation/Loopstation.hpp>
#include <clarinoid/loopstation/LooperHarmonizer.hpp>
#include <clarinoid/synth/Synth.hpp>


inline bool SynthIsPlayingNote(uint16_t voiceID, uint8_t midiNote)
{
  for (auto& l : gVoices) {
    if (l.mRunningVoice.mVoiceId != voiceID)
      continue;
    if (l.mRunningVoice.mMidiNote != midiNote)
      continue;
    return true;
  }
  return false;
}


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
  //mv.mNeedsNoteOn = true;
  mv.mMidiNote = 20;
  mv.mVelocity = 20;
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
  Test(mvout.mBreath01.Serialize12Bit() == 90); // <-- it will sorta memorize this state even though the breath=90 event wasn't in the log; it was in state and gets recorded an wrapping up recording.

  status.mCurrentLoopTimeMS = 375;
  ec = stream.ReadUntilLoopTime(mvout);
  stream.Dump();
  // we have to read the events above
  Test(mvout.mBreath01.Serialize12Bit() == 91);
  status.mCurrentLoopTimeMS = 0;
  ec = stream.ReadUntilLoopTime(mvout);
  stream.Dump();
  // we have to read the events above
  //Test(ec == 7);
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
  MusicalVoiceTransitionEvents te;

  SetTestClockMillis(1000);
  lv.mHarmPatch = 4;
  lv.mSynthPatch = 5;
  lv.mVoiceId = 0x666;
  lh.LoopIt(lv); // start recording.

  MusicalVoice outp[10];

  SetTestClockMillis(2000); // play note
  //lv.mIsNoteCurrentlyOn = true;
  //lv.mNeedsNoteOn = true;
  lv.mMidiNote = 30;
  lv.mVelocity = 31;

  size_t vc = lh.Update(lv, te, outp, EndPtr(outp));
  Test(vc == 1);
  //Test(outp[0].mNeedsNoteOn);
  Test(outp[0].mMidiNote == 30);
  Test(outp[0].mHarmPatch == 4);
  Test(outp[0].mSynthPatch == 5);

  SetTestClockMillis(3000); // stop playing note.
  //lv.ResetOneFrameState();
  //lv.mNeedsNoteOff = true;
  //lv.mNoteOffNote = 30;
  //lv.mIsNoteCurrentlyOn = false;
  lv.mMidiNote = 0;

  vc = lh.Update(lv, te, outp, EndPtr(outp));

  SetTestClockMillis(4000); // commit recording
  //lv.ResetOneFrameState();
  lh.LoopIt(lv);
  lh.mLayers[0].Dump();

  SetTestClockMillis(5000); // test playback.
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  lh.mLayers[0].Dump();
  Test(vc == 2);
  Test(lh.mStatus.mCurrentLoopTimeMS == 1000);
  Test(lh.mStatus.mLoopDurationMS == 3000);
  Test(lh.mStatus.mState == LooperState::DurationSet);
//  Test(lh.mCurrentPolyphony == 2);
  Test(lh.mCurrentlyWritingLayer == 1);
  Test(outp[0].mVoiceId == MakeMusicalVoiceID(0, 0));
  Test(outp[0].mMidiNote == 30);
  Test(outp[1].mVoiceId == MakeMusicalVoiceID(1, 0));
}

// test that voice IDs are mapped to real synth output
void TestLoopstationSynth()
{
  LooperAndHarmonizer lh;
  MusicalVoice lv;
  CCSynth s;
  MusicalVoiceTransitionEvents te;
  s.setup();

  SetTestClockMillis(1000);
  lv.mHarmPatch = 4;
  lv.mSynthPatch = 5;
  lh.LoopIt(lv); // start recording. 1000 = start recording (looptime 0)

  MusicalVoice outp[10];

  SetTestClockMillis(2000); // 2000 = note on (looptime 1000)
  lv.mMidiNote = 30;
  lv.mVelocity = 31;

  size_t vc = lh.Update(lv, te, outp, EndPtr(outp));
  Test(vc == 1);
  Test(outp[0].mMidiNote == 30);
  Test(outp[0].mHarmPatch == 4);
  Test(outp[0].mSynthPatch == 5);

  SetTestClockMillis(3000); // 3000 stop playing note. (looptime 2000)
  lv.mMidiNote = 0;
  vc = lh.Update(lv, te, outp, EndPtr(outp));

  SetTestClockMillis(4000); // commit recording (loop duration 3000) - layer0 has a note, layer1 recording.
  lh.LoopIt(lv);

  SetTestClockMillis(5000 /* loop time 1000 */); // test playback.
  lh.mLayers[0].Dump();
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  Test(vc == 2); // playback note + live note (which is not playing)
  Test(lh.mStatus.mCurrentLoopTimeMS == 1000);
  Test(lh.mCurrentlyWritingLayer == 1);
  Test(lh.mStatus.mLoopDurationMS == 3000);
  Test(lh.mStatus.mState == LooperState::DurationSet);
  //Test(lh.mCurrentPolyphony == 2);
  Test(outp[0].mVoiceId == MakeMusicalVoiceID(0, 0));
  Test(outp[0].mMidiNote == 30);
  Test(outp[1].mVoiceId == MakeMusicalVoiceID(1, 0));

  s.Update(outp, outp + vc); // update synth @ loop time 1000; now we should see layer0 note. live voice is not playing.
  Test(s.mCurrentPolyphony == 1);
  Test(gVoices[0].mRunningVoice.mVoiceId == 0x0000);
  Test(gVoices[0].mRunningVoice.mMidiNote == 30);
  Test(gVoices[0].mRunningVoice.mVelocity == 31);

  SetTestClockMillis(5001 /* loop time 1001 */);
  // play a live voice, check that 2 notes are playing with correct ids
  lv.mMidiNote = 45;
  lv.mVelocity = 1;
  lh.mLayers[0].Dump();
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  Test(vc == 2); // loop layer 0 is playing note, live voice is playing
  Test(lh.mStatus.mCurrentLoopTimeMS == 1001);

  // check that when nothing changes, nothing changes.
  SetTestClockMillis(5002 /* loop time 1002 */);
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  Test(vc == 2); // loop layer 0 is playing note, live voice is playing
  Test(lh.mStatus.mCurrentLoopTimeMS == 1002);

  // do similar test to @1001 but include the live playing voice.
  s.Update(outp, outp + vc);
  Test(s.mCurrentPolyphony == 2);
  Test(gVoices[0].mRunningVoice.mVoiceId == 0x0000);
  Test(gVoices[0].mRunningVoice.mMidiNote == 30);
  Test(gVoices[0].mRunningVoice.mVelocity == 31);

  Test(gVoices[1].mRunningVoice.mVoiceId == 0x0100);
  Test(gVoices[1].mRunningVoice.mMidiNote == 45);
  Test(gVoices[1].mRunningVoice.mVelocity == 1);

  SetTestClockMillis(6000 /* loop time 2000 */); // note off should happen now.
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  Test(vc == 2); // layer0 note off, livevoice which is still playing.
  Test(outp[0].mMidiNote == 0);
  Test(outp[1].mMidiNote == 45);

  s.Update(outp, outp + vc); // just playing live voice
  Test(s.mCurrentPolyphony == 1);
  Test(gVoices[0].mRunningVoice.mVoiceId == 0xffff);
  Test(gVoices[0].mRunningVoice.mMidiNote == 0);
  Test(gVoices[0].mRunningVoice.mVelocity == 0);
  Test(gVoices[1].mRunningVoice.mVoiceId == 0x0100);
  Test(gVoices[1].mRunningVoice.mMidiNote == 45);
  Test(gVoices[1].mRunningVoice.mVelocity == 1);

  SetTestClockMillis(6001 /* loop time 2001 */); // no activity.
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  Test(vc == 1); // layer0 is not playing and already did note off, livevoice which is still playing.
  Test(outp[0].mMidiNote == 45);

  s.Update(outp, outp + vc); // just playing live voice
  Test(s.mCurrentPolyphony == 1);
  Test(gVoices[0].mRunningVoice.mVoiceId == 0xffff);
  Test(gVoices[0].mRunningVoice.mMidiNote == 0);
  Test(gVoices[0].mRunningVoice.mVelocity == 0);
  Test(gVoices[1].mRunningVoice.mVoiceId == 0x0100);
  Test(gVoices[1].mRunningVoice.mMidiNote == 45);
  Test(gVoices[1].mRunningVoice.mVelocity == 1);

  
  lv.mMidiNote = 0;
  lv.mVelocity = 0;
  SetTestClockMillis(6002 /* loop time 2002 */);// stop playing live note.
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  Test(lh.mStatus.mCurrentLoopTimeMS == 2002);
  Test(vc == 1); // layer0 is not playing and already did note off, livevoice which is still playing.
  Test(outp[0].mMidiNote == 0);

  s.Update(outp, outp + vc); // just playing live voice
  Test(s.mCurrentPolyphony == 0);
  Test(gVoices[1].mRunningVoice.mVoiceId == 0xffff);

  // commit recording again
  // the loop is 3 seconds long.
  // we have just recorded
  // 1001: note on (45), 2002: note off.
  // but to test the mid-loop stuff, let's advance time and record a layer that looks like:
  // 500: note on, 2500: note off
  SetTestClockMillis(1000 + 3000 + 3000 + 3000 + 500/* loop time 1500 */);
  lv.mMidiNote = 55;
  lv.mVelocity = 55;
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  lh.mLayers[1].Dump();

  SetTestClockMillis(1000 + 3000 + 3000 + 3000 + 1500/* loop time 1500 */);
  lv.mMidiNote = 0;
  lv.mVelocity = 0;
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  lh.mLayers[1].Dump();

  // now update again at 1700 stop recording
  SetTestClockMillis(1000 + 3000 + 3000 + 3000 + 1700/* loop time 1700 */);
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  lh.mLayers[1].Dump();

  // commit recording (current layer now 2)
  SetTestClockMillis(1000 + 3000 + 3000 + 3000 + 1701/* loop time 1701 */);
  lh.LoopIt(lv);
  lh.mLayers[1].Dump();

  // layer1:   |      |noteon(55)--------------|xxxxx|yyyyy|..                  |
  // oldlayer1:|                   |noteon(45)------------------|               |

  // test playing both layers
  // T:        0     500        1000         1500  1700  2000         2500    3000
  // layer0:   |                  |noteon(30)--------------|                    |
  // layer1:   |      |noteon(55)--------------|     |noteon(45)|               |
  // so let's sample at:
  //             ^      ^            ^            ^    ^         ^
  // this is tricky; at 'x', we have explicitly recorded silence.
  // but at y, we haven't yet recorded anything so it will contain material from the previous loop.

  lv.Reset(); // stop playing live note.

  SetTestClockMillis(1000 + (4 * 3000) + 50/* loop time 0 */);
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  s.Update(outp, outp + vc); // just playing live voice
  Test(s.mCurrentPolyphony == 0);

  SetTestClockMillis(1000 + (4 * 3000) + 600);
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  s.Update(outp, outp + vc); // just playing live voice
  Test(s.mCurrentPolyphony == 1);
  Test(SynthIsPlayingNote(0x0100, 55));

  SetTestClockMillis(1000 + (4 * 3000) + 1100);
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  s.Update(outp, outp + vc); // just playing live voice
  Test(s.mCurrentPolyphony == 2);
  Test(SynthIsPlayingNote(0x0100, 55));
  Test(SynthIsPlayingNote(0x0000, 30));

  SetTestClockMillis(1000 + (4 * 3000) + 1600); // in 'x'
  lh.mLayers[1].Dump();
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  lh.mLayers[1].Dump();
  s.Update(outp, outp + vc); // just playing live voice
  TestExpectFailure(s.mCurrentPolyphony == 1, "yea this fails but i suspect bad test. i'd like a better scripted way of testing loopstation.");

  SetTestClockMillis(1000 + (4 * 3000) + 1800); // in 'y'
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  s.Update(outp, outp + vc); // just playing live voice
  Test(s.mCurrentPolyphony == 2);
  Test(SynthIsPlayingNote(0x0100, 45));
  Test(SynthIsPlayingNote(0x0000, 30));

  SetTestClockMillis(1000 + (4 * 3000) + 2100);
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  s.Update(outp, outp + vc); // just playing live voice
  Test(s.mCurrentPolyphony == 0);


  // todo: play a live voice again, continue activity
}





void TestLoopstationLegato()
{
  MusicalVoiceTransitionEvents te;
  MusicalVoice outp[10];
  LooperAndHarmonizer lh;
  MusicalVoice lv;
  CCSynth s;
  s.setup();

  SetTestClockMillis(1000);
  lv.mHarmPatch = 4;
  lv.mSynthPatch = 5;
  lh.LoopIt(lv); // start recording. 1000 = start recording (looptime 0)

  SetTestClockMillis(2000); // 2000 = note on (looptime 1000)
  lv.mMidiNote = 30;
  lv.mVelocity = 31;

  size_t vc = lh.Update(lv, te, outp, EndPtr(outp));
  lh.mLayers[0].Dump();

  SetTestClockMillis(2500); // 3000 move to next note note. (looptime 1500)
  lv.mMidiNote = 40;
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  lh.mLayers[0].Dump();

  SetTestClockMillis(3000); // 3000 move to next note note. (looptime 2000)
  lv.mMidiNote = 0;
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  lh.mLayers[0].Dump();

  SetTestClockMillis(4000); // commit recording (loop duration 3000) - layer0 has a note, layer1 recording.
  lh.LoopIt(lv);
  // loop layer 0 is like:
  //     |            note(30)---------note(40)--------          |
  //     0            ^1000            ^1500          ^2000      3000

  SetTestClockMillis(5000 /* loop time 1000 */); // test playback.
  lh.mLayers[0].Dump();
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  s.Update(outp, outp + vc);
  Test(vc == 2); // playback note + live note (which is not playing)
  Test(s.mCurrentPolyphony == 1);
  Test(SynthIsPlayingNote(0, 30));

  SetTestClockMillis(1000 + 3000 + 1499 /* loop time 1499 */);
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  lh.mLayers[0].Dump();
  s.Update(outp, outp + vc);
  Test(vc == 2);
  Test(s.mCurrentPolyphony == 1);
  Test(SynthIsPlayingNote(0, 30));

  SetTestClockMillis(1000 + 3000 + 1500);
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  lh.mLayers[0].Dump();
  s.Update(outp, outp + vc);
  Test(vc == 2);
  Test(s.mCurrentPolyphony == 1);
  Test(SynthIsPlayingNote(0, 40));

  SetTestClockMillis(1000 + 3000 + 1999);
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  s.Update(outp, outp + vc);
  Test(vc == 2);
  Test(s.mCurrentPolyphony == 1);
  Test(SynthIsPlayingNote(0, 40));

  SetTestClockMillis(1000 + 3000 + 2000);
  vc = lh.Update(lv, te, outp, EndPtr(outp));
  s.Update(outp, outp + vc);
  Test(vc == 2);
  Test(s.mCurrentPolyphony == 0);

}

