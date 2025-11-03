#pragma once

#include <AudioStream.h>
#include <algorithm>
#include <cmath>

#include <clarinoid/basic/Math.hpp>

namespace clarinoid
{
namespace detail
{
inline float CalcFollowerCoef(float windowMS)
{
    if (windowMS <= 0.0f)
    {
        return 0.0f;
    }
    float samples = windowMS * kSampleRateF / 1000.0f;
    if (samples < 1.0f)
    {
        samples = 1.0f;
    }
    return std::exp(-1.0f / samples);
}
} // namespace detail

class RMSDetector
{
    float mContinuousValue = 0.0f;
    float mWindowMS = -1.0f;
    float mAlpha = 1.0f;

  public:
    void Reset()
    {
        mContinuousValue = 0.0f;
    }

    void SetWindowSize(float windowMS)
    {
        if (std::fabs(windowMS - mWindowMS) < 1e-6f)
        {
            return;
        }
        mWindowMS = windowMS;
        float follower = detail::CalcFollowerCoef(windowMS);
        mAlpha = 1.0f - follower;
    }

    float ProcessSample(float sample)
    {
        float squared = sample * sample;
        mContinuousValue += mAlpha * (squared - mContinuousValue);
        return std::sqrt(std::max(0.0f, mContinuousValue));
    }
};

class PeakDetector
{
    int mClipHoldSamples = 0;
    int mPeakHoldSamples = 0;
    float mPeakFalloffPerSample = 1.0f;

    int mClipHoldCounter = 0;
    int mPeakHoldCounter = 0;
    bool mClipIndicator = false;
    float mCurrentPeak = 0.0f;

  public:
    void SetParams(float clipHoldMS, float peakHoldMS, float peakFalloffMaxMS)
    {
        mClipHoldSamples = static_cast<int>(std::max(0.0f, clipHoldMS) * kSampleRateF / 1000.0f + 0.5f);
        mPeakHoldSamples = static_cast<int>(std::max(0.0f, peakHoldMS) * kSampleRateF / 1000.0f + 0.5f);

        if (peakFalloffMaxMS <= 0.0f)
        {
            mPeakFalloffPerSample = 1.0f;
        }
        else
        {
            float falloffSamples = peakFalloffMaxMS * kSampleRateF / 1000.0f;
            if (falloffSamples < 1.0f)
            {
                falloffSamples = 1.0f;
            }
            mPeakFalloffPerSample = std::max(1.0f / falloffSamples, 1e-7f);
        }

        Reset();
    }

    void Reset()
    {
        mClipHoldCounter = 0;
        mPeakHoldCounter = 0;
        mClipIndicator = false;
        mCurrentPeak = 0.0f;
    }

    void ProcessSample(float sample)
    {
        float rectified = std::fabs(sample);

        if (rectified >= 1.0f)
        {
            mClipIndicator = true;
            mClipHoldCounter = mClipHoldSamples;
        }
        else if (mClipHoldCounter > 0)
        {
            --mClipHoldCounter;
        }
        else
        {
            mClipIndicator = false;
        }

        if (rectified >= mCurrentPeak)
        {
            mCurrentPeak = rectified;
            mPeakHoldCounter = mPeakHoldSamples;
        }
        else if (mPeakHoldCounter > 0)
        {
            --mPeakHoldCounter;
        }
        else if (mCurrentPeak > 0.0f)
        {
            mCurrentPeak = std::max(0.0f, mCurrentPeak - mPeakFalloffPerSample);
        }
    }

    float CurrentPeak() const
    {
        return mCurrentPeak;
    }

    bool ClipIndicator() const
    {
        return mClipIndicator;
    }
};

struct AudioAnalysisState
{
    // All linear amplitudes normalised to 0-1.
    float rmsLinear = 0.0f;
    bool isClipping = false;
    float peakLinear = 0.0f;
    float heldPeakLinear = 0.0f;
};

struct IAnalysisStream
{
    virtual ~IAnalysisStream() = default;
    virtual void Reset() = 0;

    float CurrentRmsLinear() const
    {
        return mCurrentRmsLinear;
    }

    bool IsClipping() const
    {
        return mIsClipping;
    }

    float CurrentPeakLinear() const
    {
        return mCurrentPeakLinear;
    }

    float CurrentHeldPeakLinear() const
    {
        return mCurrentHeldPeakLinear;
    }

