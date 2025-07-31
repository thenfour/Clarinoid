/*  Teensy 4.x (Cortex-M7F) fixed-point sqrt benchmark harness
    - Kernels:
        1) sqrt_qF_float      : float-assisted (FPU)
        2) sqrt_qF_restoring  : pure-integer, division-free restoring method
        3) sqrt_Q32_to_Q16_NR : Heron/Newton divide-based (specialized to Q32->Q16), with fixes
    - Formats tested: Q32->Q16, Q24->Q12 (edit kFormatsToTest to add more)

    Results:
      - Correctness: max/mean/RMS abs error in ULP (raw LSBs of result), and max/mean relative error (ppm)
      - Speed: cycles/call (DWT) + fallback to micros() if DWT not available

    Notes:
      - Inputs are treated as *unsigned* fixed-point.
      - For an unsigned QF input, sqrt returns QG where G = ceil(F/2).
      - Reference uses double and ties-to-nearest (half-up) rounding to target grid.
*/

#include <Arduino.h>
#include <stdint.h>
#include <math.h>
#include <limits>
#include "types.hpp"

// ========= Utility: CLZ wrapper =========
inline int clz32(uint32_t x) {
  if (x == 0) return 32;
  return __builtin_clz(x);
}

// // ========= Heron/Newton Q32->Q16 (divide-based) with fixes =========
// constexpr uint32_t sqrt_integer_guess_table[33] = {
//   55109,38968,27555,19484,13778,9742,6889,4871,3445,2436,1723,1218,862,609,431,305,216,
//   153,108,77,54,39,27,20,14,10,7,5,4,3,2,1,0,
// };

// inline uint32_t sqrt_Q32_to_Q16_NR(uint32_t in)
// {
//   if (!in) return 0;
//   int i = clz32(in);
//   uint32_t n = sqrt_integer_guess_table[i];
//   if (n == 0) n = 1; // safety

//   // Two iterations usually suffice; add exact post-correction
//   n = ((in / n) + n) >> 1;
//   n = ((in / n) + n) >> 1;

//   // Exact correction (ULP-perfect)
//   uint64_t nn = (uint64_t)n * n;
//   if (nn > in) { while ((uint64_t)(n - 1) * (n - 1) >= in) --n; }
//   else         { while ((uint64_t)(n + 1) * (n + 1) <=  in) ++n; }

//   uint64_t n2  = (uint64_t)n * n;
//   uint64_t up2 = (uint64_t)(n+1) * (n+1);
//   uint64_t thr = n2 + ((up2 - n2) >> 1);  // midpoint at n^2 + n + 0.5
//   if (in > thr) ++n;

//   return n; // Q16
// }



// // ========= Heron/Newton Q32->Q16 (divide-based) with fixes =========
// // this only works for 32-bit inputs, because of the dependence on CLZ and the table size.
// // but that's fine because for teensy we don't want 64-bit source values (which would incur a 64-bit div).
// constexpr uint32_t sqrt_integer_guess_table[31] = {
//   55109, 38968, 27555, 19484, 13778, 9742, 6889, 4871, 3445, 2436, 1723, 1218, 862, 609, 431, 305, 216,
//   153,   108,   77,    54,    39,    27,   20,   14,   10,   7,    5,    4,    3,   2,
// };

// // this is named Q32 to Q16, but it's more general than that.
// // it works for any input with F fract bits, and returns in a format of F/2 fract bits.
// // if F is odd, it will return F/2 + 1 fract bits (technically (F+1)/2).
// // 
// inline uint32_t
// sqrt_Q32_to_Q16_NR(uint32_t in)
// {
//   if (in <= 1u)
//     return in;
//   int i = clz32(in);
//   uint32_t n = sqrt_integer_guess_table[i];
//   if (n == 0)
//     n = 1; // safety

//   // Two iterations usually suffice; add exact post-correction
//   n = ((in / n) + n) >> 1;
//   n = ((in / n) + n) >> 1;

