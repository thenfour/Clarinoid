#pragma once

#include "ControlSource.hpp"
#include "FunctionHandler.hpp"
#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>

namespace clarinoid
{

struct SynthPresetAMappableFunction : FunctionHandler
{
    IInputSource *mInputSrc;
    AppSettings *mAppSettings;

    void Init(AppSettings *appSettings, IInputSource *psrc)
    {
        mInputSrc = psrc;
        mAppSettings = appSettings;
    }

    virtual void FunctionHandler_Update(const ControlValue &v) override
    {
        int nv = v.AsRoundedInt();
        nv = RotateIntoRange(nv, SYNTH_PRESET_COUNT);
        int old = mAppSettings->GetCurrentPerformancePatch().mSynthPatchA.GetValue();
        if (old != nv)
        {
            auto &p = mAppSettings->FindSynthPreset(nv);
            mInputSrc->InputSource_ShowToast(String("Synth patch A: ") + nv + " (" + (nv - old) + ")\r\n" + p.mName.GetValue());
            mAppSettings->GetCurrentPerformancePatch().mSynthPatchA.SetValue(nv);
        }
    }
    virtual ControlValue FunctionHandler_GetCurrentValue() const override
    {
        int v = mAppSettings->GetCurrentPerformancePatch().mSynthPatchA.GetValue();
        return ControlValue::IntValue(v);
    }
};

struct SynthPresetBMappableFunction : FunctionHandler
{
    IInputSource *mInputSrc = nullptr;
    AppSettings *mAppSettings = nullptr;

    void Init(AppSettings *appSettings, IInputSource *psrc)
    {
        CCASSERT(!!psrc);
        CCASSERT(!!appSettings);
        mInputSrc = psrc;
        mAppSettings = appSettings;
    }

    virtual void FunctionHandler_Update(const ControlValue &v) override
    {
        CCASSERT(!!mAppSettings);
        int nv = v.AsRoundedInt();
        if (nv < -1)
        {
            nv = SYNTH_PRESET_COUNT - 1;
        }
        if (nv >= (int)SYNTH_PRESET_COUNT)
        {
            nv = -1;
        }
        int old = mAppSettings->GetCurrentPerformancePatch().mSynthPatchB.GetValue();
        if (old != nv)
        {
            // auto &p = mAppSettings->FindSynthPreset(nv);
            auto name = mAppSettings->GetSynthPatchName(nv);
            mInputSrc->InputSource_ShowToast(String("Synth patch B: ") + nv + " (" + (nv - old) + ")\r\n" + name);
            mAppSettings->GetCurrentPerformancePatch().mSynthPatchB.SetValue(nv);
        }
    }
    virtual ControlValue FunctionHandler_GetCurrentValue() const override
    {
        int v = mAppSettings->GetCurrentPerformancePatch().mSynthPatchB.GetValue();
        return ControlValue::IntValue(v);
    }
};

struct HarmPresetMappableFunction : FunctionHandler
{
    IInputSource *mInputSrc;
    AppSettings *mAppSettings;

    void Init(AppSettings *appSettings, IInputSource *psrc)
    {
        mInputSrc = psrc;
        mAppSettings = appSettings;
    }

    virtual void FunctionHandler_Update(const ControlValue &v) override
    {
        int nv = v.AsRoundedInt();
        nv = RotateIntoRange(nv, HARM_PRESET_COUNT);
        int old = mAppSettings->GetCurrentPerformancePatch().mHarmPreset.GetValue();
        if (old != nv)
        {
            auto &p = mAppSettings->FindHarmPreset(nv);
            mInputSrc->InputSource_ShowToast(String("Harmonizer: ") + nv + " (" + (nv - old) + ")\r\n" + p.mName.GetValue());
            mAppSettings->GetCurrentPerformancePatch().mHarmPreset.SetValue(nv);
        }
    }
    virtual ControlValue FunctionHandler_GetCurrentValue() const override
    {
        int v = mAppSettings->GetCurrentPerformancePatch().mHarmPreset.GetValue();
        return ControlValue::IntValue(v);
    }
};

struct PerfPresetMappableFunction : FunctionHandler
{
    IInputSource *mInputSrc = nullptr;
    AppSettings *mAppSettings = nullptr;

