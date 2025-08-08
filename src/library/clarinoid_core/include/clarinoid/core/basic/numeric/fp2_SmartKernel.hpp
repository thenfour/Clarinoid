
// todo: a lot of "safety" checks promote straight to 64-bit, which is almost always incorrect and inefficient.
// better to add logic to each operation plan which "only promote to 64-bit when fractbits goes below N" or so.
#pragma once

#include <cassert>
#include <limits>
#include <type_traits>


#include "fp2_base.hpp"

namespace clarinoid
{

struct FxSmartKernel
{
  template <typename TFormat>
  using ValueType = FxValue<TFormat>;

  template <typename TFormatA, typename TFormatB>
  using MulResultFormat =
      std::conditional_t<(TFormatA::ValueBits + TFormatB::ValueBits <= TFormatA::StorageValueBits),
                         TFormatA,
                         std::conditional_t<(TFormatA::HeadroomBits >= TFormatB::HeadroomBits), TFormatA, TFormatB>>;


  // when adding/subtracting (or adding a signed value), you may need to give up 1 bit of precision to add a sign bit.
  // the operands must be brought into having a unified fractbits before performing the addition.
  //
  // The ideal return layout will have magbits = the larger bitness of A or B, plus 1.
  // proof:
  // max value 0.15 + 0.15 (which are just under 1 each) = just under 2. so you need to be able to represent 1.xxxx, which is a Q1.x type.
  // another way to think of it: adding 2 values is like multiplying them by 2 (shifting left by 1 bit -- requiring 1 extra bit of space).
  //
  // the result is signed if either operand is signed, or if the operation is a subtraction
  //
  // examples:
  // 8.8 minus 6.10 => 9.10, signed.
  // 4.60 plus 4.60 => 5.60 (impossible) -> 5.59 reducing precision to fit the value.
  // 64.0 plus 64.0 => 65.0 (impossible) -> 64.0 the result could overflow. employ an overflow strategy (todo in the future.)
  template <typename FormatA, typename FormatB, bool TIsSubtraction>
  struct SumPlan
  {
    static constexpr bool IsSubtraction = TIsSubtraction;
    static constexpr bool ResultIsSigned = FormatA::IsSigned || FormatB::IsSigned || IsSubtraction;

    // Align fractional bits to the maximum of the two (before any squeezing/narrowing)
    static constexpr int IdealFracBits = (FormatA::FracBits > FormatB::FracBits) ? FormatA::FracBits
                                                                                 : FormatB::FracBits;

    // Compute shifts needed to align operands to IdealFracBits
    static constexpr int AFracDelta = IdealFracBits - FormatA::FracBits;
    static constexpr int BFracDelta = IdealFracBits - FormatB::FracBits;
    static constexpr int ALeftShift = (AFracDelta > 0) ? AFracDelta : 0;
    static constexpr int ARightShift = (AFracDelta < 0) ? -AFracDelta : 0;
    static constexpr int BLeftShift = (BFracDelta > 0) ? BFracDelta : 0;
    static constexpr int BRightShift = (BFracDelta < 0) ? -BFracDelta : 0;

    // After alignment, effective magnitude bits for each operand increases by any left shift applied.
    static constexpr int MagA = FormatA::MagBits;
    static constexpr int MagB = FormatB::MagBits;

    // Ideal magnitude bits needs to accommodate possible carry ( +1 )
    //static constexpr int IdealMagBits = ((EffectiveMagA > EffectiveMagB) ? EffectiveMagA : EffectiveMagB) + 1;
    static constexpr int IdealMagBits = ((MagA > MagB) ? MagA : MagB) + 1;

    // First build a widest-possible format (up to 64-bit storage) squeezing if necessary.
    using WideRaw = std::conditional_t<ResultIsSigned, int64_t, uint64_t>;
    using WideSqueezed = SqueezedFormat<WideRaw, IdealMagBits, IdealFracBits>;
    using WideFormat = typename WideSqueezed::FormatType;  // may have reduced frac then mag to fit 64-bit

