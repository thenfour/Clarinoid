
#pragma once

//#include <clarinoid/harmonizer/harmonizer.hpp>
//#include <clarinoid/loopstation/LooperHarmonizer.hpp>
#include <clarinoid/application/ControlMapper.hpp>

/*

I'm going to remove looper & harmonizer for the moment for simplicity in dealing with polyphony.
there are 2 lists to maintain:
1) the list of notes the player is physically playing (musical voices)
2) the list of notes which the synth should be playing. (synth voices)

this class is responsible just for generating the list of musical voices playing.
synth then distributes them among synth voices.

*/

namespace clarinoid
{
struct HeldNoteInfo
{
    uint32_t mAttackTimestampMS;
    uint32_t mLiveNoteSequenceID;
    float mVelocity01;
    bool mIsPhysicallyHeld;
    MidiNote mMidiNote;
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
    MusicalEventSource(MusicalEventSourceType type) : mType(type)
    {
    }
};

struct ISynthParamProvider
{
    virtual uint8_t SynthParamProvider_GetMidiCC(uint8_t cc) = 0;
    virtual float SynthParamProvider_GetBreath01() = 0;
    virtual float SynthParamProvider_GetPitchBendN11() = 0;
};

static uint16_t gNextLiveNoteSequenceID = 1;
static inline uint16_t GetNextLiveNoteSequenceID()
{
    return ++gNextLiveNoteSequenceID;
}

struct IHeldNoteTrackerEvents
{
    virtual void IHeldNoteTrackerEvents_OnNoteOn(const HeldNoteInfo &noteInfo) = 0;
    virtual void IHeldNoteTrackerEvents_OnNoteOff(MidiNote n) = 0;
    virtual void IHeldNoteTrackerEvents_OnAllNotesOff() = 0;
};

struct IIncomingMusicalEvents
{
    virtual void IncomingMusicalEvents_OnNoteOn(MusicalEventSource source,
                                                const HeldNoteInfo &noteInfo,
                                                uint16_t synthPatchIndex,
                                                float extraGain,
                                                float extraPan) = 0;
    virtual void IncomingMusicalEvents_OnNoteOff(MusicalEventSource source, MidiNote note) = 0;
    virtual void IncomingMusicalEvents_OnAllNoteOff() = 0;
};

// takes incoming note ons & offs, and keeps track of what's happening
// i'll try to make this polyphonic-friendly but for the moment i only care about monophony.
// also attempt to care about pedal messages.
// see: 7jam, HeldNoteTracker:
// https://github.com/thenfour/digifujam/blob/ca285536909cee8c3282beb4a1f90a8cd3dc86dd/source/DFcommon/DFMusic.js#L310
struct HeldNoteTracker
{
    bool mPedalDown = false;
    std::vector<HeldNoteInfo> mHeldNotes; // NEW NOTES get placed at the BACK.
    IHeldNoteTrackerEvents *mEventHandler;

    HeldNoteTracker(IHeldNoteTrackerEvents *eventHandler) : mEventHandler(eventHandler)
    {
        mHeldNotes.reserve(128);
    }

    void AllNotesOff()
    {
        mHeldNotes.clear();
        mPedalDown = false;
        mEventHandler->IHeldNoteTrackerEvents_OnAllNotesOff();
    }
    void PedalUp()
    {
        mPedalDown = false;
        // remove all notes that are not physically held
        for (auto &noteInfo : mHeldNotes)
        {
            if (noteInfo.mIsPhysicallyHeld)
                continue;

            Serial.println(String("held note tracker PedalUp; calling events::NoteOff on ") +
                           noteInfo.mMidiNote.GetNoteDesc().mName);
            mEventHandler->IHeldNoteTrackerEvents_OnNoteOff(noteInfo.mMidiNote);
        }

        auto newEnd = std::remove_if(mHeldNotes.begin(), mHeldNotes.end(), [](const HeldNoteInfo &n) {
            bool ret = !n.mIsPhysicallyHeld;
            if (ret)
                Serial.println(String("held note tracker PedalUp; removing ") + n.mMidiNote.GetNoteDesc().mName);
            return ret;
        });
        mHeldNotes.erase(newEnd, mHeldNotes.end());

        Serial.println(String("held note tracker PedalUp; count= ") + mHeldNotes.size());
    }