    void Init(AppSettings *appSettings, IInputSource *psrc)
    {
        CCASSERT(!!psrc);
        CCASSERT(!!appSettings);
        mInputSrc = psrc;
        mAppSettings = appSettings;
    }

    virtual void FunctionHandler_Update(const ControlValue &v) override
    {
        CCASSERT(!!mAppSettings); // make sure Init() is called!
        int nv = v.AsRoundedInt();
        nv = RotateIntoRange(nv, PERFORMANCE_PATCH_COUNT);
        int old = mAppSettings->mCurrentPerformancePatch.GetValue();
        if (old != nv)
        {
            auto name = mAppSettings->GetPerfPatchName(nv);
            mInputSrc->InputSource_ShowToast(String("Perf: ") + nv + " (" + (nv - old) + ")\r\n" + name);
            mAppSettings->mCurrentPerformancePatch.SetValue(nv);
        }
    }
    virtual ControlValue FunctionHandler_GetCurrentValue() const override
    {
        int v = mAppSettings->mCurrentPerformancePatch.GetValue();
        v = RotateIntoRange(v, PERFORMANCE_PATCH_COUNT);
        return ControlValue::IntValue(v);
    }
};

struct TransposeMappableFunction : FunctionHandler
{
    IInputSource *mInputSrc;
    AppSettings *mAppSettings;

    void Init(AppSettings *appSettings, IInputSource *psrc)
    {
        mInputSrc = psrc;
        mAppSettings = appSettings;
    }

    virtual void FunctionHandler_Update(const ControlValue &v) override
    {
        int nv = v.AsRoundedInt();
        if (nv < -48)
        {
            mInputSrc->InputSource_ShowToast(String("Transposition\r\ntoo low"));
            return;
        }
        if (nv > 48)
        {
            mInputSrc->InputSource_ShowToast(String("Transposition\r\ntoo high"));
            return;
        }
        int old = mAppSettings->GetCurrentPerformancePatch().mTranspose.GetValue();
        if (old != nv)
        {
            mInputSrc->InputSource_ShowToast(String("Transpose: ") + nv + " (" + (nv - old) + ")");
            mAppSettings->GetCurrentPerformancePatch().mTranspose.SetValue(nv);
        }
    }
    virtual ControlValue FunctionHandler_GetCurrentValue() const override
    {
        return ControlValue::IntValue(mAppSettings->GetCurrentPerformancePatch().mTranspose.GetValue());
    }
};

struct InputDelegator
{
    AppSettings *mpAppSettings = nullptr;
    IInputSource *mpSrc = nullptr;

    FunctionHandler *mHandlers[(size_t)ControlMapping::Function::COUNT] = {nullptr};

    // for convenience, we can put very-common composite inputs right here. avoids
    // having to pass around stuff everywhere in the app like breath.
    VirtualSwitch mMenuBack;
    VirtualSwitch mMenuOK;
    VirtualEncoder mMenuScrollA;

    VirtualSwitch mKeyLH1;
    VirtualSwitch mKeyLH2;
    VirtualSwitch mKeyLH3;
    VirtualSwitch mKeyLH4;

    VirtualSwitch mKeyRH1;
    VirtualSwitch mKeyRH2;
    VirtualSwitch mKeyRH3;
    VirtualSwitch mKeyRH4;

    VirtualSwitch mKeyOct1;
    VirtualSwitch mKeyOct2;
    VirtualSwitch mKeyOct3;

    VirtualSwitch mKeyOct4;
    VirtualSwitch mKeyOct5;
    VirtualSwitch mKeyOct6;

    VirtualAxis mBreath;
    VirtualAxis mPitchBend;
    VirtualAxis mSustainPedal;

    VirtualSwitch mModifierFine;
    VirtualSwitch mModifierCourse;
    VirtualSwitch mModifierSynth;
    VirtualSwitch mModifierPerf;
    VirtualSwitch mModifierHarm;
    VirtualSwitch mModifierShift;

    VirtualAxis mMacroPots[4];

