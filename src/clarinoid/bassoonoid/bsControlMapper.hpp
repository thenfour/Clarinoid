// LH MCP buttons:
// 0 = x pinky
// 1 = x 1
// 2 = x 2
// 3 = x index
// 4 = key1
// 5 = key2
// 6 = key3
// 7 = key4
// 8 = oct up
// 9 = oct neutral
// 10 = oct down
// 11 = lh back
// 12 = lh ok
// 13 = lh enc button
// 14 = x oct 1
// 15 = x oct 2

// RH MCP buttons:
// 0 = cp back
// 1 = cp ok
// 2 = cp enc
// 4 = x5
// 5 = x thumb 1
// 6 = x thumb 2
// 7 = x thumb 3
// 8 = key1
// 9 = key2
// 10 = key3
// 11 = key4
// 12 = x1
// 13 = x2
// 14 = x3
// 15 = x4

#pragma once

#include "clarinoid/basic/Basic.hpp"
#include "clarinoid/application/ControlMapper.hpp"

#include "clarinoid/components/Encoder.hpp"
#include "clarinoid/components/Switch.hpp"
#include "clarinoid/components/HoneywellABPI2C.hpp"

namespace clarinoid
{


struct BassoonoidControlMapper :
    IControlMapper,
    ITask
{
    NullEncoder mNullEncoder;
    NullSwitch mNullSwitch;
    NullBreathSensor mNullBreath;

    virtual void TaskRun() override
    {
        mCPEncoder.Update();
        mRHEncoder.Update();
        mLHEncoder.Update();// reading encoders is dirt cheap. 1 microsecond.

        //Stopwatch sw;
        mRHMCP.Update();
        mLHMCP.Update();
        mBreath.Update();
        //int i2c = sw.ElapsedMicros();

        //sw.Restart();
        mVolumePot.Update();
        mPitchStrip.Update();
        mJoyX.Update();
        mJoyY.Update();
        //int ana = sw.ElapsedMicros();

        mToggleUp.Update();

        //Serial.println(String("TIMING: i2c=") + i2c + ", ana=" + ana);
    }

    CCEncoder<1, 2, 3> mRHEncoder;
    CCEncoder<1, 5, 4> mLHEncoder;
    CCEncoder<1, 31, 32> mCPEncoder;
    CCMCP23017 mRHMCP = CCMCP23017 { &Wire1 };
    CCMCP23017 mLHMCP = CCMCP23017 { &Wire2 };
    CCHoneywellAPB mBreath = CCHoneywellAPB { Wire };
    AnalogPinControl mVolumePot = AnalogPinControl { 23 };
    AnalogPinControl mPitchStrip = AnalogPinControl { 40 };
    AnalogPinControl mJoyX = AnalogPinControl { 38 };
    AnalogPinControl mJoyY = AnalogPinControl { 39 };
    DigitalPinSwitch mToggleUp = DigitalPinSwitch { 30 };

    virtual IEncoder* MenuEncoder() override { return &mLHEncoder; };
    virtual ISwitch* MenuBack() override { return &mLHMCP.mButtons[11]; };
    virtual ISwitch* MenuOK() override { return &mLHMCP.mButtons[12]; };

    virtual IAnalogControl* BreathSensor() { return &mBreath; };

    virtual ISwitch* KeyLH1() override { return &mLHMCP.mButtons[4]; };
    virtual ISwitch* KeyLH2() override { return &mLHMCP.mButtons[5]; };
    virtual ISwitch* KeyLH3() override { return &mLHMCP.mButtons[6]; };
    virtual ISwitch* KeyLH4() override { return &mLHMCP.mButtons[7]; };

    virtual ISwitch* KeyRH1() override { return &mRHMCP.mButtons[8]; };
    virtual ISwitch* KeyRH2() override { return &mRHMCP.mButtons[9]; };
    virtual ISwitch* KeyRH3() override { return &mRHMCP.mButtons[10]; };
    virtual ISwitch* KeyRH4() override { return &mRHMCP.mButtons[11]; };

    virtual ISwitch* KeyOct1() override { return &mLHMCP.mButtons[8]; };
    virtual ISwitch* KeyOct2() override { return &mLHMCP.mButtons[9]; };
    virtual ISwitch* KeyOct3() override { return &mLHMCP.mButtons[10]; };
};

} // namespace clarinoid
