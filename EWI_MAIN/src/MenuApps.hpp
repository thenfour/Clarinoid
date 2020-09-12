#pragma once

#include "MenuAppBase.hpp"
#include "MenuSettings.hpp"


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
  FloatSettingItem mTouchMaxFactor = { mRootList, "Touch max fact", 1, 2, Property<float> {
    [&]() { return gAppSettings.mTouchMaxFactor; },
    [&](const float& f)
      {
        gAppSettings.mTouchMaxFactor = f;
        gApp.SendCmd(CommandFromMain::SetTouchMaxFactor, f);
      }
  }, AlwaysEnabled };
  
  FloatSettingItem mPitchDownMin = { mRootList, "Pitchdown min", 0, 1, gAppSettings.mPitchDownMin, AlwaysEnabled };
  FloatSettingItem mPitchDownMax = { mRootList, "Pitchdown max", 0, 1, gAppSettings.mPitchDownMax, AlwaysEnabled };

  FloatSettingItem mBreathLowerBound = { mRootList, "Breath inp min", 0, 1, gAppSettings.mBreathLowerBound, AlwaysEnabled };
  FloatSettingItem mBreathUpperBound = { mRootList, "Breath inp max", 0, 1, gAppSettings.mBreathUpperBound, AlwaysEnabled };
  FloatSettingItem mBreathNoteOnThreshold = { mRootList, "Breath note on thresh", 0, 1, gAppSettings.mBreathNoteOnThreshold, AlwaysEnabled };

  BoolSettingItem mDimDisplay = { mRootList, "Display dim?", "Yes", "No", Property<bool>{ [&]() { return gAppSettings.mDisplayDim; },
    [&](const bool& x) { gAppSettings.mDisplayDim = x; gDisplay.mDisplay.dim(x); }}, AlwaysEnabled };

  BoolSettingItem mOrangeLEDs = { mRootList, "Orange LEDs?", "On", "Off", Property<bool>{
    [&]() { return gAppSettings.mOrangeLEDs; },
    [&](const bool& x)
      {
        gAppSettings.mOrangeLEDs = x;
        gApp.SendCmd(x ? CommandFromMain::EnableOrangeLED : CommandFromMain::DisableOrangeLED);
      }
    }, AlwaysEnabled };

public:

  void OnResetKeys() {
    gApp.SendCmd(CommandFromMain::ResetTouchKeys);
    gDisplay.ShowToast("Reset done");
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
  const int pageCount = 6;

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
      gDisplay.mDisplay.println(String("#:") + gEWIControl.mMusicalState.mLiveVoice.mMidiNote + " (" + (gEWIControl.mMusicalState.mLiveVoice.mIsNoteCurrentlyOn ? "ON" : "off" ) + ") " + (int)MIDINoteToFreq(gEWIControl.mMusicalState.mLiveVoice.mMidiNote) + "hz");
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
        (gEWIControl.mPhysicalState.key_lh1.IsCurrentlyPressed() ? "1" : "-") +
        (gEWIControl.mPhysicalState.key_lh2.IsCurrentlyPressed() ? "2" : "-") +
        (gEWIControl.mPhysicalState.key_lh3.IsCurrentlyPressed() ? "3" : "-") +
        (gEWIControl.mPhysicalState.key_lh4.IsCurrentlyPressed() ? "4" : "-") +
        " o:" +
        (gEWIControl.mPhysicalState.key_octave1.IsCurrentlyPressed() ? "1" : "-") +
        (gEWIControl.mPhysicalState.key_octave2.IsCurrentlyPressed() ? "2" : "-") +
        (gEWIControl.mPhysicalState.key_octave3.IsCurrentlyPressed() ? "3" : "-") +
        (gEWIControl.mPhysicalState.key_octave4.IsCurrentlyPressed() ? "4" : "-") +
        " b:" +
        (gEWIControl.mPhysicalState.key_lhExtra1.IsCurrentlyPressed() ? "1" : "-") +
        (gEWIControl.mPhysicalState.key_lhExtra2.IsCurrentlyPressed() ? "2" : "-"));

      gDisplay.mDisplay.println(String("RH k:") +
        (gEWIControl.mPhysicalState.key_rh1.IsCurrentlyPressed() ? "1" : "-") +
        (gEWIControl.mPhysicalState.key_rh2.IsCurrentlyPressed() ? "2" : "-") +
        (gEWIControl.mPhysicalState.key_rh3.IsCurrentlyPressed() ? "3" : "-") +
        (gEWIControl.mPhysicalState.key_rh4.IsCurrentlyPressed() ? "4" : "-") +
        "       " +
        " b:" +
        (gEWIControl.mPhysicalState.key_rhExtra1.IsCurrentlyPressed() ? "1" : "-") +
        (gEWIControl.mPhysicalState.key_rhExtra2.IsCurrentlyPressed() ? "2" : "-"));
      gDisplay.mDisplay.print(String("breath: ") + gEWIControl.mPhysicalState.breath01 + "  " +
        "bite: " + gEWIControl.mPhysicalState.bite01);
      gDisplay.mDisplay.print(String("  pitch: ") + gEWIControl.mPhysicalState.pitchDown01);
      gDisplay.mDisplay.print(String("  tristate: ") + ToString(gEWIControl.mPhysicalState.key_triState));
    };

