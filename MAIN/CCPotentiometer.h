
#ifndef CCPOTENTIOMETER_H
#define CCPOTENTIOMETER_H

#define VOLUMEPOT_MIN_READING 2
#define VOLUMEPOT_MAX_READING 1022
#define VOLUMEPOT_DIRTY_THRESH 0.02

class CCVolumePot : IUpdateObject
{
  uint8_t mPin;
  float mValueWhenDirty = 0;
  float mValue01;
  bool mIsDirty;
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

#endif
