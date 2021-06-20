#pragma once

#include <clarinoid/basic/Basic.hpp>
#include "MenuAppBase.hpp"
#include "MenuListControl.hpp"

namespace clarinoid
{

static const int SETTINGS_STACK_MAX_DEPTH = 10;

static bool AlwaysEnabled(void*) { return true; }
//static bool AlwaysEnabledWithCapture(void*) { return true; }


/////////////////////////////////////////////////////////////////////////////////////////////////
// implemented by the settings app to provide an aPI to editors
struct ISettingItemEditorActions
{
  virtual void CommitEditing() = 0;
  virtual CCDisplay* GetDisplay() = 0;
  virtual AppSettings* GetAppSettings() = 0;
  virtual InputDelegator* GetInputDelegator() = 0;
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
  //size_t mItemTotalCount;

  template<size_t N>
  SettingsList(ISettingItem* (&arr)[N]) :
    mItems(arr),
    mItemRawCount(N)
  {
    // mItemTotalCount = 0;
    // for (auto& i : arr) {
    //   mItemTotalCount += i->GetMultiCount();
    // }
  }
  
  size_t Count() const {
    size_t mItemTotalCount = 0;
    for (size_t i = 0; i < mItemRawCount; ++ i) {
      mItemTotalCount += mItems[i]->GetMultiCount();
    }
    return mItemTotalCount;
  }

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
    Die(String("setting item out of range; req=") + ireq);
    return nullptr;
  }
};


/////////////////////////////////////////////////////////////////////////////////////////////////
struct ISettingItemEditor// : ISettingItemEditorActions
{
  virtual void SetupEditing(ISettingItemEditorActions* papi, int x, int y) = 0;
  virtual void Update(bool backWasPressed, bool encWasPressed, int encIntDelta) = 0; // every frame.
  virtual void Render() = 0; // not every frame.
  virtual void UpdateMomentaryMode(int encIntDelta) {}
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
  typename cc::function<size_t(void*)>::ptr_t mGetItemCount = nullptr;
  typename cc::function<SettingsList*(void*,size_t)>::ptr_t mGetSubmenu = nullptr;// SettingsList* mSubmenu;
  typename cc::function<String(void*,size_t)>::ptr_t mName = nullptr;
  typename cc::function<bool(void*,size_t)>::ptr_t mIsEnabled = nullptr;
  void* mpCapture = nullptr;

  MultiSubmenuSettingItem() = default;
  
  MultiSubmenuSettingItem(typename cc::function<size_t(void*)>::ptr_t getItemCount, typename cc::function<String(void*,size_t)>::ptr_t name, typename cc::function<SettingsList*(void*,size_t)>::ptr_t pGetSubmenu, typename cc::function<bool(void*,size_t)>::ptr_t isEnabled, void* capture) :
    mGetItemCount(getItemCount),
    mGetSubmenu(pGetSubmenu),
    mName(name),
    mIsEnabled(isEnabled),
    mpCapture(capture)
  {
  }
  
  void Init(typename cc::function<size_t(void*)>::ptr_t getItemCount, typename cc::function<String(void*,size_t)>::ptr_t name, typename cc::function<SettingsList*(void*,size_t)>::ptr_t pGetSubmenu, typename cc::function<bool(void*,size_t)>::ptr_t isEnabled, void* capture)
  {
    mGetItemCount = getItemCount;
    mGetSubmenu = pGetSubmenu;
    mName = name;
    mIsEnabled = isEnabled;
    mpCapture = capture;
  }

  virtual size_t GetMultiCount() { return mGetItemCount(mpCapture); }
  virtual String GetName(size_t multiIndex) { return mName(mpCapture,multiIndex); }
  virtual SettingItemType GetType(size_t multiIndex) { return SettingItemType::Submenu; }
  virtual bool IsEnabled(size_t multiIndex) const { return mIsEnabled(mpCapture,multiIndex); }
  virtual struct SettingsList* GetSubmenu(size_t multiIndex)
  {
    return mGetSubmenu(mpCapture,multiIndex);
  }
};


struct LabelSettingItem : public ISettingItem
{
  Property<String> mText;
  Property<bool> mIsEnabled;

