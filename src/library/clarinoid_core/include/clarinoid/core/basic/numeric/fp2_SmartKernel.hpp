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

  template <typename TFormatA, typename TFormatB>
  using AddResultFormat =
      std::conditional_t<(TFormatA::FracBits == TFormatB::FracBits && TFormatA::HeadroomBits >= TFormatB::HeadroomBits),
                         TFormatA,
                         std::conditional_t<(TFormatB::HeadroomBits >= TFormatA::HeadroomBits), TFormatB, TFormatA>>;

  template <typename TFormat>
  using NegateResultFormat = std::conditional_t<
      TFormat::IsSigned,
      TFormat,
      FxFormat<typename TFormat::LayoutType, FxStorageTraits<std::make_signed_t<typename TFormat::RawType>>>>;

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
    using ResultFormat = AddResultFormat<TFormatA, TFormatB>;

    if constexpr (TFormatA::FracBits == TFormatB::FracBits)
    {
      // Same fractional bits - direct addition
      return ConstructFromRaw<ResultFormat>(a.mRawValue + b.mRawValue);
    }
    else
    {
      // Different fractional bits - need to align
      constexpr int target_frac_bits = ResultFormat::FracBits;
      constexpr int a_shift = target_frac_bits - TFormatA::FracBits;
      constexpr int b_shift = target_frac_bits - TFormatB::FracBits;

      auto aligned_a = (a_shift >= 0) ? (a.mRawValue << a_shift) : (a.mRawValue >> (-a_shift));
      auto aligned_b = (b_shift >= 0) ? (b.mRawValue << b_shift) : (b.mRawValue >> (-b_shift));

      return ConstructFromRaw<ResultFormat>(aligned_a + aligned_b);
    }
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

  // Negation (same as naive)
  template <typename TFormat>
  [[nodiscard]] static constexpr auto Negate(const ValueType<TFormat>& value)
  {
    using ResultFormat = NegateResultFormat<TFormat>;
    static_assert(ResultFormat::IsSigned, "Cannot negate - result format must be signed");
    return ConstructFromRaw<ResultFormat>(-value.mRawValue);
  }

  // for finding the SMALLEST raw type that can hold the given bits
  template <bool TWantsSign, int TMagBits, int TFracBits>
  struct SelectIntegralFormat
  {
    static constexpr int SignBits = TWantsSign ? 1 : 0;
    static constexpr int TotalBitsNeeded = SignBits + TMagBits + TFracBits;

    using raw_type =
        std::conditional_t<(TotalBitsNeeded <= 8),
                           std::conditional_t<TWantsSign, int8_t, uint8_t>,
                           std::conditional_t<(TotalBitsNeeded <= 16),
                                              std::conditional_t<TWantsSign, int16_t, uint16_t>,
                                              std::conditional_t<(TotalBitsNeeded <= 32),
                                                                 std::conditional_t<TWantsSign, int32_t, uint32_t>,
                                                                 std::conditional_t<TWantsSign, int64_t, uint64_t>>>>;

    using type = FxFormat<FxLayout<TMagBits, TFracBits>, FxStorageTraits<raw_type>>;
  };

  //template <bool TWantsSign, int TMagBits, int TFracBits>
  //using SmallestFormat = typename SelectSmallestFormat<TWantsSign, TMagBits, TFracBits>::type;

  // for finding the type that should be used for holding the literal value. Don't bother storing values in int8 / int16
  // for example; there's basically 0 chance that it will stay that way. use register-sized.
  template <bool TWantsSign, int TMagBits, int TFracBits>
  struct SelectFractionalFormat
  {
    static constexpr int SignBits = TWantsSign ? 1 : 0;
    static constexpr int TotalBitsNeeded = SignBits + TMagBits + TFracBits;

    using raw_type = std::conditional_t<(TotalBitsNeeded <= 32),
                                        std::conditional_t<TWantsSign, int32_t, uint32_t>,
                                        std::conditional_t<TWantsSign, int64_t, uint64_t>>;

    using type = FxFormat<FxLayout<TMagBits, TFracBits>, FxStorageTraits<raw_type>>;
  };

  template <bool TWantsSign, int TMagBits, int TFracBits>
  using LiteralFormat = std::conditional_t<(TFracBits > 0),
      typename SelectFractionalFormat<TWantsSign, TMagBits, TFracBits>::type,
      typename SelectIntegralFormat<TWantsSign, TMagBits, TFracBits>::type
                                                                 >;// typename SelectSmallestFormat<TWantsSign, TMagBits, TFracBits>::type;

  // -------------------------------- Literal decision descriptor -----------------
  template <typename TParsed>
  struct LiteralDecision
  {
    // Input snapshot
    using Parsed = TParsed;
    static constexpr bool SourceNegative = Parsed::neg;
    static constexpr std::uint64_t AbsValue = Parsed::abs_val;
    static constexpr bool HasFraction = Parsed::has_frac;

    // Sign policy
    static constexpr bool NeedsSign = SourceNegative;

    static constexpr int MagBits = needed_int_bits(AbsValue);

    // Choose storage & fractional bits
    //using Store = OptimalRawType<NeedsSign, MagBits, HasFraction ? (32 - MagBits - (NeedsSign ? 1 : 0)) : 0>;
    //static constexpr int TotalBits = std::numeric_limits<Store>::digits;
    //static constexpr int FracBits = HasFraction ? (TotalBits - MagBits) : 0;

    //using Layout = FxLayout<MagBits, FracBits>;
    //using Storage = FxStorageTraits<Store>;
    //using Format = FxFormat<Layout, Storage>;

    // if there's a fraction, then make sure we have bits for it.
    // - if magbits require an int64, then we can use int64_t as the raw type.
    // - otherwise, use int32.
    static constexpr int MinFractBitsNeeded = HasFraction ? 1 : 0; // we don't actualyl know how many fract bits are required to store the value accurately. we will just go for the biggest we can, once the raw type has been selected.
    using FormatWithMinFractBits = LiteralFormat<NeedsSign, MagBits, MinFractBitsNeeded>;

    // if !HasFraction, then FormatWithMinFractBits is already correct.
    // otherwise, fill out fract bits to fill remaining bits of the raw type.
    using Format = std::conditional_t<HasFraction,
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
};

}  // namespace clarinoid