//   // Exact correction (ULP-perfect)
//   uint64_t nn = (uint64_t)n * n;
//   if (nn > in) {
//     while ((uint64_t)(n - 1) * (n - 1) >= in)
//       --n;
//   } else {
//     while ((uint64_t)(n + 1) * (n + 1) <= in)
//       ++n;
//   }

//   uint64_t n2 = (uint64_t)n * n;
//   uint64_t up2 = (uint64_t)(n + 1) * (n + 1);
//   uint64_t thr = n2 + ((up2 - n2) >> 1); // midpoint at n^2 + n + 0.5
//   if (in > thr)
//     ++n;

//   return n; // Q16
// }


// 31-entry table (indices 0..30). Values fit in 16 bits.
static constexpr uint16_t sqrt_integer_guess_table[31] = {
  55109,38968,27555,19484,13778,9742,6889,4871,3445,2436,1723,1218,862,609,431,305,
  216,  153,  108,   77,   54,   39,   27,   20,   14,  10,    7,   5,   4,   3,   2
};

static inline uint32_t sqrt_Q32_to_Q16_NR(uint32_t in)
{
  if (in <= 1u) return in;                         // avoids clz==31 case and is exact

  uint32_t n = sqrt_integer_guess_table[__builtin_clz(in)];

  // Two Newton steps (Heron), 32-bit divisions
  uint32_t q = in / n;  n = (q + n) >> 1;
           q = in / n;  n = (q + n) >> 1;

  // Exact correction to floor
  // uint64_t n2 = (uint64_t)n * n;
  // if (n2 > in) { while ((uint64_t)(n-1)*(n-1) >= in) --n; }
  // else         { while ((uint64_t)(n+1)*(n+1) <=  in) ++n; }

  // Nearest rounding (optional; comment out to keep floor)
  uint64_t low  = (uint64_t)n * n;
  uint64_t high = (uint64_t)(n+1) * (n+1);
  if (in > low + ((high - low) >> 1)) ++n;

  return n;
}



// ========= Restoring (digit-by-digit) integer isqrt (division-free) =========
template<typename U>
inline U isqrt_restoring(U x)
{
  if (!x) return 0;
  // Highest power of 4 <= x
  U bit = U(1) << (std::numeric_limits<U>::digits - 2);
  while (bit > x) bit >>= 2;

  U res = 0;
  while (bit) {
    if (x >= res + bit) { x -= res + bit; res = (res >> 1) + bit; }
    else                { res >>= 1; }
    bit >>= 2;
  }
  return res;
}



// ========= Format-agnostic kernels =========
// Rule: for unsigned QF input X, let G = ceil(F/2).
// If F even: y_raw = isqrt(X)        // result is QG
// If F odd : y_raw = isqrt(X << 1)   // result is QG
template<int F>
static inline uint32_t sqrt_qF_restoring(uint32_t X)
{
  if (!X) return 0;
  constexpr bool odd = (F & 1) != 0;
  if constexpr (odd) {
    // Promote and shift to make fractional count even
    uint64_t W = (uint64_t)X << 1;
    return (uint32_t)isqrt_restoring<uint64_t>(W);
  } else {
    return isqrt_restoring<uint32_t>(X);
  }
}


// Float-assisted kernel (fast on M7 FPU)
template<int F>
inline uint32_t sqrt_qF_float1(uint32_t X)
{
  if (!X) return 0;
  constexpr int G = (F + 1) / 2;
  // x = X / 2^F
  float xf = ldexpf((float)X, -F);
  float yf = sqrtf(xf);
  float sf = ldexpf(yf, G);

  // nearest; for truncation use (uint32_t)sf
  long r = lrintf(sf);
  if (r < 0) r = 0;
  if (r > 0xFFFFFFFFl) r = 0xFFFFFFFFl;
  return (uint32_t)r;
}

template<int F>
static inline uint32_t sqrt_qF_float2(uint32_t X) {
  if (!X) return 0;
  constexpr int G = (F + 1) / 2;
  float xf = scalbnf((float)X, -F);     // X * 2^-F
  float yf = sqrtf(xf);
  float sf = scalbnf(yf, G);            // * 2^G
  long r = lrintf(sf);                  // or nearbyintf(sf) then cast
  if (r < 0) r = 0; if (r > 0xFFFFFFFFl) r = 0xFFFFFFFFl;
  return (uint32_t)r;
}

