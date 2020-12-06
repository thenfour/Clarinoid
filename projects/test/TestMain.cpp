
#define _CRT_SECURE_NO_WARNINGS

#define CLARINOID_PLATFORM_X86
#define CLARINOID_MODULE_TEST

#include <clarinoid/x86/ArduinoEmu.hpp>

#include "Test.hpp"
#include "TestScaleFollower.hpp"
#include "TestMemory.hpp"
#include "TestLoopstation.hpp"
#include "TestScales.hpp"
#include "TestHarmonizer.hpp"
#include "TestTaskManager.hpp"

int main()
{
  TestBufferUnification();

  TestReadHeader();
  Test12BitParam();
  TestDivRem();

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

  TestVoiceID();
  TestLoopstationLegato();
  TestLoopstationSynth();

  clarinoid::TestTaskManager();

  TestScales();

  //TestScaleFollower();
  //TestHarmonizer();

  return 0;
}
