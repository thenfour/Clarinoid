
#pragma once

// // COMPILE-TIME OPTIONS:
// // FP_CACHE_DOUBLE
// // FP_RUNTIME_CHECKS

namespace clarinoid {

//inline uint32_t
//__SMMULR(uint32_t a, uint32_t b)
//{
//  uint32_t r;
//  asm volatile("smmulr %0,%1,%2" : "=r"(r) : "r"(a), "r"(b));
//  return r;
//}

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

// 31-entry table (indices 0..30). Values fit in 16 bits.
static constexpr uint16_t sqrt_integer_guess_table[31] = { 55109, 38968, 27555, 19484, 13778, 9742, 6889, 4871,
                                                           3445,  2436,  1723,  1218,  862,   609,  431,  305,
                                                           216,   153,   108,   77,    54,    39,   27,   20,
                                                           14,    10,    7,     5,     4,     3,    2 };
// returns Q(n+1/2) for Q(n) input.
static inline uint32_t
sqrt_Q32_to_Q16_NR(uint32_t in)
{
  if (in <= 1u)
    return in; // avoids clz==31 case and is exact

  uint32_t n = sqrt_integer_guess_table[__builtin_clz(in)];

  // Two Newton steps (Heron), 32-bit divisions
  uint32_t q = in / n;
  n = (q + n) >> 1;
  q = in / n;
  n = (q + n) >> 1;

  // Exact correction to floor (slightly more accurate, and actually still very fast but only if really needed)
  // uint64_t n2 = (uint64_t)n * n;
  // if (n2 > in) { while ((uint64_t)(n-1)*(n-1) >= in) --n; }
  // else         { while ((uint64_t)(n+1)*(n+1) <=  in) ++n; }

  // Nearest rounding (optional; comment out to keep floor)
  uint64_t low = (uint64_t)n * n;
  uint64_t high = (uint64_t)(n + 1) * (n + 1);
  if (in > low + ((high - low) >> 1))
    ++n;

  return n;
}


//   index = top 8 bits of the mantissa in [0.5 .. 1)
//   value = round( 2^32 / mantissa )  → Q0.32 reciprocal seed
static constexpr uint32_t recip8_LUT[256] = {
  0xFFFFFFFF, 0xFE03F80F, 0xFC0FC0FC, …, 0x80808081
  /* generated once – total 1 kB */
};

// Generation script :
//  val[i] = uint32_t((1u << 32) / (0x80 + i) + 0.5)


// returns  ⌈32/F⌉ fractional bits  (same as sqrt rule)
// unsigned.
template<int F, bool twoIterations = true>
static inline uint32_t
recip_uq(uint32_t X)
{
  if (X == 0)
    return 0xFFFFFFFFu; // saturate 1/0 -> max
  constexpr bool oddF = F & 1;

  /* --- normalise ------------------------------------------------------ */
  unsigned lz = CLZ(X);
  unsigned shift = lz - (oddF ? 1u : 0u); // make mantissa in [0.5,1)
  uint32_t a = X << shift;                // Q0.32
  uint32_t idx = a >> 24;                 // top 8 frac bits
  uint32_t x = recip8_LUT[idx];           // Q0.32 seed  ~10-11 good bits

  /* --- Newton  -------------------------------------------------------- */
  // x_{n+1} = x_n * (2 − a*x_n)
  auto step = [&](uint32_t x0) -> uint32_t {
    uint32_t t = __SMMULR(a, x0); // Q0.32
    t = 0xFFFFFFFFu - t;          // (2 – a*x)
    return __SMMULR(x0, t);       // Q0.32
  };
  x = step(x); // 1st iteration  (~16 bits)
  if constexpr (twoIterations)
    x = step(x); // 2nd iteration  (~31 bits)

  /* --- denormalise ---------------------------------------------------- */
  unsigned outShift = (oddF ? (shift + 1) : shift); // same rule as sqrt
  return x >> outShift;                             //  result lives in Q⌈F⌉
}

// signed.
template<int F>
uint32_t
recip_qs(int32_t Xin)
{
  if (Xin == 0)
    return 0x7FFFFFFFu; // saturate
  uint32_t mag = Xin < 0 ? -Xin : Xin;
  uint32_t r = recip_uq<F>(mag);
  return (Xin < 0) ? -int32_t(r) : r;
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


 ///////////////////////////////////////////////////////////////////////////////////////////////////

 /// <summary>
 /// Generates a value of type T with the lowest 'bits' bits set to 1.
 /// </summary>
 /// <typeparam name="T">The integer type to use for the result. Defaults to FPAutoBaseType<bits>.</typeparam>
 /// <typeparam name="bits">The number of least significant bits to set to 1.</typeparam>
 /// <returns>A value of type T where the lowest 'bits' bits are set to 1. Returns 0 if bits is 0.</returns>
 template<uint8_t bits, typename T = FPAutoBaseType<bits>>
 static constexpr T
 FillBits()
 {
   // ensure remaining code has bits > 0
   if (bits == 0)
     return 0;
   T ret = 1ULL << std::max(0, (bits - 1)); // avoid compile warning about negative shifts
   ret -= 1;
   ret <<= 1;
   ret |= 1;
   return ret;
 }

 ///////////////////////////////////////////////////////////////////////////////////////////////////

 // does a compile-time shift of T left by B bits.
 // B can be negative, in which case it shifts right.
 template<int B, class T>
 static constexpr auto
 const_shift(const T& a, ::std::enable_if_t<(B > 0)>* = 0)
 {
   return a << ::std::integral_constant<decltype(B), B>{};
 }

 // right-shift variant when B is negative.
 template<int B, class T>
 static constexpr auto
 const_shift(const T& a, ::std::enable_if_t<(B < 0)>* = 0)
 {
   return a >> ::std::integral_constant<decltype(B), -B>{};
 }

 // noop when B is 0.
 template<int B, class T>
 static constexpr auto
 const_shift(const T& a, ::std::enable_if_t<(B == 0)>* = 0)
 {
   return a;
 }

 ///////////////////////////////////////////////////////////////////////////////////////////////////
 // SignedSaturate<n>() does a clamp(-(1<<n), (1<<n)-1)
 // TODO: check that intbits <= 31
 template<uint8_t intbits, typename Tinput> // template Tinput because it may be signed or unsigned and we want
                                            // conversions & full range to work seamlessly.
 static CL_NODISCARD int32_t
 SignedSaturate(Tinput val)
 {
   static_assert(intbits <= 31, "ssat does not support 32+ bits");
 #ifdef CLARINOID_PLATFORM_X86
   static constexpr int32_t pos = (1UL << intbits) - 1; // for 15 bits, 32767
   static constexpr int32_t neg = -(1L << intbits);     // for 15 bits, -32768
   // int32_t xpos = pos;
   // int32_t xneg = neg;
   if (val < neg)
     return neg;
   if (val > pos)
     return pos;
   return val;
 #else
   int32_t tmp;
   asm volatile("ssat %0, %1, %2" : "=r"(tmp) : "I"(intbits), "r"(val));
   return tmp;
 #endif
 }


} // namespace clarinoid