    void PedalDown()
    {
        // Serial.println(String("held note tracker PedalDown"));
        mPedalDown = true;
    }

    void NoteOff(MidiNote note)
    {
        auto existingItem = mHeldNotes.begin();
        for (; existingItem != mHeldNotes.end(); ++existingItem)
        {
            if (existingItem->mMidiNote == note)
                break;
        }
        if (existingItem == mHeldNotes.end())
        {
            // Serial.println(String("held note tracker NoteOff, but is unknown. note=") + note +
            //                "; count=" + mHeldNotes.size());
            return;
        }
        if (this->mPedalDown)
        {
            existingItem->mIsPhysicallyHeld = false;
            // Serial.println(String("held note tracker NoteOff, marking not held. note=") + note +
            //                "; count=" + mHeldNotes.size());
            return;
        }
        // Serial.println(String("held note tracker NoteOff, erasing known note=") + note +
        //                "; count=" + mHeldNotes.size());
        this->mEventHandler->IHeldNoteTrackerEvents_OnNoteOff(existingItem->mMidiNote);
        this->mHeldNotes.erase(existingItem);
    }

    HeldNoteInfo NoteOn(MidiNote note, float velocity01)
    {
        auto existingItem = mHeldNotes.begin();
        for (; existingItem != mHeldNotes.end(); ++existingItem)
        {
            if (existingItem->mMidiNote == note)
                break;
        }

        if (existingItem == mHeldNotes.end())
        {
            // add new note.
            HeldNoteInfo n;
            n.mIsPhysicallyHeld = true;
            n.mAttackTimestampMS = millis();
            n.mVelocity01 = velocity01;
            n.mMidiNote = note;
            n.mLiveNoteSequenceID = GetNextLiveNoteSequenceID();
            mHeldNotes.push_back(n);
            this->mEventHandler->IHeldNoteTrackerEvents_OnNoteOn(n);
            // Serial.println(String("held note tracker NoteOn, adding note=") + note + "; count=" + mHeldNotes.size());
            return n;
        }

        // there's an existing item to deal with. IOW, user is playing a note that's already
        // held for some reason. either a weird MIDI scenario, maybe glitched out. Or more likely,
        // user is holding pedal, and retriggering a key.
        HeldNoteInfo n = *existingItem;       // take a copy.
        this->mHeldNotes.erase(existingItem); // remove original
        n.mVelocity01 = velocity01;
        n.mAttackTimestampMS = millis();
        n.mIsPhysicallyHeld = true;
        n.mLiveNoteSequenceID = GetNextLiveNoteSequenceID();
        this->mHeldNotes.push_back(n); // add back where it belongs: as newest note.
        this->mEventHandler->IHeldNoteTrackerEvents_OnNoteOn(n);
        // Serial.println(String("held note tracker NoteOn, updating note=") + note + "; count=" + mHeldNotes.size());
        return n;
    }

    // // returns whether the held note is populated (false if no notes playing)
    // bool GetLastNoteOn(MusicalVoice &outp)
    // {
    //     if (mHeldNotes.empty())
    //         return false;
    //     outp = *(mHeldNotes.end() - 1);
    //     return true;
    // }
};

struct USBMidiMusicalState : ISynthParamProvider, IHeldNoteTrackerEvents
{
    HeldNoteTracker mHeldNotes;
    uint8_t mCurrentCCValue[128] = {0};

    float mCurrentPitchBendN11 = 0;
    float mCurrentMod01 = 0;

    static USBMidiMusicalState *gInstance;
    AppSettings *mAppSettings;
    IIncomingMusicalEvents *mEventHandler;

    USBMidiMusicalState(AppSettings *appSettings, IIncomingMusicalEvents *eventHandler)
        : mHeldNotes(this), mAppSettings(appSettings), mEventHandler(eventHandler)
    {
        // Serial.println(String("USBMidiMusicalState init"));
        CCASSERT(!gInstance);
        gInstance = this;
        gUsbMidi.setHandleNoteOff(HandleNoteOff);
        gUsbMidi.setHandleNoteOn(HandleNoteOn);
        gUsbMidi.setHandleControlChange(HandleControlChange);
        gUsbMidi.setHandlePitchChange(HandlePitchChange);
    }

