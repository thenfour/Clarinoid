
#ifndef CCMENUDEBUG_H
#define CCMENUDEBUG_H


#include "Shared_CCUtil.h"
#include <functional>

static const int SETTINGS_STACK_MAX_DEPTH = 10;
static const int MAX_SETTING_ITEMS_PER_LIST = 20;

/////////////////////////////////////////////////////////////////////////////////////////////////
// provides utils for menu apps.
// the system works like this: each app has a front page which you scroll through using the encoder. rendered using RenderFrontpage().
// clicking the encoder will enter the app, rendered with Render().
class MenuAppBaseWithUtils : public MenuAppBase
{
  bool mShowingFrontPage = true;
  
protected:
  MenuAppBaseWithUtils()
  {
    gDisplay.AddMenuApp(this);
  }
  
public:
  virtual void UpdateApp() = 0;
  virtual void RenderApp() = 0;
  virtual void RenderFrontPage() = 0;

  bool IsShowingFrontPage() const { return mShowingFrontPage; }

  ICCSwitch& BackButton() const { return gEWIControl.mPhysicalState.key_back; }
  
  virtual void OnSelected() {
    GoToFrontPage();
  }

  void GoToFrontPage() {
    mShowingFrontPage = true;
  }

  int EncoderIntDelta() {
    return gEnc.GetIntDelta();
  }

  virtual void Render() {
    if (!mShowingFrontPage) {
      this->RenderApp();
      return;
    }

    this->RenderFrontPage();
  }

  virtual void Update()
  {
    if (!mShowingFrontPage) {
      this->UpdateApp();
      return;
    }

    gDisplay.ScrollApps(gEnc.GetIntDelta());

    if (gEncButton.IsNewlyPressed()) {
      mShowingFrontPage = false;
    }
  }
};


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
    SetupModal();
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
    SetupModal();
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

  CCThrottlerT<2000> mToastTimer;
  bool mIsShowingToast = false;
  String mToastMsg;

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

  void ShowToast(const String& msg) {
    mIsShowingToast = true;
    mToastMsg = msg;
    mToastTimer.Reset();
  }

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

    if (mIsShowingToast) {
      if (mToastTimer.IsReady()) {
        mIsShowingToast = false;
      } else {
        // render toast.
        SetupModal();
        gDisplay.mDisplay.print(mToastMsg);
      }
    }
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

static bool AlwaysEnabled() { return true; }










class SynthSettingsApp : public SettingsMenuApp
{
  SettingsList mRootList;
  
  FloatSettingItem mPortamentoTime = { mRootList, "Portamento", 0.0f, 0.25f, gAppSettings.mPortamentoTime, AlwaysEnabled };
  IntSettingItem mTranspose = { mRootList, "Transpose", -48, 48, gAppSettings.mTranspose, AlwaysEnabled };
  FloatSettingItem mReverbGain = { mRootList, "Reverb gain", 0.0f, 1.0f, gAppSettings.mReverbGain, AlwaysEnabled };
  
public:
  virtual SettingsList* GetRootSettingsList()
  {
    return &mRootList;
  }

  virtual void RenderFrontPage() 
  {
    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setTextColor(WHITE);
    gDisplay.mDisplay.setCursor(0,0);

    gDisplay.mDisplay.println(String("SYNTH SETTINGS"));
    gDisplay.mDisplay.println(String(""));
    gDisplay.mDisplay.println(String(""));
    gDisplay.mDisplay.println(String("                  -->"));

    SettingsMenuApp::RenderFrontPage();
  }
};

class MetronomeSettingsApp : public SettingsMenuApp
{
  SettingsList mRootList;

public:
  BoolSettingItem mOnOff = { mRootList, "Enabled?", "On", "Off", gAppSettings.mMetronomeOn, AlwaysEnabled };
  FloatSettingItem mBPM = { mRootList, "BPM", 30.0f, 200.0f, gAppSettings.mBPM, AlwaysEnabled };
  FloatSettingItem mGain = { mRootList, "Gain", 0.0f, 1.0f, gAppSettings.mMetronomeGain, [&](){ return gAppSettings.mMetronomeOn; } };
  IntSettingItem mNote = { mRootList, "Note", 20, 120, gAppSettings.mMetronomeNote, [&](){ return gAppSettings.mMetronomeOn; } };
  IntSettingItem mDecay = { mRootList, "Decay", 1, 120, gAppSettings.mMetronomeDecayMS, [&](){ return gAppSettings.mMetronomeOn; } };

