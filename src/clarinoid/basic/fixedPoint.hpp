// todo: write tests.

// A fixed-point library specifically optimized for ARM Cortex M7 (Teensy).
// Don't want to lock ourselves into specific fixed point types like Q15, Q31, Q32.
// This library aims to be fluid about the FP type, in order to allow the compiler to fully
// optimize chained operations so intermediate datatypes are optimal, reducing the amount
// of shifting and precision loss.

// Using a fixed-point class is tricky, because for my purpose the point is to tightly control
// how operations are performed, which datatypes to select, and which precision levels are
// required.
//
// So the point of this class is to make calling code more expressive about its intent
// (kinda the goal of any abstracting class). But the caller can't completely ignore what's
// going on behind the scenes. The caller needs to be wary of avoiding certain pitfalls
// like how chaining operations "wants" to result in a never-ending increase of data type
// width.

// Some notes about performance on Teensy:
// #1 rule: 64-bit division is expensive. It's so slow that converting to double, dividing,
//          and converting back is twice as fast. Never ever do it. There's no hardware
//          instruction for it so it's basically emulated in software.
// #2 rule: 32-bit values are the fastest. 8 and 16 bit are fast but still not as fast as
//          32-bit, and because we want to reduce the amount of conversions, we should
//          gravitate towards 32-bit and nothing else.
//
// Some other notes:
// - Signed does not affect performance.
// - Inline assembly does not help. Even with the ummul instruction. It forces the compiler
//   to rearrange how it does things, set up registers in a certain way, and loses performance.
// - 64-bit ops are about >5x slower than 32 in general
// - 32-bit all ops are blazing fast; effectively free. Except division (& mod) is twice as slow.
// - 8 and 16 bit are about ~3x slower than 32.
// - conversion to and from 32-bit is not free; it's about on par with other arith ops.

// With that in mind, some comments that drive design.
// - Favor staying in 32-bit types. Don't try to stay in 8 or 16 bits; they are slower.
//   1) to avoid converting
//   2) because it's fastest by far.
// - *Strongly* avoid 64-bit types.
//
// Therefore, by default everything will be done within 32-bit types unless callers explicitly
// request maximum precision.
//
// We want to try and optimize for chained operations. Many FP libraries want to always
// convert back to the input type, for each operation. Let's not do that; ops will return the
// optimal format, to either be converted later to the type you want (no slower than baking it
// into the op), or ready to be optimally input to the next operation in the expression.
//
// Callers must specify integral bits + fractional bits.
// If callers do not specify that, then it would be impossible to retain reasonable
// precision without using 64-bit intermediate types. For staying in 32-bit types, we must
// almost always right-shift before multiplying. How much to shift depends on the *total width*
// of the value.
// Imagine multiplying A: 16.16 by 16.16, versus B: 0.16 by 0.16.
// In A, both should be shifted right 16 bits first, then multiplied to use the 32-bit space.
//       any less and you'll likely overflow.
// In B, both should not be shifted at all because we know there's enough overhead.
//       If you were to shift right 16, the value would always be 0.
// Therefore specifying intbits is absolutely necessary.
//
// Another way to see that reasoning: Because we are gravitating towards 32-bit datatypes
// always, we differ from most FP libraries. FP libraries assume datatypes saturate the
// type width. That works when you're constantly converting back to the smaller type
// (and therefore have space to use 32-bit temps). But since we stay in 32-bit land,
// even for narrow values, we need to know how much overhead is remaining to continue
// shifting optimally.

// And another way: If we DONT use intbits, we will need to use slower intermediate values,
// either needlessly converting back to int16, or needlessly converting between int64.

// Considering signedness does not affect performance, we can therefore assume
// the datatype is int32 when intbits+fractbits < 32

// It would be slightly better than just assuming intbits is the full width; it would
// immply that we convert back to an input type. We'd have to select type A or B, which
// imo would be no less confusing than supplying intbits where things are optimal.

// Note that we don't promote to 64-bit even if the operation requires it. Caller
// would need to specify to do it.
#pragma once

#include <stdint.h>
#include "Basic.hpp"
//#include <intrin.h>

// COMPILE-TIME OPTIONS:
// FP_CACHE_DOUBLE
// FP_RUNTIME_CHECKS

