// consider:
// - accuracy around 0, 1, or middle
// - speed
// - does it saturate properly?

// **tanh-around0**

// | Variant            | Speed (cycles) | Speed (graph)          | Rel max (ppm) | Rel max (graph)        | Rel mean (ppm) | Rel mean (graph)       | Rel rms (ppm) | Rel rms (graph)        |
// |:-------------------| --------------:|:-----------------------| -------------:|:-----------------------| --------------:|:-----------------------| -------------:|:-----------------------|
// | tanh (ref)         |         183.88 | `====================` |             0 | `                    ` |          0.000 | `                    ` |         0.000 | `                    ` |
// | fastertanh         |          86.88 | `=========           ` |    4294967295 | `====================` |    3524912.923 | `====================` | 116891757.215 | `====================` |
// | zangtw_fasttanh    |         130.63 | `==============      ` |      11000000 | `                    ` |       1354.859 | `                    ` |     47129.921 | `                    ` |
// | discohead_fastTanh |          59.75 | `======              ` |         32634 | `                    ` |      32553.252 | `                    ` |     32553.333 | `                    ` |
// | npisanti_fasttanh  |          83.76 | `=========           ` |        371548 | `                    ` |     353430.074 | `==                  ` |    353601.966 | `                    ` |
// | vox_fasttanh2      |          78.75 | `=========           ` |          4270 | `                    ` |       4183.067 | `                    ` |      4183.750 | `                    ` |
// | fastTanh_NIM       |          67.75 | `=======             ` |            92 | `                    ` |         30.839 | `                    ` |        41.367 | `                    ` |

// **tanh-0.5**

// | Variant            | Speed (cycles) | Speed (graph)          | Rel max (ppm) | Rel max (graph)        | Rel mean (ppm) | Rel mean (graph)       | Rel rms (ppm) | Rel rms (graph)        |
// |:-------------------| --------------:|:-----------------------| -------------:|:-----------------------| --------------:|:-----------------------| -------------:|:-----------------------|
// | tanh (ref)         |         220.31 | `====================` |             0 | `                    ` |          0.000 | `                    ` |         0.000 | `                    ` |
// | fastertanh         |          86.88 | `========            ` |         16464 | `=                   ` |      14980.131 | `=                   ` |     15051.797 | `=                   ` |
// | zangtw_fasttanh    |         130.63 | `============        ` |            25 | `                    ` |          5.504 | `                    ` |         7.485 | `                    ` |
// | discohead_fastTanh |          59.75 | `=====               ` |         14710 | `=                   ` |      10934.122 | `                    ` |     11157.544 | `                    ` |
// | npisanti_fasttanh  |          83.76 | `========            ` |        459909 | `====================` |     458979.880 | `====================` |    458980.227 | `====================` |
// | vox_fasttanh2      |          78.75 | `=======             ` |           858 | `                    ` |        811.079 | `                    ` |       812.448 | `                    ` |
// | fastTanh_NIM       |          67.75 | `======              ` |          9388 | `                    ` |       8004.306 | `                    ` |      8043.141 | `                    ` |

// **tanh-before1**

// | Variant            | Speed (cycles) | Speed (graph)          | Rel max (ppm) | Rel max (graph)        | Rel mean (ppm) | Rel mean (graph)       | Rel rms (ppm) | Rel rms (graph)        |
// |:-------------------| --------------:|:-----------------------| -------------:|:-----------------------| --------------:|:-----------------------| -------------:|:-----------------------|
// | tanh (ref)         |         230.89 | `====================` |             0 | `                    ` |          0.000 | `                    ` |         0.000 | `                    ` |
// | fastertanh         |          86.88 | `========            ` |          6480 | `                    ` |       3262.235 | `                    ` |      3727.908 | `                    ` |
// | zangtw_fasttanh    |         130.63 | `===========         ` |            17 | `                    ` |          9.299 | `                    ` |        10.368 | `                    ` |
// | discohead_fastTanh |          59.75 | `=====               ` |         30074 | `=                   ` |      26307.475 | `=                   ` |     26400.396 | `=                   ` |
// | npisanti_fasttanh  |          83.76 | `=======             ` |        474785 | `====================` |     472580.752 | `====================` |    472582.444 | `====================` |
// | vox_fasttanh2      |          78.75 | `=======             ` |           327 | `                    ` |        220.911 | `                    ` |       230.696 | `                    ` |
// | fastTanh_NIM       |          67.75 | `======              ` |         21249 | `=                   ` |      20164.981 | `=                   ` |     20175.275 | `=                   ` |

