#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

struct IModulationKRateProvider
{
    virtual float IModulationProvider_GetKRateModulationSourceValueN11(KRateModulationSource src) = 0;
    virtual void IModulationProvider_SetKRateModulationDestinationValueN11(KRateModulationDestination d, float val) = 0;
};

enum class ModulationRate : uint8_t
{
    ARate,
    KRate,
};

enum class ModulationPoleType : uint8_t
{
    Positive01,
    N11,
};

struct ModulationSourceInfo
{
    const bool mIsValidModulation = false; // for things like the "none" value.
    const size_t mIndexOverall = 0;        // index overall
    const AnyModulationSource mAnyEnumVal = AnyModulationSource::None;
    const size_t mIndexForRate = 0; // index as arate or krate.
    const ModulationRate mRate = ModulationRate::ARate;
    const KRateModulationSource mKRateEnumVal;
    const ARateModulationSource mARateEnumVal;
    const float mRangeMin = -1;
    const float mRangeMax = 1;
    const ModulationPoleType mPoleType = ModulationPoleType::N11;
};

ModulationSourceInfo gModulationSourceInfo[gAnyModulationSourceCount] = {
    {false,
     0, // index overall
     AnyModulationSource::None,
     0, // index for rate
     ModulationRate::ARate,
     (KRateModulationSource)0,
     (ARateModulationSource)0,
     -1, // range min
     1,  // range max
     ModulationPoleType::N11},

    {true,
     (size_t)AnyModulationSource::LFO1, // index overall
     AnyModulationSource::LFO1,
     (size_t)ARateModulationSource::LFO1, // index for rate
     ModulationRate::ARate,
     (KRateModulationSource)0,
     ARateModulationSource::LFO1,
     -1, // range min
     1,  // range max
     ModulationPoleType::N11},

    {true,
     (size_t)AnyModulationSource::LFO2, // index overall
     AnyModulationSource::LFO2,
     (size_t)ARateModulationSource::LFO2, // index for rate
     ModulationRate::ARate,
     (KRateModulationSource)0,
     ARateModulationSource::LFO2,
     -1, // range min
     1,  // range max
     ModulationPoleType::N11},

    {true,
     (size_t)AnyModulationSource::ENV1, // index overall
     AnyModulationSource::ENV1,
     (size_t)ARateModulationSource::ENV1, // index for rate
     ModulationRate::ARate,
     (KRateModulationSource)0,
     ARateModulationSource::ENV1,
     0, // range min
     1, // range max
     ModulationPoleType::Positive01},

    {true,
     (size_t)AnyModulationSource::ENV2, // index overall
     AnyModulationSource::ENV2,
     (size_t)ARateModulationSource::ENV2, // index for rate
     ModulationRate::ARate,
     (KRateModulationSource)0,
     ARateModulationSource::ENV2,
     0, // range min
     1, // range max
     ModulationPoleType::Positive01},

    {true,
     (size_t)AnyModulationSource::Osc1FB, // index overall
     AnyModulationSource::Osc1FB,
     (size_t)ARateModulationSource::Osc1FB, // index for rate
     ModulationRate::ARate,
     (KRateModulationSource)0,
     ARateModulationSource::Osc1FB,
     -1, // range min
     1,  // range max
     ModulationPoleType::N11},

    {true,
     (size_t)AnyModulationSource::Osc2FB, // index overall
     AnyModulationSource::Osc2FB,
     (size_t)ARateModulationSource::Osc2FB, // index for rate
     ModulationRate::ARate,
     (KRateModulationSource)0,
     ARateModulationSource::Osc2FB,
     -1, // range min
     1,  // range max
     ModulationPoleType::N11},

    {true,
     (size_t)AnyModulationSource::Osc3FB, // index overall
     AnyModulationSource::Osc3FB,
     (size_t)ARateModulationSource::Osc3FB, // index for rate
     ModulationRate::ARate,
     (KRateModulationSource)0,
     ARateModulationSource::Osc3FB,
     -1, // range min
     1,  // range max
     ModulationPoleType::N11},

    {true,
     (size_t)AnyModulationSource::Breath, // index overall
     AnyModulationSource::Breath,
     (size_t)KRateModulationSource::Breath, // index for rate
     ModulationRate::KRate,
     KRateModulationSource::Breath,
     (ARateModulationSource)0,
     0, // range min
     1, // range max
     ModulationPoleType::Positive01},

    {true,
     (size_t)AnyModulationSource::PitchStrip, // index overall
     AnyModulationSource::PitchStrip,
     (size_t)KRateModulationSource::PitchStrip, // index for rate
     ModulationRate::KRate,
     KRateModulationSource::PitchStrip,
     (ARateModulationSource)0,
     -1, // range min
     1,  // range max
     ModulationPoleType::N11},
};

