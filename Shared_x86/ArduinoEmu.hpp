

#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdio.h>
#include <Windows.h>
#undef MIN
#undef MAX
#undef min
#undef max

#include <stdint.h>
#include <string>
#include <sstream>
#include <algorithm>

struct String
{
  std::stringstream mStr;
  String() {}
  String(const char * s) : mStr(s) {}
  String(const String& rhs) { mStr << rhs.mStr.str(); }
  template<typename T>
  String(const T& rhs) { mStr << rhs; }
  size_t length() const { return mStr.str().length(); }
  template<typename T>
  String& operator +=(const T& rhs) {
    append(rhs);
    return *this;
  }
  template<typename T>
  String operator +(const T& rhs) {
    String ret(*this);
    ret += rhs;
    return ret;
  }
  template<typename T>
  String& append(const T& n) {
    mStr << n;
    return *this;
  }
  String& append(const String& n) {
    mStr << n.mStr.str();
    return *this;
  }
  String& operator =(const String& s) {
    mStr.clear();
    mStr << s.mStr.str();
    return *this;
  }
};

struct
{
  void println(const String& str)
  {
    print(str);
    print("\r\n");
  }
  void print(const String& str)
  {
    ::OutputDebugStringA(str.mStr.str().c_str());
  }
  void begin(uint32_t baud) {}
  bool operator !() { return true; }
} Serial;

void delay(uint32_t ms) {
  ::Sleep(ms);
}

uint32_t millis() {
  return GetTickCount();
}

uint32_t micros() {
  return GetTickCount() * 1000;
}


static inline void Die(const String& msg) {
  Serial.print(msg);
  DebugBreak();
}


#define CCASSERT(x) if (!(x)) { Die(String("!Assert! ") + __FILE__ + ":" + (int)__LINE__); }


namespace cc
{
  template <typename ...Args>
  static void log(const std::string& format, Args && ...args)
  {
    std::string fmt = std::string("[%x:%x] ") + format + "\r\n";
    auto size = std::snprintf(nullptr, 0, fmt.c_str(), GetCurrentProcessId(), GetCurrentThreadId(), std::forward<Args>(args)...);
    std::string output(size + 2, '\0');// to ensure the null-terminator
    output.resize(size);// so the reported length is correct.
    std::sprintf(&output[0], fmt.c_str(), GetCurrentProcessId(), GetCurrentThreadId(), std::forward<Args>(args)...);
    OutputDebugStringA(output.c_str());
  }


}

