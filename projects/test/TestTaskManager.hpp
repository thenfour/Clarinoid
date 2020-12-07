

#pragma once


#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

  void TestPeriodicTimer()
  {
    SetTestClockMicros(0);
    PeriodicTimer pt(TimeSpan::FromMicros(100));
    Test(pt.GetBeatFloat() == 0.0f);
    Test(pt.GetBeatInt() == 0);
    Test(pt.GetBeatFrac() == 0.0f);

    SetTestClockMicros(25);
    Test(FloatEquals(pt.GetBeatFloat(), 0.25f));
    Test(pt.GetBeatInt() == 0);
    Test(FloatEquals(pt.GetBeatFrac(), 0.25f));

    SetTestClockMicros(100);
    Test(pt.GetBeatFloat() == 1.00f);
    Test(pt.GetBeatInt() == 1);
    Test(pt.GetBeatFrac() == 0.0f);

    SetTestClockMicros(125);
    Test(FloatEquals(pt.GetBeatFloat(), 1.25f));
    Test(pt.GetBeatInt() == 1);
    Test(FloatEquals(pt.GetBeatFrac(), 0.25f));

    // skip beats.
    SetTestClockMicros(1080);
    Test(FloatEquals(pt.GetBeatFloat(), 10.8f));
    Test(pt.GetBeatInt() == 10);
    Test(FloatEquals(pt.GetBeatFrac(), 0.8f));
  }

  void TestCircularArray()
  {
    {
      CircularArray<int, 2> arr;
      Test(arr.GetSize() == 0);
      arr.Push(13); // 13
      Test(arr.GetSize() == 1);
      Test(arr.GetElementAt(0) == 13);
      Test(arr.GetElementAt(1) == 13);
      Test(arr.GetElementAt(2) == 13);

      arr.Push(14); // 13 14
      Test(arr.GetSize() == 2);
      Test(arr.GetElementAt(0) == 13);
      Test(arr.GetElementAt(1) == 14);
      Test(arr.GetElementAt(2) == 13);

      arr.Push(15); // 15 14
      Test(arr.GetSize() == 2);
      Test(arr.GetElementAt(0) == 14);
      Test(arr.GetElementAt(1) == 15);
      Test(arr.GetElementAt(2) == 14);
    }
  }

  struct TestTask : ITask
  {
    int mExecCount = 0;
    int mDelayMicros = 0;
    virtual void TaskRun() override {
      mExecCount++;
      delayMicroseconds(mDelayMicros);
    }
  };

  void TestTaskPlanner()
  {
    {
      SetTestClockMicros(0);
      // basic test
      TestTask t1;
      t1.mDelayMicros = 700;
      TestTask t2;
      t2.mDelayMicros = 1100; // this will take too long.
      NopTask nt;
      TaskPlanner p
      {
        TaskPlanner::TaskDeadline { TimeSpan::Zero(), &t1, "t1" },
        TaskPlanner::TaskDeadline { TimeSpan::FromMicros(1000), &t2, "t2" },
        TaskPlanner::TaskDeadline { TimeSpan::FromMicros(2000), &nt, "nop" },
      };

      auto a = p.GetNextAction(); // T1
      TestEq(a.mTTD, TimeSpan::FromMicros(0)); // we're at the beginning of the timeslice and there's t1 ready to run.
      p.ExecuteAction(a);
      TestEq(t1.mExecCount, 1); // test that it was t1 that ran.
      TestEq(micros(), 700); // t1 duration was 500.

      a = p.GetNextAction(); // Delay 300
      TestEq(a.mTask, nullptr); // we need to wait 300ms
      TestEq(a.mTTD, TimeSpan::FromMicros(300));
      p.ExecuteAction(a);
      TestEq(t1.mExecCount, 1);
      TestEq(t2.mExecCount, 0);
      TestEq(micros(), 1000);

      a = p.GetNextAction(); // T2
      TestEq(a.mTTD, TimeSpan::FromMicros(0));
      p.ExecuteAction(a);
      TestEq(t2.mExecCount, 1);
      TestEq(micros(), 2100);
      //TestEq(p.mOverhangMicros, 0); // at this point the planner hasn't reached the end of the timeslice so overhang is 0.

      a = p.GetNextAction(); // NOP; new timeslice.
      p.ExecuteAction(a);
      TestEq(micros(), 2100); // nop task doesn't affect time.
                              // it's the last task so also test that timeslice is reset.

      a = p.GetNextAction(); // T1
      p.ExecuteAction(a);
      TestEq(micros(), 2800); // timeslice pos = 800
      TestEq(t1.mExecCount, 2);

      a = p.GetNextAction(); // Delay 200. previously it was 300 but because of overhang, delay less.
      TestEq(a.mTask, nullptr);
      TestEq(a.mTTD, TimeSpan::FromMicros(200));
      p.ExecuteAction(a);
      TestEq(micros(), 3000); // timeslice pos = 1000

      a = p.GetNextAction(); // T2
      TestEq(a.mTTD, TimeSpan::FromMicros(0));
      p.ExecuteAction(a);
      TestEq(micros(), 4100); // timeslice pos = 2100
      TestEq(p.mTimesliceTimer.ElapsedTime().ElapsedMicros(), 2100);

      a = p.GetNextAction(); // NOP; new timeslice.
      p.ExecuteAction(a);

      a = p.GetNextAction(); // T1
      p.ExecuteAction(a);

      a = p.GetNextAction(); // delay 200
      p.ExecuteAction(a);
      TestEq(a.mTask, nullptr);
      TestEq(a.mTTD, TimeSpan::FromMicros(200));

      a = p.GetNextAction(); // T2
      p.ExecuteAction(a);

      a = p.GetNextAction(); // NOP
      p.ExecuteAction(a);

      // OK let's make T1 and T2 both take too long. we should creep more delayed over time.
      t1.mDelayMicros = 1100;
      t2.mDelayMicros = 1100;

      a = p.GetNextAction(); // T1
      p.ExecuteAction(a);

      a = p.GetNextAction(); // T2
      TestEq(a.mTTD, TimeSpan::FromMicros(-200)); // 100 from previous lateness, 100 from this slice's t1.
      p.ExecuteAction(a);

      a = p.GetNextAction(); // NOP
      p.ExecuteAction(a);

      a = p.GetNextAction(); // T1
      TestEq(a.mTTD, TimeSpan::FromMicros(-300));
      p.ExecuteAction(a);

      a = p.GetNextAction(); // T2
      TestEq(a.mTTD, TimeSpan::FromMicros(-400));
      p.ExecuteAction(a);

      a = p.GetNextAction(); // NOP
      p.ExecuteAction(a);


      int b = 0;
    }
  }

  void TestUptime()
  {
    SetTestClockMicros(0);
    TestEq(Uptime().ElapsedMicros(), 0);
    delayMicroseconds(2);
    TestEq(UptimeMicros64(), 2);
    delayMicroseconds(0xffffffff);
    TestEq(UptimeMicros64(), 0x100000001);
    TestEq(Uptime().ElapsedMicros(), 0x100000001);
  }

  void TestTaskManager()
  {
    TestUptime();
    TestPeriodicTimer();
    TestCircularArray();
    TestTaskPlanner();

    //{
    //  // test simple common case, task execution duration = 0
    //  SetTestClockMicros(0);
    //  TaskManager tm;
    //  TestTask t1;
    //  TestTask t2;
    //  auto& t1Data = tm.mTasks[tm.AddTask(t1, 50, "high pri task", PriorityClass::Normal)];
    //  auto& t2Data = tm.mTasks[tm.AddTask(t2, 100, "led task", PriorityClass::Normal)];
    //  SetTestClockMicros(20);
    //  // ttd for both should be -2; they are both due at the same time.
    //  TestEq(t1Data.DueTimeMicros(), 0);
    //  TestEq(t2Data.DueTimeMicros(), 0);
    //  auto a = tm.GetNextAction();
    //  Test(a.mTask != nullptr);
    //  tm.ExecuteAction(a);
    //  Test(t1.mExecCount == 1);
    //  Test(t1Data.mNextDeadlineMicros == 50);
    //  SetTestClockMicros(40);
    //  TestEq(t1Data.DueTimeMicros(), 50);

    //  SetTestClockMicros(50);
    //  a = tm.GetNextAction();
    //  tm.ExecuteAction(a);
    //  Test(t2.mExecCount == 1); // it runs task 2 because it was due at 0, 50 micros ago.
    //  a = tm.GetNextAction();
    //  tm.ExecuteAction(a);
    //  Test(t1.mExecCount == 2); // now everything is satisfied and nothing is scheduled until 100.
    //  a = tm.GetNextAction();
    //  Test(a.mTask == nullptr);
    //  Test(a.mTTDMicros == 50);
    //  tm.ExecuteAction(a);
    //  a = tm.GetNextAction();
    //  TestEq(a.mTTDMicros, 0);
    //  TestEq(micros(), 100);

    //  // if a task skips an entire period, then it tries to request earlier execution.
    //  t2.mExecCount = 0;
    //  SetTestClockMicros(400);
    //  a = tm.GetNextAction();
    //  tm.ExecuteAction(a);
    //  TestEq(t2.mExecCount, 1);
    //  TestEq(t2Data.mNextDeadlineMicros, 450);
    //}

    //{
    //  // test critical task
    //  SetTestClockMicros(0);
    //  TaskManager tm;
    //  TestTask t1;
    //  TestTask t2;
    //  t1.mDelayMicros = 5;
    //  t2.mDelayMicros = 5;
    //  auto& t1Data = tm.mTasks[tm.AddTask(t1, 10, "t1", PriorityClass::Normal)]; // we're going to make both take 5 us to execute
    //  auto& t2Data = tm.mTasks[tm.AddTask(t2, 10, "t2", PriorityClass::Critical)];
    //  SetTestClockMicros(4);
    //  auto a = tm.GetNextAction(); // there is a critical task due; it will be t2.
    //  Test(a.mTask != nullptr);
    //  tm.ExecuteAction(a);
    //  TestEq(t2.mExecCount, 1);
    //  TestEq(micros(), 9);
    //  TestEq(t1Data.DueTimeMicros(), 0);
    //  TestEq(t2Data.DueTimeMicros(), 10);

    //  // right now there's not enough time for t1 to complete.
    //  a = tm.GetNextAction(); // no critical task due yet and t1 has never been run. so it's estimation is 0. it will get run.
    //  tm.ExecuteAction(a);
    //  TestEq(t1.mExecCount, 1);
    //  TestEq(micros(), 14);
    //  TestEq(t1Data.DueTimeMicros(), 10);
    //  TestEq(t2Data.DueTimeMicros(), 10);

    //  TestEq(t1Data.EstimateExecutionDurationMicros(), 5);
    //  TestEq(t2Data.EstimateExecutionDurationMicros(), 5);

    //  a = tm.GetNextAction(); // crit will be called again because it's overdue 4 us.
    //  tm.ExecuteAction(a);
    //  TestEq(t2.mExecCount, 2);
    //  TestEq(micros(), 19);
    //  TestEq(t1Data.DueTimeMicros(), 10); // 9 overdue.
    //  TestEq(t2Data.DueTimeMicros(), 20); // 1 left.

    //  a = tm.GetNextAction(); // we must wait for crit to be due. normal pri task would cause crit to be overdue, even though it's long overdue.
    //  tm.ExecuteAction(a);
    //  TestEq(t2.mExecCount, 2);
    //  TestEq(micros(), 20);

    //  a = tm.GetNextAction(); // crit is due; it will be run.
    //  tm.ExecuteAction(a);
    //  TestEq(t2.mExecCount, 3);
    //  TestEq(micros(), 25);
    //  TestEq(t2Data.DueTimeMicros(), 30);

    //  a = tm.GetNextAction(); // norm task is long overdue but it finally has the time to run.
    //  tm.ExecuteAction(a);
    //  TestEq(t1.mExecCount, 2);
    //  TestEq(micros(), 30);
    //}

    //{
    //  // test that equal tasks will alternate execution.
    //  SetTestClockMicros(0);
    //  TaskManager tm;
    //  TestTask t1;
    //  TestTask t2;
    //  t1.mDelayMicros = 5;
    //  t2.mDelayMicros = 5;
    //  auto& t1Data = tm.mTasks[tm.AddTask(t1, 10, "t1", PriorityClass::Normal)];
    //  auto& t2Data = tm.mTasks[tm.AddTask(t2, 10, "t2", PriorityClass::Normal)];

    //  for (int i = 0; i < 10; ++i)
    //  {
    //    auto a = tm.GetNextAction();
    //    tm.ExecuteAction(a);
    //    delayMicroseconds(i); // just to perturb things.

    //    a = tm.GetNextAction();
    //    tm.ExecuteAction(a);
    //    delayMicroseconds(i * 2);
    //    TestEq(t1.mExecCount, t2.mExecCount);
    //  }
    //}

  }




}



