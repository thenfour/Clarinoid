#pragma once

#include <Arduino.h>

#ifndef CCASSERT // LHRH get this ------------------------------

static inline void Die(const String& msg) {
  Serial.begin(9600);
  while(!Serial);
  Serial.println(msg);
  while(true) {
    delay(500);
  }
}

#define CCASSERT(x) if (!(x)) { Die(String("!Assert! ") + __FILE__ + ":" + (int)__LINE__); }
  
#endif // CCASSERT


struct IList {
  virtual int List_GetItemCount() const = 0;
  virtual String List_GetItemCaption(int i) const = 0;
};


static int RotateIntoRange(const int& val, const int& itemCount) {
  CCASSERT(itemCount > 0);
  int ret = val;
  while(ret < 0) {
    ret += itemCount; // todo: optimize
  }
  return ret % itemCount;
}

static inline int AddConstrained(int orig, int delta, int min_, int max_) {
  CCASSERT(max_ >= min_);
  if (max_ <= min_)
    return min_;
  int ret = orig + delta;
  int period = max_ - min_ + 1; // if [0,0], add 1 each time to reach 0. If [3,6], add 4.
  while (ret < min_)
    ret += period;
  while (ret > max_)
    ret -= period;
  return ret;
}


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


template<typename T>
void set(T *dest, size_t elements, T val)
{
  while (elements-- > 0)
  {
    *dest = val;
    ++dest;
  }
}

// assumes T is integral
// performs integral division but with common 0.5 rounding
template<typename T>
T idiv_round(T dividend, T divisor)
{
    return (dividend + (divisor / 2)) / divisor;
}


String ToString(void* p) {
  static char x[20];
  sprintf(x, "%p", p);
  return String(x);
}

const char *ToString(bool p) {
  if (p) return "true";
  return "false";
}


enum class Tristate
{
  Null,
  Position1,
  Position2,
  Position3
};

const char *ToString(Tristate t) {
  switch (t){
    case Tristate::Position1:
      return "Pos1";
    case Tristate::Position2:
      return "Pos1";
    case Tristate::Position3:
      return "Pos3";
    default:
      break;
  }
  return "null";
}