  virtual SettingsList* GetRootSettingsList()
  {
    return &mRootList;
  }

  virtual void RenderFrontPage() 
  {
    float beatFloat = gSynth.mMetronomeTimer.GetBeatFloat(60000.0f / gAppSettings.mBPM);
    float beatFrac = beatFloat - floor(beatFloat);
    int beatInt = (int)floor(beatFloat);
    CCPlot(beatInt);
    bool altBeat = (beatInt & 1) != 0;

    bool highlight = beatFrac < 0.1;
    gDisplay.mDisplay.setTextSize(1);
    if (highlight) {
      gDisplay.mDisplay.fillScreen(WHITE);
    }
    gDisplay.mDisplay.setTextColor(highlight ? BLACK : WHITE);
    gDisplay.mDisplay.setCursor(0,0);

    gDisplay.mDisplay.println(String("METRONOME SETTINGS"));
    gDisplay.mDisplay.print(gAppSettings.mMetronomeOn ? "ENABLED" : "disabled");
    gDisplay.mDisplay.println(String(" bpm=") + gAppSettings.mBPM);

    const int r = 4;
    int x = beatFrac * (RESOLUTION_X - r*2);
    if (altBeat)
      x = gDisplay.mDisplay.width() - x;
    gDisplay.mDisplay.fillCircle(x, gDisplay.mDisplay.getCursorY() + r, r, highlight ? BLACK : WHITE);
    
    gDisplay.mDisplay.println(String(""));
    gDisplay.mDisplay.println(String("                  -->"));

    SettingsMenuApp::RenderFrontPage();
  }

  virtual ISettingItemEditor* GetBackEditor() {
    return mBPM.GetEditor();
  }
};

/////////////////////////////////////////////////////////////////////////////////////////////////
class SystemSettingsApp : public SettingsMenuApp
{
  SettingsList mRootList;
  
  TriggerSettingItem mResetKeys = { mRootList, "Reset all keys", [&](){ this->OnResetKeys(); }, AlwaysEnabled };
  BoolSettingItem mDimDisplay = { mRootList, "Display dim?", "Yes", "No", Property<bool>{ [&]() { return gAppSettings.mDisplayDim; },
    [&](const bool& x) { gAppSettings.mDisplayDim = x; gDisplay.mDisplay.dim(x); }}, AlwaysEnabled };

public:

  void OnResetKeys() {
    gApp.SendCmd(CommandFromMain::ResetTouchKeys);
    ShowToast("Reset done");
  }

  virtual SettingsList* GetRootSettingsList()
  {
    return &mRootList;
  }

  virtual void RenderFrontPage() 
  {
    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setTextColor(WHITE);
    gDisplay.mDisplay.setCursor(0,0);

    gDisplay.mDisplay.println(String("SYSTEM SETTINGS"));
    SettingsMenuApp::RenderFrontPage();
  }
};


/////////////////////////////////////////////////////////////////////////////////////////////////
class DebugMenuApp : public MenuAppBaseWithUtils
{
public:
  virtual void RenderFrontPage() {
    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setTextColor(WHITE);
    gDisplay.mDisplay.setCursor(0,0);
    gDisplay.mDisplay.println("DEBUG PAGES");
    gDisplay.mDisplay.println(String("Uptime: ") + (int)((float)millis() / 1000) + " sec");
    gDisplay.mDisplay.println(String("Notes played: ") + gEWIControl.mMusicalState.noteOns);
    gDisplay.mDisplay.println(String("                  -->"));
  }

  int mPage = 0;
  const int pageCount = 8;

