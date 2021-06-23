#pragma once

#include <clarinoid/basic/Basic.hpp>
#include "MenuAppBase.hpp"
#include "MenuListControl.hpp"

namespace clarinoid
{

// feed it data and it will plot.
template <int TmaxWidth /*, int Tspeed*/> // Tspeed is # of plots per column
struct Plotter
{
    static constexpr size_t SamplesCapacity = TmaxWidth /* * Tspeed*/; // theoretical maximum samples to take
    float mSamples[SamplesCapacity];
    // size_t mSamplesCapacity; // # of samples that we will actually use in mVals (width based on display area passed
    // to Init())
    size_t mCursor = 0;
    size_t mValidSamples = 0; // how many elements in mSamples are valid.
    // float *mSamples = nullptr;
    // float mSamples[];

    void Init()
    {
    }

    // void Init(/*CCDisplay* display, RectI displayArea*/)
    // {
    //     //mDisplay = display;
    //     //CCASSERT(!!mDisplay);
    //     //mDisplayArea = displayArea;
    //     //mSamplesCapacity = TmaxWidth * Tspeed;
    //     //mSamples = new float[mSamplesCapacity];
    // }

    void clear()
    {
        mValidSamples = 0;
        mCursor = 0;
    }

    void Plot(float val1)
    {
        // CCASSERT(!!mDisplay);
        mSamples[mCursor] = val1;
        mValidSamples = max(mValidSamples, mCursor);
        mCursor = (mCursor + 1) % SamplesCapacity;
    }

    void Render(CCDisplay &mDisplay, RectI mDisplayArea)
    {
        if (mValidSamples == 0)
            return;

        int iSample = mCursor - 1;
        int xLeftBound = (mDisplayArea.width > (int)mValidSamples)
                             ? (mDisplayArea.width - mValidSamples)
                             : 0; // show blank for left pixels if there are not enough samples.
        for (int x = (mDisplayArea.width - 1); x >= xLeftBound; --x)
        // while(false)
        {
            if (iSample < 0)
                iSample += SamplesCapacity;

            float f = Clamp(mSamples[iSample], 0.0f, 1.0f);
            float y = map(f, 0.0f, 1.0f, float(mDisplayArea.height - 1), 0.0f);
            mDisplay.mDisplay.drawPixel(mDisplayArea.x + x, mDisplayArea.y + y, WHITE);

            --iSample;
        }
    }
};

} // namespace clarinoid
