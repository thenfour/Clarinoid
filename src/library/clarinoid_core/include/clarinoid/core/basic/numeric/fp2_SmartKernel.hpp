#pragma once

#include <cassert>
#include <limits>
#include <type_traits>


#include "fp2_base.hpp"

namespace clarinoid
{

// Smart kernel that exploits headroom and shift elision for better performance
struct FxSmartKernel
{
  // Value type - just use FxValue for now
  template <typename TFormat>
  using ValueType = FxValue<TFormat>;

  // Helper to select optimal raw type based on requirements
  template <bool TWantsSign, int TIntBits, int TFracBits>
  using OptimalRawType = OptimalRawType_t<TWantsSign, TIntBits, TFracBits>;

  // Smart multiplication result format selection
  template <typename TFormatA, typename TFormatB>
  using MulResultFormat = std::conditional_t<
      // If the total intermediate bits fit in the left operand's storage with headroom
      (TFormatA::ValueBits + TFormatB::ValueBits <= TFormatA::StorageValueBits),
      TFormatA,  // Use left operand format (no promotion needed)

      // If the left operand has more headroom, use it
      std::conditional_t<(TFormatA::HeadroomBits >= TFormatB::HeadroomBits), TFormatA, TFormatB>>;

  // Smart addition result format selection
  template <typename TFormatA, typename TFormatB>
  using AddResultFormat = std::conditional_t<
      // If same fractional bits and A has more headroom, use A
      (TFormatA::FracBits == TFormatB::FracBits && TFormatA::HeadroomBits >= TFormatB::HeadroomBits),
      TFormatA,
      // Otherwise use B if it has more headroom
      std::conditional_t<(TFormatB::HeadroomBits >= TFormatA::HeadroomBits), TFormatB, TFormatA>>;

  // Negation result format - same smart promotion as naive for now
  template <typename TFormat>
  using NegateResultFormat = std::conditional_t<
      TFormat::IsSigned,
      TFormat,
      FxFormat<typename TFormat::LayoutType, FxStorageTraits<std::make_signed_t<typename TFormat::RawType>>>>;

  // constants
  template <typename TFormat>
  static constexpr typename TFormat::RawType RawOne()
  {
    return TFormat::RawMax;  // Full-scale normalized
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
      auto result_raw = a.mRawValue + b.mRawValue;
      return ConstructFromRaw<ResultFormat>(result_raw);
    }
    else
    {
      // Different fractional bits - need to align
      constexpr int target_frac_bits = ResultFormat::FracBits;
      constexpr int a_shift = target_frac_bits - TFormatA::FracBits;
      constexpr int b_shift = target_frac_bits - TFormatB::FracBits;

      auto aligned_a = (a_shift >= 0) ? (a.mRawValue << a_shift) : (a.mRawValue >> (-a_shift));
      auto aligned_b = (b_shift >= 0) ? (b.mRawValue << b_shift) : (b.mRawValue >> (-b_shift));

      auto result_raw = aligned_a + aligned_b;
      return ConstructFromRaw<ResultFormat>(result_raw);
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
      using IntermediateType = std::conditional_t<sizeof(typename ResultFormat::RawType) <= 4,
                                                  int64_t,
                                                  int64_t>;  // Could use __int128 for 64-bit types

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
};

}  // namespace clarinoid
