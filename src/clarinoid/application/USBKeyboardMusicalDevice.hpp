// held notes is a strange concept.
// there are many different definitions & usages for such a struct.
// and i think eventually they'll have to be turned into something more global.
// - note because user is physically pressing the key <-- tracked in heldnotetracker.mPhysicallyPressed
// - note because user is holding the damper pedal <-- tracked in heldnotetracker list
// - note because voicing mode allows it (think monophony where 1 note is playing only) <-- not tracked. we just send
// notes to the synth and it decides to accept or not.
// - note because the synth is playing it <-- synth voice has a running voice.

#pragma once

#include <clarinoid/synth/MusicalVoice.hpp>
#include <clarinoid/synth/MusicalDevice.hpp>

namespace clarinoid
{

// this represents a physical device, plus handles translating physical musical events to events based on synth patch
// voicing modes.
struct USBKeyboardMusicalDevice : ISynthParamProvider, IHeldNoteTrackerEvents<HeldNoteInfo>
{
    HeldNoteTracker mHeldNotes;

    uint8_t mCurrentCCValue[128] = {0};
    float mMacroValues[4] = {0};

    float mCurrentPitchBendN11 = 0;
    float mCurrentMod01 = 0;

    static USBKeyboardMusicalDevice *gInstance;
    AppSettings *mAppSettings;
    IMusicalDeviceEvents *mpEventHandler;
    void *mpCapture = nullptr;

    // when voicing mode changes we should send note offs.
    VoicingMode mLastVoicingModeA = VoicingMode::Polyphonic;
    VoicingMode mLastVoicingModeB = VoicingMode::Polyphonic;

    USBKeyboardMusicalDevice(AppSettings *appSettings, IMusicalDeviceEvents *eventHandler, void *cap)
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

    // events coming from held note tracker
    virtual void IHeldNoteTrackerEvents_OnNoteOn(const HeldNoteInfo &noteInfo) override
    {
        //Serial.println("USBKeyboardMusicalDevice::IHeldNoteTrackerEvents_OnNoteOn");
        mpEventHandler->IMusicalDeviceEvents_OnPhysicalNoteOn(noteInfo, mpCapture);

        // EMIT live notes to synth.
        const auto &perf = mAppSettings->GetCurrentPerformancePatch();
        DoNoteOnForPerfVoice(
            noteInfo, MusicalEventSourceType::LivePlayA, perf, perf.mSynthPresetA, perf.mSynthAEnabled);
        DoNoteOnForPerfVoice(
            noteInfo, MusicalEventSourceType::LivePlayB, perf, perf.mSynthPresetB, perf.mSynthBEnabled);
    }

    void DoNoteOnForPerfVoice(const HeldNoteInfo &noteInfo,
                              MusicalEventSourceType source,
                              const PerformancePatch &perf,
                              int16_t synthPatchId,
                              bool perfSynthPatchEnabled)
    {
        MusicalVoice mv;
        mv.mNoteInfo = noteInfo;
        mv.mSource = MusicalEventSource{source};
        mv.mpParamProvider = this;
        //mv.mpPerf = &perf;
        bool isValidPatch = mAppSettings->IsValidSynthPatchId(synthPatchId);
        if (isValidPatch)
        {
            mv.mpSynthPatch = &mAppSettings->FindSynthPreset(synthPatchId);
        }
        mv.mIsActive = isValidPatch && perfSynthPatchEnabled;
        if (mv.mIsActive)
        {
            // Serial.println(String("emitting A. enabled:") + mv.mpPerf->mSynthAEnabled +
            //                ", patchID: " + mv.mpPerf->mSynthPresetA + ", pPatch:" + (uintptr_t)mv.mpSynthPatch);
            mpEventHandler->IMusicalDeviceEvents_OnDeviceNoteOn(mv, mpCapture);
        }
    }

    virtual void IHeldNoteTrackerEvents_OnNoteOff(const HeldNoteInfo &noteInfo, const HeldNoteInfo *trillNote) override
    {
        // mVoicingHelper.IHeldNoteTrackerEvents_OnNoteOff(noteInfo, trillNote);
        mpEventHandler->IMusicalDeviceEvents_OnPhysicalNoteOff(noteInfo, trillNote, mpCapture);

        const auto &perf = mAppSettings->GetCurrentPerformancePatch();
        DoNoteOffForPerfVoice(
            noteInfo, trillNote, MusicalEventSourceType::LivePlayA, perf, perf.mSynthPresetA, perf.mSynthAEnabled);
        DoNoteOffForPerfVoice(
            noteInfo, trillNote, MusicalEventSourceType::LivePlayB, perf, perf.mSynthPresetB, perf.mSynthBEnabled);
    }

