#pragma once

#include <clarinoid/basic/Basic.hpp>
#include "AudioBufferUtils.hpp"

namespace clarinoid
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// handles modulation routing (which modulation sources apply to which destinations) for VOICES.
// inputs are modulation sources,
struct VoiceModulationMatrixNode : public AudioStream
{
    audio_block_t *inputQueueArray[ModulationSourceViableCount];
    SynthPreset *mSynthPatch = nullptr;

    CCThrottlerT<35> mth;

    VoiceModulationMatrixNode() : AudioStream(ModulationSourceViableCount, inputQueueArray)
    {
    }

    void SetSynthPatch(SynthPreset *patch)
    {
        mSynthPatch = patch;
    }

    virtual void update() override
    {
        if (!mSynthPatch)
            return;

        // mind ModulationSourceSkip and ModulationDestinationSkip when converting between indices & enum values.
        audio_block_t *sources[ModulationSourceViableCount] = {nullptr};
        audio_block_t *destinations[ModulationDestinationViableCount] = {nullptr};

        auto processModulation = [&](SynthModulationSpec &modulation) {
            // modulation.mSource is a kind of ModulationSource,
            // but since we just line up indices & enum values, no need to switch().
            size_t sourceIndex;
            size_t destIndex;

            // ensure the mod mapping is valid & map to real indices
            if (!ModulationSourceToIndex(modulation.mSource, sourceIndex))
                return;
            if (!ModulationDestinationToIndex(modulation.mDest, destIndex))
                return;

            // ensure we have a source
            audio_block_t *source = sources[sourceIndex];
            if (!source)
            {
                source = sources[sourceIndex] = receiveReadOnly(sourceIndex);
            }
            if (!source)
                return;

            // ensure we have a destination
            audio_block_t *dest = destinations[destIndex];
            if (!dest)
            {
                dest = destinations[destIndex] = allocate();
                if (!dest)
                    return; // no memory? just act like this iteration didn't happen; we should proceed gracefully as
                            // if this mapping doesn't exist.
                // the first time the destination is allocated, it should be INITIALIZED.
                audioBufferCopyAndApplyGain(source->data, dest->data, modulation.mScaleN11);
            }
            else
            {
                // the subsequent times the same destination is used, it should be ADDED.
                audioBufferMixInPlaceWithGain(
                    dest->data, source->data, gainToSignedMultiply32x16(modulation.mScaleN11));
            }
        };

        for (auto &modulation : mSynthPatch->mModulations)
        {
            processModulation(modulation);
        }

        // process FM feedback modulations.
        // SynthModulationSpec fb1;
        // fb1.mSource = ModulationSource::Osc1FB;
        // fb1.mDest = ModulationDestination::Osc1Phase;
        // fb1.mScaleN11 = mSynthPatch->mOsc[0].mFMFeedbackGain;
        // processModulation(fb1);

        // SynthModulationSpec fb2;
        // fb2.mSource = ModulationSource::Osc2FB;
        // fb2.mDest = ModulationDestination::Osc2Phase;
        // fb2.mScaleN11 = mSynthPatch->mOsc[1].mFMFeedbackGain;
        // processModulation(fb2);

        // SynthModulationSpec fb3;
        // fb3.mSource = ModulationSource::Osc3FB;
        // fb3.mDest = ModulationDestination::Osc3Phase;
        // fb3.mScaleN11 = mSynthPatch->mOsc[2].mFMFeedbackGain;
        // processModulation(fb3);

        // process FB algo modulations
        SynthModulationSpec fm3to2;
        fm3to2.mSource = ModulationSource::Osc3FB;
        fm3to2.mDest = ModulationDestination::Osc2Phase;
        fm3to2.mScaleN11 = mSynthPatch->mOsc[2].mGain;

        SynthModulationSpec fm3to1;
        fm3to1.mSource = ModulationSource::Osc3FB;
        fm3to1.mDest = ModulationDestination::Osc1Phase;
        fm3to1.mScaleN11 = mSynthPatch->mOsc[2].mGain;

        SynthModulationSpec fm2to1;
        fm2to1.mSource = ModulationSource::Osc2FB;
        fm2to1.mDest = ModulationDestination::Osc1Phase;
        fm2to1.mScaleN11 = mSynthPatch->mOsc[1].mGain;

        switch (mSynthPatch->mFMAlgo)
        {
        default:
        case FMAlgo::c1c2c3_NoFM: // [1][2][3]
            // nop. no modulations necessary.
            break;
        case FMAlgo::c1m2m3_Chain: // [1<2<3]
            processModulation(fm3to2);
            processModulation(fm2to1);
            break;
        case FMAlgo::c1m2c3_FM12_NoFM3: // [1<2][3]
            processModulation(fm2to1);
            break;
        case FMAlgo::c1m23: // [1<(2&3)]
            processModulation(fm3to1);
            processModulation(fm2to1);
            break;
        }

        for (auto *src : sources)
        {
            if (!src)
                continue;
            release(src);
        }

        for (size_t i = 0; i < SizeofStaticArray(destinations); ++i)
        {
            if (!destinations[i])
                continue;
            transmit(destinations[i], i);
            release(destinations[i]);
        }
    }
};

} // namespace clarinoid
