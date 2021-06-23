

#pragma once

#include <cmath>
#include <algorithm>

namespace clarinoid
{
namespace filters
{
using real = float;

template <typename T>
constexpr real Real(const T x)
{
    return (real)x;
};
constexpr real RealPI = Real(3.14159265358979323846264338327950288);
constexpr real PITimes2 = Real(2.0 * RealPI);
constexpr real Real0 = 0;
constexpr real Real1 = 1;
constexpr real Real2 = 2;
inline real fasttanh(real p_input, real p_tanh_factor)
{
    return ::tanh(p_tanh_factor * p_input);
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

// flags
enum class FilterCapabilities
{
    None = 0,
    Resonance = 1,
    Saturation = 2,
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
    virtual void Reset() = 0; // honestly not even sure what reset really does
};

} // namespace filters
} // namespace clarinoid
