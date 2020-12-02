#pragma once

#include <clarinoid/basic/Basic.hpp>
#include "MenuAppBase.hpp"
#include "MenuListControl.hpp"

namespace clarinoid
{

static const int SETTINGS_STACK_MAX_DEPTH = 10;

static bool AlwaysEnabled() { return true; }
static bool AlwaysEnabledWithCapture(void*) { return true; }


/////////////////////////////////////////////////////////////////////////////////////////////////
// implemented by the settings app to provide an aPI to editors
struct ISettingItemEditorActions
{
  virtual void CommitEditing() = 0;
  virtual CCDisplay* GetDisplay() = 0;
  virtual AppSettings* GetAppSettings() = 0;
  virtual IControlMapper* GetControlMapper() = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
enum class SettingItemType
{
  Custom, // all values enter an "editing" state with their own editors.
  Bool, // bool values don't have an editing state; they just toggle.
  Trigger, // not a setting, but an action you trigger
  Submenu
};

/////////////////////////////////////////////////////////////////////////////////////////////////
struct ISettingItem
{
  virtual size_t GetMultiCount() { return 1; } // return 1 if this is 1 item. otherwise this item gets replicated N times, with an index parameter.

  virtual String GetName(size_t multiIndex) = 0;
  virtual String GetValueString(size_t multiIndex) { return ""; } // not used for trigger types or submenu types
  virtual SettingItemType GetType(size_t multiIndex) = 0;
  virtual bool IsEnabled(size_t multiIndex) const = 0; // grayed or not

  virtual void ToggleBool(size_t multiIndex) {} // for bool types
  virtual void Trigger(size_t multiIndex) {} // for trigger types
  virtual struct ISettingItemEditor* GetEditor(size_t multiIndex) { return nullptr; } // for custom types

  virtual struct SettingsList* GetSubmenu(size_t multiIndex) { return nullptr; } // for submenu type
};



struct SettingsList
{
  ISettingItem** mItems;
  size_t mItemRawCount;
  size_t mItemTotalCount;

  template<size_t N>
  SettingsList(ISettingItem* (&arr)[N]) :
    mItems(arr),
    mItemRawCount(N)
  {
    mItemTotalCount = 0;
    for (auto& i : arr) {
      mItemTotalCount += i->GetMultiCount();
    }
  }
  
  size_t Count() const { return mItemTotalCount; }

  // to consider: LUT or binary search. but since this is just for the menu system, not critical.
  ISettingItem* GetItem(size_t ireq, size_t& multiIndex)
  {
    if (ireq < 0) {
      CCASSERT(!"SettingsList::GetItem < 0");
    }
    size_t itemBeginIndex = 0;
    for (size_t iraw = 0; iraw < mItemRawCount; ++ iraw)
    {
      size_t itemEndIndex = itemBeginIndex + mItems[iraw]->GetMultiCount();
      if (ireq < itemEndIndex) {
        multiIndex = ireq - itemBeginIndex;
        return mItems[iraw];
      }
      itemBeginIndex += mItems[iraw]->GetMultiCount();
    }
    CCASSERT(!"setting item out of range");
    return nullptr;
  }
};


/////////////////////////////////////////////////////////////////////////////////////////////////
struct ISettingItemEditor// : ISettingItemEditorActions
{
  virtual void SetupEditing(ISettingItemEditorActions* papi, int x, int y) = 0;
  virtual void Update(bool backWasPressed, bool encWasPressed, int encIntDelta) = 0; // every frame.
  virtual void Render() = 0; // not every frame.
  virtual void UpdateMomentaryMode(int encIntDelta) = 0;

  // just for convenience.
  //virtual void CommitEditing() { }
};

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

struct BoolSettingItem : public ISettingItem
{
  String mName;
  Property<bool> mBinding;
  String mTrueCaption;
  String mFalseCaption;
  typename cc::function<bool()>::ptr_t mIsEnabled;
  
  BoolSettingItem(const String& name, const String& trueCaption, const String& falseCaption, const Property<bool>& binding, typename cc::function<bool()>::ptr_t isEnabled) :
    mName(name),
    mBinding(binding),
    mTrueCaption(trueCaption),
    mFalseCaption(falseCaption),
    mIsEnabled(isEnabled)
  {
  }

  virtual String GetName(size_t multiIndex) { return mName; }
  virtual String GetValueString(size_t multiIndex) { return mBinding.GetValue() ? mTrueCaption : mFalseCaption; }
  virtual bool IsEnabled(size_t multiIndex) const { return mIsEnabled(); }
  virtual SettingItemType GetType(size_t multiIndex) { return SettingItemType::Bool; }
  virtual void ToggleBool(size_t multiIndex) {
    mBinding.SetValue(!mBinding.GetValue());
  }
};


struct TriggerSettingItem : public ISettingItem
{
  String mName;
  typename cc::function<void(void*)>::ptr_t mAction;
  typename cc::function<bool()>::ptr_t mIsEnabled;
  void* mActionCapture = nullptr;
  
