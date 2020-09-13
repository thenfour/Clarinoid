

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



//////////////////////////////////////////////////////////////////////
struct Stopwatch
{
  uint64_t mExtra = 0; // store overflow here. yes there's still overflow but this helps
  uint32_t mStartTime = 0;

  Stopwatch() {
    Restart();
  }

  // behave essentially like POD
  Stopwatch(const Stopwatch& rhs) = default;
  Stopwatch(Stopwatch&&) = default;
  Stopwatch& operator =(const Stopwatch& rhs) = default;
  Stopwatch& operator =(Stopwatch&&) = default;

  void Restart(uint64_t newTime = 0)
  {
    mExtra = newTime;
    mStartTime = micros();
  }

  uint64_t ElapsedMicros() {
    uint32_t now = micros();
    if (now < mStartTime) {
      mExtra += 0xffffffff - mStartTime;
      mStartTime = 0;
    }
    return (now - mStartTime) + mExtra;
  }
};


//////////////////////////////////////////////////////////////////////

// random utility function
template<typename T, size_t N>
constexpr size_t SizeofStaticArray(const T(&x)[N])
{
  return N;
}

template<typename T, size_t N>
constexpr const T* EndPtr(const T(&x)[N])
{
  return &x[N];
}

template<typename T, size_t N>
constexpr T* EndPtr(T(&x)[N])
{
  return &x[N];
}

int gThrottlerCount = 0;


template<uint32_t TperiodMS>
class CCThrottlerT
{
  uint32_t mPhase;
  uint32_t mPeriodStartMS;
  uint32_t mFirstPeriodStartMS;
public:
  CCThrottlerT() :
    mPhase(gThrottlerCount)
  {
    gThrottlerCount++;
    mPeriodStartMS = mFirstPeriodStartMS = millis();
  }

  void Reset() {
    mPeriodStartMS = mFirstPeriodStartMS = millis();
  }

  bool IsReady() {
    return IsReady(TperiodMS);
  }

  float GetBeatFloat(uint32_t periodMS) const {
    auto now = millis() + mPhase; // minus is more theoretically accurate but this serves the purpose just as well.
    float f = abs(float(now - mFirstPeriodStartMS) / periodMS);
    return f;
  }
  // returns 0-1 the time since the last "beat".
  float GetBeatFrac(uint32_t periodMS) const {
    float f = GetBeatFloat(periodMS);
    return f - floor(f); // fractional part only.
  }
  int GetBeatInt(uint32_t periodMS) const {
    float f = GetBeatFloat(periodMS);
    return (int)floor(f);
  }

  bool IsReady(uint32_t periodMS) {
    auto now = millis() + mPhase; // minus is more theoretically accurate but this serves the purpose just as well.
    if (now - mPeriodStartMS < periodMS) {
      return false;
    }
    mPeriodStartMS += periodMS * ((now - mPeriodStartMS) / periodMS); // this potentially advances multiple periods if needed so we don't get backed up.
    return true;
  }
};


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


struct ScopeLog
{
  String mMsg;
  ScopeLog(const String& msg) : mMsg(msg)
  {
    Serial.print("{ ");
    Serial.println(msg);
  }
  ~ScopeLog()
  {
    Serial.print("} ");
    Serial.println(mMsg);
  }
};


