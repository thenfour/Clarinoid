
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

namespace clarinoid
{

    struct Clarinoid2App : ILEDDataProvider
    {
        static constexpr size_t breathMappingIndex = 0;
        static constexpr size_t pitchUpMappingIndex = 1;
        static constexpr size_t pitchDownMappingIndex = 2;

        Clarinoid2LedsTask mLed;
        InputDelegator mInputDelegator;
        Clarinoid2ControlMapper mControlMapper;
        CCDisplay mDisplay;
        AppSettings mAppSettings;

        MusicalStateTask mMusicalStateTask;

        PerformanceApp mPerformanceApp;
        DebugDisplayApp mDebugDisplayApp;
        SystemSettingsApp mSystemSettingsApp;
        SynthSettingsApp mSynthSettingsApp;
        SynthPatchMenuApp mSynthPatchApp;
        AudioMonitorApp mAudioMonitorApp;
        MetronomeSettingsApp mMetronomeSettingsApp;
        HarmSettingsApp mHarmVoiceSettingsApp;

        Clarinoid2App() : mLed(this),
                          mDisplay(128, 64, &SPI, 9 /*DC*/, 8 /*RST*/, 10 /*CS*/, 10 * 1000000UL),
                          mMusicalStateTask(&mAppSettings, &mInputDelegator, &mControlMapper),
                          mPerformanceApp(mDisplay, &mMusicalStateTask, &mControlMapper),
                          mDebugDisplayApp(mDisplay, mControlMapper, mMusicalStateTask),
                          mSystemSettingsApp(
                              mDisplay, breathMappingIndex, pitchUpMappingIndex, pitchDownMappingIndex,
                              [](void *cap) { // raw breath value getter
                                  Clarinoid2App *pThis = (Clarinoid2App *)cap;
                                  return pThis->mControlMapper.mBreath.CurrentValue01();
                              },
                              [](void *cap) { // raw pitchbend value getter
                                  Clarinoid2App *pThis = (Clarinoid2App *)cap;
                                return pThis->mControlMapper.mPitchStrip.CurrentValue01();
                              },
                              this),
                          mSynthSettingsApp(mDisplay),
                          mSynthPatchApp(mDisplay),
                          mAudioMonitorApp(mDisplay),
                          mMetronomeSettingsApp(&mMusicalStateTask.mMetronome, &mAppSettings, mDisplay),
                          mHarmVoiceSettingsApp(mDisplay)
        {
        }

        virtual Metronome* ILEDDataProvider_GetMetronomeBeat() override
        {
            return &mMusicalStateTask.mMetronome;
        }
        virtual InputDelegator* ILEDDataProvider_GetInput() override
        {
            return &mInputDelegator;
        }
        virtual CCEWIMusicalState* ILEDDataProvider_GetMusicalState() override
        {
            return &mMusicalStateTask.mMusicalState;
        }

