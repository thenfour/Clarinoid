
#pragma once

#define CLARINOID_PLATFORM_TEENSY
#define CLARINOID_MODULE_MAIN // as opposed to some submodules like LH / RH

#include "clarinoid2BaseSystemSettings.hpp"

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

#include <clarinoid/synth/Synth.hpp>

#include "clarinoid2Led.hpp"
#include "clarinoid2ControlMapper.hpp"
#include "clarinoid2MusicalStateTask.hpp"
#include "clarinoid2DebugDisplayApp.hpp"
#include <clarinoid/menu/MenuAppSynthSettings.hpp>
#include <clarinoid/menu/MenuAppMetronome.hpp>
#include <clarinoid/menu/MenuAppHarmonizerSettings.hpp>
#include <clarinoid/menu/MenuAppPerformanceSettings.hpp>

#include <clarinoid/Gui/GuiPerformanceApp.hpp>

namespace clarinoid
{

// global for debugging / crash handling purposes
CCAdafruitSSD1306 gDisplay = {128, 64, &SPI, 9 /*DC*/, 8 /*RST*/, 10 /*CS*/, 10 * 1000000UL};

struct Clarinoid2App : ILEDDataProvider, ISysInfoProvider
{
    static constexpr size_t breathMappingIndex = 0;
    static constexpr size_t pitchUpMappingIndex = 1;
    static constexpr size_t pitchDownMappingIndex = 2;

    Clarinoid2LedsTask mLed;
    InputDelegator mInputDelegator;
    Clarinoid2ControlMapper mControlMapper;
    _CCDisplay mDisplay;
    DefaultHud mHud;
    AppSettings mAppSettings;

    MusicalStateTask mMusicalStateTask;

    PerformanceApp mPerformanceApp;
    DebugDisplayApp mDebugDisplayApp;
    SystemSettingsApp mSystemSettingsApp;
    PerformancePatchSettingsApp mPerfPatchApp;
    SynthPatchMenuApp mSynthPatchApp;
    AudioMonitorApp mAudioMonitorApp;
    MetronomeSettingsApp mMetronomeSettingsApp;
    HarmSettingsApp mHarmVoiceSettingsApp;
    HarmPatchSettingsApp mHarmPatchApp;

    GuiPerformanceApp mGuiPerformanceApp;    //(mDisplay, mMusicalStateTask.mMetronome);
    MPR121ConfigApp<10, 4> mMPR121ConfigApp; //(mDisplay, mControlMapper, mMusicalStateTask);

    TaskPlanner *mTaskPlanner = nullptr; // set after initializing it, late in the startup process.

    SimpleMovingAverage<30> mCPUUsage;
    PeakMeterUtility<2000, 300> mPeakMeter;

    Clarinoid2App()
        : mLed(this),                                                                                    //
          mDisplay(gDisplay),                                                                            //
          mHud(mDisplay, this),                                                                          //
          mMusicalStateTask(&mDisplay, &mAppSettings, &mInputDelegator, &mControlMapper),                //
          mPerformanceApp(mDisplay, &mMusicalStateTask, &mControlMapper, &mMusicalStateTask.mMetronome), //
          mDebugDisplayApp(mDisplay, mControlMapper, mMusicalStateTask),                                 //
          mSystemSettingsApp(
              mDisplay,
              breathMappingIndex,
              pitchUpMappingIndex,
              pitchDownMappingIndex,
              [](void *cap) FLASHMEM { // raw breath value getter
                  Clarinoid2App *pThis = (Clarinoid2App *)cap;
                  return pThis->mControlMapper.mBreath.CurrentValue01();
              },
              [](void *cap) FLASHMEM { // raw pitchbend value getter
                  Clarinoid2App *pThis = (Clarinoid2App *)cap;
                  return pThis->mControlMapper.mPitchStrip.CurrentValue01();
              },
              this,
              &mMusicalStateTask.mMetronome),
          mPerfPatchApp(mDisplay),    //
          mSynthPatchApp(mDisplay),   //
          mAudioMonitorApp(mDisplay), //
          mMetronomeSettingsApp(&mMusicalStateTask.mMetronome, &mAppSettings, mDisplay),
          mHarmVoiceSettingsApp(mDisplay),                            //
          mHarmPatchApp(mDisplay),                                    //
          mGuiPerformanceApp(mDisplay, mMusicalStateTask.mMetronome), //
          mMPR121ConfigApp(mDisplay, mControlMapper, mMusicalStateTask)
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
    virtual CCEWIMusicalState *ILEDDataProvider_GetMusicalState() override
    {
        return &mMusicalStateTask.mMusicalState;
    }

    virtual uint8_t ISysInfoProvider_GetPolyphony() override
    {
        return mMusicalStateTask.mSynth.mCurrentPolyphony;
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
        return MidiNote((uint8_t)mMusicalStateTask.mMusicalState.mLastPlayedNote);
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
        return mMusicalStateTask.mMusicalState.mCurrentPitchN11.GetValue();
    }
    virtual float ISysInfoProvider_GetBreath01() override
    {
        return mMusicalStateTask.mMusicalState.mCurrentBreath01.GetValue();
    }
    virtual AppSettings *ISysInfoProvider_GetSettings() override
    {
        return &mAppSettings;
    }

