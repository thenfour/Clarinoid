
#ifndef CCLEDS_H
#define CCLEDS_H

#include <Adafruit_NeoPixel.h>
#include "CCUtil.h"

class CCLeds : IUpdateObject
{

public:
  Adafruit_NeoPixel mStrip;
  CCThrottler mThrottle;

  CCLeds(uint8_t count, uint8_t pin, uint32_t updatePeriodMS) :
    mStrip(count, pin, NEO_BRG + NEO_KHZ800),
    mThrottle(updatePeriodMS)
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
};

#endif