    // Now attempt to narrow storage while still fitting required value bits.
    using Narrowed = NarrowedFormat<WideFormat, uint8_t, uint16_t, uint32_t, uint64_t>;
    using ResultFormat = typename Narrowed::FormatType;

    // Recompute final shifts relative to ResultFormat::FracBits (may have been reduced from IdealFracBits).
    static constexpr int TargetFracBits = ResultFormat::FracBits;
    static constexpr int ADeltaFinal = TargetFracBits - FormatA::FracBits;
    static constexpr int BDeltaFinal = TargetFracBits - FormatB::FracBits;
    static constexpr int ALeftShiftFinal = (ADeltaFinal > 0) ? ADeltaFinal : 0;
    static constexpr int ARightShiftFinal = (ADeltaFinal < 0) ? -ADeltaFinal : 0;
    static constexpr int BLeftShiftFinal = (BDeltaFinal > 0) ? BDeltaFinal : 0;
    static constexpr int BRightShiftFinal = (BDeltaFinal < 0) ? -BDeltaFinal : 0;

    using ResultValueType = FxValue<ResultFormat>;

    static constexpr ResultValueType Execute(const ValueType<FormatA>& a, const ValueType<FormatB>& b)
    {
      using ResRaw = typename ResultFormat::RawType;

      ResRaw a_adj = static_cast<ResRaw>(a.mRawValue);
      if constexpr (ALeftShiftFinal > 0)
        a_adj = shl_safe<ResRaw, ALeftShiftFinal>(a_adj);
      if constexpr (ARightShiftFinal > 0)
        a_adj = sar_safe<ResRaw, ARightShiftFinal>(a_adj);

      ResRaw b_adj = static_cast<ResRaw>(b.mRawValue);
      if constexpr (BLeftShiftFinal > 0)
        b_adj = shl_safe<ResRaw, BLeftShiftFinal>(b_adj);
      if constexpr (BRightShiftFinal > 0)
        b_adj = sar_safe<ResRaw, BRightShiftFinal>(b_adj);

      ResRaw result = add_or_sub_mod2n<ResRaw, IsSubtraction>(a_adj, b_adj);

      // Ensure this is constexpr as well:
      return FxValue<ResultFormat>::FromRaw(result);
    }
  };

  template <typename Format>
  struct NegatePlan
  {
    using Raw = typename Format::RawType;
    static constexpr bool InputSigned = Format::IsSigned;
    static constexpr int MagBits = Format::MagBits;
    static constexpr int FracBits = Format::FracBits;
    static constexpr int ValueBits = Format::ValueBits;
    static constexpr int HeadroomBits = Format::HeadroomBits;

    // Helper: next wider unsigned raw
    template <typename R>
    struct NextUnsigned
    {
      using type = void;
    };
    template <>
    struct NextUnsigned<uint8_t>
    {
      using type = uint16_t;
    };
    template <>
    struct NextUnsigned<uint16_t>
    {
      using type = uint32_t;
    };
    template <>
    struct NextUnsigned<uint32_t>
    {
      using type = uint64_t;
    };
    template <>
    struct NextUnsigned<uint64_t>
    {
      using type = void;
    };

    // Case classification
    static constexpr bool AlreadySigned = InputSigned;
    static constexpr bool CanSignViaHeadroom = (!AlreadySigned) && (HeadroomBits > 0);
    static constexpr bool CanPromoteRaw = (!AlreadySigned) && (HeadroomBits == 0) &&
                                          !std::is_same_v<typename NextUnsigned<std::make_unsigned_t<Raw>>::type, void>;
    static constexpr bool FullyUtilized64 = (!AlreadySigned) && (HeadroomBits == 0) &&
                                            std::is_same_v<std::make_unsigned_t<Raw>, uint64_t>;
    static constexpr bool CanDropFracBit = FullyUtilized64 && (FracBits > 0);
    static constexpr bool NegationImpossible = FullyUtilized64 && (FracBits == 0);

