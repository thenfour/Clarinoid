#pragma once

#include <Shared_CCUtil.h>
#include "MenuAppBase.hpp"
#include "MenuListControl.hpp"

static const int SETTINGS_STACK_MAX_DEPTH = 10;
static const int MAX_SETTING_ITEMS_PER_LIST = 20;

static bool AlwaysEnabled() { return true; }


/////////////////////////////////////////////////////////////////////////////////////////////////
// implemented by the settings app to provide an aPI to editors
struct ISettingItemEditorActions
{
  virtual void CommitEditing() = 0;
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
  virtual String GetName() = 0;
  virtual String GetValueString() { return ""; } // not used for trigger types or submenu types
  virtual SettingItemType GetType() = 0;
  virtual bool IsEnabled() const = 0; // grayed or not

  virtual void ToggleBool() {} // for bool types
  virtual void Trigger() {} // for trigger types
  virtual struct ISettingItemEditor* GetEditor() { return nullptr; } // for custom types
  virtual struct SettingsList* GetSubmenu() { return nullptr; } // for submenu type
};



struct SettingsList
{
  int mItemCount;
  ISettingItem* mItems[MAX_SETTING_ITEMS_PER_LIST];
  
  int Count() const { return mItemCount; }
  ISettingItem* GetItem(int i) { return mItems[i]; }
  void Add(ISettingItem* p) {
    mItems[mItemCount] = p;
    mItemCount ++;
  }
};


/////////////////////////////////////////////////////////////////////////////////////////////////
struct ISettingItemEditor : ISettingItemEditorActions
{
  virtual void SetupEditing(ISettingItemEditorActions* papi, int x, int y) = 0;
  virtual void Update(bool backWasPressed, bool encWasPressed, int encIntDelta) = 0; // every frame.
  virtual void Render() = 0; // not every frame.
  virtual void UpdateMomentaryMode(int encIntDelta) = 0;

  // just for convenience.
  virtual void CommitEditing() { }
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
    if (!mpApi) mpApi = this;
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
    gDisplay.SetupModal();
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
    gDisplay.mDisplay.print(String("") + n);
    int delta = n - oldVal;
    gDisplay.mDisplay.print(String(" (") + (delta >= 0 ? "+" : "") + delta + ")");
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
    gDisplay.mDisplay.print(String("") + n);
    float delta = n - oldVal;
    gDisplay.mDisplay.print(String(" (") + (delta >= 0 ? "+" : "") + delta + ")");
  }
};


template<typename T, typename TEditor>
struct NumericSettingItem : public ISettingItem
{
  String mName;
  TEditor mEditor;
  Property<T> mBinding;
  std::function<bool()> mIsEnabled;
  
  NumericSettingItem(SettingsList& list, const String& name, T min_, T max_, const Property<T>& binding, const std::function<bool()>& isEnabled) :
    mName(name),
    mEditor(min_, max_, binding),
    mBinding(binding),
    mIsEnabled(isEnabled)
  {
    list.Add(this);
  }

  virtual String GetName() { return mName; }
  virtual String GetValueString() { return String(mBinding.GetValue()); }
  virtual SettingItemType GetType() { return SettingItemType::Custom; }
  virtual bool IsEnabled() const { return mIsEnabled(); }

  virtual ISettingItemEditor* GetEditor() {
    return &mEditor;
  }
};

using IntSettingItem = NumericSettingItem<int, IntEditor>;
using FloatSettingItem = NumericSettingItem<float, FloatEditor>;

struct BoolSettingItem : public ISettingItem
{
  String mName;
  Property<bool> mBinding;
  String mTrueCaption;
  String mFalseCaption;
  std::function<bool()> mIsEnabled;
  
  BoolSettingItem(SettingsList& list, const String& name, const String& trueCaption, const String& falseCaption, const Property<bool>& binding, std::function<bool()> isEnabled) :
    mName(name),
    mBinding(binding),
    mTrueCaption(trueCaption),
    mFalseCaption(falseCaption),
    mIsEnabled(isEnabled)
  {
    list.Add(this);
  }