  TriggerSettingItem(const String& name, cc::function<void(void*)>::ptr_t action, void* actionCapture, typename cc::function<bool()>::ptr_t isEnabled) :
    mName(name),
    mAction(action),
    mIsEnabled(isEnabled),
    mActionCapture(actionCapture)
  {
  }

  virtual String GetName(size_t multiIndex) { return mName; }
  virtual SettingItemType GetType(size_t multiIndex) { return SettingItemType::Trigger; }
  virtual bool IsEnabled(size_t multiIndex) const { return mIsEnabled(); }
  virtual void Trigger(size_t multiIndex) {
    mAction(mActionCapture);
  }
};


struct SubmenuSettingItem : public ISettingItem
{
  String mName;
  SettingsList* mSubmenu;
  typename cc::function<bool()>::ptr_t mIsEnabled;
  
  SubmenuSettingItem(const String& name, SettingsList* pSubmenu, typename cc::function<bool()>::ptr_t isEnabled) :
    mName(name),
    mSubmenu(pSubmenu),
    mIsEnabled(isEnabled)
  {
  }

  virtual String GetName(size_t multiIndex) { return mName; }
  virtual SettingItemType GetType(size_t multiIndex) { return SettingItemType::Submenu; }
  virtual bool IsEnabled(size_t multiIndex) const { return mIsEnabled(); }
  virtual struct SettingsList* GetSubmenu(size_t multiIndex)
  {
    return mSubmenu;
  }
};



struct MultiSubmenuSettingItem : public ISettingItem
{
  size_t mItemCount;
  typename cc::function<SettingsList*(void*,size_t)>::ptr_t mGetSubmenu;// SettingsList* mSubmenu;
  typename cc::function<String(void*,size_t)>::ptr_t mName;
  typename cc::function<bool(void*,size_t)>::ptr_t mIsEnabled;
  void* mpCapture = nullptr;
  
  MultiSubmenuSettingItem(size_t itemCount, typename cc::function<String(void*,size_t)>::ptr_t name, typename cc::function<SettingsList*(void*,size_t)>::ptr_t pGetSubmenu, typename cc::function<bool(void*,size_t)>::ptr_t isEnabled, void* capture) :
    mItemCount(itemCount),
    mGetSubmenu(pGetSubmenu),
    mName(name),
    mIsEnabled(isEnabled),
    mpCapture(capture)
  {
  }

  virtual size_t GetMultiCount() { return mItemCount; }
  virtual String GetName(size_t multiIndex) { return mName(mpCapture,multiIndex); }
  virtual SettingItemType GetType(size_t multiIndex) { return SettingItemType::Submenu; }
  virtual bool IsEnabled(size_t multiIndex) const { return mIsEnabled(mpCapture,multiIndex); }
  virtual struct SettingsList* GetSubmenu(size_t multiIndex)
  {
    return mGetSubmenu(mpCapture,multiIndex);
  }
};



template<typename T>
struct EnumEditor : ISettingItemEditor
{
  // set on SetupEditing()
  ISettingItemEditorActions* mpApi;
  T mOldVal;

  Property<T> mBinding;
  const EnumInfo<T>& mEnumInfo;

  Property<int> mListSelectedItem = {
    [](void* capture) { EnumEditor<T>* pthis = (EnumEditor<T>*)capture; return (int)pthis->mBinding.GetValue(); },
    [](void* capture, const int& val) { EnumEditor<T>* pthis = (EnumEditor<T>*)capture; pthis->mBinding.SetValue((T)val); },
    this
  };

  ListControl mListControl;

  EnumEditor(const EnumInfo<T>&enumInfo, const Property<T>& binding) :
    mBinding(binding),
    mEnumInfo(enumInfo),
    mListControl(&mEnumInfo, mListSelectedItem, 12, 12, 3)
  {
  }

  virtual void SetupEditing(ISettingItemEditorActions* papi, int x, int y) {
    mpApi = papi;
    if (!mpApi) mpApi = this;
    mOldVal = mBinding.GetValue();
  }

  // for editing when holding back button. so back button is definitely pressed, and we don't want to bother
  // with encoder button pressed here.
  virtual void UpdateMomentaryMode(int encIntDelta)
  {
  }
  
  virtual void Update(bool backWasPressed, bool encWasPressed, int encIntDelta)
  {
    CCASSERT(mpApi != nullptr);
    //Serial.println(String("update enum editor ") + (int)(micros()));
    mListControl.Update();
    if (backWasPressed) {
      mBinding.SetValue(mOldVal);
      mpApi->CommitEditing();
    }
    if (encWasPressed) {
      mpApi->CommitEditing();
    }
  }
  virtual void Render()
  {
    mpApi->GetDisplay()->SetupModal();
    mListControl.Render();
  }
};


template<typename T>
struct EnumSettingItem : public ISettingItem
{
  String mName;
  EnumEditor<T> mEditor;
  Property<T> mBinding;
  const EnumInfo<T>& mEnumInfo;
  cc::function<bool()>::ptr_t mIsEnabled;

