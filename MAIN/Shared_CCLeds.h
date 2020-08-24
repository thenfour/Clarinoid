
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

  // when brightness is 255, full brightness is unlocked.
  // otherwise things are scaled to brightness. 16 means the brightest it can display is 16.
  uint8_t mBrightness = 16;

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

  void SetBrightness(uint8_t b) {
    mBrightness = b;
  }
  
  void show() { mStrip.show(); }

  uint8_t adjustCol(uint8_t x)
  {
    if (!x)
      return 0; // you cannot scale "off" to anything else.
    uint8_t ret = (uint8_t)constrain((int)x * mBrightness / 255, 0, 255);
    ret = max(ret, 1);// don't scale down brightness to 0. 1 minimum.
    return ret;
  }

  void setPixelColor(uint8_t i, uint8_t r, uint8_t g, uint8_t b) {
    if (mReversed) {
      i = mCount - 1 - i;
    }
    r = adjustCol(r);
    g = adjustCol(g);
    b = adjustCol(b);
    mStrip.setPixelColor(i, r, g, b);
  }
};

#endif
