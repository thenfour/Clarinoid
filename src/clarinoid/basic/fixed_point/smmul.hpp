/*======================================================================
  DSP-extension "high-word" multiply helpers for Cortex-M4/M7/M33
  ----------------------------------------------------------------------
  All variants take 32-bit signed inputs a,b (and optional acc) and
  return the *upper 32 bits* of the 64-bit product, with or without
  rounding and/or accumulation.

  – SMMUL   : y =  (a * b) >> 32              (truncate)
  – SMMULR  : y = ((a * b) + 0x8000_0000) >> 32   (round)
  – SMMLA   : y =  (a * b >> 32) +  acc
  – SMMLAR  : y = ((a * b + 0x8000_0000) >> 32) + acc
  – SMMLS   : y =  acc − (a * b >> 32)
  – SMMLSR  : y =  acc − ((a * b + 0x8000_0000) >> 32)

  The inline-asm form executes in **1 cycle latency / 1 cycle throughput**
  on any core that defines __ARM_FEATURE_DSP.  The software fallbacks
  compile everywhere but cost ~2–3 cycles on M7 and ~5 on M4.
======================================================================*/

#pragma once

#include <stdint.h>

#if defined(__ARM_FEATURE_DSP) && defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
/*------------------------------  fast path  ------------------------------*/
#define _SMMUL_BODY(inst, a, b, out) __asm__ volatile(inst "  %0, %1, %2" : "=r"(out) : "r"(a), "r"(b))

#define _SMMLA_BODY(inst, a, b, c, out) __asm__ volatile(inst "  %0, %1, %2, %3" : "=r"(out) : "r"(a), "r"(b), "r"(c))

static inline int32_t
smmul(int32_t a, int32_t b)
{
  int32_t r;
  _SMMUL_BODY("smmul", a, b, r);
  return r;
}
static inline int32_t
smmulr(int32_t a, int32_t b)
{
  int32_t r;
  _SMMUL_BODY("smmulr", a, b, r);
  return r;
}
static inline int32_t
smmla(int32_t a, int32_t b, int32_t acc)
{
  int32_t r;
  _SMMLA_BODY("smmla", a, b, acc, r);
  return r;
}
static inline int32_t
smmlar(int32_t a, int32_t b, int32_t acc)
{
  int32_t r;
  _SMMLA_BODY("smmlar", a, b, acc, r);
  return r;
}
static inline int32_t
smmls(int32_t a, int32_t b, int32_t acc)
{
  int32_t r;
  _SMMLA_BODY("smmls", a, b, acc, r);
  return r;
}
static inline int32_t
smmlsr(int32_t a, int32_t b, int32_t acc)
{
  int32_t r;
  _SMMLA_BODY("smmlsr", a, b, acc, r);
  return r;
}

#else
/*---------------------------  portable fall-backs  -----------------------*/
static inline int32_t
smmul(int32_t a, int32_t b)
{
  return (int32_t)(((int64_t)a * b) >> 32);
}

static inline int32_t
smmulr(int32_t a, int32_t b)
{
  return (int32_t)((((int64_t)a * b) + 0x80000000LL) >> 32);
}

static inline int32_t
smmla(int32_t a, int32_t b, int32_t acc)
{
  return acc + smmul(a, b);
}

static inline int32_t
smmlar(int32_t a, int32_t b, int32_t acc)
{
  return acc + smmulr(a, b);
}

static inline int32_t
smmls(int32_t a, int32_t b, int32_t acc)
{
  return acc - smmul(a, b);
}

static inline int32_t
smmlsr(int32_t a, int32_t b, int32_t acc)
{
  return acc - smmulr(a, b);
}
#endif /* __ARM_FEATURE_DSP */


// UMMUL instruction doesn't actually exist, but it's not really needed because you just
// do a typical unsigned long mul, and throw away the low bits.
/*  UMMUL_HI  – (a * b) >> 32          (truncate)
 *  UMMULR_HI – ((a * b) + 0x8000_0000) >> 32   (round to nearest)
 *  Works in one cycle on any v7E-M with __ARM_FEATURE_DSP
 *  Falls back to portable C on other architectures.
 */
static inline uint32_t
ummul(uint32_t a, uint32_t b)
{
#if defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
  uint32_t hi, lo; /* lo is thrown away      */
  __asm__ volatile("umull %0, %1, %2, %3" : "=&r"(lo), "=&r"(hi) : "r"(a), "r"(b));
  (void)lo;
  return hi;
#else
  return (uint32_t)(((uint64_t)a * b) >> 32);
#endif
}

static inline uint32_t
ummulr(uint32_t a, uint32_t b)
{
#if defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
  uint32_t hi, lo;
  __asm__ volatile("umull  %0, %1, %2, %3\n\t"      /* lo, hi = a * b      */
                   "adds   %0, %0, #0x80000000\n\t" /* + rounding bias  */
                   "adc    %1, %1, #0"              /* propagate carry  */
                   : "=&r"(lo), "=&r"(hi)
                   : "r"(a), "r"(b));
  (void)lo;
  return hi; /* (a*b + 0x8000_0000) >> 32 */
#else
  return (uint32_t)((((uint64_t)a * b) + 0x80000000ULL) >> 32);
#endif
}


