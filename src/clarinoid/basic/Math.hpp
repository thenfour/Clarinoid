
#pragma once

#include <algorithm>

namespace clarinoid
{

  uint16_t ClampUint32ToUint16(uint32_t a)
  {
    if (a > std::numeric_limits<uint16_t>::max())
      return std::numeric_limits<uint16_t>::max();
    return (uint16_t)a;
  }

  inline int FloatRoundToInt(float f)
  {
    return ::floorf(f + 0.5f);
  }

  inline bool FloatEquals(float f1, float f2, float eps = 0.00001f)
  {
    return fabs(f1 - f2) < eps;
  }

  inline bool FloatRoundedEqualsInt(float f, int n)
  {
    return (int)FloatRoundToInt(f) == n;
  }

  static float Lerp(float a, float b, float t)
  {
    return a * (1.0f - t) + b * t;
  }

  static float Clamp(float x, float low, float hi)
  {
    if (x <= low)
      return low;
    if (x >= hi)
      return hi;
    return x;
  }

  static int ClampI(int x, int min, int max)
  {
    if (x <= min)
      return min;
    if (x >= max)
      return max;
    return x;
  }

  // this is all utilities for shaping curves using this style:
  // https://www.desmos.com/calculator/3zhzwbfrxd

  // it's flexible using few parameters, usable without graphical feedback,
  // and can be adapted for many situations, so for the moment i pretty much only
  // use it and linear
  namespace Curve2
  {
    static float Step01(float x, float thresh)
    {
      return x < thresh ? 0 : 1;
    }

    static float Curve2_F(float c, float x, float n)
    {
      float d = ::powf(n, c - 1);
      d = std::max(d, 0.0001f); // prevent div0
      return ::powf(x, c) / d;
    }

    // this is the basic function for the curve.
    // curves between x=0,1
    // returns 0,1
    static float Eval(float _x, float _p, float slope)
    {
      if (_x <= 0)
        return 0;
      if (_x >= 1)
        return 1;
      slope = clarinoid::Clamp(slope, -.999f, .999f);
      float c = (2 / (1.0f - slope)) - 1;
      if (_x <= _p)
      {
        return Curve2_F(c, _x, _p);
      }
      return 1.0f - Curve2_F(c, 1.0f - _x, 1.0f - _p);
    }
  } // namespace Curve2

  template <typename T, T divisor>
  void DivRem(T val, T &wholeParts, T &remainder)
  {
    wholeParts = val / divisor;
    remainder = val % divisor;
  }

  template <size_t divBits, typename Tval, typename Tremainder>
  void DivRemBitwise(Tval val, size_t &wholeParts, Tremainder &remainder)
  {
    static_assert(std::is_integral<Tval>::value, "must be integral");
    static_assert(std::is_integral<Tremainder>::value, "must be integral");
    auto mask = (1 << divBits) - 1;
    wholeParts = val / mask;
    //auto rem = val & mask;// wish this would work but it doesn't.
    auto rem = (val - (wholeParts * mask));
    CCASSERT((Tremainder)std::max((decltype(rem))0, rem) <= std::numeric_limits<Tremainder>::max());
    remainder = (Tremainder)rem;
  }

  static int RotateIntoRange(const int &val, const int &itemCount)
  {
    CCASSERT(itemCount > 0);
    int ret = val;
    while (ret < 0)
    {
      ret += itemCount; // todo: optimize
    }
    return ret % itemCount;
  }

  static uint8_t RotateIntoRangeByte(int8_t val, uint8_t itemCount)
  {
    CCASSERT(itemCount > 0);
    while (val < 0)
    {
      val += itemCount; // todo: optimize
    }
    return ((uint8_t)(val)) % itemCount;
  }

  // correction gets set to the # of rotations, neg, signed. basically an "adjustment".
  static uint8_t RotateIntoRangeByte(int8_t val, uint8_t itemCount, int8_t &correction)
  {
    CCASSERT(itemCount > 0);
    correction = 0;
    while (val < 0)
    {
      val += itemCount; // todo: optimize
      --correction;
    }
    while (val >= itemCount)
    {
      val -= itemCount; // todo: optimize
      ++correction;
    }
    return val;
  }

  static inline int AddConstrained(int orig, int delta, int min_, int max_)
  {
    CCASSERT(max_ >= min_);
    if (max_ <= min_)
      return min_;
    int ret = orig + delta;
    int period = max_ - min_ + 1; // if [0,0], add 1 each time to reach 0. If [3,6], add 4.
    while (ret < min_)
      ret += period;
    while (ret > max_)
      ret -= period;
    return ret;
  }

  // assumes T is integral
  // performs integral division but with common 0.5 rounding
  template <typename T>
  T idiv_round(T dividend, T divisor)
  {
    return (dividend + (divisor / 2)) / divisor;
  }

  template <int period>
  int ModularDistance(int a, int b)
  {
    a = RotateIntoRange(a, period);
    b = RotateIntoRange(b, period);
    if (a > b)
    {
      return std::min(a - b, b + period - a);
    }
    return std::min(b - a, a + period - b);
  }

  struct SawWave
  {
    uint32_t mFrequencyMicros = 1000000; // just 1 hz frequency default
    void SetFrequency(float f)
    {
      mFrequencyMicros = (uint32_t)(1000000.0f / f);
    }
    float GetValue01(uint64_t tmicros) const
    {
      float r = (float)(tmicros % mFrequencyMicros);
      return r / mFrequencyMicros;
    }
  };

  struct PulseWave
  {
    float mFrequency;
    uint32_t mFrequencyMicros;
    float mDutyCycle01;
    uint32_t mDutyCycleMicros;

    void SetFrequencyAndDutyCycle01(float freq, float dc01)
    {
      mFrequency = freq;
      mFrequencyMicros = (uint32_t)(1000000.0f / freq);
      mDutyCycle01 = dc01;
      mDutyCycleMicros = (uint32_t)(dc01 * mFrequencyMicros);
    }

    void SetFrequency(float f)
    {
      SetFrequencyAndDutyCycle01(f, mDutyCycle01);
    }

    void SetDutyCycle01(float dc01)
    {
      SetFrequencyAndDutyCycle01(mFrequency, dc01);
    }

    PulseWave()
    {
      SetFrequencyAndDutyCycle01(1.0f, 0.5f);
    }

    int GetValue01Int(uint64_t tmicros) const
    {

      uint64_t pos = (tmicros % mFrequencyMicros);
      return (pos < mDutyCycleMicros) ? 0 : 1;
    }
  };

  // for a triangle, it's just a modified sawtooth.
  struct TriangleWave
  {
    uint32_t mFrequencyMicros;
    void SetFrequency(float f)
    {
      mFrequencyMicros = (uint32_t)(1000000.0f / f);
    }
    TriangleWave() { SetFrequency(1.0f); }
    float GetValue01(uint64_t tmicros) const
    {
      float r = (float)(tmicros % mFrequencyMicros);
      r = r / mFrequencyMicros; // saw wave
      r -= .5f;                 // 1st half the wave is negative.
      r = ::fabsf(r);           // it's a triangle but half height.
      return r * 2;
    }
  };

} // namespace clarinoid
