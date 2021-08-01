
#pragma once

#include <algorithm>
#include <cmath>

namespace clarinoid
{

template <typename T>
constexpr T gPI = T(3.1415926535897932385);

// https://www.kvraudio.com/forum/viewtopic.php?f=33&t=388650&start=45
inline float vox_fasttanh2(const float x)
{
    const float ax = fabsf(x);
    const float x2 = x * x;

    return (x * (2.45550750702956f + 2.45550750702956f * ax + (0.893229853513558f + 0.821226666969744f * ax) * x2) /
            (2.44506634652299f + (2.44506634652299f + x2) * fabsf(x + 0.814642734961073f * x * ax)));
}

// https://github.com/discohead/LXR_JCM/blob/14b4b06ce5c9f4a60528d0c2d181f47227ae87df/mainboard/LxrStm32/src/DSPAudio/ResonantFilter.c
// slightly faster than vox_fasttanh2, but it's slightly less accurate.
inline float discohead_fastTanh(float var)
{
    return 4.15f * var / (4.29f + var * var);
}

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

inline bool FloatEquals(float f1, float f2, float eps = 0.000001f)
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
static float RemapTo01(float x, float xmin, float xmax)
{
    if (FloatEquals(xmax - xmin, 0.0f))
        return x;
    x -= xmin;
    x /= xmax - xmin;
    return x;
}

// remap so src min to max become [0-1]. results are NOT clamped.
static float RemapTo01Clamped(float x, float xmin, float xmax)
{
    if (FloatEquals(xmax - xmin, 0.0f))
        return xmin;
    x -= xmin;
    x /= xmax - xmin;
    return Clamp(x, 0.0f, 1.0f);
}
// remap a 0-1 float val to a new min/max. no clamping performed anywhere so if the src is out of 0-1 range, the dest
// will be out of destMin-destMax range.
static float Remap01ToRange(float x01, float destMin, float destMax)
{
    if (FloatEquals(destMax - destMin, 0.0f))
        return destMin;
    x01 *= destMax - destMin;
    x01 += destMin;
    return x01;
}

// same as arduino's map() fn.
static float RemapToRange(float x, float amin, float amax, float bmin, float bmax)
{
  if (FloatEquals(amin - amax, 0.0f))
    return bmin;
  if (FloatEquals(bmin - bmax, 0.0f))
    return bmin;
  x -= amin;
  x /= amax - amin;
  x *= bmax - bmin;
  x += bmin;
  return x;
}

inline float Frac(float x)
{
    return x - floorf(x);
}

inline float blamp0(float x)
{
    return 1 / 3.0f * x * x * x;
}

inline float blamp1(float x)
{
    x = x - 1;
    return -1 / 3.0f * x * x * x;
}

inline float blep0(float x)
{
    return x * x;
}

inline float blep1(float x)
{
    x = 1 - x;
    return -x * x;
}

static inline float Sample16To32(int16_t s)
{
    return float(s) / 32768;
}

static inline float Sample16ToSignedRange(int16_t s, float pole)
{
    return Sample16To32(s) * pole;
}

static inline int16_t Sample32To16(float s)
{
    return (int16_t)(Clamp(s, 0.0f, 1.0f) * 32768.0f);
}

static constexpr float MIN_DECIBEL_GAIN = -60.0f;

/**
 * Converts a linear value to decibels.  Returns <= aMinDecibels if the linear
 * value is 0.
 */
inline float LinearToDecibels(float aLinearValue, float aMinDecibels = MIN_DECIBEL_GAIN)
{
    return aLinearValue ? 20.0f * std::log10(aLinearValue) : aMinDecibels;
}

/**
 * Converts a decibel value to a linear value.
 */
inline float DecibelsToLinear(float aDecibels, float aNegInfDecibels = MIN_DECIBEL_GAIN)
{
    float lin = std::pow(10.0f, 0.05f * aDecibels);
    if (lin <= aNegInfDecibels)
        return 0.0f;
    return lin;
}

String GetSignStr(float f)
{
    if (FloatEquals(f, 0.0f))
        return CHARSTR_NARROWPLUSMINUS;
    if (f > 0)
        return CHARSTR_NARROWPLUS;
    return CHARSTR_NARROWMINUS;
}

// always showing sign,
// no decimal places, just show integral value
// fixed width 2 digits
// negative infinity glyph
// decibels glyph
inline String DecibelsToIntString(float aDecibels, float aNegInfDecibels = MIN_DECIBEL_GAIN)
{
    if (aDecibels <= aNegInfDecibels)
    {
        return CHARSTR_NARROWMINUS CHARSTR_INFINITY CHARSTR_DB; // -oodb
    }
    String ret = GetSignStr(aDecibels);
    int iDecibels = (int)std::ceil(abs(aDecibels));
    if (iDecibels < 10)
    {
        ret.append(CHARSTR_DIGITWIDTHSPACE);
    }
    ret.append(iDecibels);
    ret.append(CHARSTR_DB);
    return ret;
}

// takes a linear X value and snaps it to integral values.
// when snapStrength is 0, no snapping occurs and the output is the same linear as input.
// when snapStrength is 1, this is effecitvely a floor() function.
inline float SnapPitchBend(float x, float snapStrength)
{
    if (snapStrength < 0.000001f)
        return x;
    float z = std::floor(x);
    if (snapStrength > 0.99999f)
        return z;
    float p = x - z;
    float q = snapStrength * 3;
    q = q * q * q * q * q;
    q += 1;
    return z + std::pow(p, q);
}

// this is all utilities for shaping curves using this style:
// https://www.desmos.com/calculator/3zhzwbfrxd

// it's flexible using few parameters, usable without graphical feedback,
// and can be adapted for many situations, so for the moment i pretty much only
// use it and linear
namespace Curve2
{
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
    UnipolarMapping(float srcMin, float srcMax, float destMin, float destMax, float p, float s)
        : mSrcMin(srcMin), mSrcMax(srcMax), mDestMin(destMin), mDestMax(destMax), mCurveP(p), mCurveS(s)
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