        void Main()
        {
            mControlMapper.Init(&mDisplay);

            // initialize some settings.
            mAppSettings.mTranspose = 12;
            mAppSettings.mSynthSettings.mReverbGain = 0.9f;
            mAppSettings.mSynthSettings.mPitchBendRange = 2.0f;

            TouchKeyMonitorApp mLHKeysMonitor(mDisplay, mControlMapper.mLHMPR, "LH Keys Monitor", 0, 10);
            TouchKeyMonitorApp mRHKeysMonitor(mDisplay, mControlMapper.mRHMPR, "RH Keys Monitor", 0, 4);

            IDisplayApp *allApps[] =
                {
                    &mPerformanceApp, // nice to have this as front page to know if things are running healthy.

                    &mSynthPatchApp,
                    &mSynthSettingsApp,
                    &mHarmVoiceSettingsApp,

                    &mMetronomeSettingsApp,
                    &mSystemSettingsApp,
                    &mAudioMonitorApp,

                    &mDebugDisplayApp,
                    &mLHKeysMonitor,
                    &mRHKeysMonitor,
                };

            mInputDelegator.Init(&mAppSettings, &mControlMapper);

            mAppSettings.mControlMappings[breathMappingIndex] = ControlMapping::MakeUnipolarMapping(PhysicalControl::Breath, ControlMapping::Function::Breath, 0.10f, 0.5f);
            mAppSettings.mControlMappings[breathMappingIndex].mUnipolarMapping.mCurveP = 0.50f;
            mAppSettings.mControlMappings[breathMappingIndex].mUnipolarMapping.mCurveS = 0;

            mAppSettings.mControlMappings[pitchUpMappingIndex] = ControlMapping::MakeUnipolarMapping(PhysicalControl::Pitch, ControlMapping::Function::PitchBend, 0.0f, 1.0f);
            mAppSettings.mControlMappings[pitchUpMappingIndex].mOperator = ControlMapping::Operator::Set;
            mAppSettings.mControlMappings[pitchUpMappingIndex].mUnipolarMapping.mSrcMin = 0.43f;
            mAppSettings.mControlMappings[pitchUpMappingIndex].mUnipolarMapping.mSrcMax = 0.06f;
            mAppSettings.mControlMappings[pitchUpMappingIndex].mUnipolarMapping.mCurveP = 0.50f;
            mAppSettings.mControlMappings[pitchUpMappingIndex].mUnipolarMapping.mCurveS = 0;

            mAppSettings.mControlMappings[pitchDownMappingIndex] = ControlMapping::MakeUnipolarMapping(PhysicalControl::Pitch, ControlMapping::Function::PitchBend, 0.0f, 1.0f);
            mAppSettings.mControlMappings[pitchDownMappingIndex].mOperator = ControlMapping::Operator::Subtract;
            mAppSettings.mControlMappings[pitchDownMappingIndex].mUnipolarMapping.mSrcMin = 0.66f;
            mAppSettings.mControlMappings[pitchDownMappingIndex].mUnipolarMapping.mSrcMax = 0.92f;
            mAppSettings.mControlMappings[pitchDownMappingIndex].mUnipolarMapping.mCurveP = 0.50f;
            mAppSettings.mControlMappings[pitchDownMappingIndex].mUnipolarMapping.mCurveS = 0;

            size_t im = pitchDownMappingIndex + 1;

            mAppSettings.mControlMappings[++im] = ControlMapping::MomentaryMapping(PhysicalControl::Ok, ControlMapping::Function::MenuOK);
            mAppSettings.mControlMappings[++im] = ControlMapping::MomentaryMapping(PhysicalControl::Back, ControlMapping::Function::MenuBack);

            mAppSettings.mControlMappings[++im] = ControlMapping::MomentaryMapping(PhysicalControl::Oct1, ControlMapping::Function::Oct1);
            mAppSettings.mControlMappings[++im] = ControlMapping::MomentaryMapping(PhysicalControl::Oct2, ControlMapping::Function::Oct2);
            mAppSettings.mControlMappings[++im] = ControlMapping::MomentaryMapping(PhysicalControl::Oct3, ControlMapping::Function::Oct3);
            mAppSettings.mControlMappings[++im] = ControlMapping::MomentaryMapping(PhysicalControl::Oct4, ControlMapping::Function::Oct4);
            mAppSettings.mControlMappings[++im] = ControlMapping::MomentaryMapping(PhysicalControl::Oct5, ControlMapping::Function::Oct5);
            mAppSettings.mControlMappings[++im] = ControlMapping::MomentaryMapping(PhysicalControl::Oct6, ControlMapping::Function::Oct6);

            mAppSettings.mControlMappings[++im] = ControlMapping::MomentaryMapping(PhysicalControl::LHKey1, ControlMapping::Function::LH1);
            mAppSettings.mControlMappings[++im] = ControlMapping::MomentaryMapping(PhysicalControl::LHKey2, ControlMapping::Function::LH2);
            mAppSettings.mControlMappings[++im] = ControlMapping::MomentaryMapping(PhysicalControl::LHKey3, ControlMapping::Function::LH3);
            mAppSettings.mControlMappings[++im] = ControlMapping::MomentaryMapping(PhysicalControl::LHKey4, ControlMapping::Function::LH4);

            mAppSettings.mControlMappings[++im] = ControlMapping::MomentaryMapping(PhysicalControl::RHKey1, ControlMapping::Function::RH1);
            mAppSettings.mControlMappings[++im] = ControlMapping::MomentaryMapping(PhysicalControl::RHKey2, ControlMapping::Function::RH2);
            mAppSettings.mControlMappings[++im] = ControlMapping::MomentaryMapping(PhysicalControl::RHKey3, ControlMapping::Function::RH3);
            mAppSettings.mControlMappings[++im] = ControlMapping::MomentaryMapping(PhysicalControl::RHKey4, ControlMapping::Function::RH4);

            mAppSettings.mControlMappings[++im] = ControlMapping::TypicalEncoderMapping(PhysicalControl::Enc, ControlMapping::Function::MenuScrollA);

            mAppSettings.mControlMappings[++im] = ControlMapping::ButtonIncrementMapping(PhysicalControl::LHx2, ControlMapping::Function::SynthPreset, 1.0f);
            mAppSettings.mControlMappings[++im] = ControlMapping::ButtonIncrementMapping(PhysicalControl::LHx3, ControlMapping::Function::SynthPreset, -1.0f);

            mAppSettings.mControlMappings[++im] = ControlMapping::MomentaryMapping(PhysicalControl::RHx1, ControlMapping::Function::ModifierFine);
            mAppSettings.mControlMappings[++im] = ControlMapping::MomentaryMapping(PhysicalControl::RHx2, ControlMapping::Function::ModifierCourse);

            mAppSettings.mControlMappings[++im] = ControlMapping::MomentaryMapping(PhysicalControl::RHx3, ControlMapping::Function::LoopGo);
            mAppSettings.mControlMappings[++im] = ControlMapping::MomentaryMapping(PhysicalControl::RHx4, ControlMapping::Function::LoopStop);

            mDisplay.Init(&mAppSettings, &mInputDelegator, allApps);
            mMusicalStateTask.Init();

            Wire1.setClock(400000); // use high speed mode. default speed = 100k

            FunctionTask mDisplayTask1{this, [](void *cap) {
                                           Clarinoid2App *pThis = (Clarinoid2App *)cap;
                                           pThis->mDisplay.UpdateAndRenderTask();
                                       }};

            FunctionTask mDisplayTask2{this, [](void *cap) {
                                           Clarinoid2App *pThis = (Clarinoid2App *)cap;
                                           pThis->mDisplay.DisplayTask();
                                       }};

            // the "Musical state" is the most critical. So let's run it periodically, spread through the whole time slice.
            // display tasks are also very heavy. Display1 is update/state, Display2 is SPI render.
            // musical state = about <2400 microseconds
            // display1 = about <1500 microseconds
            // display2 = about <2000 microseconds
            // LED tasks tend to be almost instantaneous (~<10 microseconds) so they can all live in the same slot.
            NopTask nopTask;

            TaskPlanner tp {
                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(0), &mMusicalStateTask, "MusS0"},
                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(1), &mDisplayTask1, "Display1"},

                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(2500), &mMusicalStateTask, "MusS1"},
                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(2501), &mLed, "mLed"},

                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(5000), &mMusicalStateTask, "MusS2"},
                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(5001), &mDisplayTask2, "Display2"},

                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(7500), &mMusicalStateTask, "MusS3"},

                TaskPlanner::TaskDeadline{TimeSpan::FromMicros(10000), &nopTask, "Nop"},
            };

            mPerformanceApp.Init(&tp);

            tp.Main();
        }
    };

} // namespace clarinoid
