
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
};

struct MusicalEventSource
{
    MusicalEventSourceType mType = MusicalEventSourceType::Null;
    uint8_t mHarmonizerVoiceIndex = 0xff;
    uint8_t mLoopstationLayerIndex = 0xff;
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
        }
    }
};

struct MusicalVoice
{
    MusicalEventSource mSource;
    HeldNoteInfo mNoteInfo;
    ISynthParamProvider *mpParamProvider = nullptr;
    bool mIsActive = false; // if false, process no further. nothing else here would be valid. if true, mpPreset is guaranteed valid.
    SynthPreset *mpSynthPatch = nullptr;
    PerformancePatch *mpPerf = nullptr;
};

static constexpr auto akspp = sizeof(MusicalVoice);

// synthesizer implements this.
// when notes have been processed by musical state, they get sent to the synth engine via this interface.
struct IMusicalEventsForSynth
{
    virtual void IMusicalEventsForSynth_OnNoteOn(const MusicalVoice& mv) = 0;
    virtual void IMusicalEventsForSynth_OnNoteOff(const MusicalVoice &mv) = 0; // mv won't be the same instance as the noteon; it's newly constructed.
    virtual void IMusicalEventsForSynth_OnAllNoteOff() = 0;
};



// #ifndef POLYPHONIC
// using MusicalVoiceID_t = uint16_t;

// static constexpr MusicalVoiceID_t MAGIC_VOICE_ID_UNASSIGNED =
//     std::numeric_limits<MusicalVoiceID_t>::max(); // used as a voice ID for voices that aren't assigned to any musical
//                                                   // voice.

// static constexpr uint8_t MAGIC_VOICE_ID_LIVE_A = 0; // when we had 1 live voice, this is 0.
// static constexpr uint8_t MAGIC_VOICE_ID_LIVE_B = 1;
// static constexpr uint8_t HarmLayerToVoiceID(int8_t h)
// {
//     return h + 2;
// }

// static inline MusicalVoiceID_t MakeMusicalVoiceID(uint8_t loopLayerID, uint8_t harmVoice)
// {
//     static_assert(HARM_VOICES < 256, "harmonizer voice ids must fit into a byte");
//     static_assert(LOOP_LAYERS < 256, "loop layer ids must fit into a byte");
//     return loopLayerID << 8 | harmVoice;
// }
// #endif // POLYPHONIC


// struct MusicalVoice
// {
//     MusicalVoice() = default;
//     MusicalVoice(const MusicalVoice &rhs) = default; // no copy ctor.
//     MusicalVoice(MusicalVoice &&) = default;         // no move.
//     MusicalVoice &operator=(const MusicalVoice &rhs) = default;
//     MusicalVoice &operator=(MusicalVoice &&) =
//         default; // no move assignment because i want these objects to stay static.

//     void Reset()
//     {
//         *this = MusicalVoice();
//     }

//     bool IsPlaying() const
//     { // does not consider mIsNoteCurrentlyMuted, because that means the note should go to the synth but it will be at 0
//       // volume.
//         return !!mMidiNote && !!mVelocity;
//     }

//     uint16_t mLiveNoteSequenceID; // the sequence
//     uint8_t mHarmVoiceID = 0;
//     uint8_t mLoopLayerID = 0;

//     // |loopLayerID----|harmVoice----|noteID-----------------------|
//     // 31             24            16               8             0
//     uint32_t GetAttackID() const
//     {
//         uint32_t ret = mLoopLayerID;
//         ret <<= 8;
//         ret |= mHarmVoiceID;
//         ret <<= 8;
//         ret |= mLiveNoteSequenceID;
//         return ret;
//     }

// #ifdef POLYPHONIC
//     // used by synth to know if this & rhs are of the same note. if true, then the new note will reset its state.
//     bool IsSameSynthContext(const MusicalVoice &rhs) const
//     {
//         return (mSynthPatchIndex == rhs.mSynthPatchIndex) &&
//             (GetAttackID() == rhs.GetAttackID());
//     }
// #else

//     // used by synth to know if this & rhs are of the same note. if true, then the new note will reset its state.
//     bool IsSameSynthContext(const MusicalVoice &rhs) const
//     {
//         return mVoiceId == rhs.mVoiceId;
//     }

//     MusicalVoiceID_t mVoiceId =
//         MAGIC_VOICE_ID_UNASSIGNED; // this is really an outlier member; it's NOT musical state but useful to keep here
//                                    // anyway even if it makes thinsg confusing.
// #endif                           // POLYPHONIC
//     uint32_t mAttackTimestampMS = 0; // millis.
//     uint32_t mReleaseTimestampMS = 0;
//     bool mIsPhysicallyHeld = false;

//     bool mIsNoteCurrentlyMuted = false; // this is needed when this is the "live" voice that has been physically played,
//                                         // but the harmonizer demands we not output it.
//     uint8_t mMidiNote = 0;
//     uint8_t mVelocity = 0;
//     AnalogValue01<> mBreath01;
//     AnalogValueN11<> mPitchBendN11;
//     int16_t mSynthPatchIndex = -1;
//     int16_t mHarmPatch = 0;

//     float mPan = 0;     // as specified by harmonizer.
//     float mGain = 1.0f; // again set by harmonizer
// };

// struct MusicalVoiceTransitionEvents
// {
//     bool mNeedsNoteOn = false;
//     bool mIsLegatoNoteOn = false; // implies needs note on
//     bool mNeedsNoteOff = false; // may be combined with note on (note off previous + note on next)
//     uint8_t mNoteOffNote = 0;
//     bool mSynthContextChanged = false;
// };

// // computes the note on & note off events as the result of musical state.
// MusicalVoiceTransitionEvents CalculateTransitionEvents(const MusicalVoice &a, const MusicalVoice &b)
// {
//     MusicalVoiceTransitionEvents ret;
//     if (!a.IsSameSynthContext(b)) {
//         // different synth patch / harm 
//         ret.mSynthContextChanged = true;
//     }
//     ret.mIsLegatoNoteOn = a.IsPlaying() && b.IsPlaying() && (a.mMidiNote != b.mMidiNote) && !ret.mSynthContextChanged;
//     ret.mNeedsNoteOn = (b.IsPlaying() && !a.IsPlaying()) || ret.mIsLegatoNoteOn || ret.mSynthContextChanged;
//     // send note off in these cases:
//     // - you are not playing but were
//     // - or, you are playing, but a different note than before.
//     ret.mNeedsNoteOff = (!b.IsPlaying() && a.IsPlaying());
//     ret.mNoteOffNote = a.mMidiNote;

//     return ret;
// }

} // namespace clarinoid