    bool operator!=(const UnipolarMapping &rhs) const
    {
        return !(*this == rhs);
    }

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
    // auto rem = val & mask;// wish this would work but it doesn't.
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

int ModularDistance(int period, int a, int b)
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
    TriangleWave()
    {
        SetFrequency(1.0f);
    }
    float GetValue01(uint64_t tmicros) const
    {
        float r = (float)(tmicros % mFrequencyMicros);
        r = r / mFrequencyMicros; // saw wave
        r -= .5f;                 // 1st half the wave is negative.
        r = ::fabsf(r);           // it's a triangle but half height.
        return r * 2;
    }
};

// wikipedia https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
template <typename T>
void drawLine(int x0, int y0, int x1, int y1, T &&drawPixel)
{
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy; /* error value e_xy */
    while (true)
    { /* loop */
        drawPixel(x0, y0, true);
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 >= dy)
        { /* e_xy+e_x > 0 */
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        { /* e_xy+e_y < 0 */
            err += dx;
            y0 += sy;
        }
    }
}

struct PieData
{
    PointF p0; // represents a0
    PointF p1; // represents a1
};

// adapted from
// https://stackoverflow.com/questions/58222657/generate-a-pieslice-in-c-without-using-the-pieslice-of-graphics-h
template <typename T>
PieData fillPie(float x0, float y0, float r, float a0, float a1, T &&drawPixel) // a0 < a1
{
    float x, y,     // circle centered point
        xx, yy, rr, // x^2,y^2,r^2
        ux, uy,     // u
        vx, vy,     // v
        sx, sy;     // pixel position
    rr = r * r;
    ux = (r)*cosf(a0);
    uy = (r)*sinf(a0);
    vx = (r)*cosf(a1);
    vy = (r)*sinf(a1);
    PieData ret;
    ret.p0 = PointF::Construct(ux, uy);
    ret.p1 = PointF::Construct(vx, vy);
    // handle big/small pies
    x = a1 - a0;
    if (x < 0)
        x = -x;
    // render small pies
    int pixelsDrawn = 0;
    if (x < gPI<float>) /* 180 deg */
    {
        for (y = -r, yy = y * y, sy = y0 + y; y <= +r; y++, yy = y * y, sy++)
        {
            for (x = -r, xx = x * x, sx = x0 + x; x <= +r; x++, xx = x * x, sx++)
            {
                if (xx + yy <= rr)                     // inside circle
                    if (((x * uy) - (y * ux) <= 0)     // x,y is above a0 in clockwise direction
                        && ((x * vy) - (y * vx) >= 0)) // x,y is below a1 in counter clockwise direction
                    {
                        drawPixel((int)::floorf(sx), (int)::floorf(sy), false);
                        ++pixelsDrawn;
                    }
            }
        }
        // drawLine(x0, y0, x0 + ux, y0 + uy, drawPixel);
    }
    else
    {
        for (y = -r, yy = y * y, sy = y0 + y; y <= +r; y++, yy = y * y, sy++)
        {
            for (x = -r, xx = x * x, sx = x0 + x; x <= +r; x++, xx = x * x, sx++)
            {
                if (xx + yy <= rr)
                {                                      // inside circle
                    if (((x * uy) - (y * ux) <= 0)     // x,y is above a0 in clockwise direction
                        || ((x * vy) - (y * vx) >= 0)) // x,y is below a1 in counter clockwise direction
                    {
                        drawPixel((int)::floorf(sx), (int)::floorf(sy), false);
                        ++pixelsDrawn;
                    }
                }
            }
        }
    }
    return ret;
}

// for adjusting numeric values with the settings menu editor...
template <typename T>
struct NumericEditRangeSpec
{
    static constexpr int DefaultCourseSteps = 12;
    static constexpr int DefaultNormalSteps = 24;
    static constexpr int DefaultFineSteps = 500;

