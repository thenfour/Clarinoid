/**********************************************************************
 *  Stereo biquad benchmark — Teensy 4.x (Cortex-M7, 600 MHz default)
 *  Author: ChatGPT, July 2025
 *---------------------------------------------------------------------
 *  Five variants timed (cycles / stereo frame):
 *    1) Q15 scalar        3) Q31 scalar
 *    2) Q15 SIMD          4) float32
 *                         5) int64 (reference / “too fancy”)
 *-------------------------------------------------------------------*/
#include <Arduino.h>


#if defined(__ARM_FEATURE_DSP) && defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
/*------------------------------  fast path  ------------------------------*/
#define _SMMUL_BODY(inst, a, b, out) \
    __asm__ volatile (inst "  %0, %1, %2" : "=r"(out) : "r"(a), "r"(b))

#define _SMMLA_BODY(inst, a, b, c, out) \
    __asm__ volatile (inst "  %0, %1, %2, %3" : "=r"(out) : "r"(a), "r"(b), "r"(c))

static inline int32_t smmul  (int32_t a, int32_t b)                { int32_t r; _SMMUL_BODY("smmul",  a,b,r); return r; }
static inline int32_t smmulr (int32_t a, int32_t b)                { int32_t r; _SMMUL_BODY("smmulr", a,b,r); return r; }
static inline int32_t smmla  (int32_t a, int32_t b, int32_t acc)   { int32_t r; _SMMLA_BODY("smmla",  a,b,acc,r); return r; }
static inline int32_t smmlar (int32_t a, int32_t b, int32_t acc)   { int32_t r; _SMMLA_BODY("smmlar", a,b,acc,r); return r; }
static inline int32_t smmls  (int32_t a, int32_t b, int32_t acc)   { int32_t r; _SMMLA_BODY("smmls",  a,b,acc,r); return r; }
static inline int32_t smmlsr (int32_t a, int32_t b, int32_t acc)   { int32_t r; _SMMLA_BODY("smmlsr", a,b,acc,r); return r; }


#endif

/*  ----------------------------------------------------------------
    ssat()  – saturate a signed integer to ±(2^(sat_bits-1)-1)
    • val       : value to clamp
    • sat_bits  : 1…32 (must be an *immediate* for the assembler)
    • returns   : val clipped to the range [-2^(sat_bits-1), 2^(sat_bits-1)-1]
    • cortex-M4/M7 instruction:  SSAT Rd, #sat_bits, Rn
   ---------------------------------------------------------------- */
static inline int32_t __SSAT(int32_t val, const int sat_bits)
{
    int32_t r;
    asm volatile("ssat %0, %1, %2"
                 : "=r"(r)                 /* output  */
                 : "I"(sat_bits), "r"(val) /* inputs  */
                 : /* no clobbers */);
    return r;
}

// pure C++ fallback
static inline int32_t ssat_c(int32_t val, int sat_bits)
{
    const int32_t max =  (1LL << (sat_bits-1)) - 1;
    const int32_t min = -(1LL << (sat_bits-1));
    if (val > max) return max;
    if (val < min) return min;
    return val;
}



/* ---------- Cycle counter helpers ---------- */
static inline void dwt_enable() {
  ARM_DEMCR |= ARM_DEMCR_TRCENA;
  ARM_DWT_CYCCNT = 0;
  ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;
}
static inline uint32_t cycles() { return ARM_DWT_CYCCNT; }

/* ---------- Benchmark parameters ---------- */
constexpr uint32_t FRAMES = 512;       // stereo frames per run
constexpr float    FS_HZ  = 48000.0f;  // sample-rate used for coef calc

/* ---------- Test buffers in DTCM ---------- */
DMAMEM __attribute__((aligned(32)))
int16_t buf_q15[FRAMES * 2];

DMAMEM __attribute__((aligned(32)))
int32_t buf_q31[FRAMES * 2];

DMAMEM __attribute__((aligned(32)))
float   buf_f32[FRAMES * 2];

