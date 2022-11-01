
// physical device -> heldnotetracker ->(IHeldNoteTrackerEvents)  ........ MusicalState also accepts "physical events" (i.e. NoteInfo, not MusicalVoice) via this interface.
// musical state -> voicing mode interpreter ->(IVoicingModeResultEvents)
// physical device ->(IMusicalDeviceEvents)
// musical state -> apply musical state ->(IMusicalEventsForSynth)
// synth

#pragma once

namespace clarinoid
{

// this is the mostly the same as IMusicalDeviceEvents, but different names, so 1 musical device can do both jobs.
struct IVoicingModeResultEvents
{
    virtual void IVoicingModeResultEvents_OnNoteOn(const HeldNoteInfo &noteInfo) = 0;
    virtual void IVoicingModeResultEvents_OnNoteOff(const HeldNoteInfo &noteInfo) = 0;
    virtual void IVoicingModeResultEvents_OnAllNotesOff() = 0;
};

// musicalstate implements this.
// when notes have been processed by a device, they get sent to the synth engine via this interface.
// it's effectively forwarded directly from musical devices, but with a capture and good naming.
struct IMusicalDeviceEvents
{
    virtual void IMusicalDeviceEvents_OnNoteOn(const MusicalVoice& noteInfo, void* cap) = 0;
    virtual void IMusicalDeviceEvents_OnNoteOff(const MusicalVoice &noteInfo, void* cap) = 0;
    virtual void IMusicalDeviceEvents_OnAllNotesOff(void* cap) = 0;

    // basically a copy-paste of IHeldNoteTrackerEvents
    virtual void IMusicalDeviceEvents_OnPhysicalNoteOn(const HeldNoteInfo &noteInfo, void* cap) = 0;
    virtual void IMusicalDeviceEvents_OnPhysicalNoteOff(const HeldNoteInfo &noteInfo, const HeldNoteInfo *trillNote, void* cap) = 0;
    virtual void IMusicalDeviceEvents_OnPhysicalAllNotesOff(void* cap) = 0;
};

// adapts heldnotetracker events for polyphony / monophony.
// NOTE: for monophony, note offs are NOT generated during legato transitions. callers / synth are expected to handle
// mono legato accordingly.
struct VoicingModeInterpreter : IHeldNoteTrackerEvents
{
    VoicingMode mVoicingMode = VoicingMode::Polyphonic;
    IVoicingModeResultEvents *mpEventHandler = nullptr;
    //void* mpCapture = nullptr;

    VoicingModeInterpreter(IVoicingModeResultEvents *pEventHandler/*, void* cap*/) : mpEventHandler(pEventHandler)//, mpCapture(cap)
    {
    }

    void SetVoicingMode(VoicingMode vm)
    {
        mVoicingMode = vm;
        IHeldNoteTrackerEvents_OnAllNotesOff();
    }

    virtual void IHeldNoteTrackerEvents_OnNoteOn(const HeldNoteInfo &noteInfo) override
    {
        // note ons are always translated to note ons.
        // in monophonic mode, should a note off be sent? i guess the synth knows how to do this.
        mpEventHandler->IVoicingModeResultEvents_OnNoteOn(noteInfo);
    }

    virtual void IHeldNoteTrackerEvents_OnNoteOff(const HeldNoteInfo &noteInfo, const HeldNoteInfo *trillNote) override
    {
        // for monophonic voicing, when you noteoff, a noteon gets generated for the previously-played note (triller
        // behavior)
        if ((mVoicingMode == VoicingMode::Monophonic) && trillNote)
        {
            mpEventHandler->IVoicingModeResultEvents_OnNoteOn(*trillNote);
            return;
        }

        mpEventHandler->IVoicingModeResultEvents_OnNoteOff(noteInfo);
    }

    virtual void IHeldNoteTrackerEvents_OnAllNotesOff() override
    {
        mpEventHandler->IVoicingModeResultEvents_OnAllNotesOff();
    }
};

} // namespace clarinoid
