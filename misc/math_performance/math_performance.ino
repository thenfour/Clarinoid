// see: https://docs.google.com/spreadsheets/d/1opYhrJZkpHPGoa15AJ_7ssJw7-nuuq9VKS0FQ72UQwA/edit?gid=0#gid=0

// Teensy 4.0/4.1 integer microbenchmarks (cycles/op)
// Measures: latency (dependent chain) & throughput (8 independent accumulators)
// Build: default Teensyduino settings are fine. Higher -O levels expose true instruction cost.
//
// IMPORTANT: Run from ITCM (default) with caches warm. Results are cycles/op at current F_CPU.

#include <Arduino.h>
#include <imxrt.h> 
#include <arm_acle.h>
//#include <arm_math.h>
#include <stdint.h>

/* ------------------------------------------------------------------
   Minimal DSP-op wrappers for Cortex-M7 / Teensy 4.x
   ------------------------------------------------------------------ */
static inline int32_t __SMMULR(int32_t a, int32_t b)  // Q31×Q31→Q31 with rounding
{
    int32_t r;
    asm volatile ("smmulr %0, %1, %2" : "=r"(r) : "r"(a), "r"(b));
    return r;
}

static inline int32_t __SMMUL    (int32_t a, int32_t b) // truncating
{
    int32_t r;
    asm volatile ("smmul %0, %1, %2"  : "=r"(r) : "r"(a), "r"(b));
    return r;
}

static inline int32_t __SMMULAR (int32_t a, int32_t b, int32_t acc) // MAC, rounding
{
    int32_t r;
    asm volatile ("smmlar %0, %1, %2, %3"
                  : "=r"(r) : "r"(a), "r"(b), "r"(acc));
    return r;
}




// // -------------------- DWT cycle counter helpers --------------------
// static inline void dwt_enable() {
//   CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;   // enable DWT/ITM
//   DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;              // enable cycle counter
// }

// static inline uint32_t cycles() {
//   return DWT->CYCCNT;
// }


static inline void dwt_enable() {
  ARM_DEMCR |= ARM_DEMCR_TRCENA;          // enable trace/DWT
  ARM_DWT_CYCCNT = 0;                     // reset the counter
  ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA; // start counting
}

static inline uint32_t cycles() {
  return ARM_DWT_CYCCNT;                  // read current cycle count
}


// -------------------- Config --------------------
static const uint32_t ITERS = 1000000;  // iterations for latency tests
static const uint32_t ITERS_TP = 250000; // iterations for throughput (8-way => 8*ITERS_TP ops)

// Sinks to keep the compiler from optimizing work away
volatile uint32_t sink32 = 0;
volatile uint64_t sink64 = 0;

// Measure empty-loop overhead (same loop shape) to subtract control cost
static uint32_t time_empty(uint32_t iters) {
  uint32_t start = cycles();
  for (uint32_t i = 0; i < iters; ++i) {
    asm volatile("" ::: "memory"); // prevent collapsing
  }
  return cycles() - start;
}
static uint32_t time_empty_unrolled8(uint32_t iters) {
  uint32_t start = cycles();
  for (uint32_t i = 0; i < iters; ++i) {
    // model 8 independent ops
    asm volatile("" ::: "memory");
    asm volatile("" ::: "memory");
    asm volatile("" ::: "memory");
    asm volatile("" ::: "memory");
    asm volatile("" ::: "memory");
    asm volatile("" ::: "memory");
    asm volatile("" ::: "memory");
    asm volatile("" ::: "memory");
  }
  return cycles() - start;
}

// -------------------- 32-bit latency (dependent chain) --------------------
static uint32_t time_add32_lat(uint32_t iters) {
  volatile uint32_t x = 1, a = 0x9E3779B9u; // golden ratio step
  uint32_t start = cycles();
  for (uint32_t i = 0; i < iters; ++i) { x = x + a; }
  uint32_t t = cycles() - start;
  sink32 = x;
  return t;
}

static uint32_t time_sub32_lat(uint32_t iters) {
  volatile uint32_t x = 1, a = 0x9E3779B9u;
  uint32_t start = cycles();
  for (uint32_t i = 0; i < iters; ++i) { x = x - a; }
  uint32_t t = cycles() - start;
  sink32 = x;
  return t;
}

static uint32_t time_mul32_lat(uint32_t iters) {
  volatile uint32_t x = 3, m = 0x9E3779B1u;
  uint32_t start = cycles();
  for (uint32_t i = 0; i < iters; ++i)
  {
     x = x * m + 1;
  } // dependent chain
  uint32_t t = cycles() - start;
  sink32 = x;
  return t;
}

// Use explicit UDIV/SDIV to avoid clever optimizations
static uint32_t time_udiv32_lat(uint32_t iters) {
  volatile uint32_t x = 0x7EC1u;
  const uint32_t d = 123457u; // non-power-of-2, non-constant-fold (kept in reg)
  uint32_t start = cycles();
  for (uint32_t i = 0; i < iters; ++i) {
    asm volatile("udiv %0, %1, %2" : "=r"(x) : "r"(x), "r"(d));
  }
  uint32_t t = cycles() - start;
  sink32 = x;
  return t;
}

