

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
        if (FloatEquals(hz, m_cutoffHz)) return;
        m_cutoffHz = hz;
        Recalc();
    }

    // 0-1
    virtual void SetResonance(real p_res) override
    {
        if (FloatEquals(p_res, m_cachedResonance)) return;
        m_cachedResonance = p_res;
        m_resonance = Real(24.5) * p_res * p_res * p_res * p_res + Real(0.5);
        m_resonance = ClampInclusive(m_resonance, Real(0.5), Real(25));
        Recalc();
    }

    virtual void SetParams(FilterType type, real cutoffHz, real reso, real saturation) override
    {
        if (FloatEquals(cutoffHz, m_cutoffHz) && FloatEquals(reso, m_cachedResonance)) return;
        m_cachedResonance = reso;
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

    virtual void ProcessInPlace(real *samplesL, real *samplesR, size_t sampleCount) override
    {
        for (size_t i = 0; i < sampleCount; ++i)
        {
            InlineProcessSample(samplesL[i], samplesR[i]);
        }
    }
    virtual void ProcessSample(real& xnL, real& xnR) override
    {
        return InlineProcessSample(xnL, xnR);
    }


    virtual void Reset() override
    {
        m_z_1L = Real0;
        m_z_2L = Real0;
        m_z_1R = Real0;
        m_z_2R = Real0;
    }

  private:
    real m_cutoffHz = 10000;

    real m_transition = -1; // i think this allows you to smoothly transition between LP, BP, and HP types.
    real m_resonance = Real(0.5);
    real m_cachedResonance = Real(-1);
    real m_alpha = 1;
    real m_alpha_0 = 1;
    real m_rho = 1;

    real m_z_1L = 0;
    real m_z_2L = 0;
    real m_z_1R = 0;
    real m_z_2R = 0;

    inline real InlineProcessSample(real xn)
    {
        real hpf = m_alpha_0 * (xn - m_rho * m_z_1L - m_z_2L);
        real bpf = m_alpha * hpf + m_z_1L;

        real lpf = m_alpha * bpf + m_z_2L;
        real r = Real1 / (Real2 * m_resonance);
        real bsf = xn - Real2 * r * bpf;

        m_z_1L = m_alpha * hpf + bpf;
        m_z_2L = m_alpha * bpf + lpf;

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

    inline void InlineProcessSample(real& xnL, real& xnR)
    {
        real hpfL = m_alpha_0 * (xnL - m_rho * m_z_1L - m_z_2L);
        real hpfR = m_alpha_0 * (xnR - m_rho * m_z_1R - m_z_2R);
        real bpfL = m_alpha * hpfL + m_z_1L;
        real bpfR = m_alpha * hpfR + m_z_1R;

        real lpfL = m_alpha * bpfL + m_z_2L;
        real lpfR = m_alpha * bpfR + m_z_2R;
        real r = Real1 / (Real2 * m_resonance);
        real bsfL = xnL - Real2 * r * bpfL;
        real bsfR = xnR - Real2 * r * bpfR;

        m_z_1L = m_alpha * hpfL + bpfL;
        m_z_2L = m_alpha * bpfL + lpfL;
        m_z_1R = m_alpha * hpfR + bpfR;
        m_z_2R = m_alpha * bpfR + lpfR;

        real transition_modded = m_transition;
        transition_modded = transition_modded > 1 ? 1 : transition_modded;
        transition_modded = transition_modded < -1 ? -1 : transition_modded;

        if (transition_modded < 0)
        {
            xnL = (1 + transition_modded) * bsfL - transition_modded * lpfL;
            xnR = (1 + transition_modded) * bsfR - transition_modded * lpfR;
        }
        else
        {
            xnL = transition_modded * hpfL + (1 - transition_modded) * bsfL;
            xnR = transition_modded * hpfR + (1 - transition_modded) * bsfR;
        }
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
