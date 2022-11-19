
#pragma once

#ifndef CLARINOID_PLATFORM_X86
#error This is only for x86 unit test stuff.
#endif


char *__utoa(unsigned value, char *str, int base)
{
    const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    int i, j;
    unsigned remainder;
    char c;

    /* Check base is supported. */
    if ((base < 2) || (base > 36))
    {
        str[0] = '\0';
        return NULL;
    }

    /* Convert to string. Digits are in reverse order.  */
    i = 0;
    do
    {
        remainder = value % base;
        str[i++] = digits[remainder];
        value = value / base;
    } while (value != 0);
    str[i] = '\0';

    /* Reverse string.  */
    for (j = 0, i--; j < i; j++, i--)
    {
        c = str[j];
        str[j] = str[i];
        str[i] = c;
    }

    return str;
}

char *utoa(unsigned value, char *str, int base)
{
    return __utoa(value, str, base);
}

//
//
//char *ultoa(unsigned long val, char *buf, int radix)
//{
//    unsigned digit;
//    int i = 0, j;
//    char t;
//
//    while (1)
//    {
//        digit = val % radix;
//        buf[i] = ((digit < 10) ? '0' + digit : 'A' + digit - 10);
//        val /= radix;
//        if (val == 0)
//            break;
//        i++;
//    }
//    buf[i + 1] = 0;
//    for (j = 0; j < i; j++, i--)
//    {
//        t = buf[j];
//        buf[j] = buf[i];
//        buf[i] = t;
//    }
//    return buf;
//}

//char *ltoa(long val, char *buf, int radix)
//{
//    if (val >= 0)
//    {
//        return ultoa(val, buf, radix);
//    }
//    else
//    {
//        buf[0] = '-';
//        ultoa(-val, buf + 1, radix);
//        return buf;
//    }
//}

#define DTOA_UPPER 0x04

//char *fcvtf(float, int, int *, int *);
//int isnanf(float x);
//int isinff(float x);

static const char *convert(double value, int ndigit, int *decpt, int *sign, int fflag)
{
    static char *buf = 0;
    static int bufsize = 0;
    union {
        uint64_t l;
        double f;
    } x;
    x.f = value;
    int exp2 = (0x7ff & (x.l >> 52)) - 1023;
    uint64_t mant = x.l & 0x000fffffffffffffULL;
    if ((*sign = x.l >> 63))
        value = -value;
    if (exp2 == 0x400)
    {
        *decpt = 0;
        return mant ? "nan" : "inf";
    }
    int exp10 = (value == 0) ? !fflag : (int)ceil(log10(value));
    if (exp10 < -307)
        exp10 = -307; /* otherwise overflow in pow() */
    value *= pow(10.0, -exp10);
    if (value)
    {
        while (value < 0.1)
        {
            value *= 10;
            --exp10;
        }
        while (value >= 1.0)
        {
            value /= 10;
            ++exp10;
        }
    }
//    CCASSERT(value == 0 || (0.1 <= value && value < 1.0));
    if (fflag)
    {
        if (ndigit + exp10 < 0)
        {
            *decpt = -ndigit;
            return "";
        }
        ndigit += exp10;
    }
    *decpt = exp10;
    if (bufsize < ndigit + 2)
    {
        bufsize = ndigit + 2;
        buf = buf ? (char *)realloc(buf, bufsize) : (char *)malloc(bufsize);
    }
    int ptr = 1;
#if 1 /* slow and safe (and dreadfully boring) */
  while (ptr <= ndigit) {
    double i;
    value= modf(value * 10, &i);
    buf[ptr++]= '0' + (int)i;
  }
  if (value >= 0.5)
    while (--ptr && ++buf[ptr] > '9')
      buf[ptr]= '0';
#else /* faster */
    x.f = value;
    exp2 = (0x7ff & (x.l >> 52)) - 1023;
//    assert(value == 0 || (-4 <= exp2 && exp2 <= -1));
    mant = x.l & 0x000fffffffffffffULL;
    if (exp2 == -1023)
        ++exp2;
    else
        mant |= 0x0010000000000000ULL;
    mant <<= (exp2 + 4); /* 56-bit denormalised signifier */
    while (ptr <= ndigit)
    {
        mant &= 0x00ffffffffffffffULL; /* mod 1.0 */
        mant = (mant << 1) + (mant << 3);
        buf[ptr++] = '0' + (mant >> 56);
    }
    if (mant & 0x0080000000000000ULL) /* 1/2 << 56 */
        while (--ptr && ++buf[ptr] > '9')
            buf[ptr] = '0';
#endif
    if (ptr)
    {
        buf[ndigit + 1] = 0;
        return buf + 1;
    }
    if (fflag)
    {
        ++ndigit;
        ++*decpt;
    }
    buf[0] = '1';
    buf[ndigit] = 0;
    return buf;
}

const char *e_cvt(double value, int ndigit, int *decpt, int *sign)
{
    return convert(value, ndigit, decpt, sign, 0);
}
const char *fcvtf(double value, int ndigit, int *decpt, int *sign)
{
    return convert(value, ndigit, decpt, sign, 1);
}