static uint32_t time_sdiv32_lat(uint32_t iters) {
  volatile int32_t x = 123456789;
  const int32_t d = -32123;
  uint32_t start = cycles();
  for (uint32_t i = 0; i < iters; ++i) {
    asm volatile("sdiv %0, %1, %2" : "=r"(x) : "r"(x), "r"(d));
  }
  uint32_t t = cycles() - start;
  sink32 = (uint32_t)x;
  return t;
}

// -------------------- 32-bit throughput (8 independent accumulators) --------------------
// static uint32_t time_add32_tp(uint32_t iters) {
//   volatile uint32_t x0=1,x1=2,x2=3,x3=4,x4=5,x5=6,x6=7,x7=8;
//   const uint32_t a = 0x9E3779B9u;
//   uint32_t start = cycles();
//   for (uint32_t i=0;i<iters;++i) {
//     x0 += a;
//      x1 += a; 
//      x2 += a;
//       x3 += a;
//        x4 += a;
//         x5 += a;
//          x6 += a;
//           x7 += a;
//   }
//   uint32_t t = cycles() - start;
//   sink32 = x0+x1+x2+x3+x4+x5+x6+x7;
//   return t;
// }

// -------------------- 32-bit throughput (8 independent accumulators) --------------------
static uint32_t time_add32_tp(uint32_t iters) {
  volatile uint32_t x0=1;
  volatile uint32_t x1=2;
  volatile uint32_t x2=3;
  volatile uint32_t x3=4;
  volatile uint32_t x4=5;
  volatile uint32_t x5=6;
  volatile uint32_t x6=7;
  volatile uint32_t x7=8;
  const uint32_t a = 0x9E3779B9u;
  uint32_t start = cycles();
  for (uint32_t i=0;i<iters;++i) {
    x0 += a;
     x1 += a; 
     x2 += a;
      x3 += a;
       x4 += a;
        x5 += a;
         x6 += a;
          x7 += a;
  }
  uint32_t t = cycles() - start;
  sink32 = x0+x1+x2+x3+x4+x5+x6+x7;
  return t;
}





static uint32_t time_sub32_tp(uint32_t iters) {
  volatile uint32_t x0=1,x1=2,x2=3,x3=4,x4=5,x5=6,x6=7,x7=8;
  const uint32_t a = 0x9E3779B9u;
  uint32_t start = cycles();
  for (uint32_t i=0;i<iters;++i) {
    x0 -= a; x1 -= a; x2 -= a; x3 -= a; x4 -= a; x5 -= a; x6 -= a; x7 -= a;
  }
  uint32_t t = cycles() - start;
  sink32 = x0+x1+x2+x3+x4+x5+x6+x7;
  return t;
}

static uint32_t time_mul32_tp(uint32_t iters) {
  volatile uint32_t x0=3,x1=5,x2=7,x3=11,x4=13,x5=17,x6=19,x7=23;
  const uint32_t m = 0x9E3779B1u;
  uint32_t start = cycles();
  for (uint32_t i=0;i<iters;++i) {
    x0 = x0 * m + 1; x1 = x1 * m + 1; x2 = x2 * m + 1; x3 = x3 * m + 1;
    x4 = x4 * m + 1; x5 = x5 * m + 1; x6 = x6 * m + 1; x7 = x7 * m + 1;
  }
  uint32_t t = cycles() - start;
  sink32 = x0+x1+x2+x3+x4+x5+x6+x7;
  return t;
}

static uint32_t time_udiv32_tp(uint32_t iters) {
  volatile uint32_t x0=0x7EC1u, x1=0x8123u, x2=0x3719u, x3=0x55AAu, x4=0xCAFEu, x5=0x1357u, x6=0x2468u, x7=0xDEADu;
  const uint32_t d = 123457u;
  uint32_t start = cycles();
  for (uint32_t i=0;i<iters;++i) {
    asm volatile("udiv %0, %0, %1" : "+r"(x0) : "r"(d));
    asm volatile("udiv %0, %0, %1" : "+r"(x1) : "r"(d));
    asm volatile("udiv %0, %0, %1" : "+r"(x2) : "r"(d));
    asm volatile("udiv %0, %0, %1" : "+r"(x3) : "r"(d));
    asm volatile("udiv %0, %0, %1" : "+r"(x4) : "r"(d));
    asm volatile("udiv %0, %0, %1" : "+r"(x5) : "r"(d));
    asm volatile("udiv %0, %0, %1" : "+r"(x6) : "r"(d));
    asm volatile("udiv %0, %0, %1" : "+r"(x7) : "r"(d));
  }
  uint32_t t = cycles() - start;
  sink32 = x0+x1+x2+x3+x4+x5+x6+x7;
  return t;
}

