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

  template <typename TValue2>
  [[nodiscard]] static constexpr auto MakeFromFxValue(const TValue2& value)
  {
    using ResultFormat = typename TValue2::FormatType;
    return Fixed<ResultFormat, TKernel>::FromFxValue(value);
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
    auto v = TKernel::template Add<TFormat, TOtherFormat>(mValue, rhs.mValue);
    //return MakeFromFxValue(v);
    using ResultFormat = typename decltype(v)::FormatType;
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
    //auto v = TKernel::template Negate<TFormat>(mValue);
    //using ResultFormat = typename decltype(v)::FormatType;
    //return Fixed<ResultFormat, TKernel>::FromRaw(v.mRawValue);

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

// ---------------- Public runtime construction API (delegates to SmartKernel) ----------------
// fx<FracBits, Raw=void, Kernel=FxSmartKernel>(value)  (signed by default)
template <int TFracBits, typename TRaw = void, typename TKernel = FxSmartKernel>
[[nodiscard]] constexpr auto fx(double value)
{
  using Decision = typename TKernel::template FracBitsDecision<TFracBits, true, TRaw>;
  auto v = TKernel::template MakeFromFracBits<TFracBits, true, TRaw>(value);
  return Fixed<typename Decision::Format, TKernel>::FromFxValue(v);
}

// fx_u<FracBits, Raw=void, Kernel=FxSmartKernel>(value)  (unsigned)
template <int TFracBits, typename TRaw = void, typename TKernel = FxSmartKernel>
[[nodiscard]] constexpr auto fx_u(double value)
{
  using Decision = typename TKernel::template FracBitsDecision<TFracBits, false, TRaw>;
  auto v = TKernel::template MakeFromFracBits<TFracBits, false, TRaw>(value);
  return Fixed<typename Decision::Format, TKernel>::FromFxValue(v);
}

// fx_m<MagBits, Raw=void, Kernel=FxSmartKernel>(value)  (signed by default)
template <int TMagBits, typename TRaw = void, typename TKernel = FxSmartKernel>
[[nodiscard]] constexpr auto fx_m(double value)
{
  using Decision = typename TKernel::template MagBitsDecision<TMagBits, true, TRaw>;
  auto v = TKernel::template MakeFromMagBits<TMagBits, true, TRaw>(value);
  return Fixed<typename Decision::Format, TKernel>::FromFxValue(v);
}

// fx_m_u<MagBits, Raw=void, Kernel=FxSmartKernel>(value)  (unsigned)
template <int TMagBits, typename TRaw = void, typename TKernel = FxSmartKernel>
[[nodiscard]] constexpr auto fx_m_u(double value)
{
  using Decision = typename TKernel::template MagBitsDecision<TMagBits, false, TRaw>;
  auto v = TKernel::template MakeFromMagBits<TMagBits, false, TRaw>(value);
  return Fixed<typename Decision::Format, TKernel>::FromFxValue(v);
}

// fx_mf<MagBits, FracBits, Raw=void, Kernel=FxSmartKernel>(value)  (signed)
template <int TMagBits, int TFracBits, typename TRaw = void, typename TKernel = FxSmartKernel>
[[nodiscard]] constexpr auto fx_mf(double value)
{
  using Decision = typename TKernel::template MagFracBitsDecision<TMagBits, TFracBits, true, TRaw>;
  auto v = TKernel::template MakeFromMagFracBits<TMagBits, TFracBits, true, TRaw>(value);
  return Fixed<typename Decision::Format, TKernel>::FromFxValue(v);
}

// fx_mf_u<MagBits, FracBits, Raw=void, Kernel=FxSmartKernel>(value)  (unsigned)
template <int TMagBits, int TFracBits, typename TRaw = void, typename TKernel = FxSmartKernel>
[[nodiscard]] constexpr auto fx_mf_u(double value)
{
  using Decision = typename TKernel::template MagFracBitsDecision<TMagBits, TFracBits, false, TRaw>;
  auto v = TKernel::template MakeFromMagFracBits<TMagBits, TFracBits, false, TRaw>(value);
  return Fixed<typename Decision::Format, TKernel>::FromFxValue(v);
}

// Default fx(value) (no template params). Provide conventional Q15.16 on int32.
[[nodiscard]] inline constexpr auto fx(double value)
{
  constexpr int FracBits = 16;
  using Decision = typename FxSmartKernel::template FracBitsDecision<FracBits, true, void>;
  auto v = FxSmartKernel::template MakeFromFracBits<FracBits, true, void>(value);
  return Fixed<typename Decision::Format, FxSmartKernel>::FromFxValue(v);
}

}  // namespace clarinoid
