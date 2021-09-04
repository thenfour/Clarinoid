
#pragma once

#include <algorithm>
#include <cmath>
#include "fastonebigheader.hpp"

#ifdef CLARINOID_PLATFORM_X86
inline float arm_sin_f32(float x)
{
    return ::sinf(x);
}
inline float arm_cos_f32(float x)
{
    return ::cosf(x);
}
inline int16_t saturate16(int32_t n)
{
    if (n < std::numeric_limits<int16_t>::min())
        return std::numeric_limits<int16_t>::min();
    if (n > std::numeric_limits<int16_t>::max())
        return std::numeric_limits<int16_t>::max();
    return (int16_t)n;
}

void arm_q15_to_float(const int16_t *in, float *out, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        out[i] = in[i] / 32767.0f;
    }
}

void arm_fill_f32(float val, float *out, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        out[i] = val;
    }
}

void arm_fill_q15(int16_t val, int16_t *out, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        out[i] = val;
    }
}

void arm_add_q15(int16_t *pSrcA, int16_t *pSrcB, int16_t *pDst, uint32_t blockSize)
{
    for (uint32_t i = 0; i < blockSize; ++i)
    {
        pDst[i] = pSrcA[i] + pSrcB[i];
    }
}

void arm_offset_q15(int16_t *pSrc, int16_t offset, int16_t *pDst, uint32_t blockSize)
{
    for (uint32_t i = 0; i < blockSize; ++i)
    {
        pDst[i] = pSrc[i] + offset;
    }
}

void arm_scale_q15(int16_t *pSrc, int16_t scaleFract, int8_t shift, int16_t *pDst, uint32_t blockSize)
{
    for (uint32_t i = 0; i < blockSize; ++i)
    {
        pDst[i] = (int16_t)(pSrc[i] * (scaleFract / 32767.0f));
    }
}

// computes ((a[15:0] << 16) | b[15:0])
static inline uint32_t pack_16b_16b(int32_t a, int32_t b)
{
    return (a << 16) | (b & 0xffff);
}

// computes (((a[31:16] + b[31:16]) << 16) | (a[15:0 + b[15:0]))  (saturates)
static inline uint32_t signed_add_16_and_16(uint32_t a, uint32_t b)
{
    union packed {
        uint32_t u32;
        int16_t i16[2];
    };
    packed ap;
    ap.u32 = a;
    packed bp;
    bp.u32 = b;
    int32_t ret1 = ap.i16[0] + bp.i16[0];

    packed ret;
    ret.i16[0] = saturate16(ret1);
    ret.i16[1] = saturate16(int32_t(ap.i16[1]) + bp.i16[1]);
    return ret.u32;
}

// computes ((a[31:0] * b[15:0]) >> 16)
static inline int32_t signed_multiply_32x16b(int32_t a, uint32_t b)
{
    return ((int64_t)a * (int16_t)(b & 0xFFFF)) >> 16;
}

// computes ((a[31:0] * b[31:16]) >> 16)
static inline int32_t signed_multiply_32x16t(int32_t a, uint32_t b)
{
    return ((int64_t)a * (int16_t)(b >> 16)) >> 16;
}

// computes limit((val >> rshift), 2**bits)
static inline int32_t signed_saturate_rshift(int32_t val, int bits, int rshift)
{
    int32_t out, max;
    out = val >> rshift;
    max = 1 << (bits - 1);
    if (out >= 0)
    {
        if (out > max - 1)
            out = max - 1;
    }
    else
    {
        if (out < -max)
            out = -max;
    }
    return out;
}

#endif

