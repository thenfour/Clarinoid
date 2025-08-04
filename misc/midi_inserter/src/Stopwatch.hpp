
#pragma once

namespace clarinoid
{

static uint32_t gUptimeLastMicrosCall = 0;
static int64_t gUptimeCurrentOffset = 0; // every time the 32-bit micros() call rolls over, this gets += 1<<32;

// for testing purposes we need to be able to act like the system is restarting from 0, uptime-wise.
void UptimeReset()
{
    gUptimeLastMicrosCall = 0;
    gUptimeCurrentOffset = 0;
}

int64_t UptimeMicros64()
{
    uint32_t m = micros();
    if (m < gUptimeLastMicrosCall)
    {
        gUptimeCurrentOffset += ((int64_t)1) << 32;
    }
    gUptimeLastMicrosCall = m;
    return gUptimeCurrentOffset + m;
}

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
} // namespace clarinoid
