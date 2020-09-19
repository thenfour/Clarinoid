
#pragma once

#define VOLUMEPOT_MIN_READING 2
#define VOLUMEPOT_MAX_READING 1022
#define VOLUMEPOT_DIRTY_THRESH 0.02

#include <clarinoid/basic/Basic.hpp>

class CCVolumePot : UpdateObjectT<ProfileObjectType::Pot>
{
  uint8_t mPin;
  float mValueWhenDirty = 0;
  float mValue01;
  bool mIsDirty;
  CCThrottlerT<10> mThrottle;
public:
  explicit CCVolumePot(uint8_t pin) :
    mPin(pin),
    mValue01(0),
    mIsDirty(false)
  {
  }

  virtual void setup()
  {
  }
  
  virtual void loop()
  {
    if (!mThrottle.IsReady())
      return;
    int32_t a = analogRead(mPin);
    a -= VOLUMEPOT_MIN_READING;
    float ret = ((float)a) / (VOLUMEPOT_MAX_READING - VOLUMEPOT_MIN_READING);
    ret = constrain(ret, 0.0f, 1.0f);
    mIsDirty = abs(ret - mValueWhenDirty) > VOLUMEPOT_DIRTY_THRESH;
    if (mIsDirty) {
      mValueWhenDirty = ret;
    }
    mValue01 = ret;
  }

  bool IsDirty() const { return mIsDirty; }

  // return 0-1
  float GetValue01() const {
    return mValue01;
  }
};
