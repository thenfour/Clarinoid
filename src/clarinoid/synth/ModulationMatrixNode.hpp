#pragma once

#include <clarinoid/basic/Basic.hpp>
#include "AudioBufferUtils.hpp"
#include "ModulationInfo.hpp"

namespace clarinoid
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// handles modulation routing (which modulation sources apply to which destinations) for VOICES.
// inputs are modulation sources,
struct VoiceModulationMatrixNode : public AudioStream
{
    audio_block_t *inputQueueArray[gARateModulationSourceCount];

    SynthPreset *mSynthPatch = nullptr;
    IModulationKRateProvider *mpkRateProvider;

    VoiceModulationMatrixNode() : AudioStream(gARateModulationSourceCount, inputQueueArray)
    {
    }

    void SetSynthPatch(SynthPreset *patch, IModulationKRateProvider *pkRateProvider)
    {
        mSynthPatch = patch;
        mpkRateProvider = pkRateProvider;
    }

    struct Buffers
    {
        audio_block_t *aRateSsources[SizeofStaticArray(gARateModulationSourceItems)] = {nullptr};
        float aRateSourceAsKRateN11[SizeofStaticArray(gARateModulationSourceItems)] = {
            0}; // when receiving an A-Rate buffer, immediately calculate a singular value when it should be used to
                // modulate k-rate destinations
        audio_block_t *aRateDestinations[SizeofStaticArray(gARateModulationDestinationItems)] = {nullptr};
        float kRateSourceValuesN11[SizeofStaticArray(gKRateModulationSourceItems)] = {0}; // default-initialize to 0
        bool kRateSourcesFilled[SizeofStaticArray(gKRateModulationSourceItems)] = {
            0}; // default-initialize to false. https://stackoverflow.com/a/1920481/402169
        float kRateDestinationValuesN11[SizeofStaticArray(gKRateModulationDestinationItems)] = {
            0}; // default-initialize to 0
        bool kRateDestinationsFilled[SizeofStaticArray(gKRateModulationDestinationItems)] = {
            0}; // default-initialize to false. https://stackoverflow.com/a/1920481/402169
    };

    audio_block_t *EnsureARateDest(Buffers &buffers, bool &isFirstAcquire, const ModulationDestinationInfo &dest)
    {
        audio_block_t *destBuffer = buffers.aRateDestinations[dest.mIndexForRate];
        isFirstAcquire = !destBuffer;
        if (isFirstAcquire)
        {
            destBuffer = allocate();
            if (!destBuffer)
                return nullptr; // no memory? just act like this iteration didn't happen; we should proceed gracefully
                                // as if this mapping doesn't exist.
            buffers.aRateDestinations[dest.mIndexForRate] = destBuffer;
            // the first time the destination is allocated, it should be INITIALIZED.
            // audioBufferCopyAndApplyGain(source->data, dest->data, modulation.mScaleN11);
        }
        return destBuffer;
        // the subsequent times the same destination is used, it should be ADDED.
        // audioBufferMixInPlaceWithGain(dest->data, source->data, gainToSignedMultiply32x16(modulation.mScaleN11));
    }

    inline float MapSourceN11ToDestN11Value(float srcN11,
                                            const SynthModulationSpec &modSpec,
                                            const ModulationSourceInfo &srcInfo,
                                            const ModulationDestinationInfo &destInfo)
    {
        return srcN11 * modSpec.mScaleN11;
        // remap N11 source value from src range to dest output range
        // note that requires that poles are either 0 (unipolar, like filter cutoff hz) or balanced (-1 to +1 or -2 to
        // +2). otherwise if the poles are not equal, the 0 point will not be centered throughout the mapping
        // return RemapToRange(srcN11,
        //                     modSpec.mSrcRangeMinN11,
        //                     modSpec.mSrcRangeMaxN11,
        //                     modSpec.mDestRangeMinN11,
        //                     modSpec.mDestRangeMaxN11);
        // TODO: Optimize, use curves, whatever.
    }

    // inline int16_t MapSourceN11ToDestN11Value(int16_t src,
    //                                           const SynthModulationSpec &modSpec,
    //                                           const ModulationSourceInfo &srcInfo,
    //                                           const ModulationDestinationInfo &destInfo)
    // {
    //     // todo: optimize by doing this all in integer math
    //     return MapSourceN11ToDestN11Value(fast::Sample16To32(src), modSpec, srcInfo, destInfo);
    // }

    float ApplyFloatValueToFloatValue(float val, float valDest, SynthModulationSpec &modulation)
    {
        // TODO: define blending operators (multiply, add)
        return val + valDest;
    }

    // for k-rate to a-rate
    void ApplyFloatValueToBuffer(float val, audio_block_t *dest, SynthModulationSpec &modulation)
    {
        int16_t v16 = fast::Sample32To16(val);
        arm_offset_q15(dest->data, v16, dest->data, AUDIO_BLOCK_SAMPLES);
    }