namespace clarinoid
{
template <typename T>
constexpr T gPI = T(3.1415926535897932385);

template <typename T>
constexpr T gLog2of10 = T(3.3219280948873622);

// hand-selected evidence-based routines to prefer for performance. see experiments
namespace fast
{
// fastlog / fasterlog don't really give much difference in performance, but faster* is less precise.
inline float ln(float x)
{
    return fastmath::fastlog(x);
}
inline float log2(float x)
{
    return fastmath::fastlog2(x);
}
inline float log10(float x)
{
    return fastmath::fastlog2(x) / gLog2of10<float>;
}
inline float fasterlog10f(float x)
{
    return fastmath::fasterlog2(x) / gLog2of10<float>;
}
inline float pow(float x, float p)
{
    return fastmath::fastpow(x, p);
}

// just as fast as ::fastsin* etc.
inline float sin(float x)
{
    return arm_sin_f32(x);
}
inline float cos(float x)
{
    return arm_cos_f32(x);
}
// https://github.com/discohead/LXR_JCM/blob/14b4b06ce5c9f4a60528d0c2d181f47227ae87df/mainboard/LxrStm32/src/DSPAudio/ResonantFilter.c
// slightly faster than vox_fasttanh2, but it's slightly less accurate.
// inline float discohead_fastTanh(float var)
inline float tanh(float var)
{
    return 4.15f * var / (4.29f + var * var);
}

// // to consider:
// // https://www.kvraudio.com/forum/viewtopic.php?f=33&t=388650&start=45
// inline float vox_fasttanh2(const float x)
// {
//     const float ax = fabsf(x);
//     const float x2 = x * x;

//     return (x * (2.45550750702956f + 2.45550750702956f * ax + (0.893229853513558f + 0.821226666969744f * ax) * x2) /
//             (2.44506634652299f + (2.44506634652299f + x2) * fabsf(x + 0.814642734961073f * x * ax)));
// }

// http://blog.bjornroche.com/2009/12/int-float-int-its-jungle-out-there.html
// and some discussion:
// https://www.kvraudio.com/forum/viewtopic.php?f=33&t=414666&sid=df76550b6763db018d3d732d9606c0f8&start=15 basically,
// even though the "accurate" versions below will scale full range -1 to 1, they are not correct because values in the
// positive & negative should be equal, not scaled by 1/32768 to fill out the range. so instead of trynig to saturate
// the whole range, just accept that there's 1 value that won't be accessible. -32768,32767 -> -1,1
//
// NOTE : try not to process things sample-by-sample though.
// there are block operations for this kind of thing:
// https://arm-software.github.io/CMSIS_5/DSP/html/group__BasicAdd.html et al.
static inline float Sample16To32(int16_t s)
{
    if (s == -32768)
        return -1.0f;
    return s / 32767.0f;
}

// -32768,32767 -> -1,1
// NOTE : try not to process things sample-by-sample though.
// there are block operations for this kind of thing:
// https://arm-software.github.io/CMSIS_5/DSP/html/group__BasicAdd.html et al.
static inline int16_t Sample32To16(float s)
{
    // saturate16(int32_t(pf[i] * 32768.0f));
    return saturate16(int32_t(s * 32767.0f));
    // return (int16_t)(Clamp(s * 32767.0f, (float)std::numeric_limits<int16_t>::min(),
    // (float)std::numeric_limits<int16_t>::max()));
}

// for some reason this is still 2x faster than arm_float_to_q15.
inline void Sample32To16Buffer(const float *in, int16_t *out)
{
    for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
    {
        out[i] = Sample32To16(in[i]);
    }
}

// 3x faster than naive copy
// template <size_t N>
inline void Sample16To32Buffer(int16_t *in, float *out)
{
    arm_q15_to_float(in, out, AUDIO_BLOCK_SAMPLES);
}

// 20x faster than naive copy
inline void FillBufferWithConstant(float val, float *out)
{
    arm_fill_f32(val, out, AUDIO_BLOCK_SAMPLES);
}

// 20x faster than naive copy
inline void FillBufferWithConstant(int16_t val, int16_t *out)
{
    arm_fill_q15(val, out, AUDIO_BLOCK_SAMPLES);
}

// adding a constant to a buffer: arm_offset_q15  <-- ~2x faster than manual
// adding 2 buffers: arm_add_q15 <-- ~2x faster than manual
// multiply 2 buffers: arm_mult_q15 <-- this is actually a very interesting function, scaling everything, multiplying as
// a vector, and saturating, with possibility of scaling to allow >1 scales

} // namespace fast

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

static constexpr float FloatEpsilon = 0.000001f;

inline bool FloatEquals(float f1, float f2, float eps = FloatEpsilon)
{
    return fabs(f1 - f2) < eps;
}

inline bool FloatLessThanOrEquals(float lhs, float rhs, float eps = FloatEpsilon)
{
    return lhs <= (rhs + eps);
}

inline bool FloatIsAbove(float lhs, float rhs, float eps = FloatEpsilon)
{
    return lhs > (rhs + eps);
}

template <typename T>
inline bool NumberEquals(T lhs, T rhs)
{
    static_assert(std::is_integral<T>::value, "Integral required.");
    return lhs == rhs;
}

inline bool NumberEquals(float lhs, float rhs)
{
    return FloatEquals(lhs, rhs);
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

// x^3 curve centered around 0.
inline float blamp0(float x)
{
    return 1 / 3.0f * x * x * x;
}

// upside down from blamp0; y=0@x=1
inline float blamp1(float x)
{
    x = x - 1;
    return -1 / 3.0f * x * x * x;
}

// just a parabola, y=0 @x=0, y=1 @x=1
inline float blep0(float x)
{
    return x * x;
}

// upside down parabola where y=0@x=1.     y=-1 @ x=0
inline float blep1(float x)
{
    x = 1 - x;
    return -x * x;
}

// 1 hz = 1000 ms
// 2 hz = 500 ms
float HertzToCycleMS(float hz)
{
    return 1000.0f / hz;
}

float CycleMSToHertz(float ms)
{
    return 1000.0f / ms;
}

// 1/1 @ 60bpm = 1000 ms.
float NoteLengthToMS(float bpmIfNeeded, float numerator, float denom)
{
    return 60000.0f * numerator / denom;
}

static constexpr float MIN_DECIBEL_GAIN = -60.0f;

/**
 * Converts a linear value to decibels.  Returns <= aMinDecibels if the linear
 * value is 0.
 */
inline float LinearToDecibels(float aLinearValue, float aMinDecibels = MIN_DECIBEL_GAIN)
{
    return (aLinearValue > FloatEpsilon) ? 20.0f * fast::log10(aLinearValue) : aMinDecibels;
}

/**
 * Converts a decibel value to a linear value.
 */
inline float DecibelsToLinear(float aDecibels, float aNegInfDecibels = MIN_DECIBEL_GAIN)
{
    float lin = fast::pow(10.0f, 0.05f * aDecibels);
    if (lin <= aNegInfDecibels)
        return 0.0f;
    return lin;
}

template <typename T>
inline const char *GetSignStr(T f)
{
    if (NumberEquals(f, T(0)))
        return CHARSTR_NARROWPLUSMINUS;
    if (f > T(0))
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
    int iDecibels = (int)std::round(aDecibels);
    String ret = GetSignStr(iDecibels);
    if (abs(iDecibels) < 10)
    {
        ret.append(CHARSTR_DIGITWIDTHSPACE);
    }
    ret.append(abs(iDecibels));
    ret.append(CHARSTR_DB);
    return ret;
}

inline String GainToIntString(float gain)
{
    return DecibelsToIntString(LinearToDecibels(gain));
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
    return z + fast::pow(p, q);
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
    float d = fast::pow(n, c - 1);
    d = std::max(d, 0.0001f); // prevent div0
    return fast::pow(x, c) / d;
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
    ux = (r)*fast::cos(a0);
    uy = (r)*fast::sin(a0);
    vx = (r)*fast::cos(a1);
    vy = (r)*fast::sin(a1);
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

} // namespace clarinoid
