#pragma once

#include <algorithm>
#include <cfloat>

#include <clarinoid/basic/Basic.hpp>
//#include <clarinoid/loopstation/LoopstationMemory.hpp>
#include <clarinoid/synth/filters/filters.hpp>

namespace clarinoid
{

class FilterNode : public AudioStream
{
    filters::OnePoleFilter mOnePole;
    filters::SEM12Filter mSEM12;
    filters::DiodeFilter mDiode;
    filters::K35Filter mK35;
    filters::MoogLadderFilter mMoog;

    filters::IFilter *mSelectedFilter = &mK35;

    bool mDCEnabled = false;
    float mDCCutoff = 0;
    filters::DCFilter mDC;

  public:
    FilterNode() : AudioStream(1, inputQueueArray)
    {
        SetParams(ClarinoidFilterType::LP_K35, 10000, 0.1f, 0.1f);
    }

    virtual void update() override
    {
        audio_block_t *block = receiveWritable();
        if (!block)
            return;
        int16_t *p = block->data;
        float tempBufferSource[AUDIO_BLOCK_SAMPLES];
        fast::Sample16To32Buffer(p, tempBufferSource);
        mSelectedFilter->ProcessInPlace(tempBufferSource, AUDIO_BLOCK_SAMPLES);
        if (mDCEnabled)
        {
            mDC.ProcessInPlace(tempBufferSource, AUDIO_BLOCK_SAMPLES);
        }

        fast::Sample32To16Buffer(tempBufferSource, p);

        transmit(block);
        release(block);
    }

    void EnableDCFilter(bool enabled, float cutoffHZ)
    {
        if (!enabled && !mDCEnabled)
            return;
        if (enabled && mDCEnabled && FloatEquals(cutoffHZ, mDCCutoff))
            return;
        mDCEnabled = enabled;
        if (enabled)
        {
            mDC.SetMinus3DBFreq(cutoffHZ);
        }
    }

    void SetParams(ClarinoidFilterType ctype, float cutoffHz, float reso, float saturation)
    {
        // select filter & set type
        filters::FilterType ft = filters::FilterType::LP;
        switch (ctype)
        {
        case ClarinoidFilterType::LP_OnePole:
            ft = filters::FilterType::LP;
            mSelectedFilter = &mOnePole;
            break;
        case ClarinoidFilterType::LP_SEM12:
            ft = filters::FilterType::LP;
            mSelectedFilter = &mSEM12;
            break;
        case ClarinoidFilterType::LP_Diode:
            ft = filters::FilterType::LP;
            mSelectedFilter = &mDiode;
            break;
        default:
        case ClarinoidFilterType::LP_K35:
            ft = filters::FilterType::LP;
            mSelectedFilter = &mK35;
            break;
        case ClarinoidFilterType::LP_Moog2:
            ft = filters::FilterType::LP2;
            mSelectedFilter = &mMoog;
            break;
        case ClarinoidFilterType::LP_Moog4:
            ft = filters::FilterType::LP4;
            mSelectedFilter = &mMoog;
            break;
        case ClarinoidFilterType::HP_OnePole:
            ft = filters::FilterType::HP;
            mSelectedFilter = &mOnePole;
            break;
        case ClarinoidFilterType::HP_K35:
            ft = filters::FilterType::HP;
            mSelectedFilter = &mK35;
            break;
        case ClarinoidFilterType::HP_Moog2:
            ft = filters::FilterType::HP2;
            mSelectedFilter = &mMoog;
            break;
        case ClarinoidFilterType::HP_Moog4:
            ft = filters::FilterType::HP4;
            mSelectedFilter = &mMoog;
            break;
        case ClarinoidFilterType::BP_Moog2:
            ft = filters::FilterType::BP2;
            mSelectedFilter = &mMoog;
            break;
        case ClarinoidFilterType::BP_Moog4:
            ft = filters::FilterType::BP4;
            mSelectedFilter = &mMoog;
            break;
        }
        mSelectedFilter->SetParams(ft, cutoffHz, reso, saturation);
    }

    audio_block_t *inputQueueArray[1];
};

class StereoFilterNode : public AudioStream
{
    filters::OnePoleFilter mOnePole;
    filters::SEM12Filter mSEM12;
    filters::DiodeFilter mDiode;
    filters::K35Filter mK35;
    filters::MoogLadderFilter mMoog;

    filters::IFilter *mSelectedFilter = &mK35;
    ClarinoidFilterType mSelectedFilterType = ClarinoidFilterType::LP_K35;

    bool mDCEnabled = false;
    float mDCCutoff = 0;
    filters::DCFilter mDC;

    float mRampGain = 1.0f;
    static constexpr int mRampLengthSamples = AUDIO_SAMPLE_RATE_EXACT * 250 / 1000; // 250 ms ramp
    static constexpr float mRampGainIncreasePerFrame = 1.0f / mRampLengthSamples * AUDIO_BLOCK_SAMPLES;

    audio_block_t *inputQueueArray[2];

