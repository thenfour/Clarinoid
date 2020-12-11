
#pragma once

#define CLARINOID_PLATFORM_TEENSY
#define CLARINOID_MODULE_MAIN // as opposed to some submodules like LH / RH
#define BASSOONOID1
#define THREE_BUTTON_OCTAVES

namespace clarinoid
{

PROGMEM const char gClarinoidVersion[] = "BASSOONOID v0.01";

static const size_t MAX_SYNTH_VOICES = 6;

static const size_t HARM_PRESET_COUNT = 16;
static const size_t HARM_VOICES = 6;
static const size_t HARM_SEQUENCE_LEN = 8;

static const size_t LOOP_LAYERS = 6;
static constexpr size_t MAX_MUSICAL_VOICES = LOOP_LAYERS * (HARM_VOICES + 1 /* each harmonized preset can also output the playing (live) note as well, so make room.*/);

static const size_t PRESET_NAME_LEN = 16;

static const size_t SYNTH_PRESET_COUNT = 16;

//static const size_t HARDWARE_BUTTON_COUNT = 33; // 2xmcp23017, 1 toggle switch
//static const size_t HARDWARE_AXIS_COUNT = 5;// breath, vol, joyx, joyy, pitchbend
//static const size_t HARDWARE_ENCODER_COUNT = 3;

enum class PhysicalSwitch : uint8_t
{
    CPBack,
    CPOk,
    CPToggleUp,
    CPEnc,
    LHx1,
    LHx2,
    LHx3,
    LHx4,
    LHEnc,
    LHBack,
    LHOk,
    LHThx1,
    LHThx2,
    LHOct1,
    LHOct2,
    LHOct3,
    LHKey1,
    LHKey2,
    LHKey3,
    LHKey4,
    RHTh1,
    RHTh2,
    RHTh3,
    //RHJoy,
    //RHEnc,
    RHx1,
    RHx2,
    RHx3,
    RHx4,
    RHx5,
    RHKey1,
    RHKey2,
    RHKey3,
    RHKey4,
    COUNT,
};

enum class PhysicalAxis : uint8_t
{
    Breath,
    Pitch,
    JoyX,
    JoyY,
    Volume,
    COUNT,
};

enum class PhysicalEncoder : uint8_t
{
    CP,
    LH,
    RH,
    COUNT,
};

// assignable slots.
static const size_t MAX_BUTTON_MAPPINGS = 64;
static const size_t MAX_AXIS_MAPPINGS = 32;
static const size_t MAX_ENCODER_MAPPINGS = 32;

} // namespace clarinoid

#include <clarinoid/basic/Basic.hpp>

#include <clarinoid/components/Switch.hpp>
#include <clarinoid/components/Leds.hpp>

#include <clarinoid/components/Encoder.hpp>
#include <clarinoid/components/Potentiometer.hpp>
#include <clarinoid/components/HoneywellABPI2C.hpp>
#include <clarinoid/components/MCP23017.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include <clarinoid/application/Display.hpp>
#include <clarinoid/application/ControlMapper.hpp>
#include <clarinoid/menu/MenuAppBase.hpp>
#include <clarinoid/menu/MenuAppSystemSettings.hpp>
#include <clarinoid/application/MusicalState.hpp>

#include <clarinoid/synth/Synth.hpp>

#include <clarinoid/bassoonoid/bsLed.hpp>
#include <clarinoid/bassoonoid/bsControlMapper.hpp>
#include <clarinoid/bassoonoid/DebugDisplayApp.hpp>
#include "MusicalStateTask.hpp"

// MIDI library is touchy about how you instantiate.
// Simplest is to do it the way it's designed for: in the main sketch, global scope.
//MIDI_CREATE_INSTANCE(HardwareSerial, Serial8, gMIDI);

namespace clarinoid
{


struct BassoonoidApp
{
    Leds1 mLed1;
    Leds2 mLed2;
    BreathLED mBreathLED;
    InputDelegator mInputDelegator;
    BassoonoidControlMapper mControlMapper;
    CCDisplay mDisplay;
    AppSettings mAppSettings;

    MusicalStateTask mMusicalStateTask;

    PerformanceApp mPerformanceApp;
    DebugDisplayApp mDebugDisplayApp;
    SystemSettingsApp mSystemSettingsApp;

    BassoonoidApp() :
        mDisplay(128, 64, &SPI, 9/*DC*/, 8/*RST*/, 10/*CS*/, 44 * 1000000UL),
        mMusicalStateTask(&mAppSettings, &mInputDelegator, &mControlMapper),
        mPerformanceApp(mDisplay, &mMusicalStateTask, &mControlMapper),
        mDebugDisplayApp(mDisplay, mControlMapper, mMusicalStateTask),
        mSystemSettingsApp(mDisplay)
    {
    }

