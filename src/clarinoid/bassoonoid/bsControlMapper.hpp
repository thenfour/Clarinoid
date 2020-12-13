
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
        mControlInfo[(size_t)PhysicalControl::CPEnc] = ControlInfo { "CPENC", &mCPEncoder };
        mControlInfo[(size_t)PhysicalControl::LHEnc] = ControlInfo { "LHENC", &mLHEncoder };
        mControlInfo[(size_t)PhysicalControl::RHEnc] = ControlInfo { "RHENC", &mRHEncoder };

        mControlInfo[(size_t)PhysicalControl::Breath] = ControlInfo { "Breath", &mBreath };
        mControlInfo[(size_t)PhysicalControl::Volume] = ControlInfo { "VolPot", &mVolumePot };
        mControlInfo[(size_t)PhysicalControl::Pitch] = ControlInfo { "Pitch", &mPitchStrip };
        mControlInfo[(size_t)PhysicalControl::JoyX] = ControlInfo { "JoyX", &mJoyX };
        mControlInfo[(size_t)PhysicalControl::JoyY] = ControlInfo { "JoyY", &mJoyY };

        mControlInfo[(size_t)PhysicalControl::CPBack] = ControlInfo { "CPBack", &mRHMCP.mButtons[0] };
        mControlInfo[(size_t)PhysicalControl::CPOk] = ControlInfo { "CPOk", &mRHMCP.mButtons[1] };
        mControlInfo[(size_t)PhysicalControl::CPToggleUp] = ControlInfo { "CPToggleUp", &mToggleUp };
        mControlInfo[(size_t)PhysicalControl::CPEncButton] = ControlInfo { "CPEncBtn", &mRHMCP.mButtons[2] };
        mControlInfo[(size_t)PhysicalControl::LHx1] = ControlInfo { "LHx1", &mLHMCP.mButtons[0] };
        mControlInfo[(size_t)PhysicalControl::LHx2] = ControlInfo { "LHx2", &mLHMCP.mButtons[1] };
        mControlInfo[(size_t)PhysicalControl::LHx3] = ControlInfo { "LHx3", &mLHMCP.mButtons[2] };
        mControlInfo[(size_t)PhysicalControl::LHx4] = ControlInfo { "LHx4", &mLHMCP.mButtons[3] };
        mControlInfo[(size_t)PhysicalControl::LHEncButton] = ControlInfo { "LHEncBtn", &mLHMCP.mButtons[13] };
        mControlInfo[(size_t)PhysicalControl::LHBack] = ControlInfo { "LHBack", &mLHMCP.mButtons[11] };
        mControlInfo[(size_t)PhysicalControl::LHOk] = ControlInfo { "LHOk", &mLHMCP.mButtons[12] };
        mControlInfo[(size_t)PhysicalControl::LHThx1] = ControlInfo { "LHThx1", &mLHMCP.mButtons[14] };
        mControlInfo[(size_t)PhysicalControl::LHThx2] = ControlInfo { "LHThx2", &mLHMCP.mButtons[15] };
        mControlInfo[(size_t)PhysicalControl::LHOct1] = ControlInfo { "LHO1", &mLHMCP.mButtons[10] };
        mControlInfo[(size_t)PhysicalControl::LHOct2] = ControlInfo { "LHO2", &mLHMCP.mButtons[9] };
        mControlInfo[(size_t)PhysicalControl::LHOct3] = ControlInfo { "LHO3", &mLHMCP.mButtons[8] };
        mControlInfo[(size_t)PhysicalControl::LHKey1] = ControlInfo { "LHKey1", &mLHMCP.mButtons[7] };
        mControlInfo[(size_t)PhysicalControl::LHKey2] = ControlInfo { "LHKey2", &mLHMCP.mButtons[6] };
        mControlInfo[(size_t)PhysicalControl::LHKey3] = ControlInfo { "LHKey3", &mLHMCP.mButtons[5] };
        mControlInfo[(size_t)PhysicalControl::LHKey4] = ControlInfo { "LHKey4", &mLHMCP.mButtons[4] };

        mControlInfo[(size_t)PhysicalControl::RHTh1] = ControlInfo { "RHTh1", &mRHMCP.mButtons[5] };
        mControlInfo[(size_t)PhysicalControl::RHTh2] = ControlInfo { "RHTh2", &mRHMCP.mButtons[6] };
        mControlInfo[(size_t)PhysicalControl::RHTh3] = ControlInfo { "RHTh3", &mRHMCP.mButtons[7] };
        mControlInfo[(size_t)PhysicalControl::RHx1] = ControlInfo { "RHx1", &mRHMCP.mButtons[4] };
        mControlInfo[(size_t)PhysicalControl::RHx2] = ControlInfo { "RHx2", &mRHMCP.mButtons[12] };
        mControlInfo[(size_t)PhysicalControl::RHx3] = ControlInfo { "RHx3", &mRHMCP.mButtons[13] };
        mControlInfo[(size_t)PhysicalControl::RHx4] = ControlInfo { "RHx4", &mRHMCP.mButtons[14] };
        mControlInfo[(size_t)PhysicalControl::RHx5] = ControlInfo { "RHx5", &mRHMCP.mButtons[15] };
        mControlInfo[(size_t)PhysicalControl::RHKey1] = ControlInfo { "RHKey1", &mRHMCP.mButtons[8] };
        mControlInfo[(size_t)PhysicalControl::RHKey2] = ControlInfo { "RHKey2", &mRHMCP.mButtons[9] };
        mControlInfo[(size_t)PhysicalControl::RHKey3] = ControlInfo { "RHKey3", &mRHMCP.mButtons[10] };
        mControlInfo[(size_t)PhysicalControl::RHKey4] = ControlInfo { "RHKey4", &mRHMCP.mButtons[11] };
    }

    virtual size_t InputSource_GetControlCount() override { return SizeofStaticArray(mControlInfo); }
    virtual ControlInfo InputSource_GetControl(PhysicalControl index) override
    {
        CCASSERT((size_t)index < SizeofStaticArray(mControlInfo));
        return mControlInfo[(size_t)index];
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

    ControlInfo mControlInfo[(size_t)PhysicalControl::COUNT];
};

} // namespace clarinoid
