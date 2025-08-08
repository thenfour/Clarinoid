#pragma once

#include <cassert>
#include <limits>
#include <type_traits>

#include "fp2_base.hpp"

namespace clarinoid
{

// Basic kernel with simple policies
struct FxNaiveKernel
{
  // Value type - just use FxValue for now
  template <typename TFormat>
  using ValueType = FxValue<TFormat>;

  // Helper to select optimal raw type based on requirements
  template <bool TWantsSign, int TIntBits, int TFracBits>
  using OptimalRawType = OptimalRawType_t<TWantsSign, TIntBits, TFracBits>;

  // For now, multiplication result uses left operand's format (naive approach)
  template <typename TFormatA, typename TFormatB>
  using MulResultFormat = TFormatA;

  // Addition result uses left operand's format (naive approach)
  template <typename TFormatA, typename TFormatB>
  using AddResultFormat = TFormatA;

  // Negation result format - convert to signed if unsigned
  template <typename TFormat>
  using NegateResultFormat = std::conditional_t<
      TFormat::IsSigned,
      TFormat,
      FxFormat<typename TFormat::LayoutType, FxStorageTraits<std::make_signed_t<typename TFormat::RawType>>>>;

  // Constants
  template <typename TFormat>
  static constexpr typename TFormat::RawType RawOne()
  {
    static_assert(TFormat::FracBits <= TFormat::StorageValueBits, "Cannot represent 1.0 - too many fractional bits");
    return typename TFormat::RawType(1) << TFormat::FracBits;
  }

  // Construction from raw
  template <typename TDestFormat>
  [[nodiscard]] static constexpr ValueType<TDestFormat> ConstructFromRaw(typename TDestFormat::RawType raw)
  {
    return FxValue<TDestFormat>::FromRaw(raw);
  }

  // Construction from float - basic implementation
  template <typename TDestFormat>
  [[nodiscard]] static constexpr ValueType<TDestFormat> ConstructFromFloat(double value)
  {
    using Raw = typename TDestFormat::RawType;
    Raw raw = static_cast<Raw>(value * RawOne<TDestFormat>());
    return FxValue<TDestFormat>::FromRaw(raw);
  }

  // Convert to float
  template <typename TFormat>
  [[nodiscard]] static constexpr double ToFloat(const ValueType<TFormat>& value)
  {
    return static_cast<double>(value.mRawValue) / RawOne<TFormat>();
  }

  // Addition - naive implementation
  template <typename TFormatA, typename TFormatB>
  [[nodiscard]] static constexpr auto Add(const ValueType<TFormatA>& a, const ValueType<TFormatB>& b)
  {
    using ResultFormat = AddResultFormat<TFormatA, TFormatB>;

    // For now, assume same fractional bits - just add raw values
    static_assert(TFormatA::FracBits == TFormatB::FracBits, "Different fractional bits not yet supported");

    auto result_raw = a.mRawValue + b.mRawValue;
    return ConstructFromRaw<ResultFormat>(result_raw);
  }

  // Multiplication - naive implementation
  template <typename TFormatA, typename TFormatB>
  [[nodiscard]] static constexpr auto Mul(const ValueType<TFormatA>& a, const ValueType<TFormatB>& b)
  {
    using ResultFormat = MulResultFormat<TFormatA, TFormatB>;

    // Basic multiplication: multiply raw values and shift down by fractional bits
    using IntermediateType = std::conditional_t<sizeof(typename TFormatA::RawType) <= 4,
                                                int64_t,
                                                int64_t>;  // For now, always use int64_t

    IntermediateType intermediate = static_cast<IntermediateType>(a.mRawValue) *
                                    static_cast<IntermediateType>(b.mRawValue);

    // Shift down by the fractional bits (assuming same frac bits for now)
    static_assert(TFormatA::FracBits == TFormatB::FracBits, "Different fractional bits not yet supported");

    auto result_raw = static_cast<typename ResultFormat::RawType>(intermediate >> TFormatA::FracBits);

    return ConstructFromRaw<ResultFormat>(result_raw);
  }

  // Negation
  template <typename TFormat>
  [[nodiscard]] static constexpr auto Negate(const ValueType<TFormat>& value)
  {
    using ResultFormat = NegateResultFormat<TFormat>;
    static_assert(ResultFormat::IsSigned, "Cannot negate - result format must be signed");
    return ConstructFromRaw<ResultFormat>(-value.mRawValue);
  }
};

}  // namespace clarinoid
