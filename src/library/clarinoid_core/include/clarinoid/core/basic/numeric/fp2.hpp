#pragma once

#include <cassert>
#include <limits>
#include <type_traits>


#include "fp2_NaiveKernel.hpp"
#include "fp2_SmartKernel.hpp"
#include "fp2_base.hpp"


namespace clarinoid
{
template <typename TFormat, typename TKernel = FxSmartKernel>
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

  private:
  struct CtorFromRaw
  {
  };
  struct CtorFromFxValue
  {
  };

  // Private constructor to prevent accidental use
  constexpr Fixed(CtorFromRaw, RawType raw)
      : mValue(TKernel::template ConstructFromRaw<TFormat>(raw))
  {
  }

  // Private constructor to prevent accidental use
  constexpr Fixed(CtorFromFxValue, const ValueType& value)
      : mValue(value)
  {
  }

public:

  // From raw (explicit to prevent accidents)
  [[nodiscard]] static constexpr Fixed FromRaw(RawType raw)
  {
    return {CtorFromRaw{}, raw};
  }

  // From FxValue
  [[nodiscard]] static constexpr Fixed FromFxValue(const ValueType value)
  {
    return {CtorFromFxValue{}, value};
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
// (Implemented below)

// ----------------------------- Detail helpers for deduction ------------------
namespace fx_detail
{
// Portable, always-constexpr bit width computation.
// Returns number of value bits required to represent v (no sign), 0 if v==0.
constexpr int bits_for_magnitude(std::uint64_t v) noexcept
{
  int b = 0;
  while (v)
  {
    ++b;
    v >>= 1;
  }
  return b;
}

// Select smallest signed raw type that can hold value bits (IntBits + FracBits) ignoring sign bit itself.
template <int ValueBits>
struct select_signed_raw
{
  static_assert(ValueBits > 0, "ValueBits must be positive");
  using type =
      std::conditional_t<(ValueBits <= std::numeric_limits<int8_t>::digits),
                         int8_t,
                         std::conditional_t<(ValueBits <= std::numeric_limits<int16_t>::digits),
                                            int16_t,
                                            std::conditional_t<(ValueBits <= std::numeric_limits<int32_t>::digits),
                                                               int32_t,
                                                               int64_t>>>;  // assume fits in 64
  static constexpr int raw_value_bits = std::numeric_limits<type>::digits;
};

template <int IntBits, bool HasFrac>
struct choose_format_from_I
{
  // For integer-only: allocate exactly IntBits (frac=0) in smallest raw.
  // For fractional: maximize frac bits by filling remainder of chosen raw.
  static_assert(IntBits >= 0, "negative IntBits");
  using Raw = typename select_signed_raw<(
      IntBits == 0 ? 1 : IntBits)>::type;  // at least 1 value bit to keep Raw selection sane
  static constexpr int raw_bits = std::numeric_limits<Raw>::digits;
  static constexpr int FracBits = HasFrac ? (raw_bits - IntBits) : 0;
  static_assert(FracBits >= 0, "Not enough bits for requested IntBits");
  using Format = FxFormat<FxLayout<IntBits, FracBits>, FxStorageTraits<Raw>>;
};

// Deduction from a (constexpr) floating value V (limited: requires compiler to treat path as constant expr for type usage).
// We offer a wrapper that returns a Fixed with deduced type.

template <typename T>
struct is_constexpr_float : std::bool_constant<std::is_floating_point_v<T>>
{
};

}  // namespace fx_detail

// ----------------------------- Explicit aliases --------------------------------
// Explicit IntBits + FracBits (auto raw signed)
template <int I, int TF, typename Raw = OptimalRawType_t<true, I, TF>, typename K = FxSmartKernel>
using fx_if = Fixed<FxFormat<FxLayout<I, TF>, FxStorageTraits<Raw>>, K>;

// Explicit IntBits + FracBits + explicit raw
template <int I, int TF, typename Raw, typename K = FxSmartKernel>
using fx_if_r = Fixed<FxFormat<FxLayout<I, TF>, FxStorageTraits<Raw>>, K>;

// Fractional-only: specify F; fill remaining bits of Raw (default int32_t) as IntBits
template <int TF, typename Raw = int32_t, typename K = FxSmartKernel>
using fx_f = Fixed<
    FxFormat<FxLayout<((TF < std::numeric_limits<Raw>::digits) ? (std::numeric_limits<Raw>::digits - TF) : 0), TF>,
                   FxStorageTraits<Raw>>,
          K>;

// ----------------------------- Value-driven deduction (function) ---------------
// Returns a Fixed with deduced format (signed). for constexpr integer or float.
// Rules: IntBits = bits needed for floor(|v|). If fractional part present, FracBits = remaining bits, else 0.
// NOTE: This cannot expose the deduced type name directly (auto return only).

template <typename T, typename K = FxSmartKernel>
constexpr auto fx_auto(T v)
{
  // Determine magnitude integer part
  long double av = v < 0 ? -static_cast<long double>(v) : static_cast<long double>(v);
  auto int_part = static_cast<std::uint64_t>(av);
  int I = fx_detail::bits_for_magnitude(int_part);
  bool has_frac = (static_cast<long double>(int_part) != av);
  if (I == 0 && int_part == 0)
    I = 0;  // keep 0 if magnitude <1
  // Choose format
  if (!has_frac)
  {
    using RawSel = typename fx_detail::select_signed_raw<(I == 0 ? 1 : I)>::type;  // at least 1 value bit
    constexpr int raw_bits = std::numeric_limits<RawSel>::digits;
    using Format = FxFormat<FxLayout<I, 0>, FxStorageTraits<RawSel>>;
    return Fixed<Format, K>(static_cast<double>(v));
  }
  else
  {
    using RawSel = typename fx_detail::select_signed_raw<(I == 0 ? 1 : I)>::type;
    constexpr int raw_bits = std::numeric_limits<RawSel>::digits;
    int frac = raw_bits - I;
    if (frac < 1)
      frac = 1;  // ensure at least 1 fractional bit if fractional value present
    using Format = FxFormat<FxLayout<I, (raw_bits - I)>, FxStorageTraits<RawSel>>;
    return Fixed<Format, K>(static_cast<double>(v));
  }
}

// ----------------------------- Example usage mapping ---------------------------
// auto a = fx_if<1,15>{ 3.25 };          // explicit I,F
// auto b = fx_if_r<2,4,uint16_t>{ 5.5 };  // explicit I,F,Raw
// auto c = fx_f<15>{ 1.0 };               // F only, fill remaining as IntBits
// auto d = fx_auto(10.5);                 // deduced IntBits + frac
// auto e = fx_auto(42.0);                 // integer only (FracBits=0)

}  // namespace clarinoid
