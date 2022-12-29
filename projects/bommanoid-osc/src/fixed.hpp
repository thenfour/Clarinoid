// this is not a very complete or ideal solution.
// there are a lot of considerations that this was MEANT to abstract, but really can't, in the name of optimization.
// * normalized values don't actually go from 0-1. 1 would be out of range (32768 when int16_t max is 32767). that
//   creates DSP-specific special cases that should be considered.
// * arith operators should be optimized to be compatible between various FP configurations
// * a LOT of energy is spent in managing promotion, mostly to avoid promotion to 64-bit. That's a pretty manual
// operation,
//   and the dev needs to know everything going on behind-the-scenes for it to work. it means your use of Fixed<> will
//   almost always be more obfuscated than just using commented manual work.
// * because there are so many cases, and such care needs to be taken at the edge of ranges (in the case of normalized
// values),
//   all code needs to be tested carefully. so again what value is this really providing?
// * inconsistent mutability. i should probably stick to 100% immutable.

// But to be honest, it is still helpful in the case you want to work with FP numbers.
// because yes you need to understand all those details for each and every operation. That will now be expressed in the code.
// but tests are 100% necessary because it's just very fragile and really pushes your assumptions to the limit.

#pragma once

#include <stdint.h>

template <typename T>
struct FPTypeInfo
{
};

template <>
struct FPTypeInfo<uint8_t>
{
    using MyType = uint8_t;
    using PromotedType = uint16_t;
    static constexpr MyType NormalizedScale = 0xff;
    static constexpr int32_t MaxFractBits = 8;
};

template <>
struct FPTypeInfo<uint16_t>
{
    using MyType = uint16_t;
    using PromotedType = uint32_t;
    static constexpr MyType NormalizedScale = 0xffff;
    static constexpr int32_t MaxFractBits = 16;
};

template <>
struct FPTypeInfo<uint32_t>
{
    using MyType = uint32_t;
    using PromotedType = uint64_t;
    static constexpr MyType NormalizedScale = 0xffffffff;
    static constexpr int32_t MaxFractBits = 32;
};

template <>
struct FPTypeInfo<uint64_t>
{
    using MyType = uint64_t;
    using PromotedType = uint64_t;
    static constexpr MyType NormalizedScale = 0xffffffffffffffff;
    static constexpr int32_t MaxFractBits = 64;
};

template <>
struct FPTypeInfo<int8_t>
{
    using MyType = int8_t;
    using PromotedType = int16_t;
    static constexpr MyType NormalizedScale = 0x7f;
    static constexpr int32_t MaxFractBits = 7;
};

template <>
struct FPTypeInfo<int16_t>
{
    using MyType = int16_t;
    using PromotedType = int32_t;
    static constexpr MyType NormalizedScale = 0x7fff;
    static constexpr int32_t MaxFractBits = 15;
};

template <>
struct FPTypeInfo<int32_t>
{
    using MyType = int32_t;
    using PromotedType = int64_t;
    static constexpr MyType NormalizedScale = 0x7fffffff;
    static constexpr int32_t MaxFractBits = 31;
};

template <>
struct FPTypeInfo<int64_t>
{
    using MyType = int64_t;
    using PromotedType = int64_t;
    static constexpr MyType NormalizedScale = 0x7fffffffffffffff;
    static constexpr int32_t MaxFractBits = 63;
};

template <typename T, int32_t TBits>
static constexpr T GetFPScalingFactor()
{
    using Info = FPTypeInfo<T>;
    using PromotedType = typename Info::PromotedType;
    static_assert(TBits >= 0, "bits >= 0");
    return (TBits >= Info::MaxFractBits) ? Info::NormalizedScale : ((static_cast<PromotedType>(1) << TBits) - 1);
}

template <typename TFixed, int32_t TScalingShift>
struct Fixed
{
    using FixedType = TFixed;
    using PromotedType = typename FPTypeInfo<TFixed>::PromotedType;
    using MyT = Fixed<TFixed, TScalingShift>;

