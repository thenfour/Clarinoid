
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
#include <clarinoid/midi/midi.hpp>
#include <clarinoid/application/MusicalState.hpp>

#include <clarinoid/synth/Synth.hpp>

#include "bsLed.hpp"
#include "bsControlMapper.hpp"
#include "DebugDisplayApp.hpp"
#include <clarinoid/menu/MenuAppSynthSettings.hpp>
#include <clarinoid/menu/MenuAppMetronome.hpp>
#include "MusicalStateTask.hpp"

namespace clarinoid
{

struct BassoonoidApp : ILEDDataProvider
{
    static constexpr size_t breathMappingIndex = 13;
    static constexpr size_t joyPitchMappingIndex = 21;

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
    AudioMonitorApp mAudioMonitorApp;
    MetronomeSettingsApp mMetronomeSettingsApp;

    BassoonoidApp()
        : mLed1(this), mLed2(this), mBreathLED(this),
          mDisplay(128, 64, &SPI, 9 /*DC*/, 8 /*RST*/, 10 /*CS*/ /*, 44 * 1000000UL*/),
          mMusicalStateTask(&mAppSettings, &mInputDelegator, &mControlMapper),
          mPerformanceApp(mDisplay, &mMusicalStateTask, &mControlMapper),
          mDebugDisplayApp(mDisplay, mControlMapper, mMusicalStateTask),
          mSystemSettingsApp(
              mDisplay,
              breathMappingIndex,
              joyPitchMappingIndex,
              joyPitchMappingIndex,
              [](void *cap) { // raw breath value getter
                  BassoonoidApp *pThis = (BassoonoidApp *)cap;
                  return pThis->mControlMapper.mBreath.CurrentValue01();
              },
              [](void *cap) { // raw joy pitchbend value getter
                  BassoonoidApp *pThis = (BassoonoidApp *)cap;
                  return pThis->mControlMapper.mJoyY.CurrentValue01();
              },
              this),
          mSynthSettingsApp(mDisplay), mAudioMonitorApp(mDisplay),
          mMetronomeSettingsApp(&mMusicalStateTask.mMetronome, &mAppSettings, mDisplay)
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

