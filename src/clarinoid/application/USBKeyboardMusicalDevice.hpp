
#pragma once

#include <clarinoid/synth/MusicalVoice.hpp>
#include <clarinoid/synth/MusicalDevice.hpp>

namespace clarinoid
{

struct USBKeyboardMusicalDevice : ISynthParamProvider, IHeldNoteTrackerEvents
{
    HeldNoteTracker mHeldNotes;
    uint8_t mCurrentCCValue[128] = {0};
    float mMacroValues[4] = {0};

    float mCurrentPitchBendN11 = 0;
    float mCurrentMod01 = 0;

    static USBKeyboardMusicalDevice *gInstance;
    AppSettings *mAppSettings;
    IMusicalDeviceEvents *mpEventHandler;
    void* mpCapture = nullptr;

    // bool isPhysicallyPressed(uint32_t liveNoteSequenceID) const
    // {
    //     return mHeldNotes.isPhysicallyPressed(liveNoteSequenceID);
    // }

    USBKeyboardMusicalDevice(AppSettings *appSettings, IMusicalDeviceEvents *eventHandler, void* cap)
        : mHeldNotes(this), mAppSettings(appSettings), mpEventHandler(eventHandler), mpCapture(cap)
    {
        // singleton bacause handling events in static methods
        CCASSERT(!gInstance);
        gInstance = this;
        gUsbMidi.setHandleNoteOff(HandleNoteOff);
        gUsbMidi.setHandleNoteOn(HandleNoteOn);
        gUsbMidi.setHandleControlChange(HandleControlChange);
        gUsbMidi.setHandlePitchChange(HandlePitchChange);
    }

    virtual void IHeldNoteTrackerEvents_OnNoteOn(const HeldNoteInfo &noteInfo) override
    {
        mpEventHandler->IMusicalDeviceEvents_OnNoteOn(noteInfo, mpCapture);
    }
    virtual void IHeldNoteTrackerEvents_OnNoteOff(const HeldNoteInfo &noteInfo, const HeldNoteInfo *trillNote) override
    {
        mpEventHandler->IMusicalDeviceEvents_OnNoteOff(noteInfo, trillNote, mpCapture);
    }
    virtual void IHeldNoteTrackerEvents_OnAllNotesOff() override
    {
        mpEventHandler->IMusicalDeviceEvents_OnAllNotesOff(mpCapture);
    }

    virtual uint8_t SynthParamProvider_GetMidiCC(MidiCCValue cc) override
    {
        return mCurrentCCValue[(int)cc];
    }
    virtual float SynthParamProvider_GetBreath01() override
    {
        return 0;
    }
    virtual float SynthParamProvider_GetPitchBendN11() override
    {
        return mCurrentPitchBendN11;
    }
    virtual float SynthParamProvider_GetMacroValue01(size_t i) override
    {
        CCASSERT(i < SizeofStaticArray(mMacroValues));
        return mMacroValues[i];
    }

    // similar function to MusicalState::GetLiveMusicalVoice(const MusicalVoice &existing) const
    const HeldNoteInfo* GetLiveNoteInfo(const HeldNoteInfo& noteInfo) const {
        return mHeldNotes.FindExisting(noteInfo.mLiveNoteSequenceID);
    }

    struct DupeEventChecker
    {
        static constexpr uint32_t dupeEventBoundaryMS = 2;
        int lastA = -1; // hope these are acceptable initial values
        int lastB = -1;
        Stopwatch lastTime;
        bool RegisterEventAndCheckDupe(int A, int B)
        {
            if ((lastA == A) && (lastB == B) && (lastTime.ElapsedTime().ElapsedMillisI() < dupeEventBoundaryMS))
            {
                return true;
            }
            lastA = A;
            lastB = B;
            lastTime.Restart();
            return false;
        }
    };

    static void HandleNoteOff(uint8_t channel, uint8_t note, uint8_t velocity)
    {
        static DupeEventChecker gDupeChecker;
        if (gDupeChecker.RegisterEventAndCheckDupe(note, velocity))
        {
            return;
        }
        // Serial.println(String("midi device HandleNoteOff: ") + note);
        gInstance->mHeldNotes.NoteOff(MidiNote(note));
    }

    static void HandleNoteOn(uint8_t channel, uint8_t note, uint8_t velocity)
    {
        static DupeEventChecker gDupeChecker;
        if (gDupeChecker.RegisterEventAndCheckDupe(note, velocity))
        {
            return;
        }
        // Serial.println(String("midi device note on: ") + note);
        float velocity01 = float(velocity) / 127.0f;
        gInstance->mHeldNotes.NoteOn(MidiNote(note), velocity01);
    }

    static void HandleControlChange(uint8_t channel, uint8_t control, uint8_t value)
    {
        static DupeEventChecker gDupeChecker;
        if (gDupeChecker.RegisterEventAndCheckDupe(control, value))
        {
            return;
        }
        // Serial.println(String("midi device HandleControlChange : ") + control + " -> " + value);
        //      https://anotherproducer.com/online-tools-for-musicians/midi-cc-list/
        //      64	Damper Pedal on/off	≤63 off, ≥64 on	On/off switch that controls sustain pedal. Nearly every synth
        //      will react to CC 64. (See also Sostenuto CC 66)
        if (control == (int)MidiCCValue::DamperPedal)
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
        else if (control == (int)MidiCCValue::AllNotesOff)
        {
            // all notes off
            gInstance->mHeldNotes.AllNotesOff();
        }
        gInstance->mCurrentCCValue[control] = value;
    }

    static void HandlePitchChange(uint8_t channel, int pitch /* -8192 to 8191 inclusive */)
    {
        static DupeEventChecker gDupeChecker;
        if (gDupeChecker.RegisterEventAndCheckDupe(pitch, 0))
        {
            return;
        }
        // Serial.println(String("pb: ") + pitch);
        if (pitch >= 8191)
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
        gUsbHost.Task();
        gUsbMidi.read();
    }
};

USBKeyboardMusicalDevice *USBKeyboardMusicalDevice::gInstance = nullptr;

} // namespace clarinoid
