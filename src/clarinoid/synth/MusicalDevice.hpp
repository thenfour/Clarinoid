
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

// musical state uses this to listen to events from teh korg keyboard.
// usb midi output listens to this to pass to usb midi.
struct IMidiEventHandler
{
    virtual void IMidiEvents_OnNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) = 0;
    virtual void IMidiEvents_OnNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) = 0;
    virtual void IMidiEvents_OnPitchBend(uint8_t channel, int value /*-8192 to 8191*/) = 0;
    virtual void IMidiEvents_OnControlChange(uint8_t channel, uint8_t cc, uint8_t val) = 0;
    virtual void IMidiEvents_OnSysEx(uint32_t length, const uint8_t *data, bool hasTerm=false) = 0;
};

} // namespace clarinoid
