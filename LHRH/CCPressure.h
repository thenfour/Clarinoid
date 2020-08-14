
#ifndef CCAIRPRESSURE_H
#define CCAIRPRESSURE_H

#define PRESSURE_NOISE_FLOOR_01 (0.05f)

class CCPressure : IUpdateObject
{
  uint8_t mPin;
  uint32_t mMin;
  uint32_t mMax;
  float mValue01;
public:
  explicit CCPressure(uint8_t pin) :
    mPin(pin),
    mMin(0xffffffff),
    mMax(200)
  {
  }

  virtual void setup()
  {
  }
  
  virtual void loop()
  {
    uint32_t a = analogRead(mPin);
    if (a < mMin) {
      mMin = a;
    }
    if (a > mMax) {
      mMax = a;
    }
    a -= mMin;
    float ret = ((float)a) / (mMax - mMin);
    if (ret < PRESSURE_NOISE_FLOOR_01) {
      ret = 0;
    }
    mValue01 = ret;
  }

  uint32_t GetMin() const { return mMin; }
  uint32_t GetMax() const { return mMax; }

  // return 0-1
  float Value01() const {
    return mValue01;
  }
};

#endif
