
#include <cstdint>
// #include <imxrt.h>

namespace clarinoid {
struct Teensy
{
  static inline void cycles_enable()
  {
    ARM_DEMCR |= ARM_DEMCR_TRCENA;          // enable trace/DWT
    ARM_DWT_CYCCNT = 0;                     // reset the counter
    ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA; // start counting
  }
  static inline uint32_t cycles_now()
  {
    return ARM_DWT_CYCCNT; // read current cycle count
  }

  static inline int32_t smmul(int32_t a, int32_t b)
  {
    int32_t r;
    __asm__ volatile("smmul  %0, %1, %2" : "=r"(r) : "r"(a), "r"(b));
    return r;
  }

  static inline int32_t smmulr(int32_t a, int32_t b)
  {
    int32_t r;
    __asm__ volatile("smmulr %0, %1, %2" : "=r"(r) : "r"(a), "r"(b));
    return r;
  }

  static inline int32_t smmla(int32_t a, int32_t b, int32_t acc)
  {
    int32_t r;
    __asm__ volatile("smmla  %0, %1, %2, %3" : "=r"(r) : "r"(a), "r"(b), "r"(acc));
    return r;
  }

  // CLZ for 0 is undefined, so needs to be handled.
  static inline int clz32(uint32_t x) { return x ? __builtin_clz(x) : 32; }
};
}
