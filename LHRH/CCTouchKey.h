#ifndef CCTOUCHKEY_H
#define CCTOUCHKEY_H

#include "CCUtil.h"

uint32_t mTouchKeyGlobalMax = 2000;
uint32_t mTouchKeyGlobalMin = 1000;
//on a scale of 0-100, what is the theshold for "touched"
uint8_t mTouchKeyThreshold = 17;

class CCTouchKey : IUpdateObject
{
  uint8_t mPin;
  bool mUseLocal;
  uint32_t mLocalMax;
  uint32_t mLocalMin;
  bool mIsTouched;
  bool mIsDirty;
public:
  explicit CCTouchKey(uint8_t pin) :
    mPin(pin),
    mUseLocal(false),
    mLocalMax(mTouchKeyGlobalMax),
    mLocalMin(mTouchKeyGlobalMin),
    mIsTouched(false),
    mIsDirty(false)
  {
  }

  bool IsPressed() const { return mIsTouched; }
  bool IsDirty() const { return mIsDirty; }

  virtual void setup()
  {
    // no call to pinMode needed.
  }

  virtual void loop()
  {
    uint32_t raw = touchRead(mPin);
    if (raw < mLocalMin) {
      mLocalMin = raw;
    }
    if (raw > mLocalMax) {
      mLocalMax = raw;
    }
    if (raw < mTouchKeyGlobalMin) {
      mTouchKeyGlobalMin = raw;
    }
    if (raw > mTouchKeyGlobalMax) {
      mTouchKeyGlobalMax = raw;
    }

    if (mUseLocal) {
      raw -= mLocalMin;
      raw *= 100;
      raw /= mLocalMax - mLocalMin;
    } else {
      raw -= mTouchKeyGlobalMin;
      raw *= 100;
      raw /= mTouchKeyGlobalMax - mTouchKeyGlobalMin;
    }

    // raw is now 0-100
    if (mIsTouched) {
      if (raw < mTouchKeyThreshold) {
        mIsTouched = false;
        mIsDirty = true;
      }
    } else {
      if (raw > mTouchKeyThreshold) {
        mIsTouched = true;
        mIsDirty = true;
        mUseLocal = true;// now we have local range so it's reliable.
      }
    }
  }
};

#endif
