
// **recip (Q32 → Q32)**

// | Variant  | Speed (cycles) | Speed (graph)          | ULP max    | ULP max (graph)        | ULP mean       | ULP
// mean (graph)       | ULP rms        | ULP rms (graph)        | Rel max (ppm) | Rel max (graph)        | Rel mean
// (ppm) | Rel mean (graph)       | Rel rms (ppm) | Rel rms (graph)        |
// |:---------| --------------:|:-----------------------| ----------:|:-----------------------|
// --------------:|:-----------------------| --------------:|:-----------------------|
// -------------:|:-----------------------| --------------:|:-----------------------|
// -------------:|:-----------------------| | 1-Newton |          26.50 | `=================   ` | 4294967295 |
// `====================` | 4277109694.965 | `====================` | 4277200991.458 | `====================` | 1000000
// | `====================` |     995872.463 | `====================` |    995878.527 | `====================` | |
// 2-Newton |          31.50 | `====================` | 4294967295 | `====================` | 4294805956.058 |
// `====================` | 4294871478.227 | `====================` |       1000000 | `====================` |
// 999992.946 | `====================` |    999992.946 | `====================` |

// **recip**

// | Variant     | Speed (cycles) | Speed (graph)          | Rel max (ppm) | Rel max (graph)        | Rel mean (ppm) |
// Rel mean (graph)       | Rel rms (ppm) | Rel rms (graph)        |
// |:------------| --------------:|:-----------------------| -------------:|:-----------------------|
// --------------:|:-----------------------| -------------:|:-----------------------| | recip_base  |          55.50 |
// `============        ` |             0 | `                    ` |          0.000 | `                    ` | 0.000 | `
// ` | | recip_fast1 |          90.88 | `=================== ` |       1000000 | `====================` |    1000000.000
// | `====================` |   1000000.000 | `====================` | | recip_fast2 |          95.88 |
// `====================` |       1000000 | `====================` |    1000000.000 | `====================` |
// 1000000.000 | `====================` |

// so here FX is 1/2 the processing of the float variation. but i:
// - haven't verified what's up with the reported precision issue (false alarm? edge case? totally messed up?)
// - still haven't understood the input vs. output fixed point layouts.

#include <math.h>

#pragma once

// let's use this as a way to make input & output fixed formats more explicit
// int bits and integer bits are NOT expected to add up to the value digits of T;
// they can be smaller, leaving "unused overhead" in storage which helps subsequent ops
// know how best to shift and retain precision.
//
// so for example the value -3.5 would probably be stored as s2.1
// using this struct, functions can return a value that also specifies its layout.
template<int TIntBits, int TFractBits, typename T>
struct Fx
{
  static_assert(TIntBits + TFractBits <= int(sizeof(T) * 8), "Layout does not fit into underlying type");

  using RawType = T;

  static constexpr int kIntBits = TIntBits;
  static constexpr int kFractBits = TFractBits;
  static constexpr RawType kScale = RawType(1) << TFractBits;

  RawType mRawValue{ 0 };

  /*--------- ctors ---------------------------------------------------*/
  constexpr Fx() = default;

  // from raw storage (no scaling)
  static constexpr Fx fromRaw(RawType v)
  {
    Fx r;
    r.mRawValue = v;
    return r;
  }

  // implicit from raw value so that existing arithmetic still works
  constexpr explicit Fx(RawType v)
    : mRawValue(v)
  {
  }

  // from floating-point
  template<typename TF, typename = std::enable_if_t<std::is_floating_point_v<TF>>>
  constexpr explicit Fx(TF f)
    : mRawValue(static_cast<RawType>(std::round(f * kScale)))
  {
  }

  /*--------- conversions ---------------------------------------------*/
  template<typename TF = float>
  constexpr TF toFloat() const
  {
    return TF(mRawValue) / TF(kScale);
  }

  constexpr RawType raw() const { return mRawValue; }
};

