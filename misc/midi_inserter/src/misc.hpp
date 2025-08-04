#pragma once

#include "Stopwatch.hpp"

#undef B01 // from binary.h, i hate crap like this that conflicts

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

static float Lerp(float a, float b, float t)
{
    return a * (1.0f - t) + b * t;
}

// bits must be able to fit in a signed int size.
template <int TresolutionBits>
struct ColorT
{
    using this_t = ColorT<TresolutionBits>;
    int R = 0;
    int G = 0;
    int B = 0;

    float R01() const
    {
        return AnalogTo01(R);
    }
    float G01() const
    {
        return AnalogTo01(G);
    }
    float B01() const
    {
        return AnalogTo01(B);
    }

    static constexpr int MaxValue = (1 << TresolutionBits) - 1;

    float AnalogTo01(int x) const
    {
        return float(x) / MaxValue;
    }

    int _01ToAnalog(float x) const
    {
        return int(x * MaxValue);
    }

    ColorT()
    {
    }
    ColorT(int r, int g, int b) : R(r), G(g), B(b)
    {
    }
    ColorT(float r01, float g01, float b01) : R(_01ToAnalog(r01)), G(_01ToAnalog(g01)), B(_01ToAnalog(b01))
    {
    }

    bool Equals(const this_t &rhs) const
    {
        if (rhs.R != this->R)
            return false;
        if (rhs.G != this->G)
            return false;
        if (rhs.B != this->B)
            return false;
        return true;
    }

    int GetLevel() const
    {
        return R;
    }

    float GetLevel01() const
    {
        return AnalogTo01(this->R);
    }
    void SetLevel01(float x)
    {
        this->G = this->B = this->R = _01ToAnalog(x);
    }
    this_t &operator=(const this_t &rhs) = default;

    static this_t Mix(const this_t &lhs, const this_t &rhs, float x01)
    {
        if (x01 <= 0)
            return lhs;
        if (x01 >= 1)
            return rhs;
        return {Lerp(lhs.R01(), rhs.R01(), x01), Lerp(lhs.G01(), rhs.G01(), x01), Lerp(lhs.B01(), rhs.B01(), x01)};
    }
};

using Color = ColorT<gLEDPWMResolution>;

namespace Colors
{
static const Color Black = {};
static const Color White = {1.0f, 1.0f, 1.0f};
}; // namespace Colors

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
