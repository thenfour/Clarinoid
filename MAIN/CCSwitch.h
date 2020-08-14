// represents typical switches on a digital pin.
// no interrupts, just poll.
// connect pin to gnd.

#ifndef CCSWITCH_H
#define CCSWITCH_H

#include <Bounce.h>
#include "CCUtil.h"

class CCOnOffSwitch : IUpdateObject
{
  uint8_t mPin;
  CCThrottler mThrottle;
  bool mIsPressed;
  bool mIsDirty;
  Bounce mBounce;

public:
  explicit CCOnOffSwitch(uint8_t pin, uint32_t updatePeriodMS, uint32_t bouncePeriodMS) :
    mPin(pin),
    mThrottle(updatePeriodMS),
    mIsPressed(false),
    mIsDirty(false),
    mBounce(pin, bouncePeriodMS)
  {
    //
  }

  virtual void setup()
  {
    pinMode(mPin, INPUT_PULLUP);
  }

  virtual void loop()
  {
    mIsDirty = false;
    if (!mThrottle.IsReady())
      return;
    mBounce.update();
    if (mBounce.fallingEdge()) {
      mIsPressed = true;
      mIsDirty = true;
    }
    if (mBounce.risingEdge()) {
      mIsPressed = false;
      mIsDirty = true;
    }
  }

  bool IsPressed() const {
    return mIsPressed;
  }
  bool IsDirty() const {
    return mIsDirty;
  }
};

#endif
