#ifndef CCTOUCHKEY_H
#define CCTOUCHKEY_H

#include "CCUtil.h"

// typical idle is 600-900, depends on the surface area.
// for small washers, edge touch = about 800, normal = 2000, absolute max 3500 / thresh is around 1000 to avoid noise
// for big washers, edge touch = about 1300, normal = 2500-3000, absolute max 6000
// in order to save payload space

//uint16_t mTouchKeyGlobalMax = 0;
//uint16_t mTouchKeyGlobalMin = ;
//on a scale of 0-100, what is the theshold for "touched"
//uint8_t mTouchKeyThreshold = 17;

class CCTouchKey : IUpdateObject
{
  uint8_t mPin;
  //uint16_t mLocalMax;
  //uint16_t mLocalMin;
  uint32_t mTouchedThreshold;
  bool mIsTouched;
  bool mIsDirty;
public:
  explicit CCTouchKey(uint8_t pin, uint32_t touchedThreshold) :
    mPin(pin),
    //mUseLocal(false),
    mTouchedThreshold(touchedThreshold),
    //mLocalMax(mTouchKeyGlobalMax),
    //mLocalMin(mTouchKeyGlobalMin),
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
    bool newState = raw > mTouchedThreshold;
    mIsDirty = newState != mIsTouched;
    mIsTouched = newState;
  }
//    
//    if (raw < mLocalMin) {
//      mLocalMin = raw;
//    }
//    if (raw > mLocalMax) {
//      mLocalMax = raw;
//    }
//    if (raw < mTouchKeyGlobalMin) {
//      mTouchKeyGlobalMin = raw;
//    }
//    if (raw > mTouchKeyGlobalMax) {
//      mTouchKeyGlobalMax = raw;
//    }
//
//    if (mUseLocal) {
//      raw -= mLocalMin;
//      raw *= 100;
//      raw /= mLocalMax - mLocalMin;
//    } else {
//      raw -= mTouchKeyGlobalMin;
//      raw *= 100;
//      raw /= mTouchKeyGlobalMax - mTouchKeyGlobalMin;
//    }
//
//    // raw is now 0-100
//    if (mIsTouched) {
//      if (raw < mTouchKeyThreshold) {
//        mIsTouched = false;
//        mIsDirty = true;
//      }
//    } else {
//      if (raw > mTouchKeyThreshold) {
//        mIsTouched = true;
//        mIsDirty = true;
//        mUseLocal = true;// now we have local range so it's reliable.
//      }
//    }
//  }
};

#endif