    void Main()
    {
        Wire1.setClock(400000); // use high speed mode. default speed = 100k
        mControlMapper.Init(&mDisplay);

        IDisplayApp *allApps[] = {
            &mGuiPerformanceApp,

            &mPerformanceApp, // nice to have this as front page to know if things are running healthy.
            &mPerfPatchApp,
            &mHarmPatchApp,
            &mSynthPatchApp,

            &mHarmVoiceSettingsApp,

            &mMetronomeSettingsApp,
            &mSystemSettingsApp, // <-- perf patch selector
            &mAudioMonitorApp,

            &mDebugDisplayApp,
            &mMPR121ConfigApp,
        };

        mInputDelegator.Init(&mAppSettings, &mControlMapper);

        mAppSettings.mControlMappings[breathMappingIndex] =
            ControlMapping::MakeUnipolarMapping(PhysicalControl::Breath, ControlMapping::Function::Breath, 0.10f, 0.5f);
        mAppSettings.mControlMappings[breathMappingIndex].mUnipolarMapping.mCurveP = 0.38f;
        mAppSettings.mControlMappings[breathMappingIndex].mUnipolarMapping.mCurveS = 0;

        mAppSettings.mControlMappings[pitchUpMappingIndex] = ControlMapping::MakeUnipolarMapping(
            PhysicalControl::Pitch, ControlMapping::Function::PitchBend, 0.0f, 1.0f);
        mAppSettings.mControlMappings[pitchUpMappingIndex].mOperator = ControlMapping::Operator::Set;
        mAppSettings.mControlMappings[pitchUpMappingIndex].mUnipolarMapping.mSrcMin = 0.43f;
        mAppSettings.mControlMappings[pitchUpMappingIndex].mUnipolarMapping.mSrcMax = 0.06f;
        mAppSettings.mControlMappings[pitchUpMappingIndex].mUnipolarMapping.mCurveP = 0.50f;
        mAppSettings.mControlMappings[pitchUpMappingIndex].mUnipolarMapping.mCurveS = 0;

        mAppSettings.mControlMappings[pitchDownMappingIndex] = ControlMapping::MakeUnipolarMapping(
            PhysicalControl::Pitch, ControlMapping::Function::PitchBend, 0.0f, 1.0f);
        mAppSettings.mControlMappings[pitchDownMappingIndex].mOperator = ControlMapping::Operator::Subtract;
        mAppSettings.mControlMappings[pitchDownMappingIndex].mUnipolarMapping.mSrcMin = 0.66f;
        mAppSettings.mControlMappings[pitchDownMappingIndex].mUnipolarMapping.mSrcMax = 0.92f;
        mAppSettings.mControlMappings[pitchDownMappingIndex].mUnipolarMapping.mCurveP = 0.50f;
        mAppSettings.mControlMappings[pitchDownMappingIndex].mUnipolarMapping.mCurveS = 0;

        size_t im = pitchDownMappingIndex + 1;

        mAppSettings.mControlMappings[++im] =
            ControlMapping::MomentaryMapping(PhysicalControl::Ok, ControlMapping::Function::MenuOK);
        mAppSettings.mControlMappings[++im] =
            ControlMapping::MomentaryMapping(PhysicalControl::Back, ControlMapping::Function::MenuBack);
        mAppSettings.mControlMappings[++im] =
            ControlMapping::MomentaryMapping(PhysicalControl::EncButton, ControlMapping::Function::DisplayFontToggle);

        mAppSettings.mControlMappings[++im] =
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::Oct1, ControlMapping::Function::Oct1);
        mAppSettings.mControlMappings[++im] =
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::Oct2, ControlMapping::Function::Oct2);
        mAppSettings.mControlMappings[++im] =
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::Oct3, ControlMapping::Function::Oct3);
        mAppSettings.mControlMappings[++im] =
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::Oct4, ControlMapping::Function::Oct4);
        mAppSettings.mControlMappings[++im] =
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::Oct5, ControlMapping::Function::Oct5);
        mAppSettings.mControlMappings[++im] =
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::Oct6, ControlMapping::Function::Oct6);

        mAppSettings.mControlMappings[++im] =
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::LHKey1, ControlMapping::Function::LH1);
        mAppSettings.mControlMappings[++im] =
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::LHKey2, ControlMapping::Function::LH2);
        mAppSettings.mControlMappings[++im] =
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::LHKey3, ControlMapping::Function::LH3);
        mAppSettings.mControlMappings[++im] =
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::LHKey4, ControlMapping::Function::LH4);

        mAppSettings.mControlMappings[++im] =
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::RHKey1, ControlMapping::Function::RH1);
        mAppSettings.mControlMappings[++im] =
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::RHKey2, ControlMapping::Function::RH2);
        mAppSettings.mControlMappings[++im] =
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::RHKey3, ControlMapping::Function::RH3);
        mAppSettings.mControlMappings[++im] =
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::RHKey4, ControlMapping::Function::RH4);

        mAppSettings.mControlMappings[++im] =
            ControlMapping::TypicalEncoderMapping(PhysicalControl::Enc, ControlMapping::Function::MenuScrollA);

        // mAppSettings.mControlMappings[++im] =
        //     ControlMapping::ButtonIncrementMapping(PhysicalControl::LHx2,
        //     ControlMapping::Function::SynthPreset, 1.0f);
        // mAppSettings.mControlMappings[++im] =
        //     ControlMapping::ButtonIncrementMapping(PhysicalControl::LHx3, ControlMapping::Function::SynthPreset,
        //     -1.0f);

        //  NORMAL     SHIFT
        // - synth+    synthB+
        // - synth-    synthB-
        //
        // - harm+     perf+
        // - harm-     perf-
        mAppSettings.mControlMappings[++im] =
            ControlMapping::ButtonIncrementMapping(PhysicalControl::RHx1, ControlMapping::Function::SynthPresetA, 1.0f);
        mAppSettings.mControlMappings[++im] = ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx2, ControlMapping::Function::SynthPresetA, -1.0f);

        mAppSettings.mControlMappings[++im] = ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx1, ControlMapping::Function::SynthPresetB, 1.0f, ModifierKey::Shift);
        mAppSettings.mControlMappings[++im] = ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx2, ControlMapping::Function::SynthPresetB, -1.0f, ModifierKey::Shift);

        mAppSettings.mControlMappings[++im] =
            ControlMapping::ButtonIncrementMapping(PhysicalControl::RHx3, ControlMapping::Function::HarmPreset, 1.0f);
        mAppSettings.mControlMappings[++im] =
            ControlMapping::ButtonIncrementMapping(PhysicalControl::RHx4, ControlMapping::Function::HarmPreset, -1.0f);

        mAppSettings.mControlMappings[++im] = ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx3, ControlMapping::Function::PerfPreset, 1.0f, ModifierKey::Shift);
        mAppSettings.mControlMappings[++im] = ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx4, ControlMapping::Function::PerfPreset, -1.0f, ModifierKey::Shift);

        mAppSettings.mControlMappings[++im] =
            ControlMapping::MomentaryMapping(PhysicalControl::LHx3, ControlMapping::Function::ModifierCourse);

        mAppSettings.mControlMappings[++im] =
            ControlMapping::MomentaryMapping(PhysicalControl::LHx2, ControlMapping::Function::ModifierFine);
        mAppSettings.mControlMappings[++im] =
            ControlMapping::MomentaryMapping(PhysicalControl::LHx2, ControlMapping::Function::ModifierShift);

        mAppSettings.mControlMappings[++im] =
            ControlMapping::MomentaryMapping(PhysicalControl::LHx1, ControlMapping::Function::HarmPresetOnOffToggle);

        mDisplay.Init(&mAppSettings, &mInputDelegator, &mHud, allApps);
        mMusicalStateTask.Init();

        FunctionTask mDisplayTask1{this, [](void *cap) FLASHMEM {
                                       Clarinoid2App *pThis = (Clarinoid2App *)cap;
                                       NoInterrupts ni;
                                       pThis->mDisplay.UpdateAndRenderTask();
                                   }};

        FunctionTask mDisplayTask2{this, [](void *cap) FLASHMEM {
                                       Clarinoid2App *pThis = (Clarinoid2App *)cap;
                                       NoInterrupts ni;
                                       pThis->mDisplay.DisplayTask();
                                   }};

        NopTask nopTask;
        TaskPlanner::TaskDeadline plan[] = {
            // NB: run an update task before display tasks in order to initialize things on 1st frame.
            // let's give musicalstatetask a period of <2900 micros, because that's the length of each audio buffer
            // size.
            // and a total plan length that gives the display about 60fps (1667 micros)

            //
            {TimeSpan::FromMicros(0), &mMusicalStateTask, "Mus1"},
            {TimeSpan::FromMicros(0), &mLed, "Led1"},

            //
            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 1), &mMusicalStateTask, "Mus2"},
            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 1), &mDisplayTask1, "Display1"},

            //
            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 2), &mMusicalStateTask, "Mus3"},

            //
            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 3), &mMusicalStateTask, "Mus4"},
            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 3), &mLed, "Led2"},

            //
            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 4), &mMusicalStateTask, "Mus5"},
            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 4), &mDisplayTask2, "Display2"},

            //
            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 5), &mMusicalStateTask, "Mus5"},

            //
            {TimeSpan::FromMicros(MUSICALSTATE_TIMESLICE_PERIOD_MICROS * 6), &nopTask, "Nop"},
        };

        TaskPlanner tp = {plan};

        mTaskPlanner = &tp;
        mPerformanceApp.Init(&tp);

        AudioInterrupts();
        tp.Main();
    }
};

} // namespace clarinoid
