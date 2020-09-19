#pragma once

#include <clarinoid/basic/Basic.hpp>

template<int TStep>
class CCEncoder : UpdateObjectT<ProfileObjectType::Encoder>
{
  Encoder mEnc;

  // valid during this loop iteration
  bool mIsDirty;
  int mRawValue = 0;
  float mFloatValue;

  int mIntValue = 0; // divided by TStep
  int mIntDelta = 0;

public:
  CCEncoder(uint8_t pin1, uint8_t pin2) :
    mEnc(pin1, pin2),
    mIsDirty(false)
  {
    mEnc.write(0);
  }

  bool IsDirty() const { return mIsDirty; }
  int GetIntValue() const { return mIntValue; }
  int GetFloatValue() const { return mFloatValue; }
  int GetIntDelta() const { return mIntDelta; }

  virtual void setup()
  {
    // pinMode() is set by encoder lib
  }

  virtual void loop()
  {
    int32_t r = mEnc.read();
    mFloatValue = (float)r / TStep;
    int newIntValue = idiv_round((int)r, TStep);
    mIntDelta = newIntValue - mIntValue;
    mIntValue = newIntValue;
    mIsDirty = r != mRawValue;
    if (mIsDirty) {
      //Serial.println(String("encoder dirty; intval=") + mIntValue + " delta=" + mIntDelta );
    }
    mRawValue = r;
  }
};

