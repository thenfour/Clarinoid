
#ifndef CCDISPLAY_H
#define CCDISPLAY_H


#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "CCUtil.h"

// CC: not sure why every example uses software SPI. is there a hardware SPI that would work together with the audio shield?
// If using software SPI (the default case):
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13


class CCDisplay : IUpdateObject
{
  
public:
  Adafruit_SSD1306 mDisplay;
  CCThrottler mThrottle;

  CCDisplay() : 
    mDisplay(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS),
    mThrottle(50)
  {
  }

  virtual void setup() {
    // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
    mDisplay.begin(SSD1306_SWITCHCAPVCC);
    mDisplay.clearDisplay();
    mDisplay.display();
  }

  virtual void loop() {
    if (!mThrottle.IsReady())
      return;
  }
};

#endif
