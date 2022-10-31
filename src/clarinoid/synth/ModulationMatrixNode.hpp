#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <utility>
#include "AudioBufferUtils.hpp"
#include "ModulationInfo.hpp"
#include "Patch.hpp"

namespace clarinoid
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// handles modulation routing (which modulation sources apply to which destinations) for VOICES.
// inputs are modulation sources,
struct VoiceModulationMatrixNode : public AudioStream
{
    // specifies params to use in x = x * mul + add
    struct PolarityConversionKernel15p16
    {
        static constexpr int32_t To15p16(int32_t num, int32_t den)
        {
            return (int32_t(num) << 16) / den;
        };

        static constexpr int16_t AddToSample16(int32_t num, int32_t den)
        {
            return int16_t((int32_t(num) * 32767) / den); // or 32768 saturated?
        };

        const int32_t mul; // is 15.16
        const int16_t add; // is NOT 15.16. just an integer.

        PolarityConversionKernel15p16(int mulNum, int mulDen, int addNum, int addDen)
            : mul(To15p16(mulNum, mulDen)), add(AddToSample16(addNum, addDen))
        {
        }

        // not actually used in clarinoid; it's for testing.
        int16_t Transfer(int16_t x) const
        {
            return ((int32_t(x) * mul) >> 16) + add;
        }
    };

    // specifies params to use in x = x * mul + add
    struct PolarityConversionKernelFloat
    {
        const float mul; // is 15.16
        const float add; // is NOT 15.16. just an integer.
        PolarityConversionKernelFloat(int mulNum, int mulDen, int addNum, int addDen)
            : mul(float(mulNum) / mulDen), add(float(addNum) / addDen)
        {
        }
        float Transfer(float x) const
        {
            return x * mul + add;
        }
    };

    template <typename T>
    static T GetPolarityConversion(ModulationPoleType inpPolarity, ModulationPolarityTreatment outpPolarity)
    {
        if (inpPolarity == clarinoid::ModulationPoleType::Positive01 &&
            outpPolarity == clarinoid::ModulationPolarityTreatment::AsBipolar)
        {
            // return (x - .5) * 2; // or x*2-1
            // return {To15p16(2, 1), To15p16(-1, 1)};
            return {2, 1, -1, 1};
        }
        else if (inpPolarity == clarinoid::ModulationPoleType::N11 &&
                 outpPolarity == clarinoid::ModulationPolarityTreatment::AsPositive01)
        {
            // return x * .5 + .5;
            return {1, 2, 1, 2};
        }
        else if (inpPolarity == clarinoid::ModulationPoleType::Positive01 &&
                 outpPolarity == clarinoid::ModulationPolarityTreatment::AsPositive01Inverted)
        {
            // return 1 - x; // or x*-1+1
            return {-1, 1, 1, 1};
        }
        else if (inpPolarity == clarinoid::ModulationPoleType::N11 &&
                 outpPolarity == clarinoid::ModulationPolarityTreatment::AsPositive01Inverted)
        {
            // return .5 - x * .5; // or x*-.5+.5
            return {-1, 2, 1, 2};
        }
        else if (inpPolarity == clarinoid::ModulationPoleType::Positive01 &&
                 outpPolarity == clarinoid::ModulationPolarityTreatment::AsBipolarInverted)
        {
            // return .5 - x * 2; // or x*-2+1
            return {-2, 1, 1, 1};
        }
        else if (inpPolarity == clarinoid::ModulationPoleType::N11 &&
                 outpPolarity == clarinoid::ModulationPolarityTreatment::AsBipolarInverted)
        {
            // return -x; // x*-1+0
            return {-1, 1, 0, 1};
        }
        // return x; // x*1+0
        return {1, 1, 0, 1};
    }

    audio_block_t *inputQueueArray[gARateModulationSourceCount];

    SynthPreset *mSynthPatch = nullptr;
    IModulationProvider *mpkRateProvider;

    VoiceModulationMatrixNode() : AudioStream(gARateModulationSourceCount, inputQueueArray)
    {
    }

    void SetSynthPatch(SynthPreset *patch, IModulationProvider *pkRateProvider)
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

