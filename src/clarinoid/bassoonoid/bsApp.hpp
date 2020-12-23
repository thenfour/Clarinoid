
#pragma once

#define CLARINOID_PLATFORM_TEENSY
#define CLARINOID_MODULE_MAIN // as opposed to some submodules like LH / RH

#include "bsBaseSystemSettings.hpp"

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
#include <clarinoid/menu/MenuAppSynthSettings.hpp>
#include <clarinoid/application/MusicalState.hpp>

#include <clarinoid/synth/Synth.hpp>

#include <clarinoid/bassoonoid/bsLed.hpp>
#include <clarinoid/bassoonoid/bsControlMapper.hpp>
#include <clarinoid/bassoonoid/DebugDisplayApp.hpp>
#include <clarinoid/menu/MenuAppSynthSettings.hpp>
#include "MusicalStateTask.hpp"

// MIDI library is touchy about how you instantiate.
// Simplest is to do it the way it's designed for: in the main sketch, global scope.
//MIDI_CREATE_INSTANCE(HardwareSerial, Serial8, gMIDI);

namespace clarinoid
{

    struct BassoonoidApp
    {
        static constexpr size_t breathMappingIndex = 13;

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
        SynthSettingsApp mSynthSettingsApp;

        BassoonoidApp() : mDisplay(128, 64, &SPI, 9 /*DC*/, 8 /*RST*/, 10 /*CS*/, 44 * 1000000UL),
                          mMusicalStateTask(&mAppSettings, &mInputDelegator, &mControlMapper),
                          mPerformanceApp(mDisplay, &mMusicalStateTask, &mControlMapper),
                          mDebugDisplayApp(mDisplay, mControlMapper, mMusicalStateTask),
                          mSystemSettingsApp(
                              mDisplay, breathMappingIndex, [](void *cap) {
                                  BassoonoidApp *pThis = (BassoonoidApp *)cap;
                                  return pThis->mControlMapper.mBreath.CurrentValue01();
                              },
                              this),
                          mSynthSettingsApp(mDisplay)
        {
        }

        void Main()
        {
            mControlMapper.Init(&mDisplay);

            IDisplayApp *allApps[] =
                {
                    &mPerformanceApp,
                    &mDebugDisplayApp,
                    &mSystemSettingsApp,
                    &mSynthSettingsApp,
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

            mAppSettings.mControlMappings[breathMappingIndex] = ControlMapping::UnipolarMapping(PhysicalControl::Breath, ControlMapping::Function::Breath, 0.11f, 0.5f);
            mAppSettings.mControlMappings[breathMappingIndex].mNPolarMapping.Unipolar().mCurveP = 0.65f;
            mAppSettings.mControlMappings[breathMappingIndex].mNPolarMapping.Unipolar().mCurveS = -0.15f;

            mAppSettings.mControlMappings[14] = ControlMapping::TypicalEncoderMapping(PhysicalControl::LHEnc, ControlMapping::Function::MenuScrollA);

            mAppSettings.mControlMappings[15] = ControlMapping::ButtonIncrementMapping(PhysicalControl::LHThx1, ControlMapping::Function::Transpose, 12.0f);
            mAppSettings.mControlMappings[16] = ControlMapping::ButtonIncrementMapping(PhysicalControl::LHThx2, ControlMapping::Function::Transpose, -12.0f);

            mAppSettings.mControlMappings[17] = ControlMapping::ButtonIncrementMapping(PhysicalControl::RHTh3, ControlMapping::Function::SynthPreset, 1.0f);
            mAppSettings.mControlMappings[18] = ControlMapping::ButtonIncrementMapping(PhysicalControl::RHTh2, ControlMapping::Function::SynthPreset, -1.0f);

            mAppSettings.mControlMappings[19] = ControlMapping::MomentaryMapping(PhysicalControl::RHx2, ControlMapping::Function::ModifierFine);
            mAppSettings.mControlMappings[20] = ControlMapping::MomentaryMapping(PhysicalControl::RHx3, ControlMapping::Function::ModifierCourse);

            mAppSettings.mControlMappings[21] = ControlMapping::UnipolarMapping(PhysicalControl::JoyY, ControlMapping::Function::PitchBend, 0.9f, 0.1f, -1.0f, 1.0f);

            mDisplay.Init(&mAppSettings, &mInputDelegator, allApps);
            mMusicalStateTask.Init();

            FunctionTask mDisplayTask1{this, [](void *cap) {
                                           BassoonoidApp *pThis = (BassoonoidApp *)cap;
                                           pThis->mDisplay.UpdateAndRenderTask();
                                       }};

            FunctionTask mDisplayTask2{this, [](void *cap) {
                                           BassoonoidApp *pThis = (BassoonoidApp *)cap;
                                           pThis->mDisplay.DisplayTask();
                                       }};

            // the "Musical state" task tends to take about 1000 microseconds, and is the most
            // critical. So let's run it every 2000 microseconds.
            // that means every other task can just get interlaced between these.
            // Other tasks can run much slower; even at 60fps we have a whole period of 16667 micros.
            // that means running musical state 8 times and everything else gets placed between.
            NopTask nopTask;

            TaskPlanner tp{
                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(0), &mMusicalStateTask, "MusS0"},
                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(1000), &mDisplayTask1, "Display1"},
                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(2000), &mMusicalStateTask, "MusS1"},
                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(3000), &mDisplayTask2, "Display2"},
                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(4000), &mMusicalStateTask, "MusS2"},
                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(5000), &mLed1, "mLed1"},
                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(6000), &mMusicalStateTask, "MusS3"},
                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(7000), &mLed2, "mLed2"},
                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(8000), &mMusicalStateTask, "MusS4"},
                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(9000), &mBreathLED, "mBreathLED"},
                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(10000), &mMusicalStateTask, "MusS5"},
                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(12000), &mMusicalStateTask, "MusS6"},
                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(14000), &mMusicalStateTask, "MusS7"},
                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(16000), &nopTask, "Nop"},
            };

            mPerformanceApp.Init(&tp);

            tp.Main();
        }
    };

} // namespace clarinoid
