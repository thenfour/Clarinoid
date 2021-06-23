#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

// TStep is the encoder's detent step.
template <int TStep, uint8_t Tpin1, uint8_t Tpin2>
struct CCEncoder : public IEncoder
{
    Encoder mEnc;
    float mState;
    int32_t mRawValue;

  public:
    CCEncoder() : mEnc(Tpin1, Tpin2)
    {
        mEnc.write(0);
    }

    virtual float CurrentValue() const
    {
        return mState;
    }

    int32_t RawValue() const
    {
        return mRawValue;
    }

    void Update()
    {
        mRawValue = mEnc.read();
        mState = (float)mRawValue / TStep;
    }
};

} // namespace clarinoid
