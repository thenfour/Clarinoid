#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

struct IModulationProvider
{
    virtual float IModulationProvider_GetKRateModulationSourceValueN11(KRateModulationSource src) = 0;
    virtual std::pair<AudioStream *, size_t> IModulationProvider_GetARateSourcePort(ARateModulationSource src) = 0;
    virtual std::pair<AudioStream *, size_t> IModulationProvider_GetARateDestinationPort(
        ARateModulationDestination dest) = 0;
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
     (size_t)AnyModulationSource::LFO3, // index overall
     AnyModulationSource::LFO3,
     (size_t)ARateModulationSource::LFO3, // index for rate
     ModulationRate::ARate,
     (KRateModulationSource)0,
     ARateModulationSource::LFO3,
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
     (size_t)AnyModulationSource::ENV3, // index overall
     AnyModulationSource::ENV3,
     (size_t)ARateModulationSource::ENV3, // index for rate
     ModulationRate::ARate,
     (KRateModulationSource)0,
     ARateModulationSource::ENV3,
     0, // range min
     1, // range max
     ModulationPoleType::Positive01},

    // {true,
    //  (size_t)AnyModulationSource::Osc1FB, // index overall
    //  AnyModulationSource::Osc1FB,
    //  (size_t)ARateModulationSource::Osc1FB, // index for rate
    //  ModulationRate::ARate,
    //  (KRateModulationSource)0,
    //  ARateModulationSource::Osc1FB,
    //  -1, // range min
    //  1,  // range max
    //  ModulationPoleType::N11},

    // {true,
    //  (size_t)AnyModulationSource::Osc2FB, // index overall
    //  AnyModulationSource::Osc2FB,
    //  (size_t)ARateModulationSource::Osc2FB, // index for rate
    //  ModulationRate::ARate,
    //  (KRateModulationSource)0,
    //  ARateModulationSource::Osc2FB,
    //  -1, // range min
    //  1,  // range max
    //  ModulationPoleType::N11},

    // {true,
    //  (size_t)AnyModulationSource::Osc3FB, // index overall
    //  AnyModulationSource::Osc3FB,
    //  (size_t)ARateModulationSource::Osc3FB, // index for rate
    //  ModulationRate::ARate,
    //  (KRateModulationSource)0,
    //  ARateModulationSource::Osc3FB,
    //  -1, // range min
    //  1,  // range max
    //  ModulationPoleType::N11},

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

    {true,
     (size_t)AnyModulationSource::Velocity, // index overall
     AnyModulationSource::Velocity,
     (size_t)KRateModulationSource::Velocity, // index for rate
     ModulationRate::KRate,
     KRateModulationSource::Velocity,
     (ARateModulationSource)0,
     0, // range min
     1, // range max
     ModulationPoleType::Positive01},

    {true,
     (size_t)AnyModulationSource::NoteValue, // index overall
     AnyModulationSource::NoteValue,
     (size_t)KRateModulationSource::NoteValue, // index for rate
     ModulationRate::KRate,
     KRateModulationSource::NoteValue,
     (ARateModulationSource)0,
     0, // range min
     1, // range max
     ModulationPoleType::Positive01},

    {true,
     (size_t)AnyModulationSource::RandomTrigger, // index overall
     AnyModulationSource::RandomTrigger,
     (size_t)KRateModulationSource::RandomTrigger, // index for rate
     ModulationRate::KRate,
     KRateModulationSource::RandomTrigger,
     (ARateModulationSource)0,
     0, // range min
     1, // range max
     ModulationPoleType::Positive01},

    {true,
     (size_t)AnyModulationSource::ModWheel, // index overall
     AnyModulationSource::ModWheel,
     (size_t)KRateModulationSource::ModWheel, // index for rate
     ModulationRate::KRate,
     KRateModulationSource::ModWheel,
     (ARateModulationSource)0,
     0, // range min
     1, // range max
     ModulationPoleType::Positive01},

    {true,
     (size_t)AnyModulationSource::Macro1, // index overall
     AnyModulationSource::Macro1,
     (size_t)KRateModulationSource::Macro1, // index for rate
     ModulationRate::KRate,
     KRateModulationSource::Macro1,
     (ARateModulationSource)0,
     0, // range min
     1, // range max
     ModulationPoleType::Positive01},

    {true,
     (size_t)AnyModulationSource::Macro2, // index overall
     AnyModulationSource::Macro2,
     (size_t)KRateModulationSource::Macro2, // index for rate
     ModulationRate::KRate,
     KRateModulationSource::Macro2,
     (ARateModulationSource)0,
     0, // range min
     1, // range max
     ModulationPoleType::Positive01},

    {true,
     (size_t)AnyModulationSource::Macro3, // index overall
     AnyModulationSource::Macro3,
     (size_t)KRateModulationSource::Macro3, // index for rate
     ModulationRate::KRate,
     KRateModulationSource::Macro3,
     (ARateModulationSource)0,
     0, // range min
     1, // range max
     ModulationPoleType::Positive01},

    {true,
     (size_t)AnyModulationSource::Macro4, // index overall
     AnyModulationSource::Macro4,
     (size_t)KRateModulationSource::Macro4, // index for rate
     ModulationRate::KRate,
     KRateModulationSource::Macro4,
     (ARateModulationSource)0,
     0, // range min
     1, // range max
     ModulationPoleType::Positive01},

    {true,
     (size_t)AnyModulationSource::Pedal, // index overall
     AnyModulationSource::Pedal,
     (size_t)KRateModulationSource::Pedal, // index for rate
     ModulationRate::KRate,
     KRateModulationSource::Pedal,
     (ARateModulationSource)0,
     0, // range min
     1, // range max
     ModulationPoleType::Positive01},

};

