

#define _CRT_SECURE_NO_WARNINGS
#define EWI_UNIT_TESTS

#include "ArduinoEmu.hpp"
#include "..\..\EWI_MAIN\src\Loopstation.hpp"

inline void Test_(bool b, const char *str) {
  if (!b) {
    cc::log("Fail: %s", str);
    DebugBreak();
  }
  else {
    cc::log("Pass: %s", str);
  }
}

#define Test(x) (Test_(x, #x))

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
  mv.mBreath01 = 0.5f;
  stream.ResetBufferForRecording(status, buf, EndPtr(buf));
  stream.Write(mv);
  cc::log("---");
  stream.Dump();
  Test(stream.DebugGetStream().size() == 1);

  status.mCurrentLoopTimeMS = 100;
  mv.mBreath01 = 0.0f;
  mv.mSynthPatch = 5;
  mv.mNeedsNoteOn = true;
  mv.mMidiNote = 20;
  stream.Write(mv);
  cc::log("---");
  stream.Dump();
  Test(stream.DebugGetStream().size() == 4);

  // simulate pressing LoopIt again at 1sec.
  status.mState = LooperState::DurationSet;
  status.mLoopDurationMS = 1000;
  status.mCurrentLoopTimeMS = 0;

  stream.WrapUpRecording();
  cc::log("---");
  stream.Dump();

  Test(stream.DebugGetStream().size() == 4);

  status.mCurrentLoopTimeMS = 0;
  MusicalVoice mvout;
  stream.ReadUntilLoopTime(mvout);
  Test(mvout.mBreath01 == 0.5f); // ON the cursor, we should get the breath change applied.
  stream.ReadUntilLoopTime(mvout);
  Test(mvout.mBreath01 == 0.5f); // reading again at the same place should not change anything.
  status.mCurrentLoopTimeMS = 90;
  stream.ReadUntilLoopTime(mvout); // advancing to before the next event should also not change anything
  Test(mvout.mBreath01 == 0.5f);

  status.mCurrentLoopTimeMS = 110; // advancing to AFTER the event should change things.
  stream.ReadUntilLoopTime(mvout);
  Test(mvout.mBreath01 == 0.0f);
  Test(mvout.mSynthPatch == 5);
  Test(mvout.mMidiNote == 20);

  status.mCurrentLoopTimeMS = 10; // wrapping the loop. we should go back to original state.
  stream.ReadUntilLoopTime(mvout);
  Test(mvout.mBreath01 == 0.5f);
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
  stream.ResetBufferForRecording(status, buf, EndPtr(buf));

  // some events at 10ms, 20ms, 50ms, loop len 1 second.
  status.mCurrentLoopTimeMS = 10;
  mv.mBreath01 = 1;
  stream.Write(mv);
  cc::log("---");
  stream.Dump();
  Test(stream.DebugGetStream().size() == 1);

  status.mCurrentLoopTimeMS = 20;
  mv.mPitchBendN11 = 2;
  stream.Write(mv);
  cc::log("---");
  stream.Dump();
  Test(stream.DebugGetStream().size() == 2);

  status.mCurrentLoopTimeMS = 50;
  mv.mHarmPatch = 5;
  mv.mSynthPatch = 5;
  mv.mBreath01 = 5;
  stream.Write(mv);
  cc::log("---");
  stream.Dump();
  Test(stream.DebugGetStream().size() == 5);

  // simulate pressing LoopIt again at 1sec.
  status.mCurrentLoopTimeMS = 0;
  stream.WrapUpRecording();
  cc::log("---");
  stream.Dump();

  Test(stream.DebugGetStream().size() == 5);

  status.mCurrentLoopTimeMS = 0;
  stream.ReadUntilLoopTime(mvout);

  status.mCurrentLoopTimeMS = 100;
  stream.ReadUntilLoopTime(mvout);

  Test(mvout.mBreath01 == 5);
  Test(mvout.mHarmPatch == 5);
  Test(mvout.mSynthPatch == 5);
  Test(mvout.mPitchBendN11 == 2);
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
  stream.ResetBufferForRecording(status, buf, EndPtr(buf));

  // some events at 10ms, 20ms, 50ms, loop len 1 second.
  status.mCurrentLoopTimeMS = 10;
  mv.mBreath01 = 1;
  stream.Write(mv);
  cc::log("---");
  stream.Dump();
  Test(stream.DebugGetStream().size() == 1);

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
  stream.ResetBufferForRecording(status, buf, EndPtr(buf));

  // some events at 10ms, 20ms, 50ms, loop len 1 second.
  status.mCurrentLoopTimeMS = 10;
  mv.mBreath01 = 1;
  stream.Write(mv);
  stream.Dump();
  Test(stream.DebugGetStream().size() == 1);

  status.mCurrentLoopTimeMS = 20;
  mv.mPitchBendN11 = 2;
  stream.Write(mv);
  stream.Dump();
  Test(stream.DebugGetStream().size() == 2);

  status.mCurrentLoopTimeMS = 50;
  mv.mHarmPatch = 5;
  mv.mSynthPatch = 5;
  mv.mBreath01 = 5;
  stream.Write(mv);
  stream.Dump();
  Test(stream.DebugGetStream().size() == 5);

  // simulate pressing LoopIt again at 1sec.
  stream.WrapUpRecording();

  status.mCurrentLoopTimeMS = 60;
  stream.ReadUntilLoopTime(mvout);
  Test(mvout.mBreath01 == 5);

  status.mCurrentLoopTimeMS = 15;
  stream.ReadUntilLoopTime(mvout);
  Test(mvout.mBreath01 == 1);

  status.mCurrentLoopTimeMS = 14; // should loop all the way around.
  stream.ReadUntilLoopTime(mvout);
  Test(mvout.mBreath01 == 1);
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
  stream.ResetBufferForRecording(status, buf, EndPtr(buf));
  stream.WrapUpRecording();

  status.mCurrentLoopTimeMS = 60;
  stream.ReadUntilLoopTime(mvout);

  status.mCurrentLoopTimeMS = 15;
  stream.ReadUntilLoopTime(mvout);

  status.mCurrentLoopTimeMS = 14; // should loop all the way around.
  stream.ReadUntilLoopTime(mvout);
}

