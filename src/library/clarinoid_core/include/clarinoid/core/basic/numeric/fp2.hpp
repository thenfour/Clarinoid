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
    Fixed r;
    r.mValue = TKernel::template ConstructFromRaw<TFormat>(raw);
    return r;
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
    auto v = TKernel::template Add<TFormat, TOtherFormat>(mValue, rhs.mValue);
    return Fixed<ResultFormat, TKernel>::FromRaw(v.mRawValue);
  }

  template <typename TOtherFormat, typename TOtherKernel>
  [[nodiscard]] constexpr auto operator*(const Fixed<TOtherFormat, TOtherKernel>& rhs) const
  {
    using ResultFormat = typename TKernel::template MulResultFormat<TFormat, TOtherFormat>;
    auto v = TKernel::template Mul<TFormat, TOtherFormat>(mValue, rhs.mValue);
    return Fixed<ResultFormat, TKernel>::FromRaw(v.mRawValue);
  }

  [[nodiscard]] constexpr auto operator-() const
  {
    using ResultFormat = typename TKernel::template NegateResultFormat<TFormat>;
    auto v = TKernel::template Negate<TFormat>(mValue);
    return Fixed<ResultFormat, TKernel>::FromRaw(v.mRawValue);
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

// I would like for callers to be able to construct via many convenient ways:

// auto val = fx<15> { y }; // uses default raw type (signed by default as well), cannot deduce intbits so full overhead used.
// auto val = fx<15, int16_t> { y }; // explicit fract bits and type; this is similar to other conventional libraries.
// auto val = fx<1,15> { y }; // explicit intbits & fracbits, default raw type.
// auto val = fx<2,4, uint16_t> { y }; // explicit I,F, and raw type.

// auto val = fx<15> { 10.5 }; // explicit fracbits, deduces intbits and signedness via constexpr value.
// auto val = fx<4, uint16_t> { 1.7 }; // explicit I,F, and raw type.
// auto val = fx<4, uint16_t> { 1.7 }; // explicit I,F, and raw type.


// // Convenient type aliases for common formats
// using Q15_16 = Fixed<FxFormat<FxLayout<15, 16>, FxStorageTraits<int32_t>>>;
// using Q7_8 = Fixed<FxFormat<FxLayout<7, 8>, FxStorageTraits<int16_t>>>;
// using Q0_31 =
//     Fixed<FxFormat<FxLayout<0, 31>, FxStorageTraits<int32_t>>>;  // Valid: 31 value bits in 31-bit signed storage

// // Smart kernel variants with optimizations
// using Q15_16_Smart = Fixed<FxFormat<FxLayout<15, 16>, FxStorageTraits<int32_t>>, FxSmartKernel>;
// using Q7_8_Smart = Fixed<FxFormat<FxLayout<7, 8>, FxStorageTraits<int16_t>>, FxSmartKernel>;
// using Q0_31_Smart = Fixed<FxFormat<FxLayout<0, 31>, FxStorageTraits<int32_t>>, FxSmartKernel>;

// Convenient template alias for automatic raw type selection
// Usage: Fx<1, 15> (signed by default), FixedAuto<1, 15, false> (unsigned), FixedAuto<1, 15, true, int16_t> (explicit raw type)
// template <int TIntBits,
//           int TFracBits,
//           typename TRaw = typename FxSmartKernel::template OptimalRawType<TWantsSign, TIntBits, TFracBits>,
//           typename TKernel = FxSmartKernel>
// using Fx = Fixed<FxFormat<FxLayout<TIntBits, TFracBits>, FxStorageTraits<TRaw>>, TKernel>;

// // explicitly signed type
// template <int TIntBits,
//           int TFracBits,
//           typename TRaw = typename FxSmartKernel::template OptimalRawType<true /* wants sign */, TIntBits, TFracBits>,
//           typename TKernel = FxSmartKernel>
// using FixedAutoS = Fixed<FxFormat<FxLayout<TIntBits, TFracBits>, FxStorageTraits<TRaw>>, TKernel>;

// // explicitly unsigned.
// template <int TIntBits,
//           int TFracBits,
//           typename TRaw = typename FxSmartKernel::template OptimalRawType<true /* wants sign */, TIntBits, TFracBits>,
//           typename TKernel = FxSmartKernel>
// using FixedAutoS = Fixed<FxFormat<FxLayout<TIntBits, TFracBits>, FxStorageTraits<TRaw>>, TKernel>;

// // deduce intbits, signed
// template <int TFracBits,
//           int TIntBits = ,
//           typename TRaw = typename FxSmartKernel::template OptimalRawType<true /* wants sign */, TIntBits, TFracBits>,
//           typename TKernel = FxSmartKernel>
// using FixedQS = Fixed<FxFormat<FxLayout<TIntBits, TFracBits>, FxStorageTraits<TRaw>>, TKernel>;


}  // namespace clarinoid
