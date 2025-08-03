
#pragma once

namespace clarinoid {

//
template <typename TString> struct Format {
  static const _Char OpenQuote = '\"';
  static const _Char CloseQuote = '\"';

  static const _Char ReplaceChar = '%';
  static const _Char NamedArgOpenChar = '{';
  static const _Char NamedArgCloseChar = '}';
  static const _Char EscapeChar = '^';
  static const _Char NewlineChar = '|';

  // notepad, winword, ultraedit do not support these characters
  // but wordpad & devenv do. not really enough support to justify using these
  // ever, considering anything that supports them will also support \r\n
  static void AppendNewLine(_String &s) {
#if LIBCC_UNICODENEWLINES == 1
    if (IsUnicode()) {
      s.push_back(0x2028); // the Unicode line separator char
    } else {
      s.push_back('\r');
      s.push_back('\n');
    }
#else
    s.push_back('\r');
    s.push_back('\n');
#endif
  }

  static void AppendNewParagraph(_String &s) {
#if LIBCC_UNICODENEWLINES == 1
    if (IsUnicode()) {
      s.push_back(0x2029); // the Unicode new paragraph char
    } else {
      s.push_back('\r');
      s.push_back('\n');
    }
#else
    s.push_back('\r');
    s.push_back('\n');
#endif
  }

  // Construction / Assignment
  FormatX() : m_isRendered(false), m_argumentCharSize(0) {}

  // Construction / Assignment

  explicit FormatX(const _String &s)
      : m_Format(s), m_isRendered(false), m_argumentCharSize(0) {}

  explicit FormatX(const _Char *s)
      : m_Format(s), m_isRendered(false), m_argumentCharSize(0) {}

  void Clear() {
    m_argumentCharSize = 0;
    m_isRendered = false;
    m_dynArguments.clear();
    // m_dynArgumentCount.myval = 0;
    m_Format.clear();
  }

  void SetFormat(const _String &s) {
    Clear();
    m_Format = s;
  }

  void SetFormat(const _Char *s) {
    Clear();
    if (s == 0)
      return;
    m_Format = s;
  }

  // "GET" methods
  const _String &Str() const {
    Render();
    return m_rendered;
  }
  const _Char *CStr() const {
    Render();
    return m_rendered.c_str();
  }
  operator _String() const {
    Render();
    return m_rendered;
  }

  // POINTER -----------------------------
  template <typename T> _This &p(const T *v) {
    static const int Digits =
        (sizeof(uintptr_t) *
         2); // number of digits (32-bit == 4 bytes == 8 digits)
    _Char arg[Digits + 3] = {'0', 'x'}; // +2 for prefix, +1 for null term.
    uintptr_t temp = *(reinterpret_cast<uintptr_t *>(&v));
    _UnsignedNumberToString<_Char, 16, Digits, '0'>(arg + 2 + Digits, temp);
    return s(arg);
  }

  _This &p(const void *v) {
    static const int Digits =
        (sizeof(uintptr_t) *
         2); // number of digits (32-bit == 4 bytes == 8 digits)
    _Char arg[Digits + 3] = {'0', 'x'}; // +2 for prefix, +1 for null term.
    uintptr_t temp = *(reinterpret_cast<uintptr_t *>(&v));
    _UnsignedNumberToString<_Char, 16, Digits, '0'>(arg + 2 + Digits, temp);
    return s(arg);
  }

  // CHARACTER (count) -----------------------------
  template <typename T> _This &c(T v) {
    AddArg(static_cast<_Char>(v), 1);
    return *this;
  }

  template <typename T> _This &c(T v, size_t count) {
    AddArg(static_cast<_Char>(v), count);
    return *this;
  }

  // STRING (maxlen) -----------------------------
  template <size_t MaxLen> _This &s(const _Char *s) {
    AddArg(s, MaxLen);
    return *this;
  }

  _This &s(const _Char *s, size_t MaxLen) {
    AddArg(s, (int)MaxLen);
    return *this;
  }

  _This &s(const _Char *s) {
    AddArg(s);
    return *this;
  }

  template <size_t MaxLen> _This &s(const _String &s) {
    AddArg(s.c_str(), (int)MaxLen);
    return *this;
  }

  _This &s(const _String &s, size_t MaxLen) {
    AddArg(s.c_str(), (int)MaxLen);
    return *this;
  }

  _This &s(const _String &s) {
    AddArg(s.c_str());
    return *this;
  }

  // now all that but in foreign char types
  template <typename aChar> _This &s(const aChar *foreign) {
    if (foreign) {
      _String native;
      StringConvert(foreign, native);
      return s(native);
    }
    return *this;
  }

  template <typename fChar, typename fTraits, typename fAlloc>
  _This &s(const LibCC::FormatX<fChar, fTraits, fAlloc> &str) {
    return s(str.Str());
  }

  /*
          dilemma here is... do we convert foreign to native first, or truncate
     the string first? if i want to support localized strings correctly i should
     convert first. but that may mean potentially creating copies of huge
     strings when maxlen might be very small. i figure that's a rare enough case
     that i should go for accuracy.
  */
  template <size_t MaxLen, typename aChar> _This &s(const aChar *foreign) {
    if (foreign) {
      _String native;
      StringConvert(foreign, native);
      return s<MaxLen>(native);
    }
    return *this;
  }

  template <typename aChar> _This &s(const aChar *foreign, size_t MaxLen) {
    if (foreign) {
      _String native;
      StringConvert(foreign, native);
      return s(native, MaxLen);
    }
    return *this;
  }

  template <typename aChar, typename aTraits, typename aAlloc>
  _This &s(const std::basic_string<aChar, aTraits, aAlloc> &x) {
    _String native;
    StringConvert(x, native);
    return s(native);
  }

  template <size_t MaxLen, typename aChar, typename aTraits, typename aAlloc>
  _This &s(const std::basic_string<aChar, aTraits, aAlloc> &x) {
    _String native;
    StringConvert(x, native);
    return s<MaxLen>(native);
  }

  template <typename aChar, typename aTraits, typename aAlloc>
  _This &s(const std::basic_string<aChar, aTraits, aAlloc> &x, size_t MaxLen) {
    _String native;
    StringConvert(x, native);
    return s(native, MaxLen);
  }

  _This &NewLine() {
    AppendNewLine(m_Composite);
    return *this;
  }

  _This &NewParagraph() {
    AppendNewParagraph(m_Composite);
    return *this;
  }

  // QUOTED STRINGS (maxlen)
  template <size_t MaxLen> _This &qs(const _Char *s) {
    AddArg(s, MaxLen, OpenQuote, CloseQuote);
    return *this;
  }

  _This &qs(const _Char *s, size_t MaxLen) {
    AddArg(s, (int)MaxLen, OpenQuote, CloseQuote);
    return *this;
  }

  _This &qs(const _Char *s) {
    AddArg(s, OpenQuote, CloseQuote);
    return *this;
  }

  template <size_t MaxLen> _This &qs(const _String &s) {
    AddArg(s.c_str(), (int)MaxLen, OpenQuote, CloseQuote);
    return *this;
  }

  _This &qs(const _String &s, size_t MaxLen) {
    AddArg(s.c_str(), (int)MaxLen, OpenQuote, CloseQuote);
    return *this;
  }

  _This &qs(const _String &s) {
    AddArg(s.c_str(), OpenQuote, CloseQuote);
    return *this;
  }

  // (now the same stuff but with foreign characters)
  template <typename aChar> _This &qs(const aChar *foreign) {
    if (foreign) {
      _String native;
      StringConvert(foreign, native);
      return qs(native);
    }
    return *this;
  }

  template <size_t MaxLen, typename aChar> _This &qs(const aChar *foreign) {
    if (foreign) {
      _String native;
      StringConvert(foreign, native);
      return qs<MaxLen>(native);
    }
    return *this;
  }

  template <typename aChar> _This &qs(const aChar *foreign, size_t MaxLen) {
    if (foreign) {
      _String native;
      StringConvert(foreign, native);
      return qs(native, MaxLen);
    }
    return *this;
  }

  template <typename aChar, typename aTraits, typename aAlloc>
  _This &qs(const std::basic_string<aChar, aTraits, aAlloc> &x) {
    _String native;
    StringConvert(x, native);
    return qs(native);
  }

  template <size_t MaxLen, typename aChar, typename aTraits, typename aAlloc>
  _This &qs(const std::basic_string<aChar, aTraits, aAlloc> &x) {
    _String native;
    StringConvert(x, native);
    return qs<MaxLen>(native);
  }

  template <typename aChar, typename aTraits, typename aAlloc>
  _This &qs(const std::basic_string<aChar, aTraits, aAlloc> &x, size_t MaxLen) {
    _String native;
    StringConvert(x, native);
    return qs(native, MaxLen);
  }

  // UNSIGNED LONG -----------------------------
  template <size_t Base, size_t Width, _Char PadChar>
  _This &ul(unsigned long n) {
    const size_t BufferSize =
        _BufferSizeNeededInteger<Width, unsigned long>::Value;
    _Char buf[BufferSize];
    _Char *p = buf + BufferSize - 1;
    *p = 0;
    return s(_UnsignedNumberToString<_Char, Base, Width, PadChar>(p, n));
  }

  template <size_t Base, size_t Width> _This &ul(unsigned long n) {
    return ul<Base, Width, '0'>(n);
  }

  template <size_t Base> _This &ul(unsigned long n) {
    return ul<Base, 0, '0'>(n);
  }

  _This &ul(unsigned long n) { return ul<10, 0, '0'>(n); }

  _This &ul(unsigned long n, size_t Base, size_t Width = 0,
            _Char PadChar = '0') {
    const size_t BufferSize =
        _RuntimeBufferSizeNeededInteger<unsigned long>(Width);
    _Char *buf = (_Char *)_alloca(BufferSize * sizeof(_Char));
    _Char *p = buf + BufferSize - 1;
    *p = 0;
    return s(_RuntimeUnsignedNumberToString<unsigned long>(p, n, Base, Width,
                                                           PadChar));
  }

  // SIGNED LONG -----------------------------
  template <size_t Base, size_t Width, _Char PadChar, bool ForceShowSign>
  _This &l(signed long n) {
    const size_t BufferSize =
        _BufferSizeNeededInteger<Width, signed long>::Value;
    _Char buf[BufferSize];
    _Char *p = buf + BufferSize - 1;
    *p = 0;
    return s(_SignedNumberToString<_Char, Base, Width, PadChar, ForceShowSign>(
        p, n));
  }

  template <size_t Base, size_t Width, _Char PadChar> _This &l(signed long n) {
    return l<Base, Width, PadChar, false>(n);
  }

  template <size_t Base, size_t Width> _This &l(signed long n) {
    return l<Base, Width, '0', false>(n);
  }

  template <size_t Base> _This &l(signed long n) {
    return l<Base, 0, '0', false>(n);
  }

  _This &l(signed long n) { return l<10, 0, '0', false>(n); }

  _This &l(signed long n, size_t Base, size_t Width = 0, _Char PadChar = '0',
           bool ForceShowSign = false) {
    const size_t BufferSize =
        _RuntimeBufferSizeNeededInteger<unsigned long>(Width);
    _Char *buf = (_Char *)_alloca(BufferSize * sizeof(_Char));
    _Char *p = buf + BufferSize - 1;
    *p = 0;
    return s(_RuntimeSignedNumberToString(p, n, Base, Width, PadChar,
                                          ForceShowSign));
  }

  // UNSIGNED INT (just stubs for ul()) -----------------------------
  template <size_t Base, size_t Width, _Char PadChar>
  _This &ui(unsigned int n) {
    return ul<Base, Width, PadChar>(n);
  }
  template <size_t Base, size_t Width> _This &ui(unsigned int n) {
    return ul<Base, Width>(n);
  }
  template <size_t Base> _This &ui(unsigned int n) { return ul<Base>(n); }
  _This &ui(unsigned int n) { return ul(n); }
  _This &ui(unsigned int n, size_t Base, size_t Width = 0,
            _Char PadChar = '0') {
    return ul(n, Base, Width, PadChar);
  }

  // SIGNED INT -----------------------------
  template <size_t Base, size_t Width, _Char PadChar, bool ForceShowSign>
  _This &i(signed int n) {
    return l<Base, Width, PadChar, ForceShowSign>(n);
  }
  template <size_t Base, size_t Width, _Char PadChar> _This &i(signed int n) {
    return l<Base, Width, PadChar>(n);
  }
  template <size_t Base, size_t Width> _This &i(signed int n) {
    return l<Base, Width>(n);
  }
  template <size_t Base> _This &i(signed int n) { return l<Base>(n); }
  _This &i(signed int n) { return l(n); }
  _This &i(signed int n, size_t Base, size_t Width = 0, _Char PadChar = '0',
           bool ForceShowSign = false) {
    return l(n, Base, Width, PadChar, ForceShowSign);
  }

  // FLOAT ----------------------------- 3.14   [intwidth].[decwidth]
  // integralwidth is the MINIMUM digits.  Decimalwidth is the MAXIMUM digits.
  template <size_t DecimalWidthMax, size_t DecimalWidthMin,
            size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign,
            size_t Base>
  _This &f(float val) {
    QuickString<_Char> back = AddArg();
    _AppendFloat<_Char, SinglePrecisionFloat, Base, DecimalWidthMax,
                 DecimalWidthMin, IntegralWidthMin, PaddingChar, ForceSign>(
        val, back);
    m_argumentCharSize += back.size();
    return *this;
  }

  template <size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar,
            bool ForceSign>
  _This &f(float val) {
    return f<DecimalWidthMax, 1, IntegralWidthMin, PaddingChar, ForceSign, 10>(
        val);
  }

  template <size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar>
  _This &f(float val) {
    return f<DecimalWidthMax, 1, IntegralWidthMin, PaddingChar, false, 10>(val);
  }

  template <size_t DecimalWidthMax, size_t IntegralWidthMin>
  _This &f(float val) {
    return f<DecimalWidthMax, 1, IntegralWidthMin, '0', false, 10>(val);
  }

  template <size_t DecimalWidthMax> _This &f(float val) {
    return f<DecimalWidthMax, 1, 1, '0', false, 10>(val);
  }

  _This &f(float val) { return f<2, 1, 1, '0', false, 10>(val); }

  _This &f(float val, size_t DecimalWidthMax, size_t IntegralWidthMin = 1,
           _Char PaddingChar = '0', bool ForceSign = false, size_t Base = 10) {
    QuickString<_Char> back = AddArg();
    _RuntimeAppendFloat<SinglePrecisionFloat>(val, Base, DecimalWidthMax, 1,
                                              IntegralWidthMin, PaddingChar,
                                              ForceSign, back);
    m_argumentCharSize += back.size();
    return *this;
  }

  // DOUBLE -----------------------------
  template <size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar,
            bool ForceSign, size_t Base>
  _This &d(double val) {
    QuickString<_Char> back = AddArg();
    _AppendFloat<_Char, DoublePrecisionFloat, Base, DecimalWidthMax, 1,
                 IntegralWidthMin, PaddingChar, ForceSign>(val, back);
    m_argumentCharSize += back.size();
    return *this;
  }

  template <size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar,
            bool ForceSign>
  _This &d(double val) {
    return d<DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign, 10>(
        val);
  }

  template <size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar>
  _This &d(double val) {
    return d<DecimalWidthMax, IntegralWidthMin, PaddingChar, false, 10>(val);
  }

  template <size_t DecimalWidthMax, size_t IntegralWidthMin>
  _This &d(double val) {
    return d<DecimalWidthMax, IntegralWidthMin, '0', false, 10>(val);
  }

  template <size_t DecimalWidthMax> _This &d(double val) {
    return d<DecimalWidthMax, 1, '0', false, 10>(val);
  }

  _This &d(double val) { return d<3, 1, '0', false, 10>(val); }

  _This &d(double val, size_t DecimalWidthMax, size_t IntegralWidthMin = 1,
           _Char PaddingChar = '0', bool ForceSign = false, size_t Base = 10) {
    QuickString<_Char> n = AddArg();
    _RuntimeAppendFloat<DoublePrecisionFloat, _Char>(val, Base, DecimalWidthMax,
                                                     1, IntegralWidthMin,
                                                     PaddingChar, ForceSign, n);
    m_argumentCharSize += n.size();
    return *this;
  }

  // UNSIGNED INT 64 -----------------------------
  template <size_t Base, size_t Width, _Char PadChar>
  _This &ui64(unsigned __int64 n) {
    const size_t BufferSize =
        _BufferSizeNeededInteger<Width, unsigned __int64>::Value;
    _Char buf[BufferSize];
    _Char *p = buf + BufferSize - 1;
    *p = 0;
    return s(_UnsignedNumberToString<_Char, Base, Width, PadChar>(p, n));
  }

  template <size_t Base, size_t Width> _This &ui64(unsigned __int64 n) {
    return ui64<Base, Width, '0'>(n);
  }

  template <size_t Base> _This &ui64(unsigned __int64 n) {
    return ui64<Base, 0, 0>(n);
  }

  _This &ui64(unsigned __int64 n) { return ui64<10, 0, 0>(n); }

  _This &ui64(unsigned __int64 n, size_t Base, size_t Width = 0,
              _Char PadChar = '0') {
    const size_t BufferSize =
        _RuntimeBufferSizeNeededInteger<unsigned __int64>(Width);
    _Char *buf = (_Char *)_alloca(BufferSize * sizeof(_Char));
    _Char *p = buf + BufferSize - 1;
    *p = 0;
    return s(_RuntimeUnsignedNumberToString<unsigned __int64>(p, n, Base, Width,
                                                              PadChar));
  }

  // SIGNED INT 64 -----------------------------
  template <size_t Base, size_t Width, _Char PadChar, bool ForceShowSign>
  _This &i64(signed __int64 n) {
    const size_t BufferSize =
        _BufferSizeNeededInteger<Width, unsigned __int64>::Value;
    _Char buf[BufferSize];
    _Char *p = buf + BufferSize - 1;
    *p = 0;
    return s(_SignedNumberToString<Base, Width, PadChar, ForceShowSign>(p, n));
  }

  template <size_t Base, size_t Width, _Char PadChar> _This &i64(__int64 n) {
    return i64<Base, Width, PadChar, false>(n);
  }

  template <size_t Base, size_t Width> _This &i64(__int64 n) {
    return i64<Base, Width, '0', false>(n);
  }

  template <size_t Base> _This &i64(__int64 n) {
    return i64<Base, 0, 0, false>(n);
  }

  _This &i64(__int64 n) { return i64<10, 0, 0, false>(n); }

  _This &i64(signed __int64 n, size_t Base = 10, size_t Width = 0,
             _Char PadChar = '0', bool ForceShowSign = false) {
    const size_t BufferSize =
        _RuntimeBufferSizeNeededInteger<unsigned __int64>(Width);
    _Char *buf = (_Char *)_alloca(BufferSize * sizeof(_Char));
    _Char *p = buf + BufferSize - 1;
    *p = 0;
    return s(_RuntimeSignedNumberToString<signed __int64>(
        p, n, Base, Width, PadChar, ForceShowSign));
  }

  // GETLASTERROR() -----------------------------
#ifdef WIN32
  _This &gle(int code) {
    _String str;
    FormatMessageGLE(str, code);
    return s(str);
  }

  _This &gle() { return gle(GetLastError()); }
#endif

  // CONVENIENCE OPERATOR () -----------------------------
  _This &operator()(int n, size_t Base = 10, size_t Width = 0,
                    _Char PadChar = '0', bool ForceShowSign = false) {
    return i(n, Base, Width, PadChar, ForceShowSign);
  }
  _This &operator()(unsigned int n, size_t Base = 10, size_t Width = 0,
                    _Char PadChar = '0') {
    return ui(n, Base, Width, PadChar);
  }
  _This &operator()(__int64 n, size_t Base = 10, size_t Width = 0,
                    _Char PadChar = '0', bool ForceShowSign = false) {
    return i64(n, Base, Width, PadChar, ForceShowSign);
  }
  _This &operator()(unsigned __int64 n, size_t Base = 10, size_t Width = 0,
                    _Char PadChar = '0') {
    return ui64(n, Base, Width, PadChar);
  }
  _This &operator()(float n, size_t DecimalWidthMax = 2,
                    size_t IntegralWidthMin = 1, _Char PaddingChar = '0',
                    bool ForceSign = false, size_t Base = 10) {
    return f(n, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign,
             Base);
  }
  _This &operator()(double n, size_t DecimalWidthMax = 2,
                    size_t IntegralWidthMin = 1, _Char PaddingChar = '0',
                    bool ForceSign = false, size_t Base = 10) {
    return d(n, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign,
             Base);
  }
  _This &operator()(const char *n) { return s(n); }
  _This &operator()(const wchar_t *n) { return s(n); }
  _This &operator()(const void *n) { return p(n); }
  template <typename RChar>
  _This &operator()(const std::basic_string<RChar> &n) {
    return s(n);
  }
  template <typename RChar> _This &operator()(const FormatX<RChar> &n) {
    return s(n);
  }

private:
  void Render() const {
    if (m_isRendered)
      return;
    int currentSequentialArg = 0;
    int highestUsedSequentialArg = -1;

    m_isRendered = true;
    m_rendered.clear();
    m_rendered.reserve(m_Format.size() + m_argumentCharSize);

    const _Char *begin = m_Format.c_str();
    const _Char *end = begin + m_Format.size();
    const _Char *it = begin;

    for (; it != end; ++it) {
      _Char ch = *it;
      switch (ch) {
      case EscapeChar:
        ++it;
        if (it != end) {
          m_rendered.push_back(*it);
        }
        break;
      case NewlineChar:
        AppendNewLine(m_rendered);
        break;
      case ReplaceChar:
        if (currentSequentialArg >= (int)m_dynArguments.size()) {
          m_rendered.push_back(
              ch); // if you put too many replacechars, then just ignore it.
        } else {
          m_rendered.append(GetArg(currentSequentialArg).c_str());
          highestUsedSequentialArg =
              std::max(highestUsedSequentialArg, currentSequentialArg);
          ++currentSequentialArg;
        }
        break;
      case NamedArgOpenChar: {
        int argIndex = 0;
        const _Char *it2 = it;
        while (true) {
          ++it2;
          if (it2 == end) {
            // unclosed named arg.
            m_rendered.push_back(ch);
            break;
          }
          wchar_t ch2 = *it2;
          if (ch2 >= '0' && ch2 <= '9') {
            argIndex =
                (argIndex * 10) + (ch2 - '0'); // construct an integer index
          } else if (ch2 == NamedArgCloseChar) {
            if (argIndex < (int)m_dynArguments.size()) {
              // success!
              m_rendered.append(GetArg(argIndex).c_str());
              highestUsedSequentialArg =
                  std::max(highestUsedSequentialArg, (int)argIndex);
              it = it2; // advance the cursor.
            } else {
              // index out of range
              m_rendered.push_back(ch);
            }
            break;
          } else {
            // unrecognized char
            m_rendered.push_back(ch);
            break;
          }
        }
        break;
      }
      default:
        m_rendered.push_back(ch);
        break;
      }
    }

    // append unused args. this is how the old Format() works.
    currentSequentialArg = highestUsedSequentialArg + 1;
    while (currentSequentialArg < (int)m_dynArguments.size()) {
      m_rendered.append(GetArg(currentSequentialArg).c_str());
      currentSequentialArg++;
    }
  }

  _String m_Format; // the original format string.  this plus arguments that are
                    // fed in is used to build m_Composite.
  mutable _String m_rendered;
  mutable bool m_isRendered;

  void AddArg(const _Char *s) {
    m_argumentCharSize += LibCC::StringLength(s);
    m_dynArguments.push_back(s);
  }

  void AddArg(const _Char *s, _Char open, _Char close) {
    m_argumentCharSize += LibCC::StringLength(s) + 2;
    m_dynArguments.push_back(s, open, close);
  }

  void AddArg(const _Char *s, int maxLen) {
    m_dynArguments.push_back(s, maxLen);
    m_argumentCharSize += maxLen;
  }

  void AddArg(const _Char *s, int maxLen, _Char open, _Char close) {
    m_dynArguments.push_back(s, maxLen, open, close);
    m_argumentCharSize += maxLen;
  }

  void AddArg(_Char ch, size_t count) {
    m_dynArguments.push_back(ch, count);
    m_argumentCharSize += count;
  }

  QuickString<_Char> AddArg() { return m_dynArguments.push_back(); }

  const QuickString<_Char> GetArg(size_t i) const { return m_dynArguments[i]; }

  size_t m_argumentCharSize;
  QuickStringList<_Char> m_dynArguments;
};

} // namespace clarinoid
