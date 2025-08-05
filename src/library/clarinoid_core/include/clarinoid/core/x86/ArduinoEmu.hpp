
#pragma once

#ifndef CLARINOID_PLATFORM_X86
  #error This is only for x86 platform
#endif

#define PROGMEM
#define EXTMEM

#ifndef AUDIO_BLOCK_SAMPLES
  #define AUDIO_BLOCK_SAMPLES 128
#endif

#include <iostream>
#include <stdio.h>
#ifndef NOMINMAX
  #define NOMINMAX
#endif
#include <Windows.h>

#include <algorithm>
#include <sstream>
#include <stdint.h>
#include <string>

// from core_pins.h
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define INPUT_PULLDOWN 3
#define OUTPUT_OPENDRAIN 4
#define INPUT_DISABLE 5
#define LSBFIRST 0
#define MSBFIRST 1
#define _BV(n) (1 << (n))
#define CHANGE 4
#define FALLING 2
#define RISING 3

#define DMAMEM
#define F

class __FlashStringHelper;


extern uint64_t gTestClockMicros;

inline uint32_t millis()
{
  return (uint32_t)((gTestClockMicros / 1000) & 0xffffffff);
}

inline uint32_t micros()
{
  return (uint32_t)(gTestClockMicros & 0xffffffff);
}

#include "../basic/Uptime.hpp"

inline void SetTestClockMillis(int64_t ms)
{
  clarinoid::UptimeReset();
  gTestClockMicros = ms * 1000;
}
inline void SetTestClockMicros(int64_t m)
{
  clarinoid::UptimeReset();
  gTestClockMicros = m;
}

inline void delay(uint32_t ms)
{
  gTestClockMicros += ((uint64_t)ms) * 1000;
}
inline void delayMicroseconds(uint32_t m)
{
  gTestClockMicros += m;
}

inline void yield() {}


// An inherited class for holding the result of a concatenation.  These
// result objects are assumed to be writable by subsequent concatenations.
class StringSumHelper;

// The string class
class String
{
public:
  // constructors
  String(const char* cstr = (const char*)NULL);
  String(const __FlashStringHelper* pgmstr);
  String(const String& str);
#if __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__)
  String(String&& rval);
  String(StringSumHelper&& rval);
#endif
  String(char c);
  String(unsigned char c);
  String(int, unsigned char base = 10);
  String(unsigned int, unsigned char base = 10);
  String(long, unsigned char base = 10);
  String(unsigned long, unsigned char base = 10);
  String(long long, unsigned char base = 10);
  String(unsigned long long, unsigned char base = 10);
  String(float num, unsigned char digits = 2);
  String(double num, unsigned char digits = 2)
      : String((float)num, digits)
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
  String& copy(const char* cstr, unsigned int length);
  String& copy(const __FlashStringHelper* s)
  {
    return copy((const char*)s, strlen((const char*)s));
  }
  void move(String& rhs);
  String& operator=(const String& rhs);
  String& operator=(const char* cstr);
  String& operator=(const __FlashStringHelper* pgmstr);
#if __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__)
  String& operator=(String&& rval);
  String& operator=(StringSumHelper&& rval);
#endif
  String& operator=(char c);

  // append
  String& append(const String& str);
  String& append(const char* cstr);
  String& append(const __FlashStringHelper* s)
  {
    return append((const char*)s, strlen((const char*)s));
  }
  String& append(char c);
  String& append(unsigned char c)
  {
    return append((int)c);
  }
  String& append(int num);
  String& append(unsigned int num);
  String& append(long num);
  String& append(unsigned long num);
  String& append(long long num);
  String& append(unsigned long long num);
  String& append(float num);
  String& append(double num)
  {
    return append((float)num);
  }
  String& operator+=(const String& rhs)
  {
    return append(rhs);
  }
  String& operator+=(const char* cstr)
  {
    return append(cstr);
  }
  String& operator+=(const __FlashStringHelper* pgmstr)
  {
    return append(pgmstr);
  }
  String& operator+=(char c)
  {
    return append(c);
  }
  String& operator+=(unsigned char c)
  {
    return append((int)c);
  }
  String& operator+=(int num)
  {
    return append(num);
  }
  String& operator+=(unsigned int num)
  {
    return append(num);
  }
  String& operator+=(long num)
  {
    return append(num);
  }
  String& operator+=(unsigned long num)
  {
    return append(num);
  }
  String& operator+=(long long num)
  {
    return append(num);
  }
  String& operator+=(unsigned long long num)
  {
    return append(num);
  }
  String& operator+=(float num)
  {
    return append(num);
  }
  String& operator+=(double num)
  {
    return append(num);
  }