char *dtostrf(float val, int width, unsigned int precision, char *buf)
{
    int decpt, sign, reqd, pad;
    const char *s, *e;
    char *p;

    int awidth = abs(width);
    if (std::isnan(val))
    {
        int ndigs = (val < 0) ? 4 : 3;
        awidth = (awidth > ndigs) ? awidth - ndigs : 0;
        if (width < 0)
        {
            while (awidth)
            {
                *buf++ = ' ';
                awidth--;
            }
        }
        if (copysignf(1.0f, val) < 0)
            *buf++ = '-';
        if (DTOA_UPPER)
        {
            *buf++ = 'N';
            *buf++ = 'A';
            *buf++ = 'N';
        }
        else
        {
            *buf++ = 'n';
            *buf++ = 'a';
            *buf++ = 'n';
        }
        while (awidth)
        {
            *buf++ = ' ';
            awidth--;
        }
        *buf = 0;
        return buf;
    }
    if (std::isinf(val))
    {
        int ndigs = (val < 0) ? 4 : 3;
        awidth = (awidth > ndigs) ? awidth - ndigs : 0;
        if (width < 0)
        {
            while (awidth)
            {
                *buf++ = ' ';
                awidth--;
            }
        }
        if (val < 0)
            *buf++ = '-';
        if (DTOA_UPPER)
        {
            *buf++ = 'I';
            *buf++ = 'N';
            *buf++ = 'F';
        }
        else
        {
            *buf++ = 'i';
            *buf++ = 'n';
            *buf++ = 'f';
        }
        while (awidth)
        {
            *buf++ = ' ';
            awidth--;
        }
        *buf = 0;
        return buf;
    }

    s = fcvtf(val, precision, &decpt, &sign);

    // if only 1 digit in output
    if (precision == 0 && decpt == 0)
    {
        // round and move decimal point
        s = (*s < '5') ? "0" : "1";
        decpt++;
    }

    // if all zeros, limit to precision
    if (-decpt > (int)precision)
    {
        s = "0";
        decpt = -(int)precision;
    }

    reqd = strlen(s);

    // add 1 for decimal point
    if (reqd > decpt)
        reqd++;

    // add 1 for zero in front of decimal point
    if (decpt == 0)
        reqd++;

    // if leading zeros after decimal point
    if (decpt < 0 && precision > 0)
    {
        // ensure enough trailing zeros, add 2 for '0.'
        reqd = precision + 2;

        if (strlen(s) > precision + decpt)
        {
            // bug in fcvtf. e.g. 0.012, precision 2 should return 1 instead of 12.
            // However, 1.2, precision 0 returns correct value. So shift values so
            // that decimal point is after the first digit, then convert again

            int newPrecision = precision;
            int newDecimalPoint;

            // shift decimal point
            while (newPrecision > 0)
            {
                val *= 10.0f;
                newPrecision--;
            }

            // round after accounting for leading 0's
            s = fcvtf(val, newPrecision, &newDecimalPoint, &sign);

            // if rounded up to new digit (e.g. 0.09 to 0.1), move decimal point
            if (newDecimalPoint - decpt == precision + 1)
                decpt++;
        }
    }

    // add 1 for sign if negative
    if (sign)
        reqd++;

    p = buf;
    e = p + reqd;
    pad = width - reqd;
    if (pad > 0)
    {
        e += pad;
        while (pad-- > 0)
            *p++ = ' ';
    }
    if (sign)
        *p++ = '-';
    if (decpt == 0 && precision > 0)
    {
        *p++ = '0';
        *p++ = '.';
    }
    else if (decpt < 0 && precision > 0)
    {
        *p++ = '0';
        *p++ = '.';
        // print leading zeros
        while (decpt < 0)
        {
            decpt++;
            *p++ = '0';
        }
    }
    // print digits
    while (p < e)
    {
        *p++ = *s++;
        if (p == e)
            break;
        if (--decpt == 0)
            *p++ = '.';
    }
    if (width < 0)
    {
        pad = (reqd + width) * -1;
        while (pad-- > 0)
            *p++ = ' ';
    }
    *p = 0;

    // char format[20];
    // sprintf(format, "%%%d.%df", width, precision);
    // sprintf(buf, format, val);
    return buf;
}









// When compiling programs with this class, the following gcc parameters
// dramatically increase performance and memory (RAM) efficiency, typically
// with little or no increase in code size.
//     -felide-constructors
//     -std=c++0x

// Brian Cook's "no overhead" Flash String type (message on Dec 14, 2010)
// modified by Mikal Hart for his FlashString library
class __FlashStringHelper;
#ifndef F
#define F(string_literal) ((const __FlashStringHelper *)(string_literal))
#endif

// An inherited class for holding the result of a concatenation.  These
// result objects are assumed to be writable by subsequent concatenations.
class StringSumHelper;

// The string class
class String
{
  public:
    // constructors
    String(const char *cstr = (const char *)NULL);
    String(const __FlashStringHelper *pgmstr);
    String(const String &str);
#if __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__)
    String(String &&rval);
    String(StringSumHelper &&rval);
#endif
    String(char c);
    String(unsigned char c);
    String(int, unsigned char base = 10);
    String(unsigned int, unsigned char base = 10);
    String(long, unsigned char base = 10);
    String(unsigned long, unsigned char base = 10);
    String(float num, unsigned char digits = 2);
    String(double num, unsigned char digits = 2) : String((float)num, digits)
    {
    }
    ~String(void);

    // memory management
    unsigned char reserve(unsigned int size);
    inline unsigned int length(void) const
    {
        return len;
    }

    // copy and move
    String &copy(const char *cstr, unsigned int length);
    String &copy(const __FlashStringHelper *s)
    {
        return copy((const char *)s, strlen((const char *)s));
    }
    void move(String &rhs);
    String &operator=(const String &rhs);
    String &operator=(const char *cstr);
    String &operator=(const __FlashStringHelper *pgmstr);
#if __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__)
    String &operator=(String &&rval);
    String &operator=(StringSumHelper &&rval);
#endif
    String &operator=(char c);

    // append
    String &append(const String &str);
    String &append(const char *cstr);
    String &append(const __FlashStringHelper *s)
    {
        return append((const char *)s, strlen((const char *)s));
    }
    String &append(char c);
    String &append(unsigned char c)
    {
        return append((int)c);
    }
    String &append(int num);
    String &append(unsigned int num);
    String &append(long num);
    String &append(unsigned long num);
    String &append(float num);
    String &append(double num)
    {
        return append((float)num);
    }
    String &operator+=(const String &rhs)
    {
        return append(rhs);
    }
    String &operator+=(const char *cstr)
    {
        return append(cstr);
    }
    String &operator+=(const __FlashStringHelper *pgmstr)
    {
        return append(pgmstr);
    }
    String &operator+=(char c)
    {
        return append(c);
    }
    String &operator+=(unsigned char c)
    {
        return append((int)c);
    }
    String &operator+=(int num)
    {
        return append(num);
    }
    String &operator+=(unsigned int num)
    {
        return append(num);
    }
    String &operator+=(long num)
    {
        return append(num);
    }
    String &operator+=(unsigned long num)
    {
        return append(num);
    }
    String &operator+=(float num)
    {
        return append(num);
    }
    String &operator+=(double num)
    {
        return append(num);
    }

