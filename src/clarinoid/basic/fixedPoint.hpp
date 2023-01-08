// todo: runtime shift operands (& corresponding operators)
// todo: pow
// todo: sqrt
// todo: sine / trig

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
    static constexpr bool PromotedIs64Bits = false;
    static constexpr bool Is64BitType = false;
    static constexpr MyType NormalizedScale = 0xff;
    static constexpr int32_t MaxFractBits = 8;
};

template <>
struct FPTypeInfo<uint16_t>
{
    using MyType = uint16_t;
    using PromotedType = uint32_t;
    static constexpr bool PromotedIs64Bits = false;
    static constexpr bool Is64BitType = false;
    static constexpr MyType NormalizedScale = 0xffff;
    static constexpr int32_t MaxFractBits = 16;
};

template <>
struct FPTypeInfo<uint32_t>
{
    using MyType = uint32_t;
    using PromotedType = uint64_t;
    static constexpr bool PromotedIs64Bits = true;
    static constexpr bool Is64BitType = false;
    static constexpr MyType NormalizedScale = 0xffffffff;
    static constexpr int32_t MaxFractBits = 32;
};

template <>
struct FPTypeInfo<uint64_t>
{
    using MyType = uint64_t;
    using PromotedType = uint64_t;
    static constexpr bool PromotedIs64Bits = true;
    static constexpr bool Is64BitType = true;
    static constexpr MyType NormalizedScale = 0xffffffffffffffff;
    static constexpr int32_t MaxFractBits = 64;
};

template <>
struct FPTypeInfo<int8_t>
{
    using MyType = int8_t;
    using PromotedType = int16_t;
    static constexpr bool PromotedIs64Bits = false;
    static constexpr bool Is64BitType = false;
    static constexpr MyType NormalizedScale = 0x7f;
    static constexpr int32_t MaxFractBits = 7;
};

template <>
struct FPTypeInfo<int16_t>
{
    using MyType = int16_t;
    using PromotedType = int32_t;
    static constexpr bool PromotedIs64Bits = false;
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
// selects datatype that can hold the given # of bits. allows 64-bits.
template <uint8_t valueBits>
using FPAutoBaseType = typename std::conditional<
    (valueBits <= 31),
    int32_t,
    typename std::conditional<(valueBits <= 32),
                              uint32_t,
                              typename std::conditional<(valueBits <= 63), int64_t, uint64_t>::type>::type>::type;

///////////////////////////////////////////////////////////////////////////////////////////////////
// selects datatype that can hold the given # of bits. disallows 64-bits.
template <uint8_t valueBits>
using FPAutoBaseType32 = typename std::conditional<(valueBits <= 31), int32_t, uint32_t>::type;

///////////////////////////////////////////////////////////////////////////////////////////////////
// try to automatically select an amount of fract bits, given integral bit width.
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
struct FPAutoFractBits
{
    static constexpr uint8_t value = (TIntBits <= 31)   ? (31 - TIntBits)
                                     : (TIntBits == 32) ? (0)
                                     : (TIntBits <= 63) ? (63 - TIntBits)
                                                        : (0);
};

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
// specify both fract bits and int bits, so we know how much overhead there is and can choose small types as needed.
template <uint8_t TIntBits,
          uint8_t TFractBits = FPAutoFractBits<TIntBits>::value,
          typename TBaseType = FPAutoBaseType<TIntBits + TFractBits>>
struct Fixed
{
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

    using MyT = Fixed<TIntBits, TFractBits, TBaseType>;
    using BaseType = TBaseType;
    static constexpr uint8_t kFractBits = TFractBits;

    // some info about the base type.
    using TypeInfo = FPTypeInfo<BaseType>;
    static constexpr uint8_t kIntBits = TIntBits;
    static constexpr bool IsUnitType =
        (kFractBits == TypeInfo::MaxFractBits); // behavior is slightly different for "unit" fp values that represent
                                                // either [0,1) or [-1,1).

    BaseType mValue;

    // value to multiply by to convert from floating-point.
    static constexpr BaseType mScalingValue =
        IsUnitType ? (TypeInfo::NormalizedScale >> kIntBits) : (1ULL << kFractBits);

    static constexpr BaseType gUnity = mScalingValue;
    static constexpr BaseType mFractMask = (1ULL << kFractBits) - 1;

    static constexpr float kEpsilonF = 1.0f / float(gUnity);
    static constexpr double kEpsilonD = 1.0 / double(gUnity);

  private:
    explicit constexpr Fixed(BaseType v) : mValue(v)
    {
        cacheDouble();
    }

  public:
    static MyT FromFixed(BaseType v)
    {
        return MyT{v};
    }

