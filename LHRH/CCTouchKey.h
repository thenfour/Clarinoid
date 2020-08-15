#ifndef CCTOUCHKEY_H
#define CCTOUCHKEY_H

#include <touchablePin.h>
#include "Shared_CCUtil.h"

// because we're optimizing the untouched state so much, we increase the max factor used to detect touches.
const float CCEWI_TOUCHABLE_PIN_MAX_FACTOR = 1.8;

// touchRead() is very slow because it waits to detect the full charge.
// Fortunately touchablePin has been created which short circuits if "touched" is satisfied.
// the drawback is that you must initialize it when you know the key is untouched.
// so we periodically call the original touchread to detect if the value
// is less than we've seen before. if so, we suspect it's untouched.
const int MaxTouchKeys = 30;

class CCTouchKeyCalibrator : IUpdateObject
{
  uint16_t mMinValues[MaxTouchKeys];
  touchablePin* mTouchablePins[MaxTouchKeys];

  CCThrottler mThrottle;
  uint8_t mIndex = 0;

public:
  uint8_t mPinCount = 0;

  CCTouchKeyCalibrator() : 
    mThrottle(20)
  {
  }

  // returns the index of this pin. it can be used to stagger reads within frames.
  int Add(touchablePin* p) {
    mTouchablePins[mPinCount] = p;
    mMinValues[mPinCount] = touchRead(p->pinNumber);
    int ret = mPinCount;
    mPinCount ++;
    return ret;
  }

  virtual void setup() { }
  virtual void loop() {
    if (!mThrottle.IsReady()) {
      return;
    }
    ;
    int n = touchRead(mTouchablePins[mIndex]->pinNumber);
    if (n < mMinValues[mIndex]) {
      mMinValues[mIndex] = n;
      mTouchablePins[mIndex]->initUntouched();
      //Serial.println(String("Updating pin ") + mTouchablePins[mIndex]->pinNumber + " with minvalue " + n);
    }

    mIndex = (mIndex + 1) % mPinCount;
  }
};

CCTouchKeyCalibrator gTouchKeyCalibrator;

const int KeysToSamplePerFrame = 2;

class CCTouchKey : IUpdateObject
{
  touchablePin mTouchablePin;
  uint8_t mPin;
  bool mIsTouched;
  bool mIsDirty;
  int mStaggerGroup;
  int mFrameNumber = 0;
public:
  explicit CCTouchKey(uint8_t pin) :
    mTouchablePin(pin, CCEWI_TOUCHABLE_PIN_MAX_FACTOR),
    mPin(pin),
    mIsTouched(false),
    mIsDirty(false)
  {
    mStaggerGroup = gTouchKeyCalibrator.Add(&mTouchablePin) / KeysToSamplePerFrame;
  }

  bool IsPressed() const { return mIsTouched; }
  bool IsDirty() const { return mIsDirty; }

  virtual void setup()
  {
    // no call to pinMode needed.
  }

  virtual void loop()
  {
    int totalGroups = gTouchKeyCalibrator.mPinCount / KeysToSamplePerFrame;
    if (mFrameNumber % totalGroups == mStaggerGroup) {
      bool newState = mTouchablePin.isTouched();
      mIsDirty = newState != mIsTouched;
      mIsTouched = newState;
    }
    mFrameNumber ++;
  }

};

#endif
