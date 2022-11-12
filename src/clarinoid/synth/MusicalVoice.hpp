
#pragma once

namespace clarinoid
{

struct ISynthParamProvider
{
    virtual uint8_t SynthParamProvider_GetMidiCC(MidiCCValue cc) = 0;
    virtual float SynthParamProvider_GetBreath01() = 0;
    virtual float SynthParamProvider_GetPitchBendN11() = 0;
    virtual float SynthParamProvider_GetMacroValue01(size_t i) = 0;
};

enum class MusicalEventSourceType : uint8_t
{
    Null,
    LivePlayA,
    LivePlayB,
    Harmonizer,
    Loopstation,
    LoopstationHarmonizer, // uses both loop layer + harm voice
};

EnumItemInfo<MusicalEventSourceType> gMusicalEventSourceTypeItems[6] = {
    {MusicalEventSourceType::Null, "Null"},
    {MusicalEventSourceType::LivePlayA, "LivePlayA"},
    {MusicalEventSourceType::LivePlayB, "LivePlayB"},
    {MusicalEventSourceType::Harmonizer, "Harmonizer"},
    {MusicalEventSourceType::Loopstation, "Loopstation"},
    {MusicalEventSourceType::LoopstationHarmonizer, "LoopstationHarmonizer"},
};

EnumInfo<MusicalEventSourceType> gMusicalEventSourceTypeInfo("MusicalEventSourceType", gMusicalEventSourceTypeItems);

struct MusicalEventSource
{
    MusicalEventSourceType mType = MusicalEventSourceType::Null;
    uint8_t mHarmonizerVoiceIndex = 0xff;
    uint8_t mLoopstationLayerIndex = 0xff;

    String ToString() const
    {
        String ret = String("src:");
        ret += gMusicalEventSourceTypeInfo.GetValueDisplayName(mType);
        ret += String(", h:") + mHarmonizerVoiceIndex + ", l:" + mLoopstationLayerIndex;
        return ret;
    }

    MusicalEventSource()
    {
    }
    MusicalEventSource(MusicalEventSourceType type, uint8_t harmonizerVoiceIndex, uint8_t loopstationLayerIndex)
        : mType(type), mHarmonizerVoiceIndex(harmonizerVoiceIndex), mLoopstationLayerIndex(loopstationLayerIndex)
    {
    }
    explicit MusicalEventSource(MusicalEventSourceType type) : mType(type)
    {
    }
    bool Equals(const MusicalEventSource &rhs) const
    {
        if (mType != rhs.mType)
            return false;
        switch (mType)
        {
        default:
        case MusicalEventSourceType::Null:
        case MusicalEventSourceType::LivePlayA:
        case MusicalEventSourceType::LivePlayB:
            return true;
        case MusicalEventSourceType::Harmonizer:
            return mHarmonizerVoiceIndex == rhs.mHarmonizerVoiceIndex;
        case MusicalEventSourceType::Loopstation:
            return mLoopstationLayerIndex == rhs.mLoopstationLayerIndex;
        case MusicalEventSourceType::LoopstationHarmonizer:
            return (mLoopstationLayerIndex == rhs.mLoopstationLayerIndex) && (mHarmonizerVoiceIndex == rhs.mHarmonizerVoiceIndex);
        }
    }
};

struct MusicalVoice
{
    bool mIsActive =
        false; // if false, process no further. nothing else here would be valid. if true, mpPreset is guaranteed valid.
    MusicalEventSource mSource;
    HeldNoteInfo mNoteInfo;
    uint32_t mHarmonizerSourceNoteID = 0; // for harmonized notes, which is the source ID? this tells the synthesizer how to associate notes for note-off.
    ISynthParamProvider *mpParamProvider = nullptr;
    const SynthPatch *mpSynthPatch = nullptr;
    //const PerformancePatch *mpPerf = nullptr;

    String ToString() const 
    {
        return String("[MV ") + (mIsActive ? "ON " : "OFF") +//
         mSource.ToString() +
         ", note:" + mNoteInfo.ToString() + //
         ", prov:" + uintptr_t(mpParamProvider) +//
          ", patch:" + uintptr_t(mpSynthPatch) + //
          //", perf:" + uintptr_t(mpPerf) +//
           "]";
    }
};

static constexpr auto aks5pp = sizeof(MusicalEventSource);
static constexpr auto ak4pp = sizeof(HeldNoteInfo);
static constexpr auto aksp6p = sizeof(MusicalVoice);

// synthesizer implements this.
// when notes have been processed by musical state, they get sent to the synth engine via this interface.
struct IMusicalEventsForSynth
{
    virtual void IMusicalEventsForSynth_OnNoteOn(const MusicalVoice &mv) = 0;
    virtual void IMusicalEventsForSynth_OnNoteOff(
        const MusicalVoice &mv) = 0; // mv won't be the same instance as the noteon; it's newly constructed.
    virtual void IMusicalEventsForSynth_OnAllNoteOff(MusicalEventSource src) = 0;
};

} // namespace clarinoid