    // Decide result layout & raw type
    // 1. Already signed: keep same
    // 2. Use headroom -> same bit layout, switch to signed raw of same width
    // 3. Promote raw -> same layout, wider signed raw
    // 4. Drop one fractional bit -> FracBits-1, signed raw of same width (int64_t)
    // 5. Impossible -> static_assert

    // Result fractional bits after potential drop
    static constexpr int ResultFracBits = CanDropFracBit ? (FracBits - 1) : FracBits;
    static constexpr int ResultMagBits = MagBits;  // unchanged

    // Raw type selection
    using ChosenRaw = std::conditional_t<
        AlreadySigned,
        Raw,
        std::conditional_t<
            CanSignViaHeadroom,
            std::make_signed_t<std::make_unsigned_t<Raw>>,
            std::conditional_t<CanPromoteRaw,
                               std::make_signed_t<typename NextUnsigned<std::make_unsigned_t<Raw>>::type>,
                               std::conditional_t<CanDropFracBit, int64_t, void>>>>;

    static_assert(!NegationImpossible,
                  "NegatePlan: cannot negate an unsigned fully-utilized 64-bit integer with no fractional bits (no "
                  "space for sign)");

    using ResultFormat = FxFormat<FxLayout<ResultMagBits, ResultFracBits>, FxStorageTraits<ChosenRaw>>;
    using ResultValueType = FxValue<ResultFormat>;

    // Execution adjustments
    static constexpr bool DropFrac = CanDropFracBit;

    template <typename TInValue>
    static constexpr ResultValueType Execute(const TInValue& v)
    {
      using SignedRaw = typename ResultFormat::RawType;
      // Adjust raw if we dropped a fractional bit (right shift 1 -> truncate toward +inf for unsigned; acceptable per spec)
      auto rawIn = v.mRawValue;
      if constexpr (DropFrac)
      {
        rawIn = static_cast<decltype(rawIn)>(rawIn >> 1);  // lose 1 LS fractional bit
      }
      // Cast to signed target raw and negate
      auto signedVal = static_cast<SignedRaw>(rawIn);
      auto neg = static_cast<SignedRaw>(-signedVal);
      return ResultValueType::FromRaw(neg);
    }
  };

  //template <typename TFormat>
  //using NegateResultFormat = std::conditional_t<
  //    TFormat::IsSigned,
  //    TFormat,
  //    FxFormat<typename TFormat::LayoutType, FxStorageTraits<std::make_signed_t<typename TFormat::RawType>>>>;

  // constants
  template <typename TFormat>
  static constexpr typename TFormat::RawType RawOne()
  {
    if constexpr (TFormat::MagBits == 0)
    {
      // If no magnitude bits, raw one is the max value for the fractional bits (useful for normalized full scale)
      return TFormat::RawMax;
    }
    // Otherwise, raw one is 1 shifted left by the fractional bits
    return static_cast<typename TFormat::RawType>(1) << TFormat::FracBits;
  }

  // Construction methods (same as naive)
  template <typename TDestFormat>
  [[nodiscard]] static constexpr ValueType<TDestFormat> ConstructFromRaw(typename TDestFormat::RawType raw)
  {
    return FxValue<TDestFormat>::FromRaw(raw);
  }

  template <typename TDestFormat>
  [[nodiscard]] static constexpr ValueType<TDestFormat> ConstructFromFloat(double value)
  {
    using Raw = typename TDestFormat::RawType;
    Raw raw = static_cast<Raw>(value * RawOne<TDestFormat>());
    return FxValue<TDestFormat>::FromRaw(raw);
  }

  template <typename TFormat>
  [[nodiscard]] static constexpr double ToFloat(const ValueType<TFormat>& value)
  {
    return static_cast<double>(value.mRawValue) / RawOne<TFormat>();
  }

