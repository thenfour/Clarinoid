
#pragma once

#include <clarinoid/harmonizer/harmonizer.hpp>
#include <clarinoid/loopstation/LooperHarmonizer.hpp>
#include <clarinoid/application/ControlMapper.hpp>

namespace clarinoid
{

// takes incoming note ons & offs, and keeps track of what's happening
// i'll try to make this polyphonic-friendly but for the moment i only care about monophony.
// also attempt to care about pedal messages.
// see: 7jam, HeldNoteTracker:
// https://github.com/thenfour/digifujam/blob/ca285536909cee8c3282beb4a1f90a8cd3dc86dd/source/DFcommon/DFMusic.js#L310
struct HeldNoteTracker
{
    struct HeldNoteInfo
    {
        uint8_t note;
        uint8_t velocity;
        bool isPhysicallyHeld;
        uint32_t timestamp; // micros
    };

    bool mPedalDown = false;
    std::vector<HeldNoteInfo> mHeldNotes; // NEW NOTES get placed at the BACK.

    HeldNoteTracker()
    {
        mHeldNotes.reserve(128);
    }

    void AllNotesOff()
    {
        mHeldNotes.clear();
        mPedalDown = false;
    }
    void PedalUp()
    {
        mPedalDown = false;
        // remove all notes that are not physically held
        std::remove_if(mHeldNotes.begin(), mHeldNotes.end(), [](const HeldNoteInfo &n) { return !n.isPhysicallyHeld; });
        //Serial.println(String("held note tracker PedalUp; count= ") + mHeldNotes.size());
    }

    void PedalDown()
    {
        //Serial.println(String("held note tracker PedalDown"));
        mPedalDown = true;
    }

    void NoteOff(uint8_t note)
    {
        auto existingItem = mHeldNotes.begin();
        for (; existingItem != mHeldNotes.end(); ++existingItem)
        {
            if (existingItem->note == note)
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
            existingItem->isPhysicallyHeld = false;
            // Serial.println(String("held note tracker NoteOff, marking not held. note=") + note +
            //                "; count=" + mHeldNotes.size());
            return;
        }
        // Serial.println(String("held note tracker NoteOff, erasing known note=") + note +
        //                "; count=" + mHeldNotes.size());
        this->mHeldNotes.erase(existingItem);
    }

    void NoteOn(uint8_t note, uint8_t velocity)
    {
        auto existingItem = mHeldNotes.begin();
        for (; existingItem != mHeldNotes.end(); ++existingItem)
        {
            if (existingItem->note == note)
                break;
        }

        if (existingItem == mHeldNotes.end())
        {
            // add new note.
            HeldNoteInfo n;
            n.isPhysicallyHeld = true;
            n.timestamp = micros();
            n.velocity = velocity;
            n.note = note;
            mHeldNotes.push_back(n);
            // Serial.println(String("held note tracker NoteOn, adding note=") + note + "; count=" + mHeldNotes.size());
            return;
        }

        // there's an existing item to deal with.
        HeldNoteInfo n = *existingItem;       // take a copy.
        this->mHeldNotes.erase(existingItem); // remove original
        n.velocity = velocity;
        n.timestamp = micros();
        n.isPhysicallyHeld = true;
        this->mHeldNotes.push_back(n); // add back where it belongs: as newest note.
        // Serial.println(String("held note tracker NoteOn, updating note=") + note + "; count=" + mHeldNotes.size());
    }

    // returns whether the held note is populated (false if no notes playing)
    bool GetLastNoteOn(HeldNoteInfo &outp)
    {
        if (mHeldNotes.empty())
            return false;
        outp = *(mHeldNotes.end() - 1);
        return true;
    }
};

struct USBMidiReader
{
    HeldNoteTracker mHeldNotes;
    uint8_t mCurrentCCValue[256] = {0};

    float mCurrentPitchBendN11 = 0;
    float mCurrentMod01 = 0;

    static USBMidiReader *gInstance;

    USBMidiReader() // : mUsb(), mMidiDevice(mUsb)
    {
        Serial.println(String("USBMidiReader init"));
        CCASSERT(!gInstance);
        gInstance = this;
        gUsbMidi.setHandleNoteOff(HandleNoteOff);
        gUsbMidi.setHandleNoteOn(HandleNoteOn);
        gUsbMidi.setHandleControlChange(HandleControlChange);
        gUsbMidi.setHandlePitchChange(HandlePitchChange);
    }

