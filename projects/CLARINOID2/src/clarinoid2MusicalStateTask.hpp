
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
    ScaleFollower mScaleFollower;

    // for timing subtasks
    SimpleMovingAverage<15> mInputTiming;
    SimpleMovingAverage<15> mMusicalStateTiming;
    SimpleMovingAverage<15> mSynthStateTiming;

    MusicalStateTask(CCDisplay* pDisplay, AppSettings *appSettings, InputDelegator *input, Clarinoid2ControlMapper *controlMapper)
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
        {
            int m1 = micros();
            mControlMapper->TaskRun();
            mpInput->Update();
            int m2 = micros();
            mInputTiming.Update((float)(m2 - m1));
        }
        {
            int m1 = micros();
            mMusicalState.Update();
            int m2 = micros();
            mMusicalStateTiming.Update((float)(m2 - m1));
        }

        {
            // does its own interrupt disabling.
            int m1 = micros();
            mSynth.Update(mMusicalState.mMusicalVoices, mMusicalState.mMusicalVoices + mMusicalState.mVoiceCount);
            int m2 = micros();
            mSynthStateTiming.Update((float)(m2 - m1));
        }
    }
};

} // namespace clarinoid
