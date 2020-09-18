#pragma once

#ifndef EWI_UNIT_TESTS
// Unit test project must emulate arduino stuff.
#include <Arduino.h>
#endif

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

struct Ptr
{
  uintptr_t mP = 0;
  Ptr() { }
  Ptr(const Ptr& rhs) : mP(rhs.mP) { }
  explicit Ptr(void* p) : mP((uintptr_t)p) {}
  explicit Ptr(uintptr_t p) : mP(p) {}
  explicit Ptr(Ptr&& rhs) : mP(rhs.mP) {}

  Ptr& operator =(uintptr_t rhs) { mP = rhs; return *this; }
  Ptr& operator =(const Ptr& rhs) { mP = rhs.mP; return *this; }
  Ptr& operator =(Ptr&& rhs) { mP = rhs.mP; return *this; }

  bool operator ==(const Ptr& rhs) const { return mP == rhs.mP; }
  bool operator !=(const Ptr& rhs) const { return mP != rhs.mP; }
  bool operator <(const Ptr& rhs) const { return mP < rhs.mP; }
  bool operator <=(const Ptr& rhs) const { return mP <= rhs.mP; }
  bool operator >(const Ptr& rhs) const { return mP > rhs.mP; }
  bool operator >=(const Ptr& rhs) const { return mP >= rhs.mP; }

  operator bool() const { return !!mP; }

  Ptr PlusBytes(size_t b) const {
    return Ptr(mP + b);
  }
  void AdvanceBytes(int b) {
    mP += b;
  }

  size_t DistanceBytes(const Ptr& rhs) const {
    if (rhs.mP >= mP)
      return rhs.mP - mP;
    return mP - rhs.mP;
  }

  template<typename Tother>
  Tother Peek() const
  {
    Tother ret;
    Peek(ret);
    return ret;
  }
  template<typename Tother>
  void Peek(Tother& ret) const
  {
    Tother* otherP = (Tother*)mP;
    ret = *otherP;
  }

  template<typename Tother>
  Tother Read()
  {
    Tother ret;
    Read(ret);
    return ret;
  }
  template<typename Tother>
  void Read(Tother& ret)
  {
    Peek(ret);
    AdvanceBytes(sizeof(Tother));
  }
  template<typename Tother, size_t N>
  void ReadArray(Tother(&ret)[N])
  {
    Tother* pother = (Tother*)mP;
    Tother* pval = ret;
    Tother* pend = pother + N;
    mP = (uintptr_t)pend;
    for (; pother != pend; ++pother, ++pval) {
      *pval = *pother;
    }
  }
  template<typename Tother>
  void WriteInPlace(const Tother& val) {
    Tother* pother = (Tother*)mP;
    *pother = val;
  }
  template<typename Tother>
  void Write(const Tother& val) {
    Tother* pother = (Tother*)mP;
    *pother = val;
    ++pother;
    mP = (uintptr_t)pother;
  }
  template<typename Tother, size_t N>
  void WriteArray(const Tother(&val)[N]) {
    Tother* pother = (Tother*)mP;
    const Tother* pval = &(val[0]);
    Tother* pend = pother + N;
    mP = (uintptr_t)pend;
    for (; pother != pend; ++pother, ++pval) {
      *pother = *pval;
    }
  }
};


template<size_t divBits, typename Tval, typename Tremainder>
void DivRemBitwise(Tval val, size_t& wholeParts, Tremainder& remainder)
{
  auto mask = (1 << divBits) - 1;
  wholeParts = val / mask;
  auto rem = (val - (wholeParts * mask));
  CCASSERT(rem <= std::numeric_limits<Tremainder>::max());
  remainder = (Tremainder)rem;
}

// for moving left, we can go forward order.
// for moving right, we should go backwards.
static void OrderedMemcpy(Ptr dest, Ptr src, size_t bytes)
{
  if (dest == src)
    return;
  if (dest < src) {
    const Ptr destEnd = dest.PlusBytes(bytes);
    for (; dest != destEnd;) {
      dest.Write(src.Read<uint8_t>());
    }
    return;
  }
  src.AdvanceBytes(bytes - 1);
  for (Ptr d = dest.PlusBytes(bytes - 1); d >= dest; d.AdvanceBytes(-1), src.AdvanceBytes(-1)) {
    d.WriteInPlace(src.Peek<uint8_t>());
  }
}

static void SwapMem(Ptr begin, const Ptr& end, Ptr dest)
{
  if (begin == dest)
    return;
  uint8_t temp;
  for (; begin != end;) {
    begin.Peek(temp);
    begin.Write(dest.Peek<uint8_t>());
    dest.Write(temp);
  }
}
