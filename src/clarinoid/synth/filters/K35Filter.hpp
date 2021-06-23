
// "Designing Software Synthesizer Plug-Ins in C++" // https://willpirkle.com

#pragma once

#include "OnePoleFilter.hpp"

namespace clarinoid
{
namespace filters
{
struct K35Filter : IFilter
{
    K35Filter()
    {
        m_LPF1.SetType(FilterType::LP);
        m_LPF2.SetType(FilterType::LP);
        m_HPF1.SetType(FilterType::HP);
        m_HPF2.SetType(FilterType::HP);
    }

    virtual void SetType(FilterType type) override
    {
        switch (type)
        {
        default:
        case FilterType::LP:
        case FilterType::LP2:
        case FilterType::LP4:
            m_FilterType = FilterType::LP;
            Recalc();
            break;
        case FilterType::HP:
        case FilterType::HP2:
        case FilterType::HP4:
            m_FilterType = FilterType::HP;
            Recalc();
            break;
        }
        // not supported.
    }

    virtual FilterCapabilities GetCapabilities() override
    {
        return (FilterCapabilities)((int)FilterCapabilities::Resonance | (int)FilterCapabilities::Saturation);
    }

    virtual void SetCutoffFrequency(real hz) override
    {
        m_cutoffHz = hz;
        Recalc();
    }

    virtual void SetSaturation(real amt) override
    {
        m_overdrive = amt;
        Recalc();
    }

    // 0-1
    virtual void SetResonance(real res) override
    {
        // note: m_k must never be zero else division by zero
        // note2 original was 1.99 but dont want self oscillation
        m_k = res * Real(1.95) + Real(0.01);
        m_k = ClampInclusive(m_k, Real(0.01), Real(1.96));
        Recalc();
    }

    virtual void SetParams(FilterType type, real cutoffHz, real reso, real saturation) override
    {
        switch (type)
        {
        default:
        case FilterType::LP:
        case FilterType::LP2:
        case FilterType::LP4:
            m_FilterType = FilterType::LP;
            break;
        case FilterType::HP:
        case FilterType::HP2:
        case FilterType::HP4:
            m_FilterType = FilterType::HP;
            break;
        }
        m_cutoffHz = cutoffHz;
        m_overdrive = saturation;
        m_k = reso * Real(1.95) + Real(0.01);
        m_k = ClampInclusive(m_k, Real(0.01), Real(1.96));
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
        m_LPF1.Reset();
        m_LPF2.Reset();
        m_HPF1.Reset();
        m_HPF2.Reset();
    }

  private:
    FilterType m_FilterType = FilterType::LP;
    real m_cutoffHz = 10000;
    real m_overdrive = 0;

    real m_k = Real(0.01);
    real m_alpha = 0;

    OnePoleFilter m_LPF1;
    OnePoleFilter m_LPF2;
    OnePoleFilter m_HPF1;
    OnePoleFilter m_HPF2;

    inline real InlineProcessSample(real xn)
    {
        real y;
        if (m_FilterType == FilterType::LP)
        {
            real y1 = m_LPF1.InlineProcessSample(xn);
            real s35 = m_LPF2.getFeedbackOutput() + m_HPF1.getFeedbackOutput();
            real u = m_alpha * (y1 + s35);

            y = m_k * m_LPF2.InlineProcessSample(u);
            m_HPF1.InlineProcessSample(y);
        }
        else
        {
            real y1 = m_HPF1.InlineProcessSample(xn);
            real s35 = m_HPF2.getFeedbackOutput() + m_LPF1.getFeedbackOutput();
            real u = m_alpha * (y1 + s35);

            y = m_k * u;
            m_LPF1.InlineProcessSample(m_HPF2.InlineProcessSample(y));
        }
        y /= m_k;

        // make this one a bit easier (3.f), its very aggresive
        applyOverdrive(y, m_overdrive, Real(3));

        return y;
    }
    inline void Recalc()
    {
        // BZT
        real wd = PITimes2 * m_cutoffHz;
        real wa = (2 * SampleRate) * ::tan(wd * OneOverSampleRate * Real(0.5));
        real g = wa * OneOverSampleRate * Real(0.5);
        real G = g / (Real1 + g);

        m_LPF1.m_alpha = G;
        m_LPF2.m_alpha = G;
        m_HPF1.m_alpha = G;
        m_HPF2.m_alpha = G;

        m_alpha = Real1 / (Real1 - m_k * G + m_k * G * G);

        if (m_FilterType == FilterType::LP)
        {
            m_LPF2.m_beta = (m_k - m_k * G) / (Real1 + g);
            m_HPF1.m_beta = -Real1 / (Real1 + g);
        }
        else
        {
            m_HPF2.m_beta = -Real1 * G / (Real1 + g);
            m_LPF1.m_beta = Real1 / (Real1 + g);
        }
    }
};

} // namespace filters
} // namespace clarinoid