/* ---------- Sawtooth fill so compiler can't fold ---------- */
static void fill_buffers() {
  for (uint32_t i = 0; i < FRAMES * 2; ++i) {
    int16_t s16 = (int16_t)((i * 101u) & 0xFFFF);
    buf_q15[i] = s16;
    buf_q31[i] = int32_t(s16) << 16;        // reuse pattern
    buf_f32[i] = float(s16) / 32768.0f;
  }
}

/* ------------------------------------------------------------------ */
/*  Biquad coefficient helpers  (direct-form I)                       */
/*  Simple 1 kHz low-pass, Q=0.707 (Butterworth) @ 48 kHz             */
/* ------------------------------------------------------------------ */
struct BiquadF32
{
  float b0,b1,b2,a1,a2;
};

int a = 0;

inline auto lp1k_f32()
{
  float w0 = 2.0f * PI * 1000.0f / FS_HZ;
  float cosw = cosf(w0), sinw = sinf(w0);
  float alpha = sinw / 1.41421356f;          // Q = √½
  float a0 = 1.0f + alpha;
  float b0 = (1.0f - cosw) / 2.0f / a0;
  float b1 =  (1.0f - cosw)       / a0;
  float b2 = b0;
  float a1 = -2.0f * cosw        / a0;
  float a2 = (1.0f - alpha)      / a0;
  return BiquadF32{b0,b1,b2,a1,a2};
}

/* ---------- Fixed-point coefficient packs ---------- */
struct BiquadQ15 { int16_t b0,b1,b2,a1,a2; int32_t z1,z2; };
struct BiquadQ31 { int32_t b0,b1,b2,a1,a2; int64_t z1,z2; };
struct BiquadI64 { int64_t b0,b1,b2,a1,a2; int64_t z1,z2; };

static BiquadQ15  coef_q15;
static BiquadQ31  coef_q31;
static BiquadI64  coef_i64;
static BiquadF32  coef_f32;

/* ---------- DSP-extension wrappers for half-word MAC ---------- */
static inline int32_t smlad (int32_t a, int32_t b, int32_t c)
{ int32_t r; asm volatile("smlad %0,%1,%2,%3" : "=r"(r) : "r"(a), "r"(b), "r"(c)); return r; }

/* ========== Variant 1 : Q15 scalar ================================= */
static uint32_t time_bq_q15_scalar()
{
  volatile int16_t *p = buf_q15;
  BiquadQ15 biqL = coef_q15, biqR = coef_q15;

  uint32_t t0 = cycles();
  for (uint32_t n = 0; n < FRAMES; ++n) {
    /* ----- Left ----- */
    int32_t xn = *p;
    int32_t acc = (xn * biqL.b0)       + biqL.z1;               // 1
    biqL.z1  = (xn * biqL.b1) + biqL.z2
             - ((acc * biqL.a1) >> 15);                         // 2
    biqL.z2  = (xn * biqL.b2)
             - ((acc * biqL.a2) >> 15);                         // 3
    *p++ = int16_t(__SSAT(acc >> 15,16));

    /* ----- Right ----- */
    xn  = *p;
    acc = (xn * biqR.b0)       + biqR.z1;
    biqR.z1 = (xn * biqR.b1) + biqR.z2
            - ((acc * biqR.a1) >> 15);
    biqR.z2 = (xn * biqR.b2)
            - ((acc * biqR.a2) >> 15);
    *p++ = int16_t(__SSAT(acc >> 15,16));
  }
  uint32_t dt = cycles() - t0;
  coef_q15.z1 = biqL.z1 + biqR.z1;      // keep side-effect
  return dt;
}

