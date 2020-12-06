
#pragma once

#include "Profiler.hpp"
#include "function.hpp"
#include "Util.hpp"

namespace clarinoid
{

struct ITask
{
  virtual void TaskRun() { CCASSERT(false); }
};

struct TaskExecutionData
{
  int mDurationMicros;
  int mTTD; // negative = the task was overdue
};

enum class PriorityClass
{
  Normal,
  Critical
};

struct TaskInfo
{
  const char* mName = "Unnamed task";
  ITask* mProc;
  PriorityClass mPriority = PriorityClass::Normal;

  int32_t mNextDeadlineMicros = 0;
  int32_t mPeriodMicros = 0;
  uint32_t mExecutionCount = 0;
  SimpleMovingAverage<15> mExecutionTimeMicros;
  CircularArray<TaskExecutionData, 128> mExecutionHistory;

  void Run(int32_t ttd /* the previous TTD you reported */)
  {
    //NoInterrupts ni; // for timing it's important that tasks don't get interrupted
    int m1 = micros();
    if (ttd < -mPeriodMicros) {
      // you had to skip an entire period. set deadline for 50% of period.
      mNextDeadlineMicros = micros() + (mPeriodMicros >> 1);
    }
    else {
      // execution within normal time range; set next deadline for next "beat".
      mNextDeadlineMicros = micros() + mPeriodMicros + ttd;
    }
    mProc->TaskRun();
    int m2 = micros();
    mExecutionCount++;
    mExecutionTimeMicros.Update((float)(m2 - m1));
    TaskExecutionData d;
    d.mDurationMicros = m2 - m1;
    d.mTTD = ttd;
    mExecutionHistory.Push(d);
  }

  int32_t EstimateExecutionDurationMicros()
  {
    if (mExecutionTimeMicros.GetValidSampleCount() == 0)
      return 0; // we don't know, therefore return a length of 0 to guarantee that we can at least someday execute and get this data.
    return (int32_t)mExecutionTimeMicros.GetValue();
  }

  int32_t DueTimeMicros()
  {
    return mNextDeadlineMicros;
  }
};

struct TaskManager
{
  static const size_t MAX_TASKS = 10;
  TaskInfo mCriticalTask;
  TaskInfo mTasks[MAX_TASKS];
  size_t mTaskCount = 0;

  size_t AddTask(ITask& proc, uint32_t periodMicros, const char* name, PriorityClass pri)
  {
    //log(String("taskman adding task ") + name);
    mTaskCount ++;
    CCASSERT(mTaskCount < MAX_TASKS);
    auto& t = mTasks[mTaskCount - 1];
    t.mPriority = pri;
    t.mProc = &proc;
    t.mPeriodMicros = periodMicros;
    //t.mTimer.SetPeriod(periodMicros);
    t.mName = name;
    return mTaskCount - 1;
  }

  size_t mTaskCursor = 0; // the index of the task queued up for examination. so it's the one AFTER the one that just ran.

  struct TaskActionSpec
  {
    size_t mTaskIndex;
    TaskInfo* mTask; // when 0, notihng should be run and TTDmicros is actually the micros that we're safe to delay execution.
    int mTTDMicros;
  };

  // determines what action to take right now.
  TaskActionSpec GetNextAction()
  {
    int32_t now = micros();
    int32_t minCritDueMicros = std::numeric_limits<int32_t>::max();

    // first figure out if any critical tasks are due, and if not, then when is the first one due?
    for (size_t it = 0; it < mTaskCount; ++ it)
    {
      TaskInfo& t = mTasks[it];
      if (t.mPriority != PriorityClass::Critical)
        continue;
      int32_t dueMicros = t.DueTimeMicros();
      if (dueMicros <= now) {
          // critical task due = always run.
          return { it, &t, dueMicros - now };
      }
      minCritDueMicros = std::min(minCritDueMicros, dueMicros);
    }

    // find a normal pri class task, starting at the cursor, which
    // - is due and won't cause a crit task to be delayed? run it.
    // if still nothing, it means there are no crit tasks due, and any normal
    // task would cause crit to be delayed. so delay until min of any task due
    // and try again.
    int32_t minNormDueMicros = std::numeric_limits<int32_t>::max();
    for (size_t c = 0; c < mTaskCount; ++c)
    {
      size_t it = (c + mTaskCursor) % mTaskCount;
      TaskInfo& t = mTasks[it];
      if (t.mPriority != PriorityClass::Normal)
        continue;
      int32_t dueMicros = t.DueTimeMicros();
      if (dueMicros <= now) {
          // task due. would it cause a delay?
        int32_t duration = t.EstimateExecutionDurationMicros();
        auto estCompletion = duration + now;
        if (estCompletion <= minCritDueMicros) {
          // OK we can squeeze it in.
          return { it, &t, dueMicros - now };
        }
      }
      else {
        // not due yet; maybe we can wait?
        minNormDueMicros = std::min(minNormDueMicros, dueMicros);
      }
    }
    return { 0, nullptr, std::min(minNormDueMicros, minCritDueMicros) - now };
  }

  void ExecuteAction(const TaskActionSpec& a)
  {
    if (!a.mTask)
    {
      delayMicroseconds(a.mTTDMicros);
      return;
    }
    a.mTask->Run(a.mTTDMicros);
    mTaskCursor = a.mTaskIndex + 1; // search next task.
  }


  void Main()
  {
    while (true)
    {
      auto a = GetNextAction();
      ExecuteAction(a);
    }
  }

};


constexpr auto aoe = sizeof(TaskManager);

} // namespace clarinoid
