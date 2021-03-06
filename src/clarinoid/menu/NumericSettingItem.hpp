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

using IntSettingItem = NumericSettingItem<int, IntEditor>;
using FloatSettingItem = NumericSettingItem<float, FloatEditor>;


} // namespace clarinoid