/* ========== Variant 2 : Q15 SIMD ================================== */
/*  Two 16-bit samples packed in one 32-bit word.                     */
static uint32_t time_bq_q15_simd()
{
  volatile int32_t *p = (int32_t*)buf_q15;   // L|R packed
  /* Pack coefficients: b0L|b0R, etc.  Both channels identical so duplicate. */
  int32_t b0 = (coef_q15.b0 << 16) | (uint16_t)coef_q15.b0;
  int32_t b1 = (coef_q15.b1 << 16) | (uint16_t)coef_q15.b1;
  int32_t b2 = (coef_q15.b2 << 16) | (uint16_t)coef_q15.b2;
  int32_t a1 = (coef_q15.a1 << 16) | (uint16_t)coef_q15.a1;
  int32_t a2 = (coef_q15.a2 << 16) | (uint16_t)coef_q15.a2;
  int32_t z1 = 0, z2 = 0;

  uint32_t t0 = cycles();
  for (uint32_t n = 0; n < FRAMES; ++n) {
    int32_t xn   = *p;
    int32_t acc  = smlad(b0, xn, z1);                 // (b0*x)+z1
    z1           = smlad(b1, xn, z2) - smlad(a1, acc, 0);
    z2           =                - smlad(a2, acc, smlad(b2, xn, 0));
    /* acc holds L|R Q30; narrow, repack */
    int32_t out  = ((acc & 0xFFFF0000)      ) |      // R
                   ((acc >> 16) & 0xFFFF);
    *p++ = out;
  }
  uint32_t dt = cycles() - t0;
  coef_q15.z1 = z1;                      // prevent optimise-away
  return dt;
}

/* ========== Variant 3 : Q31 scalar ================================ */
static uint32_t time_bq_q31_scalar()
{
  volatile int32_t *p = buf_q31;
  BiquadQ31 biqL = coef_q31, biqR = coef_q31;

  uint32_t t0 = cycles();
  for (uint32_t n = 0; n < FRAMES; ++n) {
    int64_t xn = *p;
    int64_t acc = ((int64_t)xn * biqL.b0 >> 31) + biqL.z1;
    biqL.z1 = ((int64_t)xn * biqL.b1 >> 31) + biqL.z2
            - ((acc * biqL.a1) >> 31);
    biqL.z2 = ((int64_t)xn * biqL.b2 >> 31)
            - ((acc * biqL.a2) >> 31);
    *p++ = int32_t(acc);

    xn   = *p;
    acc  = ((int64_t)xn * biqR.b0 >> 31) + biqR.z1;
    biqR.z1 = ((int64_t)xn * biqR.b1 >> 31) + biqR.z2
            - ((acc * biqR.a1) >> 31);
    biqR.z2 = ((int64_t)xn * biqR.b2 >> 31)
            - ((acc * biqR.a2) >> 31);
    *p++ = int32_t(acc);
  }
  uint32_t dt = cycles() - t0;
  coef_q31.z1 = biqL.z1 + biqR.z1;
  return dt;
}

/*****************************************************************
 *  Variant 3b : Q31-DSP (SMMUL/SMMLA based)
 *****************************************************************/
static uint32_t time_bq_q31_dsp()
{
  volatile int32_t *p = buf_q31;
  /* Local biquad copies with 32-bit state (direct-form II transposed) */
  int32_t b0 = coef_q31.b0, b1 = coef_q31.b1, b2 = coef_q31.b2;
  int32_t a1 = coef_q31.a1, a2 = coef_q31.a2;
  int32_t z1L = 0, z2L = 0, z1R = 0, z2R = 0;

  uint32_t t0 = cycles();
  for (uint32_t n = 0; n < FRAMES; ++n) {
    /* ------------ Left channel ------------ */
    int32_t xn = *p;
    int32_t acc = smmlar(xn, b0, z1L);          // y = b0*x + z1   (round)
    z1L = smmlar(xn, b1, z2L) - smmlar(acc, a1, 0);
    z2L =                      - smmlar(acc, a2, smmlar(xn, b2, 0));
    *p++ = acc;

    /* ------------ Right channel ----------- */
    xn   = *p;
    acc  = smmlar(xn, b0, z1R);
    z1R  = smmlar(xn, b1, z2R) - smmlar(acc, a1, 0);
    z2R  =                     - smmlar(acc, a2, smmlar(xn, b2, 0));
    *p++ = acc;
  }
  uint32_t dt = cycles() - t0;
  coef_q31.z1 = z1L + z1R;                       // keep side-effect
  return dt;
}