// **tanh-after1**

// | Variant            | Speed (cycles) | Speed (graph)          | Rel max (ppm) | Rel max (graph)        | Rel mean (ppm) | Rel mean (graph)       | Rel rms (ppm) | Rel rms (graph)        |
// |:-------------------| --------------:|:-----------------------| -------------:|:-----------------------| --------------:|:-----------------------| -------------:|:-----------------------|
// | tanh (ref)         |         235.89 | `====================` |             0 | `                    ` |          0.000 | `                    ` |         0.000 | `                    ` |
// | fastertanh         |          86.88 | `=======             ` |         10350 | `                    ` |       3923.782 | `                    ` |      4812.833 | `                    ` |
// | zangtw_fasttanh    |         130.63 | `===========         ` |            16 | `                    ` |          5.578 | `                    ` |         6.718 | `                    ` |
// | discohead_fastTanh |          59.75 | `=====               ` |         42530 | `==                  ` |      36669.366 | `==                  ` |     36846.305 | `==                  ` |
// | npisanti_fasttanh  |          83.76 | `=======             ` |        483439 | `====================` |     479180.101 | `====================` |    479186.656 | `====================` |
// | vox_fasttanh2      |          78.75 | `=======             ` |           479 | `                    ` |        435.229 | `                    ` |       437.563 | `                    ` |
// | fastTanh_NIM       |          67.75 | `======              ` |         24469 | `=                   ` |      23007.652 | `=                   ` |     23026.570 | `=                   ` |

// **tanh-below-n2**

// | Variant            | Speed (cycles) | Speed (graph)          | Rel max (ppm) | Rel max (graph)        | Rel mean (ppm) | Rel mean (graph)       | Rel rms (ppm) | Rel rms (graph)        |
// |:-------------------| --------------:|:-----------------------| -------------:|:-----------------------| --------------:|:-----------------------| -------------:|:-----------------------|
// | tanh (ref)         |         243.98 | `====================` |             0 | `                    ` |          0.000 | `                    ` |         0.000 | `                    ` |
// | fastertanh         |          86.88 | `=======             ` |          1215 | `                    ` |         20.129 | `                    ` |        89.991 | `                    ` |
// | zangtw_fasttanh    |         130.63 | `===========         ` |             1 | `                    ` |          0.033 | `                    ` |         0.155 | `                    ` |
// | discohead_fastTanh |          59.75 | `=====               ` |        744899 | `================    ` |     489282.017 | `====================` |    537621.067 | `====================` |
// | npisanti_fasttanh  |          83.76 | `=======             ` |        504308 | `===========         ` |     500881.855 | `====================` |    500883.355 | `=================== ` |
// | vox_fasttanh2      |          78.75 | `======              ` |          1905 | `                    ` |        574.059 | `                    ` |       811.638 | `                    ` |
// | fastTanh_NIM       |          67.75 | `======              ` |        942513 | `====================` |     345213.184 | `==============      ` |    455800.371 | `=================   ` |

// **tanh-above-p2**

