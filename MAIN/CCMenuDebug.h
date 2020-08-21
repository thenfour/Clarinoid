
#ifndef CCMENUDEBUG_H
#define CCMENUDEBUG_H


#include "Shared_CCUtil.h"

static const int SETTINGS_STACK_MAX_DEPTH = 10;
static const int MAX_SETTING_ITEMS_PER_LIST = 20;


/////////////////////////////////////////////////////////////////////////////////////////////////
// provides utils for menu apps.
// the system works like this: each app has a front page which you scroll through using the encoder. rendered using RenderFrontpage().
// clicking the encoder will enter the app, rendered with Render().
class MenuAppBaseWithUtils : public MenuAppBase
{
protected:
  CCDisplay& mCCDisplay;
  Adafruit_SSD1306& mDisplay;
  CCEWIApp& mApp;

  bool mShowingFrontPage = true;
  
  MenuAppBaseWithUtils(CCDisplay& display, CCEWIApp& app) :
    mCCDisplay(display),
    mDisplay(display.mDisplay),
    mApp(app)
  {
    //Serial.println("adding an app...");
    display.AddMenuApp(this);
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
      //Serial.println("calling RenderApp");
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

    mCCDisplay.ScrollApps(gEnc.GetIntDelta());

    if (gEncButton.IsNewlyPressed()) {
      //Serial.println("pressed encoder; showing app");
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
  Submenu
};

/////////////////////////////////////////////////////////////////////////////////////////////////
struct ISettingItem
{
  virtual String GetName() = 0;
  virtual String GetValueString() = 0;  
  virtual SettingItemType GetType() = 0;

  virtual void ToggleBool() {} // for bool types
  virtual struct ISettingItemEditor* GetEditor() { return nullptr; } // for custom types
  virtual struct SettingsList* GetSubmenu() { return nullptr; } // for submenu type
};


/////////////////////////////////////////////////////////////////////////////////////////////////
struct ISettingItemEditor : ISettingItemEditorActions
{
  virtual void SetupEditing(ISettingItemEditorActions* papi, int x, int y) = 0;
  virtual void Update(bool backWasPressed, bool encWasPressed, int encIntDelta) = 0; // every frame.
  virtual void Render(Adafruit_SSD1306& mDisplay) = 0; // not every frame.
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
  T& mBinding;
  T oldVal;

  NumericEditor(T min_, T max_, T& binding) :
    mMin(min_),
    mMax(max_),
    mBinding(binding)
  {  
  }

  virtual T Add(T rhs, int encoderIntDelta) = 0;
  virtual void DrawValue(T val, T oldVal, Adafruit_SSD1306& mDisplay) = 0;
  
  virtual void SetupEditing(ISettingItemEditorActions* papi, int x, int y) {
    mpApi = papi;
    if (!mpApi) mpApi = this;
    oldVal = mBinding;
    this->x = x;
    this->y = y;
  }

  // for editing when holding back button. so back button is definitely pressed, and we don't want to bother
  // with encoder button pressed here.
  virtual void UpdateMomentaryMode(int encIntDelta)
  {
    mBinding = constrain(Add(mBinding, encIntDelta), mMin, mMax);
  }
  
  virtual void Update(bool backWasPressed, bool encWasPressed, int encIntDelta)
  {
    mBinding = constrain(Add(mBinding, encIntDelta), mMin, mMax);
    if (backWasPressed) {
      mBinding = oldVal;
      mpApi->CommitEditing();
    }
    if (encWasPressed) {
      mpApi->CommitEditing();
    }
  }
  virtual void Render(Adafruit_SSD1306& mDisplay)
  {
    mDisplay.setCursor(0,0);
    mDisplay.fillRect(2, 2, mDisplay.width() - 2, mDisplay.height() - 2, SSD1306_BLACK);
    mDisplay.drawRect(4, 4, mDisplay.width()-4, mDisplay.height()-4, SSD1306_WHITE);
    mDisplay.setCursor(12,12);
    mDisplay.setTextSize(1);
    mDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
    DrawValue(mBinding, oldVal, mDisplay);
  }
};

struct IntEditor : NumericEditor<int>
{
  IntEditor(int min_, int max_, int& binding) :
    NumericEditor(min_, max_, binding)
  {  
  }
  virtual int Add(int n, int encDelta) {
    return n + encDelta;
  }
  virtual void DrawValue(int n, int oldVal, Adafruit_SSD1306& mDisplay)
  {
    mDisplay.print(String("") + n);
    int delta = n - oldVal;
    mDisplay.print(String(" (") + (delta >= 0 ? "+" : "") + delta + ")");
  }
};

struct FloatEditor : NumericEditor<float>
{
  FloatEditor(float min_, float max_, float& binding) :
    NumericEditor(min_, max_, binding)
  {  
  }
  virtual float Add(float n, int encDelta) {
    return n + (float)encDelta * (mMax - mMin) / 100;
  }
  virtual void DrawValue(float n, float oldVal, Adafruit_SSD1306& mDisplay)
  {
    mDisplay.print(String("") + n);
    float delta = n - oldVal;
    mDisplay.print(String(" (") + (delta >= 0 ? "+" : "") + delta + ")");
  }
};


template<typename T, typename TEditor>
struct NumericSettingItem : public ISettingItem
{
  String mName;
  T& mBinding;
  TEditor mEditor;
  
  NumericSettingItem(const String& name, T min_, T max_, T& binding) :
    mName(name),
    mBinding(binding),
    mEditor(min_, max_, binding)
  {
  }

  virtual String GetName() { return mName; }
  virtual String GetValueString() { return String(mBinding); }
  virtual SettingItemType GetType() { return SettingItemType::Custom; }

  virtual ISettingItemEditor* GetEditor() {
    //Serial.println(String("returning editor @ ") + (uintptr_t)&mEditor);
    return &mEditor;
  }
};

using IntSettingItem = NumericSettingItem<int, IntEditor>;
using FloatSettingItem = NumericSettingItem<float, FloatEditor>;



struct BoolSettingItem : public ISettingItem
{
  String mName;
  bool& mBinding;
  String mTrueCaption;
  String mFalseCaption;
  
  BoolSettingItem(const String& name, const String& trueCaption, const String& falseCaption, bool& binding) :
    mName(name),
    mBinding(binding),
    mTrueCaption(trueCaption),
    mFalseCaption(falseCaption)
  {
  }

  virtual String GetName() { return mName; }
  virtual String GetValueString() { return mBinding ? mTrueCaption : mFalseCaption; }
  virtual SettingItemType GetType() { return SettingItemType::Bool; }
  virtual void ToggleBool() {
    mBinding = !mBinding;
  }
};


struct SettingsList
{
  int mItemCount;
  ISettingItem* mItems[MAX_SETTING_ITEMS_PER_LIST];
  
  int Count() const { return mItemCount; }
  ISettingItem* GetItem(int i) { return mItems[i]; }
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

public:
  SettingsMenuApp(CCDisplay& display, CCEWIApp& app) :
    MenuAppBaseWithUtils(display, app)
  {
    //Serial.println("SettingsMenuApp");
  }

  virtual void CommitEditing() {
    mpCurrentEditor = nullptr;
  }
  
  virtual void OnSelected() {
    //Serial.println("settings menu app OnSelected");
    mNavDepth = 0;
    mNav[0].focusedItem = 0;
    mNav[0].pList = GetRootSettingsList();
    GoToFrontPage();
  }

  virtual SettingsList* GetRootSettingsList()  = 0;

  // child is responsible for RenderFrontPage()

  virtual void RenderApp()
  {
    //Serial.println(String("settings app renderApp() navdepth=") + mNavDepth);
    //delay(20);
    SettingsMenuState& state = mNav[mNavDepth];

    //Serial.println(String("RenderSettingsApp  => navdepth=") + mNavDepth + " plist=" + (uintptr_t)state.pList + " focuseditem=" + state.focusedItem + " count=" + state.pList->Count());
    //Serial.println(String("  => navdepth=") + mNavDepth + " focuseditem=" + state.focusedItem + " pList=" + (uintptr_t)state.pList + " count=" + state.pList->Count());

    mDisplay.setTextSize(1);
    mDisplay.setCursor(0,0);
    mDisplay.setTextWrap(false);
    int itemToRender = AddConstrained(state.focusedItem, -1, 0, state.pList->Count() - 1);
    const int maxItemsToRender = 4;
    const int itemsToRender = min(maxItemsToRender, state.pList->Count());
    int i = 0;
    for (; i < itemsToRender; ++ i) {
      auto* item = state.pList->GetItem(itemToRender);
      if (itemToRender == state.focusedItem) {
        mDisplay.setTextSize(1);
        mDisplay.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
      } else {
        mDisplay.setTextSize(1);
        mDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
      }
      mDisplay.println(item->GetName() + " = " + item->GetValueString());
      itemToRender = AddConstrained(itemToRender, 1, 0, state.pList->Count() - 1);
    }

    if (mpCurrentEditor) {
      mpCurrentEditor->Render(mDisplay);
    }
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
      //Serial.println(String("pressed encoder button; get item...") + state.focusedItem);
      auto* focusedItem = state.pList->GetItem(state.focusedItem);
      
      switch(focusedItem->GetType()) {
        case SettingItemType::Bool:
          focusedItem->ToggleBool();
          break;
        case SettingItemType::Custom:
          //Serial.println("custom item type; getting editor.");
          mpCurrentEditor = focusedItem->GetEditor();
          mpCurrentEditor->SetupEditing(this, 0, 0);
          break;
        case SettingItemType::Submenu:
          mNavDepth ++;
          mNav[mNavDepth].pList = focusedItem->GetSubmenu();
          mNav[mNavDepth].focusedItem = 0;
          break;
      }
    }

    // back/up when not editing
    if (BackButton().IsNewlyPressed()) 
    {
      //Serial.println("back was pressed");
      if (mNavDepth == 0) {
        GoToFrontPage();
      } else {
        mNavDepth --;
      }
    }
  }
};

class SynthSettingsApp : public SettingsMenuApp
{
  SettingsList mSynthSettingsList;
  FloatSettingItem mPortamentoTime;
  IntSettingItem mTranspose;
  FloatSettingItem mReverbGain;
  
public:
  SynthSettingsApp(CCDisplay& display, CCEWIApp& app) :
    SettingsMenuApp(display, app),
    mPortamentoTime("Portamento", 0.0f, 0.25f, gAppSettings.mPortamentoTime),
    mTranspose("Transpose", -48, 48, gAppSettings.mTranspose),
    mReverbGain("Reverb gain", 0.0f, 1.0f, gAppSettings.mReverbGain)
  {
    mSynthSettingsList.mItemCount = 3;
    mSynthSettingsList.mItems[0] = &mPortamentoTime;
    mSynthSettingsList.mItems[1] = &mTranspose;
    mSynthSettingsList.mItems[2] = &mReverbGain;
  }