    // concatenate
    friend StringSumHelper &operator+(const StringSumHelper &lhs, const String &rhs);
    friend StringSumHelper &operator+(const StringSumHelper &lhs, const char *cstr);
    friend StringSumHelper &operator+(const StringSumHelper &lhs, const __FlashStringHelper *pgmstr);
    friend StringSumHelper &operator+(const StringSumHelper &lhs, char c);
    friend StringSumHelper &operator+(const StringSumHelper &lhs, unsigned char c);
    friend StringSumHelper &operator+(const StringSumHelper &lhs, int num);
    friend StringSumHelper &operator+(const StringSumHelper &lhs, unsigned int num);
    friend StringSumHelper &operator+(const StringSumHelper &lhs, long num);
    friend StringSumHelper &operator+(const StringSumHelper &lhs, unsigned long num);
    friend StringSumHelper &operator+(const StringSumHelper &lhs, float num);
    friend StringSumHelper &operator+(const StringSumHelper &lhs, double num);
    String &concat(const String &str)
    {
        return append(str);
    }
    String &concat(const char *cstr)
    {
        return append(cstr);
    }
    String &concat(const __FlashStringHelper *pgmstr)
    {
        return append(pgmstr);
    }
    String &concat(char c)
    {
        return append(c);
    }
    String &concat(unsigned char c)
    {
        return append((int)c);
    }
    String &concat(int num)
    {
        return append(num);
    }
    String &concat(unsigned int num)
    {
        return append(num);
    }
    String &concat(long num)
    {
        return append(num);
    }
    String &concat(unsigned long num)
    {
        return append(num);
    }
    String &concat(float num)
    {
        return append(num);
    }
    String &concat(double num)
    {
        return append(num);
    }

    // comparison
    int compareTo(const String &s) const;
    unsigned char equals(const String &s) const;
    unsigned char equals(const char *cstr) const;
    // unsigned char equals(const __FlashStringHelper *pgmstr) const;
    unsigned char operator==(const String &rhs) const
    {
        return equals(rhs);
    }
    unsigned char operator==(const char *cstr) const
    {
        return equals(cstr);
    }
    unsigned char operator!=(const String &rhs) const
    {
        return !equals(rhs);
    }
    unsigned char operator!=(const char *cstr) const
    {
        return !equals(cstr);
    }
    unsigned char operator<(const String &rhs) const;
    unsigned char operator>(const String &rhs) const;
    unsigned char operator<=(const String &rhs) const;
    unsigned char operator>=(const String &rhs) const;
    unsigned char equalsIgnoreCase(const String &s) const;
    unsigned char startsWith(const String &prefix) const;
    unsigned char startsWith(const String &prefix, unsigned int offset) const;
    unsigned char endsWith(const String &suffix) const;

    // character acccess
    char charAt(unsigned int index) const;
    void setCharAt(unsigned int index, char c);
    char operator[](unsigned int index) const;
    char &operator[](unsigned int index);
    void getBytes(unsigned char *buf, unsigned int bufsize, unsigned int index = 0) const;
    void toCharArray(char *buf, unsigned int bufsize, unsigned int index = 0) const
    {
        getBytes((unsigned char *)buf, bufsize, index);
    }
    const char *c_str() const
    {
        if (!buffer)
            return &zerotermination; // https://forum.pjrc.com/threads/63842
        return buffer;
    }
    char *begin()
    {
        if (!buffer)
            reserve(20);
        return buffer;
    }
    char *end()
    {
        return begin() + length();
    }
    const char *begin() const
    {
        return c_str();
    }
    const char *end() const
    {
        return c_str() + length();
    }

    // search
    int indexOf(char ch) const;
    int indexOf(char ch, unsigned int fromIndex) const;
    int indexOf(const String &str) const;
    int indexOf(const String &str, unsigned int fromIndex) const;
    int lastIndexOf(char ch) const;
    int lastIndexOf(char ch, unsigned int fromIndex) const;
    int lastIndexOf(const String &str) const;
    int lastIndexOf(const String &str, unsigned int fromIndex) const;
    String substring(unsigned int beginIndex) const;
    String substring(unsigned int beginIndex, unsigned int endIndex) const;

    // modification
    String &replace(char find, char replace);
    String &replace(const String &find, const String &replace);
    String &remove(unsigned int index);
    String &remove(unsigned int index, unsigned int count);
    String &toLowerCase(void);
    String &toUpperCase(void);
    String &trim(void);

    // parsing/conversion
    long toInt(void) const;
    float toFloat(void) const;

  protected:
    char *buffer;          // the actual char array
    unsigned int capacity; // the array length minus one (for the '\0')
    unsigned int len;      // the String length (not counting the '\0')
    unsigned char flags;   // unused, for future features
  protected:
    void init(void);
    unsigned char changeBuffer(unsigned int maxStrLen);
    String &append(const char *cstr, unsigned int length);

  private:
    // allow for "if (s)" without the complications of an operator bool().
    // for more information http://www.artima.com/cppsource/safebool.html
    typedef void (String::*StringIfHelperType)() const;
    void StringIfHelper() const
    {
    }
    static const char zerotermination;

  public:
    operator StringIfHelperType() const
    {
        return buffer ? &String::StringIfHelper : 0;
    }
};