static uint32_t time_sdiv32_tp(uint32_t iters) {
  volatile int32_t x0= 123456789, x1=-987654321, x2=314159265, x3=-271828183;
  volatile int32_t x4= 13579, x5=-24680, x6=0x7FFFFFFF, x7=-1;
  const int32_t d = -32123;
  uint32_t start = cycles();
  for (uint32_t i=0;i<iters;++i) {
    asm volatile("sdiv %0, %0, %1" : "+r"(x0) : "r"(d));
    asm volatile("sdiv %0, %0, %1" : "+r"(x1) : "r"(d));
    asm volatile("sdiv %0, %0, %1" : "+r"(x2) : "r"(d));
    asm volatile("sdiv %0, %0, %1" : "+r"(x3) : "r"(d));
    asm volatile("sdiv %0, %0, %1" : "+r"(x4) : "r"(d));
    asm volatile("sdiv %0, %0, %1" : "+r"(x5) : "r"(d));
    asm volatile("sdiv %0, %0, %1" : "+r"(x6) : "r"(d));
    asm volatile("sdiv %0, %0, %1" : "+r"(x7) : "r"(d));
  }
  uint32_t t = cycles() - start;
  sink32 = (uint32_t)(x0+x1+x2+x3+x4+x5+x6+x7);
  return t;
}

// -------------------- 64-bit latency (dependent chain) --------------------
static uint32_t time_mul64_lat(uint32_t iters) {
  volatile uint64_t x = 5, m = 0x9E3779B97F4A7C15ULL;
  uint32_t start = cycles();
  for (uint32_t i=0;i<iters;++i) { x = x * m + 1; }
  uint32_t t = cycles() - start;
  sink64 = x;
  return t;
}

static uint32_t time_udiv64_lat(uint32_t iters) {
  volatile uint64_t x = 0x123456789ABCDEFULL;
  const volatile uint64_t d = 12345678901234567ULL; // volatile prevents const-fold/reciprocal tricks
  uint32_t start = cycles();
  for (uint32_t i=0;i<iters;++i) { x = x / d; }
  uint32_t t = cycles() - start;
  sink64 = x;
  return t;
}

static uint32_t time_sdiv64_lat(uint32_t iters) {
  volatile int64_t x = (int64_t)0x123456789ABCDEFFULL;
  const volatile int64_t d = -987654321098765LL;
  uint32_t start = cycles();
  for (uint32_t i=0;i<iters;++i) { x = x / d; }
  uint32_t t = cycles() - start;
  sink64 = (uint64_t)x;
  return t;
}

// -------------------- Saturation (SSAT/USAT) latency --------------------
static uint32_t time_ssat32_lat(uint32_t iters) {
  volatile int32_t x = 0x7F000000;
  uint32_t start = cycles();
  for (uint32_t i=0;i<iters;++i) {
    asm volatile("ssat %0, #32, %0" : "+r"(x));
  }
  uint32_t t = cycles() - start;
  sink32 = (uint32_t)x;
  return t;
}

// static uint32_t time_usat32_lat(uint32_t iters) {
//   int32_t x = -1;
//   uint32_t start = cycles();
//   for (uint32_t i=0;i<iters;++i) {
//     asm volatile("usat %0, #32, %0" : "+r"(x));
//   }
//   uint32_t t = cycles() - start;
//   sink32 = (uint32_t)x;
//   return t;
// }

static uint32_t time_ssat16_lat(uint32_t iters) {
  volatile int32_t x = 0x00018000; // value outside +/-2^15-1 range
  uint32_t start = cycles();
  for (uint32_t i=0;i<iters;++i) {
    asm volatile("ssat %0, #16, %0" : "+r"(x));
  }
  uint32_t t = cycles() - start;
  sink32 = (uint32_t)x;
  return t;
}

static uint32_t time_usat16_lat(uint32_t iters) {
  volatile int32_t x = -123456;
  uint32_t start = cycles();
  for (uint32_t i=0;i<iters;++i) {
    asm volatile("usat %0, #16, %0" : "+r"(x));
  }
  uint32_t t = cycles() - start;
  sink32 = (uint32_t)x;
  return t;
}

static uint32_t time_udiv32_lat_C(uint32_t iters) {
  volatile uint32_t x = 0x7EC1u;
  const uint32_t d  = 123457u;
  uint32_t start = cycles();
  for (uint32_t i = 0; i < iters; ++i)  x = x / d;   // plain '/'
  uint32_t t = cycles() - start;
  sink32 = x;
  return t;
}

static uint32_t time_sdiv32_lat_C(uint32_t iters) {
  volatile int32_t x = 123456789;
  const int32_t d   = -32123;
  uint32_t start = cycles();
  for (uint32_t i = 0; i < iters; ++i)  x = x / d;   // plain '/'
  uint32_t t = cycles() - start;
  sink32 = (uint32_t)x;
  return t;
}

