
#define _CRT_SECURE_NO_WARNINGS

#define CLARINOID_PLATFORM_X86
#define CLARINOID_MODULE_TEST

#include <clarinoid/x86/ArduinoEmu.hpp>

namespace clarinoid
{
  #define BASSOONOID1
  #define THREE_BUTTON_OCTAVES

  const char gClarinoidVersion[] = "BASSOONOID v0.01";

  static const size_t MAX_SYNTH_VOICES = 6;

  static const size_t HARM_PRESET_COUNT = 16;
  static const size_t HARM_VOICES = 6;
  static const size_t HARM_SEQUENCE_LEN = 8;
  static const size_t SYNTH_MODULATIONS_MAX = 8;
  static const size_t PERFORMANCE_PATCH_COUNT = 16;

  static const int DEFAULT_TRANSPOSE = 12;

  static const size_t LOOP_LAYERS = 6;
  static constexpr size_t MAX_MUSICAL_VOICES = LOOP_LAYERS * (HARM_VOICES + 1 /* each harmonized preset can also output the playing (live) note as well, so make room.*/);

  static const size_t PRESET_NAME_LEN = 16;

  static const size_t SYNTH_PRESET_COUNT = 16;

  static const size_t MAPPED_CONTROL_SEQUENCE_LENGTH = 4; // how many items in the "mapped control value sequence"

  static const size_t LOOPER_MEMORY_TOTAL_BYTES = 192000; // should be enough right?
  static const size_t LOOPER_TEMP_BUFFER_BYTES = 8192;    // a smaller buffer that's just used for intermediate copy ops

  // assignable slots.
  static const size_t MAX_CONTROL_MAPPINGS = 64;

  enum class PhysicalControl : uint8_t
  {
    Button1,
    Button2,
    Axis1,
    Axis2,
    Encoder1,
    Encoder2,
    COUNT,
  };

} // namespace clarinoid


#include "Test.hpp"
#include "TestScaleFollower.hpp"
#include "TestMemory.hpp"
#include "TestLoopstation.hpp"
#include "TestScales.hpp"
#include "TestHarmonizer.hpp"
#include "TestTaskManager.hpp"
#include "TestInputMapping.hpp"
#include "TestFilter.hpp"

int main()
//int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR lpCmdLine, int)
{
  //TestFilter();
  TestFilterPerformance();

  //clarinoid::TestInputDelegator(); // test that input, mapping, and functions work.
  //TestControlReaders();
  //TestInputMappings(); // test conditions 

  //TestBufferUnification();

  //TestReadHeader();
  //Test12BitParam();
  //TestDivRem();

  //TestHappyFlow();
  //TestConsumeMultipleEvents();
  //TestMuted();
  //TestReadingAfterLooped();
  //TestReadingEmptyBuffer();

  //TestScenarioOOM_PartialLoop();

  //TestEndRecordingWithFullLoopSimple(); // PZE
  //TestScenarioEP();

  //TestFullMusicalState1();
  //TestFullMusicalState2();

  //TestVoiceID();
  //TestLoopstationLegato();
  //TestLoopstationSynth();

  //clarinoid::TestTaskManager();

  //TestScales();

  //TestScaleFollower();
  //TestHarmonizer();

  return 0;
}