class StringSumHelper : public String
{
  public:
    StringSumHelper(const String &s) : String(s)
    {
    }
    StringSumHelper(const char *p) : String(p)
    {
    }
    StringSumHelper(const __FlashStringHelper *pgmstr) : String(pgmstr)
    {
    }
    StringSumHelper(char c) : String(c)
    {
    }
    StringSumHelper(unsigned char c) : String(c)
    {
    }
    StringSumHelper(int num) : String(num, 10)
    {
    }
    StringSumHelper(unsigned int num) : String(num, 10)
    {
    }
    StringSumHelper(long num) : String(num, 10)
    {
    }
    StringSumHelper(unsigned long num) : String(num, 10)
    {
    }
};


/*
  WString.cpp - String library for Wiring & Arduino
  ...mostly rewritten by Paul Stoffregen...
  Copyright (c) 2009-10 Hernando Barragan.  All rights reserved.
  Copyright 2011, Paul Stoffregen, paul@pjrc.com

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

//#include <Arduino.h>

/*********************************************/
/*  Constructors                             */
/*********************************************/

String::String(const char *cstr)
{
    init();
    if (cstr)
        copy(cstr, strlen(cstr));
}

String::String(const __FlashStringHelper *pgmstr)
{
    init();
    *this = pgmstr;
}

String::String(const String &value)
{
    init();
    *this = value;
}

#if __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__)
String::String(String &&rval)
{
    init();
    move(rval);
}
String::String(StringSumHelper &&rval)
{
    init();
    move(rval);
}
#endif

String::String(char c)
{
    init();
    *this = c;
}

String::String(unsigned char c)
{
    init();
    char buf[4];
    utoa(c, buf, 10);
    *this = buf;
}

String::String(const int value, unsigned char base)
{
    init();
    char buf[18];
    _itoa(value, buf, base);
    *this = buf;
}

String::String(unsigned int value, unsigned char base)
{
    init();
    char buf[17];
    utoa(value, buf, base);
    *this = buf;
}

String::String(long value, unsigned char base)
{
    init();
    char buf[34];
    _ltoa(value, buf, base);
    *this = buf;
}

String::String(unsigned long value, unsigned char base)
{
    init();
    char buf[33];
    _ultoa(value, buf, base);
    *this = buf;
}

String::String(float num, unsigned char digits)
{
    init();
    char buf[40];
    *this = dtostrf(num, digits + 2, digits, buf);
}

String::~String()
{
    free(buffer);
}

/*********************************************/
/*  Memory Management                        */
/*********************************************/

inline void String::init(void)
{
    buffer = NULL;
    capacity = 0;
    len = 0;
    flags = 0;
}

unsigned char String::reserve(unsigned int size)
{
    if (capacity >= size)
        return 1;
    if (changeBuffer(size))
    {
        if (len == 0)
            buffer[0] = 0;
        return 1;
    }
    return 0;
}

unsigned char String::changeBuffer(unsigned int maxStrLen)
{
    char *newbuffer = (char *)realloc(buffer, maxStrLen + 1);
    if (newbuffer)
    {
        buffer = newbuffer;
        capacity = maxStrLen;
        return 1;
    }
    return 0;
}

/*********************************************/
/*  Copy and Move                            */
/*********************************************/

String &String::copy(const char *cstr, unsigned int length)
{
    if (length == 0)
    {
        if (buffer)
            buffer[0] = 0;
        len = 0;
        return *this;
    }
    if (!reserve(length))
    {
        if (buffer)
        {
            free(buffer);
            buffer = NULL;
        }
        len = capacity = 0;
        return *this;
    }
    len = length;
    strcpy(buffer, cstr);
    return *this;
}

void String::move(String &rhs)
{
    if (&rhs == this)
        return;
    if (buffer)
        free(buffer);
    buffer = rhs.buffer;
    capacity = rhs.capacity;
    len = rhs.len;
    rhs.buffer = NULL;
    rhs.capacity = 0;
    rhs.len = 0;
}

String &String::operator=(const String &rhs)
{
    if (this == &rhs)
        return *this;
    return copy(rhs.buffer, rhs.len);
}

#if __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__)
String &String::operator=(String &&rval)
{
    if (this != &rval)
        move(rval);
    return *this;
}

String &String::operator=(StringSumHelper &&rval)
{
    if (this != &rval)
        move(rval);
    return *this;
}
#endif

String &String::operator=(const char *cstr)
{
    if (cstr)
    {
        copy(cstr, strlen(cstr));
    }
    else
    {
        len = 0;
    }
    return *this;
}

String &String::operator=(const __FlashStringHelper *pgmstr)
{
    copy(pgmstr);
    return *this;
}

String &String::operator=(char c)
{
    char buf[2];
    buf[0] = c;
    buf[1] = 0;
    return copy(buf, 1);
}

/*********************************************/
/*  Append                                   */
/*********************************************/

String &String::append(const String &s)
{
    return append(s.buffer, s.len);
}

String &String::append(const char *cstr, unsigned int length)
{
    unsigned int newlen = len + length;
    bool self = false;
    unsigned int buffer_offset;
    if ((cstr >= buffer) && (cstr < (buffer + len)))
    {
        self = true;
        buffer_offset = (unsigned int)(cstr - buffer);
    }
    if (length == 0 || !reserve(newlen))
        return *this;
    if (self)
    {
        memcpy(buffer + len, buffer + buffer_offset, length);
        buffer[newlen] = 0;
    }
    else
        strcpy(buffer + len, cstr);
    len = newlen;
    return *this;
}

String &String::append(const char *cstr)
{
    if (cstr)
        append(cstr, strlen(cstr));
    return *this;
}

String &String::append(char c)
{
    char buf[2];
    buf[0] = c;
    buf[1] = 0;
    append(buf, 1);
    return *this;
}

String &String::append(int num)
{
    char buf[12];
    _ltoa((long)num, buf, 10);
    append(buf, strlen(buf));
    return *this;
}

String &String::append(unsigned int num)
{
    char buf[11];
    _ultoa((unsigned long)num, buf, 10);
    append(buf, strlen(buf));
    return *this;
}

