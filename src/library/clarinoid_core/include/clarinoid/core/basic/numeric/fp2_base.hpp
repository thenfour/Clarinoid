#pragma once

#include <type_traits>


#pragma once

#include <cassert>
#include <limits>
#include <type_traits>

namespace clarinoid
{
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Computes the absolute value of a compile-time integer constant.
/// </summary>
/// <typeparam name="i">The integer value whose absolute value is to be computed.</typeparam>
template <int64_t i>
struct StaticAbs
{
  static constexpr int64_t value = i < 0 ? -i : i;
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Computes the number of bits required to represent a static integer value at compile time.
/// </summary>
/// <typeparam name="i">The integer value for which to compute the number of bits needed.</typeparam>
template <int64_t i>
struct StaticValueBitsNeeded
{
  static constexpr int64_t value_allow_zero = 1 + StaticValueBitsNeeded<(StaticAbs<i>::value >> 1)>::value_allow_zero;
  static constexpr int64_t value = value_allow_zero;
};

template <>
struct StaticValueBitsNeeded<0>
{
  static constexpr int64_t value = 1;
  static constexpr int64_t value_allow_zero = 0;
};

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

// Helper to select optimal raw type based on requirements
template <bool TWantsSign, int TIntBits, int TFracBits>
struct SelectOptimalRawType
{
  static constexpr int SignBits = TWantsSign ? 1 : 0;
  static constexpr int TotalBitsNeeded = SignBits + TIntBits + TFracBits;

  using type =
      std::conditional_t<(TotalBitsNeeded <= 8),
                         std::conditional_t<TWantsSign, int8_t, uint8_t>,
                         std::conditional_t<(TotalBitsNeeded <= 16),
                                            std::conditional_t<TWantsSign, int16_t, uint16_t>,
                                            std::conditional_t<(TotalBitsNeeded <= 32),
                                                               std::conditional_t<TWantsSign, int32_t, uint32_t>,
                                                               std::conditional_t<TWantsSign, int64_t, uint64_t>>>>;
};

// Type alias for the helper struct result
template <bool TWantsSign, int TIntBits, int TFracBits>
using OptimalRawType_t = typename SelectOptimalRawType<TWantsSign, TIntBits, TFracBits>::type;

// Terminology:
// - IntBits: number of magnitude bits (excluding sign bit)
// - FracBits: number of fractional bits
// - SignBits: 1 for signed, 0 for unsigned
// - HeadroomBits: unused bits in MSBs for overflow protection
// - StorageBits = SignBits + HeadroomBits + IntBits + FracBits

// Describes the semantic layout of the fixed point type
template <int TIntBits, int TFracBits>
struct FxLayout
{
  static_assert(TIntBits >= 0 && TFracBits >= 0, "negative bit count");
  static constexpr int IntBits = TIntBits;
  static constexpr int FracBits = TFracBits;
  static constexpr int ValueBits = TIntBits + TFracBits;  // semantic payload (excluding sign)
};
//| --------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------- |
//| **DSP “inner-loop” kernels**      | **• FMA / MAC (`y = a·b + c`)**<br>**• Multiply-accumulate with rounding & saturation**<br>**• Dot / sum-of-products helpers**                                                    | Every biquad, FIR, FFT, and envelope follower is built on “multiply then add”; a fused path saves one rounding + one saturation.      |
//| **Format / range utilities**      | **• Convert `Fixed<A>` → `Fixed<B>`** (rescale & saturate)<br>**• Left-shift with saturation (`q = sat_shift_left(x,n)`)**<br>**• Count-leading-zeros / count-leading-sign bits** | Converting between Q-formats is unavoidable at module boundaries; CLZ is the fastest way to auto-pick a safe shift.                   |
//| **Bitwise & masking**             | **• AND / OR / XOR / NOT** (on the raw value)<br>**• Bit-field extract / set**                                                                                                    | Needed for flag manipulation, packing RGBA colours, checksum parity, etc.                                                             |
//| **Comparisons / predicates**      | **• `signbit` (returns −1,0,+1)**<br>**• `hypot(x,y)`** (length for 2-D vectors)<br>**• `atan2(y,x)`**                                                                            | UI knob rendering and vector graphics both rely on `atan2`; `hypot` and `signbit` remove a branch each in dynamics processing.        |
//| **Extra transcendental variants** | **• `exp2`, `exp10`, `log2`, `log10` (direct)**<br>**• `pow2^k` (integer exponent fast path)**                                                                                    | `exp2`/`log2` often map to simple LUT-plus-poly; integer-power fast path is handy for envelope generators and sample-rate convertors. |
//| **Interpolation / smoothing**     | **• Cubic-Hermite / 3rd-order Lagrange**<br>**• B-spline (catmull-rom)**<br>**• Exponential-smoother (`y += α·(target−y)`) helper**                                               | Linear/smoothstep are fine for UI fade-ins; audio rate interpolation generally needs cubic or better to keep the noise floor down.    |
//| **Coordinate helpers (UI)**       | **• Degrees↔radians**<br>**• `wrap(angle, ±π)` / `fold(value, min,max)`**                                                                                                         | UI widgets almost always store angles in degrees even when DSP runs radians.                                                          |
//| **Decibel utilities (audio)**     | **• `lin_to_db(x)` and `db_to_lin(x)`**<br>**• `rms(x[])`**                                                                                                                       | 20·log10 and RMS are the backbone of meters and limiters.                                                                             |
//| **Random / noise**                | **• Uniform LCG / XOR-shift**<br>**• White & Pink noise accumulators**                                                                                                            | Synth LFOs, UI anim jitter, dithering, and test rigs all want a quick RNG.                                                            |
//| **Math-policy variants**          | **• Exact vs. nearest vs. stochastic rounding** for every primitive<br>**• Wrap vs. sat vs. trap overflow flavours for `add`, `sub`, `mul`**                                      | You already plan `ssat/usat`; expose the full matrix once so callers don’t reinvent.                                                  |
//

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
  static constexpr int IntBits = TLayout::IntBits;
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
  static_assert((SignBits + HeadroomBits + IntBits + FracBits) == StorageWidthBits, "Total bits is not adding up...");

  // Useful constants (RawOne moved to kernels to allow different policies)
  static constexpr RawType RawMax = ((RawType(1) << (IntBits + FracBits)) - 1);
  static constexpr RawType RawMin = IsSigned ? -RawMax - 1 : RawType(0);
};

// Simple value holder - stores the raw value and provides minimal operations
template <typename TFormat>
struct FxValue
{
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

// Forward declarations for policies
struct RoundingPolicy_Truncate;
struct OverflowPolicy_Wrap;
struct PromotionPolicy_LeftOperand;


}  // namespace clarinoid