// | Variant            | Speed (cycles) | Speed (graph)          | Rel max (ppm) | Rel max (graph)        | Rel mean (ppm) | Rel mean (graph)       | Rel rms (ppm) | Rel rms (graph)        |
// |:-------------------| --------------:|:-----------------------| -------------:|:-----------------------| --------------:|:-----------------------| -------------:|:-----------------------|
// | tanh (ref)         |         243.98 | `====================` |             0 | `                    ` |          0.000 | `                    ` |         0.000 | `                    ` |
// | fastertanh         |          86.88 | `=======             ` |          1267 | `                    ` |         20.554 | `                    ` |        92.232 | `                    ` |
// | zangtw_fasttanh    |         130.63 | `===========         ` |             2 | `                    ` |          0.058 | `                    ` |         0.167 | `                    ` |
// | discohead_fastTanh |          59.75 | `=====               ` |        744899 | `================    ` |     489282.017 | `====================` |    537621.067 | `====================` |
// | npisanti_fasttanh  |          83.76 | `=======             ` |        504308 | `===========         ` |     500881.855 | `====================` |    500883.355 | `=================== ` |
// | vox_fasttanh2      |          78.75 | `======              ` |          1905 | `                    ` |        574.059 | `                    ` |       811.638 | `                    ` |
// | fastTanh_NIM       |          67.75 | `======              ` |        942513 | `====================` |     345213.184 | `==============      ` |    455800.370 | `=================   ` |

// fastertanh : large error around 0 makes it unreliable
// npisanti_fasttanh: error around 0.5 too much, plus incorrect saturation. don't use.
// zangtw_fasttanh has good accuracy, but too slow
// leaving only these which are FAST, and behave accurately from [-1,1]:
// - vox_fasttanh2
// - discohead_fastTanh
// - fastTanh_NIM
//
// but for extended range (-16,16), only vox_fasttanh2 is good, and performs well.
//
// for fixed point implementations, it's hard to avoid a 64-bit divide, and you kinda end up needing float assistance to make it optimal.
// it means that basically, i haven't found a fixed point implementation that's usable. plus there's no headroom.

#include <clarinoid/core/basic/numeric/smmul.hpp>
#include <math.h>

#pragma once


// https://www.kvraudio.com/forum/viewtopic.php?f=33&t=388650&start=45
inline float vox_fasttanh2(const float x)
{
  const float ax = fabsf(x);
  const float x2 = x * x;

  return (x * (2.45550750702956f + 2.45550750702956f * ax + (0.893229853513558f + 0.821226666969744f * ax) * x2) /
          (2.44506634652299f + (2.44506634652299f + x2) * fabsf(x + 0.814642734961073f * x * ax)));
}

// https://github.com/npisanti/ofxPDSP/blob/e106991f4abf4314116d4e7c4ef7ad69d6ca005f/src/math/trig/fasttanh.h
inline float npisanti_fasttanh(float angle)
{
  return angle / (fabsf(2 * angle) + 3 / (2 + 2 * angle * 2 * angle));
}

