
#pragma once

#define CLARINOID_PLATFORM_TEENSY
#define CLARINOID_MODULE_MAIN // as opposed to some submodules like LH / RH

#include "testDeviceBaseSystemSettings.hpp"

#include <clarinoid/basic/Basic.hpp>

#include <clarinoid/components/Switch.hpp>
#include <clarinoid/components/Leds.hpp>

#include <clarinoid/components/Encoder.hpp>
#include <clarinoid/components/Potentiometer.hpp>
#include <clarinoid/components/HoneywellABPI2C.hpp>
#include <clarinoid/components/MCP23017.hpp>
#include <clarinoid/components/CCMPR121.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include <clarinoid/application/Display.hpp>
#include <clarinoid/application/DefaultHud.hpp>
#include <clarinoid/application/ControlMapper.hpp>
#include <clarinoid/menu/MenuAppBase.hpp>
#include <clarinoid/menu/MenuAppSystemSettings.hpp>
#include <clarinoid/menu/MenuAppSynthSettings.hpp>
#include <clarinoid/midi/midi.hpp>
#include <clarinoid/application/MusicalState.hpp>
#include <clarinoid/Gui/GuiApp.hpp>

#include <clarinoid/Gui/GuiPerformanceApp.hpp>

#include <clarinoid/synth/Synth.hpp>

#include "testDeviceControlMapper.hpp"
#include "testDeviceMusicalStateTask.hpp"
#include "testDeviceDebugDisplayApp.hpp"
#include <clarinoid/menu/MenuAppSynthSettings.hpp>
#include <clarinoid/menu/MenuAppMetronome.hpp>
#include <clarinoid/menu/MenuAppHarmonizerSettings.hpp>
#include <clarinoid/menu/MenuAppPerformanceSettings.hpp>

namespace clarinoid
{

struct TestDeviceApp : ISysInfoProvider
{
    InputDelegator mInputDelegator;
    Clarinoid2ControlMapper mControlMapper;
    _CCDisplay mDisplay;
    DefaultHud mHud;
    AppSettings mAppSettings;

    MusicalStateTask mMusicalStateTask;

    PerformanceApp mPerformanceApp;
    PerformancePatchSettingsApp mPerfPatchApp;
    SynthPatchMenuApp mSynthPatchApp;
    MetronomeSettingsApp mMetronomeSettingsApp;
    HarmSettingsApp mHarmVoiceSettingsApp;
    HarmPatchSettingsApp mHarmPatchApp;

    TaskPlanner *mTaskPlanner = nullptr; // set after initializing it, late in the startup process.

    TestDeviceApp()
        : mDisplay(128, 64, &SPI, 9 /*DC*/, 8 /*RST*/, 10 /*CS*/, 10 * 1000000UL), mHud(mDisplay, this),
          mMusicalStateTask(&mDisplay, &mAppSettings, &mInputDelegator, &mControlMapper),
          mPerformanceApp(mDisplay, &mMusicalStateTask, &mControlMapper, &mMusicalStateTask.mMetronome),
          mPerfPatchApp(mDisplay), mSynthPatchApp(mDisplay),
          mMetronomeSettingsApp(&mMusicalStateTask.mMetronome, &mAppSettings, mDisplay),
          mHarmVoiceSettingsApp(mDisplay), mHarmPatchApp(mDisplay)
    {
    }

    SimpleMovingAverage<30> mCPUUsage;
    PeakMeterUtility<2000, 300> mPeakMeter;

