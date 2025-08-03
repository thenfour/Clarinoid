#pragma once

#include "./quickstring.hpp"

namespace clarinoid {

// template <typename Tlhs, typename Trhs>
// inline void __StringAppend(QuickString<Tlhs> &lhs, Trhs *rhs) {
//   lhs.append(StringConvert<Tlhs>(rhs).c_str());
// }

template <typename _Char>
inline void
_RuntimeAppendZeroFloat(size_t DecimalWidthMax, size_t DecimalWidthMin,
                        size_t IntegralWidthMin, _Char PaddingChar,
                        bool /*ForceSign*/, QuickString<_Char> &output) {
  // zero.
  // pre-decimal part.
  // "-----0"
  if (IntegralWidthMin > 0) {
    // append padding
    output.reserve(output.size() + IntegralWidthMin);
    for (size_t i = 1; i < IntegralWidthMin; ++i) {
      output.push_back(static_cast<_Char>(PaddingChar));
    }
    // append the integral zero
    output.push_back('0');
  }
  if (DecimalWidthMax) {
    // if there are any decimal digits to set, then just append ".0"
    output.reserve(output.size() + 2);
    output.push_back('.');
    for (size_t i = 0; i < DecimalWidthMin; ++i) {
      output.push_back('0');
    }
  }
}

template <typename FloatType, typename _Char>
inline void
_RuntimeAppendNormalizedFloat(FloatType &_f, size_t Base,
                              size_t DecimalWidthMax, size_t DecimalWidthMin,
                              size_t IntegralWidthMin, _Char PaddingChar,
                              bool ForceSign, QuickString<_Char> &output) {
  // how do we know how many chars we will use?  we don't right now.
  _Char *buf = reinterpret_cast<_Char *>(
      _alloca(sizeof(_Char) * (2200 + IntegralWidthMin + DecimalWidthMax)));
  long IntegralWidthLeft = static_cast<long>(IntegralWidthMin);
  _Char *middle = buf + 2100 - DecimalWidthMax;
  _Char *sIntPart = middle;
  _Char *sDecPart = middle;
  FloatType::Mantissa _int;                   // integer part raw value
  FloatType::Mantissa _dec;                   // decimal part raw value
  FloatType::Exponent exp = _f.GetExponent(); // exponent raw value
  FloatType::Mantissa m = _f.GetMantissa();
  size_t DecBits; // how many bits out of the mantissa are used by the decimal
                  // part?

  const size_t BasicTypeBits = sizeof(FloatType::BasicType) * 8;
  if ((exp < FloatType::MantissaBits) &&
      (exp > (FloatType::MantissaBits - BasicTypeBits))) {
    // write the integral (before decimal point) part.
    DecBits = _f.MantissaBits - exp;
    _int = m >> DecBits; // the integer part.
    _dec = m & (((FloatType::Mantissa)1 << DecBits) - 1);
    do {
      --IntegralWidthLeft;
      *(--sIntPart) = DigitToChar(static_cast<unsigned char>(_int % Base));
      _int = _int / static_cast<FloatType::Mantissa>(Base);
    } while (_int);

    while (IntegralWidthLeft > 0) {
      *(--sIntPart) = static_cast<_Char>(PaddingChar);
      IntegralWidthLeft--;
    }

    // write the after-decimal part.  here we basically do long division!
    // the decimal part is basically a fraction that we convert bases on.
    // since we need to deal with a number as large as the denominator, this
    // will only work when DecBits is less than 32 (for single precsion)
    if (DecimalWidthMax) {
      size_t DecimalWidthLeft = DecimalWidthMax;
      size_t DecimalUsed = 0;
      middle[0] = '.';
      FloatType::Mantissa denominator = (FloatType::Mantissa)1
                                        << DecBits; // same as 'capacity'.
      FloatType::Mantissa &numerator(_dec);
      numerator *= static_cast<FloatType::Mantissa>(Base);
      FloatType::Mantissa digit;
      while ((numerator || (DecimalUsed < DecimalWidthMin)) &&
             DecimalWidthLeft) {
        digit = numerator / denominator; // integer division
        // add the digit, and drill down into the remainder.
        *(++sDecPart) = DigitToChar(static_cast<unsigned char>(digit % Base));
        numerator -= digit * denominator;
        numerator *= static_cast<FloatType::Mantissa>(Base);
        --DecimalWidthLeft;
        DecimalUsed++;
      }
    } else {
      middle[0] = 0;
    }
  } else {
    // We are here because doing conversions would take large numbers - too
    // large to hold in a InternalType integral.  So until i can come up with a
    // cooler way to do it, i will just do floating point divides and

    // do the integral part just like a normal int.
    FloatType::This integerPart(_f);
    integerPart.RemoveDecimal();
    integerPart.AbsoluteValue();
    FloatType::BasicType fBase = static_cast<FloatType::BasicType>(Base);
    do {
      IntegralWidthLeft--;
      // at this point integerPart has no decimal and Base of course doesnt.
      *(--sIntPart) = DigitToChar(
          static_cast<unsigned char>(fmod(integerPart.m_BasicVal, fBase)));
      integerPart.m_BasicVal /= Base;
      integerPart.RemoveDecimal();
    } while (integerPart.m_BasicVal > 0);

    while (IntegralWidthLeft > 0) {
      *(--sIntPart) = static_cast<_Char>(PaddingChar);
      IntegralWidthLeft--;
    }

    // now the decimal part.
    if (DecimalWidthMax) {
      size_t DecimalWidthLeft = DecimalWidthMax;
      size_t DecimalUsed = 0;
      middle[0] = '.';
      FloatType::This val(_f);
      val.AbsoluteValue();
      // remove integer part.
      FloatType::This integerPart2(val);
      integerPart2.RemoveDecimal();
      val.m_BasicVal -= integerPart2.m_BasicVal;
      do {
        DecimalWidthLeft--;
        DecimalUsed++;
        val.m_BasicVal *= Base;
        // isolate the integral part
        integerPart2.m_BasicVal = val.m_BasicVal;
        integerPart2.RemoveDecimal();
        *(++sDecPart) = DigitToChar(
            static_cast<unsigned char>(fmod(integerPart2.m_BasicVal, fBase)));
        // use the integral part to leave only the decimal part.
        val.m_BasicVal -= integerPart2.m_BasicVal;
      } while (((val.m_BasicVal > 0) || (DecimalUsed < DecimalWidthMin)) &&
               DecimalWidthLeft);
    } else {
      middle[0] = 0;
    }
  }

  // display the sign
  if (_f.IsNegative()) {
    *(--sIntPart) = '-';
  } else if (ForceSign) {
    *(--sIntPart) = '+';
  }

  // null terminate
  *(++sDecPart) = 0;

  __StringAppend(output, sIntPart);
}

/*
  Converts any floating point (LibCC::IEEEFloat<>) number to a string, and
  appends it just like any other string.
*/
template <typename FloatType, typename _Char>
inline void _RuntimeAppendFloat(const FloatType &_f, size_t Base,
                                size_t DecimalWidthMax, size_t DecimalWidthMin,
                                size_t IntegralWidthMin, _Char PaddingChar,
                                bool ForceSign, QuickString<_Char> &output) {
  if (!(_f.m_val & _f.ExponentMask)) {
    // exponont = 0.  that means its either zero or denormalized.
    if (_f.m_val & _f.MantissaMask) {
      // denormalized
      __StringAppend(output, "Unsupported denormalized number");
    } else {
      // zero
      return _RuntimeAppendZeroFloat(DecimalWidthMax, DecimalWidthMin,
                                     IntegralWidthMin, PaddingChar, ForceSign,
                                     output);
    }
  } else if ((_f.m_val & _f.ExponentMask) == _f.ExponentMask) {
    // exponent = MAX.  either infinity or NAN.
    if (_f.IsPositiveInfinity()) {
      __StringAppend(output, "+Inf");
    } else if (_f.IsNegativeInfinity()) {
      __StringAppend(output, "-Inf");
    } else if (_f.IsQNaN()) {
      __StringAppend(output, "QNaN");
    } else if (_f.IsSNaN()) {
      __StringAppend(output, "SNaN");
    }
  }

  // normalized number.
  _RuntimeAppendNormalizedFloat(_f, Base, DecimalWidthMax, DecimalWidthMin,
                                IntegralWidthMin, PaddingChar, ForceSign,
                                output);
}

template <typename _Char, typename FloatType, size_t Base,
          size_t DecimalWidthMax, size_t DecimalWidthMin,
          size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign>
inline void _AppendFloat(const FloatType &_f, QuickString<_Char> &output) {
  return _RuntimeAppendFloat<FloatType>(_f, Base, DecimalWidthMax,
                                        DecimalWidthMin, IntegralWidthMin,
                                        PaddingChar, ForceSign, output);
}

template <size_t Width, typename T> struct _BufferSizeNeededInteger {
  // sizeof(T) * 8 == how many bits to store the value.  considering
  // the smallest base supported is base 2 (binary), thats exactly how
  // many digits maximum for an integer type.  +1 for null terminator
  // this is basically max(size based on width, size based on sizeof())
  // and +1 for the sign.
  static const long Value = (sizeof(T) * 8) + 2 > (Width + 1)
                                ? (sizeof(T) * 8) + 2
                                : (Width + 1);
};

template <typename T>
inline long _RuntimeBufferSizeNeededInteger(size_t Width) {
  return (long)((sizeof(T) * 8) + 2 > (Width + 1) ? (sizeof(T) * 8) + 2
                                                  : (Width + 1));
}

// buf must point to a null terminator.  It is "pulled back" and the result is
// returned. its simply faster to build the string in reverse order.
template <typename T, typename _Char>
inline _Char *_RuntimeUnsignedNumberToString(_Char *buf, T num, size_t Base,
                                             size_t Width, _Char PaddingChar) {
  long PadRemaining = static_cast<long>(Width);
  _Char _PadChar = PaddingChar;
  do {
    PadRemaining--;
    *(--buf) =
        static_cast<_Char>(DigitToChar(static_cast<unsigned char>(num % Base)));
    num = num / static_cast<T>(Base);
  } while (num);

  while (PadRemaining-- > 0) {
    *(--buf) = _PadChar;
  }
  return buf;
}

template <typename _Char, size_t Base, size_t Width, _Char PaddingChar,
          typename T>
inline static _Char *_UnsignedNumberToString(_Char *buf, T num) {
  if (Base < 2) {
    static _Char x[] = {0};
    return x;
  }
  ptrdiff_t PadRemaining = static_cast<ptrdiff_t>(Width);
  _Char _PadChar = PaddingChar;
  do {
    PadRemaining--;
    *(--buf) =
        static_cast<_Char>(DigitToChar(static_cast<unsigned char>(num % Base)));
    num = num / static_cast<T>(Base);
  } while (num);

  while (PadRemaining-- > 0) {
    *(--buf) = _PadChar;
  }
  return buf;
}

// same thing, but params can be set at runtime
template <typename T, typename _Char>
inline static _Char *
_RuntimeSignedNumberToString(_Char *buf, T num, size_t Base, size_t Width,
                             _Char PaddingChar, bool ForceSign) {
  if (Base < 2) {
    static _Char x[] = {0};
    return x;
  }
  if (num < 0) {
    buf =
        _RuntimeUnsignedNumberToString(buf, -num, Base, Width - 1, PaddingChar);
    *(--buf) = '-';
  } else {
    if (ForceSign) {
      buf = _RuntimeUnsignedNumberToString(buf, num, Base, Width - 1,
                                           PaddingChar);
      *(--buf) = '+';
    } else {
      buf = _RuntimeUnsignedNumberToString(buf, num, Base, Width, PaddingChar);
    }
  }
  return buf;
}

template <typename _Char, size_t Base, size_t Width, _Char PaddingChar,
          bool ForceSign, typename T>
inline static _Char *_SignedNumberToString(_Char *buf, T num) {
  if (num < 0) {
    buf = _UnsignedNumberToString<_Char, Base, Width - 1, PaddingChar, T>(buf,
                                                                          -num);
    *(--buf) = '-';
  } else {
    if (ForceSign) {
      buf = _UnsignedNumberToString<_Char, Base, Width - 1, PaddingChar, T>(
          buf, num);
      *(--buf) = '+';
    } else {
      buf =
          _UnsignedNumberToString<_Char, Base, Width, PaddingChar, T>(buf, num);
    }
  }
  return buf;
}

} // namespace clarinoid