String &String::append(long num)
{
    char buf[12];
    _ltoa(num, buf, 10);
    append(buf, strlen(buf));
    return *this;
}

String &String::append(unsigned long num)
{
    char buf[11];
    _ultoa(num, buf, 10);
    append(buf, strlen(buf));
    return *this;
}

String &String::append(float num)
{
    char buf[30];
    dtostrf(num, 4, 2, buf);
    append(buf, strlen(buf));
    return *this;
}

/*********************************************/
/*  Concatenate                              */
/*********************************************/

StringSumHelper &operator+(const StringSumHelper &lhs, const String &rhs)
{
    StringSumHelper &a = const_cast<StringSumHelper &>(lhs);
    a.append(rhs.buffer, rhs.len);
    return a;
}

StringSumHelper &operator+(const StringSumHelper &lhs, const char *cstr)
{
    StringSumHelper &a = const_cast<StringSumHelper &>(lhs);
    if (cstr)
        a.append(cstr, strlen(cstr));
    return a;
}

StringSumHelper &operator+(const StringSumHelper &lhs, const __FlashStringHelper *pgmstr)
{
    StringSumHelper &a = const_cast<StringSumHelper &>(lhs);
    a.append(pgmstr);
    return a;
}

StringSumHelper &operator+(const StringSumHelper &lhs, char c)
{
    StringSumHelper &a = const_cast<StringSumHelper &>(lhs);
    a.append(c);
    return a;
}

StringSumHelper &operator+(const StringSumHelper &lhs, unsigned char c)
{
    StringSumHelper &a = const_cast<StringSumHelper &>(lhs);
    a.append(c);
    return a;
}

StringSumHelper &operator+(const StringSumHelper &lhs, int num)
{
    StringSumHelper &a = const_cast<StringSumHelper &>(lhs);
    a.append((long)num);
    return a;
}

StringSumHelper &operator+(const StringSumHelper &lhs, unsigned int num)
{
    StringSumHelper &a = const_cast<StringSumHelper &>(lhs);
    a.append((unsigned long)num);
    return a;
}

StringSumHelper &operator+(const StringSumHelper &lhs, long num)
{
    StringSumHelper &a = const_cast<StringSumHelper &>(lhs);
    a.append(num);
    return a;
}

StringSumHelper &operator+(const StringSumHelper &lhs, unsigned long num)
{
    StringSumHelper &a = const_cast<StringSumHelper &>(lhs);
    a.append(num);
    return a;
}

StringSumHelper &operator+(const StringSumHelper &lhs, float num)
{
    StringSumHelper &a = const_cast<StringSumHelper &>(lhs);
    a.append(num);
    return a;
}

StringSumHelper &operator+(const StringSumHelper &lhs, double num)
{
    StringSumHelper &a = const_cast<StringSumHelper &>(lhs);
    a.append(num);
    return a;
}

/*********************************************/
/*  Comparison                               */
/*********************************************/

int String::compareTo(const String &s) const
{
    if (!buffer || !s.buffer)
    {
        if (s.buffer && s.len > 0)
            return 0 - *(unsigned char *)s.buffer;
        if (buffer && len > 0)
            return *(unsigned char *)buffer;
        return 0;
    }
    return strcmp(buffer, s.buffer);
}

unsigned char String::equals(const String &s2) const
{
    return (len == s2.len && compareTo(s2) == 0);
}

unsigned char String::equals(const char *cstr) const
{
    if (len == 0)
        return (cstr == NULL || *cstr == 0);
    if (cstr == NULL)
        return buffer[0] == 0;
    return strcmp(buffer, cstr) == 0;
}

unsigned char String::operator<(const String &rhs) const
{
    return compareTo(rhs) < 0;
}

unsigned char String::operator>(const String &rhs) const
{
    return compareTo(rhs) > 0;
}

unsigned char String::operator<=(const String &rhs) const
{
    return compareTo(rhs) <= 0;
}

unsigned char String::operator>=(const String &rhs) const
{
    return compareTo(rhs) >= 0;
}

unsigned char String::equalsIgnoreCase(const String &s2) const
{
    if (this == &s2)
        return 1;
    if (len != s2.len)
        return 0;
    if (len == 0)
        return 1;
    const char *p1 = buffer;
    const char *p2 = s2.buffer;
    while (*p1)
    {
        if (tolower(*p1++) != tolower(*p2++))
            return 0;
    }
    return 1;
}

unsigned char String::startsWith(const String &s2) const
{
    if (len < s2.len)
        return 0;
    return startsWith(s2, 0);
}

unsigned char String::startsWith(const String &s2, unsigned int offset) const
{
    if (offset > len - s2.len || !buffer || !s2.buffer)
        return 0;
    return strncmp(&buffer[offset], s2.buffer, s2.len) == 0;
}

unsigned char String::endsWith(const String &s2) const
{
    if (len < s2.len || !buffer || !s2.buffer)
        return 0;
    return strcmp(&buffer[len - s2.len], s2.buffer) == 0;
}

/*********************************************/
/*  Character Access                         */
/*********************************************/

const char String::zerotermination = 0;

char String::charAt(unsigned int loc) const
{
    return operator[](loc);
}

void String::setCharAt(unsigned int loc, char c)
{
    if (loc < len)
        buffer[loc] = c;
}

char &String::operator[](unsigned int index)
{
    static char dummy_writable_char;
    if (index >= len || !buffer)
    {
        dummy_writable_char = 0;
        return dummy_writable_char;
    }
    return buffer[index];
}

char String::operator[](unsigned int index) const
{
    if (index >= len || !buffer)
        return 0;
    return buffer[index];
}

void String::getBytes(unsigned char *buf, unsigned int bufsize, unsigned int index) const
{
    if (!bufsize || !buf)
        return;
    if (index >= len)
    {
        buf[0] = 0;
        return;
    }
    unsigned int n = bufsize - 1;
    if (n > len - index)
        n = len - index;
    strncpy((char *)buf, buffer + index, n);
    buf[n] = 0;
}