    AudioAnalysisState GetState() const
    {
        AudioAnalysisState state;
        state.rmsLinear = mCurrentRmsLinear;
        state.isClipping = mIsClipping;
        state.peakLinear = mCurrentPeakLinear;
        state.heldPeakLinear = mCurrentHeldPeakLinear;
        return state;
    }

    // lake L and R channel analysis states and combine into center / mono channel, for single channel analysis
    static AudioAnalysisState Combine(const AudioAnalysisState &l, const AudioAnalysisState &r)
    {
        AudioAnalysisState combined;
        combined.rmsLinear = std::sqrt(0.5f * (l.rmsLinear * l.rmsLinear + r.rmsLinear * r.rmsLinear));
        combined.isClipping = l.isClipping || r.isClipping;
        combined.peakLinear = std::max(l.peakLinear, r.peakLinear);
        combined.heldPeakLinear = std::max(l.heldPeakLinear, r.heldPeakLinear);
        return combined;
    }

  protected:
    volatile float mCurrentRmsLinear = 0.0f;
    volatile bool mIsClipping = false;
    volatile float mCurrentPeakLinear = 0.0f;
    volatile float mCurrentHeldPeakLinear = 0.0f;
};

class AnalysisStream : public AudioStream, public IAnalysisStream
{
  public:
    explicit AnalysisStream(float peakFalloffMS = 1200.0f) : AudioStream(2, mInputQueueArray)
    {
        mRMSDetector.SetWindowSize(200.0f);
        mPeakDetector.SetParams(0.0f, 0.0f, peakFalloffMS);
        mPeakHoldDetector.SetParams(1000.0f, 1000.0f, 600.0f);
        Reset();
    }

    virtual void update() override
    {
        audio_block_t *blockL = receiveReadOnly(0);
        audio_block_t *blockR = receiveReadOnly(1);
        if (!blockL && !blockR)
        {
            return;
        }

        const int16_t *dataL = blockL ? blockL->data : nullptr;
        const int16_t *dataR = blockR ? blockR->data : nullptr;

        float latestRms = 0.0f;
        for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
        {
            float sampleL = dataL ? fast::Sample16To32(dataL[i]) : 0.0f;
            float sampleR = dataR ? fast::Sample16To32(dataR[i]) : 0.0f;
            if (!dataL && dataR)
            {
                sampleL = sampleR;
            }
            else if (!dataR && dataL)
            {
                sampleR = sampleL;
            }

            float sampleRMS = std::sqrt(0.5f * (sampleL * sampleL + sampleR * sampleR));
            float samplePeak = std::max(std::fabs(sampleL), std::fabs(sampleR));

            latestRms = mRMSDetector.ProcessSample(sampleRMS);
            mPeakDetector.ProcessSample(samplePeak);
            mPeakHoldDetector.ProcessSample(samplePeak);
        }

        mCurrentRmsLinear = latestRms;
        mIsClipping = mPeakHoldDetector.ClipIndicator();
        mCurrentPeakLinear = mPeakDetector.CurrentPeak();
        mCurrentHeldPeakLinear = mPeakHoldDetector.CurrentPeak();

        if (blockL)
        {
            release(blockL);
        }
        if (blockR)
        {
            release(blockR);
        }
    }

    virtual void Reset() override
    {
        mRMSDetector.Reset();
        mPeakDetector.Reset();
        mPeakHoldDetector.Reset();
        mCurrentRmsLinear = 0.0f;
        mIsClipping = false;
        mCurrentPeakLinear = 0.0f;
        mCurrentHeldPeakLinear = 0.0f;
    }

    void SetRMSWindow(float windowMS)
    {
        mRMSDetector.SetWindowSize(windowMS);
    }

    void SetPeakParams(float clipHoldMS, float peakHoldMS, float peakFalloffMS)
    {
        mPeakDetector.SetParams(clipHoldMS, peakHoldMS, peakFalloffMS);
    }

    void SetPeakHoldParams(float clipHoldMS, float peakHoldMS, float peakFalloffMS)
    {
        mPeakHoldDetector.SetParams(clipHoldMS, peakHoldMS, peakFalloffMS);
    }

  private:
    audio_block_t *mInputQueueArray[2];
    RMSDetector mRMSDetector;
    PeakDetector mPeakDetector;
    PeakDetector mPeakHoldDetector;
};

extern AudioAnalysisState gAnalysisStateL;
extern AudioAnalysisState gAnalysisStateC;
extern AudioAnalysisState gAnalysisStateR;

} // namespace clarinoid
