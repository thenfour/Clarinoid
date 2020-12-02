
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

    DebugDisplayApp mDebugDisplayApp;
    SystemSettingsApp mSystemSettingsApp;

    BassoonoidApp() :
        mDisplay(128, 64, &SPI, 9/*DC*/, 8/*RST*/, 10/*CS*/, 44 * 1000000UL),
        mMusicalStateTask(&mAppSettings, &mControlMapper),
        mDebugDisplayApp(mDisplay, mControlMapper, mMusicalStateTask),
        mSystemSettingsApp(mDisplay)
    {
    }

    void Main()
    {
        TaskManager tm;
        IDisplayApp* allApps[] =
        {
            &mDebugDisplayApp,
            &mSystemSettingsApp,
        };

        mDisplay.Init(&mAppSettings, &mControlMapper, allApps);
        mMusicalStateTask.Init();

        // So there's one high priority and long-running task, and the rest are chopped into small pieces.
        // The long task gets input and updates all critical musical systems. There's no sense in breaking it down into smaller pieces, it would only get less responsive.
        // The rest are low priority and don't really matter. They should therefore be as small as possible.
        // TODO: after every low-priority task, see if the high priority task is due.
        // also TODO: the low-priority tasks are pretty easy to estimate execution time. So amendment to above: see if the high priority task WOULD be due before this low-priority task completes.
        tm.AddTask(mMusicalStateTask, 500, ProfileObjectType::MusicalState, "MusicalState");
        tm.AddTask(mDisplay, FPSToMicros(60), ProfileObjectType::Display, "display");
        tm.AddTask(mLed1, FPSToMicros(6), ProfileObjectType::LED, "LED1");
        tm.AddTask(mLed2, FPSToMicros(8), ProfileObjectType::LED, "LED2");
        tm.AddTask(mBreathLED, FPSToMicros(24), ProfileObjectType::LED, "BreathLED");

        tm.Main();
    }
};

} // namespace clarinoid