    SynthPresetAMappableFunction mSynthPresetAFn;
    SynthPresetBMappableFunction mSynthPresetBFn;
    HarmPresetMappableFunction mHarmPresetFn;
    TransposeMappableFunction mTransposeFn;
    PerfPresetMappableFunction mPerfPresetFn;

    VirtualSwitch mLoopStopButton;
    VirtualSwitch mLoopGoButton;

    VirtualSwitch mBaseNoteHoldToggle;
    VirtualSwitch mMetronomeLEDToggle;
    VirtualSwitch mHarmPresetOnOffToggle;
    VirtualSwitch mDisplayFontToggle;

    void Init(AppSettings *appSettings, IInputSource *psrc)
    {
        mpAppSettings = appSettings;
        mpSrc = psrc;

        mSynthPresetAFn.Init(appSettings, psrc);
        mSynthPresetBFn.Init(appSettings, psrc);
        mHarmPresetFn.Init(appSettings, psrc);
        mTransposeFn.Init(appSettings, psrc);
        mPerfPresetFn.Init(appSettings, psrc);

        RegisterFunction(ControlMapping::Function::Nop,
                         &mMenuBack); // anything works; it's never called.

        RegisterFunction(ControlMapping::Function::ModifierCourse, &mModifierCourse);
        RegisterFunction(ControlMapping::Function::ModifierFine, &mModifierFine);
        RegisterFunction(ControlMapping::Function::ModifierSynth, &mModifierSynth);
        RegisterFunction(ControlMapping::Function::ModifierHarm, &mModifierHarm);
        RegisterFunction(ControlMapping::Function::ModifierPerf, &mModifierPerf);
        RegisterFunction(ControlMapping::Function::ModifierShift, &mModifierShift);

        RegisterFunction(ControlMapping::Function::MenuBack, &mMenuBack);
        RegisterFunction(ControlMapping::Function::MenuOK, &mMenuOK);
        RegisterFunction(ControlMapping::Function::MenuScrollA, &mMenuScrollA);

        RegisterFunction(ControlMapping::Function::LH1, &mKeyLH1);
        RegisterFunction(ControlMapping::Function::LH2, &mKeyLH2);
        RegisterFunction(ControlMapping::Function::LH3, &mKeyLH3);
        RegisterFunction(ControlMapping::Function::LH4, &mKeyLH4);

        RegisterFunction(ControlMapping::Function::RH1, &mKeyRH1);
        RegisterFunction(ControlMapping::Function::RH2, &mKeyRH2);
        RegisterFunction(ControlMapping::Function::RH3, &mKeyRH3);
        RegisterFunction(ControlMapping::Function::RH4, &mKeyRH4);

        RegisterFunction(ControlMapping::Function::Oct1, &mKeyOct1);
        RegisterFunction(ControlMapping::Function::Oct2, &mKeyOct2);
        RegisterFunction(ControlMapping::Function::Oct3, &mKeyOct3);
        RegisterFunction(ControlMapping::Function::Oct4, &mKeyOct4);
        RegisterFunction(ControlMapping::Function::Oct5, &mKeyOct5);
        RegisterFunction(ControlMapping::Function::Oct6, &mKeyOct6);

        RegisterFunction(ControlMapping::Function::Breath, &mBreath);
        RegisterFunction(ControlMapping::Function::PitchBend, &mPitchBend);

        RegisterFunction(ControlMapping::Function::MacroPot1, &mMacroPots[0]);
        RegisterFunction(ControlMapping::Function::MacroPot2, &mMacroPots[1]);
        RegisterFunction(ControlMapping::Function::MacroPot3, &mMacroPots[2]);
        RegisterFunction(ControlMapping::Function::MacroPot4, &mMacroPots[3]);

        RegisterFunction(ControlMapping::Function::SustainPedal, &mSustainPedal);

        RegisterFunction(ControlMapping::Function::SynthPresetA, &mSynthPresetAFn);
        RegisterFunction(ControlMapping::Function::SynthPresetB, &mSynthPresetBFn);
        RegisterFunction(ControlMapping::Function::HarmPreset, &mHarmPresetFn);
        RegisterFunction(ControlMapping::Function::Transpose, &mTransposeFn);
        RegisterFunction(ControlMapping::Function::PerfPreset, &mPerfPresetFn);

        RegisterFunction(ControlMapping::Function::LoopStop, &mLoopStopButton);
        RegisterFunction(ControlMapping::Function::LoopGo, &mLoopGoButton);

        RegisterFunction(ControlMapping::Function::BaseNoteHoldToggle, &mBaseNoteHoldToggle);
        RegisterFunction(ControlMapping::Function::MetronomeLEDToggle, &mMetronomeLEDToggle);
        RegisterFunction(ControlMapping::Function::HarmPresetOnOffToggle, &mHarmPresetOnOffToggle);

        RegisterFunction(ControlMapping::Function::DisplayFontToggle, &mDisplayFontToggle);

        mpSrc->InputSource_Init(this);
    }

