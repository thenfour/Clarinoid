
#pragma once


namespace clarinoid
{

struct USBMidiOutput : IMidiEventHandler
{
    USBMidiOutput()
    {
        usbMIDI.begin();
    }
    void Update()
    {
    }

    virtual void IMidiEvents_OnNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) override
    {
        //Serial.println(String("IMidiEvents_OnNoteOn ") + note + " " + velocity);
        usbMIDI.sendNoteOn(note, velocity, channel);
    }
    virtual void IMidiEvents_OnNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) override
    {
        //Serial.println(String("IMidiEvents_OnNoteOff ") + note + " " + velocity);
        usbMIDI.sendNoteOff(note, velocity, channel);
    }
    virtual void IMidiEvents_OnPitchBend(uint8_t channel, int value /*-8192 to 8191*/) override
    {
        //Serial.println(String("IMidiEvents_OnPitchBend ") + value);
        usbMIDI.sendPitchBend(value, channel);
    }
    virtual void IMidiEvents_OnControlChange(uint8_t channel, uint8_t cc, uint8_t val) override
    {
        //Serial.println(String("IMidiEvents_OnControlChange ") + cc + " " + val);
        usbMIDI.sendControlChange(cc, val, channel);
    }
    virtual void IMidiEvents_OnSysEx(uint32_t length, const uint8_t *data, bool hasTerm = false) override
    {
        // for the moment don't do anything. korg will never send sysex anyway.
    }
};

struct USBMidiInput
{
    USBMidiInput()
    {
        //
    }
    void Update()
    {
        // MIDI Controllers should discard incoming MIDI messages.
        // http://forum.pjrc.com/threads/24179-Teensy-3-Ableton-Analog-CC-causes-midi-crash
        while (usbMIDI.read())
        {
            // ignore incoming messages
        }
    }
};

} // namespace clarinoid