  // concatenate
  friend StringSumHelper& operator+(const StringSumHelper& lhs, const String& rhs);
  friend StringSumHelper& operator+(const StringSumHelper& lhs, const char* cstr);
  friend StringSumHelper& operator+(const StringSumHelper& lhs, const __FlashStringHelper* pgmstr);
  friend StringSumHelper& operator+(const StringSumHelper& lhs, char c);
  friend StringSumHelper& operator+(const StringSumHelper& lhs, unsigned char c);
  friend StringSumHelper& operator+(const StringSumHelper& lhs, int num);
  friend StringSumHelper& operator+(const StringSumHelper& lhs, unsigned int num);
  friend StringSumHelper& operator+(const StringSumHelper& lhs, long num);
  friend StringSumHelper& operator+(const StringSumHelper& lhs, unsigned long num);
  friend StringSumHelper& operator+(const StringSumHelper& lhs, long long num);
  friend StringSumHelper& operator+(const StringSumHelper& lhs, unsigned long long num);
  friend StringSumHelper& operator+(const StringSumHelper& lhs, float num);
  friend StringSumHelper& operator+(const StringSumHelper& lhs, double num);
  String& concat(const String& str)
  {
    return append(str);
  }
  String& concat(const char* cstr)
  {
    return append(cstr);
  }
  String& concat(const __FlashStringHelper* pgmstr)
  {
    return append(pgmstr);
  }
  String& concat(char c)
  {
    return append(c);
  }
  String& concat(unsigned char c)
  {
    return append((int)c);
  }
  String& concat(int num)
  {
    return append(num);
  }
  String& concat(unsigned int num)
  {
    return append(num);
  }
  String& concat(long num)
  {
    return append(num);
  }
  String& concat(unsigned long num)
  {
    return append(num);
  }
  String& concat(long long num)
  {
    return append(num);
  }
  String& concat(unsigned long long num)
  {
    return append(num);
  }
  String& concat(float num)
  {
    return append(num);
  }
  String& concat(double num)
  {
    return append(num);
  }

  // comparison
  int compareTo(const String& s) const;
  unsigned char equals(const String& s) const;
  unsigned char equals(const char* cstr) const;
  //unsigned char equals(const __FlashStringHelper *pgmstr) const;
  unsigned char operator==(const String& rhs) const
  {
    return equals(rhs);
  }
  unsigned char operator==(const char* cstr) const
  {
    return equals(cstr);
  }
  unsigned char operator!=(const String& rhs) const
  {
    return !equals(rhs);
  }
  unsigned char operator!=(const char* cstr) const
  {
    return !equals(cstr);
  }
  unsigned char operator<(const String& rhs) const;
  unsigned char operator>(const String& rhs) const;
  unsigned char operator<=(const String& rhs) const;
  unsigned char operator>=(const String& rhs) const;
  unsigned char equalsIgnoreCase(const String& s) const;
  unsigned char startsWith(const String& prefix) const;
  unsigned char startsWith(const String& prefix, unsigned int offset) const;
  unsigned char endsWith(const String& suffix) const;

  // character acccess
  char charAt(unsigned int index) const;
  void setCharAt(unsigned int index, char c);
  char operator[](unsigned int index) const;
  char& operator[](unsigned int index);
  void getBytes(unsigned char* buf, unsigned int bufsize, unsigned int index = 0) const;
  void toCharArray(char* buf, unsigned int bufsize, unsigned int index = 0) const
  {
    getBytes((unsigned char*)buf, bufsize, index);
  }
  const char* c_str() const
  {
    if (!buffer)
      return &zerotermination;  // https://forum.pjrc.com/threads/63842
    return buffer;
  }
  char* begin()
  {
    if (!buffer)
      reserve(20);
    return buffer;
  }
  char* end()
  {
    return begin() + length();
  }
  const char* begin() const
  {
    return c_str();
  }
  const char* end() const
  {
    return c_str() + length();
  }

  // search
  int indexOf(char ch) const;
  int indexOf(char ch, unsigned int fromIndex) const;
  int indexOf(const String& str) const;
  int indexOf(const String& str, unsigned int fromIndex) const;
  int lastIndexOf(char ch) const;
  int lastIndexOf(char ch, unsigned int fromIndex) const;
  int lastIndexOf(const String& str) const;
  int lastIndexOf(const String& str, unsigned int fromIndex) const;
  String substring(unsigned int beginIndex) const;
  String substring(unsigned int beginIndex, unsigned int endIndex) const;

  // modification
  String& replace(char find, char replace);
  String& replace(const String& find, const String& replace);
  String& remove(unsigned int index);
  String& remove(unsigned int index, unsigned int count);
  String& toLowerCase(void);
  String& toUpperCase(void);
  String& trim(void);

  // parsing/conversion
  long toInt(void) const;
  float toFloat(void) const;

protected:
  char* buffer;           // the actual char array
  unsigned int capacity;  // the array length minus one (for the '\0')
  unsigned int len;       // the String length (not counting the '\0')
                          //unsigned char flags;    // unused, for future features
protected:
  void init(void);
  unsigned char changeBuffer(unsigned int maxStrLen);
  String& append(const char* cstr, unsigned int length);

private:
  // allow for "if (s)" without the complications of an operator bool().
  // for more information http://www.artima.com/cppsource/safebool.html
  typedef void (String::*StringIfHelperType)() const;
  void StringIfHelper() const {}
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
  StringSumHelper(const String& s)
      : String(s)
  {
  }
  StringSumHelper(const char* p)
      : String(p)
  {
  }
  //StringSumHelper(const __FlashStringHelper* pgmstr)
  //    : String(pgmstr)
  //{
  //}
  StringSumHelper(char c)
      : String(c)
  {
  }
  StringSumHelper(unsigned char c)
      : String(c)
  {
  }
  StringSumHelper(int num)
      : String(num, 10)
  {
  }
  StringSumHelper(unsigned int num)
      : String(num, 10)
  {
  }
  StringSumHelper(long num)
      : String(num, 10)
  {
  }
  StringSumHelper(unsigned long num)
      : String(num, 10)
  {
  }
  StringSumHelper(long long num)
      : String(num, 10)
  {
  }
  StringSumHelper(unsigned long long num)
      : String(num, 10)
  {
  }
};


#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

