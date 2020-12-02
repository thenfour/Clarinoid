#pragma once

#include <clarinoid/menu/MenuSettings.hpp>
#include "bsControlMapper.hpp"
#include "MusicalStateTask.hpp"

namespace clarinoid
{

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

    ISettingItem* mArray[10] =
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
        mDisplay.mDisplay.println(String("Debug..."));
        SettingsMenuApp::RenderFrontPage();
    }
};

} // namespace clarinoid
