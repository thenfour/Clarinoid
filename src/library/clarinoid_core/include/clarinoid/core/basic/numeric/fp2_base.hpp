#pragma once

#include <type_traits>


#pragma once

#include <cassert>
#include <limits>
#include <type_traits>

namespace clarinoid
{
template <typename T>
constexpr int needed_int_bits(T value)
{
  if (value == 0)
    return 1;
  int bits = 0;
  while (value)
  {
    value >>= 1;
    ++bits;
  }
  return bits;
}

constexpr std::uint64_t pow10_u64(unsigned p)
{
  return p > 19 ? std::uint64_t{0} : (p == 0 ? 1 : pow10_u64(p - 1) * 10);
}


// Reduce fraction by GCD (constexpr)
static constexpr std::uint64_t gcd_u64(std::uint64_t a, std::uint64_t b)
{
  while (b)
  {
    auto t = a % b;
    a = b;
    b = t;
  }
  return a;
}

// Add or subtract with modulo-2^N wraparound, where N = digits(ResRaw).
// This deliberately uses unsigned arithmetic to avoid UB from signed overflow,
// keeping the operation valid in constexpr contexts.
// Note: This is NOT saturating arithmetic.
// basically this is for adding / subtracting and emulating overflow behavior.
template <typename ResRaw, bool IsSub>
constexpr ResRaw add_or_sub_mod2n(ResRaw a, ResRaw b)
{
  using U = std::make_unsigned_t<ResRaw>;
  U ua = static_cast<U>(a);
  U ub = static_cast<U>(b);

  if constexpr (IsSub)
  {
    return static_cast<ResRaw>(ua - ub);  // well-defined modulo 2^N
  }
  else
  {
    return static_cast<ResRaw>(ua + ub);  // well-defined modulo 2^N
  }
}


// Terminology:
// - MagBits: number of magnitude bits (excluding sign bit)
// - FracBits: number of fractional bits
// - SignBits: 1 for signed, 0 for unsigned
// - HeadroomBits: unused bits in MSBs for overflow protection
// - StorageBits = SignBits + HeadroomBits + MagBits + FracBits

// Describes the semantic layout of the fixed point type
template <int TMagBits, int TFracBits>
struct FxLayout
{
  static_assert(TMagBits >= 0 && TFracBits >= 0, "negative bit count");
  static constexpr int MagBits = TMagBits;
  static constexpr int FracBits = TFracBits;
  static constexpr int ValueBits = MagBits + FracBits;  // semantic payload (excluding sign)
};

// Describes the raw storage type's capabilities
template <typename TRaw>
struct FxStorageTraits
{
  static_assert(std::is_arithmetic_v<TRaw>, "TRaw must be an arithmetic type");
  static constexpr bool IsSigned = std::is_signed_v<TRaw>;
  static constexpr int StorageWidthBits = sizeof(TRaw) * 8;
  static constexpr int StorageValueBits = std::numeric_limits<TRaw>::digits;
  using RawType = TRaw;
};

// Describes the binary format (layout + storage), which is a complete description of the fixed point type.
template <typename TLayout, typename TStorageTraits>
struct FxFormat
{
  using LayoutType = TLayout;
  using StorageTraitsType = TStorageTraits;
  using RawType = typename TStorageTraits::RawType;

  // Layout properties
  static constexpr int MagBits = TLayout::MagBits;
  static constexpr int FracBits = TLayout::FracBits;
  static constexpr int ValueBits = TLayout::ValueBits;

  // Storage properties
  static constexpr bool IsSigned = TStorageTraits::IsSigned;
  static constexpr int StorageWidthBits = TStorageTraits::StorageWidthBits;
  static constexpr int StorageValueBits = TStorageTraits::StorageValueBits;

  // Derived properties
  static constexpr int SignBits = IsSigned ? 1 : 0;
  static constexpr int HeadroomBits = StorageValueBits - ValueBits;

