// control mapper maps physical control to hardware inputs

#pragma once

#include "clarinoid/basic/Basic.hpp"
#include "clarinoid/application/ControlMapper.hpp"

#include "clarinoid/components/Encoder.hpp"
#include "clarinoid/components/Switch.hpp"
#include "clarinoid/components/HoneywellABPI2C.hpp"

namespace clarinoid
{

struct Clarinoid2ControlMapper : IInputSource, ITask
{
    IDisplay *mDisplay = nullptr;

    void Init(IDisplay *display)
    {
        mDisplay = display;
    }

    virtual void TaskRun() override
    {
        NoInterrupts _ni;
        mEncoder.Update();
        mOK.Update();
        mBack.Update();
        mEncButton.Update();
        mButton3.Update();
        // mMCP.Update();
        // mLHMPR.Update();
        // mRHMPR.Update();
        // mBreath.Update();
        // mPitchStrip.Update();
    }

    virtual void InputSource_Init(struct InputDelegator *p) override
    {
        // mControlInfo[(size_t)PhysicalControl::Breath] = ControlInfo{"Breath", &mBreath};
        // mControlInfo[(size_t)PhysicalControl::Pitch] = ControlInfo{"Pitch", &mPitchStrip};

        mControlInfo[(size_t)PhysicalControl::Enc] = ControlInfo{"ENC", &mEncoder};

        mControlInfo[(size_t)PhysicalControl::Back] = ControlInfo{"CPBack", &mBack};
        mControlInfo[(size_t)PhysicalControl::Ok] = ControlInfo{"CPOk", &mOK};
        mControlInfo[(size_t)PhysicalControl::EncButton] = ControlInfo{"CPEncBtn", &mEncButton};

        mControlInfo[(size_t)PhysicalControl::Button3] = ControlInfo{"LHx1", &mButton3};
        // mControlInfo[(size_t)PhysicalControl::LHx2] = ControlInfo{"LHx2", &mMCP.mButtons[1]};
        // mControlInfo[(size_t)PhysicalControl::LHx3] = ControlInfo{"LHx3", &mMCP.mButtons[2]};

        // mControlInfo[(size_t)PhysicalControl::Oct1] = ControlInfo{"LHO1", &mLHMPR.mButtons[0]};
        // mControlInfo[(size_t)PhysicalControl::Oct2] = ControlInfo{"LHO2", &mLHMPR.mButtons[1]};
        // mControlInfo[(size_t)PhysicalControl::Oct3] = ControlInfo{"LHO3", &mLHMPR.mButtons[2]};
        // mControlInfo[(size_t)PhysicalControl::Oct4] = ControlInfo{"LHO4", &mLHMPR.mButtons[3]};
        // mControlInfo[(size_t)PhysicalControl::Oct5] = ControlInfo{"LHO5", &mLHMPR.mButtons[4]};
        // mControlInfo[(size_t)PhysicalControl::Oct6] = ControlInfo{"LHO6", &mLHMPR.mButtons[5]};

        // mControlInfo[(size_t)PhysicalControl::LHKey1] = ControlInfo{"LHKey1", &mLHMPR.mButtons[7]};
        // mControlInfo[(size_t)PhysicalControl::LHKey2] = ControlInfo{"LHKey2", &mLHMPR.mButtons[8]};
        // mControlInfo[(size_t)PhysicalControl::LHKey3] = ControlInfo{"LHKey3", &mLHMPR.mButtons[6]};
        // mControlInfo[(size_t)PhysicalControl::LHKey4] = ControlInfo{"LHKey4", &mLHMPR.mButtons[9]};

        // mControlInfo[(size_t)PhysicalControl::RHx1] = ControlInfo{"RHx1", &mMCP.mButtons[14]}; // closest to bell
        // mControlInfo[(size_t)PhysicalControl::RHx2] = ControlInfo{"RHx2", &mMCP.mButtons[13]};
        // mControlInfo[(size_t)PhysicalControl::RHx3] = ControlInfo{"RHx3", &mMCP.mButtons[12]};
        // mControlInfo[(size_t)PhysicalControl::RHx4] = ControlInfo{"RHx4", &mMCP.mButtons[11]}; // closest to
        // mouthpiec

        // mControlInfo[(size_t)PhysicalControl::RHKey1] = ControlInfo{"RHKey1", &mRHMPR.mButtons[3]};
        // mControlInfo[(size_t)PhysicalControl::RHKey2] = ControlInfo{"RHKey2", &mRHMPR.mButtons[2]};
        // mControlInfo[(size_t)PhysicalControl::RHKey3] = ControlInfo{"RHKey3", &mRHMPR.mButtons[1]};
        // mControlInfo[(size_t)PhysicalControl::RHKey4] = ControlInfo{"RHKey4", &mRHMPR.mButtons[0]};
    }

    virtual size_t InputSource_GetControlCount() override
    {
        return SizeofStaticArray(mControlInfo);
    }
    virtual ControlInfo InputSource_GetControl(PhysicalControl index) override
    {
        CCASSERT((size_t)index < SizeofStaticArray(mControlInfo));
        return mControlInfo[(size_t)index];
    }

    virtual void InputSource_ShowToast(const String &s)
    {
        mDisplay->ShowToast(s);
    }

    CCEncoder<4 /* step increment each increment */, 14 /* pin #1 */, 15 /* pin #2 */> mEncoder;
    DigitalPinSwitchT<16> mOK;
    DigitalPinSwitchT<17> mBack;
    DigitalPinSwitchT<18> mButton3;
    DigitalPinSwitchT<19> mEncButton;
 
    ControlInfo mControlInfo[(size_t)PhysicalControl::COUNT];
};

} // namespace clarinoid
