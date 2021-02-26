
// "Designing Software Synthesizer Plug-Ins in C++" // https://willpirkle.com

#pragma once

#include "OnePoleFilter.hpp"

namespace clarinoid
{
  namespace filters
  {
    struct MoogLadderFilter : IFilter
    {
      MoogLadderFilter()
      {
        m_LPF1.SetType(FilterType::LP);
        m_LPF2.SetType(FilterType::LP);
        m_LPF3.SetType(FilterType::LP);
        m_LPF4.SetType(FilterType::LP);
      }

      // IFilter
      virtual void SetType(FilterType type) override
      {
        switch (type)
        {
        default:
        case FilterType::LP:
          m_FilterType = FilterType::LP4;
          Recalc();
          break;
        case FilterType::BP:
          m_FilterType = FilterType::BP4;
          Recalc();
          break;
        case FilterType::HP:
          m_FilterType = FilterType::HP4;
          Recalc();
          break;
        case FilterType::LP2:
        case FilterType::LP4:
        case FilterType::BP2:
        case FilterType::BP4:
        case FilterType::HP2:
        case FilterType::HP4:
          m_FilterType = type;
          Recalc();
          break;
        }
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
      virtual void SetResonance(real p_res)
      {
        // this maps dQControl = 0->1 to 0-4 * 0.97 to avoid clippy self oscillation
        m_k = Real(3.88) * p_res;
        m_k = ClampInclusive(m_k, Real0, Real(3.88));
        Recalc();
      }

      virtual void SetParams(FilterType type, real cutoffHz, real reso, real saturation) override
      {
        switch (type)
        {
        default:
        case FilterType::LP:
          m_FilterType = FilterType::LP4;
          break;
        case FilterType::BP:
          m_FilterType = FilterType::BP4;
          break;
        case FilterType::HP:
          m_FilterType = FilterType::HP4;
          break;
        case FilterType::LP2:
        case FilterType::LP4:
        case FilterType::BP2:
        case FilterType::BP4:
        case FilterType::HP2:
        case FilterType::HP4:
          m_FilterType = type;
          break;
        }

        m_cutoffHz = cutoffHz;
        m_overdrive = saturation;
        m_k = Real(3.88) * reso;
        m_k = ClampInclusive(m_k, Real0, Real(3.88));
        Recalc();
      }

      virtual void ProcessInPlace(real *samples, size_t sampleCount) override
      {
        for (size_t i = 0; i < sampleCount; ++i)
        {
          samples[i] = InlineProcessSample(samples[i]);
        }
      }

      virtual real ProcessSample(real x) override { return InlineProcessSample(x); }

      virtual void Reset() override
      {
        m_LPF1.Reset();
        m_LPF2.Reset();
        m_LPF3.Reset();
        m_LPF4.Reset();
      }

      inline real InlineProcessSample(real xn)
      {
        real dSigma = m_LPF1.getFeedbackOutput() + m_LPF2.getFeedbackOutput() +
                      m_LPF3.getFeedbackOutput() + m_LPF4.getFeedbackOutput();

        // calculate input to first filter
        real dU = (xn - m_k * dSigma) * m_alpha_0;

        // --- cascade of 4 filters
        real dLP1 = m_LPF1.InlineProcessSample(dU);
        real dLP2 = m_LPF2.InlineProcessSample(dLP1);
        real dLP3 = m_LPF3.InlineProcessSample(dLP2);
        real dLP4 = m_LPF4.InlineProcessSample(dLP3);

        // --- Oberheim variations
        real output =
            m_a * dU + m_b * dLP1 + m_c * dLP2 + m_d * dLP3 + m_e * dLP4;

        applyOverdrive(output, m_overdrive, Real(3.5));

        return output;
      }

    private:
      void Recalc()
      {
        // prewarp for BZT
        real wd = PITimes2 * m_cutoffHz;

        //note: measured input to tan function, it seemed limited to (0.005699, 1.282283).
        //input for fasttan shall be limited to (-pi/2, pi/2) according to documentation
        real wa = (2 * SampleRate) * ::tan(wd * OneOverSampleRate * Real(0.5));
        real g = wa * OneOverSampleRate * Real(0.5);

        // G - the feedforward coeff in the VA One Pole
        //     same for LPF, HPF
        real G = g / (Real1 + g);

        // set alphas
        m_LPF1.m_alpha = G;
        m_LPF2.m_alpha = G;
        m_LPF3.m_alpha = G;
        m_LPF4.m_alpha = G;

        // set betas
        m_LPF1.m_beta = G * G * G / (Real1 + g);
        m_LPF2.m_beta = G * G / (Real1 + g);
        m_LPF3.m_beta = G / (Real1 + g);
        m_LPF4.m_beta = Real1 / (Real1 + g);

        m_gamma = G * G * G * G;

        m_alpha_0 = Real1 / (Real1 + m_k * m_gamma);

        // Oberheim variation
        switch (m_FilterType)
        {
        case FilterType::LP4:
          m_a = Real(0.0);
          m_b = Real(0.0);
          m_c = Real(0.0);
          m_d = Real(0.0);
          m_e = Real(1.0);
          break;
        case FilterType::LP2:
          m_a = Real(0.0);
          m_b = Real(0.0);
          m_c = Real(1.0);
          m_d = Real(0.0);
          m_e = Real(0.0);
          break;
        case FilterType::BP4:
          m_a = Real(0.0);
          m_b = Real(0.0);
          m_c = Real(4.0);
          m_d = Real(-8.0);
          m_e = Real(4.0);
          break;
        case FilterType::BP2:
          m_a = Real(0.0);
          m_b = Real(2.0);
          m_c = Real(-2.0);
          m_d = Real(0.0);
          m_e = Real(0.0);
          break;
        case FilterType::HP4:
          m_a = Real(1.0);
          m_b = Real(-4.0);
          m_c = Real(6.0);
          m_d = Real(-4.0);
          m_e = Real(1.0);
          break;
        case FilterType::HP2:
          m_a = Real(1.0);
          m_b = Real(-2.0);
          m_c = Real(1.0);
          m_d = Real(0.0);
          m_e = Real(0.0);
          break;
        }
      }

      FilterType m_FilterType = FilterType::LP4;
      real m_cutoffHz = 10000;
      real m_overdrive = 0;

      OnePoleFilter m_LPF1;
      OnePoleFilter m_LPF2;
      OnePoleFilter m_LPF3;
      OnePoleFilter m_LPF4;

      real m_k = 0;       // K, set with Q
      real m_gamma;       // see block diagram
      real m_alpha_0 = 1; // see block diagram

      // Oberheim Xpander variations
      real m_a = 0;
      real m_b = 0;
      real m_c = 0;
      real m_d = 0;
      real m_e = 0;
    };

  } // namespace filters
} // namespace clarinoid
