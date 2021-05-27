
// This is basically a clarinoid-specific "controller" for a display.

#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include "clarinoid/components/AdafruitSSD1366Wrapper.hpp"
#include <clarinoid/application/ControlMapper.hpp>

namespace clarinoid
{

static constexpr int TOAST_DURATION_MILLIS = 2000;

//////////////////////////////////////////////////////////////////////
struct IDisplayApp
{
  virtual void DisplayAppInit() {}
  virtual void DisplayAppOnSelected() {}
  virtual void DisplayAppOnUnselected() {}
  virtual void DisplayAppUpdate() = 0; // called to update internal state.
  virtual void DisplayAppRender() = 0; // called to render to display.
  virtual const char *DisplayAppGetName() = 0;
};

//////////////////////////////////////////////////////////////////////
struct CCDisplay
{
private:
  static CCAdafruitSSD1306* gDisplay; // this is only to allow the crash handler to output to the screen. not for app use in general.

public:

  CCAdafruitSSD1306 mDisplay;

  AppSettings* mAppSettings;
  InputDelegator* mInput;

  bool mIsSetup = false; // used for crash handling to try and setup this if we can
  bool mFirstAppSelected = false;

  array_view<IDisplayApp*> mApps;
  int mCurrentAppIndex = 0;

  // hardware SPI
  CCDisplay(uint8_t w, uint8_t h, SPIClass *spi, int8_t dc_pin, int8_t rst_pin, int8_t cs_pin, uint32_t bitrate = 8000000UL) :
    mDisplay(w, h, spi, dc_pin, rst_pin, cs_pin, bitrate)// 128, 64, &SPI, 9/*DC*/, 8/*RST*/, 10/*CS*/, 44 * 1000000UL)
  {
    //Serial.println(String("Display ctor, control mapper = ") + ((uintptr_t)(&mControlMapper)));
  }

  // bit-bang
  CCDisplay(uint8_t w, uint8_t h, int8_t mosi_pin, int8_t sclk_pin, int8_t dc_pin, int8_t rst_pin, int8_t cs_pin) :
    mDisplay(w, h, mosi_pin, sclk_pin, dc_pin, rst_pin, cs_pin)
  {
    //Serial.println(String("Display ctor, control mapper = ") + ((uintptr_t)(&mControlMapper)));
    //Init();
  }

  void Init(AppSettings* appSettings, InputDelegator* input, const array_view<IDisplayApp*>& apps) {
    mAppSettings = appSettings;
    mInput = input;
    mApps = apps;

    // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
    //Serial.println(String("Display init"));
    mDisplay.begin(SSD1306_SWITCHCAPVCC);
    mIsSetup = true;

    gDisplay = &mDisplay;

    pfnCrashHandler = [](){
        Serial.println(gCrashMessage);
        Serial.println(String("Display ptr = ") + (uintptr_t)gDisplay);
        // why doesn't this work???
        // gDisplay->clearDisplay();
        // gDisplay->mSolidText = true;
        // gDisplay->mTextLeftMargin = 0;
        // gDisplay->ResetClip();
        // gDisplay->setTextSize(1);
        // gDisplay->setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
        // gDisplay->setCursor(0,0);
        // gDisplay->println(gCrashMessage);
        // gDisplay->display();
        DefaultCrashHandler();
    };

    mDisplay.dim(mAppSettings->mDisplayDim);

    // welcome msg.
    ClearState();
    mDisplay.clearDisplay();

    mDisplay.println(gClarinoidVersion);
    mDisplay.display();

    for (size_t i = 0; i < mApps.mSize; ++ i)
    {
      mApps.mData[i]->DisplayAppInit();
    }
  }

  void SelectApp(int n) {
    n = RotateIntoRange(n, mApps.mSize);

    if (n == mCurrentAppIndex)
      return;

    mApps.mData[mCurrentAppIndex]->DisplayAppOnUnselected();
    mCurrentAppIndex = n;
    mApps.mData[mCurrentAppIndex]->DisplayAppOnSelected();
  }

  void ScrollApps(int delta) {
    SelectApp(mCurrentAppIndex + delta);
  }

  int mframe = 0;

  // splitting the entire "display actions" into sub tasks:
  // UpdateAndRenderTask which runs state updating and  renders to the DMA
  // DisplayTask which "uploads" to the device. This is a natural separation of things to give the task runner some method of yielding.
  void UpdateAndRenderTask() {
    CCASSERT(this->mIsSetup);

    IDisplayApp* pMenuApp = nullptr;

    if (mCurrentAppIndex < (int)mApps.mSize) {
      pMenuApp = mApps.mData[mCurrentAppIndex];

      if (!mFirstAppSelected) {
        mFirstAppSelected = true;
        pMenuApp->DisplayAppOnSelected();
      }

      pMenuApp->DisplayAppUpdate();
    }

    ClearState();
    mDisplay.clearDisplay();

    if (pMenuApp) {
      //Serial.println(String(" => render ") + pMenuApp->DisplayAppGetName());
      pMenuApp->DisplayAppRender();
    }

    if (mIsShowingToast) {
      if (mToastTimer.ElapsedTime().ElapsedMillisI() >= TOAST_DURATION_MILLIS) {
        mIsShowingToast = false;
      } else {
        // render toast.
        SetupModal();
        mDisplay.print(mToastMsg);
      }
    }

  }

  void DisplayTask()
  {
    //NoInterrupts _ni;
     mDisplay.display();
  }

  Stopwatch mToastTimer;
  bool mIsShowingToast = false;
  String mToastMsg;

  void ShowToast(const String& msg) {
    //Serial.println(String("toast: ") +msg);
    mIsShowingToast = true;
    mToastMsg = msg;
    mToastTimer.Restart();
  }

  void ClearState() {
    mDisplay.mSolidText = true;
    mDisplay.mTextLeftMargin = 0;
    mDisplay.ResetClip();
    mDisplay.setTextSize(1);
    mDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
    mDisplay.setCursor(0,0);
  }

  // draws & prepares the screen for a modal message. after this just print text whatever.
  inline void SetupModal(int pad = 1, int rectStart = 2, int textStart = 4) {
    ClearState();
    mDisplay.fillRect(pad, pad, mDisplay.width() - pad, mDisplay.height() - pad, SSD1306_BLACK);
    mDisplay.drawRect(rectStart, rectStart, mDisplay.width() - rectStart, mDisplay.height() - rectStart, SSD1306_WHITE);
    mDisplay.mTextLeftMargin = textStart;
    mDisplay.ClipToMargin(textStart);
    mDisplay.setCursor(textStart, textStart);
  }

};


CCAdafruitSSD1306* CCDisplay::gDisplay = nullptr;

} // namespace clarinoid
