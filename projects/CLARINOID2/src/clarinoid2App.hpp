
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
#include "clarinoid2DemoApp.hpp"
#include "CubeDemoApp.hpp"
#include "TorusDemoApp.hpp"
#include "GeodesicSphereDemoApp.hpp"
#include "clarinoid2RhythmGoniometer.hpp"
#include <clarinoid/menu/MenuAppSynthSettings.hpp>
#include <clarinoid/menu/MenuAppMetronome.hpp>
#include <clarinoid/menu/MenuAppHarmonizerSettings.hpp>
#include <clarinoid/menu/MenuAppPerformanceSettings.hpp>

#include <clarinoid/Gui/GuiPerformanceApp.hpp>
#include "clarinoid2BigPerfDisplayApp.hpp"

namespace clarinoid
{

// global for debugging / crash handling purposes
CCAdafruitSSD1306 gDisplay = {128, 64, &SPI, 9 /*DC*/, 8 /*RST*/, 10 /*CS*/, 10 * 1000000UL};

// global app settings instance; use gClarinoidDmaMem.gAppSettingsBuffer for backing and instantiate it using placement
// new.
AppSettings *gpAppSettings = reinterpret_cast<AppSettings *>(gClarinoidDmaMem.gAppSettingsBuffer);
StaticInit gAppSettingsInit([]() { new (gpAppSettings) AppSettings(); });

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
    // AppSettings mAppSettings;

    MusicalStateTask mMusicalStateTask;

    PerformanceApp mPerformanceApp;
    DebugDisplayApp mDebugDisplayApp;
    SystemSettingsApp mSystemSettingsApp;
    PerformancePatchSettingsApp mPerfPatchApp;
    SynthPatchMenuApp mSynthPatchApp;
    // AudioMonitorApp mAudioMonitorApp;
    DisplayTestApp mDisplayTestApp;
    // DemoApp mDemoApp{mDisplay, mMusicalStateTask};
    // CubeDemoApp mCubeDemoApp{mDisplay, mMusicalStateTask};
    // TorusDemoApp mTorusDemoApp{mDisplay, mMusicalStateTask};
    GeodesicSphereDemoApp mSphereDemoApp{mDisplay, mMusicalStateTask, *this};
    MetronomeSettingsApp mMetronomeSettingsApp;
    // HarmSettingsApp mHarmVoiceSettingsApp;
    HarmPatchSettingsApp mHarmPatchApp;

    GuiPerformanceApp mGuiPerformanceApp;    //(mDisplay, mMusicalStateTask.mMetronome);
    MPR121ConfigApp<10, 4> mMPR121ConfigApp; //(mDisplay, mControlMapper, mMusicalStateTask);
    ScaleDetectorApp mScaleDetectorApp;
    RhythmGoniometerApp mGoniometerApp;
    BigPerfDisplayApp mBigPerfDisplayApp;

    static constexpr size_t kGoniometerAppSize = sizeof(mGoniometerApp);

    TaskPlanner *mTaskPlanner = nullptr; // set after initializing it, late in the startup process.

    SimpleMovingAverage<30> mCPUUsage;
    PeakMeterUtility<2000, 300> mPeakMeter;
    static constexpr size_t kpeakmetersize = sizeof(mPeakMeter);

    Clarinoid2App()
        : mLed(this),                                                                                    //
          mDisplay(gDisplay),                                                                            //
          mHud(mDisplay, this),                                                                          //
          mMusicalStateTask(&mDisplay, gpAppSettings, &mInputDelegator, &mControlMapper),                //
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
          mPerfPatchApp(mDisplay),  //
          mSynthPatchApp(mDisplay), //
          mDisplayTestApp(mDisplay),
          // mAudioMonitorApp(mDisplay), //
          mMetronomeSettingsApp(&mMusicalStateTask.mMetronome, gpAppSettings, mDisplay),
          // mHarmVoiceSettingsApp(mDisplay, mMusicalStateTask.mMusicalState.mLooper.mHarmonizer), //
          mHarmPatchApp(mDisplay),                                       //
          mGuiPerformanceApp(mDisplay, mMusicalStateTask.mMetronome),    //
          mMPR121ConfigApp(mDisplay, mControlMapper, mMusicalStateTask), //
          mScaleDetectorApp(mDisplay, mMusicalStateTask),                //
          mGoniometerApp(mDisplay, mMusicalStateTask),                   //
          mBigPerfDisplayApp(mDisplay, mMusicalStateTask, *this)
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
    virtual std::array<SynthVoiceState, MAX_MUSICAL_VOICES> ISysInfoProvider_GetVoiceState() override
    {
        std::array<SynthVoiceState, MAX_MUSICAL_VOICES> voices{};

        for (size_t i = 0; i < mMusicalStateTask.mMusicalState.mVoiceCount; ++i)
        {
            const auto &mv = mMusicalStateTask.mMusicalState.mMusicalVoices[i];
            voices[i].mNote = mv.mMidiNote; // todo: transpose accounted for already?
            voices[i].mSynthPatchIndex = mv.mSynthPatchA;
            // todo: patch B?
            voices[i].mVoiceSource = mv.mVoiceSource;
            voices[i].mIsPlaying = mv.IsPlaying() && !mv.mIsNoteCurrentlyMuted;
        }

        return voices;
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
        return gpAppSettings;
    }

