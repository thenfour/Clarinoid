// #pragma once

// #include <cstdint>

// namespace clarinoid {

// template <class To, class From>
// constexpr To bit_cast(const From &src) noexcept {
//   static_assert(sizeof(To) == sizeof(From));
//   static_assert(std::is_trivially_copyable_v<To>);
//   static_assert(std::is_trivially_copyable_v<From>);
//   To dst{};
//   std::memcpy(&dst, &src, sizeof(To));
//   return dst; // in practice compiles to a plain mov
// }

// // this simple class just "attaches" to a float and provides a window into it's
// // inner workings. based on information from
// // http://www.duke.edu/~twf/cps104/floating.html
// // ... and http://stevehollasch.com/cgindex/coding/ieeefloat.html
// // e notation:    d.dd...En
// // 1 bit = sign
// // 8 bits = exp
// // 23 bits = mantissa
// /*
//   What are denormalized numbers?  they are normal numbers but without the
//   leading 1 and assumed exp of -126
// */
// template <typename _Float, typename _Bits, typename _Exp, typename _Frac,
//           long _ExpBits, long _FracBits>
// struct IEEEFloat {
//   using Float = _Float;
//   using Bits = _Bits;
//   using Exponent = _Exp;
//   using Mantissa = _Frac;
//   typedef IEEEFloat<Float, Bits, Exponent, Mantissa, _ExpBits, _FracBits> This;

//   static constexpr long ExponentBits = _ExpBits;
//   static constexpr long MantissaBits = _FracBits;
//   static constexpr Bits SignMask = (Bits)1 << (ExponentBits + MantissaBits);
//   static constexpr Bits MantissaMask =
//       ((Bits)1 << MantissaBits) - 1; // 0x007fffff / 0x000fffffffffffff
//   static constexpr Bits MantissaHighBit =
//       MantissaMask ^ (MantissaMask >> 1); // 0x00400000 / 0x0008000000000000
//   static constexpr Bits ExponentMask =
//       ((((Bits)1 << ExponentBits) - 1)
//        << MantissaBits); // 0x7f800000 / 0x7ff0000000000000
//   static constexpr Bits PositiveInfinity =
//       ExponentMask; // 0x7f800000 / 0x7ff0000000000000
//   static constexpr Bits NegativeInfinity =
//       SignMask | ExponentMask; // 0xff800000 / 0xfff0000000000000
//   static const Exponent ExponentBias =
//       (((Bits)1 << (ExponentBits - 1)) - 1); // 0x7f / 0x3ff

//   IEEEFloat(Float f) : m_bits(bit_cast<Bits>(f)) {}
//   IEEEFloat(Bits bits) : m_bits(bits) {}
//   IEEEFloat(const This &) = default;

//   Float ToFloat() const { return bit_cast<Float>(m_bits); }

//   bool IsPositive() const { return m_bits & SignMask ? false : true; }
//   bool IsNegative() const { return !IsPositive(); }
//   bool IsZero() const // exponent == 0  &&  mantissa == 0
//   {
//     return m_bits & (MantissaMask | ExponentMask) ? false : true;
//   }
//   bool IsDenormalized() const // exponent = 0  &&  mantissa != 0
//   {
//     return (!(m_bits & ExponentMask)) && (m_bits & MantissaMask);
//   }
//   bool IsPositiveInfinity() const // exponent == MAX
//   {
//     return m_bits == PositiveInfinity;
//   }
//   bool IsNegativeInfinity() const // exponent == MAX
//   {
//     return m_bits == NegativeInfinity;
//   }
//   bool IsInfinity() const // exponent == MAX  &&  mantissa == MAX
//   {
//     return (m_bits & (MantissaMask | ExponentMask)) ==
//            (MantissaMask | ExponentMask);
//   }
//   bool IsNaN() const // exponent == MAX  && mantissa != 0
//   {
//     return ((m_bits & ExponentMask) == ExponentMask) && (m_bits & MantissaMask);
//   }
//   bool IsQNaN() const // exponent == MAX  && mantissa high bit set
//   {
//     return ((m_bits & ExponentMask) == ExponentMask) &&
//            (m_bits & MantissaHighBit);
//   }
//   bool
//   IsSNaN() const // exponent == MAX  && mantissa != 0  && mantissa high bit 0
//   {
//     return ((m_bits & ExponentMask) == ExponentMask) &&
//            (m_bits & MantissaMask) && !(m_bits & MantissaHighBit);
//   }
//   Exponent GetExponent() const {
//     return static_cast<Exponent>(((m_bits & ExponentMask) >> MantissaBits) -
//                                  ExponentBias);
//   }