// -------- 32-bit C-style DIV throughput (8 independent) ----------
static uint32_t time_udiv32_tp_C(uint32_t iters) {
  volatile uint32_t x0=0x7EC1u, x1=0x8123u, x2=0x3719u, x3=0x55AAu,
                    x4=0xCAFEu, x5=0x1357u, x6=0x2468u, x7=0xDEADu;
  const uint32_t d = 123457u;
  uint32_t start = cycles();
  for (uint32_t i = 0; i < iters; ++i) {
    x0 = x0 / d;  x1 = x1 / d;  x2 = x2 / d;  x3 = x3 / d;
    x4 = x4 / d;  x5 = x5 / d;  x6 = x6 / d;  x7 = x7 / d;
  }
  uint32_t t = cycles() - start;
  sink32 = x0+x1+x2+x3+x4+x5+x6+x7;
  return t;
}

static uint32_t time_sdiv32_tp_C(uint32_t iters) {
  volatile int32_t x0= 123456789, x1=-987654321, x2=314159265, x3=-271828183,
                   x4= 13579,     x5=-24680,    x6=0x7FFFFFFF, x7=-1;
  const int32_t d = -32123;
  uint32_t start = cycles();
  for (uint32_t i = 0; i < iters; ++i) {
    x0 = x0 / d;  x1 = x1 / d;  x2 = x2 / d;  x3 = x3 / d;
    x4 = x4 / d;  x5 = x5 / d;  x6 = x6 / d;  x7 = x7 / d;
  }
  uint32_t t = cycles() - start;
  sink32 = (uint32_t)(x0+x1+x2+x3+x4+x5+x6+x7);
  return t;
}


static uint32_t time_smmul_lat(uint32_t iters) {
  volatile int32_t x = 0x40000000, m = 0x60000000;   // 0.5 × 0.75
  uint32_t start = cycles();
  for (uint32_t i = 0; i < iters; ++i)
      x = __SMMULR(x, m);            // round & depend
  uint32_t t = cycles() - start;
  sink32 = x;
  return t;
}

/*****************************************************************
 *  time_smmul_tp  — Q31×Q31 → Q31 (rounding) throughput test
 *  • Uses __SMMULR intrinsic (1-cycle DSP op with rounding)
 *  • Eight independent accumulators keep both M7 pipes full
 *****************************************************************/
static uint32_t time_smmul_tp(uint32_t iters)
{
  /* Start with eight different Q31 values so the compiler
     can’t CSE them and the results won’t collapse to a pattern. */
  volatile int32_t x0 = 0x40000000, x1 = 0x50000000,
                   x2 = 0x60000000, x3 = 0x2AAAAAAA,
                   x4 = 0x10000000, x5 = 0x70000000,
                   x6 = 0x15555555, x7 = 0x3FFFFFFF;

  const int32_t  m  = 0x60000000;            // 0.75 in Q31

  uint32_t start = cycles();
  for (uint32_t i = 0; i < iters; ++i) {
    /* Each multiply is independent, so the core can overlap them.
       __SMMULR = “signed multiply, take high word, with rounding”. */
    x0 = __SMMULR(x0, m);  x1 = __SMMULR(x1, m);
    x2 = __SMMULR(x2, m);  x3 = __SMMULR(x3, m);
    x4 = __SMMULR(x4, m);  x5 = __SMMULR(x5, m);
    x6 = __SMMULR(x6, m);  x7 = __SMMULR(x7, m);
  }
  uint32_t t = cycles() - start;

  /* Prevent dead-code elimination so the multiplies really run. */
  sink32 = x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7;
  return t;
}




/*****************************************************************
 *  Q31 × Q31  → Q31  — portable reference, no DSP intrinsics
 *  (a) Truncating   (b) Rounding   (c) MAC
 *****************************************************************/
static inline int32_t q31_mul_port(int32_t a, int32_t b)          // trunc
{
    return (int32_t)(((int64_t)a * (int64_t)b) >> 31);
}

static inline int32_t q31_mul_r_port(int32_t a, int32_t b)        // round
{
    return (int32_t)(((int64_t)a * (int64_t)b + (1LL << 30)) >> 31);
}

static inline int32_t q31_mac_port(int32_t acc, int32_t a, int32_t b) // trunc MAC
{
    return acc + (int32_t)(((int64_t)a * (int64_t)b) >> 31);
}

static inline int32_t q31_mac_r_port(int32_t acc, int32_t a, int32_t b) // round MAC
{
    return acc + (int32_t)(((int64_t)a * (int64_t)b + (1LL << 30)) >> 31);
}


static uint32_t time_q31mul_port_lat(uint32_t iters)
{
    volatile int32_t x = 0x40000000;       // 0.5 in Q31
    volatile const int32_t    m = 0x60000000;       // 0.75
    uint32_t start = cycles();
    for (uint32_t i = 0; i < iters; ++i)
        x = q31_mul_r_port(x, m);          // rounding multiply
    uint32_t t = cycles() - start;
    sink32 = x;
    return t;
}

