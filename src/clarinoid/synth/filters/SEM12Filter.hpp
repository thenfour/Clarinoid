

#pragma once

#include "filterbase.hpp"

namespace clarinoid
{
namespace filters
{
struct SEM12Filter : IFilter
{
    virtual void SetType(FilterType type) override
    {
    } // only LP available

    virtual FilterCapabilities GetCapabilities()
    {
        return FilterCapabilities::Resonance;
    }

    virtual void SetCutoffFrequency(real hz) override
    {
        m_cutoffHz = hz;
        Recalc();
    }

    // 0-1
    virtual void SetResonance(real p_res) override
    {
        m_resonance = Real(24.5) * p_res * p_res * p_res * p_res + Real(0.5);
        m_resonance = ClampInclusive(m_resonance, Real(0.5), Real(25));
        Recalc();
    }

    virtual void SetParams(FilterType type, real cutoffHz, real reso, real saturation) override
    {
        m_cutoffHz = cutoffHz;
        m_resonance = Real(24.5) * reso * reso * reso * reso + Real(0.5);
        m_resonance = ClampInclusive(m_resonance, Real(0.5), Real(25));
        Recalc();
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

    virtual void Reset() override
    {
        m_z_1 = 0.0;
        m_z_2 = 0.0;
    }

  private:
    real m_cutoffHz = 10000;

    real m_transition = -1;
    real m_resonance = Real(0.5);
    real m_alpha = 1;
    real m_alpha_0 = 1;
    real m_rho = 1;

    real m_z_1 = 0;
    real m_z_2 = 0;

    inline real InlineProcessSample(real xn)
    {
        real hpf = m_alpha_0 * (xn - m_rho * m_z_1 - m_z_2);
        real bpf = m_alpha * hpf + m_z_1;

        real lpf = m_alpha * bpf + m_z_2;
        real r = Real1 / (Real2 * m_resonance);
        real bsf = xn - Real2 * r * bpf;

        m_z_1 = m_alpha * hpf + bpf;
        m_z_2 = m_alpha * bpf + lpf;

        real transition_modded = m_transition;
        transition_modded = transition_modded > 1 ? 1 : transition_modded;
        transition_modded = transition_modded < -1 ? -1 : transition_modded;

        if (transition_modded < 0)
        {
            xn = (1 + transition_modded) * bsf - transition_modded * lpf;
        }
        else
        {
            xn = transition_modded * hpf + (1 - transition_modded) * bsf;
        }

        return xn;
    }

    void Recalc()
    {
        real wd = PITimes2 * m_cutoffHz;
        real wa = (2 * SampleRate) * ::tan(wd * OneOverSampleRate * Real(0.5));
        real g = wa * OneOverSampleRate * Real(0.5);

        real r = Real1 / (Real2 * m_resonance);

        m_alpha_0 = Real1 / (Real1 + Real2 * r * g + g * g);
        m_alpha = g;
        m_rho = Real2 * r + g;
    }
};

} // namespace filters
} // namespace clarinoid
