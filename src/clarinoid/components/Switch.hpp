// represents typical switches on a digital pin.
// no interrupts, just poll.

#pragma once

//#include <Bounce.h>
#include "../basic/Basic.hpp"

namespace clarinoid
{


#ifdef CLARINOID_PLATFORM_X86
#else  // CLARINOID_UNIT_TESTS

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
        pinMode(mPin, INPUT_PULLUP);
        // mCurrentValue = !!digitalReadFast(mPin);
        mCurrentValue = !!digitalRead(mPin);
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

template <uint8_t mPin, uint32_t mIntervalMillis>
struct DebouncedDigitalPinSwitchT : ISwitch
{
    Bounce mBounce;

    DebouncedDigitalPinSwitchT() : mBounce(mPin, mIntervalMillis)
    {
        pinMode(mPin, INPUT_PULLUP);
    }

    void Update()
    {
        mBounce.update();
    }

    virtual bool CurrentValue() const override
    {
        return const_cast<DebouncedDigitalPinSwitchT<mPin, mIntervalMillis> *>(this)->CurrentValue();
    }

    bool CurrentValue()
    {
        return (int)mBounce.read() == 0;
    }
};
#endif // CLARINOID_UNIT_TESTS

} // namespace clarinoid