static uint32_t time_q31mul_port_tp(uint32_t iters)
{
    volatile int32_t x0=0x40000000, x1=0x50000000, x2=0x60000000, x3=0x2AAAAAAA,
                     x4=0x10000000, x5=0x70000000, x6=0x15555555, x7=0x3FFFFFFF;
    const   int32_t  m = 0x60000000;
    uint32_t start = cycles();
    for (uint32_t i = 0; i < iters; ++i) {
        x0 = q31_mul_r_port(x0, m);  x1 = q31_mul_r_port(x1, m);
        x2 = q31_mul_r_port(x2, m);  x3 = q31_mul_r_port(x3, m);
        x4 = q31_mul_r_port(x4, m);  x5 = q31_mul_r_port(x5, m);
        x6 = q31_mul_r_port(x6, m);  x7 = q31_mul_r_port(x7, m);
    }
    uint32_t t = cycles() - start;
    sink32 = x0+x1+x2+x3+x4+x5+x6+x7;
    return t;
}








/*****************************************************************
 *  time_add64_lat / time_sub64_lat
 *  • Uses a single volatile uint64_t so every iteration does:
 *      LDR + ADD/SUB + STR + branch
 *  • Matches the methodology of your existing latency tests.
 *****************************************************************/
static uint32_t time_add64_lat(uint32_t iters)
{
  volatile uint64_t x = 1ULL;
  const    uint64_t a = 0x9E3779B97F4A7C15ULL;   // large odd increment
  uint32_t start = cycles();
  for (uint32_t i = 0; i < iters; ++i)
      x = x + a;            // dependency chain
  uint32_t t = cycles() - start;
  sink64 = x;               // keep side-effect
  return t;
}

static uint32_t time_sub64_lat(uint32_t iters)
{
  volatile uint64_t x = 1ULL;
  const    uint64_t a = 0x9E3779B97F4A7C15ULL;
  uint32_t start = cycles();
  for (uint32_t i = 0; i < iters; ++i)
      x = x - a;
  uint32_t t = cycles() - start;
  sink64 = x;
  return t;
}
/*****************************************************************
 *  time_add64_tp / time_sub64_tp
 *  • Eight independent volatile accumulators  memory traffic +
 *    ALU overlapped, just like your 32-bit TP blockers.
 *****************************************************************/
static uint32_t time_add64_tp(uint32_t iters)
{
  volatile uint64_t x0=1,x1=2,x2=3,x3=4,x4=5,x5=6,x6=7,x7=8;
  const    uint64_t a = 0x9E3779B97F4A7C15ULL;
  uint32_t start = cycles();
  for (uint32_t i = 0; i < iters; ++i) {
    x0 += a; x1 += a; x2 += a; x3 += a;
    x4 += a; x5 += a; x6 += a; x7 += a;
  }
  uint32_t t = cycles() - start;
  sink64 = x0+x1+x2+x3+x4+x5+x6+x7;
  return t;
}

static uint32_t time_sub64_tp(uint32_t iters)
{
  volatile uint64_t x0=1,x1=2,x2=3,x3=4,x4=5,x5=6,x6=7,x7=8;
  const    uint64_t a = 0x9E3779B97F4A7C15ULL;
  uint32_t start = cycles();
  for (uint32_t i = 0; i < iters; ++i) {
    x0 -= a; x1 -= a; x2 -= a; x3 -= a;
    x4 -= a; x5 -= a; x6 -= a; x7 -= a;
  }
  uint32_t t = cycles() - start;
  sink64 = x0+x1+x2+x3+x4+x5+x6+x7;
  return t;
}




/*****************************************************************
 * 64-bit multiply throughput (8 independent accumulators)
 *****************************************************************/
static uint32_t time_mul64_tp(uint32_t iters)
{
  volatile uint64_t x0=3,x1=5,x2=7,x3=11,x4=13,x5=17,x6=19,x7=23;
  const    uint64_t m = 0x9E3779B97F4A7C15ULL;
  uint32_t start = cycles();
  for (uint32_t i = 0; i < iters; ++i) {
    x0 = x0 * m + 1;  x1 = x1 * m + 1;  x2 = x2 * m + 1;  x3 = x3 * m + 1;
    x4 = x4 * m + 1;  x5 = x5 * m + 1;  x6 = x6 * m + 1;  x7 = x7 * m + 1;
  }
  uint32_t t = cycles() - start;
  sink64 = x0+x1+x2+x3+x4+x5+x6+x7;
  return t;
}

/*****************************************************************
 * 64-bit unsigned divide throughput
 *  • Uses the plain ‘/’ operator; GCC emits a helper call
 *****************************************************************/
static uint32_t time_udiv64_tp(uint32_t iters)
{
  volatile uint64_t x0=0x123456789ABCDEFULL, x1=0x0FEDCBA987654321ULL,
                    x2=0xCAFEBABEDEADBEEFULL, x3=0x1337BEEFCAFED00DULL,
                    x4=0xFFEEDDCCBBAA9988ULL, x5=0x1122334455667788ULL,
                    x6=0x76543210FEDCBA98ULL, x7=0x89ABCDEF01234567ULL;
  const    uint64_t d  = 12345678901234567ULL;
  uint32_t start = cycles();
  for (uint32_t i = 0; i < iters; ++i) {
    x0 = x0 / d;  x1 = x1 / d;  x2 = x2 / d;  x3 = x3 / d;
    x4 = x4 / d;  x5 = x5 / d;  x6 = x6 / d;  x7 = x7 / d;
  }
  uint32_t t = cycles() - start;
  sink64 = x0+x1+x2+x3+x4+x5+x6+x7;
  return t;
}

