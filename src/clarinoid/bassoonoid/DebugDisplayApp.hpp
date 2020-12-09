#pragma once

#include <clarinoid/menu/MenuSettings.hpp>
#include "bsControlMapper.hpp"
#include "MusicalStateTask.hpp"

namespace clarinoid
{


struct PerformanceApp :
    SettingsMenuApp
{
    size_t mSelectedTaskID = 0;
    TaskPlanner* mTaskManager = nullptr;
    MusicalStateTask* mpMusicalStateTask = nullptr;
    BassoonoidControlMapper* mpControls = nullptr;
    virtual const char *DisplayAppGetName() override { return "Performance"; }

    PerformanceApp(CCDisplay& d, MusicalStateTask* pMusicalStateTask, BassoonoidControlMapper* controls) :
        SettingsMenuApp(d),
        mpMusicalStateTask(pMusicalStateTask),
        mpControls(controls)
    {}

    MultiSubmenuSettingItem mTiming;

    LabelSettingItem mInput = { [](void* cap)
    {
        PerformanceApp* pThis = (PerformanceApp*)cap;
        String ret = "M->Input:";
        ret += (int)pThis->mpMusicalStateTask->mInputTiming.GetValue();
        return ret;
    }, AlwaysEnabledWithCapture, this };

    LabelSettingItem mMusicalState = { [](void* cap)
    {
        PerformanceApp* pThis = (PerformanceApp*)cap;
        String ret = "M->Music:";
        ret += (int)pThis->mpMusicalStateTask->mMusicalStateTiming.GetValue();
        return ret;
    }, AlwaysEnabledWithCapture, this };

    LabelSettingItem mSynthState = { [](void* cap)
    {
        PerformanceApp* pThis = (PerformanceApp*)cap;
        String ret = "M->Synth:";
        ret += (int)pThis->mpMusicalStateTask->mSynthStateTiming.GetValue();
        return ret;
    }, AlwaysEnabledWithCapture, this };



    void Init(TaskPlanner* tm)
    {
        mTaskManager = tm;

        mTiming.Init(
            [](void* cap) {  // get item count
                PerformanceApp* pThis = (PerformanceApp*)cap;
                return pThis->mTaskManager->mTasks.size();
            },
            [](void* cap, size_t i) // get name
            {
                PerformanceApp* pThis = (PerformanceApp*)cap;
                auto& t = pThis->mTaskManager->mTasks[i];
                String ret = String(t.mInfo.mName) + ":" + (int)t.mInfo.mExecutionTimeMicros.GetValue();
                //Serial.println(ret);
                return ret;
            },
            [](void* cap, size_t i) // get submenu
            {
                PerformanceApp* pThis = (PerformanceApp*)cap;
                pThis->mSelectedTaskID = i;
                return &pThis->mSubitemList;
            },
            [](void* cap, size_t i) { return true; },
            this
            );
    }

    LabelSettingItem mSubitemTmp = { [](void* cap)
    {
        //PerformanceApp* pThis = (PerformanceApp*)cap;
        return String("tmp.");
    }, AlwaysEnabledWithCapture, this };

    ISettingItem* mSubitemArray[1] =
    {
        &mSubitemTmp,
    };
    SettingsList mSubitemList = { mSubitemArray };


    ISettingItem* mArray[4] =
    {
        &mTiming,
        &mInput,
        &mMusicalState,
        &mSynthState,
    };
    SettingsList mRootList = { mArray };
    virtual SettingsList* GetRootSettingsList()
    {
        return &mRootList;
    }

    virtual void RenderFrontPage() 
    {
        mDisplay.mDisplay.println(String("PERFormance >"));
        mDisplay.mDisplay.println(String(" McpL: ") + (int)mpControls->mTimingMcpL.ElapsedMicros());
        mDisplay.mDisplay.println(String(" McpR: ") + (int)mpControls->mTimingMcpR.ElapsedMicros());
        mDisplay.mDisplay.println(String(" Breath: ") + (int)mpControls->mTimingBreath.ElapsedMicros());
        mDisplay.mDisplay.println(String(" Encoders: ") + (int)mpControls->mTimingEncoders.ElapsedMicros());
        mDisplay.mDisplay.println(String(" Analog: ") + (int)mpControls->mTimingAnalog.ElapsedMicros());
        mDisplay.mDisplay.println(String(" Digital: ") + (int)mpControls->mTimingDigital.ElapsedMicros());

        SettingsMenuApp::RenderFrontPage();
    }
};


/////////////////////////////////////////////////////////////////////////////////////////////////
struct DebugDisplayApp :
    SettingsMenuApp
{
    virtual const char *DisplayAppGetName() override { return "DebugDisplayApp"; }

    BassoonoidControlMapper& mControls;
    MusicalStateTask& mMusicalStateTask;

    DebugDisplayApp(CCDisplay& d, BassoonoidControlMapper& c, MusicalStateTask& mst) :
        SettingsMenuApp(d),
        mControls(c),
        mMusicalStateTask(mst)
    {}

    LabelSettingItem mBreath = { [](void* cap)
    {
        DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
        return (String)((String("Breath: ") + int(pThis->mControls.mBreath.CurrentValue01() * 1000)));
    }, AlwaysEnabledWithCapture, this };

    LabelSettingItem mLHA = { [](void* cap)
    {
        DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
        String ret = "LHA:";
        for (size_t i = 0; i < 8; ++ i)
        {
            ret += pThis->mControls.mLHMCP.mButtons[i].CurrentValue() ? (String("") + i) : String(" ");
        }
        return ret;
    }, AlwaysEnabledWithCapture, this };

    LabelSettingItem mLHB = { [](void* cap)
    {
        DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
        String ret = "LHB:";
        for (size_t i = 8; i < 16; ++ i)
        {
            ret += pThis->mControls.mLHMCP.mButtons[i].CurrentValue() ? (String("") + i) : String(" ");
        }
        return ret;
    }, AlwaysEnabledWithCapture, this };

    LabelSettingItem mRHA = { [](void* cap)
    {
        DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
        String ret = "RHA:";
        for (size_t i = 0; i < 8; ++ i)
        {
            ret += pThis->mControls.mRHMCP.mButtons[i].CurrentValue() ? (String("") + i) : String(" ");
        }
        return ret;
    }, AlwaysEnabledWithCapture, this };

    LabelSettingItem mRHB = { [](void* cap)
    {
        DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
        String ret = "RHB:";
        for (size_t i = 8; i < 16; ++ i)
        {
            ret += pThis->mControls.mRHMCP.mButtons[i].CurrentValue() ? (String("") + i) : String(" ");
        }
        return ret;
    }, AlwaysEnabledWithCapture, this };

    LabelSettingItem mCPEnc = { [](void* cap)
    {
        DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
        String ret = String("CP Encoder raw:") + pThis->mControls.mCPEncoder.CurrentValue().RawValue;
        return ret;
    }, AlwaysEnabledWithCapture, this };

    LabelSettingItem mLHEnc = { [](void* cap)
    {
        DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
        String ret = String("LH Encoder raw:") + pThis->mControls.mLHEncoder.CurrentValue().RawValue;
        return ret;
    }, AlwaysEnabledWithCapture, this };

    LabelSettingItem mRHEnc = { [](void* cap)
    {
        DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
        String ret = String("RH Encoder raw:") + pThis->mControls.mRHEncoder.CurrentValue().RawValue;
        return ret;
    }, AlwaysEnabledWithCapture, this };

    LabelSettingItem mToggleUp = { [](void* cap)
    {
        DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
        String ret = String("Toggle up:") + (pThis->mControls.mToggleUp.CurrentValue() ? "1" : "0");
        return ret;
    }, AlwaysEnabledWithCapture, this };

    LabelSettingItem mSynthPoly = { [](void* cap)
    {
        DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
        String ret = String("Synth poly:") + (pThis->mMusicalStateTask.mSynth.mCurrentPolyphony);
        return ret;
    }, AlwaysEnabledWithCapture, this };

// AudioProcessorUsage();
// AudioProcessorUsageMax();
// AudioMemoryUsage();
// AudioMemoryUsageMax();

    LabelSettingItem mAudioProcessorUsage = { [](void* cap)
    {
        //DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
        String ret = String("Audio CPU %:") + AudioProcessorUsage();
        return ret;
    }, AlwaysEnabledWithCapture, this };

    LabelSettingItem mAudioProcessorUsageMax = { [](void* cap)
    {
        //DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
        String ret = String("Audio max CPU %:") + AudioProcessorUsageMax();
        return ret;
    }, AlwaysEnabledWithCapture, this };

    LabelSettingItem mAudioMemoryUsage = { [](void* cap)
    {
        //DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
        String ret = String("Audio mem:") + AudioMemoryUsage();
        return ret;
    }, AlwaysEnabledWithCapture, this };

    LabelSettingItem mAudioMemoryUsageMax = { [](void* cap)
    {
        //DebugDisplayApp* pThis = (DebugDisplayApp*)cap;
        String ret = String("Audio mem max:") + AudioMemoryUsageMax();
        return ret;
    }, AlwaysEnabledWithCapture, this };

    ISettingItem* mArray[14] =
    {
        &mBreath,
        &mLHA,
        &mLHB,
        &mRHA,
        &mRHB,
        &mCPEnc,
        &mLHEnc,
        &mRHEnc,
        &mToggleUp,
        &mSynthPoly,
        &mAudioProcessorUsage,
        &mAudioProcessorUsageMax,
        &mAudioMemoryUsage,
        &mAudioMemoryUsageMax,
        // joyx
        // joyy
        // pitch
        // volume
    };
    SettingsList mRootList = { mArray };

    virtual SettingsList* GetRootSettingsList()
    {
        return &mRootList;
    }

    virtual void RenderFrontPage() 
    {
        mDisplay.mDisplay.println(String("Debug info ->"));
        SettingsMenuApp::RenderFrontPage();
    }
};

} // namespace clarinoid