//   index = top 8 bits of the mantissa in [0.5 .. 1)
//   value = round( 2^32 / mantissa )  → Q0.32 reciprocal seed
// static constexpr uint32_t recip8_LUT[256] = {
//   // this needs to be generated.
// };

static constexpr uint32_t
make_seed(uint32_t i)
{
  uint32_t d = 0x80u + i;                      // denominator
  uint64_t n = (uint64_t(1) << 40) + (d >> 1); // 2^40 + d/2  : add d/2 for rounding
  return uint32_t(n / d);                      // Q0.32 seed
}
template<size_t... I>
constexpr auto
make_table(std::index_sequence<I...>) -> std::array<uint32_t, 256>
{
  return { make_seed(I)... };
}

static constexpr std::array<uint32_t, 256> recip8_LUT_arr = make_table(std::make_index_sequence<256>{});

static constexpr const uint32_t* recip8_LUT = recip8_LUT_arr.data();

// // returns  ⌈32/F⌉ fractional bits  (same as sqrt rule)
// // unsigned.
// // now takes an explicit fixed-point operand
// template<int I, int F, bool twoIterations = true>
// static inline uint32_t
// recip_uq(Fx<I, F, uint32_t> Xfx)
// {
//   uint32_t X = Xfx.raw();
//   if (X == 0)
//     return 0xFFFFFFFFu;               // saturate 1/0 -> max

//   constexpr bool oddF = F & 1;

//   /* --- normalise -------------------------------------------------- */
//   unsigned lz    = clz32(X);
//   unsigned shift = lz - (oddF ? 1u : 0u);   // mantissa → [0.5,1)
//   uint32_t a     = X << shift;              // Q0.32
//   uint32_t idx   = a >> 24;                 // top-8 frac bits
//   uint32_t x     = recip8_LUT[idx];         // Q0.32 seed

//   /* --- Newton ----------------------------------------------------- */
//   auto step = [&](uint32_t x0) -> uint32_t {
//       uint32_t t = smmulr(a, x0);           // Q0.32
//       t = 0xFFFFFFFFu - t;                  // (2 – a*x)
//       return smmulr(x0, t);                 // Q0.32
//   };
//   x = step(x);                              // 1st iter
//   if constexpr (twoIterations)
//       x = step(x);                          // 2nd iter

//   /* --- denormalise ------------------------------------------------ */
//   unsigned outShift = oddF ? (shift + 1u) : shift;
//   return x >> outShift;                     // result in Q⌈32/F⌉
// }
// // signed.
// // TODO: input Fx<> and output Fx<> to make it more explicit
// template<int F>
// uint32_t
// recip_qs(int32_t Xin)
// {
//   if (Xin == 0)
//     return 0x7FFFFFFFu; // saturate
//   uint32_t mag = Xin < 0 ? -Xin : Xin;
//   uint32_t r = recip_uq<F>(mag);
//   return (Xin < 0) ? -int32_t(r) : r;
// }

template<int A, int B>
inline constexpr int CeilDiv_v = (A + B - 1) / B;

template<int I, int F, bool twoIterations = true>
static inline auto
recip_uq(Fx<I, F, uint32_t> Xin) -> Fx</* integer bits  */ CeilDiv_v<F, 32> + I,
                                       /* fractional bits*/ 32 - CeilDiv_v<F, 32>,
                                       uint32_t>
{
  using OutFx = Fx<CeilDiv_v<F, 32> + I, 32 - CeilDiv_v<F, 32>, uint32_t>;

  uint32_t X = Xin.raw();
  if (X == 0)
    return OutFx::fromRaw(0xFFFFFFFFu); // 1/0  →  max

  constexpr bool oddF = F & 1;

  /* ---------- normalise :  bring mantissa to 0.5 … 1 -------------- */
  unsigned lz = clz32(X);               // 0…32
  int shift = static_cast<int>(lz) - 1; // left  (+) or right (−)

  uint32_t a = (shift >= 0) ? (X << shift) : (X >> -shift); // Q0.32 mantissa

  /* ---------- LUT seed + Newton iterations ------------------------ */
  uint32_t idx = a >> 24;       // top-8 frac bits
  uint32_t x = recip8_LUT[idx]; // Q0.32 seed

  auto step = [&](uint32_t x0) -> uint32_t {
    uint32_t t = smmulr(a, x0); // Q0.32
    t = 0xFFFF'FFFFu - t;       // (2 – a*x0)
    return smmulr(x0, t);       // Q0.32
  };
  x = step(x);
  if constexpr (twoIterations)
    x = step(x);

  /* ---------- de-normalise ---------------------------------------- */
  // want:  1/X = (x / 2^32) * 2^shift
  if (shift >= 0)
    x >>= (oddF ? shift + 1 : shift); // small input  → right shift
  else
    x <<= (-shift); // large input  → left  shift

  return OutFx::fromRaw(x);
}