  EnumSettingItem(const String& name, const EnumInfo<T>& enumInfo, const Property<T>& binding, cc::function<bool()>::ptr_t isEnabled) :
    mName(name),
    mEditor(enumInfo, binding),
    mBinding(binding),
    mEnumInfo(enumInfo),
    mIsEnabled(isEnabled)
  {
  }

  virtual String GetName(size_t multiIndex) { return mName; }
  virtual String GetValueString(size_t multiIndex) { return mEnumInfo.GetValueString(mBinding.GetValue()); }
  virtual SettingItemType GetType(size_t multiIndex) { return SettingItemType::Custom; }
  virtual bool IsEnabled(size_t multiIndex) const { return mIsEnabled(); }
  virtual ISettingItemEditor* GetEditor(size_t multiIndex) {
    return &mEditor;
  }
};




struct LabelSettingItem : public ISettingItem
{
  cc::function<String()>::ptr_t mText = nullptr;
  cc::function<bool()>::ptr_t mIsEnabled = nullptr;

  cc::function<String(void*)>::ptr_t mTextWithCapture = nullptr;
  cc::function<bool(void*)>::ptr_t mIsEnabledWithCapture = nullptr;
  void* mCapture = nullptr;

  LabelSettingItem(cc::function<String()>::ptr_t text, cc::function<bool()>::ptr_t isEnabled) :
    mText(text),
    mIsEnabled(isEnabled)
  {
  }

  LabelSettingItem(cc::function<String(void*)>::ptr_t text, cc::function<bool(void*)>::ptr_t isEnabled, void* capture) :
    mTextWithCapture(text),
    mIsEnabledWithCapture(isEnabled),
    mCapture(capture)
  {
  }

  virtual String GetName(size_t multiIndex) { return mText ? mText() : mTextWithCapture(mCapture); }
  virtual String GetValueString(size_t multiIndex) { return ""; }
  virtual SettingItemType GetType(size_t multiIndex) { return SettingItemType::Custom; }
  virtual bool IsEnabled(size_t multiIndex) const { return mIsEnabled ? mIsEnabled() : mIsEnabledWithCapture(mCapture); }
  virtual ISettingItemEditor* GetEditor() {
    return nullptr;
  }
};






struct StringSettingItem : public ISettingItem
{
  cc::function<String()>::ptr_t mText;
  cc::function<bool()>::ptr_t mIsEnabled;

  StringSettingItem(cc::function<String()>::ptr_t text, cc::function<bool()>::ptr_t isEnabled) :
    mText(text),
    mIsEnabled(isEnabled)
  {
  }

