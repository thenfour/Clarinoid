
#pragma once

#include <clarinoid/basic/Teensy.hpp>
#include <cstdint>

#include "harnessTypes.hpp"

/*========================= Your sqrt candidates (examples) =========================*/
// Heron NR (Q32->Q16), lean table (31 entries) and early exit
static constexpr uint16_t sqrt_integer_guess_table[31] = {55109, 38968, 27555, 19484, 13778, 9742, 6889, 4871,
                                                          3445,  2436,  1723,  1218,  862,   609,  431,  305,
                                                          216,   153,   108,   77,    54,    39,   27,   20,
                                                          14,    10,    7,     5,     4,     3,    2};

static inline uint32_t sqrt_Q32_to_Q16_NR(uint32_t in)
{
  if (in <= 1u)
    return in;  // 0->0, 1->1
  uint32_t n = sqrt_integer_guess_table[clarinoid::Teensy::clz32(in)];
  uint32_t q = in / n;
  n = (q + n) >> 1;
  q = in / n;
  n = (q + n) >> 1;

  // exact floor -- maybe ~10 cycles extra for slightly more precision (nearly like reference impl)
  // uint64_t n2 = (uint64_t)n * n;
  // if (n2 > in) { while ((uint64_t)(n-1)*(n-1) >= in) --n; }
  // else         { while ((uint64_t)(n+1)*(n+1) <=  in) ++n; }

  // nearest rounding
  uint64_t low = (uint64_t)n * n;
  uint64_t high = (uint64_t)(n + 1) * (n + 1);
  if (in > low + ((high - low) >> 1))
    ++n;
  return n;
}

// Restoring kernel generalized for QF -> Qceil(F/2)
template <int F>
static inline uint32_t sqrt_qF_restoring(uint32_t X)
{
  if (!X)
    return 0;
  constexpr bool odd = (F & 1) != 0;
  using U = uint32_t;
  using W = uint64_t;
  W Wv = odd ? (W)X << 1 : (W)X;

  // isqrt_restoring
  auto isqrt_rest = [](W x) -> U
  {
    if (!x)
      return 0;
    W bit = W(1) << (std::numeric_limits<W>::digits - 2);
    while (bit > x)
      bit >>= 2;
    W res = 0;
    while (bit)
    {
      if (x >= res + bit)
      {
        x -= res + bit;
        res = (res >> 1) + bit;
      }
      else
      {
        res >>= 1;
      }
      bit >>= 2;
    }
    return (U)res;
  };

  U r = isqrt_rest(Wv);

  // nearest rounding on Wv
  uint64_t r2 = (uint64_t)r * r;
  uint64_t rem = Wv - r2;
  if ((rem << 1) > ((uint64_t)r * 2 + 1))
    ++r;
  return r;  // Qceil(F/2)
}

// Float-assisted (generic F)
template <int F>
static inline uint32_t sqrt_qF_float(uint32_t X)
{
  if (!X)
    return 0;
  constexpr int G = (F + 1) / 2;
  float xf = scalbnf((float)X, -F);
  float yf = sqrtf(xf);
  float sf = scalbnf(yf, G);
  return (uint32_t)lrintf(sf);
}

// Float-assisted (generic F)
template <int F>
static inline uint32_t sqrt_qF_vsqrt32(uint32_t X)
{
  if (!X)
    return 0;
  constexpr int G = (F + 1) / 2;
  float xf = scalbnf((float)X, -F);
  //float yf = sqrtf(xf);
  float yf;
  asm volatile("vsqrt.f32 %0, %1" : "=t"(yf) : "t"(xf));
  float sf = scalbnf(yf, G);
  return (uint32_t)lrintf(sf);
}

/* Wrappers with unified UQFUnary signature for registration */
static uint32_t sqrt_Q32_to_Q16_NR_wrap(uint32_t X)
{
  return sqrt_Q32_to_Q16_NR(X);
}
static uint32_t sqrt_q32_rest_wrap(uint32_t X)
{
  return sqrt_qF_restoring<32>(X);
}
static uint32_t sqrt_q24_rest_wrap(uint32_t X)
{
  return sqrt_qF_restoring<24>(X);
}
static uint32_t sqrt_q32_float_wrap(uint32_t X)
{
  return sqrt_qF_float<32>(X);
}
static uint32_t sqrt_q24_float_wrap(uint32_t X)
{
  return sqrt_qF_float<24>(X);
}
static uint32_t sqrt_q32_vsqrt32_wrap(uint32_t X)
{
  return sqrt_qF_vsqrt32<32>(X);
}
static uint32_t sqrt_q24_vsqrt32_wrap(uint32_t X)
{
  return sqrt_qF_vsqrt32<24>(X);
}


// Plain lib call (the compiler will usually map this to VSQRT.F32 on M7)
static float sqrtf_libm(float x)
{
  return sqrtf(x);
}

// Explicit FPU instruction variant (forces the single VSQRT.F32 instruction)
static inline float vsqrt32(float x)
{
  float r;
  asm volatile("vsqrt.f32 %0, %1" : "=t"(r) : "t"(x));
  return r;
}


static const VariantUQF kSqrtQ32Variants[] = {
    {"Heron NR (div)", &sqrt_Q32_to_Q16_NR_wrap},
    {"restoring (int)", &sqrt_q32_rest_wrap},
    {"float-assisted-32", &sqrt_q32_float_wrap},
    {"float-assisted-24", &sqrt_q24_float_wrap},
    {"float-assisted-vsqrt32-32", &sqrt_q32_vsqrt32_wrap},
    {"float-assisted-vsqrt32-24", &sqrt_q24_vsqrt32_wrap},
};

static const VariantFloat kSqrtFVariants[] = {
    {"sqrtf (libm)", &sqrtf_libm},
    {"vsqrt.f32 (FPU)", &vsqrt32},
};