ModulationSourceInfo *gKRateModulationSourceInfoPtrs[gKRateModulationSourceCount] = {nullptr};
ModulationSourceInfo *gARateModulationSourceInfoPtrs[gARateModulationSourceCount] = {nullptr};

StaticInit gModSourceInit = {[]() {
    // ensure indices are sequential, match the any enum val
    int prevIndex = -1;
    for (auto &info : gModulationSourceInfo)
    {
        CCASSERT(info.mIndexOverall == (size_t)info.mAnyEnumVal);
        CCASSERT((int)info.mIndexOverall == (prevIndex + 1));
        prevIndex++;
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
    // const float mRangeMin = 0;
    // const float mRangeMax = 0;
    // const ModulationPoleType mPoleType = ModulationPoleType::N11;
};

ModulationDestinationInfo gModulationDestinationInfo[gAnyModulationDestinationCount] = {
    {
        false,                                  // valid
        (size_t)AnyModulationDestination::None, // index overall
        AnyModulationDestination::None,
        0, // index for rate
        ModulationRate::ARate,
        (KRateModulationDestination)0,
        (ARateModulationDestination)0,
        //  0, // range min
        //  0, // range max
        //  ModulationPoleType::N11},
    },

    {
        true,
        (size_t)AnyModulationDestination::Osc1PulseWidth, // index overall
        AnyModulationDestination::Osc1PulseWidth,
        (size_t)ARateModulationDestination::Osc1PulseWidth, // index for rate
        ModulationRate::ARate,
        (KRateModulationDestination)0,
        ARateModulationDestination::Osc1PulseWidth,
        //  -1, // range min
        //  1,  // range max
        //  ModulationPoleType::N11},
    },
    {
        true,
        (size_t)AnyModulationDestination::Osc1Phase, // index overall
        AnyModulationDestination::Osc1Phase,
        (size_t)ARateModulationDestination::Osc1Phase, // index for rate
        ModulationRate::ARate,
        (KRateModulationDestination)0,
        ARateModulationDestination::Osc1Phase,
        //  -1, // range min
        //  1,  // range max
        //  ModulationPoleType::N11},
    },
    {
        true,
        (size_t)AnyModulationDestination::Osc2PulseWidth, // index overall
        AnyModulationDestination::Osc2PulseWidth,
        (size_t)ARateModulationDestination::Osc2PulseWidth, // index for rate
        ModulationRate::ARate,
        (KRateModulationDestination)0,
        ARateModulationDestination::Osc2PulseWidth,
        //  -1, // range min
        //  1,  // range max
        //  ModulationPoleType::N11},
    },
    {
        true,
        (size_t)AnyModulationDestination::Osc2Phase, // index overall
        AnyModulationDestination::Osc2Phase,
        (size_t)ARateModulationDestination::Osc2Phase, // index for rate
        ModulationRate::ARate,
        (KRateModulationDestination)0,
        ARateModulationDestination::Osc2Phase,
        //  -1, // range min
        //  1,  // range max
        //  ModulationPoleType::N11},
    },
    {
        true,
        (size_t)AnyModulationDestination::Osc3PulseWidth, // index overall
        AnyModulationDestination::Osc3PulseWidth,
        (size_t)ARateModulationDestination::Osc3PulseWidth, // index for rate
        ModulationRate::ARate,
        (KRateModulationDestination)0,
        ARateModulationDestination::Osc3PulseWidth,
        //  -1, // range min
        //  1,  // range max
        //  ModulationPoleType::N11},
    },
    {
        true,
        (size_t)AnyModulationDestination::Osc3Phase, // index overall
        AnyModulationDestination::Osc3Phase,
        (size_t)ARateModulationDestination::Osc3Phase, // index for rate
        ModulationRate::ARate,
        (KRateModulationDestination)0,
        ARateModulationDestination::Osc3Phase,
        //  -1, // range min
        //  1,  // range max
        //  ModulationPoleType::N11},
    },

    {
        true,
        (size_t)AnyModulationDestination::FilterCutoff, // index overall
        AnyModulationDestination::FilterCutoff,
        (size_t)KRateModulationDestination::FilterCutoff, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::FilterCutoff,
        (ARateModulationDestination)0,
        //  0, // range min
        //  1, // range max
        //  ModulationPoleType::Positive01},
    },

    {
        true,
        (size_t)AnyModulationDestination::MasterVolume, // index overall
        AnyModulationDestination::MasterVolume,
        (size_t)KRateModulationDestination::MasterVolume, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::MasterVolume,
        (ARateModulationDestination)0,
        //  0, // range min
        //  1, // range max
        //  ModulationPoleType::Positive01},
    },

    {
        true,
        (size_t)AnyModulationDestination::Detune, // index overall
        AnyModulationDestination::Detune,
        (size_t)KRateModulationDestination::Detune, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Detune,
        (ARateModulationDestination)0,
    },

    {
        true,
        (size_t)AnyModulationDestination::Osc1Volume, // index overall
        AnyModulationDestination::Osc1Volume,
        (size_t)KRateModulationDestination::Osc1Volume, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Osc1Volume,
        (ARateModulationDestination)0,
        //  0, // range min
        //  1, // range max
        //  ModulationPoleType::Positive01},
    },

    {
        true,
        (size_t)AnyModulationDestination::Osc2Volume, // index overall
        AnyModulationDestination::Osc2Volume,
        (size_t)KRateModulationDestination::Osc2Volume, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Osc2Volume,
        (ARateModulationDestination)0,
        //  0, // range min
        //  1, // range max
        //  ModulationPoleType::Positive01},
    },

    {
        true,
        (size_t)AnyModulationDestination::Osc3Volume, // index overall
        AnyModulationDestination::Osc3Volume,
        (size_t)KRateModulationDestination::Osc3Volume, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Osc3Volume,
        (ARateModulationDestination)0,
        //  0, // range min
        //  1, // range max
        //  ModulationPoleType::Positive01},
    },



    {
        true,
        (size_t)AnyModulationDestination::Osc1RingModAmt, // index overall
        AnyModulationDestination::Osc1RingModAmt,
        (size_t)KRateModulationDestination::Osc1RingModAmt, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Osc1RingModAmt,
        (ARateModulationDestination)0,
    },


    {
        true,
        (size_t)AnyModulationDestination::Osc2RingModAmt, // index overall
        AnyModulationDestination::Osc2RingModAmt,
        (size_t)KRateModulationDestination::Osc2RingModAmt, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Osc2RingModAmt,
        (ARateModulationDestination)0,
    },


    {
        true,
        (size_t)AnyModulationDestination::Osc3RingModAmt, // index overall
        AnyModulationDestination::Osc3RingModAmt,
        (size_t)KRateModulationDestination::Osc3RingModAmt, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Osc3RingModAmt,
        (ARateModulationDestination)0,
    },





    {
        true,                                             // is a modulation
        (size_t)AnyModulationDestination::Osc1FMFeedback, // index overall
        AnyModulationDestination::Osc1FMFeedback,
        (size_t)KRateModulationDestination::Osc1FMFeedback, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Osc1FMFeedback,
        (ARateModulationDestination)0,
        //  0, // range min
        //  1, // range max
        //  ModulationPoleType::Positive01},
    },

    {
        true,                                             // is a modulation
        (size_t)AnyModulationDestination::Osc2FMFeedback, // index overall
        AnyModulationDestination::Osc2FMFeedback,
        (size_t)KRateModulationDestination::Osc2FMFeedback, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Osc2FMFeedback,
        (ARateModulationDestination)0,
        //  0, // range min
        //  1, // range max
        //  ModulationPoleType::Positive01},
    },

    {
        true,                                             // is a modulation
        (size_t)AnyModulationDestination::Osc3FMFeedback, // index overall
        AnyModulationDestination::Osc3FMFeedback,
        (size_t)KRateModulationDestination::Osc3FMFeedback, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Osc3FMFeedback,
        (ARateModulationDestination)0,
        //  0, // range min
        //  1, // range max
        //  ModulationPoleType::Positive01},
    },

    {
        true,                                                // is a modulation
        (size_t)AnyModulationDestination::OverallFMStrength, // index overall
        AnyModulationDestination::OverallFMStrength,
        (size_t)KRateModulationDestination::OverallFMStrength, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::OverallFMStrength,
        (ARateModulationDestination)0,
        //  0, // range min
        //  1, // range max
        //  ModulationPoleType::Positive01},
    },

    {
        true,                                             // is a modulation
        (size_t)AnyModulationDestination::FMStrength2To1, // index overall
        AnyModulationDestination::FMStrength2To1,
        (size_t)KRateModulationDestination::FMStrength2To1, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::FMStrength2To1,
        (ARateModulationDestination)0,
        //  0, // range min
        //  1, // range max
        //  ModulationPoleType::Positive01},
    },

    {
        true,                                             // is a modulation
        (size_t)AnyModulationDestination::FMStrength3To1, // index overall
        AnyModulationDestination::FMStrength3To1,
        (size_t)KRateModulationDestination::FMStrength3To1, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::FMStrength3To1,
        (ARateModulationDestination)0,
        //  0, // range min
        //  1, // range max
        //  ModulationPoleType::Positive01},
    },

    {
        true,                                             // is a modulation
        (size_t)AnyModulationDestination::FMStrength1To2, // index overall
        AnyModulationDestination::FMStrength1To2,
        (size_t)KRateModulationDestination::FMStrength1To2, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::FMStrength1To2,
        (ARateModulationDestination)0,
        //  0, // range min
        //  1, // range max
        //  ModulationPoleType::Positive01},
    },

    {
        true,                                             // is a modulation
        (size_t)AnyModulationDestination::FMStrength3To2, // index overall
        AnyModulationDestination::FMStrength3To2,
        (size_t)KRateModulationDestination::FMStrength3To2, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::FMStrength3To2,
        (ARateModulationDestination)0,
        //  0, // range min
        //  1, // range max
        //  ModulationPoleType::Positive01},
    },

    {
        true,                                             // is a modulation
        (size_t)AnyModulationDestination::FMStrength1To3, // index overall
        AnyModulationDestination::FMStrength1To3,
        (size_t)KRateModulationDestination::FMStrength1To3, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::FMStrength1To3,
        (ARateModulationDestination)0,
        //  0, // range min
        //  1, // range max
        //  ModulationPoleType::Positive01},
    },

    {
        true,                                             // is a modulation
        (size_t)AnyModulationDestination::FMStrength2To3, // index overall
        AnyModulationDestination::FMStrength2To3,
        (size_t)KRateModulationDestination::FMStrength2To3, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::FMStrength2To3,
        (ARateModulationDestination)0,
        //  0, // range min
        //  1, // range max
        //  ModulationPoleType::Positive01},
    },

    {
        true,                                                 // is a modulation
        (size_t)AnyModulationDestination::Osc1FrequencyParam, // index overall
        AnyModulationDestination::Osc1FrequencyParam,
        (size_t)KRateModulationDestination::Osc1FrequencyParam, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Osc1FrequencyParam,
        (ARateModulationDestination)0,
        //  0, // range min
        //  1, // range max
        //  ModulationPoleType::Positive01},
    },

    {
        true,                                                 // is a modulation
        (size_t)AnyModulationDestination::Osc2FrequencyParam, // index overall
        AnyModulationDestination::Osc2FrequencyParam,
        (size_t)KRateModulationDestination::Osc2FrequencyParam, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Osc2FrequencyParam,
        (ARateModulationDestination)0,
        //  0, // range min
        //  1, // range max
        //  ModulationPoleType::Positive01},
    },

    {
        true,                                                 // is a modulation
        (size_t)AnyModulationDestination::Osc3FrequencyParam, // index overall
        AnyModulationDestination::Osc3FrequencyParam,
        (size_t)KRateModulationDestination::Osc3FrequencyParam, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Osc3FrequencyParam,
        (ARateModulationDestination)0,
        //  0, // range min
        //  1, // range max
        //  ModulationPoleType::Positive01},
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Osc1SyncFrequencyParam, // index overall
        AnyModulationDestination::Osc1SyncFrequencyParam,
        (size_t)KRateModulationDestination::Osc1SyncFrequencyParam, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Osc1SyncFrequencyParam,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Osc2SyncFrequencyParam, // index overall
        AnyModulationDestination::Osc2SyncFrequencyParam,
        (size_t)KRateModulationDestination::Osc2SyncFrequencyParam, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Osc2SyncFrequencyParam,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Osc3SyncFrequencyParam, // index overall
        AnyModulationDestination::Osc3SyncFrequencyParam,
        (size_t)KRateModulationDestination::Osc3SyncFrequencyParam, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Osc3SyncFrequencyParam,
        (ARateModulationDestination)0,
    },


    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env1DelayTime, // index overall
        AnyModulationDestination::Env1DelayTime,
        (size_t)KRateModulationDestination::Env1DelayTime, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env1DelayTime,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env1AttackTime, // index overall
        AnyModulationDestination::Env1AttackTime,
        (size_t)KRateModulationDestination::Env1AttackTime, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env1AttackTime,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env1AttackCurve, // index overall
        AnyModulationDestination::Env1AttackCurve,
        (size_t)KRateModulationDestination::Env1AttackCurve, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env1AttackCurve,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env1HoldTime, // index overall
        AnyModulationDestination::Env1HoldTime,
        (size_t)KRateModulationDestination::Env1HoldTime, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env1HoldTime,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env1DecayTime, // index overall
        AnyModulationDestination::Env1DecayTime,
        (size_t)KRateModulationDestination::Env1DecayTime, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env1DecayTime,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env1DecayCurve, // index overall
        AnyModulationDestination::Env1DecayCurve,
        (size_t)KRateModulationDestination::Env1DecayCurve, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env1DecayCurve,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env1SustainLevel, // index overall
        AnyModulationDestination::Env1SustainLevel,
        (size_t)KRateModulationDestination::Env1SustainLevel, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env1SustainLevel,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env1ReleaseTime, // index overall
        AnyModulationDestination::Env1ReleaseTime,
        (size_t)KRateModulationDestination::Env1ReleaseTime, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env1ReleaseTime,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env1ReleaseCurve, // index overall
        AnyModulationDestination::Env1ReleaseCurve,
        (size_t)KRateModulationDestination::Env1ReleaseCurve, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env1ReleaseCurve,
        (ARateModulationDestination)0,
    },



    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env2DelayTime, // index overall
        AnyModulationDestination::Env2DelayTime,
        (size_t)KRateModulationDestination::Env2DelayTime, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env2DelayTime,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env2AttackTime, // index overall
        AnyModulationDestination::Env2AttackTime,
        (size_t)KRateModulationDestination::Env2AttackTime, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env2AttackTime,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env2AttackCurve, // index overall
        AnyModulationDestination::Env2AttackCurve,
        (size_t)KRateModulationDestination::Env2AttackCurve, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env2AttackCurve,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env2HoldTime, // index overall
        AnyModulationDestination::Env2HoldTime,
        (size_t)KRateModulationDestination::Env2HoldTime, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env2HoldTime,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env2DecayTime, // index overall
        AnyModulationDestination::Env2DecayTime,
        (size_t)KRateModulationDestination::Env2DecayTime, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env2DecayTime,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env2DecayCurve, // index overall
        AnyModulationDestination::Env2DecayCurve,
        (size_t)KRateModulationDestination::Env2DecayCurve, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env2DecayCurve,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env2SustainLevel, // index overall
        AnyModulationDestination::Env2SustainLevel,
        (size_t)KRateModulationDestination::Env2SustainLevel, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env2SustainLevel,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env2ReleaseTime, // index overall
        AnyModulationDestination::Env2ReleaseTime,
        (size_t)KRateModulationDestination::Env2ReleaseTime, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env2ReleaseTime,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env2ReleaseCurve, // index overall
        AnyModulationDestination::Env2ReleaseCurve,
        (size_t)KRateModulationDestination::Env2ReleaseCurve, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env2ReleaseCurve,
        (ARateModulationDestination)0,
    },



    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env3DelayTime, // index overall
        AnyModulationDestination::Env3DelayTime,
        (size_t)KRateModulationDestination::Env3DelayTime, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env3DelayTime,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env3AttackTime, // index overall
        AnyModulationDestination::Env3AttackTime,
        (size_t)KRateModulationDestination::Env3AttackTime, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env3AttackTime,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env3AttackCurve, // index overall
        AnyModulationDestination::Env3AttackCurve,
        (size_t)KRateModulationDestination::Env3AttackCurve, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env3AttackCurve,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env3HoldTime, // index overall
        AnyModulationDestination::Env3HoldTime,
        (size_t)KRateModulationDestination::Env3HoldTime, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env3HoldTime,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env3DecayTime, // index overall
        AnyModulationDestination::Env3DecayTime,
        (size_t)KRateModulationDestination::Env3DecayTime, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env3DecayTime,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env3DecayCurve, // index overall
        AnyModulationDestination::Env3DecayCurve,
        (size_t)KRateModulationDestination::Env3DecayCurve, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env3DecayCurve,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env3SustainLevel, // index overall
        AnyModulationDestination::Env3SustainLevel,
        (size_t)KRateModulationDestination::Env3SustainLevel, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env3SustainLevel,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env3ReleaseTime, // index overall
        AnyModulationDestination::Env3ReleaseTime,
        (size_t)KRateModulationDestination::Env3ReleaseTime, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env3ReleaseTime,
        (ARateModulationDestination)0,
    },

    {
        true,                                                     // is a modulation
        (size_t)AnyModulationDestination::Env3ReleaseCurve, // index overall
        AnyModulationDestination::Env3ReleaseCurve,
        (size_t)KRateModulationDestination::Env3ReleaseCurve, // index for rate
        ModulationRate::KRate,
        KRateModulationDestination::Env3ReleaseCurve,
        (ARateModulationDestination)0,
    },

};

ModulationDestinationInfo *gKRateModulationDestinationInfoPtrs[gKRateModulationDestinationCount] = {nullptr};
ModulationDestinationInfo *gARateModulationDestinationInfoPtrs[gARateModulationDestinationCount] = {nullptr};

StaticInit gModDestinationInit = {[]() {
    // ensure indices are sequential, match the any enum val
    int prevIndex = -1;
    for (auto &info : gModulationDestinationInfo)
    {
        CCASSERT(info.mIndexOverall == (size_t)info.mAnyEnumVal);
        CCASSERT((int)info.mIndexOverall == (prevIndex + 1));
        prevIndex++;
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
