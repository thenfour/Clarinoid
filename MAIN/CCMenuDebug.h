
#ifndef CCMENUDEBUG_H
#define CCMENUDEBUG_H


#include "Shared_CCUtil.h"

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
    display.AddMenuApp(this);
  }
  
public:
  virtual void UpdateApp() = 0;
  virtual void RenderApp() = 0;
  virtual void RenderFrontPage() = 0;
  
  virtual void OnSelected() {
    GoToFrontPage();
  }

  void GoToFrontPage() {
    mShowingFrontPage = true;
  }

  bool WasBackPressed() {
    return gEWIControl.mPhysicalState.key_back_is_dirty && gEWIControl.mPhysicalState.key_back;
  }
  int EncoderIntDelta() {
    return gEnc.GetIntDelta();
  }
  bool WasEncoderButtonPressed() {
    return gEncButton.IsPressed() && gEncButton.IsDirty();
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

    mCCDisplay.ScrollApps(gEnc.GetIntDelta());

    if (gEncButton.IsPressed()) {
      mShowingFrontPage = false;
    }
  }
};

/////////////////////////////////////////////////////////////////////////////////////////////////
class MetronomeMenuApp : public MenuAppBaseWithUtils
{
public:
  MetronomeMenuApp(CCDisplay& display, CCEWIApp& app) : MenuAppBaseWithUtils(display, app)
  {
  }

  virtual void RenderFrontPage() 
  {
    mDisplay.setTextSize(1);
    mDisplay.setTextColor(WHITE);
    mDisplay.setCursor(0,0);

    mDisplay.println(String("Metronome"));
    mDisplay.println(String("BPM=") + gSynth.mMetronomeBPM);
  }

  virtual void RenderApp()
  {
    mDisplay.setTextSize(1);
    mDisplay.setTextColor(WHITE);
    mDisplay.setCursor(0,0);
    mDisplay.println(String("editing BPM=") + gSynth.mMetronomeBPM);
  }
  virtual void UpdateApp()
  {
    if (gEnc.IsDirty()) {
      gSynth.mMetronomeBPM += gEnc.GetIntDelta();
    }

    if (WasBackPressed()) {
      GoToFrontPage();
      return;
    }

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
  }

  virtual void UpdateApp()
  {
    if (WasBackPressed()) {
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
      mDisplay.println(String("transpose:") + gEWIControl.mTranspose);
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
