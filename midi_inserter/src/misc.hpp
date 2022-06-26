

#pragma once

#include "Stopwatch.hpp"

template <typename T>
T clamp(T v, T low, T top)
{
    if (v < low)
        return low;
    if (v > top)
        return top;
    return v;
}

template <typename T, size_t N>
constexpr size_t SizeofStaticArray(const T (&x)[N])
{
    return N;
}

template <typename byte, size_t lenB>
bool IsArrayEqual(size_t lenA, const byte *a, const byte (&b)[lenB])
{
    if (lenA != lenB)
        return false;
    for (int i = 0; i < (int)lenA; ++i)
    {
        if (a[i] != b[i])
            return false;
    }
    return true;
}

// returns true if A begins with B.
template <typename byte, size_t lenB>
bool ArrayBeginsWith(size_t lenA, const byte *a, const byte (&b)[lenB])
{
    if (lenB > lenA)
        return false;
    for (int i = 0; i < (int)lenB; ++i)
    {
        if (a[i] != b[i])
            return false;
    }
    return true;
}

String byteArrayToString(int len, const byte *data)
{
    String ret{"["};
    for (int i = 0; i < len; ++i)
    {
        ret += String((int)data[i], 16);
        ret += " ";
    }
    ret += "]";
    return ret;
}

struct Blinker
{
    clarinoid::Stopwatch sw;
    int mPeriodMS;
    explicit Blinker(int periodMS) : mPeriodMS(periodMS)
    {
    }

    bool GetState()
    {
        auto p2x = sw.ElapsedTime().ElapsedMillisI() % (mPeriodMS * 2);
        return p2x < mPeriodMS;
    }
};

// pin-driven LED which you trigger to blink it. the green LEDs, plus the RED led basically.
// doesn't actually drive the LED; just calculates the brightness level for caller to use.
struct TriggerLed
{
    clarinoid::Stopwatch sw;
    const int mDurationMS;
    explicit TriggerLed(int durationMS) : mDurationMS(durationMS)
    {
    }

    void Trigger()
    {
        sw.Restart();
    }

    double GetState(bool log = false)
    {
        //int elapsed = sw.ElapsedTime().ElapsedMillisI();
        // return elapsed > mDurationMS ? 0 : 1;

        int elapsed = sw.ElapsedTime().ElapsedMillisI();
        elapsed = mDurationMS - elapsed;
        double x = elapsed;
        x /= mDurationMS;
        if (x >= 0 && x <= 1 && log)
        {
            Serial.println(String("ElapsedMS:") + elapsed + ", durationMS:" + mDurationMS + ", x:" + x +
                           ", ret:" + clamp(x, (double)0.0, (double)1.0));
        }
        return clamp(x, (double)0.0, (double)1.0);
    }
};

struct BigButtonReader
{
    Bounce mBounce; // = Bounce(8, 3);

    clarinoid::Stopwatch sw; // reset each state change.
    int mPin;
    int mRelaxTimeMS;
    // bool mLastValue;

    BigButtonReader(int pin, int bounceIntervalMS, int relaxMS)
        : mBounce(pin, bounceIntervalMS), mPin(pin), mRelaxTimeMS(relaxMS)
    {
        pinMode(mPin, INPUT_PULLUP);
        // mLastValue = this->LiveRead();
        sw.Restart();
    }

    bool DidTrigger()
    {
        if (sw.ElapsedTime().ElapsedMillisI() < mRelaxTimeMS)
            return false;
        mBounce.update();
        bool r = mBounce.fallingEdge();
        if (r)
        {
            sw.Restart();
        }
        return r;
    }
};

struct LEDPin
{
    int mPin;
    TriggerLed fastPulse;
    TriggerLed slowPulse;
    double mBrightness;
    int mMinTriggerIntervalMS;
    clarinoid::Stopwatch mTriggerStopwatch;

    // static constexpr float gamma = 2.2f;

    LEDPin(int pin, int fastMS, int slowMS, double brightness, int minTriggerIntervalMS)
        : mPin(pin), fastPulse(fastMS), slowPulse(slowMS), mBrightness(brightness),
          mMinTriggerIntervalMS(minTriggerIntervalMS)
    {
        pinMode(mPin, OUTPUT);

        // setting analog resolution causes other problems and ugliness. don't.
        // see https://www.pjrc.com/teensy/td_pulse.html
        // on how to balance these values.
        // analogWriteResolution(10); // now analogWrite is 0 - 1023
        // analogWriteFrequency(pin, 46875);
    }

    void Trigger()
    {
        if (mTriggerStopwatch.ElapsedTime().ElapsedMillisI() < mMinTriggerIntervalMS)
            return;
        mTriggerStopwatch.Restart();
        fastPulse.Trigger();
        slowPulse.Trigger();
    }

    int GetAnalogLevel(double f)
    {
        // gamma correct; try to make these things a bit more linear.
        return clamp((int)(f * f * f * f * mBrightness * 255), 0, 255);
    }

    void SetLevel(double f)
    {
        analogWrite(mPin, GetAnalogLevel(f));
    }

    // called to present the trigger state
    void Present()
    {
        double val = std::max(fastPulse.GetState(), slowPulse.GetState() * 0.40);
        // double val = slowPulse.GetState();
        SetLevel(val);
    }
};