    void Main()
    {
        IDisplayApp* allApps[] =
        {
            &mPerformanceApp,
            &mDebugDisplayApp,
            &mSystemSettingsApp,
        };

        mInputDelegator.Init(&mAppSettings, &mControlMapper);

        mAppSettings.mButtonMappings[0] = ButtonMapping::MomentaryMapping(PhysicalSwitch::LHOk, ButtonMapping::Destination::MenuOK);
        mAppSettings.mButtonMappings[1] = ButtonMapping::MomentaryMapping(PhysicalSwitch::LHBack, ButtonMapping::Destination::MenuBack);

        mAppSettings.mButtonMappings[2] = ButtonMapping::MomentaryMapping(PhysicalSwitch::LHOct1, ButtonMapping::Destination::Oct1);
        mAppSettings.mButtonMappings[3] = ButtonMapping::MomentaryMapping(PhysicalSwitch::LHOct2, ButtonMapping::Destination::Oct2);
        mAppSettings.mButtonMappings[4] = ButtonMapping::MomentaryMapping(PhysicalSwitch::LHOct3, ButtonMapping::Destination::Oct3);

        mAppSettings.mButtonMappings[5] = ButtonMapping::MomentaryMapping(PhysicalSwitch::LHKey1, ButtonMapping::Destination::LH1);
        mAppSettings.mButtonMappings[6] = ButtonMapping::MomentaryMapping(PhysicalSwitch::LHKey2, ButtonMapping::Destination::LH2);
        mAppSettings.mButtonMappings[7] = ButtonMapping::MomentaryMapping(PhysicalSwitch::LHKey3, ButtonMapping::Destination::LH3);
        mAppSettings.mButtonMappings[8] = ButtonMapping::MomentaryMapping(PhysicalSwitch::LHKey4, ButtonMapping::Destination::LH4);

        mAppSettings.mButtonMappings[9] = ButtonMapping::MomentaryMapping(PhysicalSwitch::RHKey1, ButtonMapping::Destination::RH1);
        mAppSettings.mButtonMappings[10] = ButtonMapping::MomentaryMapping(PhysicalSwitch::RHKey2, ButtonMapping::Destination::RH2);
        mAppSettings.mButtonMappings[11] = ButtonMapping::MomentaryMapping(PhysicalSwitch::RHKey3, ButtonMapping::Destination::RH3);
        mAppSettings.mButtonMappings[12] = ButtonMapping::MomentaryMapping(PhysicalSwitch::RHKey4, ButtonMapping::Destination::RH4);

        mAppSettings.mAxisMappings[0] = AxisMapping::SimpleMapping(PhysicalAxis::Breath, AxisMapping::Destination::Breath);

        mAppSettings.mEncoderMappings[0] = EncoderMapping::SimpleMapping(PhysicalEncoder::LH, EncoderMapping::Destination::MenuScroll);

        mDisplay.Init(&mAppSettings, &mInputDelegator, allApps);
        mMusicalStateTask.Init();

        FunctionTask mDisplayTask1 { this, [](void* cap){
            BassoonoidApp* pThis = (BassoonoidApp*)cap;
            pThis->mDisplay.UpdateAndRenderTask();
        } };

        FunctionTask mDisplayTask2 { this, [](void* cap){
            BassoonoidApp* pThis = (BassoonoidApp*)cap;
            pThis->mDisplay.DisplayTask();
        } };

        // there's no need to have a sophisticated task manager which decides which task
        // to run based on periodicity, priority, etc.
        // we have a static number of tasks which are very easy to manually piece together
        // into an execution plan.
        //
        // the "Musical state" task tends to take about 1000 microseconds, and is the most
        // critical. So let's run it every 2000 microseconds.
        // that means every other task can just get interlaced between these.
        // Other tasks can run much slower; even at 60fps we have a whole period of 16667 micros.
        // that means running musical state 8 times and everything else gets placed between.
        NopTask nopTask;

        TaskPlanner tp {
            TaskPlanner::TaskDeadline { TimeSpan::FromMicros(0), &mMusicalStateTask, "MusS0" },
            TaskPlanner::TaskDeadline { TimeSpan::FromMicros(1000), &mDisplayTask1, "Display1" },
            TaskPlanner::TaskDeadline { TimeSpan::FromMicros(2000), &mMusicalStateTask, "MusS1" },
            TaskPlanner::TaskDeadline { TimeSpan::FromMicros(3000), &mDisplayTask2, "Display2" },
            TaskPlanner::TaskDeadline { TimeSpan::FromMicros(4000), &mMusicalStateTask, "MusS2" },
            TaskPlanner::TaskDeadline { TimeSpan::FromMicros(5000), &mLed1, "mLed1" },
            TaskPlanner::TaskDeadline { TimeSpan::FromMicros(6000), &mMusicalStateTask, "MusS3" },
            TaskPlanner::TaskDeadline { TimeSpan::FromMicros(7000), &mLed2, "mLed2" },
            TaskPlanner::TaskDeadline { TimeSpan::FromMicros(8000), &mMusicalStateTask, "MusS4" },
            TaskPlanner::TaskDeadline { TimeSpan::FromMicros(9000), &mBreathLED, "mBreathLED" },
            TaskPlanner::TaskDeadline { TimeSpan::FromMicros(10000), &mMusicalStateTask, "MusS5" },
            TaskPlanner::TaskDeadline { TimeSpan::FromMicros(12000), &mMusicalStateTask, "MusS6" },
            TaskPlanner::TaskDeadline { TimeSpan::FromMicros(14000), &mMusicalStateTask, "MusS7" },
            TaskPlanner::TaskDeadline { TimeSpan::FromMicros(16000), &nopTask, "Nop" },
        };

        mPerformanceApp.Init(&tp);

        tp.Main();
    }
};

} // namespace clarinoid
