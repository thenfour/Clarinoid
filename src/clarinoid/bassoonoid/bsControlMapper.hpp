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
    IInputSource,
    ITask
{
    TimeSpan mTimingMcpR;
    TimeSpan mTimingMcpL;
    TimeSpan mTimingBreath;
    TimeSpan mTimingEncoders;
    TimeSpan mTimingAnalog;
    TimeSpan mTimingDigital;

    virtual void TaskRun() override
    {
        NoInterrupts _ni;
        Stopwatch sw;
        mCPEncoder.Update();
        mRHEncoder.Update();
        mLHEncoder.Update();// reading encoders is dirt cheap. 1 microsecond.
        mTimingEncoders = sw.ElapsedTime();

        sw.Restart();
        mRHMCP.Update();
        mTimingMcpR = sw.ElapsedTime();

        sw.Restart();
        mLHMCP.Update();
        mTimingMcpL = sw.ElapsedTime();

        sw.Restart();
        mBreath.Update();
        mTimingBreath = sw.ElapsedTime();

        sw.Restart();
        mVolumePot.Update();
        mPitchStrip.Update();
        mJoyX.Update();
        mJoyY.Update();
        mTimingAnalog = sw.ElapsedTime();

        sw.Restart();
        mToggleUp.Update();
        mTimingDigital = sw.ElapsedTime();
    }

    virtual void InputSource_Init(struct InputDelegator* p) override
    {
        mEncoderInfo[(size_t)PhysicalEncoder::CP] = EncoderInfo { "CPENC", &mCPEncoder };
        mEncoderInfo[(size_t)PhysicalEncoder::LH] = EncoderInfo { "LHENC", &mLHEncoder };
        mEncoderInfo[(size_t)PhysicalEncoder::RH] = EncoderInfo { "RHENC", &mRHEncoder };

        mAxisInfo[(size_t)PhysicalAxis::Breath] = AxisInfo { "Breath", &mBreath };
        mAxisInfo[(size_t)PhysicalAxis::Volume] = AxisInfo { "VolPot", &mVolumePot };
        mAxisInfo[(size_t)PhysicalAxis::Pitch] = AxisInfo { "Pitch", &mPitchStrip };
        mAxisInfo[(size_t)PhysicalAxis::JoyX] = AxisInfo { "JoyX", &mJoyX };
        mAxisInfo[(size_t)PhysicalAxis::JoyY] = AxisInfo { "JoyY", &mJoyY };

        mSwitchInfo[(size_t)PhysicalSwitch::CPBack] = SwitchInfo { "CPBack", &mRHMCP.mButtons[0] };
        mSwitchInfo[(size_t)PhysicalSwitch::CPOk] = SwitchInfo { "CPOk", &mRHMCP.mButtons[1] };
        mSwitchInfo[(size_t)PhysicalSwitch::CPToggleUp] = SwitchInfo { "CPToggleUp", &mToggleUp };
        mSwitchInfo[(size_t)PhysicalSwitch::CPEnc] = SwitchInfo { "CPEnc", &mRHMCP.mButtons[2] };
        mSwitchInfo[(size_t)PhysicalSwitch::LHx1] = SwitchInfo { "LHx1", &mLHMCP.mButtons[0] };
        mSwitchInfo[(size_t)PhysicalSwitch::LHx2] = SwitchInfo { "LHx2", &mLHMCP.mButtons[1] };
        mSwitchInfo[(size_t)PhysicalSwitch::LHx3] = SwitchInfo { "LHx3", &mLHMCP.mButtons[2] };
        mSwitchInfo[(size_t)PhysicalSwitch::LHx4] = SwitchInfo { "LHx4", &mLHMCP.mButtons[3] };
        mSwitchInfo[(size_t)PhysicalSwitch::LHEnc] = SwitchInfo { "LHEnc", &mLHMCP.mButtons[13] };
        mSwitchInfo[(size_t)PhysicalSwitch::LHBack] = SwitchInfo { "LHBack", &mLHMCP.mButtons[11] };
        mSwitchInfo[(size_t)PhysicalSwitch::LHOk] = SwitchInfo { "LHOk", &mLHMCP.mButtons[12] };
        mSwitchInfo[(size_t)PhysicalSwitch::LHThx1] = SwitchInfo { "LHThx1", &mLHMCP.mButtons[14] };
        mSwitchInfo[(size_t)PhysicalSwitch::LHThx2] = SwitchInfo { "LHThx2", &mLHMCP.mButtons[15] };
        mSwitchInfo[(size_t)PhysicalSwitch::LHOct1] = SwitchInfo { "LHO1", &mLHMCP.mButtons[10] };
        mSwitchInfo[(size_t)PhysicalSwitch::LHOct2] = SwitchInfo { "LHO2", &mLHMCP.mButtons[9] };
        mSwitchInfo[(size_t)PhysicalSwitch::LHOct3] = SwitchInfo { "LHO3", &mLHMCP.mButtons[8] };
        mSwitchInfo[(size_t)PhysicalSwitch::LHKey1] = SwitchInfo { "LHKey1", &mLHMCP.mButtons[7] };
        mSwitchInfo[(size_t)PhysicalSwitch::LHKey2] = SwitchInfo { "LHKey2", &mLHMCP.mButtons[6] };
        mSwitchInfo[(size_t)PhysicalSwitch::LHKey3] = SwitchInfo { "LHKey3", &mLHMCP.mButtons[5] };
        mSwitchInfo[(size_t)PhysicalSwitch::LHKey4] = SwitchInfo { "LHKey4", &mLHMCP.mButtons[4] };

        mSwitchInfo[(size_t)PhysicalSwitch::RHTh1] = SwitchInfo { "RHTh1", &mRHMCP.mButtons[5] };
        mSwitchInfo[(size_t)PhysicalSwitch::RHTh2] = SwitchInfo { "RHTh2", &mRHMCP.mButtons[6] };
        mSwitchInfo[(size_t)PhysicalSwitch::RHTh3] = SwitchInfo { "RHTh3", &mRHMCP.mButtons[7] };
        mSwitchInfo[(size_t)PhysicalSwitch::RHx1] = SwitchInfo { "RHx1", &mRHMCP.mButtons[4] };
        mSwitchInfo[(size_t)PhysicalSwitch::RHx2] = SwitchInfo { "RHx2", &mRHMCP.mButtons[12] };
        mSwitchInfo[(size_t)PhysicalSwitch::RHx3] = SwitchInfo { "RHx3", &mRHMCP.mButtons[13] };
        mSwitchInfo[(size_t)PhysicalSwitch::RHx4] = SwitchInfo { "RHx4", &mRHMCP.mButtons[14] };
        mSwitchInfo[(size_t)PhysicalSwitch::RHx5] = SwitchInfo { "RHx5", &mRHMCP.mButtons[15] };
        mSwitchInfo[(size_t)PhysicalSwitch::RHKey1] = SwitchInfo { "RHKey1", &mRHMCP.mButtons[8] };
        mSwitchInfo[(size_t)PhysicalSwitch::RHKey2] = SwitchInfo { "RHKey2", &mRHMCP.mButtons[9] };
        mSwitchInfo[(size_t)PhysicalSwitch::RHKey3] = SwitchInfo { "RHKey3", &mRHMCP.mButtons[10] };
        mSwitchInfo[(size_t)PhysicalSwitch::RHKey4] = SwitchInfo { "RHKey4", &mRHMCP.mButtons[11] };
    }

    virtual size_t InputSource_GetSwitchCount() override { return SizeofStaticArray(mSwitchInfo); }
    virtual SwitchInfo InputSource_GetSwitch(PhysicalSwitch index) override
    {
        CCASSERT((size_t)index < SizeofStaticArray(mSwitchInfo));
        return mSwitchInfo[(size_t)index];
    }
    virtual size_t InputSource_GetAxisCount() override { return SizeofStaticArray(mAxisInfo); }
    virtual AxisInfo InputSource_GetAxis(PhysicalAxis index) override
    {
        CCASSERT((size_t)index < SizeofStaticArray(mAxisInfo));
        return mAxisInfo[(size_t)index];
    }
    virtual size_t InputSource_GetEncoderCount() override { return SizeofStaticArray(mEncoderInfo); }
    virtual EncoderInfo InputSource_GetEncoder(PhysicalEncoder index) override
    {
        CCASSERT((size_t)index < SizeofStaticArray(mEncoderInfo));
        return mEncoderInfo[(size_t)index];
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

    SwitchInfo mSwitchInfo[(size_t)PhysicalSwitch::COUNT];
    AxisInfo mAxisInfo[(size_t)PhysicalAxis::COUNT];
    EncoderInfo mEncoderInfo[(size_t)PhysicalEncoder::COUNT];
};

} // namespace clarinoid
