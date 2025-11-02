
#pragma once

#include <type_traits>

namespace clarinoid
{
// int32_t micros...
// max 32 bit signed int = 2147483647 micros
// milliseconds = 2147483.647
// seconds = 2147.483647
// minutes = 35.79139411666667
// so 64 bit is needed for longer timespans.
template <typename T> // T must be signed integral type.
struct TimeSpanT
{
    static_assert(std::is_integral<T>::value, "TimeSpanT requires integral type");
    static_assert(std::is_signed<T>::value, "TimeSpanT requires signed type");

    using underlying_type = T;

  private:
    underlying_type mMicros =
        0; // signed, because it simplifies math, less error-prone, and is still an enormous amount of time.
  public:
    TimeSpanT()
    {
    }
    explicit TimeSpanT(underlying_type duration_micros) : mMicros(duration_micros)
    {
    }
    TimeSpanT(const TimeSpanT &rhs) = default;
    TimeSpanT(TimeSpanT &&rhs) = default;

    TimeSpanT &operator=(const TimeSpanT &rhs)
    {
        mMicros = rhs.mMicros;
        return *this;
    }
    TimeSpanT &operator-=(const TimeSpanT &rhs)
    {
        mMicros -= rhs.mMicros;
        return *this;
    }
    TimeSpanT &operator+=(const TimeSpanT &rhs)
    {
        mMicros += rhs.mMicros;
        return *this;
    }

    bool operator>(const TimeSpanT &rhs) const
    {
        return mMicros > rhs.mMicros;
    }
    bool operator>=(const TimeSpanT &rhs) const
    {
        return mMicros >= rhs.mMicros;
    }
    bool operator<(const TimeSpanT &rhs) const
    {
        return mMicros < rhs.mMicros;
    }
    bool operator<=(const TimeSpanT &rhs) const
    {
        return mMicros <= rhs.mMicros;
    }
    bool operator==(const TimeSpanT &rhs) const
    {
        return mMicros == rhs.mMicros;
    }
    bool operator!=(const TimeSpanT &rhs) const
    {
        return mMicros != rhs.mMicros;
    }

    underlying_type ElapsedMicros() const
    {
        return mMicros;
    }
    underlying_type ElapsedMillisI() const
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
        return (float(mMicros) * bpm / 60000000);
    }

    bool IsZero() const
    {
        return mMicros == 0;
    }
    bool IsNonZero() const
    {
        return mMicros != 0;
    }

    static TimeSpanT FromMicros(underlying_type m)
    {
        return TimeSpanT{m};
    }
    static TimeSpanT FromMillis(underlying_type m)
    {
        return TimeSpanT{m * 1000};
    }
    static TimeSpanT FromBPM(float bpm)
    {
        return FromMillis(underlying_type(60000.0f / bpm));
    }
    static TimeSpanT FromBeats(float beats, float bpm)
    {
        return FromMillis(underlying_type(beats * 60000.0f / bpm));
    }
    static TimeSpanT FromFPS(float bpm)
    {
        return FromMillis(underlying_type(1000.0f / bpm));
    }
    static TimeSpanT Zero()
    {
        return TimeSpanT{};
    }
};

using TimeSpan = TimeSpanT<int64_t>;

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

//////////////////////////////////////////////////////////////////////
// a stopwatch that has smaller footprint, light and optimized. it's always running.
// does not account for overflow
struct StopwatchLight
{
    uint32_t mStartMicros = 0;

    StopwatchLight()
    {
        Restart();
    }

    void Restart()
    {
        mStartMicros = micros();
    }

    uint32_t ElapsedMicros()
    {
        uint32_t now = micros();
        return now - mStartMicros;
    }
    TimeSpan ElapsedTime()
    {
        return TimeSpan::FromMicros(ElapsedMicros());
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
