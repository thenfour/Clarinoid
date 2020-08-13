
#ifndef CCUTIL_H
#define CCUTIL_H

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

#endif