    virtual uint8_t ISysInfoProvider_GetPolyphony() override
    {
        return 0;
    }
    virtual float ISysInfoProvider_GetAudioCPUUsage() override
    {
        return AudioProcessorUsage();
    }
    virtual float ISysInfoProvider_GetTaskManagerCPUUsage() override
    {
        if (!mTaskPlanner)
            return 0.0f;
        float p = (float)mTaskPlanner->mPreviousTimeSliceDelayTime.ElapsedMicros();
        p /= mTaskPlanner->mTimesliceDuration.ElapsedMicros();
        p = 1.0f - p;
        return p * 100.0f;
    }
    virtual float ISysInfoProvider_GetPeak() override
    {
        float peak, heldPeak;
        mPeakMeter.Update(peak, heldPeak);
        return heldPeak;
    }
    virtual MidiNote ISysInfoProvider_GetNote() override
    {
        return MidiNote(4, millis() / 600 % 12);
    }
    virtual float ISysInfoProvider_GetTempo() override
    {
        return mMusicalStateTask.mMetronome.mBPM;
    }
    virtual Metronome *ISysInfoProvider_GetMetronome() override
    {
        return &mMusicalStateTask.mMetronome;
    }
    virtual float ISysInfoProvider_GetPitchBendN11() override
    {
        return 0;
    }
    virtual float ISysInfoProvider_GetBreath01() override
    {
        return 0;
    }
    virtual AppSettings *ISysInfoProvider_GetSettings() override
    {
        return &mAppSettings;
    }

    void Main()
    {
        mControlMapper.Init(&mDisplay);

        GuiTestApp mGuiTestApp(mDisplay);
        GuiPerformanceApp mGuiPerformanceApp(mDisplay, mMusicalStateTask.mMetronome);

        IDisplayApp *allApps[] = {
            &mPerformanceApp, // nice to have this as front page to know if things are running healthy.
            &mGuiTestApp,
            &mGuiPerformanceApp,

            &mPerfPatchApp,
            &mHarmPatchApp,
            &mSynthPatchApp,

            &mHarmVoiceSettingsApp,

            &mMetronomeSettingsApp,
        };

        mInputDelegator.Init(&mAppSettings, &mControlMapper);

        size_t im = 0;

        mAppSettings.mControlMappings[im++] =
            ControlMapping::MomentaryMapping(PhysicalControl::Ok, ControlMapping::Function::MenuOK);
        mAppSettings.mControlMappings[im++] =
            ControlMapping::MomentaryMapping(PhysicalControl::Back, ControlMapping::Function::MenuBack);

        mAppSettings.mControlMappings[im++] =
            ControlMapping::MomentaryMapping(PhysicalControl::Button3, ControlMapping::Function::DisplayFontToggle);

        mAppSettings.mControlMappings[im++] =
            ControlMapping::MomentaryMapping(PhysicalControl::EncButton, ControlMapping::Function::DisplayFontToggle);

        mAppSettings.mControlMappings[im++] =
            ControlMapping::TypicalEncoderMapping(PhysicalControl::Enc, ControlMapping::Function::MenuScrollA);

        mDisplay.Init(&mAppSettings, &mInputDelegator, &mHud, allApps);
        mMusicalStateTask.Init();

        Wire1.setClock(400000); // use high speed mode. default speed = 100k

        FunctionTask mDisplayTask1{this, [](void *cap) {
                                       TestDeviceApp *pThis = (TestDeviceApp *)cap;
                                       pThis->mDisplay.UpdateAndRenderTask();
                                   }};

        FunctionTask mDisplayTask2{this, [](void *cap) {
                                       TestDeviceApp *pThis = (TestDeviceApp *)cap;
                                       pThis->mDisplay.DisplayTask();
                                   }};

        NopTask nopTask;

        // Important that a state task runs before display tasks, to ensure there's not a single
        // uninitialized glitch frame at startup
        TaskPlanner::TaskDeadline plan[] = {
            {TimeSpan::FromMicros(0), &mMusicalStateTask, "MusS0"},
            {TimeSpan::FromMicros(2000), &mDisplayTask1, "Display1"},
            {TimeSpan::FromMicros(4000), &mMusicalStateTask, "MusS1"},
            {TimeSpan::FromMicros(6000), &mDisplayTask2, "Display2"},
            {TimeSpan::FromMicros(8000), &nopTask, "Nop"},
        };
        TaskPlanner tp{plan};

        mTaskPlanner = &tp;
        mPerformanceApp.Init(&tp);

        tp.Main();
    }
};

} // namespace clarinoid