ModulationSourceInfo *gKRateModulationSourceInfoPtrs[gKRateModulationSourceCount] = {nullptr};
ModulationSourceInfo *gARateModulationSourceInfoPtrs[gARateModulationSourceCount] = {nullptr};

StaticInit gModSourceInit = {[]() {
    // ensure indices are sequential, match the any enum val
    int prevIndex = -1;
    for (auto& info : gModulationSourceInfo) {
        CCASSERT(info.mIndexOverall == (size_t)info.mAnyEnumVal);
        CCASSERT((int)info.mIndexOverall == (prevIndex + 1));
        prevIndex ++;
    }

    // populate simple lookup tables
    for (ModulationSourceInfo &info : gModulationSourceInfo)
    {
        if (!info.mIsValidModulation)
            continue;
        switch (info.mRate)
        {
        case ModulationRate::ARate:
            CCASSERT(info.mIndexForRate < gARateModulationSourceCount);
            CCASSERT(!gARateModulationSourceInfoPtrs[info.mIndexForRate]);
            gARateModulationSourceInfoPtrs[info.mIndexForRate] = &info;
            CCASSERT(info.mIndexForRate == (size_t)info.mARateEnumVal);
            break;
        case ModulationRate::KRate:
        default:
            CCASSERT(info.mIndexForRate < gKRateModulationSourceCount);
            CCASSERT(!gKRateModulationSourceInfoPtrs[info.mIndexForRate]);
            gKRateModulationSourceInfoPtrs[info.mIndexForRate] = &info;
            CCASSERT(info.mIndexForRate == (size_t)info.mKRateEnumVal);
            break;
        }
    }
    // verify correctness.
    for (auto p : gKRateModulationSourceInfoPtrs)
    {
        CCASSERT(!!p);
    }
    for (auto p : gARateModulationSourceInfoPtrs)
    {
        CCASSERT(!!p);
    }
}};

static const ModulationSourceInfo &GetModulationSourceInfo(AnyModulationSource m)
{
    return gModulationSourceInfo[(size_t)m];
}

struct ModulationDestinationInfo
{
    const bool mIsValidModulation = false; // for things like the "none" value.
    const size_t mIndexOverall = 0;        // index overall
    const AnyModulationDestination mAnyEnumVal = AnyModulationDestination::None;
    const size_t mIndexForRate = 0; // index as arate or krate.
    const ModulationRate mRate = ModulationRate::ARate;
    const KRateModulationDestination mKRateEnumVal;
    const ARateModulationDestination mARateEnumVal;
    const float mRangeMin = 0;
    const float mRangeMax = 0;
    const ModulationPoleType mPoleType = ModulationPoleType::N11;
};

