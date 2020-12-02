#pragma once

#include "MovingAverage.hpp"
#include "Stopwatch.hpp"

namespace clarinoid
{

template<size_t Nsamples = 120>
struct framerateCalculator
{
  float getFPS() const { return mVal.GetValue(); }
  float getMinFPS() const
  {
    if (mVal.GetValidSampleCount() == 0)
      return 0.0f;
    float ret = mVal.GetSample(0);
    for (size_t i = 1; i < mVal.GetValidSampleCount(); ++ i) {
      float th = mVal.GetSample(i);
      if (th < ret)
        ret = th;
    }
    return ret;
  }
  float getMaxFPS() const
  {
    float ret = 0.0f;
    for (size_t i = 0; i < mVal.GetValidSampleCount(); ++ i) {
      float th = mVal.GetSample(i);
      if (th > ret)
        ret = th;
    }
    return ret;
  }

  void onFrame() {
    auto microsSinceLastFrame = mTimer.ElapsedMicros();
    // get frame rate of that one frame.
    mVal.Update(float(1000 * 1000) / microsSinceLastFrame);
    mTimer.Restart();
  }

  Stopwatch mTimer;
  SimpleMovingAverage<Nsamples> mVal;
};


} // namespace clarinoid
