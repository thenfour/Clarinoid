
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

struct CCMCP23017
{
    Adafruit_MCP23017 mMcp;
    uint16_t mCurrentValue = 0;
    uint16_t mPreviousValue = 0;

    InvertedBitButton mButtons[16] = {
        InvertedBitButton{mCurrentValue, 0},
        InvertedBitButton{mCurrentValue, 1},
        InvertedBitButton{mCurrentValue, 2},
        InvertedBitButton{mCurrentValue, 3},
        InvertedBitButton{mCurrentValue, 4},
        InvertedBitButton{mCurrentValue, 5},
        InvertedBitButton{mCurrentValue, 6},
        InvertedBitButton{mCurrentValue, 7},
        InvertedBitButton{mCurrentValue, 8},
        InvertedBitButton{mCurrentValue, 9},
        InvertedBitButton{mCurrentValue, 10},
        InvertedBitButton{mCurrentValue, 11},
        InvertedBitButton{mCurrentValue, 12},
        InvertedBitButton{mCurrentValue, 13},
        InvertedBitButton{mCurrentValue, 14},
        InvertedBitButton{mCurrentValue, 15},
    };

    explicit CCMCP23017(TwoWire *theWire, int address = 0x20)
    {
        NoInterrupts _ni;
        mMcp.begin(theWire);
        // theWire->setClock(400000); // use high speed mode. default speed = 100k
        for (int i = 0; i < 16; ++i)
        {
            mMcp.pinMode(i, INPUT);
            mMcp.pullUp(i, HIGH); // turn on a 100K pullup internally
        }
    }

    void Update()
    {
        auto newCurrentVal = mMcp.readGPIOAB();
        mPreviousValue = mCurrentValue;
        mCurrentValue = newCurrentVal;
    }
};

} // namespace clarinoid