  public:
    StereoFilterNode() : AudioStream(2, inputQueueArray)
    {
        SetParams(ClarinoidFilterType::LP_K35, 10000, 0.1f, 0.1f);
    }

    virtual void update() override
    {
        if (!mEnabled)
            return;
        audio_block_t *blockL = receiveWritable(0);
        if (!blockL)
        {
            return;
        }

        audio_block_t *blockR = receiveWritable(1);
        if (!blockR)
        {
            release(blockL);
            return;
        }

        int16_t *pL = blockL->data;
        int16_t *pR = blockR->data;
        float tempBufferSourceL[AUDIO_BLOCK_SAMPLES];
        float tempBufferSourceR[AUDIO_BLOCK_SAMPLES];
        fast::Sample16To32Buffer(pL, tempBufferSourceL);
        fast::Sample16To32Buffer(pR, tempBufferSourceR);
        mSelectedFilter->ProcessInPlace(tempBufferSourceL, tempBufferSourceR, AUDIO_BLOCK_SAMPLES);
        if (mDCEnabled)
        {
            mDC.ProcessInPlace(tempBufferSourceL, tempBufferSourceR, AUDIO_BLOCK_SAMPLES);
        }
        arm_scale_f32(tempBufferSourceL, mRampGain, tempBufferSourceL, AUDIO_BLOCK_SAMPLES);
        arm_scale_f32(tempBufferSourceR, mRampGain, tempBufferSourceR, AUDIO_BLOCK_SAMPLES);
        fast::Sample32To16Buffer(tempBufferSourceL, pL);
        fast::Sample32To16Buffer(tempBufferSourceR, pR);
        mRampGain = std::min(1.0f, mRampGain + mRampGainIncreasePerFrame);

        transmit(blockL, 0);
        release(blockL);
        transmit(blockR, 1);
        release(blockR);
    }

    void EnableDCFilter(bool enabled, float cutoffHZ)
    {
        if (!enabled && !mDCEnabled)
            return;
        if (enabled && mDCEnabled && FloatEquals(cutoffHZ, mDCCutoff))
            return;
        mDCCutoff = cutoffHZ;
        mDCEnabled = enabled;
        if (enabled)
        {
            mDC.SetMinus3DBFreq(cutoffHZ);
        }
    }

    bool mEnabled = true;
    void Disable()
    {
        mEnabled = false;
    }
    void Enable()
    {
        mEnabled = true;
    }

    void ResetFilterState()
    {
        mOnePole.Reset();
        mSEM12.Reset();
        mDiode.Reset();
        mK35.Reset();
        mMoog.Reset();
    }

    void TriggerRamp()
    {
        mRampGain = 0.0f;
    }

    void SetParams(ClarinoidFilterType ctype, float cutoffHz, float reso, float saturation)
    {
        // select filter & set type
        filters::FilterType ft = filters::FilterType::LP;
        bool filterTypeChanged = mSelectedFilterType != ctype;
        if (filterTypeChanged)
        {
            TriggerRamp();
        }
        switch (ctype)
        {
        case ClarinoidFilterType::LP_OnePole:
            ft = filters::FilterType::LP;
            mSelectedFilter = &mOnePole;
            break;
        case ClarinoidFilterType::LP_SEM12:
            ft = filters::FilterType::LP;
            mSelectedFilter = &mSEM12;
            break;
        case ClarinoidFilterType::LP_Diode:
            ft = filters::FilterType::LP;
            mSelectedFilter = &mDiode;
            break;
        default:
        case ClarinoidFilterType::LP_K35:
            ft = filters::FilterType::LP;
            mSelectedFilter = &mK35;
            break;
        case ClarinoidFilterType::LP_Moog2:
            ft = filters::FilterType::LP2;
            mSelectedFilter = &mMoog;
            break;
        case ClarinoidFilterType::LP_Moog4:
            ft = filters::FilterType::LP4;
            mSelectedFilter = &mMoog;
            break;
        case ClarinoidFilterType::HP_OnePole:
            ft = filters::FilterType::HP;
            mSelectedFilter = &mOnePole;
            break;
        case ClarinoidFilterType::HP_K35:
            ft = filters::FilterType::HP;
            mSelectedFilter = &mK35;
            break;
        case ClarinoidFilterType::HP_Moog2:
            ft = filters::FilterType::HP2;
            mSelectedFilter = &mMoog;
            break;
        case ClarinoidFilterType::HP_Moog4:
            ft = filters::FilterType::HP4;
            mSelectedFilter = &mMoog;
            break;
        case ClarinoidFilterType::BP_Moog2:
            ft = filters::FilterType::BP2;
            mSelectedFilter = &mMoog;
            break;
        case ClarinoidFilterType::BP_Moog4:
            ft = filters::FilterType::BP4;
            mSelectedFilter = &mMoog;
            break;
        }
        mSelectedFilterType = ctype;
        mSelectedFilter->SetParams(ft, cutoffHz, reso, saturation);
    }
};

} // namespace clarinoid
