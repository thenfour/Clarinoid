// control mapper maps physical control to hardware inputs

#pragma once

#include "clarinoid/basic/Basic.hpp"
#include "clarinoid/application/ControlMapper.hpp"

#include "clarinoid/components/Encoder.hpp"
#include "clarinoid/components/Switch.hpp"
#include "clarinoid/components/HoneywellABPI2C.hpp"

namespace clarinoid
{

struct Clarinoid2ControlMapper :
    IInputSource,
    ITask
{
    CCDisplay* mDisplay = nullptr;

    void Init(CCDisplay* display)
    {
        mDisplay = display;
    }

    virtual void TaskRun() override
    {
        NoInterrupts _ni;
        mEncoder.Update();
        mMCP.Update();
        mLHMPR.Update();
        mRHMPR.Update();
        mBreath.Update();
        mPitchStrip.Update();
    }

    virtual void InputSource_Init(struct InputDelegator* p) override
    {
        mControlInfo[(size_t)PhysicalControl::Breath] = ControlInfo { "Breath", &mBreath };
        mControlInfo[(size_t)PhysicalControl::Pitch] = ControlInfo { "Pitch", &mPitchStrip };

        mControlInfo[(size_t)PhysicalControl::Enc] = ControlInfo { "ENC", &mEncoder };

        mControlInfo[(size_t)PhysicalControl::Back] = ControlInfo { "CPBack", &mMCP.mButtons[4] };
        mControlInfo[(size_t)PhysicalControl::Ok] = ControlInfo { "CPOk", &mMCP.mButtons[5] };
        mControlInfo[(size_t)PhysicalControl::EncButton] = ControlInfo { "CPEncBtn", &mMCP.mButtons[3] };

        mControlInfo[(size_t)PhysicalControl::LHx1] = ControlInfo { "LHx1", &mMCP.mButtons[0] };
        mControlInfo[(size_t)PhysicalControl::LHx2] = ControlInfo { "LHx2", &mMCP.mButtons[1] };
        mControlInfo[(size_t)PhysicalControl::LHx3] = ControlInfo { "LHx3", &mMCP.mButtons[2] };

        mControlInfo[(size_t)PhysicalControl::Oct1] = ControlInfo { "LHO1", &mLHMPR.mButtons[0] };
        mControlInfo[(size_t)PhysicalControl::Oct2] = ControlInfo { "LHO2", &mLHMPR.mButtons[1] };
        mControlInfo[(size_t)PhysicalControl::Oct3] = ControlInfo { "LHO3", &mLHMPR.mButtons[2] };
        mControlInfo[(size_t)PhysicalControl::Oct4] = ControlInfo { "LHO4", &mLHMPR.mButtons[3] };
        mControlInfo[(size_t)PhysicalControl::Oct5] = ControlInfo { "LHO5", &mLHMPR.mButtons[4] };
        mControlInfo[(size_t)PhysicalControl::Oct6] = ControlInfo { "LHO6", &mLHMPR.mButtons[5] };

        mControlInfo[(size_t)PhysicalControl::LHKey1] = ControlInfo { "LHKey1", &mLHMPR.mButtons[7] };
        mControlInfo[(size_t)PhysicalControl::LHKey2] = ControlInfo { "LHKey2", &mLHMPR.mButtons[8] };
        mControlInfo[(size_t)PhysicalControl::LHKey3] = ControlInfo { "LHKey3", &mLHMPR.mButtons[6] };
        mControlInfo[(size_t)PhysicalControl::LHKey4] = ControlInfo { "LHKey4", &mLHMPR.mButtons[9] };

        mControlInfo[(size_t)PhysicalControl::RHx1] = ControlInfo { "RHx1", &mMCP.mButtons[14] };
        mControlInfo[(size_t)PhysicalControl::RHx2] = ControlInfo { "RHx2", &mMCP.mButtons[13] };
        mControlInfo[(size_t)PhysicalControl::RHx3] = ControlInfo { "RHx3", &mMCP.mButtons[12] };
        mControlInfo[(size_t)PhysicalControl::RHx4] = ControlInfo { "RHx4", &mMCP.mButtons[11] };

        mControlInfo[(size_t)PhysicalControl::RHKey1] = ControlInfo { "RHKey1", &mRHMPR.mButtons[3] };
        mControlInfo[(size_t)PhysicalControl::RHKey2] = ControlInfo { "RHKey2", &mRHMPR.mButtons[2] };
        mControlInfo[(size_t)PhysicalControl::RHKey3] = ControlInfo { "RHKey3", &mRHMPR.mButtons[1] };
        mControlInfo[(size_t)PhysicalControl::RHKey4] = ControlInfo { "RHKey4", &mRHMPR.mButtons[0] };
    }

    virtual size_t InputSource_GetControlCount() override { return SizeofStaticArray(mControlInfo); }
    virtual ControlInfo InputSource_GetControl(PhysicalControl index) override
    {
        CCASSERT((size_t)index < SizeofStaticArray(mControlInfo));
        return mControlInfo[(size_t)index];
    }

    virtual void InputSource_ShowToast(const String& s) 
    {
        mDisplay->ShowToast(s);
    }

    CCEncoder<4/* step increment each increment */, 2 /* pin #1 */, 3 /* pin #2 */> mEncoder;
    CCMPR121 mLHMPR = CCMPR121 { &Wire1, 0x5A, 10 };
    CCMPR121 mRHMPR = CCMPR121 { &Wire1, 0x5B, 4 };
    CCMCP23017 mMCP = CCMCP23017 { &Wire1, 0x20 };
    CCHoneywellAPB mBreath = CCHoneywellAPB { Wire1, 0x28 };
    AnalogPinControl mPitchStrip = AnalogPinControl { 23 };

    ControlInfo mControlInfo[(size_t)PhysicalControl::COUNT];
};

} // namespace clarinoid