  // Smart addition - handles different fractional bits via shifting
  template <typename TFormatA, typename TFormatB>
  [[nodiscard]] static constexpr auto Add(const ValueType<TFormatA>& a, const ValueType<TFormatB>& b)
  {
    return SumPlan<TFormatA, TFormatB, false>::Execute(a, b);
  }

  template <typename TFormatA, typename TFormatB>
  [[nodiscard]] static constexpr auto Subtract(const ValueType<TFormatA>& a, const ValueType<TFormatB>& b)
  {
    return SumPlan<TFormatA, TFormatB, true>::Execute(a, b);
  }

  // Smart multiplication with shift elision
  template <typename TFormatA, typename TFormatB>
  [[nodiscard]] static constexpr auto Mul(const ValueType<TFormatA>& a, const ValueType<TFormatB>& b)
  {
    using ResultFormat = MulResultFormat<TFormatA, TFormatB>;

    // Calculate required intermediate precision
    constexpr int intermediate_value_bits = TFormatA::ValueBits + TFormatB::ValueBits;
    constexpr int result_headroom = ResultFormat::HeadroomBits;

    if constexpr (intermediate_value_bits <= ResultFormat::StorageValueBits)
    {
      // We can fit the multiplication result in the target storage without promotion!
      // This is the key optimization - no 64-bit intermediate needed

      constexpr int total_frac_bits = TFormatA::FracBits + TFormatB::FracBits;
      constexpr int target_frac_bits = ResultFormat::FracBits;
      constexpr int shift_amount = total_frac_bits - target_frac_bits;

      auto intermediate = static_cast<typename ResultFormat::RawType>(a.mRawValue) *
                          static_cast<typename ResultFormat::RawType>(b.mRawValue);

      typename ResultFormat::RawType result_raw;
      if constexpr (shift_amount > 0)
      {
        result_raw = intermediate >> shift_amount;
      }
      else if constexpr (shift_amount < 0)
      {
        result_raw = intermediate << (-shift_amount);
      }
      else
      {
        result_raw = intermediate;  // No shift needed!
      }

      return ConstructFromRaw<ResultFormat>(result_raw);
    }
    else
    {
      // Fallback to wider intermediate type
      using IntermediateType = int64_t;

      IntermediateType intermediate = static_cast<IntermediateType>(a.mRawValue) *
                                      static_cast<IntermediateType>(b.mRawValue);

      constexpr int total_frac_bits = TFormatA::FracBits + TFormatB::FracBits;
      constexpr int target_frac_bits = ResultFormat::FracBits;
      constexpr int shift_amount = total_frac_bits - target_frac_bits;

      typename ResultFormat::RawType result_raw;
      if constexpr (shift_amount > 0)
      {
        result_raw = static_cast<typename ResultFormat::RawType>(intermediate >> shift_amount);
      }
      else if constexpr (shift_amount < 0)
      {
        result_raw = static_cast<typename ResultFormat::RawType>(intermediate << (-shift_amount));
      }
      else
      {
        result_raw = static_cast<typename ResultFormat::RawType>(intermediate);
      }

      return ConstructFromRaw<ResultFormat>(result_raw);
    }
  }

  template <typename TFormat>
  [[nodiscard]] static constexpr auto Negate(const ValueType<TFormat>& a)
  {
    return NegatePlan<TFormat>::Execute(a);
  }


  // for finding the SMALLEST raw type that can hold the given bits
  template <bool TWantsSign, int TMagBits, int TFracBits>
  struct SelectIntegralFormat
  {
    using raw_type =
        typename PickUsableType<TWantsSign, TMagBits + TFracBits, uint8_t, uint16_t, uint32_t, uint64_t>::type;
    using type = FxFormat<FxLayout<TMagBits, TFracBits>, FxStorageTraits<raw_type>>;
  };

