// think NumericSettingItem but for gain, in decibels.
// NOTE:
// - the value you're editing is a linear gain factor (1=unity, 0=-inf)
// - but the edit range spec is in decibels (0=unity, 6.04 = 2x etc.)

#pragma once

#include <clarinoid/basic/Basic.hpp>
#include "MenuSettings.hpp"
#include "MenuAppBase.hpp"
#include "MenuListControl.hpp"

namespace clarinoid
{

////////////////////////////////////////////////////////////////////////////////////////////////
struct GainEditor : ISettingItemEditor
{
    ISettingItemEditorActions *mpApi;
    NumericEditRangeSpecWithBottom mRangeInDb;

    Property<float> mBindingLinear;
    float mOldValLinear;

    GainEditor(const NumericEditRangeSpecWithBottom &rangeInDb_, const Property<float> &binding)
        : mRangeInDb(rangeInDb_), mBindingLinear(binding)
    {
    }

    virtual void DrawValue(float newLin, float oldLin)
    {
        float newDB = LinearToDecibels(newLin);
        float oldDB = LinearToDecibels(oldLin);

        if (newDB <= MIN_DECIBEL_GAIN)
        {
            this->mpApi->GetDisplay()->println(String("-inf db"));
        }
        else
        {
            this->mpApi->GetDisplay()->println(String("") + newDB + " db");
        }
        int deltaDb = (int)(newDB - oldDB);
        this->mpApi->GetDisplay()->println(String(" (") + (deltaDb >= 0 ? "+" : "") + deltaDb + " db)");

        this->mpApi->GetDisplay()->println("");
        this->mpApi->GetDisplay()->println(String("") + newLin + " lin");
    }

    virtual void SetupEditing(ISettingItemEditorActions *papi, int x, int y)
    {
        mpApi = papi;
        CCASSERT(!!papi);
        mOldValLinear = mBindingLinear.GetValue();
    }

    // for editing when holding back button. so back button is definitely pressed, and we don't want to bother
    // with encoder button pressed here.
    virtual void UpdateMomentaryMode(int encIntDelta)
    {
        float valLinear = mBindingLinear.GetValue();
        float valInDb = LinearToDecibels(valLinear);
        auto r = mRangeInDb.AdjustValue(valInDb,
                                        encIntDelta,
                                        mpApi->GetInputDelegator()->mModifierCourse.CurrentValue(),
                                        mpApi->GetInputDelegator()->mModifierFine.CurrentValue());
        if (r.first)
        {
            float adjValDb = r.second;
            mBindingLinear.SetValue(DecibelsToLinear(adjValDb));
        }
    }

    virtual void Update(bool backWasPressed, bool encWasPressed, int encIntDelta)
    {
        float valLinear = mBindingLinear.GetValue();
        float valInDb = LinearToDecibels(valLinear);
        auto r = mRangeInDb.AdjustValue(valInDb,
                                        encIntDelta,
                                        mpApi->GetInputDelegator()->mModifierCourse.CurrentValue(),
                                        mpApi->GetInputDelegator()->mModifierFine.CurrentValue());
        if (r.first)
        {
            float adjValDb = r.second;
            mBindingLinear.SetValue(DecibelsToLinear(adjValDb));
        }

        if (backWasPressed)
        {
            mBindingLinear.SetValue(mOldValLinear);
            mpApi->CommitEditing();
        }
        if (encWasPressed)
        {
            mpApi->CommitEditing();
        }
    }

    virtual void Render()
    {
        mpApi->GetDisplay()->SetupModal();
        DrawValue(mBindingLinear.GetValue(), mOldValLinear);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////
struct GainSettingItem : public ISettingItem
{
    String mName;
    GainEditor mEditor;
    Property<float> mBinding;
    Property<bool> mIsEnabled;

    GainSettingItem(const String &name,
                    const NumericEditRangeSpecWithBottom &range_,
                    const Property<float> &binding,
                    const Property<bool> &isEnabled)
        : mName(name), mEditor(range_, binding), mBinding(binding), mIsEnabled(isEnabled)
    {
    }

    virtual String GetName(size_t multiIndex) override
    {
        return mName;
    }
    virtual String GetValueString(size_t multiIndex)
    {
        float lin = mBinding.GetValue();
        float db = LinearToDecibels(lin);
        if (db <= MIN_DECIBEL_GAIN)
            return "-inf db";
        return String(db) + " db";
    }
    virtual SettingItemType GetType(size_t multiIndex)
    {
        return SettingItemType::Custom;
    }
    virtual bool IsEnabled(size_t multiIndex) const
    {
        return mIsEnabled.GetValue();
    }

    virtual ISettingItemEditor *GetEditor(size_t multiIndex)
    {
        return &mEditor;
    }
};

} // namespace clarinoid
