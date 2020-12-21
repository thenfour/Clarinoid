

#pragma once

#include "clarinoid/basic/Basic.hpp"
#include "clarinoid/components/Leds.hpp"

namespace clarinoid
{


DMAMEM byte gLED1DisplayMemory[20*12]; // 12 bytes per LED

struct Leds1 :
    Leds<20, 1>,
    ITask
{
    Leds1() :
        Leds(gLED1DisplayMemory)
    {}

    int n = 0;
    virtual void TaskRun() override
    {
        for (int i = 0; i < mPixelCount; ++ i)
        {
            uint8_t o = (n == i) ? 32 : 0;
            SetPixel(i, 0, o, 0);
        }
        n = (n+1)%mPixelCount;
        Show();
    }
};



DMAMEM byte gLED2DisplayMemory[64*12]; // 12 bytes per LED

struct Leds2 :
    Leds<64, 29>,
    ITask
{
    Leds2() :
        Leds(gLED2DisplayMemory)
    {}

    int nR = 0;
    int nG = 0;
    int nB = 0;
    int periodR = 25;
    int periodG = 26;
    int periodB = 27;
    CCThrottlerT<48> mTh;

    virtual void TaskRun() override 
    {
        if (!mTh.IsReady()) return;
        for (int i = 0; i < mPixelCount; ++ i)
        {
            SetPixel(i,
                (nR == (i%periodR)) ? 1 : 0,
                (nG == (i%periodG)) ? 1 : 0,
                (nB == (i%periodB)) ? 1 : 0
            );
        }
        nR = (nR+1)%periodR;
        nG = (nG+1)%periodG;
        nB = (nB+1)%periodB;
        Show();
    }
};

struct BreathLED :
    DigitalPinRGBLed<36,37,33>,
    ITask
{
    TriangleWave mWaveR;
    TriangleWave mWaveG;
    TriangleWave mWaveB;
    BreathLED()
    {
        mWaveR.SetFrequency(1.0f);
        mWaveG.SetFrequency(0.6f);
        mWaveB.SetFrequency(0.4f);
    }
    virtual void TaskRun() override 
    {
        //Serial.println(String("breath led ") + mWaveR.GetValue01(micros()));
        SetPixel(ColorF {
            mWaveR.GetValue01(micros()),
            mWaveG.GetValue01(micros()),
            mWaveB.GetValue01(micros())
            });
    }
};

} // namespace clarinoid