/*****************************************************************
 * 64-bit signed divide throughput
 *****************************************************************/
static uint32_t time_sdiv64_tp(uint32_t iters)
{
  volatile int64_t x0 =  0x123456789ABCDEFFLL,  x1 = -0x123456789ABCDEFFLL,
                   x2 =  0x31415926535897FLL,   x3 = -0x27182818284590LL,
                   x4 =  135791357913579LL,     x5 = -24680246802468LL,
                   x6 =  0x7FFFFFFFFFFFFFFFLL,  x7 = -1LL;
  const    int64_t d  = -987654321098765LL;
  uint32_t start = cycles();
  for (uint32_t i = 0; i < iters; ++i) {
    x0 = x0 / d;  x1 = x1 / d;  x2 = x2 / d;  x3 = x3 / d;
    x4 = x4 / d;  x5 = x5 / d;  x6 = x6 / d;  x7 = x7 / d;
  }
  uint32_t t = cycles() - start;
  sink64 = (uint64_t)(x0+x1+x2+x3+x4+x5+x6+x7);
  return t;
}








/*****************************************************************
 *  Floating-point micro-ops  –  latency (dependent chain)
 *****************************************************************/
static uint32_t time_fadd_lat(uint32_t iters) {
  volatile float x = 1.0f, a = 0.618f;
  uint32_t t0 = cycles();
  for (uint32_t i = 0; i < iters; ++i) x = x + a;
  uint32_t dt = cycles() - t0;
  sink32 = *(uint32_t*)&x;              // side-effect
  return dt;
}
static uint32_t time_fsub_lat(uint32_t iters) {
  volatile float x = 1.0f, a = 0.618f;
  uint32_t t0 = cycles();
  for (uint32_t i = 0; i < iters; ++i) x = x - a;
  uint32_t dt = cycles() - t0;
  sink32 = *(uint32_t*)&x;
  return dt;
}
static uint32_t time_fmul_lat(uint32_t iters) {
  volatile float x = 1.0f, a = 1.000123f;
  uint32_t t0 = cycles();
  for (uint32_t i = 0; i < iters; ++i) x = x * a;
  uint32_t dt = cycles() - t0;
  sink32 = *(uint32_t*)&x;
  return dt;
}
static uint32_t time_fdiv_lat(uint32_t iters) {
  volatile float x = 1.0f, a = 1.000123f;
  uint32_t t0 = cycles();
  for (uint32_t i = 0; i < iters; ++i) x = x / a;
  uint32_t dt = cycles() - t0;
  sink32 = *(uint32_t*)&x;
  return dt;
}
static uint32_t time_f2i_lat(uint32_t iters) {
  volatile float x = 12345.678f;
  volatile int32_t y = 0;
  uint32_t t0 = cycles();
  for (uint32_t i = 0; i < iters; ++i) y = (int32_t)x;
  uint32_t dt = cycles() - t0;
  sink32 = y;
  return dt;
}
static uint32_t time_i2f_lat(uint32_t iters) {
  volatile int32_t x = 123456789;
  volatile float   y = 0.0f;
  uint32_t t0 = cycles();
  for (uint32_t i = 0; i < iters; ++i) y = (float)x;
  uint32_t dt = cycles() - t0;
  sink32 = *(uint32_t*)&y;
  return dt;
}

/*****************************************************************
 *  Floating-point micro-ops  –  throughput (8 independent chains)
 *****************************************************************/
static uint32_t time_fadd_tp(uint32_t iters) {
  volatile float x0=0.1f,x1=0.2f,x2=0.3f,x3=0.4f,x4=0.5f,x5=0.6f,x6=0.7f,x7=0.8f;
  const   float  a = 0.618f;
  uint32_t t0 = cycles();
  for (uint32_t i = 0; i < iters; ++i) {
    x0 += a; x1 += a; x2 += a; x3 += a;
    x4 += a; x5 += a; x6 += a; x7 += a;
  }
  uint32_t dt = cycles() - t0;
  sink32 = *(uint32_t*)&x0;            // prevent optimisation
  return dt;
}
static uint32_t time_fsub_tp(uint32_t iters) {
  volatile float x0=0.1f,x1=0.2f,x2=0.3f,x3=0.4f,x4=0.5f,x5=0.6f,x6=0.7f,x7=0.8f;
  const   float  a = 0.618f;
  uint32_t t0 = cycles();
  for (uint32_t i = 0; i < iters; ++i) {
    x0 -= a; x1 -= a; x2 -= a; x3 -= a;
    x4 -= a; x5 -= a; x6 -= a; x7 -= a;
  }
  uint32_t dt = cycles() - t0;
  sink32 = *(uint32_t*)&x0;            // prevent optimisation
  return dt;
    }