    // a-rate to a-rate calculation.
    // here we have not yet mapped the values through modulation mapping.
    void ApplyMappingToBufferAndAssign(audio_block_t *src, audio_block_t *dest, SynthModulationSpec &modulation)
    {
        arm_scale_q15(src->data, fast::Sample32To16(modulation.mScaleN11), 0, dest->data, AUDIO_BLOCK_SAMPLES);
    }

    // // used for a-rate to a-rate calculation
    void ApplyMappingToBufferAndApply(audio_block_t *src, audio_block_t *dest, SynthModulationSpec &modulation)
    {
        int16_t t[AUDIO_BLOCK_SAMPLES];
        arm_scale_q15(src->data, fast::Sample32To16(modulation.mScaleN11), 0, t, AUDIO_BLOCK_SAMPLES);
        arm_add_q15(dest->data, t, dest->data, AUDIO_BLOCK_SAMPLES);
    }

    float ARateBufferToKRateValue(audio_block_t *b)
    {
        // tempting to think about other ways to do this, but this is really the best because:
        // - this value is guaranteed to actually be in the buffer (for example if you do some kind of average or
        // interpolation, a square wave would turn into silence) now whether it's best to take the first, middle, last
        // sample of the buffer, i don't think it really matters tbh.
        return fast::Sample16To32(b->data[0]);
    }

    void ApplyFloatValueToKRateDest(float val,
                                    Buffers &buffers,
                                    size_t destIndexForRate,
                                    SynthModulationSpec &modulation)
    {
        // k-rate to k-rate; easy.
        if (!buffers.kRateDestinationsFilled[destIndexForRate])
        {
            // 1st time setting dest.
            buffers.kRateDestinationsFilled[destIndexForRate] = true;
            buffers.kRateDestinationValuesN11[destIndexForRate] = val;
        }
        else
        {
            // apply to existing.
            buffers.kRateDestinationValuesN11[destIndexForRate] =
                ApplyFloatValueToFloatValue(val, buffers.kRateDestinationValuesN11[destIndexForRate], modulation);
        }
    }

    void ProcessModulation(Buffers &buffers, SynthModulationSpec &modulation)
    {
        auto sourceInfo = GetModulationSourceInfo(modulation.mSource);
        if (!sourceInfo.mIsValidModulation) return;
        auto destInfo = GetModulationDestinationInfo(modulation.mDest);
        if (!destInfo.mIsValidModulation) return;

        if (sourceInfo.mRate == ModulationRate::KRate)
        {
            // ensure we have a source
            float val;
            if (buffers.kRateSourcesFilled[sourceInfo.mIndexForRate])
            {
                val = buffers.kRateSourceValuesN11[sourceInfo.mIndexForRate];
            }
            else
            {
                buffers.kRateSourcesFilled[sourceInfo.mIndexForRate] = true;
                val = mpkRateProvider->IModulationProvider_GetKRateModulationSourceValueN11(sourceInfo.mKRateEnumVal);
                buffers.kRateSourceValuesN11[sourceInfo.mIndexForRate] = val;
            }

            val = MapSourceN11ToDestN11Value(val, modulation, sourceInfo, destInfo); // transform val.

            // we have a source value; apply to destination
            if (destInfo.mRate == ModulationRate::KRate)
            {
                // k-rate to k-rate; easy.
                ApplyFloatValueToKRateDest(val, buffers, destInfo.mIndexForRate, modulation);
                return;
            }

            // k-rate source to a-rate dest
            bool isFirstAquire = true;
            auto *dest = EnsureARateDest(buffers, isFirstAquire, destInfo);
            if (!dest)
                return; // act like this modulation just doesn't exist.
            if (isFirstAquire)
            {
                int16_t s16 = fast::Sample32To16(val);
                fast::FillBufferWithConstant(s16, dest->data);
                return;
            }

            ApplyFloatValueToBuffer(val, dest, modulation);
            return;
        }

        // ensure we have the A-Rate source buffer
        audio_block_t *source = buffers.aRateSsources[sourceInfo.mIndexForRate];
        float sourceAsKRate;
        if (source)
        {
            sourceAsKRate = buffers.aRateSourceAsKRateN11[sourceInfo.mIndexForRate];
        }
        else
        {
            source = receiveReadOnly(sourceInfo.mIndexForRate);
            if (!source)
                return; // act like this modulation just doesn't exist.
            buffers.aRateSsources[sourceInfo.mIndexForRate] = source;
            sourceAsKRate = ARateBufferToKRateValue(source);
            buffers.aRateSourceAsKRateN11[sourceInfo.mIndexForRate] = sourceAsKRate;
        }

        // we have a source buffer; modulate.
        if (destInfo.mRate == ModulationRate::KRate)
        {
            // a-rate source to k-rate dest. treat the a-rate like k-rate, and do like above with k-rate source to
            // k-rate dest.
            float mappedVal = MapSourceN11ToDestN11Value(sourceAsKRate, modulation, sourceInfo, destInfo);
            ApplyFloatValueToKRateDest(mappedVal, buffers, destInfo.mIndexForRate, modulation);
            return;
        }
        // a-rate source to a-rate dest.
        bool isFirstAquire = true;
        auto *dest = EnsureARateDest(buffers, isFirstAquire, destInfo);
        if (!dest)
            return; // act like this modulation just doesn't exist.
        if (isFirstAquire)
        {
            ApplyMappingToBufferAndAssign(source, dest, modulation);
            return;
        }
        ApplyMappingToBufferAndApply(source, dest, modulation);
        return;
    };