  virtual String GetName() { return mName; }
  virtual String GetValueString() { return mBinding.GetValue() ? mTrueCaption : mFalseCaption; }
  virtual bool IsEnabled() const { return mIsEnabled(); }
  virtual SettingItemType GetType() { return SettingItemType::Bool; }
  virtual void ToggleBool() {
    mBinding.SetValue(!mBinding.GetValue());
  }
};


struct TriggerSettingItem : public ISettingItem
{
  String mName;
  std::function<void()> mAction;
  std::function<bool()> mIsEnabled;
  
  TriggerSettingItem(SettingsList& list, const String& name, std::function<void()> action, const std::function<bool()>& isEnabled) :
    mName(name),
    mAction(action),
    mIsEnabled(isEnabled)
  {
    list.Add(this);
  }

  virtual String GetName() { return mName; }
  virtual SettingItemType GetType() { return SettingItemType::Trigger; }
  virtual bool IsEnabled() const { return mIsEnabled(); }
  virtual void Trigger() {
    mAction();
  }
};


struct SubmenuSettingItem : public ISettingItem
{
  String mName;
  SettingsList* mSubmenu;
  std::function<bool()> mIsEnabled;
  
  SubmenuSettingItem(SettingsList& list, const String& name, SettingsList* pSubmenu, const std::function<bool()>& isEnabled) :
    mName(name),
    mSubmenu(pSubmenu),
    mIsEnabled(isEnabled)
  {
    list.Add(this);
  }

  virtual String GetName() { return mName; }
  virtual SettingItemType GetType() { return SettingItemType::Submenu; }
  virtual bool IsEnabled() const { return mIsEnabled(); }
  virtual struct SettingsList* GetSubmenu()
  {
    return mSubmenu;
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
    [&]() { return (int)mBinding.GetValue(); },
    [&](const int& val) { mBinding.SetValue((T)val); }
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
    gDisplay.SetupModal();
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
  std::function<bool()> mIsEnabled;

  EnumSettingItem(SettingsList& list, const String& name, const EnumInfo<T>& enumInfo, const Property<T>& binding, const std::function<bool()>& isEnabled) :
    mName(name),
    mEditor(enumInfo, binding),
    mBinding(binding),
    mEnumInfo(enumInfo),
    mIsEnabled(isEnabled)
  {
    list.Add(this);
  }

  virtual String GetName() { return mName; }
  virtual String GetValueString() { return mEnumInfo.GetValueString(mBinding.GetValue()); }
  virtual SettingItemType GetType() { return SettingItemType::Custom; }
  virtual bool IsEnabled() const { return mIsEnabled(); }
  virtual ISettingItemEditor* GetEditor() {
    return &mEditor;
  }
};



struct SettingsMenuState
{
  SettingsList* pList;
  int focusedItem;
};



/////////////////////////////////////////////////////////////////////////////////////////////////
class SettingsMenuApp : public MenuAppBaseWithUtils, public ISettingItemEditorActions
{
  SettingsMenuState mNav[SETTINGS_STACK_MAX_DEPTH];
  int mNavDepth = 0;
  ISettingItemEditor* mpCurrentEditor = nullptr;
//
//  CCThrottlerT<2000> mToastTimer;
//  bool mIsShowingToast = false;
//  String mToastMsg;

public:

  virtual void CommitEditing() {
    mpCurrentEditor = nullptr;
  }
  