/* ========== Variant 4 : float32 =================================== */
static uint32_t time_bq_f32()
{
  volatile float *p = buf_f32;
  BiquadF32 b = coef_f32;

  uint32_t t0 = cycles();
  float z1L=0,z2L=0,z1R=0,z2R=0;
  for (uint32_t n = 0; n < FRAMES; ++n) {
    float xn = *p;
    float y  = b.b0*xn + z1L;
    z1L      = b.b1*xn + z2L - b.a1*y;
    z2L      = b.b2*xn         - b.a2*y;
    *p++     = y;

    xn  = *p;
    y   = b.b0*xn + z1R;
    z1R = b.b1*xn + z2R - b.a1*y;
    z2R = b.b2*xn         - b.a2*y;
    *p++ = y;
  }
  uint32_t dt = cycles() - t0;
  coef_f32.a1 = z1L + z1R;               // keep side-effect
  return dt;
}

/* ========== Variant 5 : int64 luxury ============================== */
static uint32_t time_bq_i64()
{
  volatile int32_t *p = buf_q31;
  BiquadI64 b = coef_i64;

  uint32_t t0 = cycles();
  for (uint32_t n = 0; n < FRAMES; ++n) {
    int64_t xn = *p;
    int64_t y  = (b.b0 * xn + b.z1) >> 31;
    b.z1       = (b.b1 * xn + b.z2 - (b.a1 * y >> 31));
    b.z2       = (b.b2 * xn        - (b.a2 * y >> 31));
    *p++       = int32_t(y);

    xn = *p;
    y  = (b.b0 * xn + b.z1) >> 31;
    b.z1 = (b.b1 * xn + b.z2 - (b.a1 * y >> 31));
    b.z2 = (b.b2 * xn        - (b.a2 * y >> 31));
    *p++ = int32_t(y);
  }
  uint32_t dt = cycles() - t0;
  coef_i64.z1 = b.z1;
  return dt;
}

/* ---------- Pretty-print helper ---------- */
static void pr(const char* tag, uint32_t clk)
{ Serial.printf("%-12s : %8.3f cyc/frame\n", tag, float(clk)/FRAMES); }

/* ========== Setup / run ==================================================== */
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) { }
  dwt_enable();

  /* --- prepare coefficients in all formats --- */
  coef_f32 = lp1k_f32();

  coef_q15.b0 = int16_t(lroundf(coef_f32.b0 * 32768.0f));
  coef_q15.b1 = int16_t(lroundf(coef_f32.b1 * 32768.0f));
  coef_q15.b2 = coef_q15.b0;
  coef_q15.a1 = int16_t(lroundf(coef_f32.a1 * -32768.0f));
  coef_q15.a2 = int16_t(lroundf(coef_f32.a2 * -32768.0f));

  coef_q31.b0 = int32_t(lroundf(coef_f32.b0 * 2147483648.0f));
  coef_q31.b1 = int32_t(lroundf(coef_f32.b1 * 2147483648.0f));
  coef_q31.b2 = coef_q31.b0;
  coef_q31.a1 = int32_t(lroundf(coef_f32.a1 * -2147483648.0f));
  coef_q31.a2 = int32_t(lroundf(coef_f32.a2 * -2147483648.0f));

  coef_i64.b0 = int64_t(coef_q31.b0);
  coef_i64.b1 = int64_t(coef_q31.b1);
  coef_i64.b2 = int64_t(coef_q31.b2);
  coef_i64.a1 = int64_t(coef_q31.a1);
  coef_i64.a2 = int64_t(coef_q31.a2);

  Serial.println("\n=== Stereo biquad, 512 frames ===");

  uint32_t t;

  fill_buffers(); t = time_bq_q15_scalar(); pr("Q15 scalar",  t);
  fill_buffers(); t = time_bq_q15_simd();   pr("Q15 SIMD",    t);
  fill_buffers(); t = time_bq_q31_scalar(); pr("Q31 scalar",  t);
  fill_buffers(); t = time_bq_q31_dsp();    pr("Q31 DSP",  t);
  fill_buffers(); t = time_bq_f32();        pr("float32",     t);
  fill_buffers(); t = time_bq_i64();        pr("int64",       t);

  Serial.println("\nDone.");
}

void loop() {}
