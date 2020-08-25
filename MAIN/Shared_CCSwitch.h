// represents typical switches on a digital pin.
// no interrupts, just poll.
// connect pin to gnd.

#ifndef CCSWITCH_H
#define CCSWITCH_H

#include <Bounce.h>
#include "Shared_CCUtil.h"

struct ICCSwitch
{
  virtual bool IsCurrentlyPressed() const = 0;
  virtual bool IsNewlyPressed() const = 0;
  virtual bool IsDirty() const = 0;
};

// like CCOnOffSwitch but you update it manually instead of using bounce library + physical pins
struct CCVirtualSwitch : ICCSwitch
{
  bool mIsDirty = false;
  bool mIsCurrentlyPressed = false;
  bool mIsNewlyPressed = false;

  void Update(bool isCurrentlyPressed)
  {
    mIsDirty = mIsCurrentlyPressed != isCurrentlyPressed;
    mIsNewlyPressed = !mIsCurrentlyPressed && isCurrentlyPressed;
    mIsCurrentlyPressed = isCurrentlyPressed;
  }
  
  virtual bool IsDirty() const { return mIsDirty; }
  virtual bool IsNewlyPressed() const { return mIsNewlyPressed; }
  virtual bool IsCurrentlyPressed() const { return mIsCurrentlyPressed; }
};

class CCOnOffSwitch : IUpdateObject, ICCSwitch
{
  uint8_t mPin;
  bool mIsCurrentlyPressed = false;
  bool mIsNewlyPressed = false;
  bool mIsDirty = false;
  Bounce mBounce;

public:
  explicit CCOnOffSwitch(uint8_t pin, uint32_t bouncePeriodMS) :
    mPin(pin),
    mBounce(pin, bouncePeriodMS)
  {
  }

  virtual void setup()
  {
    pinMode(mPin, INPUT_PULLUP);
  }

  virtual void loop()
  {
    mIsDirty = false;
    mIsNewlyPressed = false;
    mBounce.update();
    if (mBounce.fallingEdge()) {
      mIsDirty = !mIsCurrentlyPressed;
      mIsNewlyPressed = !mIsCurrentlyPressed;
      mIsCurrentlyPressed = true;
    } else if (mBounce.risingEdge()) {
      mIsDirty = mIsCurrentlyPressed;
      mIsCurrentlyPressed = false;
    }
  }

  bool IsCurrentlyPressed() const {
    return mIsCurrentlyPressed;
  }
  bool IsNewlyPressed() const {
    return mIsNewlyPressed;
  }
  bool IsDirty() const {
    return mIsDirty;
  }
};

#endif
