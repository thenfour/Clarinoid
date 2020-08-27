
#ifndef CCUTIL_H
#define CCUTIL_H

#include <functional>

//////////////////////////////////////////////////////////////////////
const int MaxUpdateObjects = 30;

// allows globally-created objects to sign up for updates
uint8_t UpdateObjectCount = 0;
class IUpdateObject* UpdateObjects[MaxUpdateObjects];

class IUpdateObject
{
public:
  IUpdateObject() {
    UpdateObjects[UpdateObjectCount] = this;
    UpdateObjectCount ++;
  }
  virtual void setup() {}
  virtual void loop() {}
};

inline void SetupUpdateObjects() {
  for (uint8_t i = 0; i < UpdateObjectCount; ++ i)
  {
    UpdateObjects[i]->setup();
  }
}

inline void UpdateUpdateObjects() {
  for (uint8_t i = 0; i < UpdateObjectCount; ++ i)
  {
    UpdateObjects[i]->loop();
  }
}

//////////////////////////////////////////////////////////////////////
int gThrottlerCount = 0;
class CCThrottler
{
  uint32_t mPeriodMS;
  uint32_t mPhase;
  uint32_t mPeriodStartMS;
public:
  CCThrottler(uint32_t periodMS) :
    mPeriodMS(periodMS),
    mPhase(gThrottlerCount)
  {
    gThrottlerCount ++;
    mPeriodStartMS = millis();
  }

  void Reset() {
    mPeriodStartMS = millis();
  }

  bool IsReady() {
    return IsReady(mPeriodMS);
  }
  
  bool IsReady(uint32_t periodMS) {
    auto m = millis() + mPhase; // minus is more theoretically accurate but this serves the purpose just as well.
    if (m - mPeriodStartMS < periodMS) {
      return false;
    }
    mPeriodStartMS = m; // note that overshoot will not be subtracted! so periods will often be longer than requested.
    return true;
  }
};

//////////////////////////////////////////////////////////////////////
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
    gThrottlerCount ++;
    mPeriodStartMS = mFirstPeriodStartMS = millis();
  }

  void Reset() {
    mPeriodStartMS = mFirstPeriodStartMS = millis();
  }

  bool IsReady() {
    return IsReady(TperiodMS);
  }

  // TODO: THIS will get out of phase with the actual IsReady() triggers, because IsReady() resets
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


//////////////////////////////////////////////////////////////////////
// you can't toggle LEDs at the fastest rate possible; it just looks constant.
// but you can't just change state every X frames, because you'll catch phase and won't see both phases equally.
// for example if you blink every 10ms, but update every 20ms, then you just never see activity.
// to solve that, ANY changes during the minperiod will toggle the LED. so it can't double-toggle and reset.
class ActivityLED : IUpdateObject
{
  bool mState;
  bool mNeedsToggle;
  CCThrottler mThrottle;

public:
  ActivityLED(uint32_t minPeriodMS) :
    mState(false),
    mNeedsToggle(false),
    mThrottle(minPeriodMS)
  {
  }

  void Touch()
  {
    mNeedsToggle = true;
  }
  
  bool GetState() const
  {
    return mState;
  }
  
  virtual void setup() {}
  
  virtual void loop() {
    if (mThrottle.IsReady())
    {
      if (mNeedsToggle) {
        mState = !mState;
        mNeedsToggle = false;
      }
    }
  }
};

//////////////////////////////////////////////////////////////////////
// same but different timeout values depending on whether we're on or off. allows different pulse for on/off
class AsymmetricActivityLED : IUpdateObject
{
  bool mState;
  bool mNeedsToggle;
  uint32_t mOnPeriod;
  uint32_t mOffPeriod;
  CCThrottler mThrottle;

public:
  AsymmetricActivityLED(uint32_t onPeriod, uint32_t offPeriod) :
    mState(false),
    mNeedsToggle(false),
    mOnPeriod(onPeriod),
    mOffPeriod(offPeriod),
    mThrottle(onPeriod)
  {
  }

  void Touch()
  {
    mNeedsToggle = true;
  }
  