    FixedType mValue;
    static constexpr int32_t mScalingShift = TScalingShift;

    static constexpr FixedType mScalingValue =
        GetFPScalingFactor<TFixed, TScalingShift>(); // FPTypeInfo<TFixed>::ScalingFactor<TScalingShift>(); //(1 <<
                                                     // mScalingShift) - 1;// yes. 16-bit normalized gets scaled by
                                                     // 32767 for example.
    static constexpr FixedType mFractMask = mScalingValue;

    template <intmax_t B, class T>
    static constexpr auto const_shift(const T &a, ::std::enable_if_t<(B > 0)> * = 0)
    {
        return a << ::std::integral_constant<decltype(B), B>{};
    }
    template <intmax_t B, class T>
    static constexpr auto const_shift(const T &a, ::std::enable_if_t<(B < 0)> * = 0)
    {
        return a >> ::std::integral_constant<decltype(B), -B>{};
    }
    template <intmax_t B, class T>
    static constexpr auto const_shift(const T &a, ::std::enable_if_t<(B == 0)> * = 0)
    {
        return a;
    }

    static MyT FromFixed(FixedType v)
    {
        return MyT{v};
    }

#ifdef _DEBUG
    double mDoubleValue = 0;
    constexpr void cacheDouble()
    {
        mDoubleValue = ToDouble();
    }
#else
    constexpr void cacheDouble()
    {
    }
#endif

  private:
    explicit constexpr Fixed(FixedType v) : mValue(v)
    {
        cacheDouble();
    }

  public:
    explicit constexpr Fixed() : mValue(0)
    {
        cacheDouble();
    }

    // convert floating point to fixed.
    template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    explicit constexpr Fixed(const T &v) : mValue(static_cast<FixedType>((double)v * mScalingValue))
    { // just use double precision to ensure enough headroom for the scaling
        cacheDouble();
    }

    // convert a fixed type to another, which is really just a shift. but which direction, and before or after casting
    // is important.
    template <typename TOtherFixed, int32_t TOtherScalingShift>
    explicit constexpr Fixed(const Fixed<TOtherFixed, TOtherScalingShift> &v)
        : mValue(static_cast<FixedType>(const_shift<mScalingShift - TOtherScalingShift>(v.mValue)))
    {
        cacheDouble();
    }

    template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    MyT &operator=(double v)
    {
        mValue = static_cast<FixedType>(v * mScalingValue);
        cacheDouble();
        return *this;
    }

    template <typename TOtherFixed, int32_t TOtherScalingShift>
    MyT &operator=(const Fixed<TOtherFixed, TOtherScalingShift> &v)
    {
        mValue = static_cast<FixedType>(const_shift<mScalingShift - TOtherScalingShift>(v.mValue));
        cacheDouble();

        return *this;
    }

    // if you're already using a promoted type, here.
    template <typename TotherULT>
    MyT NonPromotingDiv(const Fixed<TotherULT, mScalingShift> &r) const
    {
        return MyT{(mValue << mScalingShift) / r.mValue};
    }

    template <typename TOtherFixed, int32_t TOtherScalingShift>
    Fixed<TOtherFixed, TOtherScalingShift> ToFixed() const
    {
        return Fixed<TOtherFixed, TOtherScalingShift>{*this};
    }

    MyT &operator*=(const MyT &r)
    {
        mValue = ((PromotedType)mValue * r.mValue) >> mScalingShift;
        cacheDouble();

        return *this;
    }

    Fixed<PromotedType, mScalingShift * 2> operator*(const MyT &r)
    {
        return Fixed<PromotedType, mScalingShift * 2>::FromFixed(((PromotedType)mValue * r.mValue));
    }

    // use with caution of course. lets compiler do temporary promotion if it's free, but anyway result is original
    // type.
    template <typename TOtherType = PromotedType>
    MyT NonPromotingMultiply(const Fixed<TOtherType, TScalingShift> &r) const
    {
        return MyT{static_cast<FixedType>((mValue * r.mValue) >> mScalingShift)};
    }

