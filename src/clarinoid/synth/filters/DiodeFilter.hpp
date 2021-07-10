
// "Designing Software Synthesizer Plug-Ins in C++" // https://willpirkle.com

#pragma once

#include "OnePoleFilter.hpp"

namespace clarinoid
{
namespace filters
{
struct DiodeFilter : public IFilter
{
    DiodeFilter()
    {
        m_LPF1.SetType(FilterType::LP);
        m_LPF2.SetType(FilterType::LP);
        m_LPF3.SetType(FilterType::LP);
        m_LPF4.SetType(FilterType::LP);
    }

    // IFilter
    virtual void SetType(FilterType type) override
    {
    } // only lowpass supported
    virtual FilterCapabilities GetCapabilities() override
    {
        return (FilterCapabilities)((int)FilterCapabilities::Resonance | (int)FilterCapabilities::Saturation);
    }
    virtual void SetCutoffFrequency(real hz) override
    {
        if (FloatEquals(m_cutoffHz, hz)) return;
        m_cutoffHz = hz;
        Recalc();
    }

    virtual void SetSaturation(real amt) override
    {
        if (FloatEquals(m_overdrive, amt)) return;
        m_overdrive = amt;
        Recalc();
    }

    // 0-1
    virtual void SetResonance(real amt) override
    {
        amt *= 16;
        if (FloatEquals(m_k, amt)) return;
        m_k = amt;
        Recalc();
    }

    virtual void SetParams(FilterType type, real cutoffHz, real reso, real saturation) override
    {
        reso *= 16;
        if (FloatEquals(m_cutoffHz, cutoffHz) && FloatEquals(m_overdrive, saturation) && FloatEquals(m_k, reso))
            return;
        m_cutoffHz = cutoffHz;
        m_overdrive = saturation;
        m_k = reso;
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
        m_LPF1.Reset();
        m_LPF2.Reset();
        m_LPF3.Reset();
        m_LPF4.Reset();
    }

  private:
    real m_cutoffHz = 10000;
    real m_overdrive = 0;

    real m_k = 0;
    real m_gamma = 0;
    real m_sg1 = 0;
    real m_sg2 = 0;
    real m_sg3 = 0;
    real m_sg4 = 0;

    OnePoleFilter m_LPF1;
    OnePoleFilter m_LPF2;
    OnePoleFilter m_LPF3;
    OnePoleFilter m_LPF4;

    void Recalc()
    {
        // calc alphas
        real wd = PITimes2 * m_cutoffHz;
        real wa = (Real2 * SampleRate) * ::tan(wd * OneOverSampleRate * Real(0.5));
        real g = wa * OneOverSampleRate / Real2;

        real G4 = Real(0.5) * g / (Real1 + g);
        real G3 = Real(0.5) * g / (Real1 + g - Real(0.5) * g * G4);
        real G2 = Real(0.5) * g / (Real1 + g - Real(0.5) * g * G3);
        real G1 = g / (Real1 + g - g * G2);
        m_gamma = G4 * G3 * G2 * G1;

        m_sg1 = G4 * G3 * G2;
        m_sg2 = G4 * G3;
        m_sg3 = G4;
        m_sg4 = Real1;

        real G = g / (Real1 + g);

        m_LPF1.m_alpha = G;
        m_LPF2.m_alpha = G;
        m_LPF3.m_alpha = G;
        m_LPF4.m_alpha = G;

        m_LPF1.m_beta = Real1 / (Real1 + g - g * G2);
        m_LPF2.m_beta = Real1 / (Real1 + g - Real(0.5) * g * G3);
        m_LPF3.m_beta = Real1 / (Real1 + g - Real(0.5) * g * G4);
        m_LPF4.m_beta = Real1 / (Real1 + g);

        m_LPF1.m_delta = g;
        m_LPF2.m_delta = Real(0.5) * g;
        m_LPF3.m_delta = Real(0.5) * g;
        m_LPF4.m_delta = 0;

        m_LPF1.m_gamma = Real1 + G1 * G2;
        m_LPF2.m_gamma = Real1 + G2 * G3;
        m_LPF3.m_gamma = Real1 + G3 * G4;
        m_LPF4.m_gamma = Real1;

        m_LPF1.m_epsilon = G2;
        m_LPF2.m_epsilon = G3;
        m_LPF3.m_epsilon = G4;
        m_LPF4.m_epsilon = 0;

        m_LPF1.m_a_0 = Real1;
        m_LPF2.m_a_0 = Real(0.5);
        m_LPF3.m_a_0 = Real(0.5);
        m_LPF4.m_a_0 = Real(0.5);
    }

