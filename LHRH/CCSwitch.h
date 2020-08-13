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
  bool mState;
  Bounce mBounce;

public:
  CCOnOffSwitch(uint8_t pin, uint32_t updatePeriodMS, uint32_t bouncePeriodMS) :
    mPin(pin),
    mThrottle(updatePeriodMS),
    mState(false),
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
    if (!mThrottle.IsReady())
      return;
    mBounce.update();
    if (mBounce.fallingEdge())
      mState = true;
    if (mBounce.risingEdge())
      mState = false;
  }

  bool State() const {
    return mState;
  }
};

#endif
