
#pragma once

#include <clarinoid/basic/Basic.hpp>

// because we're optimizing the untouched state so much, we increase the max factor used to detect touches.
const float CCEWI_TOUCHABLE_PIN_MAX_FACTOR = 1.5;

// touchRead() is very slow because it waits to detect the full charge.
// Fortunately touchablePin has been created which short circuits if "touched" is satisfied.
// the drawback is that you must initialize it when you know the key is untouched.
// so we periodically call the original touchread to detect if the value
// is less than we've seen before. if so, we suspect it's untouched.
const int MaxTouchKeys = 15;
const int TOUCH_KEY_CALIB_UNTOUCHED_MOVING_AVG_SAMPLES = 5; // how many samples do we average together. helps avoid transients, but adds a teeny tiny delay
const int TOUCH_KEY_CALIB_THROTTLE_MS = 30; // how often to check for recalibration.

struct TouchableKeyInfo {
  TouchableKeyInfo(uint8_t pin, int keyDescIndex) :
    mTouchablePin(pin, CCEWI_TOUCHABLE_PIN_MAX_FACTOR),
    mKeyDescIndex(keyDescIndex),
    mDesc(gKeyDesc[keyDescIndex])
  {
  }
  bool mDebug = false;
  uint32_t mTouchReadMicros = 0;
  uint32_t mTouchReadValue = 0;
  float mMinValue; // we should set this as soon as we have enough moving avg samples.
  SimpleMovingAverage<TOUCH_KEY_CALIB_UNTOUCHED_MOVING_AVG_SAMPLES> mRunningValues;
  touchablePin mTouchablePin;
  int mKeyDescIndex;
  KeyDesc& mDesc;
  float mMaxFactor = CCEWI_TOUCHABLE_PIN_MAX_FACTOR;

  void Reset() {
    //int m1 = micros();
    ///*int n = */mTouchablePin.touchRead();
    //int m2 = micros();
    //mMinValue = 0; // will be re-set later.
    mRunningValues.Clear();
    auto pinNumber = mTouchablePin.pinNumber; // save for re-construction
    mTouchablePin.~touchablePin();
    new (&mTouchablePin) touchablePin(pinNumber, mMaxFactor);
    //mTouchablePin.initUntouched();
    // auto pinNumber = mTouchablePin.pinNumber; // save for re-construction
    // mTouchablePin.~touchablePin();
    // new (&mTouchablePin) touchablePin(pinNumber, mMaxFactor);
  }

  void SetMaxFactor(float newMaxFactor) {
    mMaxFactor = newMaxFactor;
    Reset();
    // // for some reason touchablePin doesn't have a method to change this. so we have to re-construct the object.
    // auto pinNumber = mTouchablePin.pinNumber; // save for re-construction
    // mTouchablePin.~touchablePin();
    // new (&mTouchablePin) touchablePin(pinNumber, newMaxFactor);
  }
};

class CCTouchKeyCalibrator : UpdateObjectT<ProfileObjectType::TouchKeyCalibration>
{
  CCThrottlerT<TOUCH_KEY_CALIB_THROTTLE_MS> mThrottle;
  uint8_t mIndex = 0;

public:
  uint8_t mKeyCount = 0;
  TouchableKeyInfo* mKeys[MaxTouchKeys];

  // returns the index of this pin. it can be used to stagger reads within frames.
  int Add(TouchableKeyInfo* ki) {
    mKeys[mKeyCount] = ki;
    int ret = mKeyCount ++;
    ki->Reset();
    return ret;
  }

  void ResetAllKeys() {
    //Serial.println("resetting all keys");
    for (int i = 0; i < mKeyCount; ++ i) {
      mKeys[i]->Reset();
    }
  }

  void SetMaxFactorOfAllKeys(float newMaxFactor) {
    for (int i = 0; i < mKeyCount; ++ i) {
      mKeys[i]->SetMaxFactor(newMaxFactor);
    }
  }

  virtual void loop()
  {
   if (!mThrottle.IsReady()) {
     return;
   }

   int m1 = micros();
   int n = mKeys[mIndex]->mTouchablePin.touchRead();
   int m2 = micros();
   auto& k = *mKeys[mIndex];
   k.mRunningValues.Update((float)n);

   if (m2 > m1) { // avoid overflow
     k.mTouchReadMicros = m2 - m1;
   }
   k.mTouchReadValue = n;

   if (k.mRunningValues.GetTotalSamplesTaken() == TOUCH_KEY_CALIB_UNTOUCHED_MOVING_AVG_SAMPLES) {
     // after we have a complete smoothed value buffer, initialize the min value.
     k.mMinValue = k.mRunningValues.GetValue();
   } else if (k.mRunningValues.GetTotalSamplesTaken() > TOUCH_KEY_CALIB_UNTOUCHED_MOVING_AVG_SAMPLES) {
     float av = k.mRunningValues.GetValue();
     if (av < (k.mMinValue / CCEWI_TOUCHABLE_PIN_MAX_FACTOR)) { // if significantly lower.
       k.mMinValue = av;
       k.mTouchablePin.initUntouched();      
     }
   }

   mIndex = (mIndex + 1) % mKeyCount;
  }
};

CCTouchKeyCalibrator gTouchKeyCalibrator;

class CCTouchKey* gTouchKeyLookup[SizeofStaticArray(gKeyDesc)] = {nullptr};

class CCTouchKey : UpdateObjectT<ProfileObjectType::TouchKey>
{
  bool mIsTouched = false;
  bool mIsDirty = false;
  TouchableKeyInfo ki;
  //int mStaggerGroup;
  CCThrottlerT<5> mDebugThrottle;

public:

  explicit CCTouchKey(uint8_t pin, int keyDescIndex, bool debug = false) :
    ki(pin, keyDescIndex)
  {
    ki.mDebug = debug;
    gTouchKeyLookup[keyDescIndex] = this;
    /*int n = */gTouchKeyCalibrator.Add(&ki);
    //mStaggerGroup = n;
  }

  bool IsPressed() const { return mIsTouched; }
  bool IsDirty() const { return mIsDirty; }

  virtual void setup()
  {
    // no call to pinMode needed.
  }

  virtual void loop()
  {
    if (ki.mKeyDescIndex == gFocusedKeyIndex && mDebugThrottle.IsReady()) {
      //Serial.println(String("sampling debug...") + millis());
      uint32_t m = micros();
      int n = ki.mTouchablePin.touchRead();
      uint32_t m2 = micros();
      if (m < m2) {
        gFocusedKeyData = ki.mKeyDescIndex;
        gFocusedTouchReadMicros = m2 - m;
        gFocusedTouchReadValue = n;
        gFocusedTouchReadUntouchedMicros = ki.mTouchablePin.untouchedTime;
        gFocusedTouchReadThresholdMicros = ki.mTouchablePin.untouchedTime * CCEWI_TOUCHABLE_PIN_MAX_FACTOR;
      }
    }

    uint8_t groupBit = 1 << (gFrameNumber % 6); // this is the stagger group bit this frame corresponds to (B100, B010, or B001)
    if (!(ki.mDesc.mStaggerGroup & groupBit)) {
      return;
    }
      
    bool newState = ki.mTouchablePin.isTouched();
    mIsDirty = newState != mIsTouched;
    mIsTouched = newState;
  }

};

