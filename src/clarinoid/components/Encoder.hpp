#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{


struct NullEncoder :
  public IEncoder
{
  virtual EncoderState CurrentValue() const
  {
    EncoderState ret;
    ret.CommonValue = 0;
    ret.FloatValue = 0;
    ret.RawValue = 0;
    return ret;
  }
};

// TStep is the encoder's detent step.
template<int TStep, uint8_t Tpin1, uint8_t Tpin2>
struct CCEncoder :
  public IEncoder
{
  Encoder mEnc;
  EncoderState mState;

public:
  CCEncoder() :
    mEnc(Tpin1, Tpin2)
  {
    mEnc.write(0);
  }

  virtual EncoderState CurrentValue() const
  {
    return mState;
  }

  void Update()
  {
    int32_t r = mEnc.read();
    mState.RawValue = r;
    mState.FloatValue = (float)r / TStep;
    mState.CommonValue = idiv_round((int)r, TStep);
  }
};

} // namespace clarinoid
