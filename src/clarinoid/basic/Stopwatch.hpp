
#pragma once

namespace clarinoid
{

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


// template<uint32_t TperiodMicros>
// struct PeriodicTimer
// {
//   Stopwatch mSw;

//   void Reset() {
//     mSw.Restart(0);
//   }

//   float GetBeatFloat() {
//     float f = abs(float(mSw.ElapsedMicros()) / TperiodMicros);
//     return f;
//   }
//   // returns 0-1 the time since the last "beat".
//   float GetBeatFrac() {
//     float f = GetBeatFloat();
//     return f - floor(f); // fractional part only.
//   }
//   int GetBeatInt() {
//     float f = GetBeatFloat();
//     return (int)floor(f);
//   }

//   int32_t BeatsToMicros(float b) const {
//     return b * TperiodMicros;
//   }
// };


constexpr uint32_t MillisToMicros(uint32_t ms)
{
  return ms * 1000;
}

constexpr uint32_t Micros(uint32_t ms)
{
  return ms;
}

constexpr uint32_t FPSToMicros(uint32_t fps)
{
  return 1000000 / fps;
}


// TODO: support smoothly changing periods, not jerky behavior like this.
struct PeriodicTimer
{
  Stopwatch mSw;
  uint32_t mPeriodMicros = MillisToMicros(100);

  explicit PeriodicTimer(uint32_t periodMicros) :
    mPeriodMicros(periodMicros)
  {
    CCASSERT(periodMicros != 0);
  }

  PeriodicTimer()
  {
  }

  void SetPeriod(uint32_t periodMicros)
  {
    CCASSERT(periodMicros != 0);
    mPeriodMicros = periodMicros;
  }

  void Reset() {
    mSw.Restart(0);
  }

  float GetBeatFloat() {
    float f = abs(float(mSw.ElapsedMicros()) / mPeriodMicros);
    return f;
  }
  // returns 0-1 the time since the last "beat".
  float GetBeatFrac() {
    float f = GetBeatFloat();
    return f - floor(f); // fractional part only.
  }
  int GetBeatInt() {
    float f = GetBeatFloat();
    return (int)floor(f);
  }

  int32_t BeatsToMicros(float b) const {
    return (int32_t)(b * mPeriodMicros);
  }
};


//////////////////////////////////////////////////////////////////////
template<uint32_t TperiodMS>
class CCThrottlerT
{
  uint32_t mPeriodStartMS;
  uint32_t mFirstPeriodStartMS;
public:
  CCThrottlerT()
  {
    mPeriodStartMS = mFirstPeriodStartMS = millis();
  }

  void Reset() {
    mPeriodStartMS = mFirstPeriodStartMS = millis();
  }

  bool IsReady() {
    return IsReady(TperiodMS);
  }

  float GetBeatFloat(uint32_t periodMS) const {
    auto now = millis(); // minus is more theoretically accurate but this serves the purpose just as well.
    float f = abs(float(now - mFirstPeriodStartMS) / periodMS);
    return f;
  }
  // returns 0-1 the time since the last "beat".
  float GetBeatFrac(uint32_t periodMS) const {
    float f = GetBeatFloat(periodMS);
    return f - floor(f); // fractional part only.
  }
  int GetBeatInt(uint32_t periodMS) const {
    float f = GetBeatFloat(periodMS);
    return (int)floor(f);
  }
  
  bool IsReady(uint32_t periodMS) {
    auto now = millis(); // minus is more theoretically accurate but this serves the purpose just as well.
    if (now - mPeriodStartMS < periodMS) {
      return false;
    }
    mPeriodStartMS += periodMS * ((now - mPeriodStartMS) / periodMS); // this potentially advances multiple periods if needed so we don't get backed up.
    return true;
  }
};


} // namespace clarinoid
