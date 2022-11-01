
// physical device -> heldnotetracker ->(IHeldNoteTrackerEvents)
// physical device ->(IMusicalDeviceEvents)
// musical state -> voicing mode interpreter ->(IMusicalDeviceEvents)
// musical state -> apply musical state ->(IMusicalEvents)
// synth

#pragma once

namespace clarinoid
{

// musicalstate implements this.
// when notes have been processed by a device, they get sent to the synth engine via this interface.
// it's effectively forwarded directly from musical devices, but with a capture and good naming.
struct IMusicalDeviceEvents
{
    virtual void IMusicalDeviceEvents_OnNoteOn(const HeldNoteInfo& noteInfo, void* cap) = 0;
    virtual void IMusicalDeviceEvents_OnNoteOff(const HeldNoteInfo &noteInfo, const HeldNoteInfo *trillNote, void* cap) = 0;
    virtual void IMusicalDeviceEvents_OnAllNotesOff(void* cap) = 0;
};

// adapts heldnotetracker events for polyphony / monophony.
// NOTE: for monophony, note offs are NOT generated during legato transitions. callers / synth are expected to handle
// mono legato accordingly.
struct VoicingModeInterpreter : IHeldNoteTrackerEvents
{
    VoicingMode mVoicingMode = VoicingMode::Polyphonic;
    IMusicalDeviceEvents *mpEventHandler = nullptr;
    void* mpCapture = nullptr;

    VoicingModeInterpreter(IMusicalDeviceEvents *pEventHandler, void* cap) : mpEventHandler(pEventHandler), mpCapture(cap)
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
        mpEventHandler->IMusicalDeviceEvents_OnNoteOn(noteInfo, mpCapture);
    }

    virtual void IHeldNoteTrackerEvents_OnNoteOff(const HeldNoteInfo &noteInfo, const HeldNoteInfo *trillNote) override
    {
        // for monophonic voicing, when you noteoff, a noteon gets generated for the previously-played note (triller
        // behavior)
        if ((mVoicingMode == VoicingMode::Monophonic) && trillNote)
        {
            mpEventHandler->IMusicalDeviceEvents_OnNoteOn(*trillNote, mpCapture);
            return;
        }

        mpEventHandler->IMusicalDeviceEvents_OnNoteOff(noteInfo, nullptr, mpCapture);
    }

    virtual void IHeldNoteTrackerEvents_OnAllNotesOff() override
    {
        mpEventHandler->IMusicalDeviceEvents_OnAllNotesOff(mpCapture);
    }
};

} // namespace clarinoid
