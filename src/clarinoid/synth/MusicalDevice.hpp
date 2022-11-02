
// physical device -> heldnotetracker ->(IHeldNoteTrackerEvents)  ........ MusicalState also accepts "physical events" (i.e. NoteInfo, not MusicalVoice) via this interface.
// musical state -> voicing mode interpreter ->(IVoicingModeResultEvents)
// physical device ->(IMusicalDeviceEvents)
// musical state -> apply musical state ->(IMusicalEventsForSynth)
// synth

#pragma once

namespace clarinoid
{
// musicalstate implements this.
// when notes have been processed by a device, they get sent to the synth engine via this interface.
// it's effectively forwarded directly from musical devices, but with a capture and good naming.
struct IMusicalDeviceEvents
{
    virtual void IMusicalDeviceEvents_OnDeviceNoteOn(const MusicalVoice& noteInfo, void* cap) = 0;
    virtual void IMusicalDeviceEvents_OnDeviceNoteOff(const MusicalVoice &noteInfo, void* cap) = 0;
    virtual void IMusicalDeviceEvents_OnDeviceAllNotesOff(const MusicalEventSource &source, void* cap) = 0;

    // basically a copy-paste of IHeldNoteTrackerEvents
    virtual void IMusicalDeviceEvents_OnPhysicalNoteOn(const HeldNoteInfo &noteInfo, void* cap) = 0;
    virtual void IMusicalDeviceEvents_OnPhysicalNoteOff(const HeldNoteInfo &noteInfo, const HeldNoteInfo *trillNote, void* cap) = 0;
    virtual void IMusicalDeviceEvents_OnPhysicalAllNotesOff(void* cap) = 0;
};

} // namespace clarinoid
