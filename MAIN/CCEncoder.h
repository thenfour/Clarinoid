// stepped encoder (not continuous)

#ifndef CCENCODER_H
#define CCENCODER_H

#define ENCODER_DO_NOT_USE_INTERRUPTS

#include <Encoder.h>

#include "CCUtil.h"

class CCEncoder : IUpdateObject
{
  Encoder mEnc;

  // valid during this loop iteration
  bool mIsDirty;
  int mValue;

public:
  CCEncoder(uint8_t pin1, uint8_t pin2) :
    mEnc(pin1, pin2),
    mIsDirty(false),
    mValue(0)
  {
    mEnc.write(0);
  }

  bool IsDirty() const { return mIsDirty; }
  int GetValue() const { return mValue; }

  virtual void setup()
  {
    // pinMode() is set by encoder lib
  }

  virtual void loop()
  {
    int32_t r = mEnc.read();
    //mDelta = idiv_round(r, mIncrementAmt);
    mIsDirty = mValue != r;
    mValue = r;
  }
};


#endif
