
#ifndef CCUTIL_H
#define CCUTIL_H

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
  virtual void setup() = 0;
  virtual void loop() = 0;
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
class CCThrottler
{
  uint32_t mPeriodMS;
  uint32_t mPeriodStartMS;
public:
  CCThrottler(uint32_t periodMS) :
    mPeriodMS(periodMS)
  {
    mPeriodStartMS = millis();
  }

  bool IsReady() {
    auto m = millis();
    if (m - mPeriodStartMS < mPeriodMS) {
      return false;
    }
    mPeriodStartMS = m; // note that overshoot will not be subtracted! so periods will often be longer than requested.
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
      }
    }
  }
};

//////////////////////////////////////////////////////////////////////
// convert values to LED indication colorant values
inline uint8_t col(bool b) { return b ? 8 : 0; }
inline uint8_t col(bool b, uint8_t ledmin, uint8_t ledmax)
{
  return b ? ledmax : ledmin;
}
inline uint8_t col(float f01) { return (uint8_t)(f01 * 255); }


//////////////////////////////////////////////////////////////////////

// random utility function
template<typename T, size_t N>
size_t SizeofStaticArray(const T(&x)[N])
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


#endif