    T mRangeMin;
    T mRangeMax;

    T mCourseStep;
    T mNormalStep;
    T mFineStep;

    explicit NumericEditRangeSpec(T rangeMin, T rangeMax) : mRangeMin(rangeMin), mRangeMax(rangeMax)
    {
        mCourseStep = (T)(float(rangeMax - rangeMin) / DefaultCourseSteps);
        mNormalStep = (T)(float(rangeMax - rangeMin) / DefaultNormalSteps);
        mFineStep = (T)(float(rangeMax - rangeMin) / DefaultFineSteps);
        if (mCourseStep == 0)
        {
            if (std::is_integral<T>::value)
                mCourseStep = 1;
            else
                mCourseStep = (T)1 / 100;
        }
        if (mNormalStep == 0)
        {
            if (std::is_integral<T>::value)
                mNormalStep = 1;
            else
                mNormalStep = (T)1 / 100;
        }
        if (mFineStep == 0)
        {
            if (std::is_integral<T>::value)
                mFineStep = 1;
            else
                mFineStep = (T)1 / 100;
        }
    }

    explicit NumericEditRangeSpec(T rangeMin, T rangeMax, T courseStep, T normalStep, T fineStep)
        : mRangeMin(rangeMin), mRangeMax(rangeMax), mCourseStep(courseStep), mNormalStep(normalStep),
          mFineStep(fineStep)
    {
    }

    virtual T AdjustValue(T f, int encoderIntDelta, bool isCoursePressed, bool isFinePressed)
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

    virtual float remap(T val, float newMin, float newMax) const
    {
        float v01 = RemapTo01((float)val, (float)mRangeMin, (float)mRangeMax);
        return Remap01ToRange(v01, newMin, newMax);
    }
};

// this is a weird funciton but it's convenient for decibel settings.
// it allows that below the minimum, there's an extra value of -inf db.
struct NumericEditRangeSpecWithBottom : NumericEditRangeSpec<float>
{
    static constexpr float BOTTOM_VALUE = MIN_DECIBEL_GAIN;

    explicit NumericEditRangeSpecWithBottom(float rangeMin, float rangeMax) : NumericEditRangeSpec(rangeMin, rangeMax)
    {
        CCASSERT(rangeMin > BOTTOM_VALUE);
    }

    explicit NumericEditRangeSpecWithBottom(float rangeMin,
                                            float rangeMax,
                                            float courseStep,
                                            float normalStep,
                                            float fineStep)
        : NumericEditRangeSpec(rangeMin, rangeMax, courseStep, normalStep, fineStep)
    {
        CCASSERT(rangeMin > BOTTOM_VALUE);
    }

    using T = float;

    bool IsBottom(T val) const
    {
        return val <= BOTTOM_VALUE;
    }

