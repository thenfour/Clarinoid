#pragma once

#include <type_traits>

// mul
// div
// reciprocal
// modulo

// wrap
// fold

// floor
// ceil
// fract part
// int part
// round
   
// shift
// saturating left shift
// rotate

// abs
// negate
// make signed
// make unsigned (remove sign bit)
// get sign
   
// add
// sub

// min
// max

// clamp
// signed saturate (ssat)
// unsigned saturate (usat)

// lerp
// step
// smoothstep
// smootherstep
// other DSP interpolation functions
// bilinear interp
// reverse lerp
// map
// smooth min
// smooth max (log, exp, ..)

// hash
// rand
   
// sqrt
// pow
// exp
// log

// sinh
// cosh
// tanh
// sin
// cos
// tan
// asin
// acos
// atan

// modcurve
// massive's freq knob math

// >
// >=
// <
// <=
// ==(exact)
// ==(approx)
// !=

// erf https://en.wikipedia.org/wiki/Error_function
// inverseerf
// lgamma
// digamma
// lambertw
// lambertwexpx
// sigmoid -> exp



//| Category                          | Commonly-needed ops that aren’t on your list                                                                                                                                      | Why they tend to show up                                                                                                              |
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


namespace clarinoid {

// int bits and fract bits are NOT expected to sum to the storage bits of the underlying type.
// we expect much of the time there is "unused overhead" in the storage, which lets operations like multiplication
// make better decisions about shifting the result.

// for clarity as i develop, i'm separating layout / storage / format / value, but some could possibly be combined.
// describes the layout of the fixed point type
template<int TIntBits, int TFracBits>
struct FxLayout
{
  static_assert(I >= 0 && F >= 0, "negative bit count");
  static constexpr IntBits = TIntBits;
  static constexpr FracBits = TFracBits;
  static constexpr ValueBits = TIntBits + TFracBits;
};

// describes the raw value type's capabilities
template<typename TRaw>
struct FxStorage
{
  static constexpr bool IsSigned = std::is_signed<TRaw>::value;
  static constexpr bool SignBits = IsSigned ? 1 : 0;
  static constexpr int StorageWidthBits = sizeof(TRaw) * 8;
  static constexpr int StorageValueBits = StorageWidthBits - SignBits;
};

// describes the binary format (layout + type), which is a complete description of the fixed point type.
template<typename TLayout, typename TStorage>
struct FxFormat
{
  static_assert(std::is_arithmetic_v<TStorage>, "TStorage must be an arithmetic type (e.g. int32_t, uint8_t, etc.)");
  using LayoutType = TLayout;
  using StorageType = TStorage;

  using TLayout::TIntBits;
  using TLayout::TFracBits;
  using TLayout::ValueBits;

  using TRaw::IsSigned;
  using TRaw::SignBits;
  using TRaw::StorageWidthBits;
  using TRaw::StorageValueBits;
};

// based on format, actually stores the value and could maybe provide some primitive operations.
template<typename TFormat>
struct FxValue
{
  using RawType = TFormat::StorageType;
  RawType mValue;

  // possibly provide primitve ops here which don't make any decisions about format?
};

// defines behavior
struct FxPrototypeKernel
{
    // type helpers

  // value format could for example return a type that holds a `float` for debugging purposes.
  // or it could refuse to return types that are not compatible with the format.
  template<typename TFixedFormat>
  using ValueType = FxValue<TFixedFormat>;

  template<typename TFormatA, typename TFormatB>
  using MulResultFormat =
    // todo:
    // - add sign if needed
    // - based on the fx layout, promotion may not be necessary.
    // - or demotion from 64 to 32-bits might be a possibility, and take it.
    FxFormat<typename TFormatA::LayoutType, typename TFormatA::StorageType>;
  ;

  // todo: construction
  template<typename TFormatDesired, typename TFormatExisting>
  [[nodiscard]] constexpr auto static ConstructFromFixed(const TFormat::StorageType& rhsRaw)
  {
    return StorageType{};
  }

  // todo: construction
  template<typename TFormat>
  [[nodiscard]] constexpr auto static ConstructFromFloat(const TFormat::StorageType& rhsRaw)
  {
    return StorageType{};
  }

  // multiplication
  // in theory, could decide to use a SMMUL intrinsic, or perform strategic shifting to retain within a 32-bit type, ...
  template<typename TFormatA, typename TFormatB>
  [[nodiscard]] constexpr auto static mul(const FxValue<TFormatA>& a, const FxValue<TFormatB>& b) const
  {
    using ResultFormat = MulResultFormat<TFormatA, TFormatB>;
    return ResultFormat::ConstructFrom(a.mValue * b.mValue);
  }
};

template<typename TFormat, typename TKernel>
struct Fixed
{
  using FormatType = TFormat;
  using KernelType = TKernel;

  // raw type is specified explicitly by caller.
  using ValueType = TKernel::template ValueType<TFormat>;
  ValueType mStore;

  [[nodiscard]] constexpr RawValue() const { return mStore.mValue; }

  // constructors
  // from float or int values
  // from raw
  // from Fixed<>

  // to float
  // to Fixed<> (so Q31 x = Q31(0.5f) * 3 + 4 / 10; converts directly back to Q31, hiding all intermediate types)

  // add
  // sub
  // mul
  // div

  // negate
  // reciprocal
  // abs


  template<typename TFormatB, typename TKernelB>
  [[nodiscard]] constexpr auto MultipliedWith(const Fixed<TFormatB, TKernelB>& rhs) const
  {
    return TKernel::template mul<TIntBits, TFracBits, TIntBitsB, TFracBitsB>(RawValue(), rhs.RawValue());
  }
};

} // namespace clarinoid
