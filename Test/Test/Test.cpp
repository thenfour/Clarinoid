

#define _CRT_SECURE_NO_WARNINGS
#define EWI_UNIT_TESTS

//#define EWI_LOOP_USE_MARKER

#include "..\..\Shared_x86\ArduinoEmu.hpp"
#include "..\..\EWI_MAIN\src\Loopstation.hpp"

static size_t gTestCount = 0;
static size_t gTestPassCount = 0;

inline void Test_(bool b, const char *str) {
  gTestCount++;
  if (!b) {
    cc::log("[%d] THIS FAILED: >>> %s <<<", gTestCount, str);
    DebugBreak();
  }
  else {
    gTestPassCount++;
    cc::log("[%d] Pass: %s", gTestCount, str);
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
  stream.StartRecording(status, mv, buf, EndPtr(buf));
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

  Test(stream.DebugGetStream().size() == 7);

  status.mCurrentLoopTimeMS = 0;
  MusicalVoice mvout;
  stream.Dump();
  stream.ReadUntilLoopTime(mvout);
  stream.Dump();
  Test(mvout.mBreath01 == 0.5f); // ON the cursor, we should get the breath change applied.
  stream.ReadUntilLoopTime(mvout);
  stream.Dump();
  Test(mvout.mBreath01 == 0.5f); // reading again at the same place should not change anything.
  status.mCurrentLoopTimeMS = 90;
  stream.ReadUntilLoopTime(mvout); // advancing to before the next event should also not change anything
  stream.Dump();
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
  stream.StartRecording(status, mv, buf, EndPtr(buf));

  // some events at 10ms, 20ms, 50ms, loop len 1 second.
  status.mCurrentLoopTimeMS = 10;
  mv.mBreath01 = 1;
  stream.Write(mv);
  cc::log("---");
  stream.Dump();
  Test(stream.DebugGetStream().size() == 2);

  status.mCurrentLoopTimeMS = 20;
  mv.mPitchBendN11 = 2;
  stream.Write(mv);
  cc::log("---");
  stream.Dump();
  Test(stream.DebugGetStream().size() == 3);

  status.mCurrentLoopTimeMS = 50;
  mv.mHarmPatch = 5;
  mv.mSynthPatch = 5;
  mv.mBreath01 = 5;
  stream.Write(mv);
  cc::log("---");
  stream.Dump();
  Test(stream.DebugGetStream().size() == 6);

  // simulate pressing LoopIt again at 1sec.
  status.mCurrentLoopTimeMS = 0;
  stream.WrapUpRecording();
  cc::log("---");
  stream.Dump();

  Test(stream.DebugGetStream().size() == 8);

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
  stream.StartRecording(status, mv, buf, EndPtr(buf));

  // some events at 10ms, 20ms, 50ms, loop len 1 second.
  status.mCurrentLoopTimeMS = 10;
  mv.mBreath01 = 1;
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
  stream.StartRecording(status, mv, buf, EndPtr(buf));

  // some events at 10ms, 20ms, 50ms, loop len 1 second.
  status.mCurrentLoopTimeMS = 10;
  mv.mBreath01 = 1;
  stream.Write(mv);
  stream.Dump();
  Test(stream.DebugGetStream().size() == 2);

  status.mCurrentLoopTimeMS = 20;
  mv.mPitchBendN11 = 2;
  stream.Write(mv);
  stream.Dump();
  Test(stream.DebugGetStream().size() == 3);

  status.mCurrentLoopTimeMS = 50;
  mv.mHarmPatch = 5;
  mv.mSynthPatch = 5;
  mv.mBreath01 = 5;
  stream.Write(mv);
  stream.Dump();
  Test(stream.DebugGetStream().size() == 6);

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
  stream.StartRecording(status, mv, buf, EndPtr(buf));
  stream.Dump();
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
  status.mCurrentLoopTimeMS = 0;

  stream.StartRecording(status, mv, buf, EndPtr(buf));

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
  //Test(stream.mPrevCursor.mP == nullptr);
  //Test(stream.mRecStartCursor.mP == stream.mBufferBegin.mP);
  stream.Dump(); // you should see a NOP event of 50ms next to the breath event of 100ms, to sit directly on the loop boundary.
  stream.Write(mv); // this one makes the recording longer than 1 loop.
  stream.Dump(); // you should see a NOP event of 50ms next to the breath event of 100ms, to sit directly on the loop boundary.
  Test(stream.mPrevCursor.mP != nullptr);
  //Test(stream.mRecStartCursor.mP > stream.mBufferBegin.mP);

  //// test aligning on a loop boundary (EOL nops)
  //auto eventList = stream.DebugGetStream();
  //Test(ContainsAny(eventList, [&](const auto& ev) { return ev.mLoopTimeMS == 0; }));
  ////Test(ContainsAny(eventList, [&](const auto& ev) { return ev.mLoopTimeMS == 1000; }));

  // and if we examine what's going on at the new zero cursor, we should see 100ms breath=4.
  //LoopCursor cz;
  //cz.mLoopTimeMS = 0;
  //cz.mP = stream.mRecStartCursor.mP;
  //auto ze = cz.PeekEvent();
  //Test(ze.mHeader.mTimeSinceLastEventMS = 100);
  //Test(ze.mHeader.mEventType == LoopEventType::Breath);
  //Test(ze.mParams.mBreathParams.mBreath01 == 4);

  //stream.Dump();
  //status.mCurrentLoopTimeMS = 0;
  stream.WrapUpRecording();
  stream.Dump();

  // read it back. 
  status.mCurrentLoopTimeMS = 0;
  size_t ec = stream.ReadUntilLoopTime(mvout);
  stream.Dump();
  //Test(ec == 0);// there is no event at 0.
  status.mCurrentLoopTimeMS = 100;
  ec = stream.ReadUntilLoopTime(mvout);
  stream.Dump();
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

  stream.StartRecording(status, mv, buf, buf + 50);

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

  stream.StartRecording(status, mv, buf, EndPtr(buf));

  status.mCurrentLoopTimeMS = 0;
  for (int i = 0; i < 100; ++i) {
    status.mCurrentLoopTimeMS += 125;
    status.mCurrentLoopTimeMS %= status.mLoopDurationMS;
    mv.mBreath01 = (float)(i + 1);
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
  // [5ac:29e4]     [0135FB08 (+0) t=250: dly=125, type=Breath, params=11]
  // [5ac:29e4]     [0135FB18 (+16) t=375: dly=125, type=Breath, params=12]
  // [5ac:29e4]     [0135FB28 (+32) t=500: dly=125, type=Breath, params=13]
  // [5ac:29e4]     [0135FB38 (+48) t=625: dly=125, type=Breath, params=14]
  // [5ac:29e4]     [0135FB48 (+64) t=750: dly=125, type=Breath, params=15]
  // [5ac:29e4]     [0135FB58 (+80) t=875: dly=125, type=Nop, params=]
  // [5ac:29e4] Z   [0135FB64 (+92) t=0: dly=0, type=Breath, params=16]
  // [5ac:29e4]     [0135FB74 (+108) t=0: dly=125, type=Breath, params=17]
  // [5ac:29e4]  E  [0135FB84 (+124) t=125: dly=125, type=Breath, params=2]
  // [5ac:29e4]     buf used: 140, event count: 9 , validEnd:0135FB94, bufend:0135FB94
  status.mCurrentLoopTimeMS = 250;
  size_t ec = stream.ReadUntilLoopTime(mvout);
  stream.Dump();
  // we have to read the events above
  Test(mvout.mBreath01 == 34);
  status.mCurrentLoopTimeMS = 0;
  ec = stream.ReadUntilLoopTime(mvout);
  stream.Dump();
  // we have to read the events above
  Test(ec == 7);
  Test(mvout.mBreath01 == 32);
}


void TestFullMusicalState1()
{
  // |xxE---PxxxxZxx (epz)
  MusicalVoice mv;
  MusicalVoice mvout;
  uint8_t buf[300];
  LoopEventStream stream;

  LoopStatus status;
  status.mState = LooperState::DurationSet;
  status.mLoopDurationMS = 60000;

  // start with some state @ 1000ms.
  status.mCurrentLoopTimeMS = 1000;
  mv.mBreath01 = 1001.0f;
  mv.mPitchBendN11 = 1002.0f;
  mv.mSynthPatch = 1003;
  mv.mHarmPatch = 1004;

  stream.StartRecording(status, mv, buf, EndPtr(buf));
  stream.Dump();

  // we somehow wrap all the way around to t=3 with totalyl new state.
  // |-n             nnn-----------------|
  status.mCurrentLoopTimeMS = 3;
  mv.mBreath01 = 1;
  mv.mPitchBendN11 = 2;
  mv.mSynthPatch = 3;
  mv.mHarmPatch = 4;
  stream.Write(mv);
  stream.Dump();

  // and at t=50ms with different state.
  // |-n---nn        nnn-----------------|
  status.mCurrentLoopTimeMS = 50;
  mv.mPitchBendN11 = 52;
  mv.mSynthPatch = 53;
  stream.Write(mv);
  stream.Dump();

  stream.WrapUpRecording();
  stream.Dump();

  // check musical state against what we wrote.
  status.mCurrentLoopTimeMS = 1000;
  stream.ReadUntilLoopTime(mvout);
  stream.Dump();
  Test(mvout.mBreath01 == 1001.0f);
  Test(mvout.mPitchBendN11 == 1002.0f);
  Test(mvout.mSynthPatch == 1003);
  Test(mvout.mHarmPatch == 1004);

  //// and then end of buffer. THIS overwrites the initial state recorded!
  //// |-n---nn-----------------------nn----|
  //status.mCurrentLoopTimeMS = 59000;
  //mv.mBreath01 = 880;
  //mv.mPitchBendN11 = 881;
  //mv.mSynthPatch = 882;
  //mv.mHarmPatch = 883;
  //stream.Write(mv);
  //stream.Dump();
}

void TestFullMusicalState2()
{
  // |xxE---PxxxxZxx (epz)
  MusicalVoice mv;
  MusicalVoice mvout;
  uint8_t buf[300];
  LoopEventStream stream;

  LoopStatus status;
  status.mState = LooperState::DurationSet;
  status.mLoopDurationMS = 60000;

  // start with some state @ 1000ms.
  status.mCurrentLoopTimeMS = 1000;
  mv.mBreath01 = 1001.0f;
  mv.mPitchBendN11 = 1002.0f;
  mv.mSynthPatch = 1003;
  mv.mHarmPatch = 1004;

  stream.StartRecording(status, mv, buf, EndPtr(buf));
  stream.Dump();

  // we somehow wrap all the way around to t=3 with totalyl new state.
  status.mCurrentLoopTimeMS = 3;
  mv.mBreath01 = 1;
  mv.mPitchBendN11 = 2;
  mv.mSynthPatch = 3;
  mv.mHarmPatch = 4;
  stream.Write(mv);
  stream.Dump();

  // and at t=50ms with different state.
  status.mCurrentLoopTimeMS = 50;
  mv.mPitchBendN11 = 52;
  mv.mSynthPatch = 53;
  stream.Write(mv);
  stream.Dump();

  // and then for fun
  status.mCurrentLoopTimeMS = 59000;
  mv.mBreath01 = 880;
  mv.mPitchBendN11 = 881;
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
  Test(mvout.mBreath01 == 1);
  Test(mvout.mPitchBendN11 == 52);
  Test(mvout.mSynthPatch == 53);
  Test(mvout.mHarmPatch == 4);
}

int main()
{
  {
    uint8_t b[] = "Zabcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789";
    auto s = UnifyCircularBuffer_Left<10>(b + 1, EndPtr(b) - 1, b, b + 1);
    Test(strcmp("abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789Z", (const char*)b) == 0);
    Test(*(EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "4567123";
    auto s = UnifyCircularBuffer_Left(b + 4, EndPtr(b) - 1, b, b + 4);
    Test(strncmp("1234567", (const char*)b, 7) == 0);
    Test(*(EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "456123";
    UnifyCircularBuffer_Left(b + 3, EndPtr(b) - 1, b, b + 3);
    Test(strncmp("123456", (const char*)b, 6) == 0);
    Test(*(EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "45123";
    UnifyCircularBuffer_Left(b + 2, EndPtr(b) - 1, b, b + 2);
    Test(strncmp("12345", (const char*)b, 5) == 0);
    Test(*(EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "4123";
    UnifyCircularBuffer_Left(b + 1, EndPtr(b) - 1, b, b + 1);
    Test(strncmp("1234", (const char*)b, 4) == 0);
    Test(*(EndPtr(b) - 1) == 0);
  }


  {
    uint8_t b[] = "34..12";
    UnifyCircularBuffer_Left(b + 4, EndPtr(b) - 1, b, b + 2);
    Test(strncmp("1234", (const char*)b, 4) == 0);
    Test(*(EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "3..12";
    UnifyCircularBuffer_Left(b + 3, EndPtr(b) - 1, b, b + 1);
    Test(strncmp("123", (const char*)b, 3) == 0);
    Test(*(EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "23..1";
    UnifyCircularBuffer_Left(b + 4, EndPtr(b) - 1, b, b + 2);
    Test(strncmp("123", (const char*)b, 3) == 0);
    Test(*(EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "9..12345678";
    UnifyCircularBuffer_Left(b + 3, EndPtr(b) - 1, b, b + 1);
    Test(strncmp("123456789", (const char*)b, 9) == 0);
    Test(*(EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "23456789..1";
    UnifyCircularBuffer_Left(b + 10, EndPtr(b) - 1, b, b + 8);
    Test(strncmp("123456789", (const char*)b, 9) == 0);
    Test(*(EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "456789.123";
    UnifyCircularBuffer_Left(b + 7, EndPtr(b) - 1, b, b + 6);
    Test(strncmp("123456789", (const char*)b, 9) == 0);
    Test(*(EndPtr(b) - 1) == 0); // no overruns.
  }

  // test 0-sized segments.
  {
    uint8_t b[] = "123";
    UnifyCircularBuffer_Left(b, EndPtr(b) - 1, b, b);
    Test(strncmp("123", (const char*)b, 3) == 0);
    Test(*(EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "123";
    UnifyCircularBuffer_Left(EndPtr(b) - 1, EndPtr(b) - 1, b, EndPtr(b) - 1);
    Test(strncmp("123", (const char*)b, 3) == 0);
    Test(*(EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "";
    UnifyCircularBuffer_Left(b, EndPtr(b) - 1, 0, 0);
    Test(*(EndPtr(b) - 1) == 0);
  }



  TestHappyFlow();
  TestConsumeMultipleEvents();
  TestMuted();
  TestReadingAfterLooped();
  TestReadingEmptyBuffer();

  TestScenarioOOM_PartialLoop();

  TestEndRecordingWithFullLoopSimple(); // PZE
  TestScenarioEP();

  TestFullMusicalState1();
  TestFullMusicalState2();

  return 0;
}