static inline float vsqrt32(float x) {
  float r;
  asm volatile("vsqrt.f32 %0, %1" : "=t"(r) : "t"(x));  // maps to FPU sqrt
  return r;
}

template<int F>
static inline uint32_t sqrt_qF_float3(uint32_t X) {
  if (!X) return 0;
  constexpr int G = (F + 1) / 2;
  float xf = scalbnf((float)X, -F);
  float yf = vsqrt32(xf);                // ensure we don’t call libm
  float sf = scalbnf(yf, G);
  return (uint32_t)lrintf(sf);
}







// ========= Reference model & error metrics =========
inline uint32_t ref_sqrt_round_to_QG(uint32_t X, int F)
{
  if (!X) return 0;
  int G = (F + 1) / 2;
  double xf = (double)X / (double)(uint64_t(1) << F);
  double yf = sqrt(xf);
  double yq = yf * (double)(uint64_t(1) << G);
  // round to nearest, ties away from zero (positive domain)
  uint64_t r = (uint64_t) llround(yq);
  if (r > 0xFFFFFFFFull) r = 0xFFFFFFFFull;
  return (uint32_t)r;
}

// ========= PRNG =========
static inline uint32_t xorshift32(uint32_t &s) {
  uint32_t x = s;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  s = x;
  return x;
}

// ========= Benchmark harness =========

// Wrappers that bind F at compile-time into a uniform function pointer
template<int F> uint32_t call_floatK1(uint32_t X)     { return sqrt_qF_float1<F>(X); }
template<int F> uint32_t call_floatK2(uint32_t X)     { return sqrt_qF_float2<F>(X); }
template<int F> uint32_t call_floatK3(uint32_t X)     { return sqrt_qF_float3<F>(X); }
template<int F> uint32_t call_restoreK(uint32_t X)   { return sqrt_qF_restoring<F>(X); }

// List of kernels to test
KernelDesc kKernels[] = {
  { "float-assisted1",    /*make_for_F=*/nullptr, /*q32q16=*/nullptr },
  { "float-assisted2",    /*make_for_F=*/nullptr, /*q32q16=*/nullptr },
  { "float-assisted3",    /*make_for_F=*/nullptr, /*q32q16=*/nullptr },
  { "restoring (int)",   /*make_for_F=*/nullptr, /*q32q16=*/nullptr },
  { "Heron NR (div) Q32->Q16", /*make_for_F=*/nullptr, /*q32q16=*/sqrt_Q32_to_Q16_NR },
};

// Helper to install F-bound wrappers at runtime
template<int F>
void bind_kernels_for_F() {
  kKernels[0].make_for_F = &call_floatK1<F>;
  kKernels[0].make_for_F = &call_floatK2<F>;
  kKernels[0].make_for_F = &call_floatK3<F>;
  kKernels[1].make_for_F = &call_restoreK<F>;
  // kKernels[2] only supports Q32->Q16 via q32q16
}

// Test configuration
static const uint32_t kNumRandom = 100000; // random samples
static const uint32_t kNumSweep  = 65536;  // uniform sweep across range
static const bool     kDoEdge    = true;