    static void HandleNoteOff(uint8_t channel, uint8_t note, uint8_t velocity)
    {
        // Serial.println(String("midi device HandleNoteOff: ") + note);
        gInstance->mHeldNotes.NoteOff(note);
    }
    static void HandleNoteOn(uint8_t channel, uint8_t note, uint8_t velocity)
    {
        // Serial.println(String("midi device note on: ") + note);
        gInstance->mHeldNotes.NoteOn(note, velocity);
    }
    static void HandleControlChange(uint8_t channel, uint8_t control, uint8_t value)
    {
        //Serial.println(String("midi device HandleControlChange : ") + control + " -> " + value);
        //   https://anotherproducer.com/online-tools-for-musicians/midi-cc-list/
        //   64	Damper Pedal on/off	≤63 off, ≥64 on	On/off switch that controls sustain pedal. Nearly every synth
        //   will react to CC 64. (See also Sostenuto CC 66)
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
            gInstance->mCurrentPitchBendN11 = ((float)pitch / 8192);
        }
    }

    void Update()
    {
        // auto m1 = micros();
        gUsbHost.Task();
        gUsbMidi.read();
        // auto m2 = micros();
        // Serial.println(String("updating usb midi; ") + int(m2 - m1));
    }
};

USBMidiReader *USBMidiReader::gInstance = nullptr;

// this serves the same function as CCEWIMusicalState, but instead of
// musical state coming from breath control, pitch device, and touch keys,
// the musical state comes from a USB MIDI device.
struct USBMidiMusicalState
{
    IDisplay *mpDisplay;
    AppSettings *mAppSettings;
    InputDelegator *mInput;
    Metronome *mMetronome;
    ScaleFollower *mScaleFollower;
    IInputSource *mInputSrc;
    USBMidiReader mMidiDevice;

    MusicalVoice mLiveVoice; // the voice that's actually playing that represents what the player is playing
                             // (not a harmonized voice or looped voice)
    MusicalVoice
        mMusicalVoices[MAX_MUSICAL_VOICES]; // these are all the voices that WANT to be played (after transpose,
                                            // harmonize, looping, etc). May be more than synth polyphony.
    size_t mVoiceCount = 0;

    LooperAndHarmonizer mLooper;

    int nUpdates = 0;
    int noteOns = 0;

    int mLastPlayedNote = 0; // valid even when mLiveVoice is not.

    USBMidiMusicalState(IDisplay *pDisplay,
                        AppSettings *appSettings,
                        InputDelegator *inputDelegator,
                        Metronome *metronome,
                        ScaleFollower *scaleFollower,
                        IInputSource *inputSrc)
        : mpDisplay(pDisplay), mAppSettings(appSettings), mInput(inputDelegator), mMetronome(metronome),
          mScaleFollower(scaleFollower), mInputSrc(inputSrc), mLooper(appSettings, metronome, scaleFollower)
    {
    }

    void Update()
    {
        mMidiDevice.Update();
        MusicalVoice mNewState(mLiveVoice);

        mNewState.mBreath01 = mMidiDevice.mCurrentMod01;
        mNewState.mPitchBendN11 = mMidiDevice.mCurrentPitchBendN11;

        HeldNoteTracker::HeldNoteInfo heldNote;
        bool isPlaying = mMidiDevice.mHeldNotes.GetLastNoteOn(heldNote);
        mNewState.mVelocity = isPlaying ? heldNote.velocity : 0;
        mNewState.mMidiNote = isPlaying ? heldNote.note : 0;

        auto &perf = mAppSettings->GetCurrentPerformancePatch();
        mNewState.mHarmPatch = perf.mHarmPreset;
        mNewState.mSynthPatchA = perf.mSynthPresetA;
        mNewState.mSynthPatchB = perf.mSynthPresetB;
        mNewState.mPan = 0; // panning is part of musical state because harmonizer needs it per voice, but it's
                            // actually calculated by the synth based on synth preset.

        mNewState.mMidiNote =
            ClampInclusive(mNewState.mMidiNote + mAppSettings->GetCurrentPerformancePatch().mTranspose, 0, 127);

        if (isPlaying)
        {
            mLastPlayedNote = mNewState.mMidiNote;
        }

        auto transitionEvents = CalculateTransitionEvents(mLiveVoice, mNewState);

        if (transitionEvents.mNeedsNoteOn)
        {
            //Serial.println(String("MS note on: ") + mNewState.mMidiNote);
        }
        else if (mNewState.IsPlaying())
        {
            // Serial.println(String("is playing, but transition says no. oldstate.note=") + mLiveVoice.mMidiNote);
        }

        mLiveVoice = mNewState;

        // we have calculated mLiveVoice, converting physical to live musical state.
        // now take the live musical state, and fills out mMusicalVoices based on harmonizer & looper settings.
        size_t newVoiceCount = mLooper.Update(mLiveVoice, transitionEvents, mMusicalVoices, EndPtr(mMusicalVoices));
        mVoiceCount = newVoiceCount;
    }
};

static constexpr auto aoeusnthcphic = sizeof(USBMidiMusicalState);
static constexpr auto aoeucc444 = sizeof(USBMidiReader);

} // namespace clarinoid
