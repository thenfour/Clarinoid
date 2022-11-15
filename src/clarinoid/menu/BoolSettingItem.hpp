#pragma once

#include <clarinoid/basic/Basic.hpp>
#include "MenuAppBase.hpp"
#include "MenuListControl.hpp"

namespace clarinoid
{

struct BoolSettingItem : public ISettingItem
{
    String mName;
    Property<bool> mBinding;
    String mTrueCaption;
    String mFalseCaption;
    Property<bool> mIsEnabled;

    BoolSettingItem(const String &name,
                    const String &trueCaption,
                    const String &falseCaption,
                    const Property<bool> &binding,
                    const Property<bool> &isEnabled)
        : mName(name), mBinding(binding), mTrueCaption(trueCaption), mFalseCaption(falseCaption), mIsEnabled(isEnabled)
    {
    }

    virtual String GetName(size_t multiIndex)
    {
        return mName;
    }
    virtual String GetValueString(size_t multiIndex)
    {
        return mBinding.GetValue() ? mTrueCaption : mFalseCaption;
    }
    virtual bool IsEnabled(size_t multiIndex) const
    {
        return mIsEnabled.GetValue();
    }
    virtual SettingItemType GetType(size_t multiIndex)
    {
        return SettingItemType::Bool;
    }
    virtual void ToggleBool(size_t multiIndex)
    {
        mBinding.SetValue(!mBinding.GetValue());
    }
};

struct MultiBoolSettingItem : public ISettingItem
{
    cc::function<size_t(void *)>::ptr_t mGetItemCount;
    cc::function<String(void *, size_t)>::ptr_t mLabelGetter;
    String mTrueCaption;
    String mFalseCaption;
    typename cc::function<bool(void *, size_t)>::ptr_t mValueGetter;
    typename cc::function<void(void *, size_t, const bool &val)>::ptr_t mValueSetter;
    cc::function<bool(void *, size_t)>::ptr_t mIsEnabled;
    void *mpCapture;

    MultiBoolSettingItem(cc::function<size_t(void *)>::ptr_t getItemCount,
                         cc::function<String(void *, size_t)>::ptr_t labelGetter,
                         const String &trueCaption,
                         const String &falseCaption,
                         typename cc::function<bool(void *, size_t multiIndex)>::ptr_t valueGetter,
                         typename cc::function<void(void *, size_t multiIndex, const bool &val)>::ptr_t valueSetter,
                         cc::function<bool(void *, size_t)>::ptr_t isEnabled,
                         void *capture)
        : mGetItemCount(getItemCount), //
          mLabelGetter(labelGetter),   //
          mTrueCaption(trueCaption),   //
          mFalseCaption(falseCaption), //
          mValueGetter(valueGetter),   //
          mValueSetter(valueSetter),   //
          mIsEnabled(isEnabled),       //
          mpCapture(capture)
    {
    }

    virtual size_t GetMultiCount()
    {
        return mGetItemCount(mpCapture);
    }
    virtual String GetName(size_t multiIndex)
    {
        return mLabelGetter(mpCapture, multiIndex);
    }
    virtual String GetValueString(size_t multiIndex)
    {
        bool b = mValueGetter(mpCapture, multiIndex);
        return b ? mTrueCaption : mFalseCaption;
    }
    virtual SettingItemType GetType(size_t multiIndex)
    {
        return SettingItemType::Bool;
    }
    virtual bool IsEnabled(size_t multiIndex) const
    {
        return mIsEnabled(mpCapture, multiIndex);
    }
    virtual void ToggleBool(size_t multiIndex)
    {
        bool b = mValueGetter(mpCapture, multiIndex);
        mValueSetter(mpCapture, multiIndex, !b);
    }
};

} // namespace clarinoid