    // since k-rate are not in the audio graph, just hold their values here. synthvoice can get the values from here.
    float mKRateDestinationValuesN11[SizeofStaticArray(gKRateModulationDestinationItems)] = {0};

    float GetKRateDestinationValue(KRateModulationDestination dest) const
    {
        return mKRateDestinationValuesN11[(int)dest];
    }

    void ResetKRateModulations()
    {
        for (float &f : mKRateDestinationValuesN11)
        {
            f = 0;
        }
    }
    float EnsureKRateSource(Buffers &buffers, const ModulationSourceInfo &sourceInfo)
    {
        // ensure we have a source
        if (buffers.kRateSourcesFilled[sourceInfo.mIndexForRate])
        {
            return buffers.kRateSourceValuesN11[sourceInfo.mIndexForRate];
        }

        buffers.kRateSourcesFilled[sourceInfo.mIndexForRate] = true;
        float ret = mpkRateProvider->IModulationProvider_GetKRateModulationSourceValueN11(sourceInfo.mKRateEnumVal);
        buffers.kRateSourceValuesN11[sourceInfo.mIndexForRate] = ret;
        return ret;
    }

    audio_block_t *EnsureARateSource(Buffers &buffers, const ModulationSourceInfo &sourceInfo, float &valAsKRate)
    {
        audio_block_t *source = buffers.aRateSsources[sourceInfo.mIndexForRate];
        if (source)
        {
            valAsKRate = buffers.aRateSourceAsKRateN11[sourceInfo.mIndexForRate];
        }
        else
        {
            source = receiveReadOnly(sourceInfo.mIndexForRate);
            if (!source)
                return nullptr; // act like this modulation just doesn't exist.
            buffers.aRateSsources[sourceInfo.mIndexForRate] = source;

            // tempting to think about other ways to do this, but this is really the best because:
            // - this value is guaranteed to actually be in the buffer (for example if you do some kind of average or
            // interpolation, a square wave would turn into silence) now whether it's best to take the first, middle,
            // last sample of the buffer, i don't think it really matters tbh.
            valAsKRate = fast::Sample16To32(source->data[0]);

            buffers.aRateSourceAsKRateN11[sourceInfo.mIndexForRate] = valAsKRate;
        }
        return source;
    }

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

    std::pair<bool, float> GetKRateSourceValueFromAnySource(Buffers &buffers, const ModulationSourceInfo &sourceInfo)
    {
        if (sourceInfo.mRate == ModulationRate::KRate)
        {
            return std::make_pair(true, EnsureKRateSource(buffers, sourceInfo));
        }
        // A-Rate source
        float sourceAsKRate;
        audio_block_t *source =
            EnsureARateSource(buffers, sourceInfo, sourceAsKRate); // buffers.aRateSsources[sourceInfo.mIndexForRate];
        if (!source)
            return std::make_pair(false, 0.0f);
        return std::make_pair(true, sourceAsKRate);
    }

    // converts a single krate source value to a destination +/- value to apply
    inline float MapKRateValue(float kRateSourceValue,
                               Buffers &buffers,
                               const SynthModulationSpec &modSpec,
                               const ModulationSourceInfo &sourceInfo,
                               const ModulationDestinationInfo &destInfo)
    {
        auto polarityMapping =
            GetPolarityConversion<PolarityConversionKernelFloat>(sourceInfo.mPoleType, modSpec.mSourcePolarity);
        float ret = polarityMapping.Transfer(kRateSourceValue);
        auto curveState = gModCurveLUT.BeginLookupI(modSpec.mCurveShape);
        ret = gModCurveLUT.Transfer32(ret, curveState);
        ret *= modSpec.mScaleN11;

        const auto &auxInfo = GetModulationSourceInfo(modSpec.mAuxSource);
        if (auxInfo.mIsValidModulation && modSpec.mAuxEnabled && modSpec.mAuxAmount)
        {
            // we need to apply aux source.
            auto optAuxVal = GetKRateSourceValueFromAnySource(buffers, auxInfo);
            if (optAuxVal.first)
            {
                auto auxPolarityMapping =
                    GetPolarityConversion<PolarityConversionKernelFloat>(auxInfo.mPoleType, modSpec.mAuxPolarity);
                float auxVal = auxPolarityMapping.Transfer(optAuxVal.second);
                auto auxCurveState = gModCurveLUT.BeginLookupI(modSpec.mAuxCurveShape);
                auxVal = gModCurveLUT.Transfer32(auxVal, auxCurveState);

                float auxBase = 1.0f - modSpec.mAuxAmount; // can be precalculated
                float auxMul = auxBase + auxVal * modSpec.mAuxAmount;

                ret *= auxMul;
            }
        }
        return ret;
    }