template<typename Tel, typename Tpred>
bool ContainsAny(const std::vector<Tel>& container, Tpred pred) {
  for (auto& el : container) {
    if (pred(el)) {
      return true;
    }
  }
  return false;
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

  stream.ResetBufferForRecording(status, buf, EndPtr(buf));

  status.mCurrentLoopTimeMS = 0;
  mv.mBreath01 = 0;
  stream.Write(mv);

  status.mCurrentLoopTimeMS = 300;
  mv.mBreath01 = 1;
  stream.Write(mv);

  status.mCurrentLoopTimeMS = 600;
  mv.mBreath01 = 2;
  stream.Write(mv);

  status.mCurrentLoopTimeMS = 950;
  mv.mBreath01 = 3;
  stream.Write(mv);

  status.mCurrentLoopTimeMS = 100;
  mv.mBreath01 = 4;
  Test(stream.mWritePrevCursor.mP == nullptr);
  Test(stream.mWriteZeroTimeCursor == stream.mEventsBegin.mP);
  stream.Dump(); // you should see a NOP event of 50ms next to the breath event of 100ms, to sit directly on the loop boundary.
  stream.Write(mv); // this one makes the recording longer than 1 loop.
  stream.Dump(); // you should see a NOP event of 50ms next to the breath event of 100ms, to sit directly on the loop boundary.
  Test(stream.mWritePrevCursor.mP != nullptr);
  Test(stream.mWriteZeroTimeCursor > stream.mEventsBegin.mP);

  // test aligning on a loop boundary (EOL nops)
  auto eventList = stream.DebugGetStream();
  Test(ContainsAny(eventList, [&](const auto& ev) { return ev.mLoopTimeMS == 0; }));
  //Test(ContainsAny(eventList, [&](const auto& ev) { return ev.mLoopTimeMS == 1000; }));

  // and if we examine what's going on at the new zero cursor, we should see 100ms breath=4.
  LoopCursor cz;
  cz.mLoopTimeMS = 0;
  cz.mP = stream.mWriteZeroTimeCursor;
  auto ze = cz.PeekEvent();
  Test(ze.mHeader.mTimeSinceLastEventMS = 100);
  Test(ze.mHeader.mEventType == LoopEventType::Breath);
  Test(ze.mParams.mBreathParams.mBreath01 == 4);

  stream.Dump(); // you should see a NOP event of 50ms next to the breath event of 100ms, to sit directly on the loop boundary.
  stream.WrapUpRecording();
  stream.Dump(); // you should see a NOP event of 50ms next to the breath event of 100ms, to sit directly on the loop boundary.

  // read it back. 
  status.mCurrentLoopTimeMS = 0;
  size_t ec = stream.ReadUntilLoopTime(mvout);
  Test(ec == 0);// there is no event at 0.
  status.mCurrentLoopTimeMS = 100;
  ec = stream.ReadUntilLoopTime(mvout);
  stream.Dump(); // you should see a NOP event of 50ms next to the breath event of 100ms, to sit directly on the loop boundary.
  Test(ec == 1);// breath is changed at 100.
  Test(mvout.mBreath01 == 4);
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

  stream.ResetBufferForRecording(status, buf, buf + 50);

  status.mCurrentLoopTimeMS = 0;
  for (int i = 0; i < 10; ++i) {
    status.mCurrentLoopTimeMS += 5000;
    status.mCurrentLoopTimeMS %= status.mLoopDurationMS;
    mv.mBreath01 = (float)(i + 1);
    stream.Write(mv);
    stream.Dump();
  }

  // Note: if we did not LOCK out recording, then some events could make it into
  // the buffer if they're small enough.
  Test(stream.DebugGetStream().size() == 2);
}