    virtual T AdjustValue(T f, int encoderIntDelta, bool isCoursePressed, bool isFinePressed) override
    {
        T step = mNormalStep;
        if (isCoursePressed && !isFinePressed)
            step = mCourseStep;
        if (isFinePressed && !isCoursePressed)
            step = mFineStep;

        T ret = f;

        if (encoderIntDelta > 0 && FloatEquals(f, BOTTOM_VALUE))
        {
            // handle the first encoder detent jumping from 0 to the range min.
            ret = mRangeMin;
            encoderIntDelta--;
        }

        ret += (step * encoderIntDelta);
        if (ret < mRangeMin)
        {
            ret = BOTTOM_VALUE;
        }
        else if (ret > mRangeMax)
        {
            ret = mRangeMax;
        }
        return ret;
    }
};

namespace StandardRangeSpecs
{
static const NumericEditRangeSpec<float> gFloat_0_1 =
    NumericEditRangeSpec<float>{0.0f, 1.0f, 0.2f /*course*/, 0.05f /*normal*/, 0.01f /*fine*/};
static const NumericEditRangeSpec<float> gFloat_N1_1 = NumericEditRangeSpec<float>{-1.0f, 1.0f};

// used by detune
static const NumericEditRangeSpec<float> gFloat_0_2 = NumericEditRangeSpec<float>{0.0f, 2.0f};

static const NumericEditRangeSpecWithBottom gMasterGainDb =
    NumericEditRangeSpecWithBottom{-30.0f, 12.0f, 3.0f /*course*/, 1.0f /*normal*/, 0.25f /*fine*/};
static const NumericEditRangeSpecWithBottom gGeneralGain =
    NumericEditRangeSpecWithBottom{-30.0f, 12.0f, 3.0f /*course*/, 1.0f /*normal*/, 0.25f /*fine*/};
static const NumericEditRangeSpecWithBottom gSendGain =
    NumericEditRangeSpecWithBottom{-30.0f, 0.0f, 3.0f /*course*/, 1.0f /*normal*/, 0.25f /*fine*/};

static const NumericEditRangeSpec<float> gPortamentoRange = NumericEditRangeSpec<float>{0.0f, 0.2f};

// osc pitch and global transpose
static const NumericEditRangeSpec<int> gTransposeRange = NumericEditRangeSpec<int>{-48, 48, 6, 1, 1};
static const NumericEditRangeSpec<float> gFreqMulRange = NumericEditRangeSpec<float>{0, 24, 1.0f, 0.10f, 0.05f};
static const NumericEditRangeSpec<float> gFreqOffsetRange = NumericEditRangeSpec<float>{-5000, 5000, 500, 100, 10};

static const NumericEditRangeSpec<float> gLFOFrequency = NumericEditRangeSpec<float>{0.01f, 15.0f};

// envelope
static const NumericEditRangeSpec<float> gEnvDelayMS = NumericEditRangeSpec<float>{0.0f, 2000.0f, 100, 25, 5};
static const NumericEditRangeSpec<float> gEnvAttackMS = NumericEditRangeSpec<float>{0.0f, 2000.0f, 100, 25, 5};
static const NumericEditRangeSpec<float> gEnvHoldMS = NumericEditRangeSpec<float>{0.0f, 2000.0f, 100, 25, 5};
static const NumericEditRangeSpec<float> gEnvDecayMS = NumericEditRangeSpec<float>{0.0f, 10000.0f, 300, 50, 10};
static const NumericEditRangeSpec<float> gEnvSustainLevel = gFloat_0_1;
static const NumericEditRangeSpec<float> gEnvReleaseMS = NumericEditRangeSpec<float>{0.0f, 10000.0f, 300, 50, 10};

static const NumericEditRangeSpec<float> gBPMRange = NumericEditRangeSpec<float>{20, 300, 10, 2, 1};
static const NumericEditRangeSpec<int> gMetronomeNoteRange = NumericEditRangeSpec<int>{20, 120, 10, 1, 1};
static const NumericEditRangeSpec<int> gMetronomeDecayRange = NumericEditRangeSpec<int>{1, 200, 10, 1, 1};
} // namespace StandardRangeSpecs

} // namespace clarinoid
