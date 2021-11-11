

#pragma once

namespace std
{
// solves some error in the gcc library
// ... .platformio\packages\toolchain-gccarmnoneeabi\arm-none-eabi\include\c++\5.4.1\bits\random.tcc:1468:42: note:
// mismatched types 'std::initializer_list<_Tp>' and 'float' https://github.com/tianzhi0549/CTPN/issues/60
double max(const float &a, const double &b)
{
    return (a < b) ? b : a;
}
} // namespace std

#include "clarinoid/basic/Basic.hpp"
#include "clarinoid/components/Leds.hpp"
#include <random>

namespace clarinoid
{
struct ILEDDataProvider
{
    virtual Metronome *ILEDDataProvider_GetMetronomeBeat() = 0;
    virtual InputDelegator *ILEDDataProvider_GetInput() = 0;
    virtual CCEWIMusicalState *ILEDDataProvider_GetMusicalState() = 0;
};

template <int ledsPerBank, uint32_t holdTimeMS, uint32_t falloffTimeMS>
struct PeakMeter
{
    float mHeldPeak = 0;
    Stopwatch mHeldPeakTime; // peak is simply held for a duration.

    template <typename Tbank1, typename Tbank2>
    void Update(const Tbank1 &setBank1, const Tbank2 &setBank2)
    {
        float peak = CCSynth::GetPeakLevel();
        float heldPeakDisplay;

        // determine a new held peak
        // if the held peak has been holding longer than 500ms, fade linear to 0.
        // constexpr uint32_t holdTimeMS = 1500;
        // constexpr uint32_t falloffTimeMS = 350;
        uint32_t holdDurationMS = (uint32_t)mHeldPeakTime.ElapsedTime().ElapsedMillisI();
        if ((peak > mHeldPeak) || holdDurationMS > (holdTimeMS + falloffTimeMS))
        {
            // new peak, or after falloff reset.
            mHeldPeak = peak;
            heldPeakDisplay = peak;
            mHeldPeakTime.Restart();
        }
        else if (holdDurationMS <= holdTimeMS)
        {
            heldPeakDisplay = mHeldPeak;
        }
        else
        {
            // falloff: remap millis from 500-1000 from heldpeak to 0.
            heldPeakDisplay = map<float, float, float, float, float>(
                holdDurationMS, holdTimeMS, holdTimeMS + falloffTimeMS, mHeldPeak, peak);
        }

        int peakLEDIndex = (int)((peak * (ledsPerBank - 1)) + 0.1f); // +.1 helps nudge it up a bit.
        int heldPeakLEDIndex = (int)((heldPeakDisplay * (ledsPerBank - 1)) + 0.1f);

        peakLEDIndex = ClampInclusive(peakLEDIndex, 0, (ledsPerBank - 1));
        heldPeakLEDIndex = ClampInclusive(heldPeakLEDIndex, 0, ledsPerBank - 1);

        // render. 1 red, 2 yellow, rest green.
        for (int i = 0; i < ledsPerBank; ++i)
        {
            uint32_t color1 = 0; // default off=black.
            uint32_t color2 = 0; // default off=black.
            uint32_t onColor = GRB(0, 128, 0);
            if (i == (ledsPerBank - 1))
                onColor = GRB(128, 0, 0);
            else if (i == (ledsPerBank - 2))
                onColor = GRB(128, 128, 0);
            else if (i == (ledsPerBank - 3))
                onColor = GRB(128, 128, 0);
            // else if (i == (ledsPerBank - 4))
            //     onColor = GRB(4, 4, 0);

            if (i <= peakLEDIndex)
            {
                color1 = onColor;
            }
            if (i == heldPeakLEDIndex)
            {
                color2 = onColor;
            }

            setBank1(i, color1);
            setBank2(i, color2);
        }
    }
};

DMAMEM byte gLED1DisplayMemory[28 * 12]; // 12 bytes per LED

struct Clarinoid2LedsTask : Leds<28 /*ledcount*/, 14 /*pin*/>, ITask
{
    //     <---------10-------->                     <------------10------------->
    //     2 3 4 5 6 7 8 9 10 11                     16 17 18 19 20 21 22 23 24 25
    // 0 1                       12 13 ------- 14 15                               26 27
    //                                                                       |  |
    //                                                                       |  +- note held
    //                                                                       +---- metronome
    //                                               16 17 18 19 20 21 22 23
    //                                                  +--+--+--+--+--+--+ peak
    //                                               + octave indicator.
    static constexpr int8_t iOctaveIndicator = 16;
    static constexpr int8_t iFirstPeak = 23;
    static constexpr int8_t iPeakCount = 7;
    static constexpr int8_t iMetronomeIndicator = 24;
    static constexpr int8_t iNoteHoldIndicator = 25;

