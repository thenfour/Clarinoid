
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

struct PCA9554
{
    uint16_t mCurrentValue = 0;
    uint16_t mPreviousValue = 0;
    TwoWire &mWire;
    int mAddress;

    InvertedBitButton mButtons[8] = {
        InvertedBitButton{mCurrentValue, 0},
        InvertedBitButton{mCurrentValue, 1},
        InvertedBitButton{mCurrentValue, 2},
        InvertedBitButton{mCurrentValue, 3},
        InvertedBitButton{mCurrentValue, 4},
        InvertedBitButton{mCurrentValue, 5},
        InvertedBitButton{mCurrentValue, 6},
        InvertedBitButton{mCurrentValue, 7},
    };

    explicit PCA9554(TwoWire &theWire, int address = 0x38) : mWire(theWire), mAddress(address)
    {
        // assume wire is already initialized.
        NoInterrupts _ni;

        mWire.beginTransmission(mAddress); // Choose the PCA9554A
        mWire.write(byte(0x03));           // port direction command
        mWire.write(byte(0xff));           // bitwise, 1 = input,  0 = output
        mWire.endTransmission();           // End I2C connection
    }

    void Update()
    {
        mWire.beginTransmission(mAddress); // Choose the PCA9554A
        mWire.write(byte(0));         // command 0 = read inputs
        mWire.endTransmission();                  // End I2C connection
        mWire.requestFrom(mAddress, 1);    // Request 1 byte
        mPreviousValue = mCurrentValue;
        mCurrentValue = mWire.read(); // Copy values to variable inputs
    }
};

} // namespace clarinoid