  virtual String GetName(size_t multiIndex) { return mText(); }
  virtual String GetValueString(size_t multiIndex) { return ""; }
  virtual SettingItemType GetType(size_t multiIndex) { return SettingItemType::Custom; }
  virtual bool IsEnabled(size_t multiIndex) const { return mIsEnabled(); }
  virtual ISettingItemEditor* GetEditor() {
    return nullptr;
  }
};






struct SettingsMenuState
{
  SettingsList* pList;
  int focusedItem;
};


// glabal because:
// - we don't want to use dynamic memory for this (so static array)
// - but we need enough space for a max stack scenario
// - we never actually need to track multiple stacks.
// - so this prevents a big redundant array being duplicated for every settings menu
SettingsMenuState gSettingsMenuNavStack[SETTINGS_STACK_MAX_DEPTH];
int gSettingsMenuNavDepth = 0;


/////////////////////////////////////////////////////////////////////////////////////////////////
// abstract class for specific settings apps
struct SettingsMenuApp :
    DisplayApp,
    ISettingItemEditorActions
{
  ISettingItemEditor* mpCurrentEditor = nullptr;

  SettingsMenuApp(CCDisplay& d) :
    DisplayApp(d)
  {
  }

  virtual void CommitEditing() {
    mpCurrentEditor = nullptr;
  }
  
  virtual void DisplayAppOnSelected() {
    gSettingsMenuNavDepth = 0;
    gSettingsMenuNavStack[0].focusedItem = 0;
    gSettingsMenuNavStack[0].pList = GetRootSettingsList();
    DisplayApp::DisplayAppOnSelected();
  }

  virtual CCDisplay* GetDisplay() { return &mDisplay; }
  virtual AppSettings* GetAppSettings() { return mAppSettings; }
  virtual IControlMapper* GetControlMapper() { return mControlMapper; }


  virtual SettingsList* GetRootSettingsList() = 0;
  virtual ISettingItemEditor* GetBackEditor() { return nullptr; }

  // children who override this must call this.
  virtual void RenderFrontPage()
  {
    if (mpCurrentEditor) {
      mpCurrentEditor->Render();
    }
  }

  virtual void RenderApp()
  {
    SettingsMenuState& state = gSettingsMenuNavStack[gSettingsMenuNavDepth];

    mDisplay.mDisplay.setTextSize(1);
    mDisplay.mDisplay.setCursor(0,0);
    mDisplay.mDisplay.setTextWrap(false);

    const size_t lineHeight = 8;
    const size_t maxItemsToRender = mDisplay.mDisplay.width() / lineHeight;
    const size_t itemsToRender = min(maxItemsToRender, state.pList->Count());
    const size_t itemsFromTop = maxItemsToRender / 3; // estimated... whatever.

    size_t itemToRender = AddConstrained(state.focusedItem, itemsFromTop, 0, state.pList->Count() - 1);
    size_t i = 0;
    for (; i < itemsToRender; ++ i) {
      size_t multiIndex = 0;
      auto* item = state.pList->GetItem(itemToRender, multiIndex);
      if (itemToRender == (size_t)state.focusedItem) {
        mDisplay.mDisplay.setTextSize(1);
        mDisplay.mDisplay.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
      } else {
        mDisplay.mDisplay.setTextSize(1);
        mDisplay.mDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
      }

      mDisplay.mDisplay.mSolidText = item->IsEnabled(multiIndex);

      switch (item->GetType(multiIndex)) {
      case SettingItemType::Trigger:
        mDisplay.mDisplay.println(item->GetName(multiIndex));
        break;
      case SettingItemType::Submenu:
        mDisplay.mDisplay.println(item->GetName(multiIndex) + " -->");
        break;
      default:
        mDisplay.mDisplay.println(item->GetName(multiIndex) + " = " + item->GetValueString(multiIndex));
        break;  
      }

      itemToRender = AddConstrained(itemToRender, 1, 0, state.pList->Count() - 1);
    }

    if (mpCurrentEditor) {
      mpCurrentEditor->Render();
    }
  }

  // handle extra ui stuff for the front page to enable "back actions".
  virtual void DisplayAppUpdate()
  {
    DisplayApp::DisplayAppUpdate(); // update input

    if (IsShowingFrontPage()) {
      if (mBack.IsNewlyPressed()) {
        mpCurrentEditor = GetBackEditor();
        if (mpCurrentEditor) {
          mpCurrentEditor->SetupEditing(nullptr, 0, 0);
        }
        return;
      } else if (mBack.IsPressedState() && mpCurrentEditor) {
        mpCurrentEditor->UpdateMomentaryMode(mEnc.GetIntDelta());
        return;
      }
      mpCurrentEditor = nullptr;
    }
  }

  
  virtual void UpdateApp()
  {
    SettingsMenuState& state = gSettingsMenuNavStack[gSettingsMenuNavDepth];

    if (mpCurrentEditor) {
      mpCurrentEditor->Update(mBack.IsNewlyPressed(), mOK.IsNewlyPressed(), mEnc.GetIntDelta());
      return;
    }

    // scrolling
    state.focusedItem = AddConstrained(state.focusedItem, mEnc.GetIntDelta(), 0, state.pList->Count() - 1);
    
    // enter
    if (mOK.IsNewlyPressed()) 
    {
      size_t multiIndex = 0;
      auto* focusedItem = state.pList->GetItem(state.focusedItem, multiIndex);
      if (!focusedItem->IsEnabled(multiIndex)) {
        // todo: show toast? or some flash or feedback for disabled items?
      } else {      
        switch(focusedItem->GetType(multiIndex)) {
          case SettingItemType::Bool:
            focusedItem->ToggleBool(multiIndex);
            break;
          case SettingItemType::Custom:
            mpCurrentEditor = focusedItem->GetEditor(multiIndex);
            if (mpCurrentEditor) {
              mpCurrentEditor->SetupEditing(this, 0, 0);
            }
            break;
          case SettingItemType::Submenu:
            gSettingsMenuNavDepth ++;
            gSettingsMenuNavStack[gSettingsMenuNavDepth].pList = focusedItem->GetSubmenu(multiIndex);
            gSettingsMenuNavStack[gSettingsMenuNavDepth].focusedItem = 0;
            break;
          case SettingItemType::Trigger:
            focusedItem->Trigger(multiIndex);
            break;
        }
      }
    }

    // back/up when not editing
    if (mBack.IsNewlyPressed()) 
    {
      if (gSettingsMenuNavDepth == 0) {
        GoToFrontPage();
      } else {
        gSettingsMenuNavDepth --;
      }
    }
  }
};

} // namespace clarinoid
