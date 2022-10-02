
#pragma once

#include "clarinoid/basic/Basic.hpp"
#include "clarinoid/application/ControlMapper.hpp"

#include "clarinoid/components/PCA9554.hpp"
#include "clarinoid/components/Encoder.hpp"
#include "clarinoid/components/Switch.hpp"
#include "clarinoid/components/HoneywellABPI2C.hpp"

namespace clarinoid
{

struct BommanoidControlMapper : IInputSource, ITask
{
    //TimeSpan mTimingMcpR;
    //TimeSpan mTimingMcpL;
    //TimeSpan mTimingBreath;
    //TimeSpan mTimingEncoders;
    //TimeSpan mTimingAnalog;
    //TimeSpan mTimingDigital;

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
        // Stopwatch sw;
        // mCPEncoder.Update();
        // mRHEncoder.Update();
        // mLHEncoder.Update(); // reading encoders is dirt cheap. 1 microsecond.
        // mTimingEncoders = sw.ElapsedTime();

        // sw.Restart();
        // mRHMCP.Update();
        // mTimingMcpR = sw.ElapsedTime();

        // sw.Restart();
        // mLHMCP.Update();
        // mTimingMcpL = sw.ElapsedTime();

        // sw.Restart();
        // mBreath.Update();
        // mTimingBreath = sw.ElapsedTime();

        // sw.Restart();
        // mVolumePot.Update();
        // mPitchStrip.Update();
        // mJoyX.Update();
        // mJoyY.Update();
        // mTimingAnalog = sw.ElapsedTime();

        // sw.Restart();
        // mToggleUp.Update();
        // mTimingDigital = sw.ElapsedTime();
    }

