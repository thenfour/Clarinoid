
#pragma once

#define CLARINOID_PLATFORM_TEENSY
#define CLARINOID_MODULE_MAIN // as opposed to some submodules like LH / RH
#define BASSOONOID1
#define THREE_BUTTON_OCTAVES

namespace clarinoid
{

const char gClarinoidVersion[] = "BASSOONOID v0.01";

static const size_t MAX_SYNTH_VOICES = 6;

static const size_t HARM_PRESET_COUNT = 16;
static const size_t HARM_VOICES = 6;
static const size_t HARM_SEQUENCE_LEN = 8;

static const size_t LOOP_LAYERS = 6;
static constexpr size_t MAX_MUSICAL_VOICES = LOOP_LAYERS * (HARM_VOICES + 1 /* each harmonized preset can also output the playing (live) note as well, so make room.*/);

static const size_t PRESET_NAME_LEN = 16;

static const size_t SYNTH_PRESET_COUNT = 16;

static const size_t MAPPED_CONTROL_SEQUENCE_LENGTH = 4; // how many items in the "mapped control value sequence"

enum class PhysicalControl : uint8_t
{
    CPBack,
    CPOk,
    CPToggleUp,
    CPEncButton,
    LHx1,
    LHx2,
    LHx3,
    LHx4,
    LHEncButton,
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
    //RHJoyButton,
    //RHEncButton,
    RHx1,
    RHx2,
    RHx3,
    RHx4,
    RHx5,
    RHKey1,
    RHKey2,
    RHKey3,
    RHKey4,

    Breath,
    Pitch,
    JoyX,
    JoyY,
    Volume,

    CPEnc,
    LHEnc,
    RHEnc,

    COUNT,
};

// assignable slots.
static const size_t MAX_CONTROL_MAPPINGS = 64;

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

        mAppSettings.mControlMappings[0] = ControlMapping::MomentaryMapping(PhysicalControl::LHOk, ControlMapping::Function::MenuOK);
        mAppSettings.mControlMappings[1] = ControlMapping::MomentaryMapping(PhysicalControl::LHBack, ControlMapping::Function::MenuBack);

        mAppSettings.mControlMappings[2] = ControlMapping::MomentaryMapping(PhysicalControl::LHOct1, ControlMapping::Function::Oct1);
        mAppSettings.mControlMappings[3] = ControlMapping::MomentaryMapping(PhysicalControl::LHOct2, ControlMapping::Function::Oct2);
        mAppSettings.mControlMappings[4] = ControlMapping::MomentaryMapping(PhysicalControl::LHOct3, ControlMapping::Function::Oct3);

        mAppSettings.mControlMappings[5] = ControlMapping::MomentaryMapping(PhysicalControl::LHKey1, ControlMapping::Function::LH1);
        mAppSettings.mControlMappings[6] = ControlMapping::MomentaryMapping(PhysicalControl::LHKey2, ControlMapping::Function::LH2);
        mAppSettings.mControlMappings[7] = ControlMapping::MomentaryMapping(PhysicalControl::LHKey3, ControlMapping::Function::LH3);
        mAppSettings.mControlMappings[8] = ControlMapping::MomentaryMapping(PhysicalControl::LHKey4, ControlMapping::Function::LH4);

        mAppSettings.mControlMappings[9] = ControlMapping::MomentaryMapping(PhysicalControl::RHKey1, ControlMapping::Function::RH1);
        mAppSettings.mControlMappings[10] = ControlMapping::MomentaryMapping(PhysicalControl::RHKey2, ControlMapping::Function::RH2);
        mAppSettings.mControlMappings[11] = ControlMapping::MomentaryMapping(PhysicalControl::RHKey3, ControlMapping::Function::RH3);
        mAppSettings.mControlMappings[12] = ControlMapping::MomentaryMapping(PhysicalControl::RHKey4, ControlMapping::Function::RH4);

        mAppSettings.mControlMappings[0] = ControlMapping::BreathMapping(PhysicalControl::Breath, ControlMapping::Function::Breath);

        mAppSettings.mControlMappings[0] = ControlMapping::MenuScrollMapping(PhysicalControl::LHEnc, ControlMapping::Function::MenuScrollA);

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