    void Main()
    {
        mControlMapper.Init(&mDisplay);

        IDisplayApp *allApps[] = {
            &mPerformanceApp,
            &mAudioMonitorApp,
            &mDebugDisplayApp,
            &mSystemSettingsApp,
            &mSynthSettingsApp,
            &mMetronomeSettingsApp,
        };

        mInputDelegator.Init(&mAppSettings, &mControlMapper);

        mAppSettings.mControlMappings[0] =
            ControlMapping::MomentaryMapping(PhysicalControl::LHOk, ControlMapping::Function::MenuOK);
        mAppSettings.mControlMappings[1] =
            ControlMapping::MomentaryMapping(PhysicalControl::LHBack, ControlMapping::Function::MenuBack);

        mAppSettings.mControlMappings[2] =
            ControlMapping::MomentaryMapping(PhysicalControl::LHOct1, ControlMapping::Function::Oct1);
        mAppSettings.mControlMappings[3] =
            ControlMapping::MomentaryMapping(PhysicalControl::LHOct2, ControlMapping::Function::Oct2);
        mAppSettings.mControlMappings[4] =
            ControlMapping::MomentaryMapping(PhysicalControl::LHOct3, ControlMapping::Function::Oct3);

        mAppSettings.mControlMappings[5] =
            ControlMapping::MomentaryMapping(PhysicalControl::LHKey1, ControlMapping::Function::LH1);
        mAppSettings.mControlMappings[6] =
            ControlMapping::MomentaryMapping(PhysicalControl::LHKey2, ControlMapping::Function::LH2);
        mAppSettings.mControlMappings[7] =
            ControlMapping::MomentaryMapping(PhysicalControl::LHKey3, ControlMapping::Function::LH3);
        mAppSettings.mControlMappings[8] =
            ControlMapping::MomentaryMapping(PhysicalControl::LHKey4, ControlMapping::Function::LH4);

        mAppSettings.mControlMappings[9] =
            ControlMapping::MomentaryMapping(PhysicalControl::RHKey1, ControlMapping::Function::RH1);
        mAppSettings.mControlMappings[10] =
            ControlMapping::MomentaryMapping(PhysicalControl::RHKey2, ControlMapping::Function::RH2);
        mAppSettings.mControlMappings[11] =
            ControlMapping::MomentaryMapping(PhysicalControl::RHKey3, ControlMapping::Function::RH3);
        mAppSettings.mControlMappings[12] =
            ControlMapping::MomentaryMapping(PhysicalControl::RHKey4, ControlMapping::Function::RH4);

        mAppSettings.mControlMappings[breathMappingIndex] =
            ControlMapping::MakeUnipolarMapping(PhysicalControl::Breath, ControlMapping::Function::Breath, 0.11f, 0.5f);
        mAppSettings.mControlMappings[breathMappingIndex].mUnipolarMapping.mCurveP = 0.50f;
        mAppSettings.mControlMappings[breathMappingIndex].mUnipolarMapping.mCurveS = 0; //-0.15f;

        mAppSettings.mControlMappings[14] =
            ControlMapping::TypicalEncoderMapping(PhysicalControl::LHEnc, ControlMapping::Function::MenuScrollA);

        mAppSettings.mControlMappings[17] = ControlMapping::ButtonIncrementMapping(
            PhysicalControl::LHThx1, ControlMapping::Function::SynthPreset, 1.0f);
        mAppSettings.mControlMappings[18] = ControlMapping::ButtonIncrementMapping(
            PhysicalControl::LHThx2, ControlMapping::Function::SynthPreset, -1.0f);

        mAppSettings.mControlMappings[19] =
            ControlMapping::MomentaryMapping(PhysicalControl::RHx2, ControlMapping::Function::ModifierFine);
        mAppSettings.mControlMappings[20] =
            ControlMapping::MomentaryMapping(PhysicalControl::RHx3, ControlMapping::Function::ModifierCourse);

        mAppSettings.mControlMappings[joyPitchMappingIndex] = ControlMapping::MakeUnipolarMapping(
            PhysicalControl::JoyY, ControlMapping::Function::PitchBend, 1.0f, 0.01f, -1.0f, 1.0f);
        mAppSettings.mControlMappings[joyPitchMappingIndex].mUnipolarMapping.mCurveP = 0.50f;
        mAppSettings.mControlMappings[joyPitchMappingIndex].mUnipolarMapping.mCurveS = 0; //-0.50f;

        mAppSettings.mControlMappings[22] =
            ControlMapping::MomentaryMapping(PhysicalControl::RHx5, ControlMapping::Function::LoopGo);
        mAppSettings.mControlMappings[23] =
            ControlMapping::MomentaryMapping(PhysicalControl::RHx4, ControlMapping::Function::LoopStop);

        mAppSettings.mControlMappings[24] =
            ControlMapping::MomentaryMapping(PhysicalControl::RHTh3, ControlMapping::Function::BaseNoteHoldToggle);
        mAppSettings.mControlMappings[25] =
            ControlMapping::MomentaryMapping(PhysicalControl::RHTh2, ControlMapping::Function::MetronomeLEDToggle);

        auto InitBassoonoidPreset = [](SynthPreset &p,
                                       const char *name,
                                       ClarinoidFilterType filt,
                                       float filterKeyScaling,
                                       float q,
                                       float filterMaxFreq,
                                       bool octDown = false) {
            p.mName = name;
            p.mSync = false;
            p.mDetune = 0.0f;
            p.mVerbSend = 0;
            p.mDelaySend = 0;

            p.mOsc1Gain = 0.0f;

            if (octDown)
            {
                p.mOsc3Gain = 1.0f;
                p.mOsc3PitchSemis = -12;
            }
            else
            {
                p.mOsc3Gain = 0.0f;
            }

            p.mOsc1Waveform = OscWaveformShape::SawSync;
            p.mOsc2Waveform = OscWaveformShape::SawSync;
            p.mOsc3Waveform = OscWaveformShape::SawSync;
            p.mOsc2Gain = 1.0f;

            p.mFilterType = filt;
            p.mFilterMinFreq = 0.0f;
            p.mFilterMaxFreq = filterMaxFreq;
            p.mFilterSaturation = 0;
            p.mFilterQ = q;
            p.mFilterKeytracking = filterKeyScaling;
        };

        size_t i = 0;
        mAppSettings.mGlobalSynthPreset = SYNTH_PRESET_COUNT - 1;
        InitBassoonoidPreset(mAppSettings.mSynthSettings.mPresets[SYNTH_PRESET_COUNT - 1],
                             "Diode-ks7-q15",
                             ClarinoidFilterType::LP_Diode,
                             0.7f,
                             0.15f,
                             15000);
        // InitBassoonoidPreset(mAppSettings.mSynthSettings.mPresets[i++],
        //                      "Diode-ks7-q15-Oct",
        //                      ClarinoidFilterType::LP_Diode,
        //                      0.7f,
        //                      0.15f,
        //                      20000,
        //                      true);
        // InitBassoonoidPreset(mAppSettings.mSynthSettings.mPresets[i++],
        //                      "Diode-ks7-q0",
        //                      ClarinoidFilterType::LP_Diode,
        //                      0.7f,
        //                      0.0f,
        //                      10000);
        // InitBassoonoidPreset(mAppSettings.mSynthSettings.mPresets[i++],
        //                      "Diode-ks9-q15",
        //                      ClarinoidFilterType::LP_Diode,
        //                      0.9f,
        //                      0.15f,
        //                      10000);
        // InitBassoonoidPreset(mAppSettings.mSynthSettings.mPresets[i++],
        //                      "Diode-ks9-q0",
        //                      ClarinoidFilterType::LP_Diode,
        //                      0.9f,
        //                      0.0f,
        //                      10000);

        // InitBassoonoidPreset(mAppSettings.mSynthSettings.mPresets[i++],
        //                      "Moog-ks7-q15",
        //                      ClarinoidFilterType::LP_Moog4,
        //                      0.7f,
        //                      0.15f,
        //                      4000);
        // InitBassoonoidPreset(
        //     mAppSettings.mSynthSettings.mPresets[i++], "Moog-ks7-q0", ClarinoidFilterType::LP_Moog4, 0.7f, 0.0f, 4000);
        // InitBassoonoidPreset(mAppSettings.mSynthSettings.mPresets[i++],
        //                      "Moog-ks9-q15",
        //                      ClarinoidFilterType::LP_Moog4,
        //                      0.9f,
        //                      0.15f,
        //                      4000);
        // InitBassoonoidPreset(
        //     mAppSettings.mSynthSettings.mPresets[i++], "Moog-ks9-q0", ClarinoidFilterType::LP_Moog4, 0.9f, 0.0f, 4000);

        // InitBassoonoidPreset(
        //     mAppSettings.mSynthSettings.mPresets[i++], "K35-ks7-q15", ClarinoidFilterType::LP_K35, 0.7f, 0.15f, 750);
        // InitBassoonoidPreset(
        //     mAppSettings.mSynthSettings.mPresets[i++], "K35-ks7-q0", ClarinoidFilterType::LP_K35, 0.7f, 0.0f, 750);
        // InitBassoonoidPreset(
        //     mAppSettings.mSynthSettings.mPresets[i++], "K35-ks9-q15", ClarinoidFilterType::LP_K35, 0.9f, 0.15f, 750);
        // InitBassoonoidPreset(
        //     mAppSettings.mSynthSettings.mPresets[i++], "K35-ks9-q0", ClarinoidFilterType::LP_K35, 0.9f, 0.0f, 750);

        // mAppSettings.mSynthSettings.mPresets[i++].mName = "Sync";

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

        // the "Musical state" is the most critical. So let's run it periodically, spread through the whole time slice.
        // display tasks are also very heavy. Display1 is update/state, Display2 is SPI render.
        // musical state = about <2400 microseconds
        // display1 = about <1500 microseconds
        // display2 = about <2000 microseconds
        // LED tasks tend to be almost instantaneous (~<10 microseconds) so they can all live in the same slot.
        NopTask nopTask;

        TaskPlanner tp{
            TaskPlanner::TaskDeadline{TimeSpan::FromMicros(0), &mMusicalStateTask, "MusS0"},
            TaskPlanner::TaskDeadline{TimeSpan::FromMicros(1), &mDisplayTask1, "Display1"},

            TaskPlanner::TaskDeadline{TimeSpan::FromMicros(5000), &mMusicalStateTask, "MusS1"},
            TaskPlanner::TaskDeadline{TimeSpan::FromMicros(5001), &mLed1, "mLed1"},
            TaskPlanner::TaskDeadline{TimeSpan::FromMicros(5002), &mLed2, "mLed2"},
            TaskPlanner::TaskDeadline{TimeSpan::FromMicros(5003), &mBreathLED, "mBreathLED"},

            TaskPlanner::TaskDeadline{TimeSpan::FromMicros(9000), &mMusicalStateTask, "MusS2"},
            TaskPlanner::TaskDeadline{TimeSpan::FromMicros(9001), &mDisplayTask2, "Display2"},

            TaskPlanner::TaskDeadline{TimeSpan::FromMicros(14000), &mMusicalStateTask, "MusS3"},

            TaskPlanner::TaskDeadline{TimeSpan::FromMicros(18000), &nopTask, "Nop"},
        };

        mPerformanceApp.Init(&tp);

        tp.Main();
    }
};

} // namespace clarinoid