    void ProcessFMModulation(Buffers &buffers,
                             SynthOscillatorSettings &osc,
                             AnyModulationSource src,
                             AnyModulationDestination dest)
    {
        SynthModulationSpec m;
        m.mSource = src;
        m.mDest = dest;
        m.SetScaleN11_Legacy(osc.mGain);
        ProcessModulation(buffers, m);
    }

    virtual void update() override
    {
        if (!mSynthPatch)
            return;

        Buffers buffers;

        for (auto &modulation : mSynthPatch->mModulations)
        {
            ProcessModulation(buffers, modulation);
        }

        // process FM algo modulations. The selected FM algorithm will make certain fixed FB->phase connections in
        // addition to user-specified modulations.
        switch (mSynthPatch->mFMAlgo)
        {
        default:
        case FMAlgo::c1c2c3_NoFM: // [1][2][3]
            // nop. no modulations necessary.
            break;
        case FMAlgo::c1m2m3_Chain: // [1<2<3]
            ProcessFMModulation(
                buffers, mSynthPatch->mOsc[2], AnyModulationSource::Osc3FB, AnyModulationDestination::Osc2Phase);
            ProcessFMModulation(
                buffers, mSynthPatch->mOsc[1], AnyModulationSource::Osc2FB, AnyModulationDestination::Osc1Phase);
            break;
        case FMAlgo::c1m2c3_FM12_NoFM3: // [1<2][3]
            ProcessFMModulation(
                buffers, mSynthPatch->mOsc[1], AnyModulationSource::Osc2FB, AnyModulationDestination::Osc1Phase);
            break;
        case FMAlgo::m1c2c3_FM21_NoFM3: // [1>2][3]
            ProcessFMModulation(
                buffers, mSynthPatch->mOsc[0], AnyModulationSource::Osc1FB, AnyModulationDestination::Osc2Phase);
            break;
        case FMAlgo::c1c2m3_FM32_NoFM1: // [1][2<3]
            ProcessFMModulation(
                buffers, mSynthPatch->mOsc[2], AnyModulationSource::Osc3FB, AnyModulationDestination::Osc2Phase);
            break;
        case FMAlgo::c1m2c3_FM23_NoFM1: // [1][2>3]
            ProcessFMModulation(
                buffers, mSynthPatch->mOsc[1], AnyModulationSource::Osc2FB, AnyModulationDestination::Osc3Phase);
            break;
        case FMAlgo::c2m2c3_FM13_Split2: // [1<2][2>3]
            ProcessFMModulation(
                buffers, mSynthPatch->mOsc[1], AnyModulationSource::Osc2FB, AnyModulationDestination::Osc1Phase);
            ProcessFMModulation(
                buffers, mSynthPatch->mOsc[1], AnyModulationSource::Osc2FB, AnyModulationDestination::Osc3Phase);
            break;
        case FMAlgo::c1m23: // [1<(2&3)]
            ProcessFMModulation(
                buffers, mSynthPatch->mOsc[2], AnyModulationSource::Osc3FB, AnyModulationDestination::Osc1Phase);
            ProcessFMModulation(
                buffers, mSynthPatch->mOsc[1], AnyModulationSource::Osc2FB, AnyModulationDestination::Osc1Phase);
            break;
        }

        for (auto *src : buffers.aRateSsources)
        {
            if (!src)
                continue;
            release(src);
        }

        for (size_t i = 0; i < SizeofStaticArray(buffers.kRateDestinationValuesN11); ++i)
        {
            if (!buffers.kRateDestinationsFilled[i])
                continue;
            mpkRateProvider->IModulationProvider_SetKRateModulationDestinationValueN11(
                (KRateModulationDestination)i, buffers.kRateDestinationValuesN11[i]);
        }

        for (size_t i = 0; i < SizeofStaticArray(buffers.aRateDestinations); ++i)
        {
            if (!buffers.aRateDestinations[i])
                continue;
            transmit(buffers.aRateDestinations[i], i);
            release(buffers.aRateDestinations[i]);
        }
    }
};

} // namespace clarinoid
