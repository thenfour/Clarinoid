#pragma once

#include <clarinoid/basic/Basic.hpp>
#include "MenuAppBase.hpp"
#include "MenuListControl.hpp"

namespace clarinoid
{

// feed it data and it will plot.
template</*int TdisplayWidth, */int Tspeed> // Tspeed is # of plots per column
struct Plotter
{
  //static constexpr size_t SamplesCapacity = TdisplayWidth * Tspeed; // theoretical maximum samples to take
  //float mSamples[SamplesCapacity];
  size_t mSamplesCapacity; // # of samples that we will actually use in mVals (width based on display area passed to Init())
  size_t mCursor = 0;
  size_t mValidSamples = 0; // how many elements in mSamples are valid.
  RectI mDisplayArea;
  CCDisplay* mDisplay = nullptr;
  float *mSamples = nullptr;

  void Init(CCDisplay* display, RectI displayArea)
  {
      mDisplay = display;
      CCASSERT(!!mDisplay);
      mDisplayArea = displayArea;
      mSamplesCapacity = displayArea.width * Tspeed;
      mSamples = new float[mSamplesCapacity];
  }

  void clear() {
    mValidSamples = 0;
    mCursor = 0;
  }
  
    void Plot(float val1) {
        CCASSERT(!!mDisplay);
        mSamples[mCursor] = val1;
        mValidSamples = max(mValidSamples, mCursor);
        mCursor = (mCursor + 1) % mSamplesCapacity;
    }
  
  void Render() {
    if (mValidSamples == 0) return;

    // draw back from cursor.
    for (int n = 0; n < (int)mValidSamples; ++ n) {
        int x = ((mSamplesCapacity - n) / Tspeed) - 1;
        int i = (int)mCursor - n - 1;
        if (i < 0)
        {
            i += mSamplesCapacity;
        }
        float f = mSamples[i];
        if (f < 0) f = 0;
        if (f > 1) f = 1;
        float y = map(f, 0.0f, 1.0f, float(mDisplayArea.height - 1), 0.0f);
        mDisplay->mDisplay.drawPixel(mDisplayArea.x + x, mDisplayArea.y + y, WHITE);
    }
  }
};



} // namespace clarinoid