    inline real InlineProcessSample(real xn)
    {
        m_LPF4.m_feedbackL = 0;
        m_LPF3.m_feedbackL = m_LPF4.getFeedbackOutputL();
        m_LPF2.m_feedbackL = m_LPF3.getFeedbackOutputL();
        m_LPF1.m_feedbackL = m_LPF2.getFeedbackOutputL();

        real sigmaL = m_sg1 * m_LPF1.getFeedbackOutputL() + m_sg2 * m_LPF2.getFeedbackOutputL() +
                     m_sg3 * m_LPF3.getFeedbackOutputL() + m_sg4 * m_LPF4.getFeedbackOutputL();

        real k_modded = m_k;
        k_modded = k_modded > 16 ? 16 : k_modded;
        k_modded = k_modded < 0 ? 0 : k_modded;

        // for passband gain compensation:
        xn *= Real1 + Real(0.3) * k_modded;

        xn = (xn - k_modded * sigmaL) / (Real1 + k_modded * m_gamma);

        xn = m_LPF1.InlineProcessSample(xn);
        xn = m_LPF2.InlineProcessSample(xn);
        xn = m_LPF3.InlineProcessSample(xn);
        xn = m_LPF4.InlineProcessSample(xn);

        applyOverdrive(xn, m_overdrive, Real(3.5));

        return xn;
    }

    inline void InlineProcessSample(real& xnL, real& xnR)
    {
        m_LPF4.m_feedbackL = 0;
        m_LPF3.m_feedbackL = m_LPF4.getFeedbackOutputL();
        m_LPF2.m_feedbackL = m_LPF3.getFeedbackOutputL();
        m_LPF1.m_feedbackL = m_LPF2.getFeedbackOutputL();

        real sigmaL = m_sg1 * m_LPF1.getFeedbackOutputL() + m_sg2 * m_LPF2.getFeedbackOutputL() +
                     m_sg3 * m_LPF3.getFeedbackOutputL() + m_sg4 * m_LPF4.getFeedbackOutputL();

        m_LPF4.m_feedbackR = 0;
        m_LPF3.m_feedbackR = m_LPF4.getFeedbackOutputR();
        m_LPF2.m_feedbackR = m_LPF3.getFeedbackOutputR();
        m_LPF1.m_feedbackR = m_LPF2.getFeedbackOutputR();

        real sigmaR = m_sg1 * m_LPF1.getFeedbackOutputR() + m_sg2 * m_LPF2.getFeedbackOutputR() +
                     m_sg3 * m_LPF3.getFeedbackOutputR() + m_sg4 * m_LPF4.getFeedbackOutputR();

        real k_modded = m_k;
        k_modded = k_modded > 16 ? 16 : k_modded;
        k_modded = k_modded < 0 ? 0 : k_modded;

        // for passband gain compensation:
        real c = Real1 + Real(0.3) * k_modded;
        xnL *= c;
        xnR *= c;

        xnL = (xnL - k_modded * sigmaL) / (Real1 + k_modded * m_gamma);
        xnR = (xnR - k_modded * sigmaR) / (Real1 + k_modded * m_gamma);

        m_LPF1.InlineProcessSample(xnL, xnR);
        m_LPF2.InlineProcessSample(xnL, xnR);
        m_LPF3.InlineProcessSample(xnL, xnR);
        m_LPF4.InlineProcessSample(xnL, xnR);

        applyOverdrive(xnL, xnR, m_overdrive, Real(3.5));
    }
};

} // namespace filters
} // namespace clarinoid
