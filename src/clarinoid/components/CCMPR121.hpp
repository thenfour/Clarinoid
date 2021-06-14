
#pragma once

// make sure to #include <Adafruit_MPR121.h>

#include <clarinoid/basic/Basic.hpp>

#include "MCP23017.hpp" // for bitbutton

namespace clarinoid
{

struct CCMPR121
{
  Adafruit_MPR121 mMpr121;
  uint16_t mCurrentValue = 0;

  BitButton mButtons[12] = {
    BitButton{ mCurrentValue, 0 },
    BitButton{ mCurrentValue, 1 },
    BitButton{ mCurrentValue, 2 },
    BitButton{ mCurrentValue, 3 },
    BitButton{ mCurrentValue, 4 },
    BitButton{ mCurrentValue, 5 },
    BitButton{ mCurrentValue, 6 },
    BitButton{ mCurrentValue, 7 },
    BitButton{ mCurrentValue, 8 },
    BitButton{ mCurrentValue, 9 },
    BitButton{ mCurrentValue, 10 },
    BitButton{ mCurrentValue, 11 },
  };

  explicit CCMPR121(TwoWire *theWire, int address)
  {
    NoInterrupts _ni;
    mMpr121.begin(address, theWire);
  }

  void Update()
  {
    NoInterrupts _ni;
    mCurrentValue = mMpr121.touched();
  }
};

} // namespace clarinoid
