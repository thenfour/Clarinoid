
#pragma once

namespace clarinoid
{
struct TimeSpan
{
  private:
    int64_t mMicros =
        0; // signed, because it simplifies math, less error-prone, and is still an enormous amount of time.
  public:
    TimeSpan()
    {
    }
    explicit TimeSpan(int64_t duration_micros) : mMicros(duration_micros)
    {
    }
    TimeSpan(const TimeSpan &rhs) = default;
    TimeSpan(TimeSpan &&rhs) = default;

    TimeSpan &operator=(const TimeSpan &rhs)
    {
        mMicros = rhs.mMicros;
        return *this;
    }
    TimeSpan &operator-=(const TimeSpan &rhs)
    {
        mMicros -= rhs.mMicros;
        return *this;
    }
    TimeSpan &operator+=(const TimeSpan &rhs)
    {
        mMicros += rhs.mMicros;
        return *this;
    }

    bool operator>(const TimeSpan &rhs) const
    {
        return mMicros > rhs.mMicros;
    }
    bool operator>=(const TimeSpan &rhs) const
    {
        return mMicros >= rhs.mMicros;
    }
    bool operator<(const TimeSpan &rhs) const
    {
        return mMicros < rhs.mMicros;
    }
    bool operator<=(const TimeSpan &rhs) const
    {
        return mMicros <= rhs.mMicros;
    }
    bool operator==(const TimeSpan &rhs) const
    {
        return mMicros == rhs.mMicros;
    }
    bool operator!=(const TimeSpan &rhs) const
    {
        return mMicros != rhs.mMicros;
    }

    int64_t ElapsedMicros() const
    {
        return mMicros;
    }
    int64_t ElapsedMillisI() const
    {
        return mMicros / 1000;
    }
    float ElapsedSeconds() const // note: LOSSY.
    {
        double d = (double)mMicros;
        d /= 1000000; // convert micros to seconds.
        return (float)d;
    }

    bool IsZero() const
    {
        return mMicros == 0;
    }
    bool IsNonZero() const
    {
        return mMicros != 0;
    }

    static TimeSpan FromMicros(int64_t m)
    {
        return TimeSpan{m};
    }
    static TimeSpan FromMillis(int64_t m)
    {
        return TimeSpan{m * 1000};
    }
    static TimeSpan FromBPM(float bpm)
    {
        return FromMillis(int64_t(60000.0f / bpm));
    }
    static TimeSpan FromFPS(float bpm)
    {
        return FromMillis(int64_t(1000.0f / bpm));
    }
    static TimeSpan Zero()
    {
        return TimeSpan{};
    }
};

TimeSpan operator-(const TimeSpan &a, const TimeSpan &b)
{
    return TimeSpan{a.ElapsedMicros() - b.ElapsedMicros()};
}
TimeSpan operator+(const TimeSpan &a, const TimeSpan &b)
{
    return TimeSpan{a.ElapsedMicros() + b.ElapsedMicros()};
}

TimeSpan Uptime()
{
    return TimeSpan::FromMicros(UptimeMicros64());
}

//////////////////////////////////////////////////////////////////////
// initially running.
struct Stopwatch
{
    bool mIsRunning = true;
    TimeSpan mPauseTime; // when we pause, remember the ElapsedTime() when we paused. later restart to here.
    TimeSpan mStartUptime;
    TimeSpan mExtraTime; // for specifying an offset.

    Stopwatch()
    {
        Restart();
    }

    // behave essentially like POD
    Stopwatch(const Stopwatch &rhs) = default;
    Stopwatch(Stopwatch &&) = default;
    Stopwatch &operator=(const Stopwatch &rhs) = default;
    Stopwatch &operator=(Stopwatch &&) = default;

    // also sets running state.
    void Restart(TimeSpan newTime = TimeSpan::FromMicros(0))
    {
        mExtraTime = newTime;
        mStartUptime = Uptime();
        mIsRunning = true;
    }

    TimeSpan ElapsedTime()
    {
        if (mIsRunning)
        {
            TimeSpan now = Uptime();
            return now - mStartUptime + mExtraTime;
        }
        return mPauseTime;
    }

    void Pause()
    {
        if (!mIsRunning)
        {
            return;
        }
        mPauseTime = ElapsedTime();
        mIsRunning = false;
    }

    void Unpause()
    {
        if (mIsRunning)
        {
            return;
        }
        mIsRunning = true;
        Restart(mPauseTime);
    }
};

// TODO: support smoothly changing periods, not jerky behavior like this.
struct PeriodicTimer
{
    Stopwatch mSw;
    TimeSpan mPeriod = TimeSpan::FromMillis(100);

    explicit PeriodicTimer(const TimeSpan &period) : mPeriod(period)
    {
        CCASSERT(mPeriod.IsNonZero());
    }

    PeriodicTimer()
    {
    }

    void SetPeriod(const TimeSpan &period)
    {
        CCASSERT(period.IsNonZero()); // avoid div0
        mPeriod = period;
    }

    void Reset()
    {
        mSw.Restart(TimeSpan::Zero());
    }

    float GetBeatFloat()
    {
        float f = (float)GetBeatInt();
        f += GetBeatFrac();
        return f;
    }
    // returns 0-1 the time since the last "beat".
    float GetBeatFrac()
    {
        auto beatMicros =
            mSw.ElapsedTime().ElapsedMicros() %
            mPeriod
                .ElapsedMicros(); // micros within the beat. mod it like this to ensure huge numbers don't cause issues.
        float frac = (float)beatMicros / mPeriod.ElapsedMicros();
        return frac;
    }
    uint32_t GetBeatInt()
    {
        auto beatInt = mSw.ElapsedTime().ElapsedMicros() / mPeriod.ElapsedMicros();
        return (uint32_t)beatInt;
    }

    TimeSpan BeatsToTimeSpan(float b) const
    {
        auto a = b * mPeriod.ElapsedMicros();
        return TimeSpan((int64_t)a);
    }
};

//////////////////////////////////////////////////////////////////////
template <uint32_t TperiodMS>
class CCThrottlerT
{
    uint32_t mPeriodStartMS;
    uint32_t mFirstPeriodStartMS;

  public:
    CCThrottlerT()
    {
        mPeriodStartMS = mFirstPeriodStartMS = millis();
    }

    void Reset()
    {
        mPeriodStartMS = mFirstPeriodStartMS = millis();
    }

    bool IsReady()
    {
        return IsReady(TperiodMS);
    }

    float GetBeatFloat(uint32_t periodMS) const
    {
        auto now = millis(); // minus is more theoretically accurate but this serves the purpose just as well.
        float f = abs(float(now - mFirstPeriodStartMS) / periodMS);
        return f;
    }
    // returns 0-1 the time since the last "beat".
    float GetBeatFrac(uint32_t periodMS) const
    {
        float f = GetBeatFloat(periodMS);
        return f - floor(f); // fractional part only.
    }
    int GetBeatInt(uint32_t periodMS) const
    {
        float f = GetBeatFloat(periodMS);
        return (int)floor(f);
    }

    bool IsReady(uint32_t periodMS)
    {
        auto now = millis(); // minus is more theoretically accurate but this serves the purpose just as well.
        if (now - mPeriodStartMS < periodMS)
        {
            return false;
        }
        mPeriodStartMS +=
            periodMS * ((now - mPeriodStartMS) /
                        periodMS); // this potentially advances multiple periods if needed so we don't get backed up.
        return true;
    }
};

} // namespace clarinoid
