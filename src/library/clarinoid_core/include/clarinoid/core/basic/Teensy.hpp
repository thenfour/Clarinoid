// actually we should not make this "Teensy" but something generic, which has teensy specializations.
// the "Teensy" project should probably also represent the device itself,
// so it should contain the pin configuration, connected devices etc.
// and for a cycle counter, there would be only 1 per device.

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
  // gotchas:
  // - there may be quirks leaving this on, so use only for benchmarking / debugging code.
  // - because it's a uint32, it will overflow after 2^32 cycles, which is about 7 seconds at 600 MHz.
  static inline void cycles_enable()
  {
#if HAVE_DWT
    ARM_DEMCR |= ARM_DEMCR_TRCENA;           // enable trace/DWT
    ARM_DWT_CYCCNT = 0;                      // reset the counter
    ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;  // start counting
#else
//
#endif
  }
  static inline uint32_t cycles_now()
  {
#if HAVE_DWT
    return ARM_DWT_CYCCNT;  // read current cycle count
#else
#endif
  }

  // CLZ for 0 is undefined, so needs to be handled.
  static inline int clz32(uint32_t x)
  {
#ifdef CLARINOID_PLATFORM_X86
    // fallback implementation
    if (x == 0)
      return 32;
    int n = 0;
    for (uint32_t mask = 0x80000000; (mask & x) == 0; mask >>= 1)
    {
      ++n;
    }
    return n;
#else
    return x ? __builtin_clz(x) : 32;
#endif
  }
};
}  // namespace clarinoid
