
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

struct HoneywellABPI2C
{
  static constexpr size_t PressureBitsResolution = 14;
  static constexpr size_t TempBitsResolution = 11;

  enum class Status : uint8_t {
    Ok = 0, // this is really the only success condition here.
    CommandMode = 1,
    StaleData = 2,
    Diagnostic = 3
  };
  
  explicit HoneywellABPI2C(TwoWire& wire, uint8_t address = 0x28) :
    mWire(wire),
    mAddress(address)
  {
    wire.begin();
  }
  
  void Update() {
    mWire.flush();
    mWire.requestFrom(mAddress, (uint8_t)4);
    uint8_t b0, b1, b2, b3;
    b0 = mWire.read();
    b1 = mWire.read();
    b2 = mWire.read();
    b3 = mWire.read();

    // b0: 2 bits status | 6 bits pressure
    // b1: 8 bits pressure
    // b2: 8 bits temperature data
    // b3: 3 bits temperature data | 5 bits 0.
    mStatus = (Status)(b0 >> 6);
    if (mStatus != Status::Ok) {
      return; // under any error condition don't use the data! even under StaleData, there's no point using the data.
    }
    uint16_t p = ((((uint16_t)b0) << 8) | b1) & ((1<<PressureBitsResolution)-1);
    // convert this 14-bit value to a 0-1 float val.
    mValue = (float)p / ((1<<PressureBitsResolution)-1);

    uint16_t t = ((((uint16_t)b2) << 3) |(b3 >> 5));
    mTemperatureC = (((float)t * 200) / 2047) - 50;
  }

  float GetPressure01() const {return mValue;}
  float GetTemperatureC() const {return mTemperatureC;}

private:
  TwoWire& mWire;
  uint8_t mAddress;
  Status mStatus;
  float mValue;
  float mTemperatureC;
};


struct CCHoneywellAPB :
  IAnalogControl
{
  HoneywellABPI2C mAbp;
  float value;

  explicit CCHoneywellAPB(TwoWire& wire, uint8_t address = 0x28) :
    mAbp(wire, address)
  {
    wire.setClock(400000); // use high speed mode. default speed = 100k
  }

  void Update()
  {
    mAbp.Update();
  }

  virtual float CurrentValue01() const
  {
    return mAbp.GetPressure01();
  }
};

} // namespace clarinoid
