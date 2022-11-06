
#pragma once

#include "clarinoid/basic/Basic.hpp"
#include "clarinoid/application/ControlMapper.hpp"

#include "clarinoid/components/PCA9554.hpp"
#include "clarinoid/components/Encoder.hpp"
#include "clarinoid/components/Switch.hpp"
#include "clarinoid/components/HoneywellABPI2C.hpp"
#include "clarinoid/components/ADS1115.hpp"

namespace clarinoid
{

struct BommanoidControlMapper : IInputSource, ITask
{
    bool mFirstTaskUpdate = true;
    IDisplay *mDisplay = nullptr;

    void Init(IDisplay *display)
    {
        mDisplay = display;
    }

    // Task manager task; basically an "Update()" sort of fn to update internal state.
    virtual void TaskRun() override
    {
        NoInterrupts _ni;
        mEncoder.Update();
        mPca.Update();
        if (mFirstTaskUpdate) {
            mADS1115.Begin();
        }
        mADS1115.Update();

        // sw.Restart();
        // mVolumePot.Update();
        // mPitchStrip.Update();
        // mJoyX.Update();
        // mJoyY.Update();

        mFirstTaskUpdate = false;
    }

    // set up the mControlInfo, mapping enum to structured control interface
    virtual void InputSource_Init(struct InputDelegator *p) override
    {
        mControlInfo[(size_t)PhysicalControl::Enc] = ControlInfo{"Enc", &mEncoder};
        mControlInfo[(size_t)PhysicalControl::EncBtn] = ControlInfo{"EncBtn", &mEncoder};
        mControlInfo[(size_t)PhysicalControl::L1] = ControlInfo{"L1", &mPca.mButtons[3]};
        mControlInfo[(size_t)PhysicalControl::L2] = ControlInfo{"L2", &mPca.mButtons[2]};
        mControlInfo[(size_t)PhysicalControl::L3] = ControlInfo{"L3", &mPca.mButtons[1]};
        mControlInfo[(size_t)PhysicalControl::L4] = ControlInfo{"L4", &mPca.mButtons[0]};
        mControlInfo[(size_t)PhysicalControl::R1] = ControlInfo{"R1", &mPca.mButtons[5]};
        mControlInfo[(size_t)PhysicalControl::R2] = ControlInfo{"R2", &mPca.mButtons[4]};
        mControlInfo[(size_t)PhysicalControl::Pedal] = ControlInfo{"Pedal", &mPca.mButtons[4]};

        mControlInfo[(size_t)PhysicalControl::Pot1] = ControlInfo{"Pot1", &mADS1115.mAnalogControls[0]};
        mControlInfo[(size_t)PhysicalControl::Pot2] = ControlInfo{"Pot2", &mADS1115.mAnalogControls[1]};
        mControlInfo[(size_t)PhysicalControl::Pot3] = ControlInfo{"Pot3", &mADS1115.mAnalogControls[2]};
        mControlInfo[(size_t)PhysicalControl::Pot4] = ControlInfo{"Pot4", &mADS1115.mAnalogControls[3]};
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

    CCEncoder<1, 30, 31> mEncoder;
    PCA9554 mPca = PCA9554{Wire, 0x38};
    ADS1115Device mADS1115 {Wire, 0x48};
    ControlInfo mControlInfo[(size_t)PhysicalControl::COUNT];
};

} // namespace clarinoid
