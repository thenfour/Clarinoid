
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
    float ElapsedSeconds() const
    {
        double d = (double)mMicros;
        d /= 1000000; // convert micros to seconds.
        return (float)d;
    }
    float ElapsedBeats(float bpm) const
    {
        return (float)(double(mMicros) * bpm / 60000000);
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
    static TimeSpan FromBeats(float beats, float bpm)
    {
        return FromMillis(int64_t(beats * 60000.0f / bpm));
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

    void PauseAndReset(TimeSpan newTime = TimeSpan::FromMicros(0))
    {
        mExtraTime = TimeSpan::Zero();
        mPauseTime = newTime;
        mStartUptime = Uptime();
        mIsRunning = false;
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

template <uint32_t holdTimeMS, uint32_t falloffTimeMS>
struct GenericPeakMeterUtility
{
    float mHeldPeak = 0;
    Stopwatch mHeldPeakTime; // peak is simply held for a duration.

    float Update(float peak /*in*/)
    {
        float heldPeak = 0;
        // determine a new held peak
        // if the held peak has been holding longer than 500ms, fade linear to 0.
        uint32_t holdDurationMS = (uint32_t)mHeldPeakTime.ElapsedTime().ElapsedMillisI();
        if ((peak > mHeldPeak) || holdDurationMS > (holdTimeMS + falloffTimeMS))
        {
            // new peak, or after falloff reset.
            mHeldPeak = peak;
            heldPeak = peak;
            mHeldPeakTime.Restart();
        }
        else if (holdDurationMS <= holdTimeMS)
        {
            heldPeak = mHeldPeak;
        }
        else
        {
            // falloff: remap millis from 500-1000 from heldpeak to 0.
            heldPeak = RemapToRange(holdDurationMS, holdTimeMS, holdTimeMS + falloffTimeMS, mHeldPeak, peak);
        }
        return heldPeak;
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