ModulationDestinationInfo gModulationDestinationInfo[gAnyModulationDestinationCount] = {
    {false,                                  // valid
     (size_t)AnyModulationDestination::None, // index overall
     AnyModulationDestination::None,
     0, // index for rate
     ModulationRate::ARate,
     (KRateModulationDestination)0,
     (ARateModulationDestination)0,
     0, // range min
     0, // range max
     ModulationPoleType::N11},

    {true,
     (size_t)AnyModulationDestination::Osc1PulseWidth, // index overall
     AnyModulationDestination::Osc1PulseWidth,
     (size_t)ARateModulationDestination::Osc1PulseWidth, // index for rate
     ModulationRate::ARate,
     (KRateModulationDestination)0,
     ARateModulationDestination::Osc1PulseWidth,
     -1, // range min
     1,  // range max
     ModulationPoleType::N11},

    {true,
     (size_t)AnyModulationDestination::Osc1Phase, // index overall
     AnyModulationDestination::Osc1Phase,
     (size_t)ARateModulationDestination::Osc1Phase, // index for rate
     ModulationRate::ARate,
     (KRateModulationDestination)0,
     ARateModulationDestination::Osc1Phase,
     -1, // range min
     1,  // range max
     ModulationPoleType::N11},

    {true,
     (size_t)AnyModulationDestination::Osc2PulseWidth, // index overall
     AnyModulationDestination::Osc2PulseWidth,
     (size_t)ARateModulationDestination::Osc2PulseWidth, // index for rate
     ModulationRate::ARate,
     (KRateModulationDestination)0,
     ARateModulationDestination::Osc2PulseWidth,
     -1, // range min
     1,  // range max
     ModulationPoleType::N11},

    {true,
     (size_t)AnyModulationDestination::Osc2Phase, // index overall
     AnyModulationDestination::Osc2Phase,
     (size_t)ARateModulationDestination::Osc2Phase, // index for rate
     ModulationRate::ARate,
     (KRateModulationDestination)0,
     ARateModulationDestination::Osc2Phase,
     -1, // range min
     1,  // range max
     ModulationPoleType::N11},

    {true,
     (size_t)AnyModulationDestination::Osc3PulseWidth, // index overall
     AnyModulationDestination::Osc3PulseWidth,
     (size_t)ARateModulationDestination::Osc3PulseWidth, // index for rate
     ModulationRate::ARate,
     (KRateModulationDestination)0,
     ARateModulationDestination::Osc3PulseWidth,
     -1, // range min
     1,  // range max
     ModulationPoleType::N11},

    {true,
     (size_t)AnyModulationDestination::Osc3Phase, // index overall
     AnyModulationDestination::Osc3Phase,
     (size_t)ARateModulationDestination::Osc3Phase, // index for rate
     ModulationRate::ARate,
     (KRateModulationDestination)0,
     ARateModulationDestination::Osc3Phase,
     -1, // range min
     1,  // range max
     ModulationPoleType::N11},

    {true,
     (size_t)AnyModulationDestination::VoiceFilterCutoff, // index overall
     AnyModulationDestination::VoiceFilterCutoff,
     (size_t)KRateModulationDestination::VoiceFilterCutoff, // index for rate
     ModulationRate::KRate,
     KRateModulationDestination::VoiceFilterCutoff,
     (ARateModulationDestination)0,
     0,     // range min
     22050, // range max
     ModulationPoleType::Positive01},

    {true,
     (size_t)AnyModulationDestination::Osc1Frequency, // index overall
     AnyModulationDestination::Osc1Frequency,
     (size_t)KRateModulationDestination::Osc1Frequency, // index for rate
     ModulationRate::KRate,
     KRateModulationDestination::Osc1Frequency,
     (ARateModulationDestination)0,
     -1, // range min
     1,  // range max
     ModulationPoleType::N11},

    {true,
     (size_t)AnyModulationDestination::Osc1Amplitude, // index overall
     AnyModulationDestination::Osc1Amplitude,
     (size_t)KRateModulationDestination::Osc1Amplitude, // index for rate
     ModulationRate::KRate,
     KRateModulationDestination::Osc1Amplitude,
     (ARateModulationDestination)0,
     -1, // range min
     1,  // range max
     ModulationPoleType::N11},

    {true,
     (size_t)AnyModulationDestination::Osc2Frequency, // index overall
     AnyModulationDestination::Osc2Frequency,
     (size_t)KRateModulationDestination::Osc2Frequency, // index for rate
     ModulationRate::KRate,
     KRateModulationDestination::Osc2Frequency,
     (ARateModulationDestination)0,
     -1, // range min
     1,  // range max
     ModulationPoleType::N11},

    {true,
     (size_t)AnyModulationDestination::Osc2Amplitude, // index overall
     AnyModulationDestination::Osc2Amplitude,
     (size_t)KRateModulationDestination::Osc2Amplitude, // index for rate
     ModulationRate::KRate,
     KRateModulationDestination::Osc2Amplitude,
     (ARateModulationDestination)0,
     -1, // range min
     1,  // range max
     ModulationPoleType::N11},

    {true,
     (size_t)AnyModulationDestination::Osc3Frequency, // index overall
     AnyModulationDestination::Osc3Frequency,
     (size_t)KRateModulationDestination::Osc3Frequency, // index for rate
     ModulationRate::KRate,
     KRateModulationDestination::Osc3Frequency,
     (ARateModulationDestination)0,
     -1, // range min
     1,  // range max
     ModulationPoleType::N11},

    {true,
     (size_t)AnyModulationDestination::Osc3Amplitude, // index overall
     AnyModulationDestination::Osc3Amplitude,
     (size_t)KRateModulationDestination::Osc3Amplitude, // index for rate
     ModulationRate::KRate,
     KRateModulationDestination::Osc3Amplitude,
     (ARateModulationDestination)0,
     -1, // range min
     1,  // range max
     ModulationPoleType::N11},
};

