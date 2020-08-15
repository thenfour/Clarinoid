
#ifndef CCAIRPRESSURE_H
#define CCAIRPRESSURE_H

#define NOISE_FLOOR_01 (0.03f)

//#define BREATH_MIN 62
//#define BREATH_MAX 400

// attempts to estimate an analog read, for illuminating LEDs in the LHRH modules (before more precise tuning is done on the values)
class Analog01Estimator
{
public:
  uint16_t mMin = 0xffff;
  uint16_t mMax = 0;
  float mValue01 = 0;
  
  void Update(uint16_t a)
  {
    if (a < mMin) {
      mMin = a;
    }
    if (a > mMax) {
      mMax = a;
    }
    a -= mMin;
    float ret = ((float)a) / (mMax - mMin);
    if (ret < NOISE_FLOOR_01) {
      ret = 0;
    }
    mValue01 = ret;
  }
};

class CCBreathSensor : IUpdateObject
{
  uint8_t mPin;
  uint16_t mRawValue = 0;
  Analog01Estimator mValue01Estimate;
  bool mLogging;
  
public:
  explicit CCBreathSensor(uint8_t pin, bool logging = false) :
    mPin(pin),
    mLogging(logging)
  {
  }

  virtual void setup()
  {
  }
  
  virtual void loop()
  {
    uint16_t a = analogRead(mPin);
    mRawValue = a;
    if (mLogging) {
      Serial.println(a);
    }
    mValue01Estimate.Update(a);
  }

  uint16_t GetRawValue() const { return mRawValue; }

  // return 0-1
  float Value01Estimate() const {
    return mValue01Estimate.mValue01;
  }
};

using CCBiteSensor = CCBreathSensor;
using CCPitchStripSensor = CCBreathSensor;



#endif
