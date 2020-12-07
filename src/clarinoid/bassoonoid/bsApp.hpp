
#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/components/Switch.hpp>
#include <clarinoid/components/Leds.hpp>

#include <clarinoid/components/Encoder.hpp>
#include <clarinoid/components/Potentiometer.hpp>
#include <clarinoid/components/HoneywellABPI2C.hpp>
#include <clarinoid/components/MCP23017.hpp>
#include <clarinoid/application/ControlMapper.hpp>
#include <clarinoid/application/Display.hpp>
#include <clarinoid/settings/AppSettings.hpp>
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
    BassoonoidControlMapper mControlMapper;
    CCDisplay mDisplay;
    AppSettings mAppSettings;

    MusicalStateTask mMusicalStateTask;

    PerformanceApp mPerformanceApp;
    DebugDisplayApp mDebugDisplayApp;
    SystemSettingsApp mSystemSettingsApp;

    BassoonoidApp() :
        mDisplay(128, 64, &SPI, 9/*DC*/, 8/*RST*/, 10/*CS*/, 44 * 1000000UL),
        mMusicalStateTask(&mAppSettings, &mControlMapper),
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

        mDisplay.Init(&mAppSettings, &mControlMapper, allApps);
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