/*********************************************/
/*  Search                                   */
/*********************************************/

int String::indexOf(char c) const
{
    return indexOf(c, 0);
}

int String::indexOf(char ch, unsigned int fromIndex) const
{
    if (fromIndex >= len)
        return -1;
    const char *temp = strchr(buffer + fromIndex, ch);
    if (temp == NULL)
        return -1;
    return temp - buffer;
}

int String::indexOf(const String &s2) const
{
    return indexOf(s2, 0);
}

int String::indexOf(const String &s2, unsigned int fromIndex) const
{
    if (fromIndex >= len)
        return -1;
    const char *found = strstr(buffer + fromIndex, s2.buffer);
    if (found == NULL)
        return -1;
    return found - buffer;
}

int String::lastIndexOf(char theChar) const
{
    return lastIndexOf(theChar, len - 1);
}

int String::lastIndexOf(char ch, unsigned int fromIndex) const
{
    if (fromIndex >= len)
        return -1;
    char tempchar = buffer[fromIndex + 1];
    buffer[fromIndex + 1] = '\0';
    char *temp = strrchr(buffer, ch);
    buffer[fromIndex + 1] = tempchar;
    if (temp == NULL)
        return -1;
    return temp - buffer;
}

int String::lastIndexOf(const String &s2) const
{
    return lastIndexOf(s2, len - s2.len);
}

int String::lastIndexOf(const String &s2, unsigned int fromIndex) const
{
    if (s2.len == 0 || len == 0 || s2.len > len)
        return -1;
    if (fromIndex >= len)
        fromIndex = len - 1;
    int found = -1;
    for (char *p = buffer; p <= buffer + fromIndex; p++)
    {
        p = strstr(p, s2.buffer);
        if (!p)
            break;
        if ((unsigned int)(p - buffer) <= fromIndex)
            found = p - buffer;
    }
    return found;
}

String String::substring(unsigned int left) const
{
    return substring(left, len);
}

String String::substring(unsigned int left, unsigned int right) const
{
    if (left > right)
    {
        unsigned int temp = right;
        right = left;
        left = temp;
    }
    String out;
    if (left > len)
        return out;
    if (right > len)
        right = len;
    char temp = buffer[right]; // save the replaced character
    buffer[right] = '\0';
    out = buffer + left;  // pointer arithmetic
    buffer[right] = temp; // restore character
    return out;
}

/*********************************************/
/*  Modification                             */
/*********************************************/

String &String::replace(char find, char replace)
{
    if (!buffer)
        return *this;
    for (char *p = buffer; *p; p++)
    {
        if (*p == find)
            *p = replace;
    }
    return *this;
}

String &String::replace(const String &find, const String &replace)
{
    if (len == 0 || find.len == 0)
        return *this;
    int diff = replace.len - find.len;
    char *readFrom = buffer;
    char *foundAt;
    if (diff == 0)
    {
        while ((foundAt = strstr(readFrom, find.buffer)) != NULL)
        {
            memcpy(foundAt, replace.buffer, replace.len);
            readFrom = foundAt + replace.len;
        }
    }
    else if (diff < 0)
    {
        char *writeTo = buffer;
        while ((foundAt = strstr(readFrom, find.buffer)) != NULL)
        {
            unsigned int n = foundAt - readFrom;
            memcpy(writeTo, readFrom, n);
            writeTo += n;
            memcpy(writeTo, replace.buffer, replace.len);
            writeTo += replace.len;
            readFrom = foundAt + find.len;
            len += diff;
        }
        strcpy(writeTo, readFrom);
    }
    else
    {
        unsigned int size = len; // compute size needed for result
        while ((foundAt = strstr(readFrom, find.buffer)) != NULL)
        {
            readFrom = foundAt + find.len;
            size += diff;
        }
        if (size == len)
            return *this;
        if (size > capacity && !changeBuffer(size))
            return *this;
        int index = len - 1;
        while (index >= 0 && (index = lastIndexOf(find, index)) >= 0)
        {
            readFrom = buffer + index + find.len;
            memmove(readFrom + diff, readFrom, len - (readFrom - buffer));
            len += diff;
            buffer[len] = 0;
            memcpy(buffer + index, replace.buffer, replace.len);
            index--;
        }
    }
    return *this;
}

String &String::remove(unsigned int index)
{
    if (index < len)
    {
        len = index;
        buffer[len] = 0;
    }
    return *this;
}

String &String::remove(unsigned int index, unsigned int count)
{
    if (index < len && count > 0)
    {
        if (index + count > len)
            count = len - index;
        len = len - count;
        memmove(buffer + index, buffer + index + count, len - index);
        buffer[len] = 0;
    }
    return *this;
}

String &String::toLowerCase(void)
{
    if (!buffer)
        return *this;
    for (char *p = buffer; *p; p++)
    {
        *p = tolower(*p);
    }
    return *this;
}

String &String::toUpperCase(void)
{
    if (!buffer)
        return *this;
    for (char *p = buffer; *p; p++)
    {
        *p = toupper(*p);
    }
    return *this;
}

String &String::trim(void)
{
    if (!buffer || len == 0)
        return *this;
    char *begin = buffer;
    while (isspace(*begin))
        begin++;
    char *end = buffer + len - 1;
    while (isspace(*end) && end >= begin)
        end--;
    len = end + 1 - begin;
    if (begin > buffer)
        memcpy(buffer, begin, len);
    buffer[len] = 0;
    return *this;
}

/*********************************************/
/*  Parsing / Conversion                     */
/*********************************************/

long String::toInt(void) const
{
    if (buffer)
        return atol(buffer);
    return 0;
}

float String::toFloat(void) const
{
    if (buffer)
        return strtof(buffer, (char **)NULL);
    return 0.0;
}




class Print;