    MyT &operator/=(const MyT &r)
    {
        mValue = ((PromotedType)mValue << mScalingShift) / r.mValue;
        cacheDouble();

        return *this;
    }

    MyT operator/(const MyT &r)
    {
        return MyT{((PromotedType)mValue << mScalingShift) / r.mValue};
    }

    Fixed<PromotedType, TScalingShift> PromotingDiv(const MyT &r) const
    {
        return Fixed<PromotedType, TScalingShift>::FromFixed((static_cast<PromotedType>(mValue) << mScalingShift) /
                                                             r.mValue);
    }

    template <typename TPromotedType = PromotedType>
    Fixed<TPromotedType, TScalingShift> PromotingDiv(const MyT &r) const
    {
        return Fixed<TPromotedType, TScalingShift>::FromFixed((static_cast<TPromotedType>(mValue) << mScalingShift) /
                                                              r.mValue);
    }

    MyT &operator+=(const MyT &r)
    {
        mValue = mValue + r.mValue;
        cacheDouble();

        return *this;
    }

    MyT operator+(const MyT &r)
    {
        return MyT{mValue + r.mValue};
    }

    MyT &operator-=(const MyT &r)
    {
        mValue = mValue - r.mValue;
        cacheDouble();

        return *this;
    }

    MyT operator-(const MyT &r)
    {
        return MyT{mValue - r.mValue};
    }

    // this is actually sorta not great because we make signed before the subtract.
    template <typename TOther>
    Fixed<typename std::make_signed<FixedType>::type, TScalingShift> SignedSubtract(
        const Fixed<TOther, TScalingShift> &rhs) const
    {
        using SignedType = typename std::make_signed<FixedType>::type;
        using Tret = Fixed<SignedType, TScalingShift>;
        return Tret::FromFixed(static_cast<SignedType>(mValue) - rhs.mValue);
    }

    MyT Fract() const
    {
        return MyT{mValue & mFractMask};
    }
    MyT Floor() const
    {
        return MyT{mValue & ~mFractMask};
    }

    // do not attempt to bitshift signed types.
    // template <std::enable_if_t<std::is_unsigned<FixedType>::value, int> = 0>
    // constexpr MyT ShiftLeft(uint8_t amt) const
    // {
    //     return MyT{mValue << amt};
    // }

    // template <std::enable_if_t<std::is_unsigned<FixedType>::value, int> = 0>
    // constexpr MyT ShiftRight(uint8_t amt) const
    // {
    //     return MyT{mValue >> amt};
    // }

    constexpr MyT Abs() const
    {
        return MyT{mValue < 0 ? -mValue : mValue};
    }

    // operator %() / https://blog.mbedded.ninja/programming/general/fixed-point-mathematics/

    // optimized lerp routines would be a possibility

    constexpr double ToDouble() const
    {
        return (double)mValue / mScalingValue;
    }

    float ToFloat() const
    {
        return (float)mValue / mScalingValue;
    }

    bool operator<(const MyT &rhs) const
    {
        return mValue < rhs.mValue;
    }

    bool operator>(const MyT &rhs) const
    {
        return mValue > rhs.mValue;
    }

    bool operator<=(const MyT &rhs) const
    {
        return mValue <= rhs.mValue;
    }

    bool operator>=(const MyT &rhs) const
    {
        return mValue >= rhs.mValue;
    }
};

// Normalized 0-1 fixed point types
using Q15 = Fixed<int16_t, 15>; // 16-bit audio type
using Q16 = Fixed<uint16_t, 16>;
using Q32 = Fixed<uint32_t, 32>;
using Q31 = Fixed<int32_t, 31>;

float poly_blepF(float t, float dt) // t is distance from transition center, positive.
{
    // 0 <= t < 1
    if (t < dt)
    {
        t /= dt;
        // 2 * (t - t^2/2 - 0.5)
        return t + t - t * t - 1;
    }

    if (t > 1 - dt)
    {
        t = (t - 1) / dt;
        // 2 * (t^2/2 + t + 0.5)
        return t * t + t + t + 1;
    }

    // 0 otherwise
    return 0;
}