//    auto pageDebugLHRX = [&]() {
//      gDisplay.mDisplay.println(String("LH debug") + ToString(gLHSerial.mReceivedData));
//    };
//
//    auto pageDebugRHRX = [&]() {
//      gDisplay.mDisplay.println(String("RH debug") + ToString(gRHSerial.mReceivedData));
//    };

    auto pageDebugMain = [&]() {
      gDisplay.mDisplay.println(String("Max frame ms: ") + ((float)gLongestLoopMicros / 1000));
      gDisplay.mDisplay.println(String("Max idle ms:  ") + ((float)gLongestBetweenLoopMicros / 1000));
      gDisplay.mDisplay.println(String("Framerate: ") + (int)gFramerate.getFPS());
      gDisplay.mDisplay.println(String("UpdateObjects:") + gUpdateObjectCount);
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
//      case 5:
//        pageDebugLHRX();
//        break;
//      case 6:
//        pageDebugRHRX();
//        break;
      case 5:
        pageDebugMain();
        break;
    }
  }
};

// feed it data and it will plot.
// supports also 1 "boolean" series which plots solid when true, nothing when off.
const int DisplayWidth = 128;
const int DisplayHeight = 32;
template<int TseriesCount, int Tspeed> // Tspeed is # of plots per column
struct Plotter
{
  size_t mCursor = 0;
  size_t mValid = 0;
  static constexpr size_t sampleCount = DisplayWidth * Tspeed;
  int32_t vals[TseriesCount][sampleCount];
  bool boolVals[sampleCount];

  Plotter() {
    for (auto& b : boolVals) {
      b = false;      
    }
  }
  
  void clear() {
    mValid = 0;
    mCursor = 0;
  }
  
  void Plot4(uint32_t val1, uint32_t val2, uint32_t val3, uint32_t val4) {
    CCASSERT(TseriesCount == 4);
    vals[0][mCursor] = val1;
    vals[1][mCursor] = val2;
    vals[2][mCursor] = val3;
    vals[3][mCursor] = val4;
    mValid = max(mValid, mCursor);
    mCursor = (mCursor + 1) % sampleCount;
  }
  
  void Plot3b(uint32_t val1, uint32_t val2, uint32_t val3, bool boolVal) {
    CCASSERT(TseriesCount == 3);
    vals[0][mCursor] = val1;
    vals[1][mCursor] = val2;
    vals[2][mCursor] = val3;
    boolVals[mCursor] = boolVal;
    mValid = max(mValid, mCursor);
    mCursor = (mCursor + 1) % sampleCount;
  }
  