    void MapARateToARateAndApply_NoAux(audio_block_t *src,
                                       audio_block_t *dest,
                                       SynthModulationSpec &modSpec,
                                       const ModulationSourceInfo &sourceInfo,
                                       const ModulationDestinationInfo &destInfo)
    {
        auto srcCurveState = gModCurveLUT.BeginLookupI(modSpec.mCurveShape);
        int32_t sourceScale16p16 = int32_t(modSpec.mScaleN11 * 65536);
        auto srcPolarityConv =
            GetPolarityConversion<PolarityConversionKernel15p16>(sourceInfo.mPoleType, modSpec.mSourcePolarity);

        uint32_t *bufSrc32 = (uint32_t *)(src->data);
        uint32_t *bufDest32 = (uint32_t *)(dest->data);

        for (size_t i32 = 0; i32 < AUDIO_BLOCK_SAMPLES / 2; ++i32)
        {
            uint32_t x32 = bufSrc32[i32]; // process 2 16-bit samples per loop to take advantage of 32-bit processing

            // this ordering is much faster than processing all x1 first then x2
            int32_t x1 = signed_multiply_32x16b(srcPolarityConv.mul, x32); // unpacks & multiplies
            int32_t x2 = signed_multiply_32x16t(srcPolarityConv.mul, x32);
            x1 = x1 + srcPolarityConv.add;
            x2 = x2 + srcPolarityConv.add;
            x1 = gModCurveLUT.Transfer16(saturate16(x1), srcCurveState);
            x2 = gModCurveLUT.Transfer16(saturate16(x2), srcCurveState);
            x1 = signed_multiply_32x16b(sourceScale16p16, x1);
            x2 = signed_multiply_32x16b(sourceScale16p16, x2);
            x32 = pack_16b_16b(x1, x2);
            bufDest32[i32] = signed_add_16_and_16(bufDest32[i32], x32);
        }
    }

    void MapARateToARateAndApply_KRateAux(Buffers &buffers,
                                          audio_block_t *src,
                                          audio_block_t *dest,
                                          SynthModulationSpec &modSpec,
                                          const ModulationSourceInfo &sourceInfo,
                                          const ModulationDestinationInfo &destInfo,
                                          const ModulationSourceInfo &auxInfo)
    {
        auto srcCurveState = gModCurveLUT.BeginLookupI(modSpec.mCurveShape);
        int32_t sourceScale16p16 = int32_t(modSpec.mScaleN11 * 65536);

        auto srcPolarityConv =
            GetPolarityConversion<PolarityConversionKernel15p16>(sourceInfo.mPoleType, modSpec.mSourcePolarity);

        uint32_t *bufSrc32 = (uint32_t *)(src->data);
        uint32_t *bufDest32 = (uint32_t *)(dest->data);

        float auxVal = EnsureKRateSource(buffers, auxInfo);
        float auxBase = 1.0f - modSpec.mAuxAmount; // can be precalculated
        auto auxCurveState = gModCurveLUT.BeginLookupI(modSpec.mAuxCurveShape);
        auto auxPolarityConv =
            GetPolarityConversion<PolarityConversionKernelFloat>(auxInfo.mPoleType, modSpec.mAuxPolarity);

        auxVal = auxPolarityConv.Transfer(auxVal);
        auxVal = gModCurveLUT.Transfer32(auxVal, auxCurveState);
        auxVal = auxBase + auxVal * modSpec.mAuxAmount;
        int32_t auxVal16 = int32_t(auxVal * 65536);

        for (size_t i32 = 0; i32 < AUDIO_BLOCK_SAMPLES / 2; ++i32)
        {
            uint32_t x32 = bufSrc32[i32]; // process 2 16-bit samples per loop to take advantage of 32-bit processing

            // this ordering is much faster than processing all x1 first then x2
            int32_t x1 = signed_multiply_32x16b(srcPolarityConv.mul, x32); // unpacks & multiplies
            int32_t x2 = signed_multiply_32x16t(srcPolarityConv.mul, x32);
            x1 = x1 + srcPolarityConv.add;
            x2 = x2 + srcPolarityConv.add;
            x1 = gModCurveLUT.Transfer16(saturate16(x1), srcCurveState);
            x2 = gModCurveLUT.Transfer16(saturate16(x2), srcCurveState);
            x1 = signed_multiply_32x16b(sourceScale16p16, x1);
            x2 = signed_multiply_32x16b(sourceScale16p16, x2);

            // aux
            x1 = signed_multiply_32x16b(auxVal16, x1);
            x2 = signed_multiply_32x16b(auxVal16, x2);

            x32 = pack_16b_16b(x1, x2);
            bufDest32[i32] = signed_add_16_and_16(bufDest32[i32], x32);
        }
    }

