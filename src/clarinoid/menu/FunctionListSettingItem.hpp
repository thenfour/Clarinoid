

// for "Copy Preset..." style functions,
// this takes a # of items, and callbacks for display; when user selects, then a function is called.

#pragma once

#include <clarinoid/basic/Basic.hpp>
#include "MenuAppBase.hpp"
#include "MenuListControl.hpp"

namespace clarinoid
{

struct FunctionListEditor : ISettingItemEditor
{
    struct ListProxy : IList
    {
        size_t mItemCount;
        cc::function<String(void *, size_t)>::ptr_t mItemNameGetter;
        void *mCap;

        virtual int List_GetItemCount() const override
        {
            return mItemCount;
        }
        virtual String List_GetItemCaption(int i) const override
        {
            return mItemNameGetter(mCap, (size_t)i);
        }
    };

    // set on SetupEditing()
    ISettingItemEditorActions *mpApi;
    ListProxy mList;

    int mListSelectedItem = 0;
    // Property<int> mListSelectedItem = {
    //   [](void* capture) { EnumEditor<T>* pthis = (EnumEditor<T>*)capture; return (int)pthis->mBinding.GetValue(); },
    //   [](void* capture, const int& val) { EnumEditor<T>* pthis = (EnumEditor<T>*)capture;
    //   pthis->mBinding.SetValue((T)val); }, this
    // };

    ListControl mListControl;

    // cc::function<String(void*,size_t)>::ptr_t mItemNameGetter;
    cc::function<void(void *, size_t)>::ptr_t mOnClick;
    void *mCap;

    FunctionListEditor(size_t itemCount,
                       cc::function<String(void *, size_t)>::ptr_t itemNameGetter,
                       cc::function<void(void *, size_t)>::ptr_t onClick,
                       void *cap)
        : mOnClick(onClick), mCap(cap)
    {
        mList.mCap = cap;
        mList.mItemCount = itemCount;
        mList.mItemNameGetter = itemNameGetter;
    }

    virtual void SetupEditing(ISettingItemEditorActions *papi, int x, int y)
    {
        CCASSERT(!!papi);
        mpApi = papi;
        mListControl.Init(&mList,
                          &mpApi->GetInputDelegator()->mMenuScrollA,
                          Property<int>{[](void *cap) {
                                            auto *pThis = (FunctionListEditor *)cap;
                                            return pThis->mListSelectedItem;
                                        },
                                        [](void *cap, const int &val) {
                                            auto *pThis = (FunctionListEditor *)cap;
                                            pThis->mListSelectedItem = val;
                                        },
                                        this});
    }

    // for editing when holding back button. so back button is definitely pressed, and we don't want to bother
    // with encoder button pressed here.
    virtual void UpdateMomentaryMode(int encIntDelta)
    {
    }

    virtual void Update(bool backWasPressed, bool encWasPressed, int encIntDelta)
    {
        CCASSERT(mpApi != nullptr);
        mListControl.Update();
        if (backWasPressed)
        {
            mpApi->CommitEditing();
        }
        if (encWasPressed)
        {
            mOnClick(mCap, mListSelectedItem);
            mpApi->CommitEditing();
        }
    }

    virtual void Render()
    {
        mpApi->GetDisplay()->SetupModal();
        int nitems = mpApi->GetDisplay()->ClippedAreaHeight() / mpApi->GetDisplay()->mDisplay.GetLineHeight();
        mListControl.Render(mpApi->GetDisplay(), 12, 12, nitems);
    }
};

struct FunctionListSettingItem : public ISettingItem
{
    String mName;
    FunctionListEditor mEditor;
    Property<bool> mIsEnabled;
    // size_t itemCount;
    // cc::function<String(void*,size_t)>::ptr_t mItemNameGetter;
    // cc::function<void(void*,size_t)>::ptr_t mOnClick;
    // void* mCap;

    FunctionListSettingItem(const String &name,
                            size_t itemCount,
                            cc::function<String(void *, size_t)>::ptr_t itemNameGetter,
                            cc::function<void(void *, size_t)>::ptr_t onClick,
                            const Property<bool> &isEnabled,
                            void *cap)
        : mName(name), mEditor(itemCount, itemNameGetter, onClick, cap), mIsEnabled(isEnabled)
    {
    }

    virtual String GetName(size_t multiIndex)
    {
        return mName;
    }
    virtual String GetValueString(size_t multiIndex)
    {
        return "";
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
