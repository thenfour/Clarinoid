
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

// really i guess this is the whole GUI
class CCDisplay : IUpdateObject
{
  Adafruit_SSD1306 mDisplay;
  CCThrottlerT<20> mThrottle;
  CCEWIApp& mApp;

public:

  // display gets access to the whole app
  CCDisplay(CCEWIApp& app) : 
    mDisplay(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS),
    mApp(app)    
  {
  }

  virtual void setup() {
    // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
    mDisplay.begin(SSD1306_SWITCHCAPVCC);
    mDisplay.clearDisplay();
    mDisplay.display();
  }

  virtual void loop() {
    gFramerate.onFrame();
    if (!mThrottle.IsReady())
      return;

    mDisplay.clearDisplay();
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

    auto pageAudioStatus = [&]() {
      mDisplay.println(String("Memory: ") + AudioMemoryUsage());
      mDisplay.println(String("Memory Max: ") + AudioMemoryUsageMax());
      mDisplay.println(String("CPU: ") + AudioProcessorUsage());
      mDisplay.println(String("CPU Max: ") + AudioProcessorUsageMax());
    };

    int page = gEnc.GetValue() / 4;
    page = page % 7;
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
    }
    
    mDisplay.display();
  }
};

#endif
