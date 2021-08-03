
#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/synth/Synth.hpp>
#include "bsLed.hpp"
#include "bsControlMapper.hpp"

namespace clarinoid
{

// this is a task which performs the meat of the bassoonoid musical state.
// 1. converting input data to musical data
// 2. updating the synth state
struct MusicalStateTask : ITask
{
    AppSettings *mAppSettings;
    BassoonoidControlMapper *mControlMapper;
    InputDelegator *mpInput;

    Metronome mMetronome;
    CCEWIMusicalState mMusicalState;
    CCSynth mSynth;
    ScaleFollower mScaleFollower;
    // MIDI here too eventually.

    // for timing subtasks
    SimpleMovingAverage<15> mInputTiming;
    SimpleMovingAverage<15> mMusicalStateTiming;
    SimpleMovingAverage<15> mSynthStateTiming;

    MusicalStateTask(IDisplay* display, AppSettings *appSettings, InputDelegator *input, BassoonoidControlMapper *controlMapper)
        : mAppSettings(appSettings), mControlMapper(controlMapper), mpInput(input), mMetronome(*appSettings),
          mMusicalState(display, appSettings, mpInput, &mMetronome, &mScaleFollower, controlMapper)
    {
    }

    void Init()
    {
        mSynth.Init(mAppSettings, &mMetronome);
    }

    virtual void TaskRun() override
    {
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