    // example: lfo to pulse width, modulated by envelope
    void MapARateToARateAndApply_ARateAux(Buffers &buffers,
                                          audio_block_t *src,
                                          audio_block_t *dest,
                                          SynthModulationSpec &modSpec,
                                          const ModulationSourceInfo &sourceInfo,
                                          const ModulationDestinationInfo &destInfo,
                                          const ModulationSourceInfo &auxInfo)
    {
        float auxValAsKRate_Unused;
        auto *auxBuf = EnsureARateSource(buffers, auxInfo, auxValAsKRate_Unused);
        if (!auxBuf)
            return;

        float auxBase = 1.0f - modSpec.mAuxAmount; // can be precalculated
        uint32_t auxBase32 = (uint32_t)(auxBase * 65536);

        auto srcCurveState = gModCurveLUT.BeginLookupI(modSpec.mCurveShape);
        auto auxCurveState = gModCurveLUT.BeginLookupI(modSpec.mAuxCurveShape);
        int32_t sourceScale16p16 = int32_t(modSpec.mScaleN11 * 65536);
        int32_t auxScale16p16 = int32_t(modSpec.mAuxAmount * 65536);

        auto srcPolarityConv =
            GetPolarityConversion<PolarityConversionKernel15p16>(sourceInfo.mPoleType, modSpec.mSourcePolarity);
        auto auxPolarityConv =
            GetPolarityConversion<PolarityConversionKernel15p16>(auxInfo.mPoleType, modSpec.mAuxPolarity);

        uint32_t *bufSrc32 = (uint32_t *)(src->data);
        uint32_t *bufAux32 = (uint32_t *)(auxBuf->data);
        uint32_t *bufDest32 = (uint32_t *)(dest->data);

        for (size_t i32 = 0; i32 < AUDIO_BLOCK_SAMPLES / 2; ++i32)
        {
            uint32_t x32 = bufSrc32[i32]; // process 2 16-bit samples per loop to take advantage of 32-bit processing

            // this ordering is much faster than processing all x1 first then x2
            int32_t x1 = signed_multiply_32x16b(srcPolarityConv.mul, x32); // unpacks & multiplies
            int32_t x2 = signed_multiply_32x16t(srcPolarityConv.mul, x32);
            x1 = x1 + srcPolarityConv.add;
            x2 = x2 + srcPolarityConv.add;
            x1 = gModCurveLUT.Transfer16(saturate16(x1), srcCurveState);
            x2 = gModCurveLUT.Transfer16(saturate16(x2), srcCurveState);
            x1 = signed_multiply_32x16b(sourceScale16p16, x1);
            x2 = signed_multiply_32x16b(sourceScale16p16, x2);

            int32_t aux32 = bufAux32[i32];
            int32_t aux1 = signed_multiply_32x16b(auxPolarityConv.mul, aux32);
            int32_t aux2 = signed_multiply_32x16t(auxPolarityConv.mul, aux32);
            aux1 += auxPolarityConv.add;
            aux2 += auxPolarityConv.add;
            aux1 = gModCurveLUT.Transfer16(saturate16(aux1), auxCurveState);
            aux2 = gModCurveLUT.Transfer16(saturate16(aux2), auxCurveState);
            // aux1 = signed_multiply_32x16b(auxScale16p16, aux1);
            //  aux2 = signed_multiply_32x16b(auxScale16p16, aux2);
            aux1 = signed_multiply_accumulate_32x16b(auxBase32, auxScale16p16, aux1);
            aux2 = signed_multiply_accumulate_32x16b(auxBase32, auxScale16p16, aux2);
            x1 = signed_multiply_32x16b(aux1 << 1,
                                        x1); // <<1 because it's a sample value (15bits), but this wants 16bits.
            x2 = signed_multiply_32x16b(aux2 << 1, x2);

            x32 = pack_16b_16b(x1, x2);
            bufDest32[i32] = signed_add_16_and_16(bufDest32[i32], x32);
        }
    }