static uint32_t time_fmul_tp(uint32_t iters)
{
  volatile float x0=0.1f,x1=0.2f,x2=0.3f,x3=0.4f,x4=0.5f,x5=0.6f,x6=0.7f,x7=0.8f;
  const   float  a = 0.618f;
  uint32_t t0 = cycles();
  for (uint32_t i = 0; i < iters; ++i) {
    x0 *= a; x1 *= a; x2 *= a; x3 *= a;
    x4 *= a; x5 *= a; x6 *= a; x7 *= a;
  }
  uint32_t dt = cycles() - t0;
  sink32 = *(uint32_t*)&x0;            // prevent optimisation
  return dt;

    }

static uint32_t time_fdiv_tp(uint32_t iters) {
  volatile float x0=0.1f,x1=0.2f,x2=0.3f,x3=0.4f,x4=0.5f,x5=0.6f,x6=0.7f,x7=0.8f;
  const   float  a = 0.618f;
  uint32_t t0 = cycles();
  for (uint32_t i = 0; i < iters; ++i) {
    x0 /= a; x1 /= a; x2 /= a; x3 /= a;
    x4 /= a; x5 /= a; x6 /= a; x7 /= a;
  }
  uint32_t dt = cycles() - t0;
  sink32 = *(uint32_t*)&x0;            // prevent optimisation
  return dt;

   }

static uint32_t time_f2i_tp(uint32_t iters) {
  volatile float x0=1.1f,x1=2.2f,x2=3.3f,x3=4.4f,x4=5.5f,x5=6.6f,x6=7.7f,x7=8.8f;
  volatile int32_t y0,y1,y2,y3,y4,y5,y6,y7;
  uint32_t t0 = cycles();
  for (uint32_t i = 0; i < iters; ++i) {
    y0 = (int32_t)x0; y1 = (int32_t)x1; y2 = (int32_t)x2; y3 = (int32_t)x3;
    y4 = (int32_t)x4; y5 = (int32_t)x5; y6 = (int32_t)x6; y7 = (int32_t)x7;
  }
  uint32_t dt = cycles() - t0;
  sink32 = y0+y1+y2+y3+y4+y5+y6+y7;
  return dt;
}
static uint32_t time_i2f_tp(uint32_t iters) {
  volatile int32_t x0=1,x1=2,x2=3,x3=4,x4=5,x5=6,x6=7,x7=8;
  volatile float y0,y1,y2,y3,y4,y5,y6,y7;
  uint32_t t0 = cycles();
  for (uint32_t i = 0; i < iters; ++i) {
    y0 = (float)x0; y1 = (float)x1; y2 = (float)x2; y3 = (float)x3;
    y4 = (float)x4; y5 = (float)x5; y6 = (float)x6; y7 = (float)x7;
  }
  uint32_t dt = cycles() - t0;
  sink32 = *(uint32_t*)&y0;
  return dt;
}















// -------------------- Reporting helpers --------------------
static void print_result(const char* label, uint32_t measured, uint32_t empty, uint32_t iters, uint32_t ops_per_iter) {
  float cycles_per_op = float(measured - empty) / float(iters * ops_per_iter);
  Serial.printf("%-18s : %10.3f cyc/op\n", label, cycles_per_op);
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) { /* wait for USB */ }

  dwt_enable();
  //DWT->CYCCNT = 0; // clear once
  ARM_DWT_CYCCNT = 0;

  Serial.println();
  Serial.println("=== Teensy 4.x Integer Microbenchmarks ===");
  Serial.printf("F_CPU = %lu Hz\n", (unsigned long)F_CPU);
  Serial.println("Results are cycles/op (lower is better).");
  Serial.println();

  // Warm-up to fill caches
  (void)time_empty(1000);

  // -------------------- 32-bit latency --------------------
  Serial.println("[32-bit latency]");
  uint32_t overhead = time_empty(ITERS);
  print_result("add32 (lat)",  time_add32_lat(ITERS),  overhead, ITERS, 1);
  print_result("sub32 (lat)",  time_sub32_lat(ITERS),  overhead, ITERS, 1);
  print_result("mul32 (lat)",  time_mul32_lat(ITERS),  overhead, ITERS, 1);
  print_result("udiv32 (lat)", time_udiv32_lat(ITERS), overhead, ITERS, 1);
  print_result("sdiv32 (lat)", time_sdiv32_lat(ITERS), overhead, ITERS, 1);
  print_result("udiv32 (lat, /)", time_udiv32_lat_C(ITERS), overhead, ITERS, 1);
