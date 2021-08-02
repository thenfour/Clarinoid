
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
        xnminus1L = 0;
        ynminus1L = 0;
        xnminus1R = 0;
        ynminus1R = 0;
    }

    // mono processing
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
        real yn = xn - xnminus1L + R * ynminus1L;
        xnminus1L = xn;
        ynminus1L = yn;
        return yn;
    }

    // stereo processing
    virtual void ProcessInPlace(real *samplesL, real *samplesR, size_t sampleCount) override
    {
        for (size_t i = 0; i < sampleCount; ++i)
        {
            InlineProcessSample(samplesL[i], samplesR[i]);
        }
    }
    virtual void ProcessSample(real &xL, real &xR) override
    {
        InlineProcessSample(xL, xR);
    }

    inline void InlineProcessSample(real &xnL, real &xnR)
    {
        // left
        real ynL = xnL - xnminus1L + R * ynminus1L;
        xnminus1L = xnL;
        ynminus1L = ynL;
        // right
        real ynR = xnR - xnminus1R + R * ynminus1R;
        xnminus1R = xnR;
        ynminus1R = ynR;

        xnL = ynL;
        xnR = ynR;
    }

  private:
    // state L
    real xnminus1L = 0;
    real ynminus1L = 0;

    // state R
    real xnminus1R = 0;
    real ynminus1R = 0;

    real R;
};

} // namespace filters
} // namespace clarinoid
