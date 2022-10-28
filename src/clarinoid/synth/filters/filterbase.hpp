

#pragma once

#include <cmath>
#include <algorithm>

namespace clarinoid
{
namespace filters
{
using real = float;
//using real = fp::fp_t<int32_t, 12>;

template <typename T>
constexpr real Real(const T x)
{
    return static_cast<real>(x);
    //return {x};
};
constexpr real RealPI = {3.14159265358979323846264338327950288};
constexpr real PITimes2 = {real{2.0} * RealPI};
constexpr real Real0 = {0};
constexpr real Real1 = {1};
constexpr real Real2 = {2};
inline real fasttanh(real p_input, real p_tanh_factor)
{
    return ::clarinoid::fast::tanh(p_tanh_factor * p_input);
}

constexpr real SampleRate = 44100;
constexpr real OneOverSampleRate = Real1 / Real(44100);

// overdrive amt can be between 0 and up, but practical range 0-1.5 or so.
inline void applyOverdrive(real &pio_input, real m_overdrive, real p_tanh_factor)
{
    real overdrive_modded = m_overdrive;
    overdrive_modded = overdrive_modded < 0 ? 0 : overdrive_modded;
    if (overdrive_modded > Real(0.01) && overdrive_modded < 1)
    {
        // interpolate here so we have possibility of pure linear Processing
        pio_input = pio_input * (Real1 - overdrive_modded) + overdrive_modded * fasttanh(pio_input, p_tanh_factor);
    }
    else if (overdrive_modded >= 1)
    {
        pio_input = fasttanh(overdrive_modded * pio_input, p_tanh_factor);
    }
}

inline void applyOverdrive(real &L, real &R, real m_overdrive, real p_tanh_factor)
{
    real overdrive_modded = m_overdrive;
    overdrive_modded = overdrive_modded < 0 ? 0 : overdrive_modded;
    if (overdrive_modded > Real(0.01) && overdrive_modded < 1)
    {
        // interpolate here so we have possibility of pure linear Processing
        L = L * (Real1 - overdrive_modded) + overdrive_modded * fasttanh(L, p_tanh_factor);
        R = R * (Real1 - overdrive_modded) + overdrive_modded * fasttanh(R, p_tanh_factor);
    }
    else if (overdrive_modded >= 1)
    {
        L = fasttanh(overdrive_modded * L, p_tanh_factor);
        R = fasttanh(overdrive_modded * R, p_tanh_factor);
    }
}

enum class FilterType
{
    LP, // interpreted as "any lp"
    LP2,
    LP4,
    BP, // interpreted as "any bp"
    BP2,
    BP4,
    HP, // interpreted as "any hp"
    HP2,
    HP4,
};

// BINARY FLAGS
enum class FilterCapabilities
{
    None = 0,
    Cutoff = 1,
    Resonance = 2,
    Saturation = 4,
};

struct IFilter
{
    virtual void SetType(FilterType type) = 0;
    virtual FilterCapabilities GetCapabilities() = 0;
    virtual void SetCutoffFrequency(real hz) = 0;
    virtual void SetSaturation(real amt)
    {
    } // 0-1 range
    virtual void SetResonance(real amt)
    {
    }
    virtual void SetParams(FilterType type, real cutoffHz, real reso, real saturation) = 0;

    virtual void ProcessInPlace(real *samples, size_t sampleCount) = 0;
    virtual real ProcessSample(real x) = 0;
    virtual void ProcessInPlace(real *samplesL, real *samplesR, size_t sampleCount) = 0;
    virtual void ProcessSample(real &l, real &r) = 0;
    virtual void Reset() = 0; // honestly not even sure what reset really does
};

struct NullFilter : IFilter
{
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
    virtual void ProcessInPlace(real *samples, size_t sampleCount) override
    {
    }
    virtual real ProcessSample(real x) override
    {
        return x;
    }
    virtual void ProcessInPlace(real *samplesL, real *samplesR, size_t sampleCount) override
    {
    }
    virtual void ProcessSample(real &l, real &r) override
    {
    }
    virtual void Reset() override
    {
    }
};

} // namespace filters
} // namespace clarinoid
