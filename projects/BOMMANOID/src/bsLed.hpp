

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
// 'max(float, const double&)

#include <random>
#include "clarinoid/basic/Basic.hpp"
#include "clarinoid/components/Leds.hpp"

namespace clarinoid
{

DMAMEM byte gLED1DisplayMemory[20 * 12]; // 12 bytes per LED

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
            uint32_t onColor = GRB(0, 4, 0);
            if (i == (ledsPerBank - 1))
                onColor = GRB(4, 0, 0);
            else if (i == (ledsPerBank - 2))
                onColor = GRB(4, 4, 0);
            else if (i == (ledsPerBank - 3))
                onColor = GRB(4, 4, 0);
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

struct Leds1 : Leds<20, 1>, ITask
{
    PeakMeter<9, 1500, 300> mPeakMeter;
    ILEDDataProvider *mpProvider;
    Leds1(ILEDDataProvider *pProvider) : Leds(gLED1DisplayMemory), mpProvider(pProvider)
    {
    }

    virtual void TaskRun() override
    {
        mPeakMeter.Update([&](int n, uint32_t c) { this->SetPixel(n, c); },
                          [&](int n, uint32_t c) { this->SetPixel(18 - n, c); });

        if (mpProvider->ILEDDataProvider_GetMusicalState()->mHoldingBaseNote)
        {
            this->SetPixel(10, 32, 32, 0);
        }

        this->SetPixel(9, 0, 0, 0);
        if (mpProvider->ILEDDataProvider_GetMusicalState()->mAppSettings->mMetronomeLED)
        {
            float beatSlice = mpProvider->ILEDDataProvider_GetMusicalState()->mAppSettings->mMetronomeLEDDecay;
            float beatFrac = mpProvider->ILEDDataProvider_GetMetronomeBeat()->GetBeatFrac();
            if (beatFrac < beatSlice)
            {
                this->SetPixel(9,
                               mpProvider->ILEDDataProvider_GetMusicalState()->mAppSettings->mMetronomeBrightness *
                                   (1.0f - (beatFrac / beatSlice)),
                               0,
                               0);
            }
        }

        Show();
    }
};

DMAMEM byte gLED2DisplayMemory[64 * 12]; // 12 bytes per LED

struct Leds2 : Leds<64, 29>, ITask
{
    PeakMeter<10, 500, 200> mPeakMeter;
    ILEDDataProvider *mpProvider;
    Leds2(ILEDDataProvider *pProvider)
        : Leds(gLED2DisplayMemory), mpProvider(pProvider), gen(rd()), distrib(0, (1 << 20) - 1) // 20 bits of entropy.
    {
    }

    CCThrottlerT<48> mTh;
    CCThrottlerT<500> mThR;
    CCThrottlerT<309> mThG;
    CCThrottlerT<48 * 1> mThB;
    CCThrottlerT<48 * 4> mThB2;
    uint32_t mRandR;
    uint32_t mRandG;
    bool mBlue1 = true;
    bool mBlue2 = true;
    // uint32_t mRandB;
    std::random_device rd; // Will be used to obtain a seed for the random number engine
    std::mt19937 gen;      // Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<uint32_t> distrib;

