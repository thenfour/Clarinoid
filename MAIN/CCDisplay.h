
#ifndef CCDISPLAY_H
#define CCDISPLAY_H

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Shared_CCUtil.h"

// CC: not sure why every example uses software SPI. is there a hardware SPI that would work together with the audio shield?
// If using software SPI (the default case):
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13

framerateCalculator gFramerate;

//////////////////////////////////////////////////////////////////////
class MenuAppBase
{
public:
  virtual void OnSelected() {};
  virtual void OnUnselected() {}
  virtual void Update() = 0; // called each frame
  virtual void Render() = 0; // called occasionally
};

//////////////////////////////////////////////////////////////////////
// really i guess this is the whole GUI
class CCDisplay : IUpdateObject
{
  CCThrottlerT<20> mThrottle;
  CCEWIApp& mApp;

  static constexpr int MAX_MENU_APPS = 20;
  int mMenuAppCount = 0;
  MenuAppBase* mMenuApps[MAX_MENU_APPS];
  int mCurrentMenuAppIndex = 0;

public:
  Adafruit_SSD1306 mDisplay;

  void AddMenuApp(MenuAppBase* p) {
    mMenuApps[mMenuAppCount] = p;
    mMenuAppCount ++;
  }

  // display gets access to the whole app
  CCDisplay(CCEWIApp& app) : 
    mApp(app),
    mDisplay(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS)
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
    mMenuApps[mCurrentMenuAppIndex]->OnSelected();
  }

  void ScrollApps(int delta) {
    SelectApp(mCurrentMenuAppIndex + delta);
  }

//  void NextApp() {
//    SelectApp(mCurrentMenuAppIndex + 1);
//  }
//  void PreviousApp() {
//    SelectApp(mCurrentMenuAppIndex - 1);
//  }

  virtual void setup() {
    // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
    mDisplay.begin(SSD1306_SWITCHCAPVCC);
    mDisplay.clearDisplay();
    mDisplay.display();
  }

  virtual void loop() {
    gFramerate.onFrame();

    MenuAppBase* pMenuApp = mMenuApps[mCurrentMenuAppIndex];
    pMenuApp->Update();
    
    if (!mThrottle.IsReady())
      return;

    mDisplay.clearDisplay();

    pMenuApp->Render();
    
    mDisplay.display();
  }
};

#endif
