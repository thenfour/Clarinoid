
#pragma once

#define CLARINOID_PLATFORM_TEENSY
#define CLARINOID_MODULE_MAIN // as opposed to some submodules like LH / RH

#include "bmBaseSystemSettings.hpp"

#include <clarinoid/basic/Basic.hpp>

#include <clarinoid/components/Switch.hpp>
#include <clarinoid/components/Leds.hpp>

#include <clarinoid/components/Encoder.hpp>
#include <clarinoid/components/Potentiometer.hpp>
//#include <clarinoid/components/HoneywellABPI2C.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include <clarinoid/application/Display.hpp>
#include <clarinoid/application/ControlMapper.hpp>
#include <clarinoid/menu/MenuAppBase.hpp>
#include <clarinoid/menu/MenuAppSystemSettings.hpp>
#include <clarinoid/menu/MenuAppSynthSettings.hpp>
#include <clarinoid/menu/MenuAppHarmonizerSettings.hpp>
#include <clarinoid/application/USBKeyboardMusicalDevice.hpp>
#include <clarinoid/application/MusicalState.hpp>
#include <clarinoid/menu/MenuAppPerformanceSettings.hpp>
#include <clarinoid/Gui/GuiPerformanceApp.hpp>
#include <clarinoid/application/DefaultHud.hpp>

#include <clarinoid/synth/polyBlepOscillator.hpp> // https://gitlab.com/flojawi/teensy-polyblep-oscillator/-/tree/master/polySynth
#include <clarinoid/synth/PolySynth.hpp>

#include "bmLed.hpp"
#include "bmControlMapper.hpp"
#include "bmDebugDisplayApp.hpp"
#include <clarinoid/menu/MenuAppSynthSettings.hpp>
#include <clarinoid/menu/MenuAppMetronome.hpp>
#include "bmMusicalStateTask.hpp"

namespace clarinoid
{

CCAdafruitSSD1306 gDisplay = {128, 64, &SPI, 40 /*DC*/, 41 /*RST*/, 10 /*CS*/, 88 * 1000000UL};

struct BommanoidApp : ILEDDataProvider, ISysInfoProvider
{
    // static constexpr size_t breathMappingIndex = 13;
    // static constexpr size_t joyPitchMappingIndex = 21;

    // Leds1 mLed1;
    // Leds2 mLed2;
    // BreathLED mBreathLED;
    InputDelegator mInputDelegator;
    BommanoidControlMapper mControlMapper;
    _CCDisplay mDisplay;
    AppSettings mAppSettings;

    MusicalStateTask mMusicalStateTask;
    TheShinies mShinies;

    PerformanceApp mPerformanceApp;
    DebugDisplayApp mDebugDisplayApp;
    SystemSettingsApp mSystemSettingsApp;
    AudioMonitorApp mAudioMonitorApp;
    MetronomeSettingsApp mMetronomeSettingsApp;
    PerformancePatchSettingsApp mPerfPatchApp;
    GuiPerformanceApp mGuiPerformanceApp; //(mDisplay, mMusicalStateTask.mMetronome);

    TaskPlanner *mTaskPlanner = nullptr; // set after initializing it, late in the startup process.
    SimpleMovingAverage<30> mCPUUsage;
    PeakMeterUtility<2000, 300> mPeakMeter;

    static constexpr auto xaoeu = sizeof(mAppSettings);

    BommanoidApp()
        : mDisplay(gDisplay), mMusicalStateTask(&mDisplay, &mAppSettings, &mInputDelegator, &mControlMapper),
          mShinies(&mAppSettings),                                                                       //
          mPerformanceApp(mDisplay, &mMusicalStateTask, &mControlMapper, mAppSettings, mInputDelegator), //
          mDebugDisplayApp(mDisplay, mControlMapper, mMusicalStateTask, mAppSettings, mInputDelegator),
          mSystemSettingsApp(
              mDisplay,
              -1,
              -1,
              -1,
              [](void *cap) FLASHMEM { // raw breath value getter
                  // BommanoidApp *pThis = (BommanoidApp *)cap;
                  return 0.0f; // pThis->mControlMapper.mBreath.CurrentValue01();
              },
              [](void *cap) FLASHMEM { // raw joy pitchbend value getter
                  // BommanoidApp *pThis = (BommanoidApp *)cap;
                  return 0.0f; // pThis->mControlMapper.mJoyY.CurrentValue01();
              },
              this,
              &mMusicalStateTask.mMetronome,
              mAppSettings,
              mInputDelegator),
          mAudioMonitorApp(mDisplay, mAppSettings, mInputDelegator),                                      //
          mMetronomeSettingsApp(&mMusicalStateTask.mMetronome, &mAppSettings, mDisplay, mInputDelegator), //
          mPerfPatchApp(mDisplay, mAppSettings, mInputDelegator),                                         //
          mGuiPerformanceApp(mDisplay, mMusicalStateTask.mMetronome, mAppSettings, mInputDelegator)       //
    {
    }

    virtual Metronome *ILEDDataProvider_GetMetronomeBeat() override
    {
        return &mMusicalStateTask.mMetronome;
    }
    virtual InputDelegator *ILEDDataProvider_GetInput() override
    {
        return &mInputDelegator;
    }
    virtual MusicalState *ILEDDataProvider_GetMusicalState() override
    {
        return &mMusicalStateTask.mMusicalState;
    }

    virtual uint8_t ISysInfoProvider_GetPolyphony() override
    {
        return mMusicalStateTask.mSynth.GetCurrentPolyphony_ForDisplay();
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
        return MidiNote(0);
        // return MidiNote((uint8_t)mMusicalStateTask.mMusicalState.mLastPlayedNote);
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
        // return mMusicalStateTask.mMusicalState.mCurrentPitchN11.GetValue();
    }
    virtual float ISysInfoProvider_GetBreath01() override
    {
        return 0;
        // return mMusicalStateTask.mMusicalState.mCurrentBreath01.GetValue();
    }
    virtual AppSettings *ISysInfoProvider_GetSettings() override
    {
        return &mAppSettings;
    }

