
#pragma once

#include <clarinoid/synth/MusicalVoice.hpp>
#include <clarinoid/synth/MusicalDevice.hpp>

namespace clarinoid
{

// this represents a physical device, plus handles translating physical musical events to events based on synth patch
// voicing modes.
struct USBKeyboardMusicalDevice : ISynthParamProvider, IHeldNoteTrackerEvents, IVoicingModeResultEvents
{
    HeldNoteTracker mHeldNotes;
    VoicingModeInterpreter mVoicingHelper;

    uint8_t mCurrentCCValue[128] = {0};
    float mMacroValues[4] = {0};

    float mCurrentPitchBendN11 = 0;
    float mCurrentMod01 = 0;

    static USBKeyboardMusicalDevice *gInstance;
    AppSettings *mAppSettings;
    IMusicalDeviceEvents *mpEventHandler;
    void *mpCapture = nullptr;

    USBKeyboardMusicalDevice(AppSettings *appSettings, IMusicalDeviceEvents *eventHandler, void *cap)
        : mHeldNotes(&mVoicingHelper), mVoicingHelper(this), mAppSettings(appSettings), mpEventHandler(eventHandler),
          mpCapture(cap)
    {
        // singleton bacause handling events in static methods
        CCASSERT(!gInstance);
        gInstance = this;
        gUsbMidi.setHandleNoteOff(HandleNoteOff);
        gUsbMidi.setHandleNoteOn(HandleNoteOn);
        gUsbMidi.setHandleControlChange(HandleControlChange);
        gUsbMidi.setHandlePitchChange(HandlePitchChange);
    }

    // events coming from held note tracker
    virtual void IHeldNoteTrackerEvents_OnNoteOn(const HeldNoteInfo &noteInfo) override
    {
        mVoicingHelper.IHeldNoteTrackerEvents_OnNoteOn(noteInfo);
        mpEventHandler->IMusicalDeviceEvents_OnPhysicalNoteOn(noteInfo, mpCapture);
    }
    virtual void IHeldNoteTrackerEvents_OnNoteOff(const HeldNoteInfo &noteInfo, const HeldNoteInfo *trillNote) override
    {
        mVoicingHelper.IHeldNoteTrackerEvents_OnNoteOff(noteInfo, trillNote);
        mpEventHandler->IMusicalDeviceEvents_OnPhysicalNoteOff(noteInfo, trillNote, mpCapture);
    }
    virtual void IHeldNoteTrackerEvents_OnAllNotesOff() override
    {
        mVoicingHelper.IHeldNoteTrackerEvents_OnAllNotesOff();
        mpEventHandler->IMusicalDeviceEvents_OnPhysicalAllNotesOff(mpCapture);
    }

    // events coming from voice mode interpreter
    virtual void IVoicingModeResultEvents_OnNoteOn(const HeldNoteInfo &noteInfo) override
    {
        // EMIT live notes to synth.
        MusicalVoice mv;
        mv.mNoteInfo = noteInfo;
        mv.mSource = MusicalEventSource{MusicalEventSourceType::LivePlayA};
        mv.mpParamProvider = this;
        mv.mpPerf = &mAppSettings->GetCurrentPerformancePatch();
        if (mv.mpPerf->mSynthPresetA != -1)
        {
            mv.mpSynthPatch = &mAppSettings->FindSynthPreset(mv.mpPerf->mSynthPresetA);
        }
        mv.mIsActive = mv.mpPerf->mSynthAEnabled && (mv.mpPerf->mSynthPresetA != -1);
        // TODO: collect other things like harmonize emit live note etc.
        if (mv.mIsActive)
        {
            // Serial.println(String("emitting A. enabled:") + mv.mpPerf->mSynthAEnabled +
            //                ", patchID: " + mv.mpPerf->mSynthPresetA + ", pPatch:" + (uintptr_t)mv.mpSynthPatch);
            mpEventHandler->IMusicalDeviceEvents_OnNoteOn(mv, mpCapture);
        }

        // now do live patch B

        mv.mSource = MusicalEventSource{MusicalEventSourceType::LivePlayB};
        if (mv.mpPerf->mSynthPresetB != -1)
        {
            mv.mpSynthPatch = &mAppSettings->FindSynthPreset(mv.mpPerf->mSynthPresetB);
        }
        mv.mIsActive = mv.mpPerf->mSynthBEnabled && (mv.mpPerf->mSynthPresetB != -1);
        // TODO: collect other things like harmonize emit live note etc.
        if (mv.mIsActive)
        {
            // Serial.println(String("emitting B. enabled:") + mv.mpPerf->mSynthBEnabled +
            //                ", patchID: " + mv.mpPerf->mSynthPresetB + ", pPatch:" + (uintptr_t)mv.mpSynthPatch);
            mpEventHandler->IMusicalDeviceEvents_OnNoteOn(mv, mpCapture);
        }
    }