    void DoNoteOffForPerfVoice(const HeldNoteInfo &noteInfo,
                               const HeldNoteInfo *trillNote,
                               MusicalEventSourceType source,
                               const PerformancePatch &perf,
                               int16_t synthPatchId,
                               bool perfSynthPatchEnabled)
    {
        MusicalVoice mv;
        bool synthPatchValid = mAppSettings->IsValidSynthPatchId(synthPatchId);
        if (synthPatchValid)
        {
            mv.mpSynthPatch = &mAppSettings->FindSynthPreset(synthPatchId);
        }

        if (mv.mpSynthPatch->mVoicingMode == VoicingMode::Monophonic && !!trillNote)
        {
            // play trill note instead.
            DoNoteOnForPerfVoice(*trillNote, source, perf, synthPatchId, perfSynthPatchEnabled);
            return;
        }

        // proceed with note off.
        mv.mNoteInfo = noteInfo;
        mv.mSource = MusicalEventSource{source};
        mv.mpParamProvider = this;
        //mv.mpPerf = &perf;
        mv.mIsActive = perfSynthPatchEnabled && synthPatchValid;
        if (mv.mIsActive)
        {
            // Serial.println(String("note off A. enabled:") + mv.mpPerf->mSynthAEnabled +
            //                ", patchID: " + mv.mpPerf->mSynthPresetA + ", pPatch:" + (uintptr_t)mv.mpSynthPatch);
            mpEventHandler->IMusicalDeviceEvents_OnDeviceNoteOff(mv, mpCapture);
        }
    }

    // virtual void IHeldNoteTrackerEvents_OnAllNotesOff() override
    // {
    //     mpEventHandler->IMusicalDeviceEvents_OnDeviceAllNotesOff(mpCapture);
    //     mpEventHandler->IMusicalDeviceEvents_OnPhysicalAllNotesOff(mpCapture);
    // }

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

    const MusicalVoice GetLiveMusicalVoice_ForPerfVoice(const MusicalVoice &existing,
                                                        MusicalEventSourceType source,
                                                        const PerformancePatch &perf,
                                                        int16_t synthPatchId,
                                                        bool perfSynthPatchEnabled,
                                                        VoicingMode& cachedVoicingMode)
    {
        MusicalVoice mv = existing;
        //mv.mpPerf = &perf;
        bool validSynthPatch = mAppSettings->IsValidSynthPatchId(synthPatchId);
        if (validSynthPatch)
        {
            mv.mpSynthPatch = &mAppSettings->FindSynthPreset(synthPatchId);

            // if the voicing mode has changed, send all notes off for this source
            if (mv.mpSynthPatch->mVoicingMode != cachedVoicingMode) {
                mpEventHandler->IMusicalDeviceEvents_OnDeviceAllNotesOff(MusicalEventSource{source}, mpCapture);
                cachedVoicingMode = mv.mpSynthPatch->mVoicingMode;
            }
        }
        mv.mIsActive = perfSynthPatchEnabled && validSynthPatch;
        if (!mv.mIsActive)
        {
            return mv;
        }

        const HeldNoteInfo *pCurrentNoteInfo = mHeldNotes.FindExisting(existing.mNoteInfo.mLiveNoteSequenceID);
        mv.mIsActive = !!pCurrentNoteInfo;
        if (!mv.mIsActive)
        {
            return mv;
        }

        mv.mNoteInfo = *pCurrentNoteInfo;
        return mv;
    }

    const MusicalVoice GetLiveMusicalVoice(const MusicalVoice &existing)
    {
        const auto &perf = mAppSettings->GetCurrentPerformancePatch();
        switch (existing.mSource.mType)
        {
        case MusicalEventSourceType::LivePlayA:
            return GetLiveMusicalVoice_ForPerfVoice(existing, MusicalEventSourceType::LivePlayA, perf, perf.mSynthPresetA, perf.mSynthAEnabled, mLastVoicingModeA);
        case MusicalEventSourceType::LivePlayB:
            return GetLiveMusicalVoice_ForPerfVoice(existing, MusicalEventSourceType::LivePlayA, perf, perf.mSynthPresetB, perf.mSynthBEnabled, mLastVoicingModeB);
        default:
            CCASSERT(!"didn't expect that source type");
        }
        // unreachable
        return MusicalVoice{};
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
        //Serial.println("USBKeyboardMusicalDevice::HandleNoteOn");
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
