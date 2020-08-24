#ifndef CCTOUCHKEY_H
#define CCTOUCHKEY_H

#include <touchablePin.h>
#include "Shared_CCUtil.h"

// because we're optimizing the untouched state so much, we increase the max factor used to detect touches.
const float CCEWI_TOUCHABLE_PIN_MAX_FACTOR = 1.3;


// touchRead() is very slow because it waits to detect the full charge.
// Fortunately touchablePin has been created which short circuits if "touched" is satisfied.
// the drawback is that you must initialize it when you know the key is untouched.
// so we periodically call the original touchread to detect if the value
// is less than we've seen before. if so, we suspect it's untouched.
const int MaxTouchKeys = 30;
const int TOUCH_KEY_CALIB_UNTOUCHED_MOVING_AVG_SAMPLES = 15; // how many samples do we average together
const int TOUCH_KEY_CALIB_THROTTLE_MS = 20;

struct TouchableKeyInfo {
  TouchableKeyInfo(uint8_t pin) :
    mTouchablePin(pin, CCEWI_TOUCHABLE_PIN_MAX_FACTOR)
  {
  }
  bool mDebug = false;
  uint32_t mTouchReadMicros = 0;
  uint32_t mTouchReadValue = 0;
  float mMinValue; // we should set this as soon as we have enough moving avg samples.
  SimpleMovingAverage<TOUCH_KEY_CALIB_UNTOUCHED_MOVING_AVG_SAMPLES> mRunningValues;
  touchablePin mTouchablePin;
};

class CCTouchKeyCalibrator : IUpdateObject
{
  CCThrottlerT<TOUCH_KEY_CALIB_THROTTLE_MS> mThrottle;
  uint8_t mIndex = 0;

public:
  uint8_t mKeyCount = 0;
  TouchableKeyInfo* mKeys[MaxTouchKeys];

  // returns the index of this pin. it can be used to stagger reads within frames.
  int Add(TouchableKeyInfo* ki) {
    mKeys[mKeyCount] = ki;    
    return mKeyCount ++;
  }

  virtual void loop() {
    if (!mThrottle.IsReady()) {
      return;
    }

    int m1 = micros();
    int n = touchRead(mKeys[mIndex]->mTouchablePin.pinNumber);
    int m2 = micros();
    auto& k = *mKeys[mIndex];
    k.mRunningValues.Update((float)n);

    if (m2 > m1) {
      k.mTouchReadMicros = m2 - m1;
    }
    k.mTouchReadValue = n;

    if (k.mRunningValues.GetSampleCount() == TOUCH_KEY_CALIB_UNTOUCHED_MOVING_AVG_SAMPLES) {
      k.mMinValue = k.mRunningValues.GetValue();
    } else if (k.mRunningValues.GetSampleCount() > TOUCH_KEY_CALIB_UNTOUCHED_MOVING_AVG_SAMPLES) {
      float av = k.mRunningValues.GetValue();
      if (av < k.mMinValue) {
        k.mMinValue = av;
        k.mTouchablePin.initUntouched();      
      }
    }

    mIndex = (mIndex + 1) % mKeyCount;
  }
};

CCTouchKeyCalibrator gTouchKeyCalibrator;

class CCTouchKey : IUpdateObject
{
  bool mIsTouched = false;
  bool mIsDirty = false;
  TouchableKeyInfo ki;
  int mStaggerGroup;

public:

  explicit CCTouchKey(uint8_t pin, bool debug = false) :
    ki(pin)
  {
    ki.mDebug = debug;
    //gTouchKeyCalibrator.Add(&ki);
    mStaggerGroup = gTouchKeyCalibrator.Add(&ki);// / KeysToSamplePerFrame;
  }

  bool IsPressed() const { return mIsTouched; }
  bool IsDirty() const { return mIsDirty; }
  uint32_t GetTouchReadMicros() const { return ki.mTouchReadMicros; }
  uint32_t GetTouchReadMaxMicros() const { return ki.mTouchReadMicros * CCEWI_TOUCHABLE_PIN_MAX_FACTOR; }

  virtual void setup()
  {
    // no call to pinMode needed.
  }

  virtual void loop()
  {
    if ((gFrameNumber % gTouchKeyCalibrator.mKeyCount) != mStaggerGroup)
      return;
      
    bool newState = ki.mTouchablePin.isTouched();
    mIsDirty = newState != mIsTouched;
    mIsTouched = newState;

    if (ki.mDebug) {
      uint32_t m = micros();
      int n = touchRead(ki.mTouchablePin.pinNumber);
      uint32_t m2 = micros();
      if (m < m2) {
        CCPlot(m2 - m);
        CCPlot(ki.mTouchablePin.untouchedTime);
        CCPlot(ki.mTouchablePin.untouchedTime * CCEWI_TOUCHABLE_PIN_MAX_FACTOR);
      }
    }
  }

};

#endif
