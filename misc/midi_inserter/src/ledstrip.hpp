
#pragma once

#include "misc.hpp"
#include "anim.hpp"

struct ILedArray
{
    virtual void ILedArray_SetPixelColor(int n, const Color &c) = 0;
};

// hardware driver which applies an animation to a WS2812b led
struct WS2812Led : IAnimationTarget
{
    IAnimation *mpCurrentAnimation = &gOffAnimation;
    ILedArray *mpParent = nullptr;
    int mIndex = 0;

    virtual void IAnimationTarget_SetAnimation(IAnimation *anim) override
    {
        mpCurrentAnimation = anim;
    }

    static int _01ToAnalogLevel(float f)
    {
        return 255.0f * std::pow(f, 4.5f); // gamma-correct pin LEDs
    }

    virtual void Render()
    {
        if (!mpParent)
            return;
        auto c = mpCurrentAnimation->IAnimation_GetColor();
        mpParent->ILedArray_SetPixelColor(mIndex, c);
    }
};

// hardware driver
template <int Tpin, int Tcount>
struct Ledstrip : ILedArray
{
    byte mDrawingMemory[Tcount * 3];
    WS2812Serial mStrip;

    WS2812Led mLeds[Tcount];

    Ledstrip(void *displayMemory) : mStrip(Tcount, displayMemory, mDrawingMemory, Tpin, WS2812_GRB)
    {
        mStrip.begin();
        for (int i = 0; i < Tcount; ++i)
        {
            mStrip.setPixelColor(i, 0);
            mLeds[i].mpParent = this;
            mLeds[i].mIndex = i;
        }
        mStrip.show();
    }

    static int _01ToValue(float x)
    {
        x = std::pow(x, 3.5f);
        return x * 255;
    }

    virtual void ILedArray_SetPixelColor(int n, const Color &c) override
    {
        if (n < 0 || n >= Tcount)
            return;
        mStrip.setPixelColor(Tcount - n - 1, _01ToValue(c.R01()), _01ToValue(c.G01()), _01ToValue(c.B01()));
    }

    void Render()
    {
        for (auto &led : mLeds)
        {
            led.Render();
        }
        mStrip.show();
    }
};
