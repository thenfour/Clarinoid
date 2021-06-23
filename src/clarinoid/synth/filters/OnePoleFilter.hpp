
// "Designing Software Synthesizer Plug-Ins in C++" // https://willpirkle.com

#pragma once

#include "filterbase.hpp"

namespace clarinoid
{
namespace filters
{
struct OnePoleFilter : public IFilter
{

    void setFeedback(real fb)
    {
        m_feedback = fb;
    }

    real getFeedbackOutput()
    {
        return m_beta * (m_z_1 + m_feedback * m_delta);
    }

    // IFilter
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
        return FilterCapabilities::None;
    }
    virtual void SetCutoffFrequency(real hz) override
    {
        m_cutoffHz = hz;
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
            Recalc();
            break;
        case FilterType::HP:
        case FilterType::HP2:
        case FilterType::HP4:
            m_FilterType = FilterType::HP;
            Recalc();
            break;
        }

        m_cutoffHz = cutoffHz;
        Recalc();
    }

    virtual void Reset() override
    {
        m_z_1 = 0;
        m_feedback = 0;
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
        // for diode filter support
        xn = xn * m_gamma + m_feedback + m_epsilon * getFeedbackOutput();
        // calculate v(n)
        real vn = (m_a_0 * xn - m_z_1) * m_alpha;
        // form LP output
        real lpf = vn + m_z_1;
        // update memory
        m_z_1 = vn + lpf;
        if (m_FilterType == FilterType::LP)
        {
            return lpf;
        }
        return xn - lpf;
    }

    real m_alpha = 1; // Feed Forward coeff
    real m_beta = 0;
    real m_gamma = 1;    // Pre-Gain
    real m_delta = 0;    // FB_IN Coeff
    real m_epsilon = 0;  // FB_OUT scalar
    real m_a_0 = 1;      // input gain
    real m_feedback = 0; // our own feedback coeff from S

  private:
    FilterType m_FilterType = FilterType::LP;
    real m_cutoffHz = 10000;

    real m_z_1 = 0; // z-1 storage location

    void Recalc()
    {
        real wd = PITimes2 * m_cutoffHz;
        real T = OneOverSampleRate;
        real wa = (2 / T) * tan(wd * T / 2);
        real g = wa * T / 2;
        m_alpha = g / (Real(1) + g);
    }
};

} // namespace filters
} // namespace clarinoid
