#pragma once

#include <clarinoid/basic/Basic.hpp>
#include "MenuAppBase.hpp"
#include "MenuListControl.hpp"

namespace clarinoid
{


template<typename T>
struct NumericEditor : ISettingItemEditor
{
  ISettingItemEditorActions* mpApi;
  int x;
  int y;
  NumericEditRangeSpec<T> mRange;
  Property<T> mBinding;
  T oldVal;

  NumericEditor(const NumericEditRangeSpec<T>& range_, const Property<T>& binding) :
    mRange(range_),
    mBinding(binding)
  {
  }

  virtual void DrawValue(T val, T oldVal) = 0;
  
  virtual void SetupEditing(ISettingItemEditorActions* papi, int x, int y) {
    mpApi = papi;
    CCASSERT(!!papi);
    oldVal = mBinding.GetValue();
    this->x = x;
    this->y = y;
  }

  // for editing when holding back button. so back button is definitely pressed, and we don't want to bother
  // with encoder button pressed here.
  virtual void UpdateMomentaryMode(int encIntDelta)
  {
    T val = mRange.AdjustValue(mBinding.GetValue(), encIntDelta, mpApi->GetInputDelegator()->mModifierCourse.CurrentValue(), mpApi->GetInputDelegator()->mModifierFine.CurrentValue());
    mBinding.SetValue(val);
  }
  
  virtual void Update(bool backWasPressed, bool encWasPressed, int encIntDelta)
  {
    T val = mRange.AdjustValue(mBinding.GetValue(), encIntDelta, mpApi->GetInputDelegator()->mModifierCourse.CurrentValue(), mpApi->GetInputDelegator()->mModifierFine.CurrentValue());
    mBinding.SetValue(val);

    if (backWasPressed) {
      mBinding.SetValue(oldVal);
      mpApi->CommitEditing();
    }
    if (encWasPressed) {
      mpApi->CommitEditing();
    }
  }
  virtual void Render()
  {
    mpApi->GetDisplay()->SetupModal();
    DrawValue(mBinding.GetValue(), oldVal);
  }
};

struct IntEditor : NumericEditor<int>
{
  IntEditor(const NumericEditRangeSpec<int>& range_, const Property<int>& binding) :
    NumericEditor(range_, binding)
  {
  }

  virtual void DrawValue(int n, int oldVal)
  {
    this->mpApi->GetDisplay()->mDisplay.print(String("") + n);
    int delta = n - oldVal;
    this->mpApi->GetDisplay()->mDisplay.print(String(" (") + (delta >= 0 ? "+" : "") + delta + ")");
  }
};

struct FloatEditor : NumericEditor<float>
{
  FloatEditor(const NumericEditRangeSpec<float>& range_, const Property<float>& binding) :
    NumericEditor(range_, binding)
  {  
  }

  virtual void DrawValue(float n, float oldVal)
  {
    this->mpApi->GetDisplay()->mDisplay.print(String("") + n);
    float delta = n - oldVal;
    this->mpApi->GetDisplay()->mDisplay.print(String(" (") + (delta >= 0 ? "+" : "") + delta + ")");
  }
};


template<typename T, typename TEditor>
struct NumericSettingItem : public ISettingItem
{
  String mName;
  TEditor mEditor;
  Property<T> mBinding;
  Property<bool> mIsEnabled;
  
  NumericSettingItem(const String& name, const NumericEditRangeSpec<T>& range_, const Property<T>& binding, const Property<bool>& isEnabled) :
    mName(name),
    mEditor(range_, binding),
    mBinding(binding),
    mIsEnabled(isEnabled)
  {
  }

  virtual String GetName(size_t multiIndex) { return mName; }
  virtual String GetValueString(size_t multiIndex) { return String(mBinding.GetValue()); }
  virtual SettingItemType GetType(size_t multiIndex) { return SettingItemType::Custom; }
  virtual bool IsEnabled(size_t multiIndex) const { return mIsEnabled.GetValue(); }

  virtual ISettingItemEditor* GetEditor(size_t multiIndex) {
    return &mEditor;
  }
};




template<typename T, typename TEditor>
struct MultiNumericSettingItem : public ISettingItem
{
  cc::function<size_t(void*)>::ptr_t mGetItemCount;
  cc::function<String(void*,size_t)>::ptr_t mLabelGetter;
  cc::function<String(void*,size_t)>::ptr_t mValueStringGetter;

  typename cc::function<T(void*, size_t) >::ptr_t mValueGetter;
  typename cc::function<void(void*, size_t, const T& val)>::ptr_t mValueSetter;

  size_t mEditingMultiIndex = 0; // which multi item index is currently being edited.

  cc::function<bool(void*,size_t)>::ptr_t mIsEnabled;
  void* mCapture;

  TEditor mEditor;

  MultiNumericSettingItem(
    cc::function<size_t(void*)>::ptr_t getItemCount,
    const NumericEditRangeSpec<T>& range_,
    cc::function<String(void*, size_t)>::ptr_t labelGetter,
    cc::function<String(void*,size_t)>::ptr_t valueStringGetter,
    typename cc::function<T(void*, size_t multiIndex)>::ptr_t valueGetter,
    typename cc::function<void(void*, size_t multiIndex, const T& val)>::ptr_t valueSetter,
    cc::function<bool(void*, size_t)>::ptr_t isEnabled,
    void* capture) :

    mGetItemCount(getItemCount),
    mLabelGetter(labelGetter),
    mValueStringGetter(valueStringGetter),
    mValueGetter(valueGetter),
    mValueSetter(valueSetter),
    mIsEnabled(isEnabled),
    mCapture(capture),
    mEditor(range_, Property<T>{
      [](void* cap){ // get
        auto* pThis = (MultiNumericSettingItem*)cap;
        return pThis->mValueGetter(pThis->mCapture, pThis->mEditingMultiIndex);
      },
      [](void* cap, const T& val) { // set
        auto* pThis = (MultiNumericSettingItem*)cap;
        pThis->mValueSetter(pThis->mCapture, pThis->mEditingMultiIndex, val);
      },
      this// cap
    })
  {
  }

  virtual size_t GetMultiCount() { return mGetItemCount(mCapture); }
  virtual String GetName(size_t multiIndex) { return mLabelGetter(mCapture, multiIndex); }
  virtual String GetValueString(size_t multiIndex) { return mValueStringGetter(mCapture, multiIndex); }
  virtual SettingItemType GetType(size_t multiIndex) { return SettingItemType::Custom; }
  virtual bool IsEnabled(size_t multiIndex) const { return mIsEnabled(mCapture, multiIndex); }
  virtual ISettingItemEditor* GetEditor(size_t multiIndex) {
    mEditingMultiIndex = multiIndex;
    return &mEditor;
  }
};




using IntSettingItem = NumericSettingItem<int, IntEditor>;
using FloatSettingItem = NumericSettingItem<float, FloatEditor>;


using MultiIntSettingItem = MultiNumericSettingItem<int, IntEditor>;




} // namespace clarinoid