    virtual void TaskRun() override
    {
        if (!mTh.IsReady())
            return;

        // for (int n = 0; n < 10; ++n)
        //     //Use `distrib` to transform the random unsigned int generated by gen into an int in [1, 6]
        //     std::cout << distrib(gen) << ' ';
        // std::cout << '\n';

        // update bank 1 with random techy looking garbage
        if (mThR.IsReady())
            mRandR = distrib(gen);
        if (mThG.IsReady())
            mRandG = distrib(gen);
        if (mThB.IsReady())
            mBlue1 = !mBlue1;
        if (mThB2.IsReady())
            mBlue2 = !mBlue2;

        auto nr = mRandR;
        auto ng = mRandG;
        auto nb = mRandG;
        for (int i = 1; i <= 3; ++i)
        {
            if (nr & 1)
            {
                SetPixel(i, (nr & 1) ? 1 : 0, (ng & 1) ? 1 : 0, 0);
            }
            else
            {
                SetPixel(i, 0, 0, 0);
            }
            nr >>= 1;
            ng >>= 1;
            nb >>= 1;
            if (nr & 1)
            {
                SetPixel(63 - i, (nr & 1) ? 1 : 0, (ng & 1) ? 1 : 0, 0);
            }
            else
            {
                SetPixel(63 - i, 0, 0, 0);
            }
            nr >>= 1;
            ng >>= 1;
            nb >>= 1;
        }

        {
            uint8_t p = mBlue1 ? 2 : 0;
            SetPixel(0, 0, 0, p);
            p = !mBlue1 ? 4 : 0;
            // SetPixel(63, 0,0,p);
        }
        {
            uint8_t p = mBlue2 ? 2 : 0;
            // SetPixel(1, 0,0,p);
            p = !mBlue2 ? 4 : 0;
            SetPixel(63, 0, 0, p);
        }

        // bank 1 (10 + 10)   0-9, 54-63
        // bank 2 (10 + 10)  10-19, 44-53
        // bank 3 (7 + 5)    20-26, 39-43
        // bank 4 (7 + 5)    27-33, 34-38

        mPeakMeter.Update([&](int n, uint32_t c) { this->SetPixel(n + 10, c); },
                          [&](int n, uint32_t c) { this->SetPixel(53 - n, c); });

        // for (int i = 0; i <= 9; ++ i) {
        //     // bank 1: red
        //     SetPixel(i, 1+i, 0, 0);
        //     SetPixel(63-i, 1+i, 0, 0);

        //     // bank 2: green
        //     SetPixel(10+i, 0, 1+i, 0);
        //     SetPixel(53-i, 0, 1+i, 0);
        // }

        //                       c  C#  D   D#   E   F   F#  G   G#   A  A#  B
        int bank3_indices[] = {26, 39, 25, 40, 24, 23, 41, 22, 42, 21, 43, 20};
        // for (int i = 0; i <= 6; ++ i) {
        //     SetPixel(20+i, 0, 0, 1+i);
        // }
        // for (int i = 0; i <= 4; ++ i) {
        //     SetPixel(43-i, 0, 0, 1+i);
        // }

        //                       c  C#  D   D#   E   F   F#  G   G#   A  A#  B
        int bank4_indices[] = {33, 34, 32, 35, 31, 30, 36, 29, 37, 28, 38, 27};
        // for (int i = 0; i <= 6; ++ i) {
        //     SetPixel(27+i, 1+i, 1+i, 0);
        // }
        // for (int i = 0; i <= 4; ++ i) {
        //     SetPixel(38-i, 1+i, 1+i, 0);
        // }

        auto *ms = mpProvider->ILEDDataProvider_GetMusicalState();
        MidiNote playingNote = MidiNote(ms->mLiveVoice.mMidiNote);

        // clear.
        for (int i = 0; i < 12; ++i)
        {
            SetPixel(bank3_indices[i], 0, 0, 0);
            SetPixel(bank4_indices[i], 0, 0, 0);
        }

        SetPixel(bank4_indices[(int)ms->mScaleFollower->mCurrentScale.mRootNoteIndex], 0, 4, 0);
        SetPixel(bank4_indices[playingNote.GetNoteIndex()], 8, 0, 0);

        // chasing random colors (test)
        // for (int i = 0; i < mPixelCount; ++i)
        // {
        //     SetPixel(i,
        //              (nR == (i % periodR)) ? 1 : 0,
        //              (nG == (i % periodG)) ? 1 : 0,
        //              (nB == (i % periodB)) ? 1 : 0);
        // }
        // nR = (nR + 1) % periodR;
        // nG = (nG + 1) % periodG;
        // nB = (nB + 1) % periodB;
        Show();
    }
};

struct BreathLED : DigitalPinRGBLed<36, 37, 33>, ITask
{
    ILEDDataProvider *mpProvider;
    // TriangleWave mWaveR;
    // TriangleWave mWaveG;
    // TriangleWave mWaveB;
    BreathLED(ILEDDataProvider *pProvider) : mpProvider(pProvider)
    {
        // mWaveR.SetFrequency(1.0f);
        // mWaveG.SetFrequency(0.6f);
        // mWaveB.SetFrequency(0.4f);
    }
    virtual void TaskRun() override
    {
        float breath = mpProvider->ILEDDataProvider_GetInput()->mBreath.CurrentValue01();
        breath *= 1.3f;
        int pixel = ClampInclusive(int(breath * 255), 0, 255);
        SetPixel(pixel, pixel, pixel);
        // SetPixel(ColorF{
        //     mWaveR.GetValue01(micros()),
        //     mWaveG.GetValue01(micros()),
        //     mWaveB.GetValue01(micros())});
    }
};

} // namespace clarinoid
