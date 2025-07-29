/********************************************************************
 * Teensy 4.x — audio-gain micro-benchmark
 * Compares:
 *   1) plain Q15  (16-bit)
 *   2) SIMD  Q15  (two 16-bit lanes per DSP op)
 *   3) plain Q31  (32-bit)
 *   4) float32
 *
 * Prints cycles / stereo frame   (lower = better)
 ********************************************************************/
#include <Arduino.h>

/* ---------- DWT cycle counter helpers (Teensy style) ---------- */
static inline void dwt_enable() {
  ARM_DEMCR |= ARM_DEMCR_TRCENA;
  ARM_DWT_CYCCNT = 0;
  ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;
}
static inline uint32_t cycles() { return ARM_DWT_CYCCNT; }

/* ---------- Test buffer ---------- */
constexpr uint32_t  FRAMES = 512;                 // stereo frames
constexpr uint32_t  BYTES_Q15 = FRAMES * 2 * sizeof(int16_t);
constexpr uint32_t  BYTES_Q31 = FRAMES * 2 * sizeof(int32_t);

// DTCM by default → fastest path
DMAMEM __attribute__((aligned(32)))
static int16_t  buf_q15[FRAMES * 2];
DMAMEM __attribute__((aligned(32)))
static int32_t  buf_q31[FRAMES * 2];
DMAMEM __attribute__((aligned(32)))
static float buf_f32[FRAMES * 2];


static void init_buf_f32() {
  for (uint32_t i = 0; i < FRAMES * 2; ++i)
    buf_f32[i] = float((int16_t)((i * 101) & 0xFFFF)) / 32768.0f;   // ~-1 … +1
}


/* Fill with a simple sawtooth so the compiler can’t constant-prop */
static void init_buffers() {
  for (uint32_t i = 0; i < FRAMES * 2; ++i) {
    buf_q15[i] =  (int16_t)( (i * 101) & 0xFFFF );
    buf_q31[i] = ((int32_t)buf_q15[i]) << 16;     // reuse pattern
  }
}

/* ---------- Q15 scalar implementation ---------- */
static uint32_t time_gain_q15_scalar() {
  volatile int16_t *p = buf_q15;
  const int32_t gain = 0x6000;                    // 0.75 in Q15

  uint32_t t0 = cycles();
  for (uint32_t i = 0; i < FRAMES; ++i) {
    int32_t L = *p;           L = (L * gain) >> 15;  *p++ = (int16_t)L;
    int32_t R = *p;           R = (R * gain) >> 15;  *p++ = (int16_t)R;
  }
  return cycles() - t0;
}

/* ---------- Q15 SIMD (DSP extension) ---------- */
static inline int32_t smulbb(int32_t a, int32_t b) {
  int32_t r; asm volatile ("smulbb %0, %1, %2" : "=r"(r) : "r"(a), "r"(b)); return r;
}
static inline int32_t smultt(int32_t a, int32_t b) {
  int32_t r; asm volatile ("smultt %0, %1, %2" : "=r"(r) : "r"(a), "r"(b)); return r;
}

static uint32_t time_gain_q15_simd() {
  volatile int32_t *p = (int32_t*)buf_q15;        // two samples per word
  const int32_t gain2 = (0x6000 << 16) | 0x6000;  // gain duplicated in both halves

  uint32_t t0 = cycles();
  for (uint32_t i = 0; i < FRAMES; ++i) {
    int32_t stereo = *p;                          // L|R packed
    int32_t L = smulbb(stereo, gain2);            // lower  half × gain
    int32_t R = smultt(stereo, gain2);            // upper half × gain
    *p++ = (R & 0xFFFF0000) | ((L >> 16) & 0xFFFF);
  }
  return cycles() - t0;
}

/* ---------- Q31 scalar implementation ---------- */
static uint32_t time_gain_q31_scalar() {
  volatile int32_t *p = buf_q31;
  const int64_t gain = 0x60000000LL;              // 0.75 in Q31

  uint32_t t0 = cycles();
  for (uint32_t i = 0; i < FRAMES; ++i) {
    int64_t L = *p;          L = (L * gain) >> 31;  *p++ = (int32_t)L;
    int64_t R = *p;          R = (R * gain) >> 31;  *p++ = (int32_t)R;
  }
  return cycles() - t0;
}



/* ---------- float (32-bit) gain ---------- */
static uint32_t time_gain_f32()
{
  volatile float *p = buf_f32;
  const    float  gain = 0.75f;

  uint32_t t0 = cycles();
  for (uint32_t i = 0; i < FRAMES; ++i) {
    float L = *p;  L *= gain;  *p++ = L;
    float R = *p;  R *= gain;  *p++ = R;
  }
  return cycles() - t0;
}




/* ---------- Harness ---------- */
static void print_result(const char *label, uint32_t clocks) {
  float c_per_fr = float(clocks) / FRAMES;
  Serial.printf("%-18s : %8.3f cyc/frame\n", label, c_per_fr);
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) { }
  dwt_enable();
  init_buffers();

  Serial.println("\n=== Stereo gain (512 frames) ===");

  uint32_t t;

  init_buffers();
  t = time_gain_q15_scalar();
  print_result("Q15 scalar",  t);

  init_buffers();
  t = time_gain_q15_simd();
  print_result("Q15 SIMD",    t);

  init_buffers();
  t = time_gain_q31_scalar();
  print_result("Q31 scalar",  t);

init_buf_f32();
t = time_gain_f32();
print_result("float32", t);

  Serial.println("\nDone.");
}

void loop(){}
