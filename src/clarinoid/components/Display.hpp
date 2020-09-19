
#pragma once

#include <clarinoid/basic/Basic.hpp>

#include "AdafruitSSD1366Wrapper.hpp"

//////////////////////////////////////////////////////////////////////
class MenuAppBase
{
public:
  virtual void OnSelected() {};
  virtual void OnUnselected() {}
  virtual void Update() = 0; // called each frame
  virtual void Render() = 0; // called occasionally
};

framerateCalculator gFramerate;

//////////////////////////////////////////////////////////////////////
struct CCDisplay : UpdateObjectT<ProfileObjectType::Display>
{
  CCThrottlerT<17> mThrottle;
  bool mIsSetup = false; // used for crash handling to try and setup this if we can
  bool mFirstMenuAppSelected = false;

  //Adafruit_SSD1306 mDisplay = {OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS };
  CCAdafruitSSD1306 mDisplay;

  cc::array_view<MenuAppBase*> mMenuApps;
   int mCurrentMenuAppIndex = 0;

  void InitMenuApps(const cc::array_view<MenuAppBase*>& arr)
  {
    mMenuApps = arr;
  }

  void SelectApp(int n) {
    n %= mMenuApps.mSize;
    while (n < 0)
      n += mMenuApps.mSize;

    if (n == mCurrentMenuAppIndex)
      return;

    mMenuApps.mData[mCurrentMenuAppIndex]->OnUnselected();
    mCurrentMenuAppIndex = n;
    //Serial.println(String("selecting app ") + n );
    mMenuApps.mData[mCurrentMenuAppIndex]->OnSelected();
  }

  void ScrollApps(int delta) {
    SelectApp(mCurrentMenuAppIndex + delta);
  }

  virtual void setup() {
    // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
    mDisplay.begin(SSD1306_SWITCHCAPVCC);
    mIsSetup = true;
    mDisplay.clearDisplay();
    mDisplay.display();
    mDisplay.dim(gAppSettings.mDisplayDim);

    // welcome msg.
    mDisplay.clearDisplay();
    mDisplay.setTextSize(1);
    mDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
    mDisplay.setCursor(0,0);

    mDisplay.println(gClarinoidVersion);
    mDisplay.display();
    delay(600);
  }

  virtual void loop() {
    MenuAppBase* pMenuApp = nullptr;

    if (mCurrentMenuAppIndex < (int)mMenuApps.mSize) {
      pMenuApp = mMenuApps.mData[mCurrentMenuAppIndex];

      if (!mFirstMenuAppSelected) {
        mFirstMenuAppSelected = true;
        pMenuApp->OnSelected();
      }

      pMenuApp->Update();
    }
    
    if (!mThrottle.IsReady())
     return;

    gFramerate.onFrame();

    mDisplay.clearDisplay();
    mDisplay.mSolidText = true;
    mDisplay.mTextLeftMargin = 0;
    mDisplay.mClipLeft = 0;
    mDisplay.mClipRight = RESOLUTION_X;
    mDisplay.mClipTop = 0;
    mDisplay.mClipBottom = RESOLUTION_Y;
    mDisplay.setTextSize(1);
    mDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
    mDisplay.setCursor(0,0);

    //mDisplay.println(String("working.") + micros() + "\r\n" + gFramerate.getFPS() + " fps");

    if (pMenuApp) {
      pMenuApp->Render();
    }

    if (mIsShowingToast) {
      if (mToastTimer.IsReady()) {
        mIsShowingToast = false;
      } else {
        // render toast.
        SetupModal();
        mDisplay.print(mToastMsg);
      }
    }

     mDisplay.display();
  }

  CCThrottlerT<2000> mToastTimer;
  bool mIsShowingToast = false;
  String mToastMsg;

  void ShowToast(const String& msg) {
    mIsShowingToast = true;
    mToastMsg = msg;
    mToastTimer.Reset();
  }


  // draws & prepares the screen for a modal message. after this just print text whatever.
  inline void SetupModal(int pad = 1, int rectStart = 2, int textStart = 4) {
    mDisplay.setCursor(0,0);
    mDisplay.fillRect(pad, pad, mDisplay.width() - pad, mDisplay.height() - pad, SSD1306_BLACK);
    mDisplay.drawRect(rectStart, rectStart, mDisplay.width() - rectStart, mDisplay.height() - rectStart, SSD1306_WHITE);
    mDisplay.setTextSize(1);
    mDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
    mDisplay.mTextLeftMargin = textStart;
    mDisplay.mClipLeft = textStart;
    mDisplay.mClipRight = RESOLUTION_X - textStart;
    mDisplay.mClipTop = textStart;
    mDisplay.mClipBottom = RESOLUTION_Y - textStart;
    mDisplay.setCursor(textStart, textStart);
  }

};

CCDisplay gDisplay;