// Edge/corner generator
void feed_edge_cases(uint32_t F, KernelFuncF fn, ErrorStats &estats) {
  const uint32_t G = (F + 1) / 2;
  const uint32_t maxX = 0xFFFFFFFFu;

  uint32_t cases[] = {
    0u, 1u, 2u, 3u, 4u, 7u, 8u, 15u, 16u, 31u, 32u, 63u, 64u,
    (1u << F), (1u << F) - 1u, (1u << (F ? (F-1) : 0)), maxX,
    maxX - 1u, maxX - 2u, (1u << 31), (1u << 30), (1u << 29),
  };
  const size_t N = sizeof(cases)/sizeof(cases[0]);

  for (size_t i = 0; i < N; ++i) {
    uint32_t X = cases[i];
    uint32_t y  = fn(X);
    uint32_t yr = ref_sqrt_round_to_QG(X, F);
    estats.add(y, yr);
  }

  // Add a few near power-of-two boundaries
  for (int b = 0; b < 32; ++b) {
    uint32_t p = (b == 31) ? 0x80000000u : (1u << b);
    uint32_t xs[] = { (p ? p-1 : 0u), p, (p < 0xFFFFFFFFu ? p+1 : p) };
    for (uint32_t x : xs) {
      uint32_t y  = fn(x);
      uint32_t yr = ref_sqrt_round_to_QG(x, F);
      estats.add(y, yr);
    }
  }

  // Near perfect squares in the output grid (k^2 in QG)
  for (uint32_t k = 1; k < 1024; k <<= 1) {
    uint64_t yraw = (uint64_t)k;              // candidate y in QG
    uint64_t xraw = (yraw * yraw) << (F - ((F + 1)/2)); // back to QF input
    if (xraw > 0xFFFFFFFFull) xraw = 0xFFFFFFFFull;
    uint32_t X = (uint32_t)xraw;
    uint32_t y  = fn(X);
    uint32_t yr = ref_sqrt_round_to_QG(X, F);
    estats.add(y, yr);
  }
}

// Benchmark a generic kernel for given F
BenchResult run_bench_F(const char* kname, uint32_t F, KernelFuncF fn)
{
  BenchResult out;
  ErrorStats &E = out.stats;

  // 1) Edge cases
  if (kDoEdge) {
    feed_edge_cases(F, fn, E);
  }

  // 2) Uniform sweep (low 16 bits) mapped across full range
  for (uint32_t i = 0; i < kNumSweep; ++i) {
    uint32_t X = (i << 16) | (i ^ 0xA5A5u); // spread bits
    uint32_t y  = fn(X);
    uint32_t yr = ref_sqrt_round_to_QG(X, F);
    E.add(y, yr);
  }

  // 3) Random samples (measure speed here)
  volatile uint32_t sink = 0;
  uint32_t seed = 0x12345678u;

  cycles_enable();
  uint32_t t0 = cycles();

  for (uint32_t i = 0; i < kNumRandom; ++i) {
    uint32_t X = xorshift32(seed);
    uint32_t y = fn(X);
    sink ^= y;
  }

  uint32_t t1 = cycles();
  (void)sink;

#if HAVE_DWT
  double total_cycles = (double)(uint32_t)(t1 - t0);
  out.cycles_per_call = total_cycles / (double)kNumRandom;
  out.micros_per_call = 0.0;
#else
  double total_us = (double)(uint32_t)(t1 - t0);
  out.cycles_per_call = NAN;
  out.micros_per_call = total_us / (double)kNumRandom;
#endif

  // Accuracy of random set (separate from speed timing)
  seed = 0x89ABCDEFu;
  for (uint32_t i = 0; i < kNumRandom; ++i) {
    uint32_t X = xorshift32(seed);
    uint32_t y  = fn(X);
    uint32_t yr = ref_sqrt_round_to_QG(X, F);
    E.add(y, yr);
  }

  // Report
  Serial.printf("\nKernel: %s  |  Q%u -> Q%u  |  Nrand=%u, Nsweep=%u\n",
                kname, F, (F + 1)/2, (unsigned)kNumRandom, (unsigned)kNumSweep);
#if HAVE_DWT
  Serial.printf("  Speed: %.2f cycles/call (DWT)\n", out.cycles_per_call);
#else
  Serial.printf("  Speed: %.3f us/call (micros fallback)\n", out.micros_per_call);
#endif
  E.print("overall");

  return out;
}

// Specialized bench for Q32->Q16 Heron kernel
BenchResult run_bench_Q32_Heron()
{
  // Wrap into KernelFuncF-like lambda by ignoring F at call sites
  auto fn = [](uint32_t X)->uint32_t { return sqrt_Q32_to_Q16_NR(X); };
  return run_bench_F("Heron NR (div) Q32->Q16", 32, fn);
}

// ========= Test plan =========
struct FormatToTest { uint32_t F; };