print_result("sdiv32 (lat, /)", time_sdiv32_lat_C(ITERS), overhead, ITERS, 1);
print_result("smmulr (lat)", time_smmul_lat(ITERS), overhead, ITERS, 1);
print_result("q31mul (lat)", time_q31mul_port_lat(ITERS), overhead, ITERS, 1);
  Serial.println();

  // -------------------- 32-bit throughput --------------------
  Serial.println("[32-bit throughput, 8 independent chains]");
  uint32_t overhead8 = time_empty_unrolled8(ITERS_TP);
  print_result("add32 (tp)",  time_add32_tp(ITERS_TP),  overhead8, ITERS_TP, 8);
  print_result("sub32 (tp)",  time_sub32_tp(ITERS_TP),  overhead8, ITERS_TP, 8);
  print_result("mul32 (tp)",  time_mul32_tp(ITERS_TP),  overhead8, ITERS_TP, 8);
  print_result("udiv32 (tp)", time_udiv32_tp(ITERS_TP), overhead8, ITERS_TP, 8);
  print_result("sdiv32 (tp)", time_sdiv32_tp(ITERS_TP), overhead8, ITERS_TP, 8);
print_result("udiv32 (tp, /)",  time_udiv32_tp_C(ITERS_TP), overhead8, ITERS_TP, 8);
print_result("sdiv32 (tp, /)",  time_sdiv32_tp_C(ITERS_TP), overhead8, ITERS_TP, 8);
print_result("SMMULR (tp)", time_smmul_tp(ITERS_TP), overhead8, ITERS_TP, 8);
print_result("q31mul (tp)",  time_q31mul_port_tp(ITERS_TP), overhead8, ITERS_TP, 8);
Serial.println();

  // -------------------- 64-bit latency --------------------
  Serial.println("[64-bit latency]");
  overhead = time_empty(ITERS);
  print_result("mul64 (lat)",  time_mul64_lat(ITERS),  overhead, ITERS, 1);
  print_result("udiv64 (lat)", time_udiv64_lat(ITERS), overhead, ITERS, 1);
  print_result("sdiv64 (lat)", time_sdiv64_lat(ITERS), overhead, ITERS, 1);
  Serial.println();
print_result("add64 (lat)", time_add64_lat(ITERS), overhead, ITERS, 1);
print_result("sub64 (lat)", time_sub64_lat(ITERS), overhead, ITERS, 1);
print_result("add64 (tp)", time_add64_tp(ITERS_TP), overhead8, ITERS_TP, 8);
print_result("sub64 (tp)", time_sub64_tp(ITERS_TP), overhead8, ITERS_TP, 8);
print_result("mul64 (tp)",  time_mul64_tp(ITERS_TP),  overhead8, ITERS_TP, 8);
print_result("udiv64 (tp)", time_udiv64_tp(ITERS_TP), overhead8, ITERS_TP, 8);
print_result("sdiv64 (tp)", time_sdiv64_tp(ITERS_TP), overhead8, ITERS_TP, 8);

  Serial.println();

  // -------------------- Saturation latency --------------------
  Serial.println("[Saturation latency]");
  overhead = time_empty(ITERS);
  print_result("ssat32 (lat)", time_ssat32_lat(ITERS), overhead, ITERS, 1);
  //print_result("usat32 (lat)", time_usat32_lat(ITERS), overhead, ITERS, 1);
  print_result("ssat16 (lat)", time_ssat16_lat(ITERS), overhead, ITERS, 1);
  print_result("usat16 (lat)", time_usat16_lat(ITERS), overhead, ITERS, 1);

Serial.println();


/******** 32-bit float latency ********/
Serial.println("[float latency]");
uint32_t oh = time_empty(ITERS);
print_result("fadd  (lat)",  time_fadd_lat(ITERS) , oh, ITERS, 1);
//print_result("fsub  (lat)",  time_fsub_lat(ITERS) , oh, ITERS, 1);
print_result("fmul  (lat)",  time_fmul_lat(ITERS) , oh, ITERS, 1);
print_result("fdiv  (lat)",  time_fdiv_lat(ITERS) , oh, ITERS, 1);
print_result("f2i   (lat)",  time_f2i_lat (ITERS) , oh, ITERS, 1);
print_result("i2f   (lat)",  time_i2f_lat (ITERS) , oh, ITERS, 1);
Serial.println();

/******** float throughput (8 chains) ********/
Serial.println("[float throughput, 8 independent]");
uint32_t oh8 = time_empty_unrolled8(ITERS_TP);
print_result("fadd  (tp)",  time_fadd_tp(ITERS_TP), oh8, ITERS_TP, 8);
print_result("fsub  (tp)",  time_fsub_tp(ITERS_TP), oh8, ITERS_TP, 8);
print_result("fmul  (tp)",  time_fmul_tp(ITERS_TP), oh8, ITERS_TP, 8);
print_result("fdiv  (tp)",  time_fdiv_tp(ITERS_TP), oh8, ITERS_TP, 8);
print_result("f2i   (tp)",  time_f2i_tp (ITERS_TP), oh8, ITERS_TP, 8);
print_result("i2f   (tp)",  time_i2f_tp (ITERS_TP), oh8, ITERS_TP, 8);
Serial.println();




  Serial.println("\nDone.");
}

void loop() {
  // nothing
}
