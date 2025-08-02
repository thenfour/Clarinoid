// fpmac_q31.hpp ─────────────────────────────────────────────────────────────
#pragma once

#include <cstdint>
#include <type_traits>
#include <limits>

/*  Fixed-point helpers for signed Q1.31  (range ≈ [-1, 1-2-31])  */
/*  -----------------------------------------------------------------
    mul_q31(a,b)      : (a * b) >> 31           (with rounding)
    mac_q31(acc,a,b)  : acc + mul_q31(a,b)      (w/ sat to Q31)
    fma_q31(a,b,c)    : (a * b + c) >> 31       (1-cycle on M7)
   -----------------------------------------------------------------
*/

namespace fx {

// -------- detect CMSIS intrinsics ------------------------------------------
#if defined(__ARM_FEATURE_DSP) && (__CORTEX_M >= 3)
#define FX_HAS_ARM_DSP 1
#include <arm_math.h> // brings __SMMULR, __SMLAL*, __SSAT
#else
#define FX_HAS_ARM_DSP 0
#endif

// -------- signed saturate helper (portable) --------------------------------
[[nodiscard]] inline int32_t
sat_q31(int64_t x)
{
#if FX_HAS_ARM_DSP
  // __SSAT saturates to a signed N-bit range (N=31 => Q1.31)
  return __SSAT(x, 31);
#else
  constexpr int64_t minQ31 = static_cast<int64_t>(0x8000'0000LL);
  constexpr int64_t maxQ31 = static_cast<int64_t>(0x7FFF'FFFFLL);
  return static_cast<int32_t>(x < minQ31 ? minQ31 : (x > maxQ31 ? maxQ31 : x));
#endif
}

/*────────────────────────────────────────────────────────────────── mul_q31 */
[[nodiscard]] inline int32_t
mul_q31(int32_t a, int32_t b)
{
#if FX_HAS_ARM_DSP
  /*  __SMMULR  (Signed Most significant MULtiply with Rounding)
        r = (a * b + 0x4000'0000) >> 31
      – single cycle on Cortex-M4/M7.
  */
  return __SMMULR(a, b);
#else
  // 64-bit fallback: add 0x4000'0000 for rounding to nearest
  int64_t prod = static_cast<int64_t>(a) * b + 0x4000'0000LL;
  return static_cast<int32_t>(prod >> 31);
#endif
}

/*───────────────────────────────────────────────────────────────── mac_q31 */
[[nodiscard]] inline int32_t
mac_q31(int32_t acc, int32_t a, int32_t b)
{
#if FX_HAS_ARM_DSP
  /*  __SMLAL   (Signed MuLtiply Accumulate Long)
        64-bit_acc += a * b          (no rounding, full 64-bit result)
      We then >>31 with rounding and saturate.
  */
  int64_t s64 = 0;
  __SMLAL(&s64, &s64, a, b);              // GCC/Clang CMSIS form
  s64 += static_cast<int64_t>(acc) << 31; // align acc to 64-bit
  return sat_q31((s64 + 0x4000'0000LL) >> 31);
#else
  int64_t s64 = static_cast<int64_t>(a) * b + (static_cast<int64_t>(acc) << 31);
  return sat_q31((s64 + 0x4000'0000LL) >> 31);
#endif
}

/*────────────────────────────────────────────────────────────────── fma_q31
   Like mac_q31 but returns the product-and-sum already scaled.
   On Cortex-M7  it maps to  one  SMMLAR  (mul-hi-round + add).
*/
[[nodiscard]] inline int32_t
fma_q31(int32_t a, int32_t b, int32_t c)
{
#if FX_HAS_ARM_DSP
  /*  __SMMLAR   (Signed Most-significant MULtiply Accumulate w/ Rnd)
        r = ((a * b + 0x4000'0000) >> 31) + c
      One cycle.
  */
  return __SMMLAR(a, b, c);
#else
  int64_t prod = static_cast<int64_t>(a) * b + 0x4000'0000LL;
  int32_t mul_r = static_cast<int32_t>(prod >> 31);
  return sat_q31(static_cast<int64_t>(mul_r) + c);
#endif
}


// lerp: y = (1−t)*a + t*b
inline int32_t
lerp_q31(int32_t a, int32_t b, int32_t t)
{
  // t is Q1.31 in [0,1)
  int32_t diff = b - a;           // Q1.31
  return fx::fma_q31(diff, t, a); // SMMLAR diff*t + a   (1 cycle)
}

// bilerp: classic “two lerps then one more”
inline int32_t
bilerp_q31(int32_t f00, int32_t f10, int32_t f01, int32_t f11, int32_t tx, int32_t ty)
{
  int32_t a = lerp_q31(f00, f10, tx); // row 0
  int32_t b = lerp_q31(f01, f11, tx); // row 1
  return lerp_q31(a, b, ty);          // interpolate between rows
}




} // namespace fx