  // Safety checks
  static_assert(ValueBits > 0, "ValueBits must be positive");
  static_assert(HeadroomBits >= 0, "Not enough storage bits for the requested format");
  static_assert((SignBits + HeadroomBits + MagBits + FracBits) == StorageWidthBits, "Total bits is not adding up...");

  // Useful constants (RawOne moved to kernels to allow different policies)
  static constexpr RawType RawMax = ((1ULL << (MagBits + FracBits)) - 1);
  static constexpr RawType RawMin = IsSigned ? -RawMax - 1 : RawType(0);
};

// Simple value holder - stores the raw value and provides minimal operations
template <typename TFormat>
struct FxValue
{
  using FormatType = TFormat;
  using RawType = typename TFormat::RawType;
  RawType mRawValue;

  // Construction from raw value (explicit to avoid accidents)
  static constexpr FxValue FromRaw(RawType raw)
  {
    return FxValue{raw};
  }

  constexpr FxValue()
      : mRawValue(0)
  {
  }
  constexpr explicit FxValue(RawType raw)
      : mRawValue(raw)
  {
  }
};

// constexpr shifting
template <typename T, int S>
constexpr T shl_safe(T x)
{
  static_assert(S >= 0, "shift must be non-negative");
  if constexpr (S == 0)
    return x;
  using U = std::make_unsigned_t<T>;
  static_assert(S < std::numeric_limits<U>::digits, "shift too large");
  return static_cast<T>(static_cast<U>(x) << S);
}

template <typename T, int S>
constexpr T sar_safe(T x)
{
  static_assert(S >= 0, "shift must be non-negative");
  if constexpr (S == 0)
    return x;
  using U = std::make_unsigned_t<T>;
  constexpr int W = std::numeric_limits<U>::digits;
  static_assert(S < W, "shift too large");

  if constexpr (std::is_unsigned<T>::value)
  {
    return static_cast<T>(static_cast<U>(x) >> S);
  }
  else
  {
    U ux = static_cast<U>(x);  // two's-complement representation
    U shifted = ux >> S;       // logical shift
    // If negative, sign-extend the vacated high bits:
    if (ux >> (W - 1))
    {
      shifted |= (~U{0}) << (W - S);
    }
    return static_cast<T>(shifted);
  }
}

template <typename A0, typename B0>
struct WiderOf
{
  static_assert(std::is_integral<A0>::value && std::is_integral<B0>::value,
                "WidestType requires integral (or enum) types");
  // value-capable bits (sign bit does NOT count)
  static constexpr int da = std::numeric_limits<A0>::digits;
  static constexpr int db = std::numeric_limits<B0>::digits;
  using type = std::conditional_t<(da >= db), A0, B0>;
};

template <typename... TRaw>
struct WidestType;

template <typename T>
struct WidestType<T>
{
  using type = T;
  static_assert(std::is_integral<type>::value, "WidestType requires integral (or enum) types");
};

template <typename T0, typename T1, typename... Ts>
struct WidestType<T0, T1, Ts...>
{
  using type = typename WidestType<typename WiderOf<T0, T1>::type, Ts...>::type;
};

// nice alias
template <typename... Ts>
using WidestType_t = typename WidestType<Ts...>::type;


template <typename...>
struct _dependent_false : std::false_type
{
};

// Utility: pick first provided raw type whose (signed-adjusted) digits >= NeededBits.
// signedness of provided types is ignored; we force Signed via make_signed/make_unsigned.
template <bool Signed, int NeededBits, typename... Ts>
struct PickUsableType;

template <bool Signed, int NeededBits>
struct PickUsableType<Signed, NeededBits>
{
  static_assert(!_dependent_false<std::integral_constant<bool, Signed>>::value,
                "PickUsableType: no provided type has enough value bits");
  using type = void;
};

template <bool Signed, int NeededBits, typename T0, typename... Rest>
struct PickUsableType<Signed, NeededBits, T0, Rest...>
{
  static_assert(NeededBits >= 0, "PickUsableType: NeededBits must be >= 0");