namespace clarinoid
{
// mod curve uses
// - 16p16 (unsigned)
// - 12p20
// mod matrix node
// - 15p16 (signed)

// arm_math defines
// - q7_t * @brief 8-bit fractional data type in 1.7 format.
// - q15_t * 16-bit fractional data type in 1.15 format.
// - q31_t 32-bit fractional data type in 1.31 format.
// - q63_t  64-bit fractional data type in 1.63 format.

static inline uint32_t CLZ(uint32_t value)
{
#ifdef CLARINOID_PLATFORM_X86
    unsigned int count = __lzcnt(value);
#else
    unsigned int count = __builtin_clz(value);
#endif
    //#else
    //    // Fallback implementation
    //    unsigned int count = 0;
    //    while ((value & (1 << (31 - count))) == 0 && count < 32)
    //    {
    //        count++;
    //    }
    //#endif
    return count;
}

const uint32_t sqrt_integer_guess_table[33] = {
    55109, 38968, 27555, 19484, 13778, 9742, 6889, 4871, 3445, 2436, 1723, 1218, 862, 609, 431, 305, 216,
    153,   108,   77,    54,    39,    27,   20,   14,   10,   7,    5,    4,    3,   2,   1,   0,
};

// Newton-Raphson integral square root. accepts a Q32, returns Q16. I would like to find a way to return a Q32 but I
// don't see it yet.
static inline uint32_t sqrt_Q32_to_Q16(uint32_t in)
{
    int i = CLZ(in);
    uint32_t n = sqrt_integer_guess_table[i];
    n = ((in / n) + n) >> 1;
    n = ((in / n) + n) >> 1;
    n = ((in / n) + n) >> 1;
    return n;
}

template <int32_t i>
struct StaticAbs
{
    static constexpr int32_t value = i < 0 ? -i : i;
};

template <int32_t i>
struct StaticValueBitsNeeded
{
    static constexpr int32_t value_allow_zero = 1 + StaticValueBitsNeeded<(StaticAbs<i>::value >> 1)>::value_allow_zero;
    static constexpr int32_t value = value_allow_zero;
};

template <>
struct StaticValueBitsNeeded<0>
{
    static constexpr int32_t value = 1;
    static constexpr int32_t value_allow_zero = 0;
};

template <typename T, std::enable_if_t<std::is_signed<T>::value, int> = 0>
static inline T FPAbs(T val)
{
    return val < 0 ? -val : val;
}

template <typename T, std::enable_if_t<!std::is_signed<T>::value, int> = 0>
static inline T FPAbs(T val)
{
    return val;
}

// probably optimizable via some intrinsics but not sure.
template <typename T>
static inline uint8_t ValueBitsNeededForValue(T i)
{
    if (i == 0)
    {
        return 1;
    }
    uint8_t bits = 0;
    T value = FPAbs(i);
    while (value > 0)
    {
        value >>= 1;
        bits++;
    }
    return bits;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct FPTypeInfo
{
};

template <>
struct FPTypeInfo<uint8_t>
{
    using MyType = uint8_t;
    using PromotedType = uint16_t;
    static constexpr bool Is64BitType = false;
    static constexpr MyType NormalizedScale = 0xff;
    static constexpr int32_t MaxFractBits = 8;
};

template <>
struct FPTypeInfo<uint16_t>
{
    using MyType = uint16_t;
    using PromotedType = uint32_t;
    static constexpr bool Is64BitType = false;
    static constexpr MyType NormalizedScale = 0xffff;
    static constexpr int32_t MaxFractBits = 16;
};

template <>
struct FPTypeInfo<uint32_t>
{
    using MyType = uint32_t;
    using PromotedType = uint64_t;
    static constexpr bool Is64BitType = false;
    static constexpr MyType NormalizedScale = 0xffffffff;
    static constexpr int32_t MaxFractBits = 32;
};

template <>
struct FPTypeInfo<uint64_t>
{
    using MyType = uint64_t;
    using PromotedType = uint64_t;
    static constexpr bool Is64BitType = true;
    static constexpr MyType NormalizedScale = 0xffffffffffffffff;
    static constexpr int32_t MaxFractBits = 64;
};

template <>
struct FPTypeInfo<int8_t>
{
    using MyType = int8_t;
    using PromotedType = int16_t;
    static constexpr bool Is64BitType = false;
    static constexpr MyType NormalizedScale = 0x7f;
    static constexpr int32_t MaxFractBits = 7;
};

template <>
struct FPTypeInfo<int16_t>
{
    using MyType = int16_t;
    using PromotedType = int32_t;
    static constexpr bool Is64BitType = false;
    static constexpr MyType NormalizedScale = 0x7fff;
    static constexpr int32_t MaxFractBits = 15;
};

template <>
struct FPTypeInfo<int32_t>
{
    using MyType = int32_t;
    using PromotedType = int64_t;
    static constexpr bool PromotedIs64Bits = true;
    static constexpr bool Is64BitType = false;
    static constexpr MyType NormalizedScale = 0x7fffffff;
    static constexpr int32_t MaxFractBits = 31;
};

template <>
struct FPTypeInfo<int64_t>
{
    using MyType = int64_t;
    using PromotedType = int64_t;
    static constexpr bool PromotedIs64Bits = true;
    static constexpr bool Is64BitType = true;
    static constexpr MyType NormalizedScale = 0x7fffffffffffffff;
    static constexpr int32_t MaxFractBits = 63;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// selects datatype that can hold the given # of bits. allows 8, 16, 32, or 64 bit types
template <uint8_t valueBits>
using FPAutoBaseType = typename std::conditional<
    (valueBits <= 7),
    int8_t,
    typename std::conditional<
        (valueBits <= 8),
        uint8_t,
        typename std::conditional<
            (valueBits <= 15),
            int16_t,
            typename std::conditional<
                (valueBits <= 16),
                uint16_t,
                typename std::conditional<
                    (valueBits <= 31),
                    int32_t,
                    typename std::conditional<(valueBits <= 32),
                                              uint32_t,
                                              typename std::conditional<(valueBits <= 63), int64_t, uint64_t>::type>::
                        type>::type>::type>::type>::type>::type;

///////////////////////////////////////////////////////////////////////////////////////////////////
// selects datatype that can hold the given # of bits. allows 32 or 64-bit types
template <uint8_t valueBits>
using FPAutoBaseType3264 = typename std::conditional<
    (valueBits <= 31),
    int32_t,
    typename std::conditional<(valueBits <= 32),
                              uint32_t,
                              typename std::conditional<(valueBits <= 63), int64_t, uint64_t>::type>::type>::type;

///////////////////////////////////////////////////////////////////////////////////////////////////
// selects datatype that can hold the given # of bits. Allows only 32 bit types
template <uint8_t valueBits>
using FPAutoBaseType32 = typename std::conditional<(valueBits <= 31), int32_t, uint32_t>::type;

///////////////////////////////////////////////////////////////////////////////////////////////////
// try to automatically select an amount of fract bits, given integral bit width.
template <uint8_t TIntBits>
struct FPAutoFractBitsAny
{
    static constexpr uint8_t value = (TIntBits <= 7)    ? (7 - TIntBits)
                                     : (TIntBits == 8)  ? (0)
                                     : (TIntBits <= 15) ? (15 - TIntBits)
                                     : (TIntBits == 16) ? (0)
                                     : (TIntBits <= 31) ? (31 - TIntBits)
                                     : (TIntBits == 32) ? (0)
                                     : (TIntBits <= 63) ? (63 - TIntBits)
                                                        : (0);
};

// we will either select 64 or 32-bit type. prefer signed if there's space.
// intbits  fractbits
// 0        31
// 16       15
// 31       0
// 32       0
// 33       30
// 63       0
// 64       0
template <uint8_t TIntBits>
struct FPAutoFractBits3264
{
    static constexpr uint8_t value = (TIntBits <= 31)   ? (31 - TIntBits)
                                     : (TIntBits == 32) ? (0)
                                     : (TIntBits <= 63) ? (63 - TIntBits)
                                                        : (0);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
template <uint8_t bits, typename T = FPAutoBaseType<bits>>
static constexpr T FillBits()
{
    // ensure remaining code has bits > 0
    if (bits == 0)
        return 0;
    T ret = 1ULL << std::max(0, (bits - 1)); // avoid compile warning about negative shifts
    ret -= 1;
    ret <<= 1;
    ret |= 1;
    return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template <int B, class T>
static constexpr auto const_shift(const T &a, ::std::enable_if_t<(B > 0)> * = 0)
{
    return a << ::std::integral_constant<decltype(B), B>{};
}
template <int B, class T>
static constexpr auto const_shift(const T &a, ::std::enable_if_t<(B < 0)> * = 0)
{
    return a >> ::std::integral_constant<decltype(B), -B>{};
}
template <int B, class T>
static constexpr auto const_shift(const T &a, ::std::enable_if_t<(B == 0)> * = 0)
{
    return a;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// SignedSaturate<n>() does a clamp(-(1<<n), (1<<n)-1)
// TODO: check that intbits <= 31
template <uint8_t intbits, typename Tinput> // template Tinput because it may be signed or unsigned and we want
                                            // conversions & full range to work seamlessly.
static CL_NODISCARD int32_t SignedSaturate(Tinput val)
{
    static_assert(intbits <= 31, "ssat does not support 32+ bits");
#ifdef CLARINOID_PLATFORM_X86
    static constexpr int32_t pos = (1UL << intbits) - 1; // for 15 bits, 32767
    static constexpr int32_t neg = -(1L << intbits);     // for 15 bits, -32768
    // int32_t xpos = pos;
    // int32_t xneg = neg;
    if (val < neg)
        return neg;
    if (val > pos)
        return pos;
    return val;
#else
    int32_t tmp;
    asm volatile("ssat %0, %1, %2" : "=r"(tmp) : "I"(intbits), "r"(val));
    return tmp;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// SignedSaturate<n>() does a clamp(-(1<<n), (1<<n)-1)
// TODO: check that intbits <= 31
template <uint8_t intbits, typename Tinput> // template Tinput because it may be signed or unsigned and we want
                                            // conversions & full range to work seamlessly.
static CL_NODISCARD uint32_t UnsignedSaturate(Tinput val)
{
    static_assert(intbits <= 32, "usat does not support >32 bits");
#ifdef CLARINOID_PLATFORM_X86
    static constexpr uint32_t pos = (1ULL << intbits) - 1;
    // auto xpos = pos;
    if (val < 0)
        return 0;
    if (val > pos)
        return pos;
    return val;
#else
    uint32_t tmp;
    asm volatile("usat %0, %1, %2" : "=r"(tmp) : "I"(intbits), "r"(val));
    return tmp;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// specify both fract bits and int bits, so we know how much overhead there is and can choose small types as needed.
template <uint8_t TIntBits,
          uint8_t TFractBits = FPAutoFractBits3264<TIntBits>::value,
          typename TBaseType = FPAutoBaseType3264<TIntBits + TFractBits>>
struct Fixed
{
  private:
#if defined(FP_CACHE_DOUBLE)
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

#if defined(FP_RUNTIME_CHECKS)
    constexpr void performRuntimeChecks()
    {
        auto ip = IntPart();
        if (ip)
        {
            auto b = ValueBitsNeededForValue(ip);
            CCASSERT(b <= kIntBits);
        }
    }
#else
    constexpr void performRuntimeChecks()
    {
    }
#endif
  public:
    using MyT = Fixed<TIntBits, TFractBits, TBaseType>;
    using BaseType = TBaseType;
    static constexpr uint8_t kFractBits = TFractBits;

    // some info about the base type.
    using TypeInfo = FPTypeInfo<BaseType>;
    static constexpr uint8_t kIntBits = TIntBits;
    static constexpr bool kIsSigned = std::is_signed<BaseType>::value;
    static constexpr bool IsUnitType =
        (kFractBits == TypeInfo::MaxFractBits); // behavior is slightly different for "unit" fp values that represent
                                                // either [0,1) or [-1,1).

    static_assert(TIntBits + TFractBits <= TypeInfo::MaxFractBits, "FP format too wide");

    const BaseType mValue;

    // value to multiply by to convert from floating-point.
    // std::min() is to avoid compiler warning about overflow.
    static constexpr BaseType mScalingValue =
        IsUnitType ? FillBits<kFractBits, BaseType>()
                   : BaseType(1ULL << std::min<uint8_t>(TypeInfo::MaxFractBits - 1, kFractBits));

    static constexpr BaseType gUnity = mScalingValue;
    static constexpr BaseType mFractMask = FillBits<kFractBits>();
    static constexpr BaseType mRemoveSignMask = FillBits<TypeInfo::MaxFractBits>();

    // 1/gUnity is one way to think about it, but it's just too tight for almost any operation and sorta not useful.
    // ~2/unity is more practical, while still preserving 1 at 0 fractbits.
    static constexpr float kEpsilonF = 1.99f / float(gUnity);
    static constexpr double kEpsilonD = 1.99 / double(gUnity);

  private:
    explicit constexpr Fixed(BaseType v) : mValue(v)
    {
        cacheDouble();
        performRuntimeChecks();
    }

  public:
    static constexpr MyT FromFixed(BaseType v)
    {
        return MyT{v};
    }

    constexpr Fixed() : mValue(0)
    {
        cacheDouble();
    }

    // construction from floating point types
    template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    constexpr Fixed(const T &v) : mValue(static_cast<BaseType>(v * mScalingValue))
    {
        cacheDouble();
        performRuntimeChecks();
    }

    // convert a fixed type to another, which is really just a shift. but which direction, and before or after casting
    // is important.

    // where our type is greater, cast first.
    // EXPLICIT is important here because truncation occurs easily.
    // consider for example Sqrt(unit_value); if explicit, then callers wouldn't realize why the result is incorrect.
    // meanwhile this ctor is invoked and truncates signbit & int part.
    template <uint8_t TOtherIntBits,
              uint8_t TOtherFractBits,
              typename TOtherBaseType,
              std::enable_if_t<(sizeof(BaseType) > sizeof(TOtherBaseType)), int> = 0>
    explicit constexpr Fixed(const Fixed<TOtherIntBits, TOtherFractBits, TOtherBaseType> &v)
        : mValue(const_shift<kFractBits - TOtherFractBits>(static_cast<BaseType>(v.mValue)))
    {
        cacheDouble();
        performRuntimeChecks();
    }

    // where the types are the same size; or theirs is bigger, cast after shift
    template <uint8_t TOtherIntBits,
              uint8_t TOtherFractBits,
              typename TOtherBaseType,
              std::enable_if_t<(sizeof(BaseType) <= sizeof(TOtherBaseType)), int> = 0>
    explicit constexpr Fixed(const Fixed<TOtherIntBits, TOtherFractBits, TOtherBaseType> &v)
        : mValue(static_cast<BaseType>(const_shift<kFractBits - TOtherFractBits>(v.mValue)))
    {
        cacheDouble();
        performRuntimeChecks();
    }

    // create from floating point values
    template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    static constexpr MyT FromValue(T val)
    {
        return MyT{val};
    }

    template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
    static constexpr MyT FromValue(T val)
    {
        return FromFixed(val << kFractBits);
    }

    constexpr double ToDouble() const
    {
        return (double)mValue / mScalingValue;
    }

    constexpr float ToFloat() const
    {
        return (float)mValue / mScalingValue;
    }

    template <uint8_t intbits, typename ResultType = Fixed<intbits, kFractBits, BaseType>>
    constexpr CL_NODISCARD ResultType SetIntBits() const
    {
        return ResultType{*this};
    }

    template <uint8_t fractbits, typename ResultType = Fixed<kIntBits, fractbits, BaseType>>
    constexpr CL_NODISCARD ResultType SetFractBits() const
    {
        return ResultType{*this};
    }

    template <typename TNewBaseType, typename ResultType = Fixed<kIntBits, kFractBits, TNewBaseType>>
    constexpr CL_NODISCARD ResultType SetBaseType() const
    {
        return ResultType{*this};
    }

    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB = FPAutoFractBits3264<TIntBitsB>::value,
              typename TBaseTypeB = FPAutoBaseType3264<TIntBitsB + TFractBitsB>>
    constexpr CL_NODISCARD Fixed<TIntBitsB, TFractBitsB, TBaseTypeB> Convert() const
    {
        return Fixed<TIntBitsB, TFractBitsB, TBaseTypeB>{*this};
    }

    template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    constexpr CL_NODISCARD T ToFloatingPointType() const
    {
        return static_cast<T>(mValue) / mScalingValue;
    }

  private:
    template <uint8_t TIntBitsB, uint8_t TFractBitsB, typename BaseTypeB>
    struct OperandTypeHelper
    {
        using BFixedType = Fixed<TIntBitsB, TFractBitsB, BaseTypeB>;

        // if existing types are 64-bit, then just go 64-bit. 64 bit intermediates & return.
        // shift right first in order to retain 32-bit width.

        // in order to plan the operation, start by determining the return type.
        // We use a 32-bit type unless one of the inputs is a 64-bit type. Keep sign bit always.
        // A       B        Result
        // int32   uint32   int32
        // int32   uint64   int64
        // uint32  int64    int64
        static constexpr bool ResultHasSignBit = std::is_signed<BaseType>::value || std::is_signed<BaseTypeB>::value;
        static constexpr bool InputHas64BitType =
            std::is_same<uint64_t, BaseType>::value || std::is_same<int64_t, BaseType>::value ||
            std::is_same<uint64_t, BaseTypeB>::value || std::is_same<int64_t, BaseTypeB>::value;
        using ResultBaseType =
            typename std::conditional<InputHas64BitType,
                                      typename std::conditional<ResultHasSignBit, int64_t, uint64_t>::type,
                                      typename std::conditional<ResultHasSignBit, int32_t, uint32_t>::type>::type;
        using ResultTypeInfo = FPTypeInfo<ResultBaseType>;

        // now find the intermediate format. most efficient mul type does:
        // r = a * b;
        // where r now has intbits & fractbits of A & B added.
        //
        // that only works when the optimal intermediate type fits within the desired result base type
        // if it doesn't, then we may be able to shift only 1 of them.
        //     r = (a >> x) * b;
        // or, r = a * (b >> x);
        //
        // that's possible when one of the input widths is naturally less than half of the result width.
        // then it can be kept and the other uses the biggest width.
        //
        // if both input widths are > available intermediate width, then both must be shifted first.
        // r = (a >> x) * (b >> (w-x));
        //
        // Let's calculate all this by starting with ideal intermediate width
        static constexpr uint8_t IdealIntermediateRequiredIntbits = (kIntBits + TIntBitsB);
        static constexpr uint8_t IdealIntermediateRequiredFractbits = kFractBits + TFractBitsB;
        static constexpr uint8_t IdealIntermediateRequiredWidth =
            IdealIntermediateRequiredIntbits + IdealIntermediateRequiredFractbits;

        static constexpr uint8_t ResultWidth = ResultTypeInfo::MaxFractBits;
        static constexpr uint8_t TotalRightShiftNeeded =
            (IdealIntermediateRequiredWidth > ResultWidth) ? (IdealIntermediateRequiredWidth - ResultWidth) : 0;
        // static constexpr bool ResultCanHoldIdealIntermediate = ResultWidth >= IdealIntermediateRequiredWidth;

        static constexpr uint8_t AWidth = kIntBits + kFractBits;
        static constexpr uint8_t BWidth = TIntBitsB + TFractBitsB;

        // time to distribute RightShiftNeeded between A and B.
        // for maximum precision and general safety, distribute it somewhat evenly between the 2.
        // A   *  B      IntermediateWidth ShiftNeeded Ashift    Bshift
        // 8.8    32.0   48                16          0         16
        // 8.8    0.16   32                0           0         0
        // 8.8    0.24   40                8           0         8
        // 0.24   16.16  32+24 = 56        24          0         8
        //
        // so first we shift the wider operand to be, at most extreme, as narrow as the other operand.
        // any remaining shift distribute evenly.

        // first shift wider operand to be down to equalize widths
        static constexpr uint8_t ARightShiftToBringToBWidth = std::max(0, (AWidth - BWidth));
        static constexpr uint8_t ARightShift1 = std::min(ARightShiftToBringToBWidth, TotalRightShiftNeeded);
        static constexpr uint8_t BRightShiftToBringToAWidth = std::max(0, (BWidth - AWidth));
        static constexpr uint8_t BRightShift1 = std::min(BRightShiftToBringToAWidth, TotalRightShiftNeeded);

        static constexpr uint8_t RightShiftRemainingAfterEqualizationShift = TotalRightShiftNeeded - ARightShift1;

        // now distribute evenly.
        static constexpr uint8_t RightShiftForNarrowerOperand = RightShiftRemainingAfterEqualizationShift / 2;
        static constexpr uint8_t RightShiftForWiderOperand =
            RightShiftRemainingAfterEqualizationShift - RightShiftForNarrowerOperand;

        static constexpr uint8_t AWidthAfterShift1 = AWidth - ARightShift1;
        static constexpr uint8_t BWidthAfterShift1 = BWidth - BRightShift1;

        static constexpr uint8_t ARightShift2 =
            AWidthAfterShift1 > BWidthAfterShift1 ? RightShiftForWiderOperand : RightShiftForNarrowerOperand;
        static constexpr uint8_t BRightShift2 =
            AWidthAfterShift1 > BWidthAfterShift1 ? RightShiftForNarrowerOperand : RightShiftForWiderOperand;

        static constexpr uint8_t ARightShiftBeforeMul = ARightShift1 + ARightShift2;
        static constexpr uint8_t BRightShiftBeforeMul = BRightShift1 + BRightShift2;

        // and now calculate the FP format of the intermediate (result) type. it's simple: right-shifting steals
        // from fractbits, then intbits, for both operands. Result type is them added together.
        static constexpr uint8_t AFractBitsShiftedOut = std::min(kFractBits, ARightShiftBeforeMul);
        static constexpr uint8_t AShiftedFractBits = kFractBits - AFractBitsShiftedOut;
        static constexpr uint8_t ARightShiftRemaining = ARightShiftBeforeMul - AFractBitsShiftedOut;
        static constexpr uint8_t AShiftedIntBits = kIntBits - ARightShiftRemaining;

        static constexpr uint8_t BFractBitsShiftedOut = std::min(TFractBitsB, BRightShiftBeforeMul);
        static constexpr uint8_t BShiftedFractBits = TFractBitsB - BFractBitsShiftedOut;
        static constexpr uint8_t BRightShiftRemaining = BRightShiftBeforeMul - BFractBitsShiftedOut;
        static constexpr uint8_t BShiftedIntBits = TIntBitsB - BRightShiftRemaining;

        static constexpr uint8_t ResultFractBits = AShiftedFractBits + BShiftedFractBits;
        static constexpr uint8_t ResultIntBits = AShiftedIntBits + BShiftedIntBits;

        using ResultType = Fixed<ResultIntBits, ResultFractBits, ResultBaseType>;
    };

  public:
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = OperandTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr CL_NODISCARD ResultType Multiply(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        typename TypeHelper::ResultBaseType ap = mValue;
        typename TypeHelper::ResultBaseType bp = b.mValue;

        // auto a_s = TypeHelper::ARightShiftBeforeMul;
        // auto b_s = TypeHelper::BRightShiftBeforeMul;
        // auto a_sfb = TypeHelper::AShiftedFractBits;
        // auto a_sib = TypeHelper::AShiftedIntBits;
        // auto b_sfb = TypeHelper::BShiftedFractBits;
        // auto b_sib = TypeHelper::BShiftedIntBits;

        ap >>= TypeHelper::ARightShiftBeforeMul;
        bp >>= TypeHelper::BRightShiftBeforeMul;

        auto intermediate = ap * bp;

        return ResultType::FromFixed(intermediate);
    }

  private:
    template <uint8_t TResultIntBits, uint8_t TIntBitsB, uint8_t TFractBitsB, typename BaseTypeB>
    struct DivideOperandTypeHelper
    {
        using BFixedType = Fixed<TIntBitsB, TFractBitsB, BaseTypeB>;

        static constexpr bool ResultHasSignBit = std::is_signed<BaseType>::value || std::is_signed<BaseTypeB>::value;
        static constexpr bool InputHas64BitType =
            std::is_same<uint64_t, BaseType>::value || std::is_same<int64_t, BaseType>::value ||
            std::is_same<uint64_t, BaseTypeB>::value || std::is_same<int64_t, BaseTypeB>::value;
        using ResultBaseType =
            typename std::conditional<InputHas64BitType,
                                      typename std::conditional<ResultHasSignBit, int64_t, uint64_t>::type,
                                      typename std::conditional<ResultHasSignBit, int32_t, uint32_t>::type>::type;
        using ResultTypeInfo = FPTypeInfo<ResultBaseType>;

        static constexpr uint8_t ResultFractBits = ResultTypeInfo::MaxFractBits - TResultIntBits;
        using ResultType = Fixed<TResultIntBits, ResultFractBits, ResultBaseType>;
    };

  public:
    template <uint8_t TResultIntBits, // caller must specify.
              uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DivideOperandTypeHelper<TResultIntBits, TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr CL_NODISCARD ResultType DivideSlow(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        // operands are not commutable and the result has a greater range (whereas mul is 0-bitwidth*2, divide can go to
        // infinity). Even if we take a similar approach to multiplication (deducing optimal shift amounts by looking at
        // intbits / fractbits), it won't help because the result can exceed the resulting limits (think 1 / 0.0001)

        // Using int32 for this is very unlikely to have enough overhead.
        // And using int64 is way slower than round-trip converting to float.

        // THEREFORE, always use floating point round trip, and avoid division at all costs.
        // Like, we won't support an operator for it.

        // Q: "but if you're using a float temporary, why not just use float for the whole expression; why bother with
        //     FP at all?"
        // A: Because callers should avoid divisions completely, let's assume it's the least-run operation in
        //    the expression. FP is much faster than float; so as long as ~2 additional operations are performed
        //    in the expression, there's still a net savings.
        //    but yes, doing MULTIPLE divisions in a signle expression would do multiple roundtrips to float which
        //    maybe in theory we could find a way to automatically optimize but not today. Just avoid division.
        float ta = this->ToFloat();
        float tb = b.ToFloat();
        auto intermediate = ta / tb;
        return ResultType{intermediate};
    }

    template <uint8_t TResultIntBits,
              typename TypeHelper = DivideOperandTypeHelper<TResultIntBits, kIntBits, kFractBits, BaseType>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr CL_NODISCARD ResultType ReciprocalSlow() const
    {
        MyT One{mScalingValue};
        return One.DivideSlow<TResultIntBits>(*this);
    }

    // modulo
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename ResultType = Fixed<kIntBits, (TypeInfo::MaxFractBits - kIntBits), BaseType>>
    constexpr CL_NODISCARD ResultType Modulo(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        // this is simpler than divide, because the resulting result range is always less than the input range.
        // for this to work, they must be the same fractbits. so we need to find a unified format that fills the
        // datatype, retains sign, unifies fractbits.
        // A must not reduce intbits.
        // A     B        intermediate
        // 8.3   0.32     8.24
        // 32.0  0.32     32.0

        // that can be summarized to say:
        // 1. fully saturate A's type, preserving intbits
        // 2. convert B to the intermediate type.
        ResultType ta{*this};
        ResultType tb{b};
        if (tb.mValue == 0)
            return ResultType::FromFixed(0);

        auto x = ta.mValue;
        auto N = tb.mValue;
        auto intermediate = x % N;

        // Behavior of negatives is controversial. Some other algos to consider...
        // https://stackoverflow.com/a/4003293/402169
        // auto intermediate = (x < 0) ? (x % N + N) : (x % N);
        //
        // but the simplest motivation to use built-in C++ operator % is that it enables
        // simple implementation of operator %().

        return ResultType::FromFixed(intermediate);
    }

    constexpr CL_NODISCARD MyT Floor() const
    {
        auto x = mValue & ~mFractMask;
        return MyT::FromFixed(x);
    }

    constexpr CL_NODISCARD MyT Ceil() const
    {
        BaseType t = mValue & ~mFractMask;
        t += gUnity;
        return MyT{t};
    }

    template <typename T = TBaseType, std::enable_if_t<std::is_signed<T>::value, int> = 0>
    constexpr CL_NODISCARD MyT Abs() const
    {
        return FromFixed(mValue < 0 ? -mValue : mValue);
    }

    template <typename T = TBaseType, std::enable_if_t<!std::is_signed<T>::value, int> = 0>
    constexpr CL_NODISCARD MyT Abs() const
    {
        return *this;
    }

    template <typename T = TBaseType, std::enable_if_t<std::is_signed<T>::value, int> = 0>
    constexpr CL_NODISCARD MyT Negate() const
    {
        return FromFixed(-mValue);
    }

  private:
    template <uint8_t TIntBitsB, uint8_t TFractBitsB, typename BaseTypeB>
    struct IntegerDivideHelper
    {
        using TypeB = Fixed<TIntBitsB, TFractBitsB, BaseTypeB>;
        using TypeInfoB = FPTypeInfo<BaseTypeB>;
        static_assert(TFractBitsB <= kFractBits,
                      "Dividing by a more precise number results in a value that's not representable by Fixed<>. Find "
                      "another way.");
        static constexpr uint8_t ResultFractBits = (TFractBitsB >= kFractBits) ? 0 : kFractBits - TFractBitsB;
        // if either operand is 64-bit, stay in 64-bit. otherwise 32-bit.
        static constexpr bool ResultIsSigned = kIsSigned || TypeB::kIsSigned;
        static constexpr bool ResultIs64Bit = TypeInfo::Is64BitType || TypeInfoB::Is64BitType;
        using ResultBaseType =
            typename std::conditional<ResultIs64Bit,
                                      typename std::conditional<ResultIsSigned, int64_t, uint64_t>::type,
                                      typename std::conditional<ResultIsSigned, int32_t, uint32_t>::type>::type;
        using ResultTypeInfo = FPTypeInfo<ResultBaseType>;
        static constexpr uint8_t ResultValueWidth = ResultTypeInfo::MaxFractBits;
        static constexpr uint8_t ResultIntBits =
            std::min<uint8_t>(ResultValueWidth - std::min<uint8_t>(ResultFractBits, ResultValueWidth),
                              std::max<uint8_t>(1, TIntBitsB) + std::max<uint8_t>(1, kIntBits));
        using ResultType = Fixed<ResultIntBits, ResultFractBits, ResultBaseType>;
    };

  public:
    // Does a pure integer division with the operand. One complexity of division is that the result
    // range is hard to predict. Therefore the result int bits can easily be inflated too much.
    // This always works on 32-bit value for optimization, and return value is based off the raw result
    // of the C++ integer division op. Whether you find this function useful is up to you, but at least
    // it provides a structured and accurate division routine for 32-bit.
    //
    // The result tends to have a greatly-reduced precision, and sometimes fractbits can even theoretically
    // become negative. Well, it's up to the caller to test their code then =) But this class will not support
    // negative bits, so basically it's UB.
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename ResultTypeHelper = IntegerDivideHelper<TIntBitsB, TFractBitsB, BaseTypeB>>
    constexpr CL_NODISCARD typename ResultTypeHelper::ResultType DivideFast(
        const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        using ResultType = typename ResultTypeHelper::ResultType;
        return ResultType::FromFixed(mValue / b.mValue);
    }

  private:
    // when making signed, you may need to give up 1 bit of precision.
    // the most vexing scenario would be taking a 32.0 and making it signed. One solution
    // would be to promote to int64, however we want to avoid this promotion at all costs.
    // so we will consider it an ERROR to do such a conversion.
    struct UnsignedToSignedHelper
    {
        // uint64 => int64.
        // anything else => int32
        using SignedBaseType =
            typename std::conditional<std::is_same<uint64_t, typename std::make_unsigned<BaseType>::type>::value,
                                      int64_t,
                                      int32_t>::type;
        using SignedTypeInfo = FPTypeInfo<SignedBaseType>;

        // will it fit?
        static constexpr uint8_t kOriginalWidth = kIntBits + kFractBits;
        static constexpr uint8_t kNewMaxWidth = SignedTypeInfo::MaxFractBits;
        static constexpr uint8_t kShiftNeeded =
            std::max(0, kOriginalWidth - kNewMaxWidth); // this will be either 0 or 1
        static constexpr uint8_t kNewFractBits = std::max(0, kFractBits - kShiftNeeded);
        static_assert(kIntBits + kNewFractBits <= kNewMaxWidth, "Unable to add sign bit; no space left in type.");

        using SignedType = Fixed<kIntBits, kNewFractBits, SignedBaseType>;
    };

    using CorrespondingSignedType = typename std::
        conditional<std::is_signed<BaseType>::value, MyT, typename UnsignedToSignedHelper::SignedType>::type;

  public:
    constexpr CL_NODISCARD CorrespondingSignedType MakeSigned() const
    {
        return *this;
    }

    // for unsigned, we should make it signed.
    template <typename T = TBaseType, std::enable_if_t<!std::is_signed<T>::value, int> = 0>
    constexpr CL_NODISCARD CorrespondingSignedType Negate() const
    {
        return MakeSigned().Negate();
    }

  private:
    // when subtracting (or adding a signed value), you may need to give up 1 bit of precision to add a sign bit.
    // when negating explicitly, that's an error (see the static_assert).
    // but when adding/subtracting, sometimes the caller knows that the result will remain unsigned.
    // therefore, when there's not enough space in the type for the sign bit, just leave it out.
    // that makes 32.0 - 32.0 behave in a way callers will expect, for example.
    //
    // the operands also must be brought into a unified format.
    //
    // The return base type will be the larger bitness of A or B.
    // 8.8 minus 6.10 => 8.10, signed. larger of 2, unify fractbits, room for sign.
    // 31.0 minus 0.31 => wants 31.31, preserve intbits and truncate fract with sign => 31.0 int32.
    // 32.0<uint32> minus signed 8.8 would be uint32 (no promotion to 64; intbits must be preserved and truncates)
    // 8.8 minus 32.0<uint32> => wants 32.8; no promotion to 64 bit so 32.0 uint32.
    // 31.0<int32> minus 32.0 => wants 32.0; preserve intbits => 32.0 uint32
    // 31.0<int32> minus 0.32 => wants 31.32; preserve intbits and truncate fractbits => 31.0 int32.
    // 31.0<uint32> minus 0.32 <int64> => wants 31.32 into 64-bit type => 31.32 <int64>
    // 32.0<uint32> minus 0.32 <int64> => wants 32.32 into 64-bit type => truncate fractbits to 32.31 <int64>
    // 32.0<uint32> minus 32.16 <uint64> => wants 32.16 into 64-bit result type => 32.16 int64.
    //
    template <uint8_t TIntBitsB, uint8_t TFractBitsB, typename BaseTypeB>
    struct DifferenceTypeHelper
    {
        using FixedB = Fixed<TIntBitsB, TFractBitsB, BaseTypeB>;
        static constexpr uint8_t IdealIntBits = std::max(TIntBitsB, kIntBits);
        static constexpr uint8_t IdealFractBits = std::max(TFractBitsB, kFractBits);
        static constexpr uint8_t IdealWidth = IdealIntBits + IdealFractBits;
        static constexpr bool ResultAllows64Bit = FixedB::TypeInfo::Is64BitType || TypeInfo::Is64BitType;

        // it's slightly different than naive "select the datatype that can hold this width",
        // because there are cases where we want to avoid promotion to 64-bit.
        using ResultBaseType = typename std::
            conditional<ResultAllows64Bit, FPAutoBaseType3264<IdealWidth>, FPAutoBaseType32<IdealWidth>>::type;
        using ResultBaseTypeInfo = FPTypeInfo<ResultBaseType>;

        // now truncate as needed.
        static constexpr uint8_t ResultFractBits =
            std::min<uint8_t>(IdealFractBits, ResultBaseTypeInfo::MaxFractBits - IdealIntBits);

        using ResultType = Fixed<IdealIntBits, ResultFractBits, ResultBaseType>;
        using FractResultType = Fixed<0, ResultFractBits, ResultBaseType>;
    };

  public:
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr CL_NODISCARD ResultType Subtract(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        ResultType ta{*this};
        ResultType tb{b};

        return ResultType::FromFixed(ta.mValue - tb.mValue);
    }

    template <typename TypeHelper = DifferenceTypeHelper<TIntBits, TFractBits, BaseType>,
              typename ResultType = typename TypeHelper::FractResultType,
              typename T = BaseType,
              bool TIsUnitType = IsUnitType,
              std::enable_if_t<!TIsUnitType, int> = 0>
    constexpr CL_NODISCARD ResultType Fract() const
    {
        // this will always be the same base type, just different intbits specification so optimal with elision.
        return ResultType{this->Subtract(Floor())};
    }

    // For unit types, units outside of 1 are not possible therefore nothing is needed to Fract().
    // for signed, if positive, just return.
    // if negative, then it's 1+this
    template <typename T = BaseType,
              bool TIsUnitType = IsUnitType,
              std::enable_if_t<TIsUnitType && std::is_signed<T>::value, int> = 0>
    constexpr CL_NODISCARD MyT Fract() const
    {
        return FromFixed(mValue & mRemoveSignMask);
    }

    // For unit types, units outside of 1 are not possible therefore nothing is needed to Fract(). This is effectively a
    // NOP.
    template <typename T = BaseType,
              bool TIsUnitType = IsUnitType,
              std::enable_if_t<TIsUnitType && !std::is_signed<T>::value, int> = 0>
    constexpr CL_NODISCARD MyT Fract() const
    {
        return *this;
    }

    // unit types never have integral parts
    template <bool TIsUnitType = IsUnitType, std::enable_if_t<TIsUnitType, int> = 0>
    constexpr CL_NODISCARD BaseType IntPart() const
    {
        return 0;
    }

    template <bool TIsUnitType = IsUnitType, std::enable_if_t<!TIsUnitType, int> = 0>
    constexpr CL_NODISCARD BaseType IntPart() const
    {
        return mValue >> kFractBits;
    }

    // add
    // even though we could theoretically think of ways in which this is different than subtract, none
    // of them really matter. If difference is robust to signedness and unsignedness, this will be equally.
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr CL_NODISCARD ResultType Add(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        ResultType ta{*this};
        ResultType tb{b};
        return ResultType::FromFixed(ta.mValue + tb.mValue);
    }

  private:
    // shifting can just be changing fractbits. if there are no more bits remaining,
    // then the underlying value will be shifted. sign not touched.
    template <uint8_t amt>
    struct ShiftHelper
    {
        // in order to shift left only by changing FP format bits,
        // there needs to be room to shift fractbits down by amt.
        //
        // In the case where fast method is enabled, the underlying value is not to be touched; only the FP format.
        // it really works; precision is simply moved from fract to int part, and instead of introducing new zeroes,
        // we just change the FP format.
        static constexpr bool EnableFastShiftLeft = kFractBits >= amt;
        using ShiftLeftResultType = typename std::conditional<EnableFastShiftLeft,
                                                              Fixed<kIntBits + amt, kFractBits - amt, BaseType>,
                                                              MyT>::type; // otherwise, the underlying value must be
                                                                          // shifted, and the format doesn't change.

        // for shifting right, the fast method is available when there is width enough.
        //  orig         slow result    fast result =>
        //  11111111     01111111       11111111       11111111   11111111  <-- and no more available.
        //  iiifffff     iiifffff       iiffffff       ifffffff   ffffffff
        //  the difference
        static constexpr bool EnableFastShiftRight = (kFractBits + amt) <= TypeInfo::MaxFractBits;
        static constexpr uint8_t FastRightShiftedIntBits = std::max(0, kIntBits - amt);
        using ShiftRightResultType =
            typename std::conditional<EnableFastShiftRight,
                                      Fixed<FastRightShiftedIntBits, kFractBits + amt, BaseType>,
                                      MyT>::type; // otherwise, the underlying value must be
                                                  // shifted, and the format doesn't change.
    };

  public:
    // Fast method (just change intbits & fractbits and use same value)
    template <uint8_t amt,
              typename Helper = ShiftHelper<amt>,
              typename ResultType = typename Helper::ShiftLeftResultType,
              typename std::enable_if<Helper::EnableFastShiftLeft, int>::type = 0>
    constexpr CL_NODISCARD ResultType ShiftLeft() const
    {
        return ResultType::FromFixed(mValue);
    }

    // "Slow" method (underlying value must be shifted)
    template <uint8_t amt,
              typename Helper = ShiftHelper<amt>,
              typename ResultType = typename Helper::ShiftLeftResultType,
              typename std::enable_if<!Helper::EnableFastShiftLeft, int>::type = 0>
    constexpr CL_NODISCARD ResultType ShiftLeft() const
    {
        return ResultType::FromFixed(mValue << amt);
    }

    // Fast method (just change intbits & fractbits and use same value)
    template <uint8_t amt,
              typename Helper = ShiftHelper<amt>,
              typename ResultType = typename Helper::ShiftRightResultType,
              typename std::enable_if<Helper::EnableFastShiftRight, int>::type = 0>
    constexpr CL_NODISCARD ResultType ShiftRight() const
    {
        return ResultType::FromFixed(mValue);
    }

    // "Slow" method (underlying value must be shifted)
    template <uint8_t amt,
              typename Helper = ShiftHelper<amt>,
              typename ResultType = typename Helper::ShiftRightResultType,
              typename std::enable_if<!Helper::EnableFastShiftRight, int>::type = 0>
    constexpr CL_NODISCARD ResultType ShiftRight() const
    {
        return ResultType::FromFixed(mValue >> amt);
    }

    // comparison
    // in general we use the difference_type because it has the right properties:
    // - unifies types
    // - preserves signs
    // - favors preserving intbits
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr CL_NODISCARD bool IsGreaterThan(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        ResultType ta{*this};
        ResultType tb{b};
        return ta.mValue > tb.mValue;
    }

    template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    constexpr CL_NODISCARD bool IsGreaterThan(T b) const
    {
        // float mul -> conv & int compare
        // conv -> float mul & float compare
        // you could argue that comparing via int is more optimal, but it would be less accurate at boundaries where
        // rounding makes things less clear.
        return ToFloatingPointType<T>() > b;
    }

    // if you compare against an integral value, much easier. we shift off the fract bits and compare.
    template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
    constexpr CL_NODISCARD bool IsGreaterThan(T b) const
    {
        return IntPart() > b;
    }

    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr CL_NODISCARD bool IsGreaterThanOrEquals(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        ResultType ta{*this};
        ResultType tb{b};
        return ta.mValue >= tb.mValue;
    }

    template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    constexpr CL_NODISCARD bool IsGreaterThanOrEquals(T b) const
    {
        return ToFloatingPointType<T>() >= b;
    }

    template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
    constexpr CL_NODISCARD bool IsGreaterThanOrEquals(T b) const
    {
        return IntPart() >= b;
    }

    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr CL_NODISCARD bool IsLessThan(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        ResultType ta{*this};
        ResultType tb{b};
        return ta.mValue < tb.mValue;
    }

    template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    constexpr CL_NODISCARD bool IsLessThan(T b) const
    {
        return ToFloatingPointType<T>() < b;
    }

    template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
    constexpr CL_NODISCARD bool IsLessThan(T b) const
    {
        return IntPart() < b;
    }

    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr CL_NODISCARD bool IsLessThanOrEquals(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        ResultType ta{*this};
        ResultType tb{b};
        return ta.mValue <= tb.mValue;
    }

    template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    constexpr CL_NODISCARD bool IsLessThanOrEquals(T b) const
    {
        return ToFloatingPointType<T>() <= b;
    }

    template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
    constexpr CL_NODISCARD bool IsLessThanOrEquals(T b) const
    {
        return IntPart() <= b;
    }

    // when not specifying epsilon, use built-in epsilon
    // fixed operand
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr CL_NODISCARD bool IsApproximatelyEqualTo(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        ResultType ta{*this};
        ResultType tb{b};
        return ta.mValue == tb.mValue;
    }

    // when not specifying epsilon, use built-in epsilon
    // float operand
    template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    constexpr CL_NODISCARD bool IsApproximatelyEqualTo(T b) const
    {
        auto ta = ToFloatingPointType<T>();
        auto tb = ta - b;            // sub
        auto tc = tb < 0 ? -tb : tb; // abs (tc = delta)
        auto r = tc < kEpsilonF;
        return r;
    }

    // when not specifying epsilon, use built-in epsilon
    // int operand
    template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
    constexpr CL_NODISCARD bool IsApproximatelyEqualTo(T b) const
    {
        // having any fractional part means it's not equal to an integer.
        // right-shift one to account for epsilon.
        auto tf = (mValue & mFractMask) >> 1;
        if (tf)
        {
            return false;
        }
        auto ti = this->IntPart();
        return ti == b;
    }

    // specifying epsilon as fixed point
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              uint8_t TIntBitsE,
              uint8_t TFractBitsE,
              typename BaseTypeE,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr CL_NODISCARD bool IsApproximatelyEqualTo(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b,
                                                        const Fixed<TIntBitsE, TFractBitsE, BaseTypeE> &epsilon) const
    {
        auto t1 = this->Subtract(b);
        auto t2 = t1.Abs();
        return t2 <= epsilon;
    }

    // specifying epsilon as floating point.
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename T,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType,
              std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    constexpr CL_NODISCARD bool IsApproximatelyEqualTo(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b,
                                                        T epsilon) const
    {
        auto t1 = this->Subtract(b);
        auto t2 = t1.Abs();
        return t2 <= epsilon;
    }

    // there's no "approximately equal to integer" because the intention would be to optimize, but because of rounding
    // it's not clear what caller wants. if a need arises, add the requisite function.

    // clamp
    // intbits can be reduced to the min of operands.
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              uint8_t TIntBitsC,
              uint8_t TFractBitsC,
              typename BaseTypeC,
              uint8_t TResultIntBits = std::min(kIntBits, std::min(TIntBitsB, TIntBitsC)),
              typename ResultType = Fixed<TResultIntBits, kFractBits, BaseType>
              >
    constexpr CL_NODISCARD ResultType Clamp(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &lower,
                                             const Fixed<TIntBitsC, TFractBitsC, BaseTypeC> &upper) const
    {
        if (IsGreaterThan(upper))
        {
            return ResultType{upper};
        }
        if (IsLessThan(lower))
        {
            return ResultType{lower};
        }
        return ResultType{*this};
    }

    // converts this value to another FP format, and saturates when narrowing.
    // yea maybe it seems a bit out of place, but i don't find a better way to neatly
    // access the `ssat` and `usat` instructions.
    // TODO: specialize when not narrowing.
    template <uint8_t TResultIntBits,
              uint8_t TResultFractBits = FPAutoFractBitsAny<TResultIntBits>::value,
              typename TResultBaseType = FPAutoBaseType<TIntBits + TFractBits>,
              typename ResultType = Fixed<TResultIntBits, TResultFractBits, TResultBaseType>>
    constexpr CL_NODISCARD ResultType SignedSaturate() const
    {
        // first shift to dest type.
        BaseType newVal = const_shift<TResultFractBits - kFractBits>(mValue);
        return ResultType::FromFixed(::clarinoid::SignedSaturate<TResultIntBits + TResultFractBits>(newVal));
    }

    // converts this value to another FP format, and saturates when narrowing.
    // yea maybe it seems a bit out of place, but i don't find a better way to neatly
    // access the `ssat` and `usat` instructions.
    // TODO: specialize when not narrowing.
    template <uint8_t TResultIntBits,
              uint8_t TResultFractBits = FPAutoFractBitsAny<TResultIntBits>::value,
              typename TResultBaseType = FPAutoBaseType<TIntBits + TFractBits>,
              typename ResultType = Fixed<TResultIntBits, TResultFractBits, TResultBaseType>>
    constexpr CL_NODISCARD ResultType UnsignedSaturate() const
    {
        // first shift to dest type.
        BaseType newVal = const_shift<TResultFractBits - kFractBits>(mValue);
        return ResultType::FromFixed(::clarinoid::UnsignedSaturate<TResultIntBits + TResultFractBits>(newVal));
    }

    // store as int32_t to allow loading directly into register without extending.
    static constexpr int32_t AudioWaveformSine[257] = {
        0,      804,    1608,   2410,   3212,   4011,   4808,   5602,   6393,   7179,   7962,   8739,   9512,   10278,
        11039,  11793,  12539,  13279,  14010,  14732,  15446,  16151,  16846,  17530,  18204,  18868,  19519,  20159,
        20787,  21403,  22005,  22594,  23170,  23731,  24279,  24811,  25329,  25832,  26319,  26790,  27245,  27683,
        28105,  28510,  28898,  29268,  29621,  29956,  30273,  30571,  30852,  31113,  31356,  31580,  31785,  31971,
        32137,  32285,  32412,  32521,  32609,  32678,  32728,  32757,  32767,  32757,  32728,  32678,  32609,  32521,
        32412,  32285,  32137,  31971,  31785,  31580,  31356,  31113,  30852,  30571,  30273,  29956,  29621,  29268,
        28898,  28510,  28105,  27683,  27245,  26790,  26319,  25832,  25329,  24811,  24279,  23731,  23170,  22594,
        22005,  21403,  20787,  20159,  19519,  18868,  18204,  17530,  16846,  16151,  15446,  14732,  14010,  13279,
        12539,  11793,  11039,  10278,  9512,   8739,   7962,   7179,   6393,   5602,   4808,   4011,   3212,   2410,
        1608,   804,    0,      -804,   -1608,  -2410,  -3212,  -4011,  -4808,  -5602,  -6393,  -7179,  -7962,  -8739,
        -9512,  -10278, -11039, -11793, -12539, -13279, -14010, -14732, -15446, -16151, -16846, -17530, -18204, -18868,
        -19519, -20159, -20787, -21403, -22005, -22594, -23170, -23731, -24279, -24811, -25329, -25832, -26319, -26790,
        -27245, -27683, -28105, -28510, -28898, -29268, -29621, -29956, -30273, -30571, -30852, -31113, -31356, -31580,
        -31785, -31971, -32137, -32285, -32412, -32521, -32609, -32678, -32728, -32757, -32767, -32757, -32728, -32678,
        -32609, -32521, -32412, -32285, -32137, -31971, -31785, -31580, -31356, -31113, -30852, -30571, -30273, -29956,
        -29621, -29268, -28898, -28510, -28105, -27683, -27245, -26790, -26319, -25832, -25329, -24811, -24279, -23731,
        -23170, -22594, -22005, -21403, -20787, -20159, -19519, -18868, -18204, -17530, -16846, -16151, -15446, -14732,
        -14010, -13279, -12539, -11793, -11039, -10278, -9512,  -8739,  -7962,  -7179,  -6393,  -5602,  -4808,  -4011,
        -3212,  -2410,  -1608,  -804,   0};

    // calculates sin() with period 0-1 over a fixed point integral value.
    inline CL_NODISCARD Fixed<0, 23, int32_t> Sine_2pi() const
    {
        /*
        *
        * TODO: double-check that we are calculating this correctly. original teensy code is:
        * I don't understand why the 0x10000 - scale; this is a LERP and I'd expect this to be the (b-a)
        * where b may not be 0x10000. Or maybe this is the <<8 I'm doing to unify the scales.

        for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
            index = ph >> 24;
            val1 = AudioWaveformSine[index];
            val2 = AudioWaveformSine[index+1];
            scale = (ph >> 8) & 0xFFFF;
            val2 *= scale;
            val1 *= 0x10000 - scale;
            *bp++ = multiply_32x32_rshift32(val1 + val2, magnitude);
            ph += inc;
        }

        */

        auto x1 = Fract(); // normalize range
        auto x2 = Fixed<0, 16, uint32_t>{
            x1}; // convert to Q16 to ease calculations below (otherwise we're dipping into int64 territory)
        // if x2 fractbits were Q8, the mask would just be the integral fract portion. mValue & 0xff.
        // but it's a Q16 so shift out
        auto i1 = x2.mValue >> 8; // upper word
        auto a = AudioWaveformSine[i1];
        auto b = AudioWaveformSine[i1 + 1];
        // In order for the addition to work, a must be converted from Q15 to Q23.
        // (b-a)*xp is Q23 (Q15 + Q8)
        int32_t xp = x2.mValue & 0xff; // lower word = interpolate.
        auto t = (a << 8) + (b - a) * xp;
        using ResultType = Fixed<0, 23, int32_t>;
        return ResultType::FromFixed(t);
    }

    // Calculates square root of a unit value. In other words, it cannot do sqrt of anything outside of [0,1)
    // TRUNCATES any integer part.
    // TRUNCATES sign bit.
    // i considered only enabling this for unit types, but better to just give a clear name that's not a generic
    // sqrt.
    inline constexpr Fixed<0, 16, uint16_t> SqrtUnit() const
    {
        auto t = Fixed<0, 32>{*this};
        using ReturnType = Fixed<0, 16, uint16_t>;
        return ReturnType::FromFixed(sqrt_Q32_to_Q16(t.mValue));
    }

    // non-mutating operators. only don't support any division operators.
    // *
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = OperandTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr CL_NODISCARD ResultType operator*(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        return Multiply(b);
    }

    // %
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename ResultType = Fixed<kIntBits, (TypeInfo::MaxFractBits - kIntBits), BaseType>>
    constexpr CL_NODISCARD ResultType operator%(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        return Modulo(b);
    }

    // -
    template <typename T = TBaseType, std::enable_if_t<std::is_signed<T>::value, int> = 0>
    constexpr CL_NODISCARD MyT operator-() const
    {
        return Negate();
    }

    // for unsigned, we should make it signed.
    template <typename T = TBaseType, std::enable_if_t<!std::is_signed<T>::value, int> = 0>
    constexpr CL_NODISCARD CorrespondingSignedType operator-() const
    {
        return Negate();
    }

    // +
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr CL_NODISCARD ResultType operator+(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        return Add(b);
    }

    // -
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr CL_NODISCARD ResultType operator-(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        return Subtract(b);
    }

    // >
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr CL_NODISCARD bool operator>(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        return IsGreaterThan(b);
    }

    template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
    constexpr CL_NODISCARD bool operator>(const T &b) const
    {
        return IsGreaterThan(b);
    }

    // >=
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr CL_NODISCARD bool operator>=(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        return IsGreaterThanOrEquals(b);
    }

    template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
    constexpr CL_NODISCARD bool operator>=(const T &b) const
    {
        return IsGreaterThanOrEquals(b);
    }

    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr CL_NODISCARD bool operator<(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        return IsLessThan(b);
    }

    template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
    constexpr CL_NODISCARD bool operator<(const T &b) const
    {
        return IsLessThan(b);
    }

    // <=
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr CL_NODISCARD bool operator<=(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        return IsLessThanOrEquals(b);
    }

    template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
    constexpr CL_NODISCARD bool operator<=(const T &b) const
    {
        return IsLessThanOrEquals(b);
    }

    // ==
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr CL_NODISCARD bool operator==(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        return IsApproximatelyEqualTo(b);
    }
};

// t MUST be [0,1]
template <uint8_t TIntBitsA,
          uint8_t TFractBitsA,
          typename BaseTypeA,
          uint8_t TIntBitsB,
          uint8_t TFractBitsB,
          typename BaseTypeB,
          uint8_t TIntBitsC,
          uint8_t TFractBitsC,
          typename BaseTypeC,
          typename TypeHelper = typename Fixed<TIntBitsA, TFractBitsA, BaseTypeA>::
              template DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
          typename ResultType = typename TypeHelper::ResultType>
ResultType Lerp(const Fixed<TIntBitsA, TFractBitsA, BaseTypeA> &a,
                const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b,
                const Fixed<TIntBitsC, TFractBitsC, BaseTypeC> &t01)
{
    ResultType r{a + (b - a) * t01.Fract()}; // calling Fract() drops intbits to 0, keeping overhead under control.
    return r;
}

// t MUST be [0,1]
template <uint8_t TIntBitsA,
          uint8_t TFractBitsA,
          typename BaseTypeA,
          uint8_t TIntBitsB,
          uint8_t TFractBitsB,
          typename BaseTypeB,
          typename T,
          typename TypeHelper = typename Fixed<TIntBitsA, TFractBitsA, BaseTypeA>::
              template DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
          typename ResultType = typename TypeHelper::ResultType,
          typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
ResultType Lerp(const Fixed<TIntBitsA, TFractBitsA, BaseTypeA> &a,
                const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b,
                T t01)
{
    return Lerp(a, b, Fixed<0, 31>{t01});
}

// Allows for quickly converting compile-time integral values to a Fixed<> value.
// pushes integral bits to the left and fills remaining space with fractbits.
template <int32_t value, uint8_t fractBits = (31 - StaticValueBitsNeeded<value>::value)>
static inline constexpr Fixed<StaticValueBitsNeeded<value>::value, fractBits, int32_t> FixedInteger()
{
    return Fixed<StaticValueBitsNeeded<value>::value, fractBits, int32_t>::FromValue(value);
}

static inline Fixed<0, 16, uint16_t> sqrt(const Fixed<0, 32, uint32_t> &x)
{
    using ReturnType = Fixed<0, 16, uint16_t>;
    return ReturnType::FromFixed(sqrt_Q32_to_Q16(x.mValue));
}

} // namespace clarinoid
