
#pragma once

#include "Profiler.hpp"
#include "function.hpp"

namespace clarinoid
{

struct ITask
{
  virtual void TaskRun() { CCASSERT(false); }
};

struct TaskInfo
{
  const char* mName = "Unnamed task";
  PeriodicTimer mTimer;
  uint32_t mNextBeat = 0;
  ProfileObjectType mProfileObjectType;
  ITask* mProc;

  void Run()
  {
    mNextBeat = mTimer.GetBeatInt() + 1;
    // if mNextBeat is not (prev mNextBeat + 1), then we dropped a frame.
    mProc->TaskRun();
  }

  int32_t MicrosUntilDue()
  {
    return mTimer.BeatsToMicros(mNextBeat - mTimer.GetBeatFloat());
  }
};

struct TaskManager
{
  static const size_t MAX_TASKS = 100;
  TaskInfo mTasks[MAX_TASKS];
  size_t mTaskCount = 0;

  size_t AddTask(ITask& proc, uint32_t periodMicros, ProfileObjectType profileObjectType, const char* name)
  {
    log(String("taskman adding task ") + name);
    mTaskCount ++;
    CCASSERT(mTaskCount < MAX_TASKS);
    auto& t = mTasks[mTaskCount - 1];
    t.mProc = &proc;
    t.mTimer.SetPeriod(periodMicros);
    t.mProfileObjectType = profileObjectType;
    t.mName = name;
    return mTaskCount;
  }

  // take over execution and just keep tasks running.
  // there are any number of smarter ways to schedule tasks, but for the moment we just run the next ready.
  void Main()
  {
    log(String("taskman main. taskcount = ") + mTaskCount);
    uint32_t tasksRun = 0;
    CCASSERT(mTaskCount >= 1);

    // keeps track of the full trip.
    int32_t minSkippedTTDMicros = std::numeric_limits<int32_t>::max();
    size_t tasksSkipped = 0;

    for (size_t it = 0; /*true*/; it = (it + 1) % mTaskCount) { // deal with 1 task at a time, either running or skipping.
      TaskInfo& t = mTasks[it];
      int32_t ttdMicros = t.MicrosUntilDue();
      if (ttdMicros <= 0) {
        // due. run it.
        t.Run();
        tasksRun ++;
        tasksSkipped = 0;
        minSkippedTTDMicros = std::numeric_limits<int32_t>::max();
        continue;
      }

      // task not due yet.
      minSkippedTTDMicros = std::min(minSkippedTTDMicros, ttdMicros);
      tasksSkipped ++;

      if (tasksSkipped == mTaskCount) {
        delayMicroseconds(std::min(60000, (int)minSkippedTTDMicros));
        tasksSkipped = 0;
        minSkippedTTDMicros = std::numeric_limits<int32_t>::max();
      }
    } // for()
  }
};

} // namespace clarinoid