    size_t serialCount = 0;
    PeakMeter<iPeakCount /* # leds */, 2500 /*holdtime*/, 200 /*fallofftime*/> mPeakMeter;
    ILEDDataProvider *mpProvider = nullptr;

    bool IsInnerLED(size_t i)
    {
        switch (i)
        {
        case 0:
        case 1:
        case 12:
        case 13:
        case 14:
        case 15:
            // case 26: // these are completely covered up
            // case 27: // these are completely covered up
            return true;
        }
        return false;
    }

    Clarinoid2LedsTask(ILEDDataProvider *pProvider) : Leds(gLED1DisplayMemory), mpProvider(pProvider)
    {
    }

    virtual void TaskRun() override
    {
        serialCount++;
        for (size_t i = 0; i < 28; ++i)
        {
            if (IsInnerLED(i))
            {
                this->SetPixel(i, 0, 0, 96); // inner leds = blue
            }
            else if (i >= 6 && i < 12)
            {
                this->SetPixel(i, ((serialCount & (1 << i)) > 0) ? 4 : 0, 0, 0); // front LEDs RED bliner
            }
            else if (i > 1 && i < 6)
            {
                this->SetPixel(i, 0, 0, ((serialCount & (1 << i)) > 0) ? 14 : 0); // front LEDs BLUE blinker
            }
        }

        mPeakMeter.Update([&](int n, uint32_t c) { this->SetPixel(iFirstPeak - n, c); },
                          [&](int n, uint32_t c) { this->SetPixel(iFirstPeak - n, c); });

        if (mpProvider->ILEDDataProvider_GetMusicalState()->mHoldingBaseNote)
        {
            this->SetPixel(iNoteHoldIndicator, 0, 32, 0);
        }

        // clarinoid & bassoonoid both have octave 0-5
        switch (mpProvider->ILEDDataProvider_GetMusicalState()->mLiveOctave)
        {
        case 0:
            this->SetPixel(iOctaveIndicator, 0, 0, 0); // lowest = purple
            break;
        case 1:
            this->SetPixel(iOctaveIndicator, 128, 0, 128); // green
            break;
        case 2:
            this->SetPixel(iOctaveIndicator, 0, 0, 0); // default octave = off
            break;
        case 3:
            this->SetPixel(iOctaveIndicator, 0, 0, 128); // blue
            break;
        case 4:
            this->SetPixel(iOctaveIndicator, 128, 0, 0); // red
            break;
        default:
        case 5:
            this->SetPixel(iOctaveIndicator, 128, 128, 128); // white
            break;
        }

        // static const int metronomeLEDIndex = 24;
        this->SetPixel(iMetronomeIndicator, 0, 0, 0);
        if (mpProvider->ILEDDataProvider_GetMusicalState()->mAppSettings->mMetronomeLED)
        {
            float beatSlice = mpProvider->ILEDDataProvider_GetMusicalState()->mAppSettings->mMetronomeLEDDecay;
            float beatFrac = mpProvider->ILEDDataProvider_GetMetronomeBeat()->GetBeatFrac();
            if (beatFrac < beatSlice)
            {
                this->SetPixel(iMetronomeIndicator,
                               mpProvider->ILEDDataProvider_GetMusicalState()->mAppSettings->mMetronomeBrightness *
                                   (1.0f - (beatFrac / beatSlice)),
                               0,
                               0);
            }
        }

        Show();
    }
};

} // namespace clarinoid
