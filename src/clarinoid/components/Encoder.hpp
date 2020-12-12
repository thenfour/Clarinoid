#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{


// struct NullEncoder :
//   public IEncoder
// {
//   virtual EncoderState CurrentValue() const
//   {
//     EncoderState ret;
//     ret.CommonValue = 0;
//     ret.FloatValue = 0;
//     ret.RawValue = 0;
//     return ret;
//   }
// };

// TStep is the encoder's detent step.
template<int TStep, uint8_t Tpin1, uint8_t Tpin2>
struct CCEncoder :
  public IEncoder
{
  Encoder mEnc;
  float mState;
  int32_t mRawValue;

public:
  CCEncoder() :
    mEnc(Tpin1, Tpin2)
  {
    mEnc.write(0);
  }

  virtual float CurrentValue() const
  {
    return mState;
  }

  int32_t RawValue() const { return mRawValue; }

  void Update()
  {
    mRawValue = mEnc.read();

    //mState.RawValue = r;
    mState = (float)mRawValue / TStep;
    //mState.CommonValue = idiv_round((int)r, TStep);
  }
};

} // namespace clarinoid
