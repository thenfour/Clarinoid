

#pragma once

#include "Profiler.hpp"

//////////////////////////////////////////////////////////////////////
static constexpr size_t MAX_UPDATE_OBJECTS = 30;

// allows globally-created objects to sign up for updates
size_t gUpdateObjectCount = 0;
struct UpdateObject* gUpdateObjects[MAX_UPDATE_OBJECTS];

struct UpdateObject
{
  ProfileObjectType mProfileObjectType;
  UpdateObject(ProfileObjectType profileObjectType) :
    mProfileObjectType(profileObjectType)
  {
    CCASSERT(gUpdateObjectCount < MAX_UPDATE_OBJECTS);
    gUpdateObjects[gUpdateObjectCount] = this;
    gUpdateObjectCount ++;
  }
  virtual void setup() {}
  virtual void loop() {}
};

// allows updateobjects to be profiled.
struct LoopTimer : ProfileTimer
{
  LoopTimer(UpdateObject* p) :
    ProfileTimer(p->mProfileObjectType, [](ProfileTiming& p) { return &p.mLoopMillis ;})
  {}
};

struct UpdateTimer : ProfileTimer
{
  UpdateTimer(UpdateObject* p) :
    ProfileTimer(p->mProfileObjectType, [](ProfileTiming& p) { return &p.mUpdateMillis ;})
  {}
};

struct RenderTimer : ProfileTimer
{
  RenderTimer(UpdateObject* p) :
    ProfileTimer(p->mProfileObjectType, [](ProfileTiming& p) { return &p.mRenderMillis ;})
  {}
};


template<ProfileObjectType TprofileObjectType>
struct UpdateObjectT : public UpdateObject
{
  UpdateObjectT() : UpdateObject(TprofileObjectType) {}  
};

inline void SetupUpdateObjects() {
  for (uint8_t i = 0; i < gUpdateObjectCount; ++ i)
  {
    gUpdateObjects[i]->setup();
  }
}

inline void UpdateUpdateObjects() {
  for (uint8_t i = 0; i < gUpdateObjectCount; ++ i)
  {
    LoopTimer lt(gUpdateObjects[i]);
    gUpdateObjects[i]->loop();
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
class ActivityLED : UpdateObjectT<ProfileObjectType::LED>
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
class AsymmetricActivityLED : UpdateObjectT<ProfileObjectType::LED>
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
class TransientActivityLED : UpdateObjectT<ProfileObjectType::LED>
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
class TransientEventLED : UpdateObjectT<ProfileObjectType::LED>
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