  bool GetState() const
  {
    return mState;
  }
  
  virtual void setup() {}
  
  virtual void loop() {
    if (mThrottle.IsReady(mState ? mOnPeriod : mOffPeriod))
    {
      if (mNeedsToggle) {
        mState = !mState;
        mNeedsToggle = false;
      }
    }
  }
};


//////////////////////////////////////////////////////////////////////
// Same as ActivityLED, but this tries to always return to an OFF state after a timeout.
// prevents the LED from just staying bright ON depending on phase. Use for encoder activity for example.
class TransientActivityLED : IUpdateObject
{
  bool mState;
  bool mNeedsToggle;
  CCThrottler mThrottle;
  CCThrottler mTurnOffTimeout;

public:
  TransientActivityLED(uint32_t minPeriodMS, uint32_t turnOffTimeoutMS) :
    mState(false),
    mNeedsToggle(false),
    mThrottle(minPeriodMS),
    mTurnOffTimeout(turnOffTimeoutMS)
  {
  }

  void Touch()
  {
    mNeedsToggle = true;
  }

  bool GetState() const
  {
    return mState;
  }
  
  virtual void setup() {}
  
  virtual void loop() {
    if (mThrottle.IsReady())
    {
      if (mNeedsToggle) {
        mNeedsToggle = false;
        mState = !mState;
        mTurnOffTimeout.Reset();
      } else if (mTurnOffTimeout.IsReady()) {
        mState = false;
      }
    }
  }
};



//////////////////////////////////////////////////////////////////////
// This one is off by default and you raise its flag when an event occurs.
// so similar to TransientActivity, but it's an event, not "activity".
// instant turn-on, delayed turn-off.
class TransientEventLED : IUpdateObject
{
  bool mState;
  CCThrottler mTurnOffTimeout;

public:
  TransientEventLED(uint32_t turnOffTimeoutMS) :
    mState(false),
    mTurnOffTimeout(turnOffTimeoutMS)
  {
  }

  void Touch()
  {
    mState = true;
    mTurnOffTimeout.Reset();
  }

  bool GetState() const
  {
    return mState;
  }
  
  virtual void setup() {}
  
  virtual void loop() {
    if (mTurnOffTimeout.IsReady()) {
      mState = false;
    }
  }
};



//////////////////////////////////////////////////////////////////////
// convert values to LED indication colorant values
inline uint8_t col(bool b) { return b ? 64 : 0; }
inline uint8_t col(bool b, uint8_t ledmin, uint8_t ledmax)
{
  return b ? ledmax : ledmin;
}
inline uint8_t col(float f01) { return (uint8_t)(f01 * 255); }
inline uint8_t col(float f01, int x)
{
  return (uint8_t)(f01 * x);
}


//////////////////////////////////////////////////////////////////////