  using Adj0 = std::conditional_t<Signed, std::make_signed_t<T0>, std::make_unsigned_t<T0>>;
  static constexpr int d0 = std::numeric_limits<Adj0>::digits;

  using type = std::conditional_t<(d0 >= NeededBits), Adj0, typename PickUsableType<Signed, NeededBits, Rest...>::type>;
};

// nice alias
template <bool Signed, int NeededBits, typename... Ts>
using PickUsableType_t = typename PickUsableType<Signed, NeededBits, Ts...>::type;


// calculates a format with a narrower storage, if there's headroom to do so.
// Caller supplies allowed raw (unsigned) storage types in order of desirability.
// We pick the first that can hold Needed bits; if it is narrower than original, we use it; else keep original.
template <typename TFormat, typename... TAllowedRaw>
struct NarrowedFormat
{
  static_assert(sizeof...(TAllowedRaw) > 0, "Provide at least one allowed raw type");
  using OrigRaw = typename TFormat::RawType;
  static constexpr bool Signed = TFormat::IsSigned;
  static constexpr int Needed = TFormat::ValueBits;
  static constexpr int OrigDigits = std::numeric_limits<OrigRaw>::digits;

  using CandidateRaw = typename PickUsableType<Signed, Needed, TAllowedRaw...>::type;
  static constexpr int CandDigits = std::numeric_limits<CandidateRaw>::digits;
  using RawTypeToUse = std::conditional_t<(CandDigits < OrigDigits), CandidateRaw, OrigRaw>;
  using FormatType = FxFormat<FxLayout<TFormat::MagBits, TFormat::FracBits>, FxStorageTraits<RawTypeToUse>>;
};


//if the requested layout doesn't fit into the storage type,
// truncate the fractional bits first, then the magnitude bits.
template <typename RawType, int IdealMagBits, int IdealFractBits>
struct SqueezedFormat
{
  static_assert(IdealMagBits >= 0 && IdealFractBits >= 0, "Negative bit counts");
  using StorageTraits = FxStorageTraits<RawType>;
  static constexpr int Capacity = StorageTraits::StorageValueBits;  // value bits available (excludes sign)
  static constexpr int IdealTotal = IdealMagBits + IdealFractBits;

  // Start with ideals
  static constexpr int _initialMag = IdealMagBits;
  static constexpr int _initialFrac = IdealFractBits;

  // If it fits, keep as-is
  static constexpr bool Fits = (IdealTotal <= Capacity);

  // Compute squeezed values
  static constexpr int Overflow = Fits ? 0 : (IdealTotal - Capacity);

  // Remove overflow from fractional bits first
  static constexpr int FracAfterFirstTrim = Fits ? _initialFrac
                                                 : (_initialFrac > Overflow ? _initialFrac - Overflow : 0);
  static constexpr int ConsumedFromFrac = Fits ? 0 : (_initialFrac - FracAfterFirstTrim);
  static constexpr int RemainingOverflow = Fits ? 0 : (Overflow - ConsumedFromFrac);

  // Then, if still overflow, remove from mag bits
  static constexpr int MagAfterTrim = Fits ? _initialMag
                                           : (_initialMag > RemainingOverflow ? _initialMag - RemainingOverflow : 0);

  // Guard against both becoming zero (FxFormat requires ValueBits>0). If both zero but capacity>0, force 1 mag bit.
  static constexpr bool BothZero = (MagAfterTrim == 0) && (FracAfterFirstTrim == 0);
  static constexpr int FinalMag = BothZero ? (Capacity > 0 ? 1 : 0) : MagAfterTrim;
  static constexpr int FinalFrac = BothZero ? 0 : FracAfterFirstTrim;

  using Layout = FxLayout<FinalMag, FinalFrac>;
  using FormatType = FxFormat<Layout, StorageTraits>;
};


}  // namespace clarinoid
