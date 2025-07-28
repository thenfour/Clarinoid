
#pragma once

// // COMPILE-TIME OPTIONS:
// // FP_CACHE_DOUBLE
// // FP_RUNTIME_CHECKS

namespace clarinoid {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Counts the number of leading zero bits in a 32-bit unsigned integer.
/// </summary>
/// <param name="value">The 32-bit unsigned integer whose leading zeros are to be counted.</param>
/// <returns>The number of leading zero bits in the input value.</returns>
static inline uint32_t
CLZ(uint32_t value)
{
#ifdef CLARINOID_PLATFORM_X86
  unsigned int count = __lzcnt(value);
#else
  unsigned int count = __builtin_clz(value);
#endif
  // #else
  //     // Fallback implementation
  //     unsigned int count = 0;
  //     while ((value & (1 << (31 - count))) == 0 && count < 32)
  //     {
  //         count++;
  //     }
  // #endif
  return count;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const uint32_t sqrt_integer_guess_table[33] = {
  55109, 38968, 27555, 19484, 13778, 9742, 6889, 4871, 3445, 2436, 1723, 1218, 862, 609, 431, 305, 216,
  153,   108,   77,    54,    39,    27,   20,   14,   10,   7,    5,    4,    3,   2,   1,   0,
};

// Newton-Raphson integral square root. accepts a Q32, returns Q16. I would like to find a way to return a Q32 but I
// don't see it yet.
static inline uint32_t
sqrt_Q32_to_Q16(uint32_t in)
{
  int i = CLZ(in);
  uint32_t n = sqrt_integer_guess_table[i];
  n = ((in / n) + n) >> 1;
  n = ((in / n) + n) >> 1;
  n = ((in / n) + n) >> 1;
  return n;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Computes the absolute value of a compile-time integer constant.
/// </summary>
/// <typeparam name="i">The integer value whose absolute value is to be computed.</typeparam>
template<int32_t i>
struct StaticAbs
{
  static constexpr int32_t value = i < 0 ? -i : i;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T, std::enable_if_t<std::is_signed<T>::value, int> = 0>
static inline T
FPAbs(T val)
{
  return val < 0 ? -val : val;
}

template<typename T, std::enable_if_t<!std::is_signed<T>::value, int> = 0>
static inline T
FPAbs(T val)
{
  return val;
}

// probably optimizable via some intrinsics but not sure.
// not hot path, so no extreme optimizations here.
template<typename T>
inline constexpr uint8_t
ValueBitsNeededForValue(T i)
{
  if (i == 0) {
    return 1;
  }
  uint8_t bits = 0;
  T value = FPAbs(i);
  while (value > 0) {
    value >>= 1;
    bits++;
  }
  return bits;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Computes the number of bits required to represent a static integer value at compile time.
/// </summary>
/// <typeparam name="i">The integer value for which to compute the number of bits needed.</typeparam>
template<int32_t i>
struct StaticValueBitsNeeded
{
  static constexpr int32_t value_allow_zero = 1 + StaticValueBitsNeeded<(StaticAbs<i>::value >> 1)>::value_allow_zero;
  static constexpr int32_t value = value_allow_zero;
};

template<>
struct StaticValueBitsNeeded<0>
{
  static constexpr int32_t value = 1;
  static constexpr int32_t value_allow_zero = 0;
};

} // namespace clarinoid