// random utility function
template<typename T, size_t N>
constexpr size_t SizeofStaticArray(const T(&x)[N])
{
  return N;
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

//////////////////////////////////////////////////////////////////////
// from https://github.com/thenfour/demos/blob/fa35a727549c67222d2829310e2c427c09e347e7/demo3/src/core/algorithm.hpp
struct framerateCalculator {

  framerateCalculator() {
    g_TicksPerSecond = 1000;
    g_Time = millis();
    Framerate = 0.f;
    set(FramerateSecPerFrame, SizeofStaticArray(FramerateSecPerFrame), 0.f);
    FramerateSecPerFrameIdx = 0;
    FramerateSecPerFrameAccum = 0.0f;
  }

  void onFrame() {
    uint32_t current_time = millis();
    float DeltaTime = (float)(current_time - g_Time) / g_TicksPerSecond;
    g_Time = current_time;
    FramerateSecPerFrameAccum += DeltaTime - FramerateSecPerFrame[FramerateSecPerFrameIdx];
    FramerateSecPerFrame[FramerateSecPerFrameIdx] = DeltaTime;
    FramerateSecPerFrameIdx = (FramerateSecPerFrameIdx + 1) % SizeofStaticArray(FramerateSecPerFrame);
    Framerate = 1.0f / (FramerateSecPerFrameAccum / (float)SizeofStaticArray(FramerateSecPerFrame));
  }

  float getFPS() {
    return Framerate;
  }

private:

  uint32_t g_TicksPerSecond;
  uint32_t g_Time;
  float Framerate;
  float                   FramerateSecPerFrame[120];
  int                     FramerateSecPerFrameIdx;
  float                   FramerateSecPerFrameAccum;
};

template <size_t N>
class SimpleMovingAverage
{
  public:
    void Update(float sample)
    {
        if (num_samples_ < N)
        {
            samples_[num_samples_++] = sample;
            total_ += sample;
        }
        else
        {
            float& oldest = samples_[num_samples_++ % N];
            total_ += sample - oldest;
            oldest = sample;
        }
    }

    float GetValue() const {
      return total_ / min(num_samples_, N);
    }
    size_t GetSampleCount() const { return num_samples_; }

    void Clear() {
      total_ = 0;
      num_samples_ = 0;
    }

  private:
    float samples_[N];
    size_t num_samples_ = 0;
    float total_ = 0;
};


// allows throttled plotting to Serial.
class PlotHelper : IUpdateObject
{
  CCThrottlerT<5> mThrot;
  String mFields;
public:
  virtual void loop() {
    if (mThrot.IsReady()) {
      if (mFields.length() > 0) {
        if (Serial) {
          Serial.println(mFields);
        }
      }
    }
    mFields = "";
  }

  template<typename T>
  void AppendField(const T& s) {
    if (mFields.length() > 0) {
      mFields.append("\t");
    }
    mFields.append(s);
  }
};

PlotHelper gPlot;
template<typename T>
inline void CCPlot(const T& val) {
  gPlot.AppendField(val);
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

String ToString(void* p) {
  static char x[20];
  sprintf(x, "%p", p);
  return String(x);
}

const char *ToString(bool p) {
  if (p) return "true";
  return "false";
}

// helps interpreting touch keys like buttons. like a computer keyboard, starts repeating after an initial delay.
// untouching the key will reset the delays
template<int TrepeatInitialDelayMS, int TrepeatPeriodMS>
class BoolKeyWithRepeat
{
  bool mPrevState = false;
  CCThrottlerT<TrepeatInitialDelayMS> mInitialDelayTimer;
  CCThrottlerT<TrepeatPeriodMS> mRepeatTimer;
  bool mInitialDelayPassed = false;
  bool mIsTriggered = false; // one-frame
public:
  void Update(bool pressed) { // call once a frame. each call to this will reset IsTriggered
    
    if (!mPrevState && !pressed) {
      // typical idle state; nothing to do.
      mIsTriggered = false;
      return;
    }
    
    if (mPrevState && pressed) {
      // key repeat?
      if (!mInitialDelayPassed) {
        if (mInitialDelayTimer.IsReady()) {
          mInitialDelayPassed = true;
          mRepeatTimer.Reset();
          // retrig.
          mIsTriggered = true;
          return;
        }
        // during initial delay; nothing to do.
        mIsTriggered = false;
        return;
      }
      // mInitialDelayPassed is satisfied. now we can check the normal key repeat timer.
      if (mRepeatTimer.IsReady()) {
        // trig!
        mIsTriggered = true;
        return;
      }
      // between key repeats
      mIsTriggered = false;
      return;
    }
    
    if (!mPrevState && pressed) {
      // newly pressed. reset initial repeat delay
      mInitialDelayTimer.Reset();
      mInitialDelayPassed = false;
      mPrevState = true;
      mIsTriggered = true;
      return;      
    }
    
    if (mPrevState && !pressed) {
      // newly released.
      mIsTriggered = false;
      mPrevState = false;
      return;
    }
  }

  // valid during 1 frame.
  bool IsTriggered() const {
    return mIsTriggered;
  }
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



template<typename T>
struct Property
{
  T mOwnValue;
  T* mRefBinding = &mOwnValue;
  std::function<T()> mGetter;
  std::function<void(const T&)> mSetter;
  std::function<void(const T& oldVal, const T& newVal)> mOnChange;

  // copy
  Property(const Property<T>& rhs) :
    mOwnValue(rhs.mOwnValue),
    mRefBinding(rhs.mRefBinding == &rhs.mOwnValue ? &mOwnValue : rhs.mRefBinding),
    mGetter(rhs.mGetter),
    mSetter(rhs.mSetter),
    mOnChange(rhs.mOnChange)
  {
  }

  Property(Property<T>&& rhs) :
    mOwnValue(rhs.mOwnValue),
    mRefBinding(rhs.mRefBinding == &rhs.mOwnValue ? &mOwnValue : rhs.mRefBinding),
    mGetter(std::move(rhs.mGetter)),
    mSetter(std::move(rhs.mSetter)),
    mOnChange(std::move(rhs.mOnChange))
  {
  }

  Property<T>& operator =(const Property<T>&) = delete;
  Property<T>& operator =(Property<T>&&) = delete;
  
  Property(std::function<T()> getter, std::function<void(const T&)> setter) :
    mGetter(getter),
    mSetter(setter)
  {}
  Property(T& binding) :
    mRefBinding(&binding)
  {
//    char x[100];
//    sprintf(x, "ref binding to %p", (&binding));
//    Serial.println(x);
  }
  Property(std::function<void(const T& oldVal, const T& newVal)> onChange) :
    mOnChange(onChange)
  {
  }
  Property() :
    mRefBinding(&mOwnValue)
  {
  }
  T GetValue() const
  {
    if (!!mGetter) {
      return mGetter();
    }
    CCASSERT(!!mRefBinding);
    return *mRefBinding;
  }
  void SetValue(const T& val)
  {
    T oldVal = GetValue();
    if (mRefBinding) {
      *mRefBinding = val;
    }
    if (mSetter) {
      mSetter(val);
    }
    if (oldVal != val && mOnChange) {
      mOnChange(oldVal, val);
    }
  }
};

template<typename Tprop, typename Tval>
Property<Tprop> MakePropertyByCasting(Tval& x) {
  return Property<Tprop>(
    [&]() { return static_cast<Tprop>(x); },
    [&](const Tprop& val) { x = static_cast<Tval>(val);
  });
}


struct IList {
  virtual int List_GetItemCount() const = 0;
  virtual String List_GetItemCaption(int i) const = 0;
};




template<typename T>
struct EnumItemInfo
{
  T mValue;
  const char *mName;
};

// THIS REQUIRES THAT YOUR VALUES ARE SEQUENTIAL AND THE SAME AS LIST INDICES. Runtime checks performed.
// otherwise i would need to add a bunch of inefficient 
template<typename T>
struct EnumInfo : IList {
  const size_t mItemCount;
  const EnumItemInfo<T>* mItems;
  
  template<size_t N>
  EnumInfo(const EnumItemInfo<T>(&enumItems)[N]) :
    mItemCount(N),
    mItems(enumItems)
  {
    for (size_t i = 0; i < N; ++ i) {
      if (static_cast<size_t>(mItems[i].mValue) != i) {
        Die("Enum value did not correspond to list index");
      }
    }
  }

  int List_GetItemCount() const { return mItemCount; }
  
  String List_GetItemCaption(int n) const
  {
    CCASSERT(n >= 0 && n < (int)mItemCount);
    return String(mItems[n].mName);
  }
  
  const EnumItemInfo<T>* GetItem(size_t n) {
    CCASSERT(n >0 && n <mItemCount);
    return &mItems[n];
  }

  const char *GetValueString(const T& val) const {
    return mItems[static_cast<size_t>(val)].mName;
  }

  T AddRotate(const T& val, int n) const {
    int y = static_cast<int>(val) + n;
    return static_cast<T>(RotateIntoRange(y, (int)mItemCount));
  }

  int ToInt(const T& val) const { return static_cast<int>(val); }
  T ToValue(const int& val) const { return static_cast<T>(val); }
};




#endif