    void Main()
    {
        mControlMapper.Init(&mDisplay);

        HarmSettingsApp mHarmVoiceSettingsApp(mDisplay, mAppSettings, mInputDelegator);
        SynthPatchMenuApp mSynthPatchApp(mDisplay, mAppSettings, mInputDelegator);

        IDisplayApp *allApps[] = {
            &mDebugDisplayApp,
            &mGuiPerformanceApp,

            &mPerformanceApp,
            &mSynthPatchApp,
            &mHarmVoiceSettingsApp,
            &mPerfPatchApp,

            &mAudioMonitorApp,
            &mSystemSettingsApp,
            &mMetronomeSettingsApp,
        };

        mInputDelegator.Init(&mAppSettings, &mControlMapper);

        mAppSettings.mControlMappings[0] =
            ControlMapping::MomentaryMapping(PhysicalControl::R2, ControlMapping::Function::MenuOK);
        mAppSettings.mControlMappings[1] =
            ControlMapping::MomentaryMapping(PhysicalControl::R1, ControlMapping::Function::MenuBack);

        mAppSettings.mControlMappings[2] =
            ControlMapping::MakeUnipolarMapping(PhysicalControl::Pot1, ControlMapping::Function::MacroPot1, 0.0f, 1.0f);
        mAppSettings.mControlMappings[3] =
            ControlMapping::MakeUnipolarMapping(PhysicalControl::Pot2, ControlMapping::Function::MacroPot2, 0.0f, 1.0f);
        mAppSettings.mControlMappings[4] =
            ControlMapping::MakeUnipolarMapping(PhysicalControl::Pot3, ControlMapping::Function::MacroPot3, 0.0f, 1.0f);
        mAppSettings.mControlMappings[5] =
            ControlMapping::MakeUnipolarMapping(PhysicalControl::Pot4, ControlMapping::Function::MacroPot4, 0.0f, 1.0f);

        mAppSettings.mControlMappings[6] =
            ControlMapping::MomentaryMapping(PhysicalControl::Pedal, ControlMapping::Function::SustainPedal);

        mAppSettings.mControlMappings[14] =
            ControlMapping::TypicalEncoderMapping(PhysicalControl::Enc, ControlMapping::Function::MenuScrollA);

        mAppSettings.mControlMappings[19] =
            ControlMapping::MomentaryMapping(PhysicalControl::L1, ControlMapping::Function::ModifierFine);
        mAppSettings.mControlMappings[20] =
            ControlMapping::MomentaryMapping(PhysicalControl::L2, ControlMapping::Function::ModifierCourse);

        mAppSettings.GetCurrentPerformancePatch().mSynthPresetA = SynthPresetID_Bommanoid;
        // mAppSettings.GetCurrentPerformancePatch().mMasterFXEnable = false;

        DefaultHud hud = {mDisplay, this};

        mDisplay.Init(&mAppSettings, &mInputDelegator, &hud, allApps);
        mMusicalStateTask.Init();

        FunctionTask mDisplayTask1{this, [](void *cap) FLASHMEM {
                                       NoInterrupts ni;
                                       BommanoidApp *pThis = (BommanoidApp *)cap;
                                       pThis->mDisplay.UpdateAndRenderTask();
                                   }};

        FunctionTask mDisplayTask2{this, [](void *cap) FLASHMEM {
                                       NoInterrupts ni;
                                       BommanoidApp *pThis = (BommanoidApp *)cap;
                                       pThis->mDisplay.DisplayTask();
                                   }};

        // the "Musical state" is the most critical. So let's run it periodically, spread through the whole time slice.
        // display tasks are also very heavy. Display1 is update/state, Display2 is SPI render.
        // musical state = about <2400 microseconds
        // display1 = about <1500 microseconds
        // display2 = about <2000 microseconds
        // LED tasks tend to be almost instantaneous (~<10 microseconds) so they can all live in the same slot.
        NopTask nopTask;

        // NB: run an update task before display tasks in order to initialize things on 1st frame.
        TaskPlanner::TaskDeadline plan[] = {
            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 0), &mMusicalStateTask, "MusS0"},
            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 0), &mShinies, "SH0"},

            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 1), &mMusicalStateTask, "MusS1"},
            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 1), &mDisplayTask1, "Display1"},

            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 2), &mMusicalStateTask, "MusS2"},

            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 3), &mMusicalStateTask, "MusS3"},

            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 4), &mDisplayTask2, "Display2"},

            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 5), &mMusicalStateTask, "MusS4"},
            //{TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 5), &mLed1, "mLed1"},

            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 6), &mMusicalStateTask, "MusS5"},
            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 6), &mShinies, "SH1"},

            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 7), &mMusicalStateTask, "MusS6"},
            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 8), &mMusicalStateTask, "MusS7"},
            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 9), &mMusicalStateTask, "MusS8"},
            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 10), &mMusicalStateTask, "MusS9"},
            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 11), &mMusicalStateTask, "MusSa"},

            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 12), &nopTask, "Nop"},
        };

        TaskPlanner tp{plan};
        mTaskPlanner = &tp;

        mPerformanceApp.Init(&tp);
        AudioInterrupts();
        tp.Main();
    }
};

// static constexpr auto a3oeu = sizeof(HarmSettingsApp);
static constexpr auto ao4eu = sizeof(SynthPatchMenuApp);
static constexpr auto aoe5u = sizeof(TaskPlanner::TaskDeadline);

} // namespace clarinoid