    void AddControlMapping(const ControlMapping &mapping)
    {
        if (mControlMappingInitIndex < MAX_CONTROL_MAPPINGS)
        {
            gpAppSettings->mControlMappings[mControlMappingInitIndex++] = mapping;
        }
    }

    size_t mControlMappingInitIndex = 0;

    void Main()
    {
        Wire1.setClock(400000); // use high speed mode. default speed = 100k
        mControlMapper.Init(&mDisplay);

        IDisplayApp *allApps[] = {
            &mSphereDemoApp,
            &mGuiPerformanceApp,
            //&mDisplayTestApp,

            &mPerformanceApp, // nice to have this as front page to know if things are running healthy.
            &mPerfPatchApp,
            &mHarmPatchApp,
            &mSynthPatchApp,

            //&mHarmVoiceSettingsApp,

            &mMetronomeSettingsApp,
            &mSystemSettingsApp,
            //&mAudioMonitorApp,

            &mDebugDisplayApp,
            &mMPR121ConfigApp,
            &mScaleDetectorApp,
            &mGoniometerApp,
            //&mDemoApp,
            //&mCubeDemoApp,
            //&mTorusDemoApp,
            &mBigPerfDisplayApp,
        };

        mInputDelegator.Init(gpAppSettings, &mControlMapper, &mMusicalStateTask.mMetronome);

        auto &mAppSettings = *gpAppSettings;

        mAppSettings.mControlMappings[breathMappingIndex] = ControlMapping::MakeUnipolarMapping(
            PhysicalControl::Breath, ControlMapping::Function::Breath, 0.102f, 0.35f);
        mAppSettings.mControlMappings[breathMappingIndex].mUnipolarMapping.mCurveP = 0.38f;
        mAppSettings.mControlMappings[breathMappingIndex].mUnipolarMapping.mCurveS = 0;
        mGuiPerformanceApp.mBreathCalibration = &(mAppSettings.mControlMappings[breathMappingIndex].mUnipolarMapping);

        mAppSettings.mControlMappings[pitchUpMappingIndex] = ControlMapping::MakeUnipolarMapping(
            PhysicalControl::Pitch, ControlMapping::Function::PitchBend, 0.0f, 1.0f);
        mAppSettings.mControlMappings[pitchUpMappingIndex].mOperator = ControlMapping::Operator::Set;
        mAppSettings.mControlMappings[pitchUpMappingIndex].mUnipolarMapping.mSrcMin = 0.45f;
        mAppSettings.mControlMappings[pitchUpMappingIndex].mUnipolarMapping.mSrcMax = 0.08f;
        mAppSettings.mControlMappings[pitchUpMappingIndex].mUnipolarMapping.mCurveP = 0.0f;
        mAppSettings.mControlMappings[pitchUpMappingIndex].mUnipolarMapping.mCurveS = -0.4f;

        mAppSettings.mControlMappings[pitchDownMappingIndex] = ControlMapping::MakeUnipolarMapping(
            PhysicalControl::Pitch, ControlMapping::Function::PitchBend, 0.0f, 1.0f);
        mAppSettings.mControlMappings[pitchDownMappingIndex].mOperator = ControlMapping::Operator::Subtract;
        mAppSettings.mControlMappings[pitchDownMappingIndex].mUnipolarMapping.mSrcMin = 0.66f;
        mAppSettings.mControlMappings[pitchDownMappingIndex].mUnipolarMapping.mSrcMax = 0.92f;
        mAppSettings.mControlMappings[pitchDownMappingIndex].mUnipolarMapping.mCurveP = 0.50f;
        mAppSettings.mControlMappings[pitchDownMappingIndex].mUnipolarMapping.mCurveS = 0;

        mControlMappingInitIndex = pitchDownMappingIndex + 1;

        AddControlMapping(
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::Oct1, ControlMapping::Function::Oct1));
        AddControlMapping(
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::Oct2, ControlMapping::Function::Oct2));
        AddControlMapping(
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::Oct3, ControlMapping::Function::Oct3));
        AddControlMapping(
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::Oct4, ControlMapping::Function::Oct4));
        AddControlMapping(
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::Oct5, ControlMapping::Function::Oct5));
        AddControlMapping(
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::Oct6, ControlMapping::Function::Oct6));

        AddControlMapping(
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::LHKey1, ControlMapping::Function::LH1));
        AddControlMapping(
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::LHKey2, ControlMapping::Function::LH2));
        AddControlMapping(
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::LHKey3, ControlMapping::Function::LH3));
        AddControlMapping(
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::LHKey4, ControlMapping::Function::LH4));

        AddControlMapping(
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::RHKey1, ControlMapping::Function::RH1));
        AddControlMapping(
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::RHKey2, ControlMapping::Function::RH2));
        AddControlMapping(
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::RHKey3, ControlMapping::Function::RH3));
        AddControlMapping(
            ControlMapping::UniqueMomentaryMapping(PhysicalControl::RHKey4, ControlMapping::Function::RH4));

        AddControlMapping(
            ControlMapping::TypicalEncoderMapping(PhysicalControl::Enc, ControlMapping::Function::MenuScrollA));

        // Buttons ------------------------

        AddControlMapping(ControlMapping::MomentaryMapping(PhysicalControl::Ok, ControlMapping::Function::MenuOK));
        AddControlMapping(ControlMapping::MomentaryMapping(PhysicalControl::Back, ControlMapping::Function::MenuBack));

        AddControlMapping(ControlMapping::MomentaryMapping(
            PhysicalControl::LHKey1, ControlMapping::Function::Fine, ModifierKeyFlags::Any));
        AddControlMapping(ControlMapping::MomentaryMapping(
            PhysicalControl::LHKey2, ControlMapping::Function::Course, ModifierKeyFlags::Any));

        // No modifiers
        AddControlMapping(ControlMapping::MomentaryMapping(
            PhysicalControl::EncButton, ControlMapping::Function::SoftResetMpr121, ModifierKeyFlags::None));

        AddControlMapping(ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx4, ControlMapping::Function::PerfPreset, -1.0f, ModifierKeyFlags::None));
        AddControlMapping(ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx3, ControlMapping::Function::PerfPreset, 1.0f, ModifierKeyFlags::None));

        AddControlMapping(ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx2, ControlMapping::Function::MenuScrollA, -1.0f, ModifierKeyFlags::None));
        AddControlMapping(ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx1, ControlMapping::Function::MenuScrollA, 1.0f, ModifierKeyFlags::None));

        // LHx1 (modifier 1)
        mDisplay.RegisterPreToastRenderFunc(this, [](void *capture) FLASHMEM {
            auto pThis = (Clarinoid2App *)capture;
            if (pThis->mInputDelegator.mModifier1.CurrentValue())
            {
                pThis->mDisplay.SetupModal();
                pThis->mDisplay.println("dbl: fx toggle");
                pThis->mDisplay.println("rh1: synth A +");
                pThis->mDisplay.println("rh2: synth A -");
                pThis->mDisplay.println("rh3: synth B +");
                pThis->mDisplay.println("rh4: synth B -");
            }
        });
        AddControlMapping(ControlMapping::MomentaryMapping(
            PhysicalControl::LHx1, ControlMapping::Function::Mod1, ModifierKeyFlags::Any));

        AddControlMapping(ControlMapping::MomentaryMapping(PhysicalControl::LHx1,
                                                           ControlMapping::Function::EffectsEnabledToggle,
                                                           ModifierKeyFlags::Any,
                                                           ControlMapping::Activation::DoublePress));

        AddControlMapping(ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx4, ControlMapping::Function::SynthPresetA, -1.0f, ModifierKeyFlags::Mod1));
        AddControlMapping(ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx3, ControlMapping::Function::SynthPresetA, 1.0f, ModifierKeyFlags::Mod1));

        AddControlMapping(ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx2, ControlMapping::Function::SynthPresetB, -1.0f, ModifierKeyFlags::Mod1));
        AddControlMapping(ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx1, ControlMapping::Function::SynthPresetB, 1.0f, ModifierKeyFlags::Mod1));

        // LHx2 (modifier 2)
        mDisplay.RegisterPreToastRenderFunc(this, [](void *capture) FLASHMEM {
            auto pThis = (Clarinoid2App *)capture;
            if (pThis->mInputDelegator.mModifier2.CurrentValue())
            {
                pThis->mDisplay.SetupModal();
                pThis->mDisplay.println("dbl: harm enable");
                pThis->mDisplay.println("rh1: harm -");
                pThis->mDisplay.println("rh2: harm +");
                pThis->mDisplay.println("rh3: transpose -");
                pThis->mDisplay.println("rh4: transpose +");
            }
        });

        AddControlMapping(ControlMapping::MomentaryMapping(
            PhysicalControl::LHx2, ControlMapping::Function::Mod2, ModifierKeyFlags::Any));

        AddControlMapping(ControlMapping::MomentaryMapping(PhysicalControl::LHx2,
                                                           ControlMapping::Function::HarmPresetOnOffToggle,
                                                           ModifierKeyFlags::Any,
                                                           ControlMapping::Activation::DoublePress));

        AddControlMapping(ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx4, ControlMapping::Function::HarmPreset, -1.0f, ModifierKeyFlags::Mod2));
        AddControlMapping(ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx3, ControlMapping::Function::HarmPreset, 1.0f, ModifierKeyFlags::Mod2));

        AddControlMapping(ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx2, ControlMapping::Function::Transpose, -1.0f, ModifierKeyFlags::Mod2));
        AddControlMapping(ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx1, ControlMapping::Function::Transpose, 1.0f, ModifierKeyFlags::Mod2));

        // LHx3 (modifier 3)
        mDisplay.RegisterPreToastRenderFunc(this, [](void *capture) FLASHMEM {
            auto pThis = (Clarinoid2App *)capture;
            if (pThis->mInputDelegator.mModifier3.CurrentValue())
            {
                pThis->mDisplay.SetupModal();
                pThis->mDisplay.println("dbl: metronome enable");
                pThis->mDisplay.println("rh1: bpm -");
                pThis->mDisplay.println("rh2: bpm +");
                pThis->mDisplay.println("rh3: transpose B -");
                pThis->mDisplay.println("rh4: transpose B +");
            }
        });

        AddControlMapping(ControlMapping::MomentaryMapping(
            PhysicalControl::LHx3, ControlMapping::Function::Mod3, ModifierKeyFlags::Any));

        AddControlMapping(ControlMapping::MomentaryMapping(PhysicalControl::LHx3,
                                                           ControlMapping::Function::MetronomeToggle,
                                                           ModifierKeyFlags::Any,
                                                           ControlMapping::Activation::DoublePress));

        AddControlMapping(ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx4, ControlMapping::Function::GlobalTempo, -3.0f, ModifierKeyFlags::Mod3));
        AddControlMapping(ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx3, ControlMapping::Function::GlobalTempo, 3.0f, ModifierKeyFlags::Mod3));

        AddControlMapping(ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx2, ControlMapping::Function::TransposeB, -1.0f, ModifierKeyFlags::Mod3));
        AddControlMapping(ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx1, ControlMapping::Function::TransposeB, 1.0f, ModifierKeyFlags::Mod3));

        // LHx2 + LHx3 (modifier 2 + 3)
        mDisplay.RegisterPreToastRenderFunc(this, [](void *capture) FLASHMEM {
            auto pThis = (Clarinoid2App *)capture;
            if (pThis->mInputDelegator.mModifier2.CurrentValue() && pThis->mInputDelegator.mModifier3.CurrentValue())
            {
                pThis->mDisplay.SetupModal();
                pThis->mDisplay.println("ENC: deduced/chosen scale");
                pThis->mDisplay.println("rh1: key root -");
                pThis->mDisplay.println("rh2: key root +");
                pThis->mDisplay.println("rh3: scale type -");
                pThis->mDisplay.println("rh4: scale type +");
            }
        });

        AddControlMapping(
            ControlMapping::MomentaryMapping(PhysicalControl::EncButton,
                                             ControlMapping::Function::GlobalScaleDeducedToggle,
                                             CombineFlags(ModifierKeyFlags::Mod2, ModifierKeyFlags::Mod3)));

        auto m = CombineFlags(ModifierKeyFlags::Mod2, ModifierKeyFlags::Mod3);

        AddControlMapping(ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx4, ControlMapping::Function::GlobalKeyRoot, -1.0f, m));
        AddControlMapping(ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx3, ControlMapping::Function::GlobalKeyRoot, 1.0f, m));

        AddControlMapping(ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx2, ControlMapping::Function::GlobalKeyFlavor, -1.0f, m));
        AddControlMapping(ControlMapping::ButtonIncrementMapping(
            PhysicalControl::RHx1, ControlMapping::Function::GlobalKeyFlavor, 1.0f, m));

        CCASSERT(mControlMappingInitIndex <= MAX_CONTROL_MAPPINGS);

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

        /*
        {
          "totalTime": 24000,
          "tasks": [
            {
              "shortName": "enc",
              "codeSymbol": "mMusicalStateTask.mControlMapper->mEncoderTask",
              "intervalMicros": 3001,
              "delayMicros": 0
            },
            {
              "shortName": "mus",
              "codeSymbol": "mMusicalStateTask",
              "intervalMicros": 3001,
              "delayMicros": 0
            },
            {
              "shortName": "dispA",
              "codeSymbol": "mDisplayTask1",
              "intervalMicros": 24001,
              "delayMicros": 3000
            },
            {
              "shortName": "dispB",
              "codeSymbol": "mDisplayTask2",
              "intervalMicros": 24001,
              "delayMicros": 15000
            },
            {
              "shortName": "led",
              "codeSymbol": "mLed",
              "intervalMicros": 12001,
              "delayMicros": 0
            },
            {
              "shortName": "lhq",
              "codeSymbol": "mMPR121ConfigApp.mLHStatusQuerier",
              "intervalMicros": 8001,
              "delayMicros": 1000
            },
            {
              "shortName": "rhq",
              "codeSymbol": "mMPR121ConfigApp.mRHStatusQuerier",
              "intervalMicros": 8001,
              "delayMicros": 5000
            },
            {
              "shortName": "nop",
              "codeSymbol": "nopTask",
              "intervalMicros": 24001,
              "delayMicros": 24000
            }
          ]
        }
        */

        TaskPlanner::TaskDeadline plan[] = {
            {TimeSpan::FromMicros(0), &mMusicalStateTask.mControlMapper->mEncoderTask, "enc1"},
            {TimeSpan::FromMicros(0), &mMusicalStateTask, "mus1"},
            {TimeSpan::FromMicros(0), &mLed, "led1"},
            {TimeSpan::FromMicros(1000), &mMPR121ConfigApp.mLHStatusQuerier, "lhq1"},
            {TimeSpan::FromMicros(3000), &mDisplayTask1, "dispA1"},
            {TimeSpan::FromMicros(3001), &mMusicalStateTask.mControlMapper->mEncoderTask, "enc2"},
            {TimeSpan::FromMicros(3001), &mMusicalStateTask, "mus2"},
            {TimeSpan::FromMicros(5000), &mMPR121ConfigApp.mRHStatusQuerier, "rhq1"},
            {TimeSpan::FromMicros(6002), &mMusicalStateTask.mControlMapper->mEncoderTask, "enc3"},
            {TimeSpan::FromMicros(6002), &mMusicalStateTask, "mus3"},
            {TimeSpan::FromMicros(9001), &mMPR121ConfigApp.mLHStatusQuerier, "lhq2"},
            {TimeSpan::FromMicros(9003), &mMusicalStateTask.mControlMapper->mEncoderTask, "enc4"},
            {TimeSpan::FromMicros(9003), &mMusicalStateTask, "mus4"},
            {TimeSpan::FromMicros(12001), &mLed, "led2"},
            {TimeSpan::FromMicros(12004), &mMusicalStateTask.mControlMapper->mEncoderTask, "enc5"},
            {TimeSpan::FromMicros(12004), &mMusicalStateTask, "mus5"},
            {TimeSpan::FromMicros(13001), &mMPR121ConfigApp.mRHStatusQuerier, "rhq2"},
            {TimeSpan::FromMicros(15000), &mDisplayTask2, "dispB1"},
            {TimeSpan::FromMicros(15005), &mMusicalStateTask.mControlMapper->mEncoderTask, "enc6"},
            {TimeSpan::FromMicros(15005), &mMusicalStateTask, "mus6"},
            {TimeSpan::FromMicros(17002), &mMPR121ConfigApp.mLHStatusQuerier, "lhq3"},
            {TimeSpan::FromMicros(18006), &mMusicalStateTask.mControlMapper->mEncoderTask, "enc7"},
            {TimeSpan::FromMicros(18006), &mMusicalStateTask, "mus7"},
            {TimeSpan::FromMicros(21002), &mMPR121ConfigApp.mRHStatusQuerier, "rhq3"},
            {TimeSpan::FromMicros(21007), &mMusicalStateTask.mControlMapper->mEncoderTask, "enc8"},
            {TimeSpan::FromMicros(21007), &mMusicalStateTask, "mus8"},
            {TimeSpan::FromMicros(24000), &nopTask, "nop1"},
        };

        TaskPlanner tp = {plan};

        mTaskPlanner = &tp;
        mPerformanceApp.Init(&tp);

        AudioInterrupts();
        tp.Main();
    }
};

static constexpr size_t aoesunthchpchpcipciz = sizeof(Clarinoid2App);

} // namespace clarinoid