/** The Printable class provides a way for new classes to allow themselves to be printed.
    By deriving from Printable and implementing the printTo method, it will then be possible
    for users to print out instances of this class by passing them into the usual
    Print::print and Print::println methods.
*/
class Printable
{
  public:
    virtual size_t printTo(Print &p) const = 0;
};





class Print
{
  public:
    constexpr Print() : write_error(0)
    {
    }
    virtual size_t write(uint8_t b) = 0;
    size_t write(const char *str)
    {
        return write((const uint8_t *)str, strlen(str));
    }
    virtual size_t write(const uint8_t *buffer, size_t size);
    virtual int availableForWrite(void)
    {
        return 0;
    }
    virtual void flush()
    {
    }
    size_t write(const char *buffer, size_t size)
    {
        return write((const uint8_t *)buffer, size);
    }
    // Print a string
    size_t print(const String &s);
    // Print a single character
    size_t print(char c)
    {
        return write((uint8_t)c);
    }
    // Print a string
    size_t print(const char s[])
    {
        return write(s);
    }
    // Print a string
    size_t print(const __FlashStringHelper *f)
    {
        return write((const char *)f);
    }
    // Print an unsigned number
    size_t print(uint8_t b)
    {
        return printNumber(b, 10, 0);
    }
    // Print a signed number
    size_t print(int n)
    {
        return print((long)n);
    }
    // Print an unsigned number
    size_t print(unsigned int n)
    {
        return printNumber(n, 10, 0);
    }
    // Print a signed number
    size_t print(long n);
    // Print an unsigned number
    size_t print(unsigned long n)
    {
        return printNumber(n, 10, 0);
    }
    // Print a signed number
    size_t print(int64_t n);
    // Print an unsigned number
    size_t print(uint64_t n)
    {
        return printNumber64(n, 10, 0);
    }

    // Print a number in any number base (eg, BIN, HEX, OCT)
    size_t print(unsigned char n, int base)
    {
        return printNumber(n, base, 0);
    }
    // Print a number in any number base (eg, BIN, HEX, OCT)
    size_t print(int n, int base)
    {
        return (base == 10) ? print(n) : printNumber(n, base, 0);
    }
    // Print a number in any number base (eg, BIN, HEX, OCT)
    size_t print(unsigned int n, int base)
    {
        return printNumber(n, base, 0);
    }
    // Print a number in any number base (eg, BIN, HEX, OCT)
    size_t print(long n, int base)
    {
        return (base == 10) ? print(n) : printNumber(n, base, 0);
    }
    // Print a number in any number base (eg, BIN, HEX, OCT)
    size_t print(unsigned long n, int base)
    {
        return printNumber(n, base, 0);
    }
    // Print a number in any number base (eg, BIN, HEX, OCT)
    size_t print(int64_t n, int base)
    {
        return (base == 10) ? print(n) : printNumber64(n, base, 0);
    }
    // Print a number in any number base (eg, BIN, HEX, OCT)
    size_t print(uint64_t n, int base)
    {
        return printNumber64(n, base, 0);
    }

    // Print a floating point (decimal) number
    size_t print(double n, int digits = 2)
    {
        return printFloat(n, digits);
    }
    // Print an object instance in human readable format
    size_t print(const Printable &obj)
    {
        return obj.printTo(*this);
    }
    // Print a newline
    size_t println(void);
    // Print a string and newline
    size_t println(const String &s)
    {
        return print(s) + println();
    }
    // Print a single character and newline
    size_t println(char c)
    {
        return print(c) + println();
    }
    // Print a string and newline
    size_t println(const char s[])
    {
        return print(s) + println();
    }
    // Print a string and newline
    size_t println(const __FlashStringHelper *f)
    {
        return print(f) + println();
    }

    // Print an unsigned number and newline
    size_t println(uint8_t b)
    {
        return print(b) + println();
    }
    // Print a signed number and newline
    size_t println(int n)
    {
        return print(n) + println();
    }
    // Print an unsigned number and newline
    size_t println(unsigned int n)
    {
        return print(n) + println();
    }
    // Print a signed number and newline
    size_t println(long n)
    {
        return print(n) + println();
    }
    // Print an unsigned number and newline
    size_t println(unsigned long n)
    {
        return print(n) + println();
    }
    // Print a signed number and newline
    size_t println(int64_t n)
    {
        return print(n) + println();
    }
    // Print an unsigned number and newline
    size_t println(uint64_t n)
    {
        return print(n) + println();
    }

    // Print a number in any number base (eg, BIN, HEX, OCT) and a newline
    size_t println(unsigned char n, int base)
    {
        return print(n, base) + println();
    }
    // Print a number in any number base (eg, BIN, HEX, OCT) and a newline
    size_t println(int n, int base)
    {
        return print(n, base) + println();
    }
    // Print a number in any number base (eg, BIN, HEX, OCT) and a newline
    size_t println(unsigned int n, int base)
    {
        return print(n, base) + println();
    }
    // Print a number in any number base (eg, BIN, HEX, OCT) and a newline
    size_t println(long n, int base)
    {
        return print(n, base) + println();
    }
    // Print a number in any number base (eg, BIN, HEX, OCT) and a newline
    size_t println(unsigned long n, int base)
    {
        return print(n, base) + println();
    }
    // Print a number in any number base (eg, BIN, HEX, OCT) and a newline
    size_t println(int64_t n, int base)
    {
        return print(n, base) + println();
    }
    // Print a number in any number base (eg, BIN, HEX, OCT) and a newline
    size_t println(uint64_t n, int base)
    {
        return print(n, base) + println();
    }

    // Print a floating point (decimal) number and a newline
    size_t println(double n, int digits = 2)
    {
        return print(n, digits) + println();
    }
    // Print an object instance in human readable format, and a newline
    size_t println(const Printable &obj)
    {
        return obj.printTo(*this) + println();
    }
    int getWriteError()
    {
        return write_error;
    }
    void clearWriteError()
    {
        setWriteError(0);
    }