Fixed<int32_t, 16> poly_blepQ32(Q32 t, Q32 dt) // t is distance from transition center, positive.
{
    using RetType = Fixed<int32_t, 16>;
    //  0 <= t < 1
    if (t < dt)
    {
        Q16 t16{t};
        Q16 dt16{dt};
        if (dt16.mValue == 0)
            return RetType{-1.0f};
        auto p = t16.PromotingDiv<uint32_t>(dt16); // promote, add headroom, divide.
        static constexpr Fixed<uint32_t, 16> half{0.5};
        // 2 * (t - .5*t*t - 0.5f)
        auto q = p.NonPromotingMultiply(half);
        auto s = Fixed<int32_t, 16>{q}.NonPromotingMultiply(
            p); // multiply half first, to compensate for the sign bit taking away a bit of overhead.
        s = p.SignedSubtract(s);
        s = s.SignedSubtract(half);
        s = s + s;
        return s;
    }

    auto boundary = (FPTypeInfo<uint32_t>::NormalizedScale - dt.mValue);
    if (t.mValue > boundary)
    {
        // Q16 t16{ t };
        Q16 dt16{dt};
        if (dt16.mValue == 0)
            return RetType{-1.0f};
        Fixed<int32_t, 16> p{t};
        static constexpr decltype(p) one{1.0f};
        static constexpr decltype(p) half{0.5f};
        p = p - one;
        p = p.NonPromotingDiv(dt16);
        auto q = p + p.NonPromotingMultiply(half).NonPromotingMultiply(p) + half;
        q = q + q;
        return q;
    }

    // 0 otherwise
    return RetType{0.0f};
}

float poly_blampF(float t, float dt) // t is distance from transition
{
    // 0 <= t < 1
    if (t < dt)
    {
        //    t = t / dt - 1;
        //-1 / 3 * t * t * t;
        t = t / dt - 1;
        return -1.0f / 3 * t * t * t;
    }
    if (t > (1 - dt))
    {
        t = (t - 1) / dt + 1;
        // t = (t - 1) / dt + 1;
        return 1.0f / 3 * t * t * t;
        // 1 / 3 * t * t * t;
    }
    return 0;
};

Fixed<int32_t, 16> poly_blampQ32(Q32 t, Q32 dt)
{
    using RetType = Fixed<int32_t, 16>;
    // 0 <= t < 1
    if (t < dt)
    {
        Fixed<uint32_t, 16> t16{t};
        Fixed<uint32_t, 16> dt16{dt};
        if (dt16.mValue == 0)
            return RetType{1.0f / 3};
        //    t = t / dt - 1;
        auto a = t16.NonPromotingDiv(dt16);
        Fixed<int32_t, 16> a2{a};
        static constexpr decltype(a2) one{1.0f};
        static constexpr decltype(a2) half{0.5f};
        static constexpr decltype(a2) negativeOneThird{-1.0f / 3};
        a2 -= one;

        //-1 / 3 * t * t * t;
        // t = t / dt - 1;
        auto b = negativeOneThird.NonPromotingMultiply(a2);
        b = b.NonPromotingMultiply(a2);
        b = b.NonPromotingMultiply(a2);
        return RetType{b};
    }
    auto boundary = (FPTypeInfo<uint32_t>::NormalizedScale - dt.mValue);
    if (t.mValue > boundary)
    {
        Fixed<int32_t, 16> t16{t};
        Fixed<int32_t, 16> dt16{dt};
        if (dt16.mValue == 0)
            return RetType{-1.0f};
        static constexpr decltype(t16) one{1.0f};
        t16 -= one;
        t16 = t16.NonPromotingDiv(dt16);
        t16 += one;
        // t = (t - 1) / dt + 1;

        static constexpr decltype(t16) oneThird{1.0f / 3};
        auto a = oneThird.NonPromotingMultiply(t16);
        a = a.NonPromotingMultiply(t16);
        a = a.NonPromotingMultiply(t16);
        return RetType{a};
        // return (1.0f / 3) * t * t * t;
        // 1 / 3 * t * t * t;
    }
    return RetType{0.0f};
}