    // a-rate to a-rate calculation. this is done using only integer math.
    // three variations:
    // - no aux
    // - k-rate aux
    // - a-rate aux
    void MapARateToARateAndApply(Buffers &buffers,
                                 audio_block_t *src,
                                 audio_block_t *dest,
                                 SynthModulationSpec &modSpec,
                                 const ModulationSourceInfo &sourceInfo,
                                 const ModulationDestinationInfo &destInfo,
                                 const ModulationSourceInfo &auxInfo)
    {
        bool auxEnabled = (auxInfo.mIsValidModulation && modSpec.mAuxEnabled && modSpec.mAuxAmount);
        if (!auxEnabled)
        {
            MapARateToARateAndApply_NoAux(src, dest, modSpec, sourceInfo, destInfo);
            return;
        }
        if (auxInfo.mRate == ModulationRate::KRate)
        {
            MapARateToARateAndApply_KRateAux(buffers, src, dest, modSpec, sourceInfo, destInfo, auxInfo);
            return;
        }
        MapARateToARateAndApply_ARateAux(buffers, src, dest, modSpec, sourceInfo, destInfo, auxInfo);
    }

    float ApplyFloatValueToFloatValue(float val, float valDest)
    {
        return val + valDest;
    }

    // for k-rate to a-rate
    void ApplyKRateValueToARateDest(float val, audio_block_t *dest, SynthModulationSpec &modulation)
    {
        int16_t v16 = fast::Sample32To16(val);
        arm_offset_q15(dest->data, v16, dest->data, AUDIO_BLOCK_SAMPLES);
    }

    // "Map" means converting
    // "Apply" means applying a mapped value to the output buffer (assigning or combining modulations to the same dest)
    // "Assign" applies for the first time

    void ApplyKRateValueToKRateDest(float val, Buffers &buffers, size_t destIndexForRate)
    {
        // k-rate to k-rate; easy.
        if (!buffers.kRateDestinationsFilled[destIndexForRate])
        {
            // 1st time setting dest.
            buffers.kRateDestinationsFilled[destIndexForRate] = true;
            buffers.kRateDestinationValuesN11[destIndexForRate] = val;
            return;
        }

        // apply to existing.
        buffers.kRateDestinationValuesN11[destIndexForRate] =
            ApplyFloatValueToFloatValue(val, buffers.kRateDestinationValuesN11[destIndexForRate]);
    }

    void ProcessKRateToKRate(Buffers &buffers,
                             SynthModulationSpec &modulation,
                             const ModulationSourceInfo &sourceInfo,
                             const ModulationDestinationInfo &destInfo)
    {
        float val = EnsureKRateSource(buffers, sourceInfo);
        val = MapKRateValue(val, buffers, modulation, sourceInfo, destInfo);
        ApplyKRateValueToKRateDest(val, buffers, destInfo.mIndexForRate);
    }

    void ProcessKRateToARate(Buffers &buffers,
                             SynthModulationSpec &modulation,
                             const ModulationSourceInfo &sourceInfo,
                             const ModulationDestinationInfo &destInfo)
    {
        float val = EnsureKRateSource(buffers, sourceInfo);
        val = MapKRateValue(val, buffers, modulation, sourceInfo, destInfo);

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

        ApplyKRateValueToARateDest(val, dest, modulation);
    }