    // printf is a C standard function which allows you to print any number of variables using a somewhat cryptic format
    // string
    int printf(const char *format, ...);
    // printf is a C standard function which allows you to print any number of variables using a somewhat cryptic format
    // string
    int printf(const __FlashStringHelper *format, ...);

    // format warnings are too pedantic - disable until newer toolchain offers better...
    // https://forum.pjrc.com/threads/62473?p=256873&viewfull=1#post256873
    // int printf(const char *format, ...) __attribute__ ((format (printf, 2, 3)));

  protected:
    void setWriteError(int err = 1)
    {
        write_error = err;
    }

  private:
    char write_error;
    size_t printFloat(double n, uint8_t digits);
    size_t printNumber(unsigned long n, uint8_t base, uint8_t sign);
    size_t printNumber64(uint64_t n, uint8_t base, uint8_t sign);
};



size_t Print::write(const uint8_t *buffer, size_t size)
{
    if (buffer == nullptr)
        return 0;
    size_t count = 0;
    while (size--)
        count += write(*buffer++);
    return count;
}

size_t Print::print(const String &s)
{
    uint8_t buffer[33];
    size_t count = 0;
    unsigned int index = 0;
    unsigned int len = s.length();
    while (len > 0)
    {
        s.getBytes(buffer, sizeof(buffer), index);
        unsigned int nbytes = len;
        if (nbytes > sizeof(buffer) - 1)
            nbytes = sizeof(buffer) - 1;
        index += nbytes;
        len -= nbytes;
        count += write(buffer, nbytes);
    }
    return count;
}

size_t Print::print(long n)
{
    uint8_t sign = 0;

    if (n < 0)
    {
        sign = '-';
        n = -n;
    }
    return printNumber(n, 10, sign);
}

size_t Print::print(int64_t n)
{
    if (n < 0)
        return printNumber64(-n, 10, 1);
    return printNumber64(n, 10, 0);
}

size_t Print::println(void)
{
    uint8_t buf[2] = {'\r', '\n'};
    return write(buf, 2);
}
//
//extern "C"
//{
//    __attribute__((weak)) int _write(int file, char *ptr, int len)
//    {
//        ((class Print *)file)->write((uint8_t *)ptr, len);
//        return len;
//    }
//}

//int Print::printf(const char *format, ...)
//{
//    va_list ap;
//    va_start(ap, format);
//#ifdef __STRICT_ANSI__
//    va_end(ap);
//    return 0; // TODO: make this work with -std=c++0x
//#else
//    int retval = vdprintf((int)this, format, ap);
//    va_end(ap);
//    return retval;
//#endif
//}
//
//int Print::printf(const __FlashStringHelper *format, ...)
//{
//    va_list ap;
//    va_start(ap, format);
//#ifdef __STRICT_ANSI__
//    va_end(ap);
//    return 0;
//#else
//    int retval = vdprintf((int)this, (const char *)format, ap);
//    va_end(ap);
//    return retval;
//#endif
//}

size_t Print::printNumber(unsigned long n, uint8_t base, uint8_t sign)
{
    uint8_t buf[34];
    uint8_t digit, i;

    // TODO: make these checks as inline, since base is
    // almost always a constant.  base = 0 (BYTE) should
    // inline as a call directly to write()
    if (base == 0)
    {
        return write((uint8_t)n);
    }
    else if (base == 1)
    {
        base = 10;
    }

    if (n == 0)
    {
        buf[sizeof(buf) - 1] = '0';
        i = sizeof(buf) - 1;
    }
    else
    {
        i = sizeof(buf) - 1;
        while (1)
        {
            digit = n % base;
            buf[i] = ((digit < 10) ? '0' + digit : 'A' + digit - 10);
            n /= base;
            if (n == 0)
                break;
            i--;
        }
    }
    if (sign)
    {
        i--;
        buf[i] = '-';
    }
    return write(buf + i, sizeof(buf) - i);
}

size_t Print::printNumber64(uint64_t n, uint8_t base, uint8_t sign)
{
    uint8_t buf[66];
    uint8_t digit, i;

    if (base < 2)
        return 0;
    if (n == 0)
    {
        buf[sizeof(buf) - 1] = '0';
        i = sizeof(buf) - 1;
    }
    else
    {
        i = sizeof(buf) - 1;
        while (1)
        {
            digit = n % base;
            buf[i] = ((digit < 10) ? '0' + digit : 'A' + digit - 10);
            n /= base;
            if (n == 0)
                break;
            i--;
        }
    }
    if (sign)
    {
        i--;
        buf[i] = '-';
    }
    return write(buf + i, sizeof(buf) - i);
}

size_t Print::printFloat(double number, uint8_t digits)
{
    uint8_t sign = 0;
    size_t count = 0;

    if (isnan(number))
        return print("nan");
    if (isinf(number))
        return print("inf");
    if (number > 4294967040.0f)
        return print("ovf"); // constant determined empirically
    if (number < -4294967040.0f)
        return print("ovf"); // constant determined empirically

    // Handle negative numbers
    if (number < 0.0)
    {
        sign = 1;
        number = -number;
    }

    // Round correctly so that print(1.999, 2) prints as "2.00"
    double rounding = 0.5;
    for (uint8_t i = 0; i < digits; ++i)
    {
        rounding *= 0.1;
    }
    number += rounding;

    // Extract the integer part of the number and print it
    unsigned long int_part = (unsigned long)number;
    double remainder = number - (double)int_part;
    count += printNumber(int_part, 10, sign);

    // Print the decimal point, but only if there are digits beyond
    if (digits > 0)
    {
        uint8_t n, buf[16], count = 1;
        buf[0] = '.';

        // Extract digits from the remainder one at a time
        if (digits > sizeof(buf) - 1)
            digits = sizeof(buf) - 1;

        while (digits-- > 0)
        {
            remainder *= 10.0;
            n = (uint8_t)(remainder);
            buf[count++] = '0' + n;
            remainder -= n;
        }
        count += (uint8_t)write(buf, count);
    }
    return count;
}