// float SampleToDouble(float x) {
//     return x;
// }

// float SampleToDouble(int16_t x) {
//     return Q15::FromFixed(x).ToFloat();
// }

// float SampleToDouble(int32_t x) {
//     return Q31::FromFixed(x).ToFloat();
// }

// float SampleToDouble(Q31 x) {
//     return x.ToFloat();
// }
// float SampleToDouble(Q32 x) {
//     return x.ToFloat();
// }

// float threshold = 0.0002f;
// float maxDelta = 0;
// int testCount = 0;
// int passCount = 0;

// template<typename T, typename Tfn>
// float PerformTestPoint(T pt, T pdt, Tfn proc, const char * prefix)
// {
//     auto ret = proc(pt, pdt);
//     float retD = SampleToDouble(ret);
//     float t = SampleToDouble(pt);
//     float dt = SampleToDouble(pdt);
//     //std::cout << prefix << "(" << t << ", " << dt << ") = " << retD << std::endl;
//     return retD;
// }

// void PerformTest(float t, float dt)
// {
//     //std::cout << "performing test: t=" << t << ", dt=" << dt << std::endl;
//     t -= floorf(t);
//     dt -= floorf(dt);
//     //if (fabs(t - .2) < 0.01 && fabs(dt - 1) < 0.01) {
//     //    int a = 0;
//     //}

//     float r1 = PerformTestPoint(t, dt, poly_blepF, "float");
//     float r2 = PerformTestPoint(Q32{ t }, Q32{ dt }, poly_blepQ32, "fixed");
//     float delta = fabs(r1 - r2);
//     if (delta < threshold) {
//         passCount++;
//         //std::cout << "   pass: t=" << t << ", dt=" << dt << " float=" << r1 << ", fixed=" << r2 << " [d=" << delta
//         << "]" << std::endl;
//     }
//     else {
//         std::cout << std::fixed << std::setprecision(4) << " FAIL: t=" << t << ", dt=" << dt << " float=" << r1 << ",
//         fixed=" << r2 << " [d=" << delta << "]" << std::endl;
//     }
//     testCount++;
//     maxDelta = std::max(maxDelta, delta);
// }

// void DoTests() {
//     maxDelta = 0;
//     testCount = 0;
//     passCount = 0;
//     for (float i = 0; i <= 1.001f; i += 0.01f)
//     {
//         for (float dt = 0.1f; dt <= 1.001f; dt += 0.01f) {
//             PerformTest(i, dt);
//         }
//     }

//     std::cout << "maxDelta: " << maxDelta << std::endl;
//     std::cout << "testCount: " << testCount << std::endl;
//     std::cout << "passCount: " << passCount << std::endl;

// }

// void PerformTestBlamp(float t, float dt)
// {
//     t -= floorf(t);
//     dt -= floorf(dt);

//     float r1 = PerformTestPoint(t, dt, poly_blampF, "blamp_float");
//     float r2 = PerformTestPoint(Q32{ t }, Q32{ dt }, poly_blampQ32, "blamp_fixed");
//     float delta = fabs(r1 - r2);
//     if (delta < threshold) {
//         passCount++;
//         //std::cout << "   pass: t=" << t << ", dt=" << dt << " float=" << r1 << ", fixed=" << r2 << " [d=" << delta
//         << "]" << std::endl;
//     }
//     else {
//         std::cout << std::fixed << std::setprecision(4) << " FAIL: t=" << t << ", dt=" << dt << " float=" << r1 << ",
//         fixed=" << r2 << " [d=" << delta << "]" << std::endl;
//     }
//     testCount++;
//     maxDelta = std::max(maxDelta, delta);
// }