// also tests that we can start writing from beginning again by wrapping, if we have
// enough space in the buffer.
void TestScenarioEPZ()
{
  // |xxE---PxxxxZxx (epz)
  MusicalVoice mv;
  MusicalVoice mvout;
  uint8_t buf[150];
  LoopEventStream stream;

  LoopStatus status;
  status.mState = LooperState::DurationSet;
  status.mLoopDurationMS = 1000; // we want each event to be 5 secs long,
  // and in order to actually trigger this scenario we need to capture prev
  // cursor. so we must cross a loop within the buffer. so make loop small enough.

  stream.ResetBufferForRecording(status, buf, EndPtr(buf));

  status.mCurrentLoopTimeMS = 0;
  for (int i = 0; i < 100; ++i) {
    status.mCurrentLoopTimeMS += 400;
    status.mCurrentLoopTimeMS %= status.mLoopDurationMS;
    mv.mBreath01 = (float)(i + 1);
    stream.Dump();
    stream.Write(mv);
    stream.Dump();
  }


}

void TestScenarioZEP()
{
  // |xxZxxxxE---Pxx (zep)
}


int main()
{
  TestHappyFlow();
  TestConsumeMultipleEvents();
  TestMuted();
  TestReadingAfterLooped();
  TestReadingEmptyBuffer();

  TestScenarioOOM_PartialLoop();

  TestEndRecordingWithFullLoopSimple();
  TestScenarioEPZ();

  //LooperAndHarmonizer l;
  //MusicalVoice mv;
  //MusicalVoice outp[8];
  //srand(GetTickCount());

  //// some events at 0, 100ms, loop len 1 second.

  //uint8_t buf[10000];
  //LoopEventStream stream;
  //LoopStatus status;
  //status.mState = LooperState::DurationSet;
  //status.mLoopDurationMS = 1000;
  //status.mCurrentLoopTimeMS = 0;
  //mv.mBreath01 = 0.5f;
  //stream.ResetBufferForRecording(status, buf, EndPtr(buf));
  //stream.Write(status, mv);
  //cc::log("---");
  //stream.Dump();

  //status.mCurrentLoopTimeMS = 100;
  //mv.mBreath01 = 0.0f;
  //mv.mSynthPatch = 5;
  //mv.mNeedsNoteOn = true;
  //mv.mMidiNote = 20;
  //stream.Write(status, mv);
  //cc::log("---");
  //stream.Dump();


  //stream.WrapUpRecording();
  //cc::log("---");
  //stream.Dump();

  //status.mCurrentLoopTimeMS = 0;
  //MusicalVoice mvout;
  //stream.ReadUntilLoopTime(status, mvout);
  //stream.ReadUntilLoopTime(status, mvout);
  //status.mCurrentLoopTimeMS = 90;
  //stream.ReadUntilLoopTime(status, mvout);
  //status.mCurrentLoopTimeMS = 110;
  //stream.ReadUntilLoopTime(status, mvout);
  //status.mCurrentLoopTimeMS = 10;
  //stream.ReadUntilLoopTime(status, mvout);

  //while (true) {
  //  //mv.mBreath01 = ((float)(rand() % 100)) / 100;
  //  mv.mBreath01 += 0.01f;
  //  mv.mBreath01 -= floorf(mv.mBreath01);
  //  l.Update(mv, outp, EndPtr(outp));

  //  HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
  //  DWORD events;
  //  INPUT_RECORD buffer;
  //  PeekConsoleInput(handle, &buffer, 1, &events);
  //  if (events > 0) {
  //    ReadConsoleInput(handle, &buffer, 1, &events);
  //    if (buffer.EventType == KEY_EVENT) {
  //      if (buffer.Event.KeyEvent.bKeyDown) {
  //        l.LoopIt();
  //      }
  //    }
  //  }
  //}

  return 0;
}