  virtual void UpdateApp()
  {
    if (BackButton().IsNewlyPressed()) {
      GoToFrontPage();
      return;
    }
    mPage = AddConstrained(mPage, gEnc.GetIntDelta(), 0, pageCount - 1);
    if (mPage == 0 || mPage == 1) {
      if (gEncButton.IsNewlyPressed()) {
        gLHSerial.mRxSuccess = 0;
        gLHSerial.mChecksumErrors = 0;
        gLHSerial.mSkippedPayloads = 0;
        gRHSerial.mRxSuccess = 0;
        gRHSerial.mChecksumErrors = 0;
        gRHSerial.mSkippedPayloads = 0;
      }
    }
  }
  
  virtual void RenderApp()
  {
    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setTextColor(WHITE);
    gDisplay.mDisplay.setCursor(0,0);

    auto pageRX = [&](CCMainTxRx& rx, const char *title){
      //CCMainTxRx& rx = (gEncButton.IsPressed()) ? gLHSerial : gRHSerial;
      gDisplay.mDisplay.println(String(title) + " ok:" + rx.mRxSuccess + " #" + rx.mReceivedData.serial);
      gDisplay.mDisplay.print(String("Err:") + rx.mChecksumErrors);
      gDisplay.mDisplay.println(String(" Skip: ") + rx.mSkippedPayloads);
      gDisplay.mDisplay.println(String("fps:") + (int)rx.mReceivedData.framerate + " skip%:" + (int)((float)rx.mSkippedPayloads * 100 / max(1,(int)rx.mRxSuccess)));
      gDisplay.mDisplay.println(String("rxfps:") + (int)rx.mRxRate.getFPS());
    };

    auto pageLHRX = [&](){
      pageRX(gLHSerial, "LH");
    };
    
    auto pageRHRX = [&](){
      pageRX(gRHSerial, "RH");
    };
    
    auto pageMusicalState = [&]() {
      gDisplay.mDisplay.println(String("#:") + gEWIControl.mMusicalState.mLiveVoice.mNote + " (" + (gEWIControl.mMusicalState.mLiveVoice.mIsNoteCurrentlyOn ? "ON" : "off" ) + ") " + (int)MIDINoteToFreq(gEWIControl.mMusicalState.mLiveVoice.mNote) + "hz");
      gDisplay.mDisplay.println(String("transpose:") + gAppSettings.mTranspose);
      gDisplay.mDisplay.println(String("breath:") + gEWIControl.mMusicalState.breath01.GetValue());
      gDisplay.mDisplay.print(String("pitch:") + gEWIControl.mMusicalState.pitchBendN11.GetValue());
    };
    
    auto pagePhysicalState = [&]() {
      // LH: k:1234 o:1234 b:12
      // RH: k:1234 b:12
      // wind:0.455 // bite:0.11
      // pitch:
      gDisplay.mDisplay.println(String("LH k:") +
        (gEWIControl.mPhysicalState.key_lh1.IsPressed ? "1" : "-") +
        (gEWIControl.mPhysicalState.key_lh2.IsPressed ? "2" : "-") +
        (gEWIControl.mPhysicalState.key_lh3.IsPressed ? "3" : "-") +
        (gEWIControl.mPhysicalState.key_lh4.IsPressed ? "4" : "-") +
        " o:" +
        (gEWIControl.mPhysicalState.key_octave1.IsPressed ? "1" : "-") +
        (gEWIControl.mPhysicalState.key_octave2.IsPressed ? "2" : "-") +
        (gEWIControl.mPhysicalState.key_octave3.IsPressed ? "3" : "-") +
        (gEWIControl.mPhysicalState.key_octave4.IsPressed ? "4" : "-") +
        " b:" +
        (gEWIControl.mPhysicalState.key_lhExtra1.IsPressed ? "1" : "-") +
        (gEWIControl.mPhysicalState.key_lhExtra2.IsPressed ? "2" : "-"));

      gDisplay.mDisplay.println(String("RH k:") +
        (gEWIControl.mPhysicalState.key_rh1.IsPressed ? "1" : "-") +
        (gEWIControl.mPhysicalState.key_rh2.IsPressed ? "2" : "-") +
        (gEWIControl.mPhysicalState.key_rh3.IsPressed ? "3" : "-") +
        (gEWIControl.mPhysicalState.key_rh4.IsPressed ? "4" : "-") +
        "       " +
        " b:" +
        (gEWIControl.mPhysicalState.key_rhExtra1.IsPressed ? "1" : "-") +
        (gEWIControl.mPhysicalState.key_rhExtra2.IsPressed ? "2" : "-"));
      gDisplay.mDisplay.print(String("breath: ") + gEWIControl.mPhysicalState.breath01 + "  " +
        "bite: " + gEWIControl.mPhysicalState.bite01);
      gDisplay.mDisplay.print(String("  pitch: ") + gEWIControl.mPhysicalState.pitchDown01);
      gDisplay.mDisplay.print(String("  tristate: ") + ToString(gEWIControl.mPhysicalState.key_triState));
    };

    auto pageDebugLHRX = [&]() {
      gDisplay.mDisplay.println(String("LH debug") + ToString(gLHSerial.mReceivedData));
    };

    auto pageDebugRHRX = [&]() {
      gDisplay.mDisplay.println(String("RH debug") + ToString(gRHSerial.mReceivedData));
    };

    auto pageDebugMain = [&]() {
      gDisplay.mDisplay.println(String("Main debug"));
      gDisplay.mDisplay.println(String("Max frame ms: ") + ((float)gLongestLoopMicros / 1000));
      gDisplay.mDisplay.println(String("Max idle ms:  ") + ((float)gLongestBetweenLoopMicros / 1000));
    };
    
    auto pageAudioStatus = [&]() {
      gDisplay.mDisplay.println(String("Memory: ") + AudioMemoryUsage());
      gDisplay.mDisplay.println(String("Memory Max: ") + AudioMemoryUsageMax());
      gDisplay.mDisplay.println(String("CPU: ") + AudioProcessorUsage());
      gDisplay.mDisplay.println(String("CPU Max: ") + AudioProcessorUsageMax());
    };

    switch(mPage) {
      case 0:
        pageLHRX();
        break;
      case 1:
        pageRHRX();
        break;
      case 2:
        pageMusicalState();
        break;
      case 3:
        pagePhysicalState();
        break;
      case 4:
        pageAudioStatus();
        break;
      case 5:
        pageDebugLHRX();
        break;
      case 6:
        pageDebugRHRX();
        break;
      case 7:
        pageDebugMain();
        break;
    }
  }
};