  virtual void OnSelected() {
    mNavDepth = 0;
    mNav[0].focusedItem = 0;
    mNav[0].pList = GetRootSettingsList();
    GoToFrontPage();
  }
//
//  void ShowToast(const String& msg) {
//    mIsShowingToast = true;
//    mToastMsg = msg;
//    mToastTimer.Reset();
//  }

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
    SettingsMenuState& state = mNav[mNavDepth];

    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setCursor(0,0);
    gDisplay.mDisplay.setTextWrap(false);
    int itemToRender = AddConstrained(state.focusedItem, -1, 0, state.pList->Count() - 1);
    const int maxItemsToRender = 4;
    const int itemsToRender = min(maxItemsToRender, state.pList->Count());
    int i = 0;
    for (; i < itemsToRender; ++ i) {
      auto* item = state.pList->GetItem(itemToRender);
      if (itemToRender == state.focusedItem) {
        gDisplay.mDisplay.setTextSize(1);
        gDisplay.mDisplay.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
      } else {
        gDisplay.mDisplay.setTextSize(1);
        gDisplay.mDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
      }

      gDisplay.mDisplay.mSolidText = item->IsEnabled();

      switch (item->GetType()) {
      case SettingItemType::Trigger:
        gDisplay.mDisplay.println(item->GetName());
        break;
      case SettingItemType::Submenu:
        gDisplay.mDisplay.println(item->GetName() + " -->");
        break;
      default:
        gDisplay.mDisplay.println(item->GetName() + " = " + item->GetValueString());
        break;  
      }

      itemToRender = AddConstrained(itemToRender, 1, 0, state.pList->Count() - 1);
    }

    if (mpCurrentEditor) {
      mpCurrentEditor->Render();
    }

//    if (mIsShowingToast) {
//      if (mToastTimer.IsReady()) {
//        mIsShowingToast = false;
//      } else {
//        // render toast.
//        SetupModal();
//        gDisplay.mDisplay.print(mToastMsg);
//      }
//    }
  }

  // handle extra ui stuff for the front page to enable "back actions".
  virtual void Update()
  {
    if (IsShowingFrontPage()) {
      if (BackButton().IsNewlyPressed()) {
        mpCurrentEditor = GetBackEditor();
        if (mpCurrentEditor) {
          mpCurrentEditor->SetupEditing(nullptr, 0, 0);
        }
        return;
      } else if (BackButton().IsCurrentlyPressed() && mpCurrentEditor) {
        mpCurrentEditor->UpdateMomentaryMode(EncoderIntDelta());
        return;
      }
      mpCurrentEditor = nullptr;
    }
    MenuAppBaseWithUtils::Update();    
  }

  
  virtual void UpdateApp()
  {
    SettingsMenuState& state = mNav[mNavDepth];

    if (mpCurrentEditor) {
      mpCurrentEditor->Update(BackButton().IsNewlyPressed(), gEncButton.IsNewlyPressed(), EncoderIntDelta());
      return;
    }

    // scrolling
    state.focusedItem = AddConstrained(state.focusedItem, EncoderIntDelta(), 0, state.pList->Count() - 1);
    
    // enter
    if (gEncButton.IsNewlyPressed()) 
    {
      auto* focusedItem = state.pList->GetItem(state.focusedItem);
      if (!focusedItem->IsEnabled()) {
        // todo: show toast? or some flash or feedback for disabled items?
      } else {      
        switch(focusedItem->GetType()) {
          case SettingItemType::Bool:
            focusedItem->ToggleBool();
            break;
          case SettingItemType::Custom:
            mpCurrentEditor = focusedItem->GetEditor();
            mpCurrentEditor->SetupEditing(this, 0, 0);
            break;
          case SettingItemType::Submenu:
            mNavDepth ++;
            mNav[mNavDepth].pList = focusedItem->GetSubmenu();
            mNav[mNavDepth].focusedItem = 0;
            break;
          case SettingItemType::Trigger:
            focusedItem->Trigger();
            break;
        }
      }
    }

    // back/up when not editing
    if (BackButton().IsNewlyPressed()) 
    {
      if (mNavDepth == 0) {
        GoToFrontPage();
      } else {
        mNavDepth --;
      }
    }
  }
};
