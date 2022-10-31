
#pragma once

namespace clarinoid
{
enum class EnvelopeStage : uint8_t
{
    Idle,    // output = 0
    Delay,   // output = 0
    Attack,  // output = curve from 0 to 1
    Hold,    // output = 1
    Decay,   // output = curve from 1 to 0
    Sustain, // output = SustainLevel
    Release, // output = curve from Y to 0, based on when release occurred
};

EnumItemInfo<EnvelopeStage> gEnvelopeStageItems[7] = {
    {EnvelopeStage::Idle, "Idle"},
    {EnvelopeStage::Delay, "Delay"},
    {EnvelopeStage::Attack, "Attack"},
    {EnvelopeStage::Hold, "Hold"},
    {EnvelopeStage::Decay, "Decay"},
    {EnvelopeStage::Sustain, "Sustain"},
    {EnvelopeStage::Release, "Release"},
};

EnumInfo<EnvelopeStage> gEnvelopeStageInfo("EnvelopeStage", gEnvelopeStageItems);

struct EnvelopeNode : public AudioStream
{
    EnvelopeNode() : AudioStream(0, nullptr)
    {
    }

    // advances to a specified stage. The point of this is to advance through 0-length stages so we don't end up with
    // >=1sample per stage.
    void AdvanceToStage(EnvelopeStage stage)
    {
        switch (stage)
        {
        case EnvelopeStage::Idle:
            break;
        case EnvelopeStage::Delay:
            if (FloatLessThanOrEquals(mSpec.mDelayMS, 0.0f))
            {
                AdvanceToStage(EnvelopeStage::Attack);
                return;
            }
            break;
        case EnvelopeStage::Attack:
            if (FloatLessThanOrEquals(mSpec.mAttackMS, 0.0f))
            {
                AdvanceToStage(EnvelopeStage::Hold);
                return;
            }
            break;
        case EnvelopeStage::Hold:
            if (FloatLessThanOrEquals(mSpec.mHoldMS, 0.0f))
            {
                AdvanceToStage(EnvelopeStage::Decay);
                return;
            }
            break;
        case EnvelopeStage::Decay:
            if (FloatLessThanOrEquals(mSpec.mDecayMS, 0.0f))
            {
                AdvanceToStage(EnvelopeStage::Decay);
                return;
            }
            break;
        case EnvelopeStage::Sustain:
            break;
        case EnvelopeStage::Release:
            if (FloatLessThanOrEquals(mSpec.mReleaseMS, 0.0f))
            {
                AdvanceToStage(EnvelopeStage::Idle);
                return;
            }
            // here we must determine the value to release from, based on existing stage.
            mReleaseFromValue01 = mLastOutputLevel;
            break;
        }
        mStagePos01 = 0.0f;
        mStage = stage;
        RecalcState();
    }

    void noteOn()
    {
        AdvanceToStage(EnvelopeStage::Delay);
    }

    void noteOff()
    {
        AdvanceToStage(EnvelopeStage::Release);
    }

    // used by debug displays
    EnvelopeStage GetStage() const
    {
        return this->mStage;
    }

    bool isPlaying() const
    {
        if (this->mStage == EnvelopeStage::Delay)
            return false;
        return (this->mStage != EnvelopeStage::Idle);
    }

    void SetSpec(const EnvelopeSpec &spec)
    {
        mSpec = spec;
        // bug: if you set the spec and the current stage becomes 0-length, 1 sample will be processed for it. nobody
        // cares.
        RecalcState();
    }

    float ProcessSample()
    {
        float ret = 0;
        EnvelopeStage nextStage = EnvelopeStage::Idle;

        switch (mStage)
        {
        default:
        case EnvelopeStage::Idle: {
            return 0;
        }
        case EnvelopeStage::Delay: {
            ret = 0;
            nextStage = EnvelopeStage::Attack;
            break;
        }
        case EnvelopeStage::Attack: {
            ret = gModCurveLUT.Transfer32(mStagePos01, mpLutRow); // 0-1
            nextStage = EnvelopeStage::Hold;
            break;
        }
        case EnvelopeStage::Hold: {
            ret = 1;
            nextStage = EnvelopeStage::Decay;
            break;
        }
        case EnvelopeStage::Decay: {
            // 0-1 => 1 - sustainlevel
            // curve contained within the stage, not the output 0-1 range.
            float range = 1.0f - mSpec.mSustainLevel; // could be precalculated
            ret = 1.0f - gModCurveLUT.Transfer32(1.0f - mStagePos01, mpLutRow); // 0-1
            ret = 1.0f - range * ret;
            nextStage = EnvelopeStage::Sustain;
            break;
        }
        case EnvelopeStage::Sustain: {
            return mSpec.mSustainLevel;
        }
        case EnvelopeStage::Release: {
            // 0-1 => mReleaseFromValue01 - 0
            // curve contained within the stage, not the output 0-1 range.
            ret = gModCurveLUT.Transfer32(1.0f - mStagePos01, mpLutRow); // 1-0
            ret = ret * mReleaseFromValue01;
            nextStage = EnvelopeStage::Idle;
            break;
        }
        }

        mStagePos01 += mStagePosIncPerSample;
        if (mStagePos01 >= 1.0f)
        {
            AdvanceToStage(nextStage);
        }
        return ret;
    }

    virtual void update(void)
    {
        audio_block_t *block = this->allocate();

        for (size_t i = 0; i < SizeofStaticArray(block->data); ++i)
        {
            mLastOutputLevel = ProcessSample();
            block->data[i] = fast::Sample32To16(mLastOutputLevel);
        }

        transmit(block);
        release(block);
    }

  private:
    void RecalcState()
    {
        switch (mStage)
        {
        default:
        case EnvelopeStage::Idle:
            return;
        case EnvelopeStage::Delay:
            mStagePosIncPerSample = CalculateInc01PerSampleForMS(mSpec.mDelayMS);
            return;
        case EnvelopeStage::Attack:
            mStagePosIncPerSample = CalculateInc01PerSampleForMS(mSpec.mAttackMS);
            mpLutRow = gModCurveLUT.BeginLookupI(mSpec.mAttackCurve);
            return;
        case EnvelopeStage::Hold:
            mStagePosIncPerSample = CalculateInc01PerSampleForMS(mSpec.mHoldMS);
            return;
        case EnvelopeStage::Decay:
            mStagePosIncPerSample = CalculateInc01PerSampleForMS(mSpec.mDecayMS);
            mpLutRow = gModCurveLUT.BeginLookupI(mSpec.mDecayCurve);
            return;
        case EnvelopeStage::Sustain:
            return;
        case EnvelopeStage::Release:
            mStagePosIncPerSample = CalculateInc01PerSampleForMS(mSpec.mReleaseMS);
            mpLutRow = gModCurveLUT.BeginLookupI(mSpec.mReleaseCurve);
            return;
        }
    }

    EnvelopeStage mStage = EnvelopeStage::Idle;
    float mStagePos01 = 0.0f; // where in the current stage are we?
    float mLastOutputLevel = 0.0f; // used to calculated release from value.
    float mStagePosIncPerSample =
        0.0f; // how much mStagePos01 changes per sample (recalculated when mStage changes, or when spec changes)

    q15_t *mpLutRow = nullptr; // for curved operations, this is the current transfer kernel

    float mReleaseFromValue01 = 0.0f; // when release stage begins, what value is it releasing from?

    EnvelopeSpec mSpec;
};

} // namespace clarinoid