    virtual void IVoicingModeResultEvents_OnNoteOff(const HeldNoteInfo &noteInfo) override
    {
        MusicalVoice mv;
        mv.mNoteInfo = noteInfo;
        mv.mSource = MusicalEventSource{MusicalEventSourceType::LivePlayA};
        mv.mpParamProvider = this;
        mv.mpPerf = &mAppSettings->GetCurrentPerformancePatch();
        if (mv.mpPerf->mSynthPresetA != -1)
        {
            mv.mpSynthPatch = &mAppSettings->FindSynthPreset(mv.mpPerf->mSynthPresetA);
        }
        mv.mIsActive = mv.mpPerf->mSynthAEnabled && (mv.mpPerf->mSynthPresetA != -1);
        // TODO: collect other things like harmonize emit live note etc.
        if (mv.mIsActive)
        {
            // Serial.println(String("note off A. enabled:") + mv.mpPerf->mSynthAEnabled +
            //                ", patchID: " + mv.mpPerf->mSynthPresetA + ", pPatch:" + (uintptr_t)mv.mpSynthPatch);
            mpEventHandler->IMusicalDeviceEvents_OnNoteOff(mv, mpCapture);
        }

        // now do live patch B
        mv.mSource = MusicalEventSource{MusicalEventSourceType::LivePlayB};
        if (mv.mpPerf->mSynthPresetB != -1)
        {
            mv.mpSynthPatch = &mAppSettings->FindSynthPreset(mv.mpPerf->mSynthPresetB);
        }
        mv.mIsActive = mv.mpPerf->mSynthBEnabled && (mv.mpPerf->mSynthPresetB != -1);
        // TODO: collect other things like harmonize emit live note etc.
        if (mv.mIsActive)
        {
            // Serial.println(String("note off B. enabled:") + mv.mpPerf->mSynthBEnabled +
            //                ", patchID: " + mv.mpPerf->mSynthPresetB + ", pPatch:" + (uintptr_t)mv.mpSynthPatch);
            mpEventHandler->IMusicalDeviceEvents_OnNoteOff(mv, mpCapture);
        }
    }
    virtual void IVoicingModeResultEvents_OnAllNotesOff() override
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

    // // similar function to MusicalState::GetLiveMusicalVoice(const MusicalVoice &existing) const
    // const HeldNoteInfo *GetLiveNoteInfo(const HeldNoteInfo &noteInfo) const
    // {
    //     return mHeldNotes.FindExisting(noteInfo.mLiveNoteSequenceID);
    // }

    const MusicalVoice GetLiveMusicalVoice(const MusicalVoice &existing) const
    {
        MusicalVoice mv = existing;
        mv.mpPerf = &mAppSettings->GetCurrentPerformancePatch();
        switch (existing.mSource.mType)
        {
        case MusicalEventSourceType::LivePlayA:
            if (mv.mpPerf->mSynthPresetA != -1)
            {
                mv.mpSynthPatch = &mAppSettings->FindSynthPreset(mv.mpPerf->mSynthPresetA);
            }
            mv.mIsActive = mv.mpPerf->mSynthAEnabled && (mv.mpPerf->mSynthPresetA != -1);
            // TODO: collect other things like harmonize emit live note etc.
            break;
        case MusicalEventSourceType::LivePlayB:
            if (mv.mpPerf->mSynthPresetB != -1)
            {
                mv.mpSynthPatch = &mAppSettings->FindSynthPreset(mv.mpPerf->mSynthPresetB);
            }
            mv.mIsActive = mv.mpPerf->mSynthBEnabled && (mv.mpPerf->mSynthPresetB != -1);
            // TODO: collect other things like harmonize emit live note etc.
            break;
        default:
            CCASSERT(!"didn't expect that source type");
        }

        if (!mv.mIsActive)
        {
            return mv;
        }

        // mv.mpSynthPatch = &mAppSettings->FindSynthPreset(mv.mpPerf->mSynthPresetA);
        const HeldNoteInfo *pCurrentNoteInfo = mHeldNotes.FindExisting(existing.mNoteInfo.mLiveNoteSequenceID);
        mv.mIsActive = !!pCurrentNoteInfo;

        if (!mv.mIsActive)
        {
            return mv;
        }

        mv.mNoteInfo = *pCurrentNoteInfo;
        return mv;
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
