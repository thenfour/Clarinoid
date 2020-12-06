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
  T mMin;
  T mMax;
  Property<T> mBinding;
  T oldVal;

  NumericEditor(T min_, T max_, const Property<T>& binding) :
    mMin(min_),
    mMax(max_),
    mBinding(binding)
  {
  }

  virtual T Add(T rhs, int encoderIntDelta) = 0;
  virtual void DrawValue(T val, T oldVal) = 0;
  
  virtual void SetupEditing(ISettingItemEditorActions* papi, int x, int y) {
    mpApi = papi;
    CCASSERT(!!papi);
//    if (!mpApi) mpApi = this;
    oldVal = mBinding.GetValue();
    this->x = x;
    this->y = y;
  }

  // for editing when holding back button. so back button is definitely pressed, and we don't want to bother
  // with encoder button pressed here.
  virtual void UpdateMomentaryMode(int encIntDelta)
  {
    mBinding.SetValue(constrain(Add(mBinding.GetValue(), encIntDelta), mMin, mMax));
  }
  
  virtual void Update(bool backWasPressed, bool encWasPressed, int encIntDelta)
  {
    mBinding.SetValue(constrain(Add(mBinding.GetValue(), encIntDelta), mMin, mMax));
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
  IntEditor(int min_, int max_, const Property<int>& binding) :
    NumericEditor(min_, max_, binding)
  {
  }
  virtual int Add(int n, int encDelta) {
    return n + encDelta;
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
  FloatEditor(float min_, float max_, const Property<float>& binding) :
    NumericEditor(min_, max_, binding)
  {  
  }
  virtual float Add(float n, int encDelta) {
    return n + (float)encDelta * (mMax - mMin) / 100;
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
  typename cc::function<bool()>::ptr_t mIsEnabled;
  
  NumericSettingItem(const String& name, T min_, T max_, const Property<T>& binding, typename cc::function<bool()>::ptr_t isEnabled) :
    mName(name),
    mEditor(min_, max_, binding),
    mBinding(binding),
    mIsEnabled(isEnabled)
  {
  }

  virtual String GetName(size_t multiIndex) { return mName; }
  virtual String GetValueString(size_t multiIndex) { return String(mBinding.GetValue()); }
  virtual SettingItemType GetType(size_t multiIndex) { return SettingItemType::Custom; }
  virtual bool IsEnabled(size_t multiIndex) const { return mIsEnabled(); }

  virtual ISettingItemEditor* GetEditor(size_t multiIndex) {
    return &mEditor;
  }
};

using IntSettingItem = NumericSettingItem<int, IntEditor>;
//using UInt16SettingItem = NumericSettingItem<uint16_t, IntEditor>;
using FloatSettingItem = NumericSettingItem<float, FloatEditor>;


} // namespace clarinoid
