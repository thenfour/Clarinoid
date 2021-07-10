
// "Designing Software Synthesizer Plug-Ins in C++" // https://willpirkle.com

#pragma once

#include "filterbase.hpp"

namespace clarinoid
{
namespace filters
{
struct OnePoleFilter : public IFilter
{

    void setFeedbackL(real fb)
    {
        m_feedbackL = fb;
    }

    void setFeedbackR(real fb)
    {
        m_feedbackR = fb;
    }

    real getFeedbackOutputL()
    {
        return m_beta * (m_z_1L + m_feedbackL * m_delta);
    }

    real getFeedbackOutputR()
    {
        return m_beta * (m_z_1L + m_feedbackR * m_delta);
    }

    // IFilter
    virtual void SetType(FilterType type) override
    {
        if (m_FilterType == type) return;
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
        if (FloatEquals(hz, m_cutoffHz)) return;
        m_cutoffHz = hz;
        Recalc();
    }

    virtual void SetParams(FilterType type, real cutoffHz, real reso, real saturation) override
    {
        if ((m_FilterType == type) && FloatEquals(m_cutoffHz, cutoffHz)) {
            return;
        }
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
        m_z_1L = 0;
        m_feedbackL = 0;
        m_z_1R = 0;
        m_feedbackR = 0;
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
        xn = xn * m_gamma + m_feedbackL + m_epsilon * getFeedbackOutputL();
        // calculate v(n)
        real vn = (m_a_0 * xn - m_z_1L) * m_alpha;
        // form LP output
        real lpf = vn + m_z_1L;
        // update memory
        m_z_1L = vn + lpf;
        if (m_FilterType == FilterType::LP)
        {
            return lpf;
        }
        return xn - lpf;
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

    inline void InlineProcessSample(real& xnL, real& xnR)
    {
        // for diode filter support
        // LEFT
        xnL = xnL * m_gamma + m_feedbackL + m_epsilon * getFeedbackOutputL();
        xnR = xnR * m_gamma + m_feedbackR + m_epsilon * getFeedbackOutputR();
        // calculate v(n)
        real vnL = (m_a_0 * xnL - m_z_1L) * m_alpha;
        real vnR = (m_a_0 * xnR - m_z_1R) * m_alpha;
        // form LP output
        real lpfL = vnL + m_z_1L;
        real lpfR = vnR + m_z_1R;
        // update memory
        m_z_1L = vnL + lpfL;
        m_z_1R = vnR + lpfR;
        if (m_FilterType == FilterType::LP)
        {
            xnL = lpfL;
            xnR = lpfR;
            return;
        }
        xnL = xnL - lpfL;
        xnR = xnR - lpfR;
    }

    real m_alpha = 1; // Feed Forward coeff
    real m_beta = 0;
    real m_gamma = 1;    // Pre-Gain
    real m_delta = 0;    // FB_IN Coeff
    real m_epsilon = 0;  // FB_OUT scalar
    real m_a_0 = 1;      // input gain
    real m_feedbackL = 0; // our own feedback coeff from S ..... this is written to by DiodeFilter
    real m_feedbackR = 0; // our own feedback coeff from S ..... this is written to by DiodeFilter

  private:
    FilterType m_FilterType = FilterType::LP;
    real m_cutoffHz = 10000;

    real m_z_1L = 0; // z-1 storage location, left/mono
    real m_z_1R = 0; // z-1 storage location, right

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
