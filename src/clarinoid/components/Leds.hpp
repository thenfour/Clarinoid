
#pragma once

//#include <WS2812Serial.h>
#include "../basic/Basic.hpp"

namespace clarinoid
{

// wrapper around the ws2812serial, not really necessary but it helps a bit.
template<uint8_t Tcount, uint8_t Tpin>
struct Leds
{
    static constexpr uint8_t mPixelCount = Tcount;
    byte mDrawingMemory[Tcount * 3];         //  3 bytes per LED
    WS2812Serial mLeds;

    //DMAMEM byte displayMemory[numled*12]; // 12 bytes per LED
    Leds(void* displayMemory /* 12 bytes DMA per LED */) :
        mLeds(Tcount, displayMemory, mDrawingMemory, Tpin, WS2812_GRB)
    {
        //Serial.println(String("Leds ctor this = ") + ((uintptr_t)this));
        delay(10);
        mLeds.begin();
    }

    static constexpr uint32_t GRB(uint8_t r, uint8_t g, uint8_t b)
    {
        return (r << 16) | (g << 8) | b;
    } 

    void SetPixel(uint8_t pixel, uint8_t r, uint8_t g, uint8_t b) {
        mLeds.setPixel(pixel, GRB(r,g,b));
    }

    void SetPixel(uint8_t pixel, ColorF c)
    {
        SetPixel(pixel, Float01ToInt<uint8_t, 0, 255>(c.r), Float01ToInt<uint8_t, 0, 255>(c.g), Float01ToInt<uint8_t, 0, 255>(c.b));
    }

    void Show()
    {
        mLeds.show();
    }
};

template<uint8_t Tpin>
struct DigitalPinLed
{
    DigitalPinLed()
    {
        pinMode(Tpin, OUTPUT);
    }

    // 0 and 255 are limits
    void SetPixel(uint8_t c)
    {
        //digitalWrite(Tpin, c > 128 ? HIGH: LOW);
        analogWrite(Tpin, c);
    }

    void SetPixel(float c)
    {
        SetPixel(Float01ToInt<uint8_t, 0, 255>(c));
    }
};

template<uint8_t TRpin, uint8_t TGpin, uint8_t TBpin>
struct DigitalPinRGBLed
{
    DigitalPinLed<TRpin> mR;
    DigitalPinLed<TGpin> mG;
    DigitalPinLed<TBpin> mB;

    // 0 and 255 are limits
    // void SetPixel(uint8_t r, uint8_t g, uint8_t b)
    // {
    //     mR.SetPixel(r);
    //     mG.SetPixel(g);
    //     mB.SetPixel(b);
    // }

    void SetPixel(const ColorF& c)
    {
        mR.SetPixel(c.r);
        mG.SetPixel(c.g);
        mB.SetPixel(c.b);
    }
};
} // namespace clarinoid

