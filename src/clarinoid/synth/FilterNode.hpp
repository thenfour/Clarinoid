#pragma once

#include <algorithm>
#include <cfloat>

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/loopstation/LoopstationMemory.hpp>
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
        for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
        {
            tempBufferSource[i] = (float)p[i] / 32767.0f;
        }
        mSelectedFilter->ProcessInPlace(tempBufferSource, AUDIO_BLOCK_SAMPLES);
        if (mDCEnabled)
        {
            mDC.ProcessInPlace(tempBufferSource, AUDIO_BLOCK_SAMPLES);
        }
        for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; ++i)
        {
            float f = tempBufferSource[i];
            p[i] = saturate16(int32_t(f * 32768.0f));
        }

        transmit(block);
        release(block);
    }

    void EnableDCFilter(bool enabled, float cutoffHZ)
    {
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

} // namespace clarinoid
