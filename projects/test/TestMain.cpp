
#define _CRT_SECURE_NO_WARNINGS

#define CLARINOID_PLATFORM_X86
#define CLARINOID_MODULE_TEST

#ifdef _DEBUG
#   define FP_CACHE_DOUBLE
#   define FP_RUNTIME_CHECKS
#endif

#include <clarinoid/x86/ArduinoEmu.hpp>

namespace clarinoid
{
static constexpr size_t gBufferedStreamBufferSize = 64;

#define BASSOONOID1
#define THREE_BUTTON_OCTAVES

const char gClarinoidVersion[] = "BASSOONOID v0.01";

static constexpr size_t MODAL_DIALOG_STACK_SIZE = 10;

static const size_t MAX_SYNTH_VOICES = 6;

static const size_t HARM_PRESET_COUNT = 16;
static const size_t HARM_VOICES = 6;
static const size_t HARM_SEQUENCE_LEN = 8;
static const size_t SYNTH_MODULATIONS_MAX = 8;
static const size_t PERFORMANCE_PATCH_COUNT = 16;

static const int DEFAULT_TRANSPOSE = 12;

static const size_t LOOP_LAYERS = 6;
static constexpr size_t MAX_MUSICAL_VOICES =
    LOOP_LAYERS *
    (HARM_VOICES + 1 /* each harmonized preset can also output the playing (live) note as well, so make room.*/);

static const size_t PRESET_NAME_LEN = 16;

static const size_t SYNTH_PRESET_COUNT = 32;

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

#include <clarinoid/basic/BasicBasic.hpp>
#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/menu/MenuAppSynthSettings.hpp>

#include "TestScaleFollower.hpp"
#include "TestMemory.hpp"
#include "TestLoopstation.hpp"
#include "TestScales.hpp"
#include "TestHarmonizer.hpp"
#include "TestTaskManager.hpp"
#include "TestInputMapping.hpp"
#include "TestFilter.hpp"
#include "TestModulation.hpp"
#include "TestJSON.hpp"
#include "TestFixedPoint.hpp"

namespace clarinoid
{

struct TestHUD : IHudProvider
{
    virtual int16_t IHudProvider_GetHudHeight() override
    {
        return 0;
    }
    virtual void IHudProvider_RenderHud(int16_t displayWidth, int16_t displayHeight)
    {
    
    }
};



void TestMenu()
{
    CCAdafruitSSD1306 gDisplay = {128, 64, nullptr, 40 /*DC*/, 41 /*RST*/, 10 /*CS*/, 88 * 1000000UL};
    _CCDisplay display{gDisplay};
    InputDelegator inputDelegator;
    TestControlMapper controlMapper;
    AppSettings appSettings;

    inputDelegator.Init(&appSettings, &controlMapper);

    SynthPatchMenuApp synthPatchApp{display, appSettings, inputDelegator};

    IDisplayApp *allApps[] = {
        &synthPatchApp,
    };

    TestHUD hud;

    display.Init(&appSettings, &inputDelegator, &hud, allApps);

    display.UpdateAndRenderTask();
    display.DisplayTask();
}
} // namespace clarinoid

int main()
// int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR lpCmdLine, int)
{
    // TestFilter();
    //  TestFilterPerformance();

    // TestModulation();

    clarinoid ::TestFixedPoint();

    clarinoid::TestMenu();

    //clarinoid::TestInputDelegator(); // test that input, mapping, and functions work.

    // TestBufferUnification();

    // TestReadHeader();
    // Test12BitParam();
    // TestDivRem();

    // TestHappyFlow();
    // TestConsumeMultipleEvents();
    // TestMuted();
    // TestReadingAfterLooped();
    // TestReadingEmptyBuffer();

    // TestScenarioOOM_PartialLoop();

    // TestEndRecordingWithFullLoopSimple(); // PZE
    // TestScenarioEP();

    // TestFullMusicalState1();
    // TestFullMusicalState2();

    // TestVoiceID();
    // TestLoopstationLegato();
    // TestLoopstationSynth();

    clarinoid::TestTaskManager();

    // TestScales();

    // TestScaleFollower();
    // TestHarmonizer();

    clarinoid::TestJSON();

    TestSummary();
    return 0;
}
