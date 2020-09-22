#pragma once

//////////////////////////////////////////////////////////////////////
struct Stopwatch
{
  uint64_t mExtra = 0; // store overflow here. yes there's still overflow but this helps
  uint32_t mStartTime = 0;

  Stopwatch() {
    Restart();
  }

  // behave essentially like POD
  Stopwatch(const Stopwatch& rhs) = default;
  Stopwatch(Stopwatch&&) = default;
  Stopwatch& operator =(const Stopwatch& rhs) = default;
  Stopwatch& operator =(Stopwatch&&) = default;

  void Restart(uint64_t newTime = 0)
  {
    mExtra = newTime;
    mStartTime = micros();
  }

  uint32_t ElapsedMillis() {
    return (uint32_t) ElapsedMicros() / 1000;
  }

  uint64_t ElapsedMicros() {
    uint32_t now = micros();
    if (now < mStartTime) {
      mExtra += 0xffffffff - mStartTime;
      mStartTime = 0;
    }
    return (now - mStartTime) + mExtra;
  }
};

