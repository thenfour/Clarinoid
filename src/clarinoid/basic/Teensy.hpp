#pragma once

//#include "imxrt.h"
//#include "arm_math.h"

#include <cstdint>


/*========================= Timing (Cortex-M7 DWT) =========================*/
#if defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) ||                                                          \
    (defined(__ARM_ARCH_7EM__) && defined(__CORTEX_M) && (__CORTEX_M == 7))
  #define HAVE_DWT 1
#else
  #define HAVE_DWT 0
#endif


namespace clarinoid
{
// provide access to Teensy-specific features, provide fallbacks if not available.
struct Teensy
{
  static inline void cycles_enable()
  {
    ARM_DEMCR |= ARM_DEMCR_TRCENA;           // enable trace/DWT
    ARM_DWT_CYCCNT = 0;                      // reset the counter
    ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;  // start counting
  }
  static inline uint32_t cycles_now()
  {
    return ARM_DWT_CYCCNT;  // read current cycle count
  }

  // CLZ for 0 is undefined, so needs to be handled.
  static inline int clz32(uint32_t x)
  {
    return x ? __builtin_clz(x) : 32;
  }
};
}  // namespace clarinoid