class Print
{
public:
  constexpr Print()
      : write_error(0)
  {
  }
  virtual size_t write(uint8_t b) = 0;
  size_t write(const char* str)
  {
    if (str == nullptr)
      return 0;
    return write((const uint8_t*)str, strlen(str));
  }
  virtual size_t write(const uint8_t* buffer, size_t size);
  virtual int availableForWrite(void)
  {
    return 0;
  }
  virtual void flush() {}
  size_t write(const char* buffer, size_t size)
  {
    return write((const uint8_t*)buffer, size);
  }
  // Print a string
  size_t print(const String& s);
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
  size_t print(const __FlashStringHelper* f)
  {
    return write((const char*)f);
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
  //size_t print(const Printable& obj)
  //{
  //  return obj.printTo(*this);
  //}
  // Print a newline
  size_t println(void);
  // Print a string and newline
  size_t println(const String& s)
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
  size_t println(const __FlashStringHelper* f)
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
  //size_t println(const Printable& obj)
  //{
  //  return obj.printTo(*this) + println();
  //}
  int getWriteError()
  {
    return write_error;
  }
  void clearWriteError()
  {
    setWriteError(0);
  }

  // printf is a C standard function which allows you to print any number of variables using a somewhat cryptic format string
  int printf(const char* format, ...);
  // printf is a C standard function which allows you to print any number of variables using a somewhat cryptic format string
  int printf(const __FlashStringHelper* format, ...);
  // vprintf is a C standard function that allows you to print a variable argument list with a format string
  //int vprintf(const char* format, va_list ap)
  //{
  //  return vdprintf((int)this, format, ap);
  //}

  // format warnings are too pedantic - disable until newer toolchain offers better...
  // https://forum.pjrc.com/threads/62473?p=256873&viewfull=1#post256873
  // int printf(const char *format, ...) __attribute__ ((format (printf, 2, 3)));

protected:
  void setWriteError(int err = 1)
  {
    write_error = err;
  }

private:
  int write_error;
  size_t printFloat(double n, uint8_t digits);
  size_t printNumber(unsigned long n, uint8_t base, uint8_t sign);
  size_t printNumber64(uint64_t n, uint8_t base, uint8_t sign);
};


struct SerialType
{
  void println(const String& str)
  {
    print(str);
    print("\r\n");
  }
  void print(const String& str)
  {
    ::OutputDebugStringA(str.c_str());
  }
  void begin(uint32_t baud) {}
  bool operator!() const
  {
    return true;
  }
  operator bool() const
  {
    return true;
  }
};

extern SerialType Serial;

inline void pinMode(uint8_t pin, uint8_t mode) {}
inline void init_pins(void) {}
inline void analogWrite(uint8_t pin, int val) {}
inline int analogRead(uint8_t pin)
{
  return 0;
}
inline uint32_t analogWriteRes(uint32_t bits)
{
  return 0;
}
inline uint32_t analogWriteResolution(uint32_t bits)
{
  return analogWriteRes(bits);
}

extern bool gPinStates[256];

inline void digitalWrite(uint8_t pin, uint8_t val)
{
  gPinStates[pin] = val;
}

inline bool digitalReadFast(uint8_t pin)
{
  return gPinStates[pin];
}

struct Encoder
{
  void write(int) {}
  int read()
  {
    return 0;
  }
};

struct audio_block_t
{
  int16_t data[AUDIO_BLOCK_SAMPLES];
  int transmittedAsIndex = 0;
};
//
//static audio_block_t gTestSrcBuffers[100];
//static audio_block_t gTestDestBuffers[100];
//static audio_block_t gTestTransmittedBuffers[100];
//static size_t gAllocatedDestBuffers = 0;

inline void FillAudioBuffer(audio_block_t& b, int16_t val)
{
  for (int16_t& s : b.data)
  {
    s = val;
  }
}
//
//void TestResetAudioStreams()
//{
//    gAllocatedDestBuffers = 0;
//    for (auto &b : gTestSrcBuffers)
//    {
//        FillAudioBuffer(b, 0);
//    }
//    for (auto &b : gTestDestBuffers)
//    {
//        FillAudioBuffer(b, 0);
//    }
//    for (auto &b : gTestTransmittedBuffers)
//    {
//        FillAudioBuffer(b, 0);
//    }
//}

struct AudioStream
{
  AudioStream(unsigned char ninput, audio_block_t** iqueue) {}
  static audio_block_t* allocate(void)
  {
    return nullptr;
    //auto *ret = &gTestDestBuffers[gAllocatedDestBuffers];
    //gAllocatedDestBuffers++;
    //return ret;
  }
  static void release(audio_block_t* block) {}
  void transmit(audio_block_t* block, unsigned char index)
  {
    //block->transmittedAsIndex = index;
    //gTestTransmittedBuffers[index] = *block;
  }
  audio_block_t* receiveReadOnly(unsigned int index)
  {
    return nullptr;
    //return &gTestSrcBuffers[index];
  }
  audio_block_t* receiveWritable(unsigned int index)
  {
    return nullptr;
    //return &gTestSrcBuffers[index];
  }

  virtual void update() = 0;
};

class Printable
{
public:
  virtual size_t printTo(Print& p) const = 0;
};


class CrashReportClass : public Printable
{
public:
  virtual size_t printTo(Print& p) const
  {
    return p.print("CrashReport");
  }
  static void clear();
  operator bool()
  {
    return true;
  }
  static void breadcrumb(unsigned int num, unsigned int value) {}
  static uint32_t checksum(volatile const void* data, int len)
  {
    volatile const uint16_t* p = (volatile const uint16_t*)data;
    uint32_t a = 1, b = 0;  // Adler Fletcher kinda, len < 720 bytes
    while (len > 0)
    {
      a += *p++;
      b += a;
      len -= 2;
    }
    a = a & 65535;
    b = b & 65535;
    return a | (b << 16);
  }
};


extern CrashReportClass CrashReport;
