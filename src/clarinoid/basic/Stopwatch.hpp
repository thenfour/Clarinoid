
#pragma once

//////////////////////////////////////////////////////////////////////
// initially running.
struct Stopwatch
{
  bool mIsRunning = true;
  uint64_t mPauseMicros = 0; // when we pause, remember the elapsedmicros() when we paused. later restart to here.

  uint64_t mExtra = 0; // store overflow here. yes there's still an overflow condition but this helps
  uint32_t mStartTime = 0;

  Stopwatch() {
    Restart();
  }

  // behave essentially like POD
  Stopwatch(const Stopwatch& rhs) = default;
  Stopwatch(Stopwatch&&) = default;
  Stopwatch& operator =(const Stopwatch& rhs) = default;
  Stopwatch& operator =(Stopwatch&&) = default;

  // also sets running state.
  void Restart(uint64_t newTime = 0)
  {
    mExtra = newTime;
    mStartTime = micros();
    mIsRunning = true;
  }

  uint32_t ElapsedMillis() {
    return (uint32_t) ElapsedMicros() / 1000;
  }

  uint64_t ElapsedMicros() {
    if (mIsRunning) {
      uint32_t now = micros();
      if (now < mStartTime) {
        mExtra += 0xffffffff - mStartTime;
        mStartTime = 0;
      }
      return (now - mStartTime) + mExtra;
    }

    // for paused, just return the pause time.
    return mPauseMicros;
  }

  void SetMicros(uint64_t m)
  {
    if (mIsRunning) {
      Restart(m);
      return;
    }

    // for paused, just set paused time.
    mPauseMicros = m;
  }

  void Pause() {
    if (!mIsRunning) return;
    mIsRunning = false;
    mPauseMicros = ElapsedMicros();
  }

  void Unpause() {
    if (mIsRunning) return;
    mIsRunning = true;
    Restart(mPauseMicros);
  }
};

