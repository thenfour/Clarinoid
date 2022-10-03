
#pragma once

#include <clarinoid/basic/Basic.hpp>
#include "AnalogValue.hpp"

namespace clarinoid
{
#ifndef POLYPHONIC
using MusicalVoiceID_t = uint16_t;

static constexpr MusicalVoiceID_t MAGIC_VOICE_ID_UNASSIGNED =
    std::numeric_limits<MusicalVoiceID_t>::max(); // used as a voice ID for voices that aren't assigned to any musical
                                                  // voice.

static constexpr uint8_t MAGIC_VOICE_ID_LIVE_A = 0; // when we had 1 live voice, this is 0.
static constexpr uint8_t MAGIC_VOICE_ID_LIVE_B = 1;
static constexpr uint8_t HarmLayerToVoiceID(int8_t h)
{
    return h + 2;
}

static inline MusicalVoiceID_t MakeMusicalVoiceID(uint8_t loopLayerID, uint8_t harmVoice)
{
    static_assert(HARM_VOICES < 256, "harmonizer voice ids must fit into a byte");
    static_assert(LOOP_LAYERS < 256, "loop layer ids must fit into a byte");
    return loopLayerID << 8 | harmVoice;
}
#endif // POLYPHONIC

static uint16_t gNextLiveNoteSequenceID = 1;
static inline uint16_t GetNextLiveNoteSequenceID()
{
    return ++ gNextLiveNoteSequenceID;
}

struct MusicalVoice
{
    MusicalVoice() = default;
    MusicalVoice(const MusicalVoice &rhs) = default; // no copy ctor.
    MusicalVoice(MusicalVoice &&) = default;         // no move.
    MusicalVoice &operator=(const MusicalVoice &rhs) = default;
    MusicalVoice &operator=(MusicalVoice &&) =
        default; // no move assignment because i want these objects to stay static.

    void Reset()
    {
        *this = MusicalVoice();
    }

    bool IsPlaying() const
    { // does not consider mIsNoteCurrentlyMuted, because that means the note should go to the synth but it will be at 0
      // volume.
        return !!mMidiNote && !!mVelocity;
    }

    uint16_t mLiveNoteSequenceID; // the sequence
    uint8_t mHarmVoiceID = 0;
    uint8_t mLoopLayerID = 0;

    // |loopLayerID----|harmVoice----|noteID-----------------------|
    // 31             24            16               8             0
    uint32_t GetAttackID() const
    {
        uint32_t ret = mLoopLayerID;
        ret <<= 8;
        ret |= mHarmVoiceID;
        ret <<= 8;
        ret |= mLiveNoteSequenceID;
        return ret;
    }

#ifdef POLYPHONIC
    // used by synth to know if this & rhs are of the same note. if true, then the new note will reset its state.
    bool IsSameSynthContext(const MusicalVoice &rhs) const
    {
        return (GetAttackID() == rhs.GetAttackID()) && (mSynthPatchA == rhs.mSynthPatchA);
    }
#else

    // used by synth to know if this & rhs are of the same note. if true, then the new note will reset its state.
    bool IsSameSynthContext(const MusicalVoice &rhs) const
    {
        return mVoiceId == rhs.mVoiceId;
    }

    MusicalVoiceID_t mVoiceId =
        MAGIC_VOICE_ID_UNASSIGNED; // this is really an outlier member; it's NOT musical state but useful to keep here
                                   // anyway even if it makes thinsg confusing.
#endif                           // POLYPHONIC
    uint32_t mAttackTimestampMS = 0; // millis.
    uint32_t mReleaseTimestampMS = 0;
    bool mIsPhysicallyHeld = false;

    bool mIsNoteCurrentlyMuted = false; // this is needed when this is the "live" voice that has been physically played,
                                        // but the harmonizer demands we not output it.
    uint8_t mMidiNote = 0;
    uint8_t mVelocity = 0;
    AnalogValue01<> mBreath01;
    AnalogValueN11<> mPitchBendN11;
    int16_t mSynthPatchA = 0;
    int16_t mSynthPatchB = -1; // this one can be negative to indicate mute / no patch. by the time this object reaches
                               // SynthVoice, this is always -1. but it's needed during intermittent processing.
    int16_t mHarmPatch = 0;

    float mPan = 0;     // as specified by harmonizer.
    float mGain = 1.0f; // again set by harmonizer
};

struct MusicalVoiceTransitionEvents
{
    bool mNeedsNoteOn = false;
    bool mNeedsNoteOff = false;
    uint8_t mNoteOffNote = 0;
};

// computes the note on & note off events as the result of musical state.
MusicalVoiceTransitionEvents CalculateTransitionEvents(const MusicalVoice &a, const MusicalVoice &b)
{
    MusicalVoiceTransitionEvents ret;
    bool changedPlayingMidiNote = a.IsPlaying() && b.IsPlaying() && (a.mMidiNote != b.mMidiNote);
    ret.mNeedsNoteOn = (b.IsPlaying() && !a.IsPlaying()) || changedPlayingMidiNote;
    // send note off in these cases:
    // - you are not playing but were
    // - or, you are playing, but a different note than before.
    ret.mNeedsNoteOff = (!b.IsPlaying() && a.IsPlaying()) || changedPlayingMidiNote;
    ret.mNoteOffNote = a.mMidiNote;

    return ret;
}

} // namespace clarinoid
