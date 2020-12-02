
#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/synth/Synth.hpp>
#include <clarinoid/bassoonoid/bsLed.hpp>
#include <clarinoid/bassoonoid/bsControlMapper.hpp>

namespace clarinoid
{

// this is a task which performs the meat of the bassoonoid musical state.
// 1. converting input data to musical data
// 2. updating the synth state
struct MusicalStateTask :
    ITask
{
    AppSettings* mAppSettings;
    BassoonoidControlMapper *mControlMapper;

    Metronome mMetronome;
    CCEWIMusicalState mMusicalState;
    CCSynth mSynth;
    ScaleFollower mScaleFollower;
    // MIDI here too eventually.

    MusicalStateTask(AppSettings* appSettings, BassoonoidControlMapper *controlMapper) : 
        mAppSettings(appSettings),
        mControlMapper(controlMapper),
        mMetronome(appSettings),
        mMusicalState(appSettings, controlMapper, &mMetronome, &mScaleFollower)
    {
    }

    void Init()
    {
        mSynth.Init(mAppSettings, &mMetronome);
    }

    virtual void TaskRun() override
    {
        mControlMapper->TaskRun();
        mMusicalState.Update();
        mSynth.Update(mMusicalState.mMusicalVoices, mMusicalState.mMusicalVoices + mMusicalState.mVoiceCount);
    }
};

} // namespace clarinoid