  // for finding the type that should be used for holding the literal value. Don't bother storing values in small types if fractional
  template <bool TWantsSign, int TMagBits, int TFracBits>
  struct SelectFractionalFormat
  {
    using raw_type = typename PickUsableType<TWantsSign, TMagBits + TFracBits, uint32_t, uint64_t>::type;
    using type = FxFormat<FxLayout<TMagBits, TFracBits>, FxStorageTraits<raw_type>>;
  };

  template <bool TWantsSign, int TMagBits, int TFracBits>
  using LiteralFormat = std::conditional_t<(TFracBits > 0),
                                           typename SelectFractionalFormat<TWantsSign, TMagBits, TFracBits>::type,
                                           typename SelectIntegralFormat<TWantsSign, TMagBits, TFracBits>::type>;


  // ---------------- Runtime format decision helpers ----------------
  template <int TFracBits, bool TSigned = true, typename TRawOverride = void>
  struct FracBitsDecision
  {
    static_assert(TFracBits >= 0, "Negative fractional bits");
    using Raw = std::conditional_t<
        std::is_same_v<TRawOverride, void>,
        std::conditional_t<TSigned,
                           std::conditional_t<(TFracBits < std::numeric_limits<int32_t>::digits), int32_t, int64_t>,
                           std::conditional_t<(TFracBits < std::numeric_limits<uint32_t>::digits), uint32_t, uint64_t>>,
        TRawOverride>;
    static_assert(std::is_integral_v<Raw>, "Raw must be integral");
    static constexpr int StorageValueBits = std::numeric_limits<Raw>::digits;
    static_assert(TFracBits < StorageValueBits, "Too many fractional bits for chosen raw type");
    static constexpr int MagBits = StorageValueBits - TFracBits;
    using Format = FxFormat<FxLayout<MagBits, TFracBits>, FxStorageTraits<Raw>>;
  };
  template <int TMagBits, bool TSigned = true, typename TRawOverride = void>
  struct MagBitsDecision
  {
    static_assert(TMagBits >= 0, "Negative magnitude bits");
    using Raw = std::conditional_t<
        std::is_same_v<TRawOverride, void>,
        std::conditional_t<TSigned,
                           std::conditional_t<(TMagBits < std::numeric_limits<int32_t>::digits), int32_t, int64_t>,
                           std::conditional_t<(TMagBits < std::numeric_limits<uint32_t>::digits), uint32_t, uint64_t>>,
        TRawOverride>;
    static_assert(std::is_integral_v<Raw>, "Raw must be integral");
    static constexpr int StorageValueBits = std::numeric_limits<Raw>::digits;
    static_assert(TMagBits < StorageValueBits, "Too many magnitude bits for chosen raw type");
    static constexpr int FracBits = StorageValueBits - TMagBits;
    using Format = FxFormat<FxLayout<TMagBits, FracBits>, FxStorageTraits<Raw>>;
  };
  template <int TMagBits, int TFracBits, bool TSigned = true, typename TRawOverride = void>
  struct MagFracBitsDecision
  {
    static_assert(TMagBits >= 0 && TFracBits >= 0, "Negative bit counts");
    using Raw = std::conditional_t<
        std::is_same_v<TRawOverride, void>,
        std::conditional_t<
            TSigned,
            std::conditional_t<((TMagBits + TFracBits) <= std::numeric_limits<int32_t>::digits), int32_t, int64_t>,
            std::conditional_t<((TMagBits + TFracBits) <= std::numeric_limits<uint32_t>::digits), uint32_t, uint64_t>>,
        TRawOverride>;
    static_assert(std::is_integral_v<Raw>, "Raw must be integral");
    static constexpr int StorageValueBits = std::numeric_limits<Raw>::digits;
    static_assert((TMagBits + TFracBits) <= StorageValueBits, "Bit counts exceed storage capacity");
    using Format = FxFormat<FxLayout<TMagBits, TFracBits>, FxStorageTraits<Raw>>;
  };
  template <int TFracBits, bool TSigned = true, typename TRawOverride = void>
  [[nodiscard]] static constexpr auto MakeFromFracBits(double value)
  {
    using D = FracBitsDecision<TFracBits, TSigned, TRawOverride>;
    auto v = ConstructFromFloat<typename D::Format>(value);
    return v;
  }
  template <int TMagBits, bool TSigned = true, typename TRawOverride = void>
  [[nodiscard]] static constexpr auto MakeFromMagBits(double value)
  {
    using D = MagBitsDecision<TMagBits, TSigned, TRawOverride>;
    auto v = ConstructFromFloat<typename D::Format>(value);
    return v;
  }
  template <int TMagBits, int TFracBits, bool TSigned = true, typename TRawOverride = void>
  [[nodiscard]] static constexpr auto MakeFromMagFracBits(double value)
  {
    using D = MagFracBitsDecision<TMagBits, TFracBits, TSigned, TRawOverride>;
    auto v = ConstructFromFloat<typename D::Format>(value);
    return v;
  }