    bool MatchesModifierKeys(const ControlMapping &m)
    {
        switch (m.mModifier)
        {
        case ModifierKey::None: // = 0, // requires no modifiers are pressed.
            return !mModifierFine.CurrentValue() && !mModifierCourse.CurrentValue() && !mModifierSynth.CurrentValue() &&
                   !mModifierHarm.CurrentValue() && !mModifierPerf.CurrentValue() && !mModifierShift.CurrentValue();
        case ModifierKey::Fine: // = 1,
            return mModifierFine.CurrentValue();
        case ModifierKey::Course: // = 2,
            return mModifierCourse.CurrentValue();
        case ModifierKey::Synth: // = 2,
            return mModifierSynth.CurrentValue();
        case ModifierKey::Harm: // = 2,
            return mModifierHarm.CurrentValue();
        case ModifierKey::Perf: // = 2,
            return mModifierPerf.CurrentValue();
        case ModifierKey::Shift: // = 2,
            return mModifierShift.CurrentValue();
        default:
        case ModifierKey::Any: // = 128, // special; any combination works.
            return true;
        }
    }

    // for test code.
    void ResetModifiers()
    {
        mModifierCourse.mValue = false;
        mModifierFine.mValue = false;
        mModifierSynth.mValue = false;
        mModifierHarm.mValue = false;
        mModifierPerf.mValue = false;
        mModifierShift.mValue = false;
    }

    // process all input state and delegate to handlers.
    void Update()
    {
        CCASSERT(!!mpSrc);
        CCASSERT(!!this->mpAppSettings);

        // one control may be mapped to many functions.
        // one function may be controlled by many controls.
        // this aggregation is done by the FunctionHandler.
        size_t functionCount = (size_t)ControlMapping::Function::COUNT;
        for (size_t i = 0; i < functionCount; ++i)
        {
            // CCASSERT(mHandlers[i] != nullptr);
            if (!mHandlers[i])
                continue;
            mHandlers[i]->BeginUpdate();
        }

        for (size_t i = 0; i < SizeofStaticArray(mpAppSettings->mControlMappings); ++i)
        {
            auto &mapping = mpAppSettings->mControlMappings[i];
            if (mapping.mFunction == ControlMapping::Function::Nop)
                continue;
            if (!MatchesModifierKeys(mapping))
                continue;
            if ((size_t)mapping.mFunction >= SizeofStaticArray(mHandlers)) // maybe it's just not registered yet?
                continue;

            FunctionHandler *dest = mHandlers[(size_t)mapping.mFunction]; // get the function this is
                                                                          // mapped to
            if (!!dest)
            {
                auto src = mpSrc->InputSource_GetControl(
                    mapping.mSource); // and the source control providing the value to map.
                dest->Update(mapping, src);
            }
        }

        // tell all destinations we completed.
        for (size_t i = 0; i < functionCount; ++i)
        {
            if (!mHandlers[i])
                continue;
            mHandlers[i]->EndUpdate();
        }
    }

    void RegisterFunction(ControlMapping::Function f, FunctionHandler *handler)
    {
        CCASSERT((size_t)f < SizeofStaticArray(mHandlers)); // bounds check
        CCASSERT(mHandlers[(size_t)f] == nullptr);          // do not register a handler more than once.
        mHandlers[(size_t)f] = handler;
    }
};

} // namespace clarinoid
