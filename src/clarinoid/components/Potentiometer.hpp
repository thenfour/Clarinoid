
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

struct AnalogPinControl : IAnalogAxis
{
    uint8_t mPin;
    float mValue01 = 0;
    int mRawValue = 0;

    explicit AnalogPinControl(uint8_t pin) : mPin(pin)
    {
        pinMode(mPin, INPUT);
    }

    void Update()
    {
        int32_t a = analogRead(mPin);
        mValue01 = (float)a / 1024.0f;
        // log(String("pitch: ") + a + " -> " + mValue01);
    }

    virtual float CurrentValue01() const override
    {
        return mValue01;
    }
};

} // namespace clarinoid