// feed it data and it will plot.
const int DisplayWidth = 128;
const int DisplayHeight = 32;
template<int TseriesCount, int Tspeed> // Tspeed is # of plots per column
struct Plotter
{
  size_t mCursor = 0;
  size_t mValid = 0;
  static constexpr size_t sampleCount = DisplayWidth * Tspeed;
  int32_t vals[TseriesCount][sampleCount];
  
  void clear() {
    mValid = 0;
    mCursor = 0;
  }
  
  void Plot4(uint32_t val1, uint32_t val2, uint32_t val3, uint32_t val4) {
    if (TseriesCount != 4) return;
    vals[0][mCursor] = val1;
    vals[1][mCursor] = val2;
    vals[2][mCursor] = val3;
    vals[3][mCursor] = val4;
    mValid = max(mValid, mCursor);
    mCursor = (mCursor + 1) % sampleCount;
  }
  
  void Plot1(uint32_t val1) {
    if (TseriesCount != 1) return;
    vals[0][mCursor] = val1;
    mValid = max(mValid, mCursor);
    mCursor = (mCursor + 1) % sampleCount;
  }
  
  void Render() {
    // determine min/max for scale.
    if (mValid == 0) return;
    int32_t min_ = vals[0][0];
    int32_t max_ = min_;
    for (size_t x = 0; x < mValid; ++ x) {
      for (size_t s = 0; s < TseriesCount; ++ s) {
        min_ = min(min_, vals[s][x]);
        max_ = max(max_, vals[s][x]);
      }
    }
    if (min_ == max_) max_ ++; // avoid div0

    // draw back from cursor.
    for (int n = 0; n < (int)mValid; ++ n) {
      int x = ((sampleCount - n) / Tspeed) - 1;
      int i = (int)mCursor - n - 1;
      if (i < 0)
        i += sampleCount;
      for (size_t s = 0; s < TseriesCount; ++ s) {
        uint32_t y = map(vals[s][i], min_, max_, DisplayHeight - 1, 0);
        gDisplay.mDisplay.drawPixel(x, y, WHITE);
      }
    }
    
  }
};

