#pragma once
#include <stdint.h>


#if defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) || \
    (defined(__ARM_ARCH_7EM__) && defined(__CORTEX_M) && (__CORTEX_M == 7))
  #define HAVE_DWT 1
#else
  #define HAVE_DWT 0
#endif


#if HAVE_DWT
static inline void cycles_enable() {
  ARM_DEMCR |= ARM_DEMCR_TRCENA;
  ARM_DWT_CYCCNT = 0;
  ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;
}
static inline uint32_t cycles() { return ARM_DWT_CYCCNT; }
#else
static inline void cycles_enable() {}
static inline uint32_t cycles() { return micros(); }
#endif




typedef uint32_t (*KernelFuncF)(uint32_t X);
typedef uint32_t (*KernelFuncQ32toQ16)(uint32_t X);

struct ErrorStats {
  uint64_t count = 0;
  uint64_t abs_ulp_sum = 0;
  long double abs_ulp_sq_sum = 0;
  uint32_t abs_ulp_max = 0;

  // relative error stats (ppm), skip cases where ref==0 to avoid div-by-zero
  uint64_t rel_count = 0;
  long double rel_ppm_sum = 0;
  long double rel_ppm_sq_sum = 0;
  uint32_t rel_ppm_max = 0;

  void add(uint32_t y, uint32_t yref) {
    uint32_t ulp = (y > yref) ? (y - yref) : (yref - y);
    abs_ulp_sum += ulp;
    abs_ulp_sq_sum += (long double)ulp * (long double)ulp;
    if (ulp > abs_ulp_max) abs_ulp_max = ulp;
    ++count;

    if (yref != 0) {
      // relative error in ppm: (|y - yref| / yref) * 1e6
      uint64_t num = (y > yref) ? (y - yref) : (yref - y);
      long double ppm = ((long double)num * 1000000.0L) / (long double)yref;
      rel_ppm_sum += ppm;
      rel_ppm_sq_sum += ppm * ppm;
      if ((uint32_t)ppm > rel_ppm_max) rel_ppm_max = (uint32_t)ppm;
      ++rel_count;
    }
  }

  void print(const char* label) const {
    long double mean_ulp = (count ? (long double)abs_ulp_sum / (long double)count : 0.0L);
    long double rms_ulp  = (count ? sqrt((long double)abs_ulp_sq_sum / (long double)count) : 0.0L);
    long double mean_rel_ppm = (rel_count ? (long double)rel_ppm_sum / (long double)rel_count : 0.0L);
    long double rms_rel_ppm  = (rel_count ? sqrt((long double)rel_ppm_sq_sum / (long double)rel_count) : 0.0L);

    Serial.printf("  [%s] Accuracy:\n", label);
    Serial.printf("    Abs error (ULP): max=%u  mean=%.3Lf  rms=%.3Lf\n",
                  abs_ulp_max, mean_ulp, rms_ulp);
    Serial.printf("    Rel error (ppm): max=%u  mean=%.3Lf  rms=%.3Lf  (excluding ref=0)\n",
                  rel_ppm_max, mean_rel_ppm, rms_rel_ppm);
  }
};




struct BenchResult {
  ErrorStats stats;
  double cycles_per_call = 0.0;
  double micros_per_call = 0.0; // filled if DWT not available
};

// Function signatures for kernels
// typedef uint32_t (*KernelFuncF)(uint32_t X);          // generic QF->Qceil(F/2)
// typedef uint32_t (*KernelFuncQ32toQ16)(uint32_t X);   // specialized Heron kernel

struct KernelDesc {
  const char* name;
  // Generic per-F kernel; if null, this kernel isn't available for that F
  KernelFuncF make_for_F;
  // Optional specialized Q32->Q16 kernel
  KernelFuncQ32toQ16 q32q16;
};