  // -------------------------------- Literal decision descriptor -----------------
  // TParsed has:
  // - neg: whether the literal is negative
  // - abs_val: absolute value of the literal (without sign)
  // - has_frac: whether the literal has a fractional part
  // - numerator: numerator of the literal (for fractionals)
  // - denominator: denominator of the literal (for fractionals)
  template <typename Parsed>
  struct LiteralDecision
  {
    using ParsedType = Parsed;
    static constexpr bool SourceNegative = Parsed::neg;
    static constexpr std::uint64_t AbsValue = Parsed::abs_val;
    static constexpr bool HasFraction = Parsed::has_frac;

    static constexpr bool NeedsSign = SourceNegative;

    static constexpr int MagBits = needed_int_bits(AbsValue);

    // if there's a fraction, then make sure we have bits for it.
    // - if magbits require an int64, then we can use int64_t as the raw type.
    // - otherwise, use int32.
    static constexpr int MinFractBitsNeeded =
        HasFraction
            ? 1
            : 0;  // we don't actualyl know how many fract bits are required to store the value accurately. we will just go for the biggest we can, once the raw type has been selected.
    using FormatWithMinFractBits = LiteralFormat<NeedsSign, MagBits, MinFractBitsNeeded>;

    // if !HasFraction, then FormatWithMinFractBits is already correct.
    // otherwise, fill out fract bits to fill remaining bits of the raw type.
    using Format =
        std::conditional_t<HasFraction,
                           LiteralFormat<NeedsSign, MagBits, FormatWithMinFractBits::StorageValueBits - MagBits>,
                           FormatWithMinFractBits>;

    using FxValueType = FxValue<Format>;
    static constexpr int FracBits = Format::FracBits;

    // Raw computation -- convert Parsed::numerator & denominator to the correct value. (rounded)
    static constexpr std::uint64_t scale = (FracBits > 0) ? (std::uint64_t(1) << FracBits) : 1ULL;
    static constexpr bool WillOverflow = Parsed::denominator &&
                                         (Parsed::numerator > (std::numeric_limits<std::uint64_t>::max() / scale));
    static constexpr std::uint64_t RawUnsigned = WillOverflow
                                                     ? std::numeric_limits<std::uint64_t>::max()
                                                     : (Parsed::denominator
                                                            ? ((Parsed::numerator * scale + (Parsed::denominator / 2)) /
                                                               Parsed::denominator)
                                                            : 0ULL);

    using RawType = typename Format::RawType;
    static constexpr RawType RawValue = static_cast<RawType>(SourceNegative ? -static_cast<std::int64_t>(RawUnsigned)
                                                                            : static_cast<std::int64_t>(RawUnsigned));

    static constexpr FxValueType value = FxValueType::FromRaw(RawValue);
  };

  template <typename Parsed>
  static constexpr auto MakeLiteral(const Parsed&)
  {
    return LiteralDecision<Parsed>::value();
  }


};  // class FxSmartKernel

}  // namespace clarinoid
