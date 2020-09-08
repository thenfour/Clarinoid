
#pragma once

#include <Shared_CCUtil.h>

#include "AdafruitSSD1366Wrapper.hpp"

static constexpr int MAX_MENU_APPS = 20;

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
class CCDisplay : UpdateObjectT<ProfileObjectType::Display>
{
  CCThrottlerT<17> mThrottle;
  CCEWIApp& mApp;

  int mMenuAppCount = 0;
  MenuAppBase* mMenuApps[MAX_MENU_APPS];
  int mCurrentMenuAppIndex = 0;

public:
  CCAdafruitSSD1306 mDisplay;
  bool mIsSetup = false;

  void AddMenuApp(MenuAppBase* p) {
    mMenuApps[mMenuAppCount] = p;
    mMenuAppCount ++;
  }

  // display gets access to the whole app
  CCDisplay(CCEWIApp& app) : 
    mApp(app)
  {
  }

  void SelectApp(int n) {
    n %= mMenuAppCount;
    while (n < 0)
      n += mMenuAppCount;

    if (n == mCurrentMenuAppIndex)
      return;

    mMenuApps[mCurrentMenuAppIndex]->OnUnselected();
    mCurrentMenuAppIndex = n;
    //Serial.println(String("selecting app ") + n );
    mMenuApps[mCurrentMenuAppIndex]->OnSelected();
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
  }

  bool first = true;

  virtual void loop() {    
    gFramerate.onFrame();

    MenuAppBase* pMenuApp = mMenuApps[mCurrentMenuAppIndex];

    if (first) {
      first = false;
      pMenuApp->OnSelected();
    }

    pMenuApp->Update();
    
    if (!mThrottle.IsReady())
      return;

    mDisplay.clearDisplay();
    mDisplay.mSolidText = true;
    mDisplay.mTextLeftMargin = 0;
    mDisplay.mClipLeft = 0;
    mDisplay.mClipRight = RESOLUTION_X;
    mDisplay.mClipTop = 0;
    mDisplay.mClipBottom = RESOLUTION_Y;

    pMenuApp->Render();

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

CCDisplay gDisplay(gApp);