ModulationDestinationInfo *gKRateModulationDestinationInfoPtrs[gKRateModulationDestinationCount] = {nullptr};
ModulationDestinationInfo *gARateModulationDestinationInfoPtrs[gARateModulationDestinationCount] = {nullptr};

StaticInit gModDestinationInit = {[]() {
    // ensure indices are sequential, match the any enum val
    int prevIndex = -1;
    for (auto& info : gModulationDestinationInfo) {
        CCASSERT(info.mIndexOverall == (size_t)info.mAnyEnumVal);
        CCASSERT((int)info.mIndexOverall == (prevIndex + 1));
        prevIndex ++;
    }

    // populate simple lookup tables
    for (ModulationDestinationInfo &info : gModulationDestinationInfo)
    {
        if (!info.mIsValidModulation)
            continue;
        switch (info.mRate)
        {
        case ModulationRate::ARate:
            CCASSERT(info.mIndexForRate < gARateModulationDestinationCount);
            CCASSERT(!gARateModulationDestinationInfoPtrs[info.mIndexForRate]);
            gARateModulationDestinationInfoPtrs[info.mIndexForRate] = &info;
            CCASSERT(info.mIndexForRate == (size_t)info.mARateEnumVal);
            break;
        case ModulationRate::KRate:
        default:
            CCASSERT(info.mIndexForRate < gKRateModulationDestinationCount);
            CCASSERT(!gKRateModulationDestinationInfoPtrs[info.mIndexForRate]);
            gKRateModulationDestinationInfoPtrs[info.mIndexForRate] = &info;
            CCASSERT(info.mIndexForRate == (size_t)info.mKRateEnumVal);
            break;
        }
    }
    // verify correctness.
    for (auto p : gKRateModulationDestinationInfoPtrs)
    {
        CCASSERT(!!p);
    }
    for (auto p : gARateModulationDestinationInfoPtrs)
    {
        CCASSERT(!!p);
    }
}};

static const ModulationDestinationInfo &GetModulationDestinationInfo(AnyModulationDestination m)
{
    return gModulationDestinationInfo[(size_t)m];
}

} // namespace clarinoid