class TouchKeyGraphs : public MenuAppBaseWithUtils
{
  int mKeyIndex = 0;
  Plotter<1, 2> mPlotter;
  CCThrottlerT<7> mThrottle;
  bool isPlaying = true;

  virtual void RenderFrontPage() {
    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setTextColor(WHITE);
    gDisplay.mDisplay.setCursor(0,0);
    gDisplay.mDisplay.println(String("TOUCH KEYS STATE"));
    gDisplay.mDisplay.println(String("LH k:") +
      (gEWIControl.mPhysicalState.key_lh1.IsPressed ? "1" : "-") +
      (gEWIControl.mPhysicalState.key_lh2.IsPressed ? "2" : "-") +
      (gEWIControl.mPhysicalState.key_lh3.IsPressed ? "3" : "-") +
      (gEWIControl.mPhysicalState.key_lh4.IsPressed ? "4" : "-") +
      " o:" +
      (gEWIControl.mPhysicalState.key_octave1.IsPressed ? "1" : "-") +
      (gEWIControl.mPhysicalState.key_octave2.IsPressed ? "2" : "-") +
      (gEWIControl.mPhysicalState.key_octave3.IsPressed ? "3" : "-") +
      (gEWIControl.mPhysicalState.key_octave4.IsPressed ? "4" : "-") +
      " b:" +
      (gEWIControl.mPhysicalState.key_lhExtra1.IsPressed ? "1" : "-") +
      (gEWIControl.mPhysicalState.key_lhExtra2.IsPressed ? "2" : "-"));

    gDisplay.mDisplay.println(String("RH k:") +
      (gEWIControl.mPhysicalState.key_rh1.IsPressed ? "1" : "-") +
      (gEWIControl.mPhysicalState.key_rh2.IsPressed ? "2" : "-") +
      (gEWIControl.mPhysicalState.key_rh3.IsPressed ? "3" : "-") +
      (gEWIControl.mPhysicalState.key_rh4.IsPressed ? "4" : "-") +
      "       " +
      " b:" +
      (gEWIControl.mPhysicalState.key_rhExtra1.IsPressed ? "1" : "-") +
      (gEWIControl.mPhysicalState.key_rhExtra2.IsPressed ? "2" : "-"));

  }
  virtual void RenderApp() {
    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setTextColor(WHITE);
    gDisplay.mDisplay.setCursor(0,0);
    gDisplay.mDisplay.println(gKeyDesc[mKeyIndex].mName);
    mPlotter.Render();
  }
  virtual void UpdateApp() {
    if (BackButton().IsNewlyPressed()) {
      GoToFrontPage();
      return;
    }

    gTouchKeyGraphsIsRunning = true;
    
    if (gEncButton.IsNewlyPressed()) {
      isPlaying = !isPlaying;
    }

    if (mThrottle.IsReady() && isPlaying) {
      LHRHChecksummablePayload* pData = nullptr;
      if (mKeyIndex == gRHSerial.mReceivedData.data.focusedKey) {
        pData = &gRHSerial.mReceivedData.data;
      } else if (mKeyIndex == gLHSerial.mReceivedData.data.focusedKey) {
        pData = &gLHSerial.mReceivedData.data;
      }
      if (pData) {
        mPlotter.Plot1(pData->focusedTouchReadMicros);//,
//          pData->focusedTouchReadValue,
//          pData->focusedTouchReadUntouchedMicros,
//          pData->focusedTouchReadThresholdMicros);
      }
    }

    auto oldI = mKeyIndex;
    mKeyIndex = AddConstrained(mKeyIndex, gEnc.GetIntDelta(), 0, SizeofStaticArray(gKeyDesc) - 1);
    gApp.FocusKeyDebug(mKeyIndex);
    if (mKeyIndex != oldI) {
      mPlotter.clear();
    }
  }
};


#endif