    void ProcessARateToKRate(Buffers &buffers,
                             SynthModulationSpec &modulation,
                             const ModulationSourceInfo &sourceInfo,
                             const ModulationDestinationInfo &destInfo)
    {
        float sourceAsKRate;
        audio_block_t *source =
            EnsureARateSource(buffers, sourceInfo, sourceAsKRate); // buffers.aRateSsources[sourceInfo.mIndexForRate];
        if (!source)
            return;
        float mappedVal = MapKRateValue(sourceAsKRate, buffers, modulation, sourceInfo, destInfo);
        // Serial.println(String("arate mod value: ") + sourceAsKRate + " -> " + mappedVal);
        ApplyKRateValueToKRateDest(mappedVal, buffers, destInfo.mIndexForRate);
    }

    void ProcessARateToARate(Buffers &buffers,
                             SynthModulationSpec &modulation,
                             const ModulationSourceInfo &sourceInfo,
                             const ModulationDestinationInfo &destInfo)
    {
        float sourceAsKRate;
        audio_block_t *source =
            EnsureARateSource(buffers, sourceInfo, sourceAsKRate); // buffers.aRateSsources[sourceInfo.mIndexForRate];
        if (!source)
            return;

        bool isFirstAquire = true;
        auto *dest = EnsureARateDest(buffers, isFirstAquire, destInfo);
        if (!dest)
            return; // act like this modulation just doesn't exist.
        if (isFirstAquire)
        {
            fast::FillBufferWithConstant(0, dest->data);
        }
        const auto &auxInfo = GetModulationSourceInfo(modulation.mAuxSource);
        MapARateToARateAndApply(buffers, source, dest, modulation, sourceInfo, destInfo, auxInfo);
    }

    void ProcessModulation(Buffers &buffers, SynthModulationSpec &modulation)
    {
        auto sourceInfo = GetModulationSourceInfo(modulation.mSource);
        if (!sourceInfo.mIsValidModulation)
            return;
        auto destInfo = GetModulationDestinationInfo(modulation.mDest);
        if (!destInfo.mIsValidModulation)
            return;

        // depending on the combination of A-Rate & K-rate, take an optimized path.
        if (sourceInfo.mRate == ModulationRate::KRate)
        {
            if (destInfo.mRate == ModulationRate::KRate)
            {
                ProcessKRateToKRate(buffers, modulation, sourceInfo, destInfo);
                return;
            }
            ProcessKRateToARate(buffers, modulation, sourceInfo, destInfo);
            return;
        }

        if (destInfo.mRate == ModulationRate::KRate)
        {
            ProcessARateToKRate(buffers, modulation, sourceInfo, destInfo);
            return;
        }

        ProcessARateToARate(buffers, modulation, sourceInfo, destInfo);
        return;
    };

    void ProcessFMModulation(Buffers &buffers,
                             float amount,
                             SynthOscillatorSettings &osc,
                             AnyModulationSource src,
                             AnyModulationDestination dest)
    {
        SynthModulationSpec m;
        m.mSource = src;
        m.mDest = dest;
        m.mSourcePolarity = ModulationPolarityTreatment::AsBipolar;
        m.SetScaleN11_Legacy(amount);
        ProcessModulation(buffers, m);
    }

    std::array<CCPatch, gARateModulationSourceCount> mARateSourcePatches;
    std::array<CCPatch, gARateModulationDestinationCount> mARateDestinationPatches;

    void Init(IModulationProvider *pmod)
    {
        for (auto &x : gARateModulationSourceItems) // like LFOs, ENV, and osc FB
        {
            auto srcPort = pmod->IModulationProvider_GetARateSourcePort(x.mValue);
            mARateSourcePatches[x.mIntValue].connect(*srcPort.first, srcPort.second, *this, x.mIntValue);
        }
        for (auto &x : gARateModulationDestinationItems)
        {
            auto destPort = pmod->IModulationProvider_GetARateDestinationPort(x.mValue);
            mARateDestinationPatches[x.mIntValue].connect(*this, x.mIntValue, *destPort.first, destPort.second);
        }
    }

