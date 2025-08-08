#pragma once

#include <cassert>
#include <limits>
#include <type_traits>


#include "fp2_NaiveKernel.hpp"
#include "fp2_SmartKernel.hpp"
#include "fp2_base.hpp"


namespace clarinoid
{
template <typename TFormat, typename TKernel = FxNaiveKernel>
struct Fixed
{
  using FormatType = TFormat;
  using KernelType = TKernel;
  using RawType = typename TFormat::RawType;
  using ValueType = typename TKernel::template ValueType<TFormat>;

  ValueType mValue;

  // Access raw value
  [[nodiscard]] constexpr RawType RawValue() const
  {
    return mValue.mRawValue;
  }

  // Construction
  constexpr Fixed()
      : mValue()
  {
  }

  // From raw (explicit to prevent accidents)
  [[nodiscard]] static constexpr Fixed FromRaw(RawType raw)
  {
    Fixed result;
    result.mValue = TKernel::template ConstructFromRaw<TFormat>(raw);
    return result;
  }

  // From float/double
  constexpr explicit Fixed(double value)
  {
    mValue = TKernel::template ConstructFromFloat<TFormat>(value);
  }

  // To float
  [[nodiscard]] constexpr double ToFloat() const
  {
    return TKernel::template ToFloat<TFormat>(mValue);
  }

  // Arithmetic operators
  template <typename TOtherFormat, typename TOtherKernel>
  [[nodiscard]] constexpr auto operator+(const Fixed<TOtherFormat, TOtherKernel>& rhs) const
  {
    using ResultFormat = typename TKernel::template AddResultFormat<TFormat, TOtherFormat>;
    Fixed<ResultFormat, TKernel> result;
    result.mValue = TKernel::template Add<TFormat, TOtherFormat>(mValue, rhs.mValue);
    return result;
  }

  template <typename TOtherFormat, typename TOtherKernel>
  [[nodiscard]] constexpr auto operator*(const Fixed<TOtherFormat, TOtherKernel>& rhs) const
  {
    using ResultFormat = typename TKernel::template MulResultFormat<TFormat, TOtherFormat>;
    Fixed<ResultFormat, TKernel> result;
    result.mValue = TKernel::template Mul<TFormat, TOtherFormat>(mValue, rhs.mValue);
    return result;
  }

  [[nodiscard]] constexpr auto operator-() const
  {
    using ResultFormat = typename TKernel::template NegateResultFormat<TFormat>;
    Fixed<ResultFormat, TKernel> result;
    result.mValue = TKernel::template Negate<TFormat>(mValue);
    return result;
  }

  // Comparison (same format only for now)
  [[nodiscard]] constexpr bool operator==(const Fixed& rhs) const
  {
    return mValue.mRawValue == rhs.mValue.mRawValue;
  }

  [[nodiscard]] constexpr bool operator<(const Fixed& rhs) const
  {
    return mValue.mRawValue < rhs.mValue.mRawValue;
  }
};

// Convenient type aliases for common formats
using Q15_16 = Fixed<FxFormat<FxLayout<15, 16>, FxStorageTraits<int32_t>>>;
using Q7_8 = Fixed<FxFormat<FxLayout<7, 8>, FxStorageTraits<int16_t>>>;
using Q0_31 =
    Fixed<FxFormat<FxLayout<0, 31>, FxStorageTraits<int32_t>>>;  // Valid: 31 value bits in 31-bit signed storage

// Smart kernel variants with optimizations
using Q15_16_Smart = Fixed<FxFormat<FxLayout<15, 16>, FxStorageTraits<int32_t>>, FxSmartKernel>;
using Q7_8_Smart = Fixed<FxFormat<FxLayout<7, 8>, FxStorageTraits<int16_t>>, FxSmartKernel>;
using Q0_31_Smart = Fixed<FxFormat<FxLayout<0, 31>, FxStorageTraits<int32_t>>, FxSmartKernel>;

// Convenient template alias for automatic raw type selection
// Usage: FixedAuto<1, 15> (signed by default), FixedAuto<1, 15, false> (unsigned), FixedAuto<1, 15, true, int16_t> (explicit raw type)
template <int TIntBits,
          int TFracBits,
          bool TWantsSign = true,
          typename TRaw = typename FxSmartKernel::template OptimalRawType<TWantsSign, TIntBits, TFracBits>,
          typename TKernel = FxSmartKernel>
using FixedAuto = Fixed<FxFormat<FxLayout<TIntBits, TFracBits>, FxStorageTraits<TRaw>>, TKernel>;

}  // namespace clarinoid