    virtual void IHeldNoteTrackerEvents_OnNoteOn(const HeldNoteInfo &noteInfo) override
    {
        auto &perf = gInstance->mAppSettings->GetCurrentPerformancePatch();

        if (perf.mSynthAEnabled && mAppSettings->IsValidSynthPatchId(perf.mSynthPresetA))
        {
            gInstance->mEventHandler->IncomingMusicalEvents_OnNoteOn(
                MusicalEventSource{MusicalEventSourceType::LivePlayA},
                noteInfo,
                perf.mSynthPresetA,
                perf.mSynthAGain,
                0.0f);
        }

        // if (perf.mSynthBEnabled)
        // {
        //     gInstance->mEventHandler->IncomingMusicalEvents_OnNoteOn(
        //         MusicalEventSource{MusicalEventSourceType::LivePlayB},
        //         noteInfo,
        //         perf.mSynthPresetB,
        //         perf.mSynthBGain,
        //         0.0f);
        // }
    }
    virtual void IHeldNoteTrackerEvents_OnNoteOff(MidiNote note) override
    {
        gInstance->mEventHandler->IncomingMusicalEvents_OnNoteOff(MusicalEventSource{MusicalEventSourceType::LivePlayA},
                                                                  note);
        // gInstance->mEventHandler->IncomingMusicalEvents_OnNoteOff(MusicalEventSource{MusicalEventSourceType::LivePlayB},
        //                                                           note);
    }

    virtual void IHeldNoteTrackerEvents_OnAllNotesOff() override
    {
        mEventHandler->IncomingMusicalEvents_OnAllNoteOff();
    }

    virtual uint8_t SynthParamProvider_GetMidiCC(uint8_t cc) override
    {
        return mCurrentCCValue[cc];
    }
    virtual float SynthParamProvider_GetBreath01() override
    {
        return 0;
    }
    virtual float SynthParamProvider_GetPitchBendN11() override
    {
        return mCurrentPitchBendN11;
    }

    static void HandleNoteOff(uint8_t channel, uint8_t note, uint8_t velocity)
    {
        // Serial.println(String("midi device HandleNoteOff: ") + note);
        gInstance->mHeldNotes.NoteOff(MidiNote(note));
    }

    static void HandleNoteOn(uint8_t channel, uint8_t note, uint8_t velocity)
    {
        // Serial.println(String("midi device note on: ") + note);
        float velocity01 = float(velocity) / 127.0f;
        gInstance->mHeldNotes.NoteOn(MidiNote(note), velocity01);
    }

    static void HandleControlChange(uint8_t channel, uint8_t control, uint8_t value)
    {
        Serial.println(String("midi device HandleControlChange : ") + control + " -> " + value);
        //    https://anotherproducer.com/online-tools-for-musicians/midi-cc-list/
        //    64	Damper Pedal on/off	≤63 off, ≥64 on	On/off switch that controls sustain pedal. Nearly every synth
        //    will react to CC 64. (See also Sostenuto CC 66)
        if (control == 64)
        {
            if (value < 64)
            {
                gInstance->mHeldNotes.PedalUp();
            }
            else
            {
                gInstance->mHeldNotes.PedalDown();
            }
        }
        else if (control == 123)
        {
            // all notes off
            gInstance->mHeldNotes.AllNotesOff();
        }
        gInstance->mCurrentCCValue[control] = value;
    }

    static void HandlePitchChange(uint8_t channel, int pitch /* -8192 to 8191 inclusive */)
    {
        // Serial.println(String("pb: ") + pitch);
        if (pitch == 8191)
        {
            gInstance->mCurrentPitchBendN11 = 1.0f;
        }
        else
        {
            gInstance->mCurrentPitchBendN11 = float(pitch) / 8192.0f;
        }
    }

