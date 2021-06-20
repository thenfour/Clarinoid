
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
    return (int)::floorf(f + 0.5f);
  }

  inline bool FloatEquals(float f1, float f2, float eps = 0.00001f)
  {
    return fabs(f1 - f2) < eps;
  }

  inline bool FloatRoundedEqualsInt(float f, int n)
  {
    return (int)FloatRoundToInt(f) == n;
  }


  static float Clamp(float x, float low, float hi)
  {
    if (x <= low)
      return low;
    if (x >= hi)
      return hi;
    return x;
  }

  template <typename T>
  static T ClampInclusive(T x, T minInclusive, T maxInclusive)
  {
    if (x <= minInclusive)
      return minInclusive;
    if (x >= maxInclusive)
      return maxInclusive;
    return x;
  }

  // static float Lerp(float a, float b, float t)
  // {
  //   return a * (1.0f - t) + b * t;
  // }

  // remap so src min to max become [0-1]. results are NOT clamped.
  // if min-max == 0, just return x to avoid bad behaviors
  // static float RemapTo01(float x, float xmin, float xmax)
  // {
  //   if (FloatEquals(xmax - xmin, 0.0f))
  //     return x;
  //   x -= xmin;
  //   x /= xmax - xmin;
  //   return x;
  // }

  // remap so src min to max become [0-1]. results are NOT clamped.
  // if min-max == 0, just return x to avoid bad behaviors
  static float RemapTo01Clamped(float x, float xmin, float xmax)
  {
    if (FloatEquals(xmax - xmin, 0.0f))
      return x;
    x -= xmin;
    x /= xmax - xmin;
    return Clamp(x, 0.0f, 1.0f);
  }
  // remap a 0-1 float val to a new min/max. no clamping performed anywhere so if the src is out of 0-1 range, the dest will be out of destMin-destMax range.
  // if min-max == 0, just return x to avoid bad behaviors
  static float Remap01ToRange(float x01, float destMin, float destMax)
  {
    if (FloatEquals(destMax - destMin, 0.0f))
      return x01;
    x01 *= destMax - destMin;
    x01 += destMin;
    return x01;
  }

  // this is all utilities for shaping curves using this style:
  // https://www.desmos.com/calculator/3zhzwbfrxd

  // it's flexible using few parameters, usable without graphical feedback,
  // and can be adapted for many situations, so for the moment i pretty much only
  // use it and linear
  namespace Curve2
  {
    // static float Step01(float x, float thresh)
    // {
    //   return x < thresh ? 0.0f : 1.0f;
    // }

    static float Curve2_F(float c, float x, float n)
    {
      float d = ::powf(n, c - 1);
      d = std::max(d, 0.0001f); // prevent div0
      return ::powf(x, c) / d;
    }

    // this is the basic function for the curve.
    // curves between x=0,1
    // returns 0,1 clamped.
    static float Eval(float _x, float _p, float slope)
    {
      if (_x <= 0)
        return 0;
      if (_x >= 1)
        return 1;
      slope = clarinoid::Clamp(slope, -.9999f, .9999f);
      float c = (2 / (1.0f - slope)) - 1;
      if (_x <= _p)
      {
        return Curve2_F(c, _x, _p);
      }
      return 1.0f - Curve2_F(c, 1.0f - _x, 1.0f - _p);
    }
  } // namespace Curve2

  struct UnipolarMapping
  {
    float mSrcMin;
    float mSrcMax;
    float mDestMin;
    float mDestMax;
    float mCurveP; // curve position 0-1
    float mCurveS; // curve slope, -1 to 1 technically, but more like -.9 to +.9

    UnipolarMapping() = default;
    UnipolarMapping(float srcMin, float srcMax, float destMin, float destMax, float p, float s) : mSrcMin(srcMin),
                                                                                                  mSrcMax(srcMax),
                                                                                                  mDestMin(destMin),
                                                                                                  mDestMax(destMax),
                                                                                                  mCurveP(p),
                                                                                                  mCurveS(s)
    {
    }

    bool operator==(const UnipolarMapping &rhs) const
    {
      if (!FloatEquals(mSrcMin, rhs.mSrcMin))
        return false;
      if (!FloatEquals(mSrcMax, rhs.mSrcMax))
        return false;
      if (!FloatEquals(mDestMin, rhs.mDestMin))
        return false;
      if (!FloatEquals(mDestMax, rhs.mDestMax))
        return false;
      if (!FloatEquals(mCurveP, rhs.mCurveP))
        return false;
      if (!FloatEquals(mCurveS, rhs.mCurveS))
        return false;
      return true;
    }

    bool operator!=(const UnipolarMapping &rhs) const { return !(*this == rhs); }

    bool IsSrcInRegion(float src) const
    {
      return (src >= mSrcMin) && (src <= mSrcMax);
    }

    float PerformMapping(float src) const
    {
      // remap src to 0-1 range.
      float x = RemapTo01Clamped(src, mSrcMin, mSrcMax);
      // apply curve
      x = Curve2::Eval(x, mCurveP, mCurveS);
      // remap 0-1 to dest
      x = Remap01ToRange(x, mDestMin, mDestMax);
      return x;
    }
  };

  // for two-polar. like joystick X or pitch bend strip,
  // these are 0-1 source values, but we want to map them into -1 to 1 float values.
  // instead of the 1 region of unipolar defined by {min,max}
  // we have 3 regions (negative, zero, positive), defined by the boundaries.
  // looking at the whole source range,
  // |---------------------------------------------------------------|
  //    |neg_min------neg_max|-----|pos_min----------pos_max|
  // struct NPolarMapping
  // {
  //   UnipolarMapping mNegative; // used for unipolar mapping.
  //   UnipolarMapping mPositive;

  //   UnipolarMapping &Unipolar() { return mNegative; }

  //   float PerformUnipolarMapping(float src) const
  //   {
  //     return mNegative.PerformMapping(src);
  //   }

  //   float PerformBipolarMapping(float src) const
  //   {
  //     // in fact it's not "bipolar" in any enforced sense. it's just 2 regions. if it's in the negative region, we use it.
  //     // if it's in the positive region we use that.
  //     // but you could in theory make overlapping regions or other nonsensical regions which here we just don't bother.

  //     // determine which region to let evaluate this.
  //     // if it's IN either range then it's clear.
  //     if (mNegative.IsSrcInRegion(src))
  //     {
  //       return mNegative.PerformMapping(src);
  //     }
  //     if (mPositive.IsSrcInRegion(src))
  //     {
  //       return mPositive.PerformMapping(src);
  //     }

  //     // so figure out if src is closer to the neg or positive region.
  //     float negCenter = (mNegative.mSrcMin + mNegative.mSrcMax) / 2;
  //     float posCenter = (mPositive.mSrcMin + mPositive.mSrcMax) / 2;
  //     float distToNeg = fabs(src - negCenter);
  //     float distToPos = fabs(src - posCenter);
  //     if (distToNeg < distToPos)
  //     {
  //       return mNegative.PerformMapping(src);
  //     }
  //     return mPositive.PerformMapping(src);
  //   }
  // };

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

  // for adjusting numeric values with the settings menu editor...
  template <typename T>
  struct NumericEditRangeSpec
  {
    static constexpr int DefaultCourseSteps = 30;
    static constexpr int DefaultNormalSteps = 100;
    static constexpr int DefaultFineSteps = 1000;

    T mRangeMin;
    T mRangeMax;

    T mCourseStep;
    T mNormalStep;
    T mFineStep;

    explicit NumericEditRangeSpec(T rangeMin, T rangeMax) : mRangeMin(rangeMin),
                                                            mRangeMax(rangeMax)
    {
      mCourseStep = (T)(float(rangeMax - rangeMin) / DefaultCourseSteps);
      mNormalStep = (T)(float(rangeMax - rangeMin) / DefaultNormalSteps);
      mFineStep = (T)(float(rangeMax - rangeMin) / DefaultFineSteps);
      if (mCourseStep == 0) {
        if (std::is_integral<T>::value)
          mCourseStep = 1;
        else
          mCourseStep = (T)1 / 100;
      }
      if (mNormalStep == 0) {
        if (std::is_integral<T>::value)
          mNormalStep = 1;
        else
          mNormalStep = (T)1 / 100;
      }
      if (mFineStep == 0) {
        if (std::is_integral<T>::value)
          mFineStep = 1;
        else
          mFineStep = (T)1 / 100;
      }
    }

    explicit NumericEditRangeSpec(T rangeMin, T rangeMax, T courseStep, T normalStep, T fineStep) : mRangeMin(rangeMin),
                                                                                                    mRangeMax(rangeMax),
                                                                                                    mCourseStep(courseStep),
                                                                                                    mNormalStep(normalStep),
                                                                                                    mFineStep(fineStep)
    {
    }

    T AdjustValue(T f, int encoderIntDelta, bool isCoursePressed, bool isFinePressed)
    {
      T step = mNormalStep;
      if (isCoursePressed && !isFinePressed)
        step = mCourseStep;
      if (isFinePressed && !isCoursePressed)
        step = mFineStep;

      T ret = f + (step * encoderIntDelta);
      ret = ClampInclusive(ret, mRangeMin, mRangeMax);
      return ret;
    }
  };

  namespace StandardRangeSpecs
  {
    static const NumericEditRangeSpec<float> gFloat_0_1 = NumericEditRangeSpec<float> { 0.0f, 1.0f };
    static const NumericEditRangeSpec<float> gFloat_N1_1 = NumericEditRangeSpec<float> { -1.0f, 1.0f };
   
    // used by detune
    static const NumericEditRangeSpec<float> gFloat_0_2 = NumericEditRangeSpec<float> { 0.0f, 2.0f };

    static const NumericEditRangeSpec<float> gPortamentoRange = NumericEditRangeSpec<float> { 0.0f, 0.2f };

    // osc pitch and global transpose
    static const NumericEditRangeSpec<int> gTransposeRange = NumericEditRangeSpec<int> { -48, 48, 6, 1, 1 };

    static const NumericEditRangeSpec<float> gBPMRange = NumericEditRangeSpec<float> { 20, 300, 10, 2, 1 };
    static const NumericEditRangeSpec<int> gMetronomeNoteRange = NumericEditRangeSpec<int> { 20, 120, 10, 1, 1 };
    static const NumericEditRangeSpec<int> gMetronomeDecayRange = NumericEditRangeSpec<int> { 1, 200, 10, 1, 1 };
  }

} // namespace clarinoid