template<int I, int F, bool twoIterations = true>
static inline auto
recip_qs(Fx<I, F, int32_t> Xin) -> Fx</* integer bits  */ CeilDiv_v<F, 32> + I,
                                      /* fractional bits*/ 32 - CeilDiv_v<F, 32>,
                                      int32_t>
{
  using OutFx = Fx<CeilDiv_v<F, 32> + I, 32 - CeilDiv_v<F, 32>, int32_t>;

  int32_t sX = Xin.raw();
  if (sX == 0)
    return OutFx::fromRaw(0x7FFFFFFF); // 1/0  →  saturate

  bool neg = sX < 0; // remember the sign
  uint32_t mag = neg ? uint32_t(-sX) : uint32_t(sX);

  // reuse unsigned core
  auto rMag = recip_uq<I, F, twoIterations>(Fx<I, F, uint32_t>::fromRaw(mag));

  int32_t outRaw = neg ? -int32_t(rMag.raw()) // restore sign
                       : int32_t(rMag.raw());

  return OutFx::fromRaw(outRaw);
}

static float
recip_float_baseline(float x)
{
  return 1.0f / x;
}

template<int I, int TFractBits>
static inline auto
recip_qs_floatassist(Fx<I, TFractBits, int32_t> Xin) -> Fx<CeilDiv_v<TFractBits, 32> + I, 32 - CeilDiv_v<TFractBits, 32>, int32_t>
{
  using OutFx = Fx<CeilDiv_v<TFractBits, 32> + I, 32 - CeilDiv_v<TFractBits, 32>, int32_t>;

  float fin = Xin.toFloat();
  float fout = 1.0f / fin; // slow but very accurate
  return OutFx(fout);      // ctor from float
}

using QS_IN_Q0_31 = Fx<0, 31, int32_t>; // input layout (signed)
using QS_OUT_TYP = decltype(recip_qs<QS_IN_Q0_31::kIntBits, QS_IN_Q0_31::kFractBits>(QS_IN_Q0_31{}));

static int32_t
recip_q31_newton1_wrap(int32_t xRaw)
{
  auto r = recip_qs<0, 31, false>(QS_IN_Q0_31::fromRaw(xRaw));
  return r.raw();
}

static int32_t
recip_q31_newton2_wrap(int32_t xRaw)
{
  auto r = recip_qs<0, 31, true>(QS_IN_Q0_31::fromRaw(xRaw));
  return r.raw();
}

static int32_t
recip_q31_floatassist_wrap(int32_t xRaw)
{
  auto r = recip_qs_floatassist<0, 31>(QS_IN_Q0_31::fromRaw(xRaw));
  return r.raw();
}

// TODO: create functions to operate on various:
// - fixed-point layouts
// - 1 iteration vs. 2 iterations
// - signed / unsigned
// And a float-assisted baseline (to address the question: can we make fixed point implementation that's actually better
// than float?)

static const VariantUQF kRecipQ32Variants[] = {
  { "1-Newton", &recip_q31_newton1_wrap },
  { "2-Newton", &recip_q31_newton2_wrap },
  { "float-assist", &recip_q31_floatassist_wrap },
};

static const VariantFloat kRecipFVariants[] = {
  { "recip_float_baseline", &recip_float_baseline },
};
