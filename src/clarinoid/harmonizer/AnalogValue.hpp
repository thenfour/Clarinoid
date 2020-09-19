
#pragma once

#include <clarinoid/basic/Basic.hpp>

static const size_t ANALOG_RESOLUTION_BITS = 12;


template<size_t BitsResolution = ANALOG_RESOLUTION_BITS>
struct AnalogValue01
{
  using this_t = AnalogValue01<BitsResolution>;
  void SetFloat(float v)
  {
    mFloatVal = v;
    if (v >= 1.0f) {
      mIntVal = (1 << BitsResolution) - 1;
    }
    else if (v <= 0) {
      mIntVal = 0;
    }
    else {
      mIntVal = (uint16_t)(v * (1 << BitsResolution));
    }
  }
  void Deserialize12Bit(uint16_t v) {
    mIntVal = v;
    mFloatVal = (float)v / (1 << BitsResolution);
  }
  uint16_t Serialize12Bit() const { return mIntVal; }
  float GetFloatVal() const { return mFloatVal; }
  bool operator ==(const this_t& rhs) const {
    return mIntVal == rhs.mIntVal;
  }
  //bool operator ==(float rhs) const {
  //  return mFloatVal == rhs;
  //}
  //bool operator ==(int rhs) const {
  //  return mIntVal == rhs;
  //}
  bool operator !=(const this_t& rhs) const {
    return !(*this == rhs);
  }
  bool operator !=(float rhs) const {
    return mFloatVal != rhs;
  }
  //bool operator !=(int rhs) const {
  //  return mIntVal != rhs;
  //}
  //this_t& operator =(int v) { SetInt(v); return *this; }
  this_t& operator =(float v) { SetFloat(v); return *this; }
private:
  uint16_t mIntVal = 0;
  float mFloatVal = 0;
};