// void DoTests_Blamp() {
//     maxDelta = 0;
//     testCount = 0;
//     passCount = 0;
//     for (float i = 0; i <= 1.001f; i += 0.01f)
//     {
//         for (float dt = 0.1f; dt <= 1.001f; dt += 0.01f) {
//             PerformTestBlamp(i, dt);
//         }
//     }

//     std::cout << "maxDelta: " << maxDelta << std::endl;
//     std::cout << "testCount: " << testCount << std::endl;
//     std::cout << "passCount: " << passCount << std::endl;

// }

// uint32_t* gLoc;

// float gInc = 0.002f;
// int gIter = (int)(1.0f / gInc);

// void PerfTestFloat()
// {
//     Q32 inc{ gInc };
//     Q32 t{ 0.0f };

//     //   float t = 0;
//     int check = 0;
//     for (int it = 0; it <= gIter; t += inc, ++it)
//     {
//         Q32 dt{ 0.1f };
//         for (int idt = 0; idt <= gIter; dt += inc, ++idt)
//         {
//             *gLoc = Q15{ poly_blepF(t.ToFloat(), dt.ToFloat()) }.mValue;
//             check++;
//         }
//         // Serial.println(String("check:") + check);
//     }
// }

// void PerfTestFixed()
// {
//     Q32 inc{ gInc };
//     Q32 t{ 0.0f };
//     int check = 0;
//     for (int it = 0; it <= gIter; t += inc, ++it)
//     {
//         Q32 dt{ 0.1f };
//         for (int idt = 0; idt <= gIter; dt += inc, ++idt)
//         {
//             *gLoc = poly_blepQ32(t, dt).mValue;
//             check++;
//         }
//     }

//     // Serial.println(String("check:") + check);
// }

// void PerfTestFloatBlamp()
// {
//     Q32 inc{ gInc };
//     Q32 t{ 0.0f };

//     //   float t = 0;
//     int check = 0;
//     for (int it = 0; it <= gIter; t += inc, ++it)
//     {
//         Q32 dt{ 0.1f };
//         for (int idt = 0; idt <= gIter; dt += inc, ++idt)
//         {
//             *gLoc = Q15{ poly_blampF(t.ToFloat(), dt.ToFloat()) }.mValue;
//             check++;
//         }
//         // Serial.println(String("check:") + check);
//     }
// }

// void PerfTestFixedBlamp()
// {
//     Q32 inc{ gInc };
//     Q32 t{ 0.0f };
//     int check = 0;
//     for (int it = 0; it <= gIter; t += inc, ++it)
//     {
//         Q32 dt{ 0.1f };
//         for (int idt = 0; idt <= gIter; dt += inc, ++idt)
//         {
//             *gLoc = poly_blampQ32(t, dt).mValue;
//             check++;
//         }
//     }

//     // Serial.println(String("check:") + check);
// }

// int main()
// {

//     DoTests();
//     DoTests_Blamp();
//     uint32_t val;
//     gLoc = &val;
//     static constexpr int attempts = 100;
//     {
//         int m1 = GetTickCount();
//         for (int64_t i = 0; i < attempts; ++i) {
//             PerfTestFloat();
//         }
//         int m2 = GetTickCount();
//         std::cout << "float: " << (m2 - m1) << std::endl;
//     }
//     {
//         int m1 = GetTickCount();
//         for (int64_t i = 0; i < attempts; ++i) {
//             PerfTestFixed();
//         }
//         int m2 = GetTickCount();
//         std::cout << "fixed: " << (m2 - m1) << std::endl;
//     }

//     {
//         int m1 = GetTickCount();
//         for (int64_t i = 0; i < attempts; ++i) {
//             PerfTestFloatBlamp();
//         }
//         int m2 = GetTickCount();
//         std::cout << "float: " << (m2 - m1) << std::endl;
//     }
//     {
//         int m1 = GetTickCount();
//         for (int64_t i = 0; i < attempts; ++i) {
//             PerfTestFixedBlamp();
//         }
//         int m2 = GetTickCount();
//         std::cout << "fixed: " << (m2 - m1) << std::endl;
//     }
// }
