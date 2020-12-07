
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
        mDisplay(128, 64, &SPI, 9/*DC*/, 8/*RST*/, 10/*CS*/, 88 * 1000000UL),
        mMusicalStateTask(&mAppSettings, &mControlMapper),
        mPerformanceApp(mDisplay, &mMusicalStateTask),
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

        TaskPlanner tp {
        };

        mPerformanceApp.Init(&tp);

        //CCASSERT(false);

        // tm.AddTask(mMusicalStateTask, 5000, "MusicalState", PriorityClass::Normal);
        // tm.AddTask(mDisplay, FPSToMicros(12), "display", PriorityClass::Normal);
        // tm.AddTask(mLed1, FPSToMicros(12), "LED1", PriorityClass::Normal);
        // tm.AddTask(mLed2, FPSToMicros(12), "LED2", PriorityClass::Normal);
        // tm.AddTask(mBreathLED, FPSToMicros(12), "BreathLED", PriorityClass::Normal);

        // tm.Main();
    }
};

} // namespace clarinoid
