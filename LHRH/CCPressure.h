
#ifndef CCAIRPRESSURE_H
#define CCAIRPRESSURE_H

#define NOISE_FLOOR_01 (0.03f)

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

class CCBreathSensor : UpdateObjectT<ProfileObjectType::BreathSensor>
{
  uint8_t mPin;
  uint16_t mRawValue = 0;
  Analog01Estimator mValue01Estimate;
  
public:
  explicit CCBreathSensor(uint8_t pin) :
    mPin(pin)
  {
  }

  virtual void setup()
  {
  }
  
  virtual void loop()
  {
    uint16_t a = analogRead(mPin);
    mRawValue = a;
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
