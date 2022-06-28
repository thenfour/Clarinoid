
#pragma once

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "misc.hpp"

struct Ledstrip
{
    static constexpr int gWS2812Pin = 10;
    static constexpr int gWS2812Count = 12;

    Adafruit_NeoPixel strip;
    bool mDirty = false;

    clarinoid::Stopwatch mHeartbeatTimer;
    static constexpr int gHeartbeatIntervalsMS[] = {400, 100};

    Ledstrip() : strip(gWS2812Count, gWS2812Pin, NEO_GRB + NEO_KHZ800)
    {
        strip.begin();
        for (int i = 0; i < gWS2812Count; ++i)
        {
            strip.setPixelColor(i, 0);
        }
        strip.show();
    }

    void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
    {
        // todo: check if pixel already this color. though caller is already doing this for my application so no biggie
        strip.setPixelColor(n, r, g, b);
        mDirty = true;
    }

    void setPixelColor(uint16_t n, uint32_t c)
    {
        auto x = strip.getPixelColor(n);
        if (x == c)
            return;
    }

    void Render()
    {
        if (mDirty)
        {
            mDirty = false;
            strip.show();
        }
    }
};