  void Plot1(uint32_t val1) {
    CCASSERT(TseriesCount == 1);
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

      // plot bool val
      if (boolVals[i]) {
        gDisplay.mDisplay.DrawDottedRect(x, 0, 1, RESOLUTION_Y, WHITE);
      }
    }
    
  }
};

class TouchKeyGraphs : public MenuAppBaseWithUtils
{
  int mKeyIndex = 0;
  Plotter<3, 2> mPlotter;
  CCThrottlerT<5> mThrottle;
  bool isPlaying = true;

  virtual void RenderFrontPage() {
    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setTextColor(WHITE);
    gDisplay.mDisplay.setCursor(0,0);
    gDisplay.mDisplay.println(String("TOUCH KEYS STATE"));
    gDisplay.mDisplay.println(String("LH k:") +
      (gEWIControl.mPhysicalState.key_lh1.IsCurrentlyPressed() ? "1" : "-") +
      (gEWIControl.mPhysicalState.key_lh2.IsCurrentlyPressed() ? "2" : "-") +
      (gEWIControl.mPhysicalState.key_lh3.IsCurrentlyPressed() ? "3" : "-") +
      (gEWIControl.mPhysicalState.key_lh4.IsCurrentlyPressed() ? "4" : "-") +
      " o:" +
      (gEWIControl.mPhysicalState.key_octave1.IsCurrentlyPressed() ? "1" : "-") +
      (gEWIControl.mPhysicalState.key_octave2.IsCurrentlyPressed() ? "2" : "-") +
      (gEWIControl.mPhysicalState.key_octave3.IsCurrentlyPressed() ? "3" : "-") +
      (gEWIControl.mPhysicalState.key_octave4.IsCurrentlyPressed() ? "4" : "-") +
      " b:" +
      (gEWIControl.mPhysicalState.key_lhExtra1.IsCurrentlyPressed() ? "1" : "-") +
      (gEWIControl.mPhysicalState.key_lhExtra2.IsCurrentlyPressed() ? "2" : "-"));

    gDisplay.mDisplay.println(String("RH k:") +
      (gEWIControl.mPhysicalState.key_rh1.IsCurrentlyPressed() ? "1" : "-") +
      (gEWIControl.mPhysicalState.key_rh2.IsCurrentlyPressed() ? "2" : "-") +
      (gEWIControl.mPhysicalState.key_rh3.IsCurrentlyPressed() ? "3" : "-") +
      (gEWIControl.mPhysicalState.key_rh4.IsCurrentlyPressed() ? "4" : "-") +
      "       " +
      " b:" +
      (gEWIControl.mPhysicalState.key_rhExtra1.IsCurrentlyPressed() ? "1" : "-") +
      (gEWIControl.mPhysicalState.key_rhExtra2.IsCurrentlyPressed() ? "2" : "-"));

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
        mPlotter.Plot3b(pData->focusedTouchReadMicros,
          pData->focusedTouchReadUntouchedMicros,
          pData->focusedTouchReadThresholdMicros,
          gEWIControl.mPhysicalState.mOrderedKeys[mKeyIndex]->IsCurrentlyPressed()
          );
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


enum class ProfileMenuMode
{
  Loop,
  Render,
  Update,
  Total
};
struct ProfileTimingsMenuList : IList
{
  ProfileMenuMode mMode = ProfileMenuMode::Total;
  
  virtual int List_GetItemCount() const {
    return gProfileObjectTypeCount;
  }
  virtual String List_GetItemCaption(int i) const {
    CCASSERT(i >= 0 && i < (int)gProfileObjectTypeCount);
    String ret = gProfileObjectTypeItems[i].mName;
    ret += " ";

    auto& total = gProfiler.mTimings[(size_t)(ProfileObjectType::Total)];
    auto& my = gProfiler.mTimings[(size_t)i];

    uint64_t t = 1, m = 0; // total timing & my timing
    switch(mMode) {
    case ProfileMenuMode::Loop: // show as % of total
      t = total.mLoopMillis;
      m = my.mLoopMillis;
      break;
    case ProfileMenuMode::Render: // show as % of total
      t = total.mRenderMillis;
      m = my.mRenderMillis;
      break;
    case ProfileMenuMode::Update: // show as % of total
      t = total.mUpdateMillis;
      m = my.mUpdateMillis;
      break;
    case ProfileMenuMode::Total: // show as % of total
      t = total.mLoopMillis + total.mRenderMillis + total.mUpdateMillis;
      m = my.mLoopMillis + my.mRenderMillis + my.mUpdateMillis;
      break;
    }

    double d = m;
    d /= t;
    d *= 100;// percent

    ret += d;
    ret += "%";

    return ret;
  }
};

struct ProfileMenuApp : public MenuAppBaseWithUtils
{
  ProfileTimingsMenuList mListAdapter;
  ListControl mList;
  int mSelectedItem = 0;

  ProfileMenuApp() :
    mList(&mListAdapter, mSelectedItem, 0, 0, 4)
  {}
  
  virtual void RenderFrontPage() {
    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setTextColor(WHITE);
    gDisplay.mDisplay.setCursor(0,0);
    gDisplay.mDisplay.println("Profiler -->");
  }
  virtual void RenderApp() {
    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setTextColor(WHITE);
    gDisplay.mDisplay.setCursor(0,0);
    mList.Render();
  }
  virtual void UpdateApp() {
    if (BackButton().IsNewlyPressed()) {
      GoToFrontPage();
      return;
    }
    if (gEncButton.IsNewlyPressed()) {
      // cycle mode
      switch(mListAdapter.mMode) {
      case ProfileMenuMode::Total:
        mListAdapter.mMode = ProfileMenuMode::Loop;
        gDisplay.ShowToast("Loop");
        break;
      case ProfileMenuMode::Loop:
        mListAdapter.mMode = ProfileMenuMode::Render;
        gDisplay.ShowToast("Render");
        break;
      case ProfileMenuMode::Render:
        mListAdapter.mMode = ProfileMenuMode::Update;
        gDisplay.ShowToast("Update");
        break;
      case ProfileMenuMode::Update:
        mListAdapter.mMode = ProfileMenuMode::Total;
        gDisplay.ShowToast("Total");
        break;
      }
    }
    mList.Update();
  }
};





struct HarmVoiceSettingsApp
{
  HarmVoiceSettings* mpBinding = nullptr;
  SettingsList mList;

  EnumSettingItem<HarmVoiceType> mVoiceType = { mList, "Voice Type", gHarmVoiceTypeInfo, mpBinding->mVoiceType, AlwaysEnabled };
  IntSettingItem mInterval = { mList, "Interval", -48, 48, MakePropertyByCasting<int>(mpBinding->mSequence[0].mInterval), AlwaysEnabled };

//  HarmVoiceSequenceEntry mSequence[HARM_SEQUENCE_LEN];
//  GlobalLocal mSynthPresetRef = GlobalLocal::Global;
//  int mLocalSynthPreset = 0;
//  GlobalLocal mScaleRef = GlobalLocal::Global;
//  Scale mLocalScale;
//  IntSettingItem mMinOutpNote;
//  IntSettingItem mMaxOutpNote;
//  NoteOOBBehavior mNoteOOBBehavior = NoteOOBBehavior::Drop;
//  NonDiatonicBehavior mNonDiatonicBehavior = NonDiatonicBehavior::Drop;
//  IntSettingItem mMinOutpVel;
//  IntSettingItem mMaxOutpVel;
// midi channel
  
  HarmVoiceSettingsApp(HarmVoiceSettings& binding) :
    mpBinding(&binding)
  {
  }
};

struct HarmPatchSettingsApp
{
  HarmPreset* mpBinding = nullptr;
  HarmPatchSettingsApp(HarmPreset& binding) :
    mpBinding(&binding)
  {
  }

  SettingsList mList;
  HarmVoiceSettingsApp mVoiceSettings[HARM_VOICES] = {
    { mpBinding->mVoiceSettings[0] },
    { mpBinding->mVoiceSettings[1] },
    { mpBinding->mVoiceSettings[2] },
    { mpBinding->mVoiceSettings[3] },
    { mpBinding->mVoiceSettings[4] },
    { mpBinding->mVoiceSettings[5] },
  };
  
  BoolSettingItem mIsEnabled = { mList, "Enabled?", "On", "Off", gAppSettings.mHarmSettings.mIsEnabled, AlwaysEnabled };
  SubmenuSettingItem mVoiceSubmenu1 = { mList, "Voice1", &mVoiceSettings[0].mList, AlwaysEnabled };
  SubmenuSettingItem mVoiceSubmenu2 = { mList, "Voice2", &mVoiceSettings[1].mList, AlwaysEnabled };
  SubmenuSettingItem mVoiceSubmenu3 = { mList, "Voice3", &mVoiceSettings[2].mList, AlwaysEnabled };
  SubmenuSettingItem mVoiceSubmenu4 = { mList, "Voice4", &mVoiceSettings[3].mList, AlwaysEnabled };
  SubmenuSettingItem mVoiceSubmenu5 = { mList, "Voice5", &mVoiceSettings[4].mList, AlwaysEnabled };
  SubmenuSettingItem mVoiceSubmenu6 = { mList, "Voice6", &mVoiceSettings[5].mList, AlwaysEnabled };
  IntSettingItem mGlobalSynthPreset = { mList, "Global synth preset", 0, SYNTH_PRESET_COUNT - 1, gAppSettings.mHarmSettings.mGlobalSynthPreset, AlwaysEnabled };
  IntSettingItem mMinRotationTimeMS = { mList, "Min Rotation", 0, 10000, gAppSettings.mHarmSettings.mMinRotationTimeMS, AlwaysEnabled };

//  Scale mGlobalScale;

};



class HarmSettingsApp : public SettingsMenuApp
{
  SettingsList mRootList;
  HarmPatchSettingsApp mPatchSettings[HARM_PRESET_COUNT] = {
    { gAppSettings.mHarmSettings.mPresets[0] },
    { gAppSettings.mHarmSettings.mPresets[1] },
    { gAppSettings.mHarmSettings.mPresets[2] },
    { gAppSettings.mHarmSettings.mPresets[3] },
    { gAppSettings.mHarmSettings.mPresets[4] },
    { gAppSettings.mHarmSettings.mPresets[5] },
    { gAppSettings.mHarmSettings.mPresets[6] },
    { gAppSettings.mHarmSettings.mPresets[7] },
    { gAppSettings.mHarmSettings.mPresets[8] },
    { gAppSettings.mHarmSettings.mPresets[9] },
    { gAppSettings.mHarmSettings.mPresets[10] },
    { gAppSettings.mHarmSettings.mPresets[11] },
    { gAppSettings.mHarmSettings.mPresets[12] },
    { gAppSettings.mHarmSettings.mPresets[13] },
    { gAppSettings.mHarmSettings.mPresets[14] },
    { gAppSettings.mHarmSettings.mPresets[15] },
  };
  
  BoolSettingItem mIsEnabled = { mRootList, "Enabled?", "On", "Off", gAppSettings.mHarmSettings.mIsEnabled, AlwaysEnabled };
  IntSettingItem mGlobalSynthPreset = { mRootList, "Global synth preset", 0, SYNTH_PRESET_COUNT - 1, gAppSettings.mHarmSettings.mGlobalSynthPreset, AlwaysEnabled };
  //Scale mGlobalScale;
  IntSettingItem mMinRotationTimeMS = { mRootList, "Min Rotation", 0, 10000, gAppSettings.mHarmSettings.mMinRotationTimeMS, AlwaysEnabled };

  SubmenuSettingItem mPatchSubmenu00 = { mRootList, "Preset 00", &mPatchSettings[0].mList, AlwaysEnabled };
  SubmenuSettingItem mPatchSubmenu01 = { mRootList, "Preset 01", &mPatchSettings[1].mList, AlwaysEnabled };
  SubmenuSettingItem mPatchSubmenu02 = { mRootList, "Preset 02", &mPatchSettings[2].mList, AlwaysEnabled };
  SubmenuSettingItem mPatchSubmenu03 = { mRootList, "Preset 03", &mPatchSettings[3].mList, AlwaysEnabled };
  SubmenuSettingItem mPatchSubmenu04 = { mRootList, "Preset 04", &mPatchSettings[4].mList, AlwaysEnabled };
  SubmenuSettingItem mPatchSubmenu05 = { mRootList, "Preset 05", &mPatchSettings[5].mList, AlwaysEnabled };
  SubmenuSettingItem mPatchSubmenu06 = { mRootList, "Preset 06", &mPatchSettings[6].mList, AlwaysEnabled };
  SubmenuSettingItem mPatchSubmenu07 = { mRootList, "Preset 07", &mPatchSettings[7].mList, AlwaysEnabled };
  SubmenuSettingItem mPatchSubmenu08 = { mRootList, "Preset 08", &mPatchSettings[8].mList, AlwaysEnabled };
  SubmenuSettingItem mPatchSubmenu09 = { mRootList, "Preset 09", &mPatchSettings[9].mList, AlwaysEnabled };
  SubmenuSettingItem mPatchSubmenu10 = { mRootList, "Preset 10", &mPatchSettings[10].mList, AlwaysEnabled };
  SubmenuSettingItem mPatchSubmenu11 = { mRootList, "Preset 11", &mPatchSettings[11].mList, AlwaysEnabled };
  SubmenuSettingItem mPatchSubmenu12 = { mRootList, "Preset 12", &mPatchSettings[12].mList, AlwaysEnabled };
  SubmenuSettingItem mPatchSubmenu13 = { mRootList, "Preset 13", &mPatchSettings[13].mList, AlwaysEnabled };
  SubmenuSettingItem mPatchSubmenu14 = { mRootList, "Preset 14", &mPatchSettings[14].mList, AlwaysEnabled };
  SubmenuSettingItem mPatchSubmenu15 = { mRootList, "Preset 15", &mPatchSettings[15].mList, AlwaysEnabled };

//  Scale mGlobalScale;

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
    gDisplay.mDisplay.println(String("Harmonizer [") + (gAppSettings.mHarmSettings.mIsEnabled ? "on" : "off") + "]");
    gDisplay.mDisplay.println(String("Scale: "));
    gDisplay.mDisplay.println(String("Preset: "));
    gDisplay.mDisplay.println(String("                  -->"));

    SettingsMenuApp::RenderFrontPage();
  }

};










class LoopSettingsApp : public SettingsMenuApp
{
  SettingsList mRootList;

  TriggerSettingItem mClearAll = { mRootList, "Clear All", [&](){ gEWIControl.mMusicalState.mLooper.Clear(); }, AlwaysEnabled };
  TriggerSettingItem mLoopIt = { mRootList, "Loop it", [&](){ gEWIControl.mMusicalState.mLooper.LoopIt(); }, AlwaysEnabled };

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
    gDisplay.mDisplay.println(String("Looper [") + (gAppSettings.mHarmSettings.mIsEnabled ? "on" : "off") + "]");
    gDisplay.mDisplay.println(String("Scale: "));
    gDisplay.mDisplay.println(String("Preset: "));
    gDisplay.mDisplay.println(String("                  -->"));

    SettingsMenuApp::RenderFrontPage();
  }

};




LoopSettingsApp gLoopSettingsApp;
HarmSettingsApp gHarmSettingsApp;
SystemSettingsApp gSystemSettingsApp;
TouchKeyGraphs gTouchKeyApp;
MetronomeSettingsApp gMetronomeApp;
DebugMenuApp gDebugApp;
SynthSettingsApp gSynthSettingsApp;
ProfileMenuApp gProfileApp;





