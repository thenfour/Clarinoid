
#pragma once

//#include <Adafruit_MCP23017.h>
#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

// attach to a CCMCP23017 value to represent a single switch
struct BitButton :
  ISwitch
{
  uint16_t& mMcpCurrentValue;
  int mKeyMask;

  explicit BitButton(uint16_t& mcpCurrentValue, int keyIndex /* 0-15 */) :
    mMcpCurrentValue(mcpCurrentValue),
    mKeyMask(1 << keyIndex)
  {
    CCASSERT(keyIndex >= 0 && keyIndex < 16);
  }

  virtual bool CurrentValue() const override { return !(mMcpCurrentValue & mKeyMask); }
};



struct CCMCP23017// :
  //ITask
{
  Adafruit_MCP23017 mMcp;
  uint16_t mCurrentValue = 0;
  uint16_t mPreviousValue = 0;

  BitButton mButtons[16] = {
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
    BitButton{ mCurrentValue, 12 },
    BitButton{ mCurrentValue, 13 },
    BitButton{ mCurrentValue, 14 },
    BitButton{ mCurrentValue, 15 },
  };

  explicit CCMCP23017(TwoWire *theWire)
  {
    NoInterrupts _ni;
    mMcp.begin(theWire);
    theWire->setClock(800000/*400000*/); // use high speed mode. default speed = 100k
    for (int i  = 0; i < 16; ++ i) {
      mMcp.pinMode(i, INPUT);
      mMcp.pullUp(i, HIGH);  // turn on a 100K pullup internally
    }
  }

  void Update()
  {
    NoInterrupts _ni;
    mPreviousValue = mCurrentValue;
    mCurrentValue = mMcp.readGPIOAB();
  }
};

} // namespace clarinoid
