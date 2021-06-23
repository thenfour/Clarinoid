
// "Designing Software Synthesizer Plug-Ins in C++" // https://willpirkle.com

#pragma once

#include "filterbase.hpp"

namespace clarinoid
{
namespace filters
{
struct DCFilter : public IFilter
{
    DCFilter()
    {
        SetMinus3DBFreq(10);
    }
    void SetMinus3DBFreq(real hz)
    {
        // 'How to calculate "R" for a given (-3dB) low frequency point?'
        // R = 1 - (pi*2 * frequency /samplerate)
        // "R" between 0.9 .. 1
        // "R" depends on sampling rate and the low frequency point. Do not set "R" to a fixed value
        // (e.g. 0.99) if you don't know the sample rate. Instead set R to:
        // (-3dB @ 40Hz): R = 1-(250/samplerate)
        // (-3dB @ 20Hz): R = 1-(126/samplerate)
        R = Real1 - (PITimes2 * hz / SampleRate);
    }
    // IFilter
    virtual void SetType(FilterType type) override
    {
    }
    virtual FilterCapabilities GetCapabilities() override
    {
        return FilterCapabilities::None;
    }
    virtual void SetCutoffFrequency(real hz) override
    {
    }
    virtual void SetParams(FilterType type, real cutoffHz, real reso, real saturation) override
    {
    }

    virtual void Reset() override
    {
        xnminus1 = 0;
        ynminus1 = 0;
    }

    virtual void ProcessInPlace(real *samples, size_t sampleCount) override
    {
        for (size_t i = 0; i < sampleCount; ++i)
        {
            samples[i] = InlineProcessSample(samples[i]);
        }
    }
    virtual real ProcessSample(real x) override
    {
        return InlineProcessSample(x);
    }

    inline real InlineProcessSample(real xn)
    {
        real yn = xn - xnminus1 + R * ynminus1;
        xnminus1 = xn;
        ynminus1 = yn;
        return yn;
    }

  private:
    real xnminus1 = 0;
    real ynminus1 = 0;
    real R;
};

} // namespace filters
} // namespace clarinoid
