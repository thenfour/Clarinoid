
#pragma once

#include <initializer_list>

#include "Profiler.hpp"
#include "function.hpp"
#include "Util.hpp"

namespace clarinoid
{

struct ITask
{
    virtual void TaskRun()
    {
        CCASSERT(false);
    }
};

struct NopTask : ITask
{
    virtual void TaskRun() override
    {
        // nop
    }
};

struct FunctionTask : ITask
{
    void *mpCapture = nullptr;
    cc::function<void(void *)>::ptr_t mPtr = nullptr;

    FunctionTask() = default;
    FunctionTask(void *capture, cc::function<void(void *)>::ptr_t fn) : mpCapture(capture), mPtr(fn)
    {
    }

    virtual void TaskRun() override
    {
        if (mPtr)
        {
            mPtr(mpCapture);
        }
    }
};

struct TaskExecutionData
{
    TimeSpan mDuration;
    TimeSpan mTTD; // negative = the task was overdue
};

struct TaskPlanner
{
    struct TaskInfo
    {
        const char *mName = "Unnamed task";
        ITask *mProc;

        uint32_t mExecutionCount = 0;
        SimpleMovingAverage<15> mExecutionTimeMicros;
        CircularArray<TaskExecutionData, 128> mExecutionHistory;

        void Run(TimeSpan ttd /* the ttd calculated by the task planner */)
        {
            Stopwatch sw;
            mProc->TaskRun();
            TimeSpan duration = sw.ElapsedTime();
            mExecutionCount++;
            mExecutionTimeMicros.Update((float)duration.ElapsedMicros());
            TaskExecutionData d;
            d.mDuration = duration;
            d.mTTD = ttd;
            mExecutionHistory.Push(d);
        }
    };

    struct TaskDeadline
    {
        TimeSpan mTimeSliceDeadline;
        TaskInfo mInfo;

        // put a NopTask at the end of your list to reset time slice and loop.
        TaskDeadline(TimeSpan timeSliceDeadline, ITask *task, const char *name) : mTimeSliceDeadline(timeSliceDeadline)
        {
            mInfo.mName = name;
            mInfo.mProc = task;
        }
    };

    struct TaskActionSpec
    {
        size_t mTaskDeadlineIndex;
        TaskDeadline *mTask; // when 0, notihng should be run and TTDmicros is actually the micros that we're safe to
                             // delay execution.
        TimeSpan mTTD;       // negative= this task was overdue. positive = task not due yet.
    };

    array_view<TaskDeadline> mTasks;

    TimeSpan mTimesliceDuration;
    TimeSpan mPreviousTimeSliceDelayTime;
    TimeSpan mCurrentTimeSliceDelayTime;

    size_t mTaskCursor = 0; // which task is next to execute.
    Stopwatch mTimesliceTimer;

    TaskPlanner(array_view<TaskDeadline> tasks) : mTasks(tasks)
    {
        // assert every task in order.
        TimeSpan lastTS = TimeSpan::Zero();
        for (size_t i = 0; i < mTaskCursor; ++i)
        {
            auto &t = mTasks[i];
            CCASSERT(t.mTimeSliceDeadline >= lastTS);
            lastTS = t.mTimeSliceDeadline;
        }

        mTimesliceDuration = mTasks[mTasks.mSize - 1].mTimeSliceDeadline;
        StartNewTimeSlice(TimeSpan::Zero());
    }

    void StartNewTimeSlice(TimeSpan newPos /* nonzero if you overran the existing plan */)
    {
        mPreviousTimeSliceDelayTime = mCurrentTimeSliceDelayTime;
        mCurrentTimeSliceDelayTime = TimeSpan::Zero();
        mTimesliceTimer.Restart(newPos);
        mTaskCursor = 0;
    }

    TaskActionSpec GetNextAction()
    {
        CCASSERT(mTasks.mSize);
        size_t index = mTaskCursor % mTasks.mSize;
        auto &t = mTasks[index];
        TimeSpan timeSlicePosition = mTimesliceTimer.ElapsedTime();
        TimeSpan ttd = t.mTimeSliceDeadline - timeSlicePosition;
        if (ttd <= TimeSpan::Zero())
        {
            // it's runnable without delay.
            return TaskActionSpec{index, &t, ttd};
        }

        // plan:                  |aaaa----bbbb----|
        // actual:     overhang 1  |aaaa
        //                              ^ ttd = 4, overhang = 1
        // we should delay 3, setting overhang to 0.
        return TaskActionSpec{0, nullptr, ttd};
    }

    void ExecuteAction(const TaskActionSpec &a)
    {
        if (!a.mTask)
        {
            mCurrentTimeSliceDelayTime += a.mTTD;
            delayMicroseconds((int)a.mTTD.ElapsedMicros());
            return;
        }

        a.mTask->mInfo.Run(a.mTTD);

        if (a.mTaskDeadlineIndex == (mTasks.mSize - 1))
        {
            // this is the last task of the plan. how much did we overshoot?
            TimeSpan tspos = mTimesliceTimer.ElapsedTime();
            CCASSERT(tspos >= mTimesliceDuration); // because we add delay()s, you should never end the slice early.
            StartNewTimeSlice(tspos - mTimesliceDuration);
        }

        mTaskCursor = a.mTaskDeadlineIndex + 1; // search next task.
    }

    void Main()
    {
        while (true)
        {
            auto a = GetNextAction();
            ExecuteAction(a);
            yield();
        }
    }
};

} // namespace clarinoid