  //cc::function<String(void*)>::ptr_t mTextWithCapture = nullptr;
  //cc::function<bool(void*)>::ptr_t mIsEnabledWithCapture = nullptr;
  //void* mCapture = nullptr;

  // LabelSettingItem(cc::function<String()>::ptr_t text, const Property<bool>& isEnabled) :
  //   mText(text),
  //   mIsEnabled(isEnabled)
  // {
  // }

  LabelSettingItem(const Property<String>& text, const Property<bool>& isEnabled) :
    mText(text),
    mIsEnabled(isEnabled)
  {
  }

  virtual String GetName(size_t multiIndex) { return mText.GetValue(); }
  virtual String GetValueString(size_t multiIndex) { return ""; }
  virtual SettingItemType GetType(size_t multiIndex) { return SettingItemType::Custom; }
  virtual bool IsEnabled(size_t multiIndex) const { return mIsEnabled.GetValue(); }
};




struct MultiLabelSettingItem : public ISettingItem
{
  cc::function<size_t(void*)>::ptr_t mGetItemCount = nullptr;
  cc::function<String(void*,size_t)>::ptr_t mText = nullptr;
  cc::function<bool(void*,size_t)>::ptr_t mIsEnabled = nullptr;
  void* mCapture = nullptr;

  MultiLabelSettingItem(cc::function<size_t(void*)>::ptr_t getItemCount, cc::function<String(void*, size_t)>::ptr_t text, cc::function<bool(void*, size_t)>::ptr_t isEnabled, void* capture) :
    mGetItemCount(getItemCount),
    mText(text),
    mIsEnabled(isEnabled),
    mCapture(capture)
  {
  }

  virtual size_t GetMultiCount() { return mGetItemCount(mCapture); }
  virtual String GetName(size_t multiIndex) { return mText(mCapture, multiIndex); }
  virtual String GetValueString(size_t multiIndex) { return ""; }
  virtual SettingItemType GetType(size_t multiIndex) { return SettingItemType::Custom; }
  virtual bool IsEnabled(size_t multiIndex) const { return mIsEnabled(mCapture, multiIndex); }
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

  virtual void CommitEditing() override {
    mpCurrentEditor = nullptr;
  }
  
  virtual void DisplayAppOnSelected() override {
    gSettingsMenuNavDepth = 0;
    gSettingsMenuNavStack[0].focusedItem = 0;
    gSettingsMenuNavStack[0].pList = GetRootSettingsList();
    DisplayApp::DisplayAppOnSelected();
  }

  virtual CCDisplay* GetDisplay() override { return &mDisplay; }
  virtual AppSettings* GetAppSettings() override { return mAppSettings; }
  //virtual IControlMapper* GetControlMapper() override { return mControlMapper; }
  virtual InputDelegator* GetInputDelegator() override { return mInput; }


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

    mDisplay.ClearState();

    size_t lineHeight = mDisplay.mDisplay.GetLineHeight();
    size_t maxItemsToRender = (mDisplay.mDisplay.height() + (lineHeight / 2)) / lineHeight;
    size_t itemsToRender = min(maxItemsToRender, state.pList->Count());
    int focusedItemScreenPos = maxItemsToRender / 3; // estimated... whatever.

    size_t itemToRender = AddConstrained(state.focusedItem, -focusedItemScreenPos, 0, state.pList->Count() - 1);
    //Serial.println(String("focuseditem = ") + state.focusedItem + " itemsFromTop=" + itemsFromTop + "plist count =" + state.pList->Count());
    for (size_t i = 0; i < itemsToRender; ++ i) {
      size_t multiIndex = 0;
      auto* item = state.pList->GetItem(itemToRender, multiIndex);
      if (itemToRender == (size_t)state.focusedItem) {
        mDisplay.mDisplay.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
      } else {
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

      itemToRender = (itemToRender + 1) % state.pList->Count();
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
    if (state.pList->Count() <= 1) {
      state.focusedItem = 0;
    }
    else {
      state.focusedItem = AddConstrained(state.focusedItem, mEnc.GetIntDelta(), 0, state.pList->Count() - 1);
    }
    
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
