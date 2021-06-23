#pragma once

#include <clarinoid/basic/Basic.hpp>
#include "AudioBufferUtils.hpp"

namespace clarinoid
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// handles modulation routing (which modulation sources apply to which destinations) for VOICES.
// inputs are modulation sources,
// destinations are modulation destinations
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
        audio_block_t *destinations[ModulationSourceViableCount] = {nullptr};

        for (auto &modulation : mSynthPatch->mModulations)
        {
            // modulation.mSource is a kind of ModulationSource,
            // but since we just line up indices & enum values, no need to switch().
            size_t sourceIndex;
            size_t destIndex;

            // ensure the mod mapping is valid & map to real indices
            if (!ModulationSourceToIndex(modulation.mSource, sourceIndex))
                continue;
            if (!ModulationDestinationToIndex(modulation.mDest, destIndex))
                continue;

            // ensure we have a source
            audio_block_t *source = sources[sourceIndex];
            if (!source)
            {
                source = sources[sourceIndex] = receiveReadOnly(sourceIndex);
            }
            if (!source)
                continue;

            // ensure we have a destination
            audio_block_t *dest = destinations[destIndex];
            if (!dest)
            {
                dest = destinations[destIndex] = allocate();
                if (!dest)
                    continue; // no memory? just act like this iteration didn't happen; we should proceed gracefully as
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
