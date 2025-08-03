#pragma once

#include "./quickstring.hpp"

namespace clarinoid {

// optimized vector which handles construction / destruction of QuickStringData,
// and hands out QuickString to act somewhat like a std::string
template <typename _Char> struct QuickStringList {
private:
  // QuickStringList<_Char>& operator =(QuickStringList<_Char>& rhs)
  //{
  //	return *this;
  // }
  // QuickStringList(QuickStringList<_Char>& rhs)
  //{
  // }

public:
  QuickStringList()
      : m_listLen(0), m_listAllocated(listStaticBufferSize),
        listp(listStaticBuffer) {}

  // hope we can avoid this
  QuickStringList<_Char> &operator=(const QuickStringList<_Char> &rhs) {
    if (&rhs == this)
      return *this;                // self-assign guard
    clear();                       // free old strings
    if (listp != listStaticBuffer) // free old array
      HeapFree(GetProcessHeap(), 0, listp);

    m_listLen = rhs.m_listLen;
    m_listAllocated = rhs.m_listAllocated;

    if (m_listAllocated > listStaticBufferSize) {
      listDynBuffer = static_cast<QuickStringData<_Char> *>(
          HeapAlloc(GetProcessHeap(), 0,
                    sizeof(QuickStringData<_Char>) * m_listAllocated));
      listp = listDynBuffer; // single assignment
    } else {
      listp = listStaticBuffer;
    }

    memcpy(listp, rhs.listp, m_listLen * sizeof(QuickStringData<_Char>));

    // deep-copy each string that is dynamically allocated in rhs
    for (size_t i = 0; i < m_listLen; ++i) {
      auto &s = listp[i];
      if (s.m_allocated <= QuickStringData<_Char>::staticBufferSize) {
        s.p = s.staticBuffer;
      } else {
        s.dynBuffer = static_cast<_Char *>(
            HeapAlloc(GetProcessHeap(), 0, s.m_allocated * sizeof(_Char)));
        memcpy(s.dynBuffer, rhs.listp[i].p, (s.m_len + 1) * sizeof(_Char));
        s.p = s.dynBuffer;
      }
    }
    return *this;
  }

  QuickStringList(const QuickStringList<_Char> &rhs)
      : m_listLen(0), m_listAllocated(listStaticBufferSize),
        listp(listStaticBuffer) {
    *this = rhs;
  }

  ~QuickStringList() {
    // free all strings
    clear();
    if (listp != listStaticBuffer) {
      HeapFree(GetProcessHeap(), 0, listp);
    }
  }

  size_t size() const { return m_listLen; }

  void clear() {
    QuickStringData<_Char> *i = listp;
    QuickStringData<_Char> *end = listp + m_listLen;
    for (; i != end; ++i) {
      if (i->p != i->staticBuffer) {
        HeapFree(GetProcessHeap(), 0, i->p);
      }
    }
    m_listLen = 0;
  }

  QuickString<_Char> operator[](size_t index) {
    return QuickString<_Char>(&listp[index]);
  }

  QuickString<_Char> operator[](size_t index) const {
    return QuickString<_Char>(&listp[index]);
  }

  // creates room for X strings, and returns the first one available,
  // UNCONSTRUCTED.
  QuickStringData<_Char> *AddAlloc(size_t additional) {
    if (m_listAllocated < (m_listLen + additional)) {
      // realloc. todo: actually use realloc
      size_t newAllocated =
          std::max(m_listAllocated * 2, m_listLen + additional);
      QuickStringData<_Char> *newp = (QuickStringData<_Char> *)HeapAlloc(
          GetProcessHeap(), 0, sizeof(QuickStringData<_Char>) * newAllocated);
      // copy dynBuffer to newp
      memcpy(newp, listp, m_listLen * sizeof(QuickStringData<_Char>));
      // memset(p, 0, m_len * sizeof(QuickStringData<_Char>));// DEBUGGING
      // PURPOSES ONLY
      if (listp != listStaticBuffer) {
        HeapFree(GetProcessHeap(), 0, listp);
      }

      m_listAllocated = newAllocated;
      listDynBuffer = newp;
      listp = listDynBuffer;

      // fix up pointers to static data
      QuickStringData<_Char> *i = listp;
      QuickStringData<_Char> *end = listp + m_listLen;
      for (; i != end; ++i) {
        if (i->m_allocated <= QuickStringData<_Char>::staticBufferSize)
          i->p = i->staticBuffer;
      }
    }
    m_listLen++;
    return listp + m_listLen - 1;
  }

  void ConstructAlloc(QuickStringData<_Char> *data) {
    if (data->m_allocated > QuickStringData<_Char>::staticBufferSize) {
      data->dynBuffer = (_Char *)HeapAlloc(GetProcessHeap(), 0,
                                           data->m_allocated * sizeof(_Char));
      data->p = data->dynBuffer;
    } else {
      data->p = data->staticBuffer;
    }
  }

  QuickString<_Char> push_back() {
    QuickStringData<_Char> *back = AddAlloc(1);
    ConstructQuickString(back);
    return QuickString<_Char>(back);
  }

  void ConstructQuickString(QuickStringData<_Char> *data) {
    data->p = data->staticBuffer;
    data->m_len = 0;
    data->m_allocated = QuickStringData<_Char>::staticBufferSize;
  }

  QuickString<_Char> push_back(const _Char *s, _Char open, _Char close) {
    QuickStringData<_Char> *back = AddAlloc(1);
    ConstructQuickString(back, s, open, close);
    return QuickString<_Char>(back);
  }

  void ConstructQuickString(QuickStringData<_Char> *data, const _Char *s,
                            _Char open, _Char close) {
    size_t inputLen = s == 0 ? 0 : LibCC::StringLength(s);
    data->m_len = inputLen + 2;
    data->m_allocated =
        std::max(data->m_len + 1, QuickStringData<_Char>::staticBufferSize);
    ConstructAlloc(data);

    _Char *i = data->p;
    *i = open;
    ++i;
    memcpy(i, s, sizeof(_Char) * inputLen);
    i += inputLen;
    *i = close;
    ++i;
    *i = 0;
  }

  QuickString<_Char> push_back(const _Char *s, int maxLen) {
    QuickStringData<_Char> *back = AddAlloc(1);
    ConstructQuickString(back, s, maxLen);
    return QuickString<_Char>(back);
  }

  void ConstructQuickString(QuickStringData<_Char> *data, const _Char *s,
                            int maxLen) {
    data->m_len = s == 0 ? 0 : min((int)LibCC::StringLength(s), maxLen);
    data->m_allocated =
        std::max(data->m_len + 1, QuickStringData<_Char>::staticBufferSize);
    ConstructAlloc(data);

    _Char *i = data->p;
    memcpy(i, s, sizeof(_Char) * data->m_len);
    i += data->m_len;
    *i = 0;
  }

  QuickString<_Char> push_back(const _Char *s) {
    QuickStringData<_Char> *back = AddAlloc(1);
    ConstructQuickString(back, s);
    return QuickString<_Char>(back);
  }

  void ConstructQuickString(QuickStringData<_Char> *data, const _Char *s) {
    data->m_len = s == 0 ? 0 : LibCC::StringLength(s);
    data->m_allocated =
        std::max(data->m_len + 1, QuickStringData<_Char>::staticBufferSize);
    ConstructAlloc(data);

    _Char *i = data->p;
    memcpy(i, s, sizeof(_Char) * data->m_len);
    i += data->m_len;
    *i = 0;
  }

  QuickString<_Char> push_back(_Char ch, size_t count) {
    QuickStringData<_Char> *back = AddAlloc(1);
    ConstructQuickString(back, ch, count);
    return QuickString<_Char>(back);
  }

  void ConstructQuickString(QuickStringData<_Char> *data, _Char ch,
                            size_t count) {
    data->m_len = count;
    data->m_allocated =
        std::max(data->m_len + 1, QuickStringData<_Char>::staticBufferSize);
    ConstructAlloc(data);

    _Char *i = data->p;
    _Char *end = i + count;
    while (i != end) {
      *i = ch;
      ++i;
    }
    *i = 0;
  }

  QuickString<_Char> push_back(const _Char *s, int maxLen, _Char open,
                               _Char close) {
    QuickStringData<_Char> *back = AddAlloc(1);
    ConstructQuickString(back, s, maxLen, open, close);
    return QuickString<_Char>(back);
  }

  void ConstructQuickString(QuickStringData<_Char> *data, const _Char *s,
                            int maxLen, _Char open, _Char close) {
    data->m_len =
        s == 0 ? min(maxLen, 2) : min((int)LibCC::StringLength(s) + 2, maxLen);
    data->m_allocated =
        std::max(data->m_len + 1, QuickStringData<_Char>::staticBufferSize);
    ConstructAlloc(data);

    _Char *i = data->p;
    if (data->m_len > 0) {
      *i = open;
      ++i;
      if (data->m_len > 1) {
        memcpy(i, s, sizeof(_Char) * (data->m_len - 2));
        i += data->m_len - 2;
        *i = close;
        ++i;
      }
    }
    *i = 0;
  }

  size_t m_listLen;
  size_t m_listAllocated;
  static const size_t listStaticBufferSize = 16;
  QuickStringData<_Char> listStaticBuffer[listStaticBufferSize];
  QuickStringData<_Char> *listDynBuffer;
  QuickStringData<_Char> *listp;
};

template <typename Tlhs, typename Trhs>
inline void __StringAppend(QuickString<Tlhs> &lhs, Trhs *rhs) {
  lhs.append(StringConvert<Tlhs>(rhs).c_str());
}

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
