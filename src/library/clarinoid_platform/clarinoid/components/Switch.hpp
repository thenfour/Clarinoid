// represents typical switches on a digital pin.
// no interrupts, just poll.

#pragma once

//#include <Bounce.h>
#include "../basic/Basic.hpp"

namespace clarinoid
{

struct NullSwitch : ISwitch
{
    NullSwitch()
    {
    }

    virtual bool CurrentValue() const override
    {
        return false;
    }
};

// assume pullup required
struct DigitalPinSwitch : ISwitch
{
    uint8_t mPin;
    bool mCurrentValue = false;

    explicit DigitalPinSwitch(uint8_t pin) : mPin(pin)
    {
        pinMode(pin, INPUT_PULLUP);
    }

    void Update()
    {
        mCurrentValue = !!digitalReadFast(mPin);
    }

    virtual bool CurrentValue() const override
    {
        return mCurrentValue;
    }
};

template <uint8_t mPin>
struct DigitalPinSwitchT : ISwitch
{
    bool mCurrentValue = false;

    DigitalPinSwitchT()
    {
        pinMode(mPin, INPUT_PULLUP);
    }

    void Update()
    {
        mCurrentValue = !!digitalReadFast(mPin);
    }

    virtual bool CurrentValue() const override
    {
        return mCurrentValue;
    }
};
} // namespace clarinoid