  virtual SettingsList* GetRootSettingsList()
  {
    return &mSynthSettingsList;
  }

  virtual void RenderFrontPage() 
  {
    mDisplay.setTextSize(1);
    mDisplay.setTextColor(WHITE);
    mDisplay.setCursor(0,0);

    mDisplay.println(String("SYNTH SETTINGS"));
    mDisplay.println(String(""));
    mDisplay.println(String(""));
    mDisplay.println(String("                  -->"));
  }
};

class MetronomeSettingsApp : public SettingsMenuApp
{
  SettingsList mRootList;
  
  BoolSettingItem mOnOff;
  FloatSettingItem mBPM;
  FloatSettingItem mGain;
  IntSettingItem mNote;
  IntSettingItem mDecay;

  ISettingItemEditor* mpCurrentEditor = nullptr;

public:
  MetronomeSettingsApp(CCDisplay& display, CCEWIApp& app) :
    SettingsMenuApp(display, app),
    mOnOff("Enabled?", "On", "Off", gAppSettings.mMetronomeOn),
    mBPM("BPM", 30.0f, 200.0f, gAppSettings.mPerfSettings.mBPM),
    mGain("Gain", 0.0f, 1.0f, gAppSettings.mMetronomeGain),
    mNote("Note", 20, 120, gAppSettings.mMetronomeNote),
    mDecay("Decay", 1, 120, gAppSettings.mMetronomeDecayMS)
  {
    mRootList.mItemCount = 5;
    mRootList.mItems[0] = &mOnOff;
    mRootList.mItems[1] = &mBPM;
    mRootList.mItems[2] = &mGain;
    mRootList.mItems[3] = &mNote;
    mRootList.mItems[4] = &mDecay;
  }