    virtual void update() override
    {
        if (!mSynthPatch)
        {
            return;
        }

        Buffers buffers;

        for (auto &modulation : mSynthPatch->mModulations)
        {
            ProcessModulation(buffers, modulation);
        }

        float krateFMStrength2To1 = GetKRateDestinationValue(KRateModulationDestination::FMStrength2To1);
        float krateFMStrength3To1 = GetKRateDestinationValue(KRateModulationDestination::FMStrength3To1);
        float krateFMStrength1To2 = GetKRateDestinationValue(KRateModulationDestination::FMStrength1To2);
        float krateFMStrength3To2 = GetKRateDestinationValue(KRateModulationDestination::FMStrength3To2);
        float krateFMStrength1To3 = GetKRateDestinationValue(KRateModulationDestination::FMStrength1To3);
        float krateFMStrength2To3 = GetKRateDestinationValue(KRateModulationDestination::FMStrength2To3);

        if (!FloatEquals(mSynthPatch->mFMStrength2To1, 0) || !FloatEquals(krateFMStrength2To1, 0))
        {
            ProcessFMModulation(buffers,
                                mSynthPatch->mFMStrength2To1 + krateFMStrength2To1,
                                mSynthPatch->mOsc[1],
                                AnyModulationSource::Osc2FB,
                                AnyModulationDestination::Osc1Phase);
        }
        if (!FloatEquals(mSynthPatch->mFMStrength3To1, 0) || !FloatEquals(krateFMStrength3To1, 0))
        {
            ProcessFMModulation(buffers,
                                mSynthPatch->mFMStrength3To1 + krateFMStrength3To1,
                                mSynthPatch->mOsc[2],
                                AnyModulationSource::Osc3FB,
                                AnyModulationDestination::Osc1Phase);
        }
        if (!FloatEquals(mSynthPatch->mFMStrength1To2, 0) || !FloatEquals(krateFMStrength1To2, 0))
        {
            ProcessFMModulation(buffers,
                                mSynthPatch->mFMStrength1To2 + krateFMStrength1To2,
                                mSynthPatch->mOsc[0],
                                AnyModulationSource::Osc1FB,
                                AnyModulationDestination::Osc2Phase);
        }
        if (!FloatEquals(mSynthPatch->mFMStrength3To2, 0) || !FloatEquals(krateFMStrength3To2, 0))
        {
            ProcessFMModulation(buffers,
                                mSynthPatch->mFMStrength3To2 + krateFMStrength3To2,
                                mSynthPatch->mOsc[2],
                                AnyModulationSource::Osc3FB,
                                AnyModulationDestination::Osc2Phase);
        }
        if (!FloatEquals(mSynthPatch->mFMStrength1To3, 0) || !FloatEquals(krateFMStrength1To3, 0))
        {
            ProcessFMModulation(buffers,
                                mSynthPatch->mFMStrength1To3 + krateFMStrength1To3,
                                mSynthPatch->mOsc[0],
                                AnyModulationSource::Osc1FB,
                                AnyModulationDestination::Osc3Phase);
        }
        if (!FloatEquals(mSynthPatch->mFMStrength2To3, 0) || !FloatEquals(krateFMStrength2To3, 0))
        {
            ProcessFMModulation(buffers,
                                mSynthPatch->mFMStrength2To3 + krateFMStrength2To3,
                                mSynthPatch->mOsc[1],
                                AnyModulationSource::Osc2FB,
                                AnyModulationDestination::Osc3Phase);
        }

        for (auto *src : buffers.aRateSsources)
        {
            if (!src)
                continue;
            release(src);
        }

        // store k-rate dest values for use by synthvoice; this object is the best place to keep the values.
        for (size_t i = 0; i < SizeofStaticArray(buffers.kRateDestinationValuesN11); ++i)
        {
            mKRateDestinationValuesN11[i] = buffers.kRateDestinationValuesN11[i];
        }

        for (size_t i = 0; i < SizeofStaticArray(buffers.aRateDestinations); ++i)
        {
            if (!buffers.aRateDestinations[i])
                continue;
            transmit(buffers.aRateDestinations[i], (uint8_t)i);
            release(buffers.aRateDestinations[i]);
        }
    }
};

} // namespace clarinoid