    void Update()
    {
        // auto m1 = micros();
        gUsbHost.Task();
        gUsbMidi.read();

        // auto &perf = mAppSettings->GetCurrentPerformancePatch();

        // // update pitchbend, patch info, etc.
        // for (auto it = mHeldNotes.mHeldNotes.begin(); it != mHeldNotes.mHeldNotes.end(); ++ it)
        // {
        //     it->mHarmPatch = perf.mHarmPreset;
        //     it->mSynthPatchIndex = perf.mSynthPresetA;
        //     // todo: deal with synth preset b.
        // }

        // auto m2 = micros();
        // Serial.println(String("updating usb midi; ") + int(m2 - m1));
    }
};

USBMidiMusicalState *USBMidiMusicalState::gInstance = nullptr;

// // this serves the same function as CCEWIMusicalState, but instead of
// // musical state coming from breath control, pitch device, and touch keys,
// // the musical state comes from a USB MIDI device.
// struct USBMidiMusicalState
// {
//     IDisplay *mpDisplay;
//     AppSettings *mAppSettings;
//     InputDelegator *mInput;
//     IInputSource *mInputSrc;
//     USBMidiReader mMidiDevice;

//     USBMidiMusicalState(IDisplay *pDisplay,
//                         AppSettings *appSettings,
//                         InputDelegator *inputDelegator,
//                         //Metronome *metronome,
//                         //ScaleFollower *scaleFollower,
//                         IInputSource *inputSrc)
//         : mpDisplay(pDisplay), mAppSettings(appSettings), mInput(inputDelegator), mInputSrc(inputSrc)
//     {
//     }

//     void Update()
//     {
//         mMidiDevice.Update();
//         //MusicalVoice mNewState(mLiveVoice);

//         // mNewState.mBreath01 = mMidiDevice.mCurrentMod01;
//         // mNewState.mPitchBendN11 = mMidiDevice.mCurrentPitchBendN11;

//         // HeldNoteTracker::HeldNoteInfo heldNote;
//         // bool isPlaying = mMidiDevice.mHeldNotes.GetLastNoteOn(heldNote);
//         // mNewState.mVelocity = isPlaying ? heldNote.velocity : 0;
//         // mNewState.mMidiNote = isPlaying ? heldNote.note : 0;
//         // mNewState.mAttackTimestampMS = heldNote.timestampMS;

//         // auto &perf = mAppSettings->GetCurrentPerformancePatch();
//         // mNewState.mHarmPatch = perf.mHarmPreset;
//         // mNewState.mSynthPatchA = perf.mSynthPresetA;
//         // mNewState.mSynthPatchB = perf.mSynthPresetB;
//         // mNewState.mPan = 0; // panning is part of musical state because harmonizer needs it per voice, but it's
//         //                     // actually calculated by the synth based on synth preset.

//         // mNewState.mMidiNote =
//         //     ClampInclusive(mNewState.mMidiNote + mAppSettings->GetCurrentPerformancePatch().mTranspose, 0, 127);

//         // //auto transitionEvents = CalculateTransitionEvents(mLiveVoice, mNewState);

//         // // calculate mLiveNoteSequenceID. if it's a note on, generate a new ID

//         // if (isPlaying)
//         // {
//         //     mLastPlayedNote = mNewState.mMidiNote;
//         // }

//         // if (transitionEvents.mNeedsNoteOn)
//         // {
//         //     //Serial.println(String("MS note on: ") + mNewState.mMidiNote);
//         // }
//         // else if (mNewState.IsPlaying())
//         // {
//         //     // Serial.println(String("is playing, but transition says no. oldstate.note=") +
//         mLiveVoice.mMidiNote);
//         // }

//         // mLiveVoice = mNewState;

//         // // we have calculated mLiveVoice, converting physical to live musical state.
//         // // now take the live musical state, and fills out mMusicalVoices based on harmonizer & looper settings.
//         // size_t newVoiceCount = mLooper.Update(mLiveVoice, transitionEvents, mMusicalVoices,
//         EndPtr(mMusicalVoices));
//         // mVoiceCount = newVoiceCount;
//     }
// };

static constexpr auto aoeusnthcphic = sizeof(USBMidiMusicalState);

} // namespace clarinoid