//   Mantissa GetMantissa() const {
//     return static_cast<Mantissa>(
//         m_bits & MantissaMask |
//         (static_cast<Mantissa>(1) << MantissaBits)); // add the implied 1
//   }

//   void CopyValue(Float &out) const { memcpy(&out, &m_bits, sizeof(m_bits)); }

//   static This Build(bool sign, Exponent ex, Mantissa m) {
//     Bits r = sign ? SignMask : 0;
//     r |= (static_cast<Bits>(ex) + ExponentBias) << MantissaBits;
//     r |= m & MantissaMask;
//     return This(r);
//   }

//   /*
//     Infinity
//     The values +infinity and -infinity are denoted with an exponent of all 1s
//     and a fraction of all 0s. The sign bit distinguishes between negative
//     infinity and positive infinity.
//   */
//   static This BuildPositiveInfinity() { return This(ExponentMask); }
//   static This BuildNegativeInfinity() { return This(SignMask | ExponentMask); }

//   // The value NaN (Not a Number) is used to represent a value that does not
//   // represent a real number. NaN's are represented by a bit pattern with an
//   // exponent of all 1s and a non-zero fraction. There are two categories of
//   // NaN: QNaN (Quiet NaN) and SNaN (Signalling NaN). A QNaN is a NaN with the
//   // most significant fraction bit set. QNaN's propagate freely through most
//   // arithmetic operations. These values pop out of an operation when the result
//   // is not mathematically defined.
//   static This BuildQNaN() { return This(ExponentMask | MantissaMask); }

//   // An SNaN is a NaN with the most significant fraction bit clear. It is used
//   // to signal an exception when used in operations. SNaN's can be handy to
//   // assign to uninitialized variables to trap premature usage.
//   static This BuildSNaN() { return This(ExponentMask | (MantissaMask >> 1)); }

//   void AbsoluteValue() { m_bits &= ~SignMask; }

//   /*
//     say you have a mantissa:
//     1.0101101 with exponent 3.
//     that's: 1010.1101
//     to remove the decimal, it would be:
//     that's: 1010.0
//     or, 1.01, exponent 3
//     In the case of a negative exponent, set the float to zero.

//     So the idea is simply to mask out the bits that are right of
//     the decimal point.
//   */
//   void RemoveDecimal() {
//     Bits e = (m_bits & ExponentMask) >> MantissaBits;
//     // Mantissa m = static_cast<Mantissa>(m_val & MantissaMask);

//     // if(m)
//     {
//       if (e >= ExponentBias) {
//         e -= ExponentBias;
//         if (e < MantissaBits) // if we did this when e is greater than the
//                               // mantissa bits, it would get all screwy.
//         {
//           // positive exponent.  mask out the decimal part.
//           Bits mask = 1;
//           mask <<= (MantissaBits - e); // bits
//           mask -= 1;                   // mask
//           m_bits &= ~mask;
//         }
//       } else {
//         // negative exponent; set this float to zero, retaining the current
//         // sign.
//         m_bits &= ~(ExponentMask | MantissaMask);
//       }
//     }
//   }

//   Bits m_bits;

// private:
//   This &operator=(const This &rhs) {
//     return *this;
//   } // do not allow assignment.  this is also to prevent warning C4512 "'class'
//     // : assignment operator could not be generated"
// };
// using SinglePrecisionFloat =
//     IEEEFloat<float, uint32_t, int8_t, uint32_t, 8, 23>;
// using DoublePrecisionFloat =
//     IEEEFloat<double, uint64_t, int16_t, uint64_t, 11, 52>;
// } // namespace clarinoid