  virtual SettingsList* GetRootSettingsList()
  {
//    Serial.println(String("met app returning root list ") + (uintptr_t)&mRootList);
    return &mRootList;
  }

  virtual void RenderFrontPage() 
  {
    mDisplay.setTextSize(1);
    mDisplay.setTextColor(WHITE);
    mDisplay.setCursor(0,0);

    mDisplay.println(String("METRONOME SETTINGS"));
    mDisplay.println(gAppSettings.mMetronomeOn ? "ENABLED" : "disabled");
    mDisplay.println(String("bpm=") + gAppSettings.mPerfSettings.mBPM);
    mDisplay.println(String("                  -->"));

    if (mpCurrentEditor) {
      mpCurrentEditor->Render(mDisplay);
    }
  }
  
  virtual void Update()
  {
    if (IsShowingFrontPage()) {
//      //CCPlot(String("update app") + millis());
      if (BackButton().IsNewlyPressed()) {
        mpCurrentEditor = mBPM.GetEditor();
        mpCurrentEditor->SetupEditing(nullptr, 0, 0);
        return;
      } else if (BackButton().IsCurrentlyPressed() && mpCurrentEditor) {
        mpCurrentEditor->UpdateMomentaryMode(EncoderIntDelta());
        return;
      }
      mpCurrentEditor = nullptr;
    }
    SettingsMenuApp::Update();    
  }
};


/////////////////////////////////////////////////////////////////////////////////////////////////
class DebugMenuApp : public MenuAppBaseWithUtils
{
public:
  DebugMenuApp(CCDisplay& display, CCEWIApp& app) : MenuAppBaseWithUtils(display, app)
  {
  }

