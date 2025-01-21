
#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/synth/Synth.hpp>

namespace clarinoid
{

// this is a task which performs the meat of the musical state.
// 1. converting input data to musical data
// 2. updating the synth state
struct MusicalStateTask : ITask
{
    AppSettings *mAppSettings;
    Clarinoid2ControlMapper *mControlMapper;
    InputDelegator *mpInput;

    Metronome mMetronome;
    CCEWIMusicalState mMusicalState;
    CCSynth mSynth;
    ScaleDetector mScaleFollower;

    // for timing subtasks
    SimpleMovingAverage<15> mInputTiming;
    SimpleMovingAverage<15> mMusicalStateTiming;
    SimpleMovingAverage<15> mSynthStateTiming;

    SwitchControlReader mSoftResetMpr121Reader;
    SwitchControlReader mEffectsEnableToggleReader;
    SwitchControlReader mScaleDeducedToggleReader;

    MusicalStateTask(IDisplay *pDisplay,
                     AppSettings *appSettings,
                     InputDelegator *input,
                     Clarinoid2ControlMapper *controlMapper)
        : mAppSettings(appSettings), mControlMapper(controlMapper), mpInput(input), mMetronome(*appSettings),
          mMusicalState(pDisplay, appSettings, mpInput, &mMetronome, &mScaleFollower, controlMapper)
    {
    }

    void Init()
    {
        mSynth.Init(mAppSettings, &mMetronome);
    }

    virtual void TaskRun() override
    {
        mSoftResetMpr121Reader.Update(&mpInput->mSoftResetMpr121);
        if (mSoftResetMpr121Reader.IsNewlyPressed())
        {
            mControlMapper->InputSource_ShowToast(String("Soft reset MPR121"));
            mControlMapper->mLHMPR.SoftReset();
            mControlMapper->mRHMPR.SoftReset();
        }

        mEffectsEnableToggleReader.Update(&mpInput->mEffectEnableToggle);
        if (mEffectsEnableToggleReader.IsNewlyPressed())
        {
            mAppSettings->GetCurrentPerformancePatch().mMasterFXEnable = !mAppSettings->GetCurrentPerformancePatch().mMasterFXEnable;
            mControlMapper->InputSource_ShowToast(String("Effects: ") + (mAppSettings->GetCurrentPerformancePatch().mMasterFXEnable ? "ON" : "OFF"));
        }

        mScaleDeducedToggleReader.Update(&mpInput->mGlobalScaleDeducedToggle);
        if (mScaleDeducedToggleReader.IsNewlyPressed())
        {
            switch (mAppSettings->GetCurrentPerformancePatch().mGlobalScaleRef)
            {
                case GlobalScaleRefType::Chosen:
                default:
                    mAppSettings->GetCurrentPerformancePatch().mGlobalScaleRef = GlobalScaleRefType::Deduced;
                    mControlMapper->InputSource_ShowToast(String("Scale detector enabled"));
                    break;
                case GlobalScaleRefType::Deduced:
                    mAppSettings->GetCurrentPerformancePatch().mGlobalScaleRef = GlobalScaleRefType::Chosen;
                    mControlMapper->InputSource_ShowToast(String("Manual scale\n") + mAppSettings->GetCurrentPerformancePatch().mGlobalScale.ToString());
                    break;
            }

        }

        {
            NoInterrupts ni;
            int m1 = micros();
            mControlMapper->TaskRun();
            mpInput->Update();
            int m2 = micros();
            mInputTiming.Update((float)(m2 - m1));
        }
        {
            NoInterrupts ni;
            int m1 = micros();
            mMusicalState.Update();
            int m2 = micros();
            mMusicalStateTiming.Update((float)(m2 - m1));
        }

        {
            NoInterrupts ni;
            int m1 = micros();
            mSynth.Update(mMusicalState.mMusicalVoices, mMusicalState.mMusicalVoices + mMusicalState.mVoiceCount);
            int m2 = micros();
            mSynthStateTiming.Update((float)(m2 - m1));
        }
    }
};

} // namespace clarinoid
