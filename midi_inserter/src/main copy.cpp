
#include <Arduino.h>
#include <algorithm>
//#include <Adafruit_NeoPixel.h>
//#include <Bounce.h>
#include <MIDI.h>
//#include "misc.hpp"

String byteArrayToString(int len, const byte *data)
{
    String ret{"["};
    for (int i = 0; i < len; ++i)
    {
        ret += String((int)data[i], 16);
        ret += " ";
    }
    ret += "]";
    return ret;
}

static constexpr size_t MaxBufferSize = 8192;

struct SlashKickSettings
{
    /*! Running status enables short messages when sending multiple values
    of the same type and channel.\n
    Must be disabled to send USB MIDI messages to a computer
    Warning: does not work with some hardware, enable with caution.
    */
    static const bool UseRunningStatus = false;

    /*! NoteOn with 0 velocity should be handled as NoteOf.\n
    Set to true  to get NoteOff events when receiving null-velocity NoteOn messages.\n
    Set to false to get NoteOn  events when receiving null-velocity NoteOn messages.
    */
    static const bool HandleNullVelocityNoteOnAsNoteOff = true;

    /*! Setting this to true will make MIDI.read parse only one byte of data for each
    call when data is available. This can speed up your application if receiving
    a lot of traffic, but might induce MIDI Thru and treatment latency.
    */
    static const bool Use1ByteParsing = true;

    /*! Maximum size of SysEx receivable. Decrease to save RAM if you don't expect
    to receive SysEx, or adjust accordingly.
    */
    static const unsigned SysExMaxSize = MaxBufferSize; // big payloads to set all launchpad buttons are like 350 bytes long.

    static const bool UseSenderActiveSensing = false;
    static const bool UseReceiverActiveSensing = false;
    static const uint16_t SenderActiveSensingPeriodicity = 0;
};

midi::SerialMIDI<HardwareSerial> serialMIDIB(Serial3);
midi::MidiInterface<midi::SerialMIDI<HardwareSerial>, SlashKickSettings> MIDIB(serialMIDIB);

void ErrorCallback(int8_t n)
{
        String s = "MIDI ERROR, code: ";
        s += n;
        Serial.println(s);

        new (&serialMIDIB) midi::SerialMIDI<HardwareSerial>(Serial3);
        new (&MIDIB) midi::MidiInterface<midi::SerialMIDI<HardwareSerial>, SlashKickSettings>(serialMIDIB);
        MIDIB.begin();
        MIDIB.setHandleError(ErrorCallback);
}

uint8_t buf[MaxBufferSize] = {0};

void setup()
{
    Serial3.addMemoryForRead(buf, MaxBufferSize);
    Serial.begin(9600);
    MIDIB.begin();
    MIDIB.setHandleError(ErrorCallback);
}


void loop()
{
    //Serial3.clearReadError();
    // READ MIDI B
    if (MIDIB.read())
    {
        String s = "Sysex RX, len: ";
        s += MIDIB.getSysExArrayLength();
        Serial.println(s);

        //Serial.println(String(" -> ") + byteArrayToString(MIDIB.getSysExArrayLength(), MIDIB.getSysExArray()));
    }
    if (int err = Serial3.getReadError()) {
        String s = "Serial3 read error: ";
        s += err;
        Serial.println(s);
    }
    delay(1);
}