    explicit constexpr Fixed() : mValue(0)
    {
        cacheDouble();
    }

    // construction from floating point types
    template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    explicit constexpr Fixed(const T &v) : mValue(static_cast<BaseType>((double)v * mScalingValue))
    { // use double precision to ensure enough headroom for the scaling
        cacheDouble();
    }

    // convert a fixed type to another, which is really just a shift. but which direction, and before or after casting
    // is important.

    // where our type is greater, cast first.
    template <uint8_t TOtherIntBits,
              uint8_t TOtherFractBits,
              typename TOtherBaseType,
              std::enable_if_t<(sizeof(BaseType) > sizeof(TOtherBaseType)), int> = 0>
    explicit constexpr Fixed(const Fixed<TOtherIntBits, TOtherFractBits, TOtherBaseType> &v)
        : mValue(const_shift<kFractBits - TOtherFractBits>(static_cast<BaseType>(v.mValue)))
    {
        cacheDouble();
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
    }

    constexpr double ToDouble() const
    {
        return (double)mValue / mScalingValue;
    }

    constexpr float ToFloat() const
    {
        return (float)mValue / mScalingValue;
    }

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

    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = OperandTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr [[nodiscard]] ResultType Multiply(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
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

    template <uint8_t TResultIntBits, // caller must specify.
              uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DivideOperandTypeHelper<TResultIntBits, TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr [[nodiscard]] ResultType DivideSlow(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
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
    constexpr [[nodiscard]] ResultType ReciprocalSlow() const
    {
        static constexpr MyT One{mScalingValue};
        return One.DivideSlow<TResultIntBits>(*this);
    }

    // modulo
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename ResultType = Fixed<kIntBits, (TypeInfo::MaxFractBits - kIntBits), BaseType>>
    constexpr [[nodiscard]] ResultType Modulo(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
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

    constexpr [[nodiscard]] MyT Floor() const
    {
        return MyT{mValue & ~mFractMask};
    }

    constexpr [[nodiscard]] MyT Ceil() const
    {
        BaseType t = mValue & ~mFractMask;
        t += gUnity;
        return MyT{t};
    }

    template <typename T = TBaseType, std::enable_if_t<std::is_signed<T>::value, int> = 0>
    constexpr [[nodiscard]] MyT Abs() const
    {
        // union {
        //     BaseType val;
        //     UnsignedBaseType uval;
        // } t;
        // t.val = mValue;
        // t.uval &= gAbsMask;
        return FromFixed(mValue < 0 ? -mValue : mValue);
    }

    template <typename T = TBaseType, std::enable_if_t<!std::is_signed<T>::value, int> = 0>
    constexpr [[nodiscard]] MyT Abs() const
    {
        return *this;
    }

    template <typename T = TBaseType, std::enable_if_t<std::is_signed<T>::value, int> = 0>
    constexpr [[nodiscard]] MyT Negate() const
    {
        return FromFixed(-mValue);
    }

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

    constexpr [[nodiscard]] CorrespondingSignedType MakeSigned() const
    {
        return *this;
    }

    // for unsigned, we should make it signed.
    template <typename T = TBaseType, std::enable_if_t<!std::is_signed<T>::value, int> = 0>
    constexpr [[nodiscard]] CorrespondingSignedType Negate() const
    {
        return MakeSigned().Negate();
    }

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
            conditional<ResultAllows64Bit, FPAutoBaseType<IdealWidth>, FPAutoBaseType32<IdealWidth>>::type;
        using ResultBaseTypeInfo = FPTypeInfo<ResultBaseType>;

        // now truncate as needed.
        static constexpr uint8_t ResultFractBits =
            std::min<uint8_t>(IdealFractBits, ResultBaseTypeInfo::MaxFractBits - IdealIntBits);

        using ResultType = Fixed<IdealIntBits, ResultFractBits, ResultBaseType>;
        using FractResultType = Fixed<0, ResultFractBits, ResultBaseType>;
    };

    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr [[nodiscard]] ResultType Subtract(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        ResultType ta{*this};
        ResultType tb{b};

        return ResultType::FromFixed(ta.mValue - tb.mValue);
    }

    template <typename TypeHelper = DifferenceTypeHelper<TIntBits, TFractBits, BaseType>,
              typename ResultType = typename TypeHelper::FractResultType>
    constexpr [[nodiscard]] ResultType Fract() const
    {
        // this will always be the same base type, just different intbits specification so optimal with elision.
        return ResultType{this->Subtract(Floor())};
    }

    // add
    // even though we could theoretically think of ways in which this is different than subtract, none
    // of them really matter. If difference is robust to signedness and unsignedness, this will be equally.
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr [[nodiscard]] ResultType Add(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        ResultType ta{*this};
        ResultType tb{b};
        return ResultType::FromFixed(ta.mValue + tb.mValue);
    }

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

    // Fast method (just change intbits & fractbits and use same value)
    template <uint8_t amt,
              typename Helper = ShiftHelper<amt>,
              typename ResultType = typename Helper::ShiftLeftResultType,
              typename std::enable_if<Helper::EnableFastShiftLeft, int>::type = 0>
    constexpr [[nodiscard]] ResultType ShiftLeft() const
    {
        return ResultType::FromFixed(mValue);
    }

    // "Slow" method (underlying value must be shifted)
    template <uint8_t amt,
              typename Helper = ShiftHelper<amt>,
              typename ResultType = typename Helper::ShiftLeftResultType,
              typename std::enable_if<!Helper::EnableFastShiftLeft, int>::type = 0>
    constexpr [[nodiscard]] ResultType ShiftLeft() const
    {
        return ResultType::FromFixed(mValue << amt);
    }

    // Fast method (just change intbits & fractbits and use same value)
    template <uint8_t amt,
              typename Helper = ShiftHelper<amt>,
              typename ResultType = typename Helper::ShiftRightResultType,
              typename std::enable_if<Helper::EnableFastShiftRight, int>::type = 0>
    constexpr [[nodiscard]] ResultType ShiftRight() const
    {
        return ResultType::FromFixed(mValue);
    }

    // "Slow" method (underlying value must be shifted)
    template <uint8_t amt,
              typename Helper = ShiftHelper<amt>,
              typename ResultType = typename Helper::ShiftRightResultType,
              typename std::enable_if<!Helper::EnableFastShiftRight, int>::type = 0>
    constexpr [[nodiscard]] ResultType ShiftRight() const
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
    constexpr [[nodiscard]] bool IsGreaterThan(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        ResultType ta{*this};
        ResultType tb{b};
        return ta.mValue > tb.mValue;
    }

    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr [[nodiscard]] bool IsGreaterThanOrEquals(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        ResultType ta{*this};
        ResultType tb{b};
        return ta.mValue >= tb.mValue;
    }

    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr [[nodiscard]] bool IsLessThan(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        ResultType ta{*this};
        ResultType tb{b};
        return ta.mValue < tb.mValue;
    }

    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr [[nodiscard]] bool IsLessThanOrEquals(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        ResultType ta{*this};
        ResultType tb{b};
        return ta.mValue <= tb.mValue;
    }

    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr [[nodiscard]] bool IsApproximatelyEqualTo(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        ResultType ta{*this};
        ResultType tb{b};
        return ta.mValue == tb.mValue;
    }

    // non-mutating operators. only don't support any division operators.
    // *
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = OperandTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr [[nodiscard]] ResultType operator*(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        return Multiply(b);
    }

    // %
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename ResultType = Fixed<kIntBits, (TypeInfo::MaxFractBits - kIntBits), BaseType>>
    constexpr [[nodiscard]] ResultType operator%(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        return Modulo(b);
    }

    // -
    template <typename T = TBaseType, std::enable_if_t<std::is_signed<T>::value, int> = 0>
    constexpr [[nodiscard]] MyT operator-() const
    {
        return Negate();
    }

    // for unsigned, we should make it signed.
    template <typename T = TBaseType, std::enable_if_t<!std::is_signed<T>::value, int> = 0>
    constexpr [[nodiscard]] CorrespondingSignedType operator-() const
    {
        return Negate();
    }

    // +
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr [[nodiscard]] ResultType operator+(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        return Add(b);
    }

    // -
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr [[nodiscard]] ResultType operator-(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        return Subtract(b);
    }

    // >
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr [[nodiscard]] bool operator>(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        return IsGreaterThan(b);
    }

    // >=
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr [[nodiscard]] bool operator>=(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        return IsGreaterThanOrEquals(b);
    }
    // <
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr [[nodiscard]] bool operator<(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        return IsLessThan(b);
    }

    // <=
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr [[nodiscard]] bool operator<=(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
    {
        return IsLessThanOrEquals(b);
    }

    // ==
    template <uint8_t TIntBitsB,
              uint8_t TFractBitsB,
              typename BaseTypeB,
              typename TypeHelper = DifferenceTypeHelper<TIntBitsB, TFractBitsB, BaseTypeB>,
              typename ResultType = typename TypeHelper::ResultType>
    constexpr [[nodiscard]] bool operator==(const Fixed<TIntBitsB, TFractBitsB, BaseTypeB> &b) const
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

} // namespace clarinoid