    // set up the mControlInfo, mapping enum to structured control interface
    virtual void InputSource_Init(struct InputDelegator *p) override
    {
        mControlInfo[(size_t)PhysicalControl::Enc] = ControlInfo{"Enc", &mEncoder};
        mControlInfo[(size_t)PhysicalControl::Back] = ControlInfo{"Back", &mPca.mButtons[0]};
        mControlInfo[(size_t)PhysicalControl::Ok] = ControlInfo{"Ok", &mPca.mButtons[1]};
         mControlInfo[(size_t)PhysicalControl::x1] = ControlInfo{"x1", &mPca.mButtons[2]}; // fine
         mControlInfo[(size_t)PhysicalControl::x2] = ControlInfo{"x2", &mPca.mButtons[3]}; // course

        // mControlInfo[(size_t)PhysicalControl::LHEnc] = ControlInfo{"LHENC", &mLHEncoder};
        // mControlInfo[(size_t)PhysicalControl::RHEnc] = ControlInfo{"RHENC", &mRHEncoder};

        // mControlInfo[(size_t)PhysicalControl::Breath] = ControlInfo{"Breath", &mBreath};

        // mControlInfo[(size_t)PhysicalControl::CPBack] = ControlInfo{"CPBack", &mRHMCP.mButtons[0]};
        // mControlInfo[(size_t)PhysicalControl::CPOk] = ControlInfo{"CPOk", &mRHMCP.mButtons[1]};
        // mControlInfo[(size_t)PhysicalControl::CPToggleUp] = ControlInfo{"CPToggleUp", &mToggleUp};
        // mControlInfo[(size_t)PhysicalControl::CPEncButton] = ControlInfo{"CPEncBtn", &mRHMCP.mButtons[2]};
        // mControlInfo[(size_t)PhysicalControl::LHx1] = ControlInfo{"LHx1", &mLHMCP.mButtons[0]};
        // mControlInfo[(size_t)PhysicalControl::LHx2] = ControlInfo{"LHx2", &mLHMCP.mButtons[1]};
        // mControlInfo[(size_t)PhysicalControl::LHx3] = ControlInfo{"LHx3", &mLHMCP.mButtons[2]};
        // mControlInfo[(size_t)PhysicalControl::LHx4] = ControlInfo{"LHx4", &mLHMCP.mButtons[3]};
        // mControlInfo[(size_t)PhysicalControl::LHEncButton] = ControlInfo{"LHEncBtn", &mLHMCP.mButtons[13]};
        // mControlInfo[(size_t)PhysicalControl::LHBack] = ControlInfo{"LHBack", &mLHMCP.mButtons[11]};
        // mControlInfo[(size_t)PhysicalControl::LHOk] = ControlInfo{"LHOk", &mLHMCP.mButtons[12]};
        // mControlInfo[(size_t)PhysicalControl::LHThx1] =
        //     ControlInfo{"LHThx1", &mLHMCP.mButtons[14]}; // transpose+ -> synthpreset+
        // mControlInfo[(size_t)PhysicalControl::LHThx2] =
        //     ControlInfo{"LHThx2", &mLHMCP.mButtons[15]}; // transpose- -> synthpreset -
        // mControlInfo[(size_t)PhysicalControl::LHOct1] = ControlInfo{"LHO1", &mLHMCP.mButtons[10]};
        // mControlInfo[(size_t)PhysicalControl::LHOct2] = ControlInfo{"LHO2", &mLHMCP.mButtons[9]};
        // mControlInfo[(size_t)PhysicalControl::LHOct3] = ControlInfo{"LHO3", &mLHMCP.mButtons[8]};
        // mControlInfo[(size_t)PhysicalControl::LHKey1] = ControlInfo{"LHKey1", &mLHMCP.mButtons[7]};
        // mControlInfo[(size_t)PhysicalControl::LHKey2] = ControlInfo{"LHKey2", &mLHMCP.mButtons[6]};
        // mControlInfo[(size_t)PhysicalControl::LHKey3] = ControlInfo{"LHKey3", &mLHMCP.mButtons[5]};
        // mControlInfo[(size_t)PhysicalControl::LHKey4] = ControlInfo{"LHKey4", &mLHMCP.mButtons[4]};

        // mControlInfo[(size_t)PhysicalControl::RHTh1] =
        //     ControlInfo{"RHTh1", &mRHMCP.mButtons[5]}; // base note hold   -> NOP
        // mControlInfo[(size_t)PhysicalControl::RHTh2] =
        //     ControlInfo{"RHTh2", &mRHMCP.mButtons[6]}; // synth preset -   -> NOP
        // mControlInfo[(size_t)PhysicalControl::RHTh3] =
        //     ControlInfo{"RHTh3", &mRHMCP.mButtons[7]}; // synth preset +   -> base note hold
        // mControlInfo[(size_t)PhysicalControl::RHx1] = ControlInfo{"RHx1", &mRHMCP.mButtons[4]};
        // mControlInfo[(size_t)PhysicalControl::RHx4] = ControlInfo{"RHx4", &mRHMCP.mButtons[14]}; // loop stop
        // mControlInfo[(size_t)PhysicalControl::RHx5] = ControlInfo{"RHx5", &mRHMCP.mButtons[15]}; // loop go
        // mControlInfo[(size_t)PhysicalControl::RHKey1] = ControlInfo{"RHKey1", &mRHMCP.mButtons[8]};
        // mControlInfo[(size_t)PhysicalControl::RHKey2] = ControlInfo{"RHKey2", &mRHMCP.mButtons[9]};
        // mControlInfo[(size_t)PhysicalControl::RHKey3] = ControlInfo{"RHKey3", &mRHMCP.mButtons[10]};
        // mControlInfo[(size_t)PhysicalControl::RHKey4] = ControlInfo{"RHKey4", &mRHMCP.mButtons[11]};
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

    CCEncoder<4, 26, 9> mEncoder;
    PCA9554 mPca = PCA9554 { Wire, 0x38 };

    ControlInfo mControlInfo[(size_t)PhysicalControl::COUNT];
};

} // namespace clarinoid