FormatToTest kFormatsToTest[] = {
  { 32 }, // Q32 -> Q16
  { 24 }, // Q24 -> Q12
  // Add more if desired, e.g. {16}, {20}, ...
};

// ========= Arduino setup/loop =========
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) { } // wait for USB serial

  Serial.println("\n--- Fixed-Point sqrt benchmark (Teensy 4.x / M7F) ---");
#if HAVE_DWT
  Serial.println("Timing: DWT cycle counter");
#else
  Serial.println("Timing: micros() fallback (coarser)");
#endif
  Serial.printf("Random=%u, Sweep=%u, Edge=%s\n\n",
                (unsigned)kNumRandom, (unsigned)kNumSweep, kDoEdge ? "yes" : "no");

  // Run for each requested format
  for (auto fmt : kFormatsToTest) {
    const uint32_t F = fmt.F;
    // Bind generic kernels for this F
    if (F == 32)      bind_kernels_for_F<32>();
    else if (F == 24) bind_kernels_for_F<24>();
    else if (F == 20) bind_kernels_for_F<20>();
    else if (F == 16) bind_kernels_for_F<16>();
    else {
      // Generic instantiation path (adds one more if-branch per new F you add)
      // To add a new F, copy another line above to avoid this branch in hot path.
      bind_kernels_for_F<24>(); // temporary bind; will be overwritten below
      // Rebind manually by function pointers via a small switch:
      // (For simplicity here we only pre-bind common F. Extend as needed.)
      Serial.printf("Warning: F=%u not pre-bound; skipping generic kernels for this F.\n", F);
    }

    // 1) Float-assisted
    if (F == 32) {
      BenchResult r = run_bench_F("float-assisted 1", F, call_floatK1<32>);
      (void)r;
    } else if (F == 24) {
      BenchResult r = run_bench_F("float-assisted 1", F, call_floatK1<24>);
      (void)r;
    } else if (F == 16) {
      BenchResult r = run_bench_F("float-assisted 1", F, call_floatK1<16>);
      (void)r;
    } else if (F == 20) {
      BenchResult r = run_bench_F("float-assisted 1", F, call_floatK1<20>);
      (void)r;
    }

    if (F == 32) {
      BenchResult r = run_bench_F("float-assisted 2", F, call_floatK2<32>);
      (void)r;
    } else if (F == 24) {
      BenchResult r = run_bench_F("float-assisted 2", F, call_floatK2<24>);
      (void)r;
    } else if (F == 16) {
      BenchResult r = run_bench_F("float-assisted 2", F, call_floatK2<16>);
      (void)r;
    } else if (F == 20) {
      BenchResult r = run_bench_F("float-assisted 2", F, call_floatK2<20>);
      (void)r;
    }

    if (F == 32) {
      BenchResult r = run_bench_F("float-assisted 3", F, call_floatK3<32>);
      (void)r;
    } else if (F == 24) {
      BenchResult r = run_bench_F("float-assisted 3", F, call_floatK3<24>);
      (void)r;
    } else if (F == 16) {
      BenchResult r = run_bench_F("float-assisted 3", F, call_floatK3<16>);
      (void)r;
    } else if (F == 20) {
      BenchResult r = run_bench_F("float-assisted 3", F, call_floatK3<20>);
      (void)r;
    }


    // 2) Restoring (integer)
    if (F == 32) {
      BenchResult r = run_bench_F("restoring (int)", F, call_restoreK<32>);
      (void)r;
    } else if (F == 24) {
      BenchResult r = run_bench_F("restoring (int)", F, call_restoreK<24>);
      (void)r;
    } else if (F == 16) {
      BenchResult r = run_bench_F("restoring (int)", F, call_restoreK<16>);
      (void)r;
    } else if (F == 20) {
      BenchResult r = run_bench_F("restoring (int)", F, call_restoreK<20>);
      (void)r;
    }

    // 3) Heron NR (only meaningful/optimized for Q32->Q16)
    if (F == 32) {
      BenchResult r = run_bench_Q32_Heron();
      (void)r;
    }
  }

  Serial.println("\nDone.");
}

void loop() {
  // nothing
}
