
#pragma once

#include "Uptime.hpp"

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

inline TimeSpan operator-(const TimeSpan &a, const TimeSpan &b)
{
    return TimeSpan{a.ElapsedMicros() - b.ElapsedMicros()};
}

inline TimeSpan operator+(const TimeSpan& a, const TimeSpan& b)
{
    return TimeSpan{a.ElapsedMicros() + b.ElapsedMicros()};
}

inline TimeSpan Uptime()
{
    return TimeSpan::FromMicros(UptimeMicros64());
}

} // namespace clarinoid