  virtual void RenderFrontPage() {
    mDisplay.setTextSize(1);
    mDisplay.setTextColor(WHITE);
    mDisplay.setCursor(0,0);
    mDisplay.println("SYSTEM STATUS");
    mDisplay.println(String("Uptime: ") + (int)((float)millis() / 1000) + " sec");
    mDisplay.println(String("Notes played: ") + gEWIControl.mMusicalState.noteOns);
    mDisplay.println(String("                  -->"));
  }

  virtual void UpdateApp()
  {
    if (BackButton().IsNewlyPressed()) {
      GoToFrontPage();
      return;
    }
  }
  
  virtual void RenderApp()
  {
    mDisplay.setTextSize(1);
    mDisplay.setTextColor(WHITE);
    mDisplay.setCursor(0,0);

    auto pageRX = [&](CCMainTxRx& rx, const char *title){
      //CCMainTxRx& rx = (gEncButton.IsPressed()) ? gLHSerial : gRHSerial;
      mDisplay.println(String(title) + " ok:" + rx.mRxSuccess + " #" + rx.mReceivedData.serial);
      mDisplay.print(String("Err:") + rx.mChecksumErrors);
      mDisplay.println(String(" Skip: ") + rx.mSkippedPayloads);
      mDisplay.println(String("fps:") + (int)rx.mReceivedData.framerate + "  tx:" + rx.mTXSerial);
      mDisplay.println(String("myfps:") + (int)gFramerate.getFPS());
    };

    auto pageLHRX = [&](){
      pageRX(gLHSerial, "LH");
    };
    
    auto pageRHRX = [&](){
      pageRX(gRHSerial, "RH");
    };
    
    auto pageMusicalState = [&]() {
      mDisplay.println(String("#:") + gEWIControl.mMusicalState.MIDINote + " (" + (gEWIControl.mMusicalState.isPlayingNote ? "ON" : "off" ) + ") " + (int)MIDINoteToFreq(gEWIControl.mMusicalState.MIDINote) + "hz");
      mDisplay.println(String("transpose:") + gAppSettings.mTranspose);
      mDisplay.println(String("breath:") + gEWIControl.mMusicalState.breath01.GetValue());
      mDisplay.print(String("pitch:") + gEWIControl.mMusicalState.pitchBendN11.GetValue());
    };
    
    auto pagePhysicalState = [&]() {
      // LH: k:1234 o:1234 b:12
      // RH: k:1234 b:12
      // wind:0.455 // bite:0.11
      // pitch:
      mDisplay.println(String("LH k:") +
        (gEWIControl.mPhysicalState.key_lh1 ? "1" : "-") +
        (gEWIControl.mPhysicalState.key_lh2 ? "2" : "-") +
        (gEWIControl.mPhysicalState.key_lh3 ? "3" : "-") +
        (gEWIControl.mPhysicalState.key_lh4 ? "4" : "-") +
        " o:" +
        (gEWIControl.mPhysicalState.key_octave1 ? "1" : "-") +
        (gEWIControl.mPhysicalState.key_octave2 ? "2" : "-") +
        (gEWIControl.mPhysicalState.key_octave3 ? "3" : "-") +
        (gEWIControl.mPhysicalState.key_octave4 ? "4" : "-") +
        " b:" +
        (gEWIControl.mPhysicalState.key_lhExtra1 ? "1" : "-") +
        (gEWIControl.mPhysicalState.key_lhExtra2 ? "2" : "-"));

      mDisplay.println(String("RH k:") +
        (gEWIControl.mPhysicalState.key_rh1 ? "1" : "-") +
        (gEWIControl.mPhysicalState.key_rh2 ? "2" : "-") +
        (gEWIControl.mPhysicalState.key_rh3 ? "3" : "-") +
        (gEWIControl.mPhysicalState.key_rh4 ? "4" : "-") +
        "       " +
        " b:" +
        (gEWIControl.mPhysicalState.key_rhExtra1 ? "1" : "-") +
        (gEWIControl.mPhysicalState.key_rhExtra2 ? "2" : "-"));
      mDisplay.print(String("breath: ") + gEWIControl.mPhysicalState.breath01 + "  " +
        "bite: " + gEWIControl.mPhysicalState.bite01);
      mDisplay.print(String("  pitch: ") + gEWIControl.mPhysicalState.pitchDown01);
      mDisplay.print(String("  tristate: ") + ToString(gEWIControl.mPhysicalState.key_triState));
    };

    auto pageDebugLHRX = [&]() {
      mDisplay.println(String("LH debug") + ToString(gLHSerial.mReceivedData));
    };

    auto pageDebugRHRX = [&]() {
      mDisplay.println(String("RH debug") + ToString(gRHSerial.mReceivedData));
    };

    auto pageDebugMain = [&]() {
      mDisplay.println(String("Main debug"));
      mDisplay.println(String("Max frame ms: ") + ((float)gLongestLoopMicros / 1000));
      mDisplay.println(String("Max idle ms:  ") + ((float)gLongestBetweenLoopMicros / 1000));
    };
    
    auto pageAudioStatus = [&]() {
      mDisplay.println(String("Memory: ") + AudioMemoryUsage());
      mDisplay.println(String("Memory Max: ") + AudioMemoryUsageMax());
      mDisplay.println(String("CPU: ") + AudioProcessorUsage());
      mDisplay.println(String("CPU Max: ") + AudioProcessorUsageMax());
    };

    int page = gEnc.GetIntValue();
    const int pageCount = 8;
    while (page < 0)
      page += pageCount; // makes the % operator work correctly. there's a mathematical optimized way to do this but whatev
    page = page % pageCount;
    switch(page) {
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


#endif
