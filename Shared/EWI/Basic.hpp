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

// copies memory from p1 to p2, but then puts memory from p2 to p3.
// |AaaaaBbb--aaa|
//            ^ p1
//       ^ p2 
//          ^ p3
static void MemCpyTriple(const Ptr& p1begin, const Ptr& p1end, const Ptr& p2begin, const Ptr& p2SourceEnd, Ptr p3)
{
  // p3 cannot be inside p1 or p2.
  CCASSERT(p3 <= p1begin); // you cannot output into unread source territory.
  CCASSERT(p3 >= p2SourceEnd);
  CCASSERT(p2begin < p3); // you will overwrite your dest with other dest.
  Ptr p1 = p1begin;
  Ptr p2 = p2begin;
  while (true) {
    uint8_t temp = 0;
    if (p2 < p2SourceEnd) {
      p2.Peek(temp);
    }
    else if (p1 > p1end) {
      // both past end; bail.
      return;
    }
    if (p1 < p1end) {
      p2.WriteInPlace<uint8_t>(p1.Peek<uint8_t>());
    }
    if (p2 < p2SourceEnd) {
      p3.WriteInPlace(temp);
    }
    p1.AdvanceBytes(1);
    p2.AdvanceBytes(1);
    p3.AdvanceBytes(1);
  }
}

// shift a split circular buffer into place without using some temp buffer. avoids OOM.
// returns the total size of the resulting buffer which will start at segmentBBegin.
// |Bbbb----Aaaa|  => |AaaaBbbb----|
template<size_t tempBufferBytes = 1>
static size_t UnifyCircularBuffer_Left(const Ptr& segmentABegin, const Ptr& segmentAEnd, const Ptr& segmentBBegin, const Ptr& segmentBEnd)
{
  CCASSERT(segmentBEnd >= segmentBBegin);
  CCASSERT(segmentABegin >= segmentBEnd);
  CCASSERT(segmentAEnd >= segmentABegin);

  // |Bbb--Aaaaaaaa| => |AaaaaaaaBbb--|
  // |Bbbbbbbb--Aaa| => |AaaBbbbbbbb--|

  size_t sizeA = segmentAEnd.mP - segmentABegin.mP;
  size_t sizeB = segmentBEnd.mP - segmentBBegin.mP;

  if (sizeA == 0) {
    // nothing to do; B already in place and A doesn't exist.
    return sizeB;
  }

  if (sizeB == 0)
  {
    // slide A into place
    OrderedMemcpy(segmentBBegin, segmentABegin, sizeA);
    return sizeA;
  }

  size_t len = segmentABegin.mP - segmentBBegin.mP;

  if (sizeA >= sizeB) {
    if (len >= sizeA) {
      // there's enough empty space to copy all of A in one shot.
      // |Bbb------Aaaaaaaa|
      // =>
      // |AaaaaaaaBbb------|
      MemCpyTriple(segmentABegin, segmentAEnd, segmentBBegin, segmentBEnd, segmentBBegin.PlusBytes(sizeA));
      return sizeA + sizeB;
    }

    // |Bbb--Aaaaaaaaaaa|

    // without a temp buffer, this scenario can get messy fast, recursing for every len bytes, and len could be 1.
    if (tempBufferBytes > len) {
      CCASSERT(len >= sizeB);
      // so the temp buffer can hold entire B.
      uint8_t tempBuffer[tempBufferBytes];
      memcpy(tempBuffer, (void*)segmentBBegin.mP, sizeB);
      // shift A over
      OrderedMemcpy(segmentBBegin, segmentABegin, sizeA);
      // and place B.
      memcpy((void*)segmentBBegin.PlusBytes(sizeA).mP, tempBuffer, sizeB);
      return sizeA + sizeB;
    }

    // copy a len-sized chunk of A to the front.
    // |Bbb--Aaaaaaaaaaa|
    // =>
    // |AaaaaBbb--aaaaaa|
    SwapMem(segmentABegin, segmentABegin.PlusBytes(len), segmentBBegin);

    // recurse because what we have left is a mini version of this buffer.
    // |AaaaaBbb--aaaaaa|
    //      |Bbb--aaaaaa|
    // =>   |aaaaaaBbb--|
    UnifyCircularBuffer_Left<tempBufferBytes>(segmentABegin.PlusBytes(len), segmentAEnd, segmentBBegin.PlusBytes(len), segmentBBegin.PlusBytes(len + sizeB));

    return sizeA + sizeB;
  }

  if (len >= (sizeA + sizeB)) {
    // enough space to make sure nothing overlaps while writing.
    //    |Bbbbbbbb------Aaa|
    // => |AaaBbbbbbbb------|
    OrderedMemcpy(segmentBBegin.PlusBytes(sizeA), segmentBBegin, sizeB); // move B right
    OrderedMemcpy(segmentBBegin, segmentABegin, sizeA);// move A into place.
    return sizeA + sizeB;
  }

  // here we have to swap chunks using our own buffer as a temp buffer
  //    |Bbbbbbbb--Aaa|
  // first just get A into place.
  SwapMem(segmentABegin, segmentAEnd, segmentBBegin);

  // => |Aaabbbbb--Bbb|
  // now we need to piece back together B. in fact it's just a mini-version of the big buffer. recurse.
  UnifyCircularBuffer_Left<tempBufferBytes>(segmentABegin, segmentAEnd, segmentBBegin.PlusBytes(sizeA), segmentBEnd);
  return sizeA + sizeB;
}

template<size_t tempBufferBytes = 1>
static size_t UnifyCircularBuffer_Left(void* segmentABegin, void* segmentAEnd, void* segmentBBegin, void* segmentBEnd)
{
  return UnifyCircularBuffer_Left(Ptr(segmentABegin), Ptr(segmentAEnd), Ptr(segmentBBegin), Ptr(segmentBEnd));
}
