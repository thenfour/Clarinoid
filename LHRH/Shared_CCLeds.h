
#ifndef CCLEDS_H
#define CCLEDS_H

#include <Adafruit_NeoPixel.h>
#include "Shared_CCUtil.h"

class CCLeds : IUpdateObject
{
  int mCount;
  Adafruit_NeoPixel mStrip;
  CCThrottler mThrottle;
  bool mReversed;

public:
  explicit CCLeds(uint8_t count, uint8_t pin, uint32_t updatePeriodMS, bool reversed) :
    mCount(count),
    mStrip(count, pin, NEO_GRB + NEO_KHZ800),
    mThrottle(updatePeriodMS),
    mReversed(reversed)
  {
  }

  virtual void setup()
  {
    mStrip.begin();
    mStrip.show();
  }

  virtual void loop()
  {
    if (!mThrottle.IsReady())
      return;
  }
  
  void show() { mStrip.show(); }

  void setPixelColor(uint8_t i, uint8_t r, uint8_t g, uint8_t b) {
    if (mReversed) {
      i = mCount - 1 - i;
    }
    mStrip.setPixelColor(i, r, g, b);
  }
};

#endif