// less accurate, a bit faster than vox_
// https://github.com/ftsf/nimsynth/blob/57d4e56cd0370309a12a0bf902d5d3115539adea/src/core/filter.nim
inline float fastTanh_NIM(float x)
{
  float x2 = x * x;
  return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

// https://github.com/discohead/LXR_JCM/blob/14b4b06ce5c9f4a60528d0c2d181f47227ae87df/mainboard/LxrStm32/src/DSPAudio/ResonantFilter.c
inline float discohead_fastTanh(float var)
{
  return 4.15f * var / (4.29f + var * var);
}

static inline float fastpow2(float p)
{
  float offset = (p < 0) ? 1.0f : 0.0f;
  float clipp = (p < -126) ? -126.0f : p;
  int w = clipp;
  float z = clipp - w + offset;
  union
  {
    uint32_t i;
    float f;
  } v = {static_cast<uint32_t>((1 << 23) * (clipp + 121.2740575f + 27.7280233f / (4.84252568f - z) - 1.49012907f * z))};

  return v.f;
}

static inline float fastexp(float p)
{
  return fastpow2(1.442695040f * p);
}

// https://github.com/zangtw/fastapprox/blob/8cc1dc8d888758f002758a98a393e41e8e43366e/fastapprox/src/fasthyperbolic.h
static inline float zangtw_fasttanh(float p)
{
  return -1.0f + 2.0f / (1.0f + fastexp(-2.0f * p));
}


static inline float fasterpow2(float p)
{
  float clipp = (p < -126) ? -126.0f : p;
  union
  {
    uint32_t i;
    float f;
  } v = {static_cast<uint32_t>((1 << 23) * (clipp + 126.94269504f))};
  return v.f;
}

static inline float fasterexp(float p)
{
  return fasterpow2(1.442695040f * p);
}


static inline float fastertanh(float p)
{
  return -1.0f + 2.0f / (1.0f + fasterexp(-2.0f * p));
}

static inline float libc_tanh(float p)
{
  return ::tanhf(p);
}


namespace fasttanh
{
}

// xQ31 is 1.31 fixed-point, value in ±1.  uses 64-bit intermediates.
static inline int32_t tanh_vox_q31(int32_t xQ31)
{
  // Q31 constants
  static constexpr int64_t A_Q31 = 0x4E2AECCCLL;
  static constexpr int64_t B_Q31 = 0x1C99BDCCLL;
  static constexpr int64_t C_Q31 = 0x1A23AEF7LL;
  static constexpr int64_t D_Q31 = 0x4DFFF6F4LL;
  static constexpr int64_t E_Q31 = 0x1A0DA3A6LL;

  const int32_t sign = xQ31 >> 31;
  int32_t ax = (xQ31 ^ sign) - sign;  // |x|

  int64_t x = (int64_t)xQ31;
  int64_t ax64 = (int64_t)ax;
  int64_t x2 = (x * x) >> 31;  // 64-bit » 31 → Q31
  int64_t num = x * (A_Q31 + ((A_Q31 * ax64) >> 31) + ((B_Q31 + ((C_Q31 * ax64) >> 31)) * x2 >> 31));
  // denominator
  int64_t term = ax64 + (E_Q31 * x >> 31) * ax64 >> 31;  // |x + E·x·|x||
  int64_t den = D_Q31 + ((D_Q31 + x2) * term >> 31);

  int64_t y64 = (num << 31) / den;  // restore Q31
  // saturation
  if (y64 > 0x7FFFFFFFLL)
    y64 = 0x7FFFFFFFLL;
  if (y64 < -0x80000000LL)
    y64 = -0x800000000LL;
  return (int32_t)y64;
}

static inline int16_t tanh_vox_q15(int16_t xQ15)
{
  // Q15 constants
  static constexpr int16_t A_Q15 = 0x7A4D;
  static constexpr int16_t B_Q15 = 0x16C7;
  static constexpr int16_t C_Q15 = 0x14E4;
  static constexpr int16_t D_Q15 = 0x7A06;
  static constexpr int16_t E_Q15 = 0x14B1;

  int16_t sign = xQ15 >> 15;
  int16_t ax = (xQ15 ^ sign) - sign;  // |x|

  int32_t x = (int32_t)xQ15;
  int32_t ax32 = (int32_t)ax;
  int32_t x2 = (x * x) >> 15;  // Q15

  int32_t num = x * (A_Q15 + ((A_Q15 * ax32) >> 15) + ((B_Q15 + ((C_Q15 * ax32) >> 15)) * x2 >> 15));

  int32_t term = ax32 + ((E_Q15 * x) >> 15) * ax32 >> 15;
  int32_t den = D_Q15 + ((D_Q15 + x2) * term >> 15);

  int32_t y32 = (num << 15) / den;  // back to Q15
  if (y32 > 0x7FFF)
    y32 = 0x7FFF;
  if (y32 < -0x8000)
    y32 = -0x8000;
  return (int16_t)y32;
}

// xQ31 in ±1.999 (1.31).  Uses one reciprocal-Newton step.
static inline int32_t tanh_vox_q31_fast(int32_t xQ31)
{
  //using namespace vox_q31;
  constexpr int32_t A = 0x4E2AECCC;    //  2.4555075 * 2^31
  constexpr int32_t B = 0x1C99BDCC;    //  0.89322985
  constexpr int32_t C = 0x1A23AEF7;    //  0.82122667
  constexpr int32_t D = 0x4DFFF6F4;    //  2.44506634
  constexpr int32_t E = 0x1A0DA3A6;    //  0.81464273
  constexpr int32_t ONE = 0x7FFFFFFF;  //  0.999999999 (1.31)

  // sign & magnitude
  int32_t sign = xQ31 >> 31;
  int32_t ax = (xQ31 ^ sign) - sign;  // |x|

  /* ------------ numerator ------------ */

  // t1 = A + A*|x|
  int32_t t1 = smmla(A, ax, A);  // Q31

  // t2 = B + C*|x|
  int32_t t2 = smmla(C, ax, B);  // Q31

  // x²
  int32_t x2 = smmul(xQ31, xQ31);  // Q31

  // t1 + t2*x²   (use SMMLA so both mult+add in one)
  int32_t num_body = smmla(t2, x2, t1);  // Q31

  // numerator = x * num_body
  int32_t num = smmul(xQ31, num_body);  // Q31

  /* ------------ denominator ------------ */

  // term = |x| + E*x*|x|   (x*|x| fits in 1.30; SMMUL okay)
  int32_t x_ax = smmul(xQ31, ax);      // x*|x| >>31  Q31
  int32_t term = ax + smmul(E, x_ax);  // Q31 (add w/ wrap fine)

  // D + (D + x²) * term
  int32_t tmp = D + x2;                // Q31
  int32_t den = D + smmul(tmp, term);  // Q31

  /* --------- fast reciprocal of den (one Newton) -------- */

  // initial guess y0 ≈ 1/den in 0.1 ulp: use one reciprocal estimate instruction
  // For M7 we craft y0 via a "magic" floating trick → convert back to Q31.
  float fden = den * (1.0f / 2147483648.0f);           // Q31 -> float
  int32_t y = int32_t((1.0f / fden) * 2147483648.0f);  // Q31 guess

  // Newton: y = y * (2 - den*y)   (all Q31, uses SMMULR for rounding)
  int32_t prod = smmulr(den, y);            // den*y
  int32_t two_minus = (0x40000000 - prod);  // 2 in Q30 is 0x4000_0000
  y = smmulr(y, two_minus);                 // refine to ~17-18 bits exact

  // Result = num * y  (Q31*Q31 >>31)
  int32_t out = smmul(num, y);

  /* ------------ restore sign & saturate ------------ */

  out ^= sign;
  out -= sign;

  if (out > ONE)
    out = ONE;
  if (out < -ONE)
    out = -ONE;
  return out;
}


// xQ31 in ±1.999 (1.31). no float assistance, uses 32-bit div so less precision but faster.
static inline int32_t tanh_vox_q31_fast_div32(int32_t xQ31)
{
  //using namespace vox_q31;
  constexpr int32_t A = 0x4E2AECCC;    //  2.4555075 * 2^31
  constexpr int32_t B = 0x1C99BDCC;    //  0.89322985
  constexpr int32_t C = 0x1A23AEF7;    //  0.82122667
  constexpr int32_t D = 0x4DFFF6F4;    //  2.44506634
  constexpr int32_t E = 0x1A0DA3A6;    //  0.81464273
  constexpr int32_t ONE = 0x7FFFFFFF;  //  0.999999999 (1.31)

  // sign & magnitude
  int32_t sign = xQ31 >> 31;
  int32_t ax = (xQ31 ^ sign) - sign;  // |x|

  /* ------------ numerator ------------ */

  // t1 = A + A*|x|
  int32_t t1 = smmla(A, ax, A);  // Q31

  // t2 = B + C*|x|
  int32_t t2 = smmla(C, ax, B);  // Q31

  // x²
  int32_t x2 = smmul(xQ31, xQ31);  // Q31

  // t1 + t2*x²   (use SMMLA so both mult+add in one)
  int32_t num_body = smmla(t2, x2, t1);  // Q31

  // numerator = x * num_body
  int32_t num = smmul(xQ31, num_body);  // Q31

  /* ------------ denominator ------------ */

  // term = |x| + E*x*|x|   (x*|x| fits in 1.30; SMMUL okay)
  int32_t x_ax = smmul(xQ31, ax);      // x*|x| >>31  Q31
  int32_t term = ax + smmul(E, x_ax);  // Q31 (add w/ wrap fine)

  // D + (D + x²) * term
  int32_t tmp = D + x2;                // Q31
  int32_t den = D + smmul(tmp, term);  // Q31

  /* --------- fast reciprocal of den (one Newton) -------- */

  // // initial guess y0 ≈ 1/den in 0.1 ulp: use one reciprocal estimate instruction
  // // For M7 we craft y0 via a "magic" floating trick → convert back to Q31.
  // float fden = den * (1.0f / 2147483648.0f);      // Q31 -> float
  // int32_t y  = int32_t((1.0f / fden) * 2147483648.0f); // Q31 guess

  // // Newton: y = y * (2 - den*y)   (all Q31, uses SMMULR for rounding)
  // int32_t prod = smmulr(den, y);                  // den*y
  // int32_t two_minus = (0x40000000 - prod);       // 2 in Q30 is 0x4000_0000
  // y = smmulr(y, two_minus);                       // refine to ~17-18 bits exact

  // // Result = num * y  (Q31*Q31 >>31)
  // int32_t out = smmul(num, y);

  int32_t out = (int32_t)(((int64_t)num << 15) / den);  // shift-15 keeps headroom
  out = smmul(out, 0x8000);                             // rescale back to Q31 (>>16 with rounding)


  /* ------------ restore sign & saturate ------------ */

  out ^= sign;
  out -= sign;

  if (out > ONE)
    out = ONE;
  if (out < -ONE)
    out = -ONE;
  return out;
}

//constexpr int32_t fp(double d) { return int32_t(d * 2147483648.0 + 0.5); }

//--- NIM:  x·(27+x²)/(27+9x²) ---------------------------------------------
// static inline int32_t tanh_nim_q31(int32_t x)
// {
// constexpr int32_t A_NIM_Q31 = fp(27.0);   // 0x1B000000
// constexpr int32_t B_NIM_Q31 = fp(9.0);    // 0x07000000

//     int32_t x2 = smmul(x, x);                       // x²   Q31
//     int32_t num = smmla(A_NIM_Q31, x2, A_NIM_Q31);  // 27 + x²
//     num = smmul(x, num);                            // x*(27+x²)

//     int32_t den = smmla(B_NIM_Q31, x2, A_NIM_Q31);  // 27 + 9x²
//     // multiply by reciprocal 1/den (Q2.30) pre-scaled into Brec
//     // pre-compute 1/den in float and convert once:
//     float f = (float)den * (1.0f/2147483648.0f);
//     int32_t rec = int32_t((1.0f/f) * 1073741824.0f + 0.5f); // Q2.30
//     int32_t y = smmul(rec, num)<<1;               // back to Q31
//     return y;
// }

//--- Discohead: (4.15 x)/(4.29 + x²) ---------------------------------------
// static inline int32_t tanh_disco_q31(int32_t x)
// {
// constexpr int32_t A_DISCO_Q31 = fp(4.15); // 0x84A7EFEB -- these overflow; not sure what i was expecting.
// constexpr int32_t B_DISCO_Q31 = fp(4.29); // 0x89BA5E35

//     int32_t num = smmul(A_DISCO_Q31, x);            // 4.15·x
//     int32_t den = B_DISCO_Q31 + smmul(x, x);        // 4.29+x²

//     float f = (float)den * (1.0f/2147483648.0f);
//     int32_t rec = int32_t((1.0f/f) * 1073741824.0f + 0.5f); // Q2.30
//     int32_t y = smmul(rec, num)<<1;
//     return y;
// }


static const VariantUQF kTanhQ32Variants[] = {
    {"tanh_vox_q31", &tanh_vox_q31},
    {"tanh_vox_q31_fast", &tanh_vox_q31_fast},
    {"tanh_vox_q31_fast_div32", &tanh_vox_q31_fast_div32},
    //{ "tanh_disco_q31",   &tanh_disco_q31 },
    //{ "tanh_nim_q31",   &tanh_nim_q31 },
};


static const VariantFloat kTanhFVariants[] = {
    {"tanh (ref)", &libc_tanh},
    {"fastertanh", &fastertanh},
    {"zangtw_fasttanh", &zangtw_fasttanh},
    {"discohead_fastTanh", &discohead_fastTanh},
    {"npisanti_fasttanh", &npisanti_fasttanh},
    {"vox_fasttanh2", &vox_fasttanh2},
    {"fastTanh_NIM", &fastTanh_NIM},
};
