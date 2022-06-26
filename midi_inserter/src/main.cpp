// the old one basically just inserts notes into a midi stream.
// the new one needs to actually interpret data, and forward on.
// A IN gets forwarded to A OUT
// B IN gets forwarded to B OUT
// big button inserts notes on B OUT.
// we intercept messages from A IN for our own device.
// do not attempt to recreate the novation launchpad. the performance app will have to be able to support 1, the other,
// or both, individually. so while we don't want to recreate the launchpad, we do need to tailor our messages to live
// peacefully in the same stream.

// IDENTIFICATION.
// http://midi.teragonaudio.com/tech/midispec/identity.htm
// 1. identification request, per midi spec: 240, 126, 127, 6, 1, 247
//    launchpad reply: 240,    126,      0,          6,     2,      0, 32, 41,       35, 1,      0, 0    [v1, v2, v3,
//    v4]    0xf7
//                     sysex nonrealtime channel    subid subid2    manufacturer     family      model   version end of
//                     sysex
//    midiperformer reply: 240, 126,     0 ,         1,     2,      0, 104, 105,     3, 4,       5, 6    7,8,9,10, 0xf7
// 2. launchpad all other messages: { 240,    0, 32, 41,  2, 14, ..., 0xf7 }
//                                    sysex,   mfg        generic   , end of sysex
//    http://midi.teragonaudio.com/tech/midispec/sysex.htm
//    midiperformer other messages: { 240, 0, 104, 105, [command, params...]..., 0xf7}
//

// command: 0x10: set orange level (0-255)
// command: 0x11: set blue level (0-255)
// command: 0x20+N: set led N color (0-255 R, 0-255 G, 0-255 B)

// BIG BUTTON: sends note 14 (one of the "GO" notes: see LibToollTenfour / new LaunchpadTrigger(KnownTriggers.Go, new
// LaunchpadRegion(new LaunchpadColorSpec("#0fa"), 14, 15),)
//             on MIDI B TX

// sysex identification & commands can happen on both midi ports.
// identification on 1 port will send the response to both.

#include <Arduino.h>
#include <algorithm>
#include <Adafruit_NeoPixel.h>
#include <Bounce.h>
#include <MIDI.h>
#include "misc.hpp"


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
    static const unsigned SysExMaxSize = 512; // big payloads to set all launchpad buttons are like 350 bytes long.

    static const bool UseSenderActiveSensing = false;
    static const bool UseReceiverActiveSensing = false;
    static const uint16_t SenderActiveSensingPeriodicity = 0;
};


midi::SerialMIDI<HardwareSerial> serialMIDIA(Serial1);
midi::MidiInterface<midi::SerialMIDI<HardwareSerial>, SlashKickSettings> MIDIA(serialMIDIA);

midi::SerialMIDI<HardwareSerial> serialMIDIB(Serial3);
midi::MidiInterface<midi::SerialMIDI<HardwareSerial>, SlashKickSettings> MIDIB(serialMIDIB);

constexpr int gWS2812Pin = 10;   // Digital IO pin connected to the NeoPixels.
constexpr int gWS2812Count = 12; // Number of NeoPixels

Adafruit_NeoPixel strip(gWS2812Count, gWS2812Pin, NEO_GRB + NEO_KHZ800);
bool gStripDirty = false;
clarinoid::Stopwatch gGuiTimer; // don't update leds EVERY loop; update max 60fps
static constexpr int gGuiIntervalMS = 1000 / 60;

constexpr uint8_t gSysex_subid1 = 1;
constexpr uint8_t gSysex_subid2 = 2;
constexpr uint8_t gSysex_manufacturer1 = 0;
constexpr uint8_t gSysex_manufacturer2 = 104;
constexpr uint8_t gSysex_manufacturer3 = 105;
constexpr uint8_t gSysex_family1 = 3;
constexpr uint8_t gSysex_family2 = 4;
constexpr uint8_t gSysex_model1 = 5;
constexpr uint8_t gSysex_model2 = 6;
constexpr uint8_t gSysex_version1 = 7;
constexpr uint8_t gSysex_version2 = 8;
constexpr uint8_t gSysex_version3 = 9;
constexpr uint8_t gSysex_version4 = 10;

constexpr byte gMIDIIdentifyRequest[] = {240, 126, 127, 6, 1, 247}; // per midi spec
constexpr byte gMIDIIdentifyResponse[] = {240,
                                          126,
                                          0,
                                          gSysex_subid1,
                                          gSysex_subid2,
                                          gSysex_manufacturer1,
                                          gSysex_manufacturer2,
                                          gSysex_manufacturer3,
                                          gSysex_family1,
                                          gSysex_family2,
                                          gSysex_model1,
                                          gSysex_model2,
                                          gSysex_version1,
                                          gSysex_version2,
                                          gSysex_version3,
                                          gSysex_version4,
                                          0xf7};

constexpr byte gMIDICommandPrefix[] = {240, 0, 104, 105}; //, [command, params...]..., 0xf7};

// command: 0x10: set orange level (0-255)
// command: 0x11: set blue level (0-255)
// command: 0x20+N: set led N color (0-255 R, 0-255 G, 0-255 B)
constexpr byte gMIDICommandSetOrange = 0x10;
constexpr byte gMIDICommandSetBlue = 0x11;
constexpr byte gMIDICommandSetNRGB = 0x20;

BigButtonReader gBigButton{14, 30, 200};

clarinoid::Stopwatch gNoteOffTimer; // when a noteon happens, set a timer to send note off.
bool gWaitingForNoteOff = false;
static constexpr int gNoteOffDelayMS = 50;

LEDPin gLedPin_OrangeTriangle{17, 80, 240, 1.0, 100};
LEDPin gLedPin_BlueTriangle{16, 80, 240, 1.0, 100};
LEDPin gLedPin_RedTriangle{20, 240, 3000, 1.0, 100};
LEDPin gLedPin_MidiA_RX{3, 30, 360, 0.05, 200};
LEDPin gLedPin_MidiB_TX{4, 30, 360, 0.05, 200};
LEDPin gLedPin_MidiB_RX{23, 30, 360, 0.05, 200};
LEDPin gLedPin_MidiA_TX{22, 30, 360, 0.05, 200};

constexpr int gGoNote = 14;

void PresentLeds(bool doOrangeAndBlue = false)
{
    gLedPin_MidiA_RX.Present();
    gLedPin_MidiA_TX.Present();
    gLedPin_MidiB_RX.Present();
    gLedPin_MidiB_TX.Present();
    gLedPin_RedTriangle.Present();

    if (doOrangeAndBlue)
    {
        gLedPin_BlueTriangle.Present();
        gLedPin_OrangeTriangle.Present();
    }

    if (gStripDirty)
    {
        gStripDirty = false;
        strip.show();
    }
}

void DoDemo1()
{
    // startup sequence
    int triFrame = 0;
    int txrxFrame = 0;
    int ws2812Frame = 0;
    clarinoid::Stopwatch swws2812;
    clarinoid::Stopwatch swtriangle;
    clarinoid::Stopwatch swtxrx;
    while (true)
    {
        if (swtriangle.ElapsedTime().ElapsedMillisI() > 120)
        {
            swtriangle.Restart();
            triFrame++;

            if ((triFrame % 3) == 0)
                gLedPin_OrangeTriangle.Trigger();
            if ((triFrame % 3) == 1)
                gLedPin_BlueTriangle.Trigger();
            if ((triFrame % 3) == 2)
                gLedPin_RedTriangle.Trigger();
        }
        if (swtxrx.ElapsedTime().ElapsedMillisI() > 80)
        {
            swtxrx.Restart();
            txrxFrame++;
            if ((txrxFrame % 4) == 0)
                gLedPin_MidiA_RX.Trigger();
            if ((txrxFrame % 4) == 1)
                gLedPin_MidiB_TX.Trigger();
            if ((txrxFrame % 4) == 2)
                gLedPin_MidiB_RX.Trigger();
            if ((txrxFrame % 4) == 3)
                gLedPin_MidiA_TX.Trigger();
        }
        if (swws2812.ElapsedTime().ElapsedMillisI() > 2)
        {
            swws2812.Restart();
            ws2812Frame++;
            if (ws2812Frame > 24 * 7)
                break;
            //  0: first we fill 12 leds with red,
            // 12: then erase them
            // 24: then fill with green
            // 36: erase them
            //
            int shade = (ws2812Frame / 24) % 7; // this is if it's red, green, blue, or otherwise.
            int brightness = 16;
            shade += 1;
            int r = brightness * (shade & 1);
            int g = brightness * ((shade >> 1) & 1);
            int b = brightness * ((shade >> 2) & 1);
            int iled = ws2812Frame % 24; // which led frame within this shade?
            if (iled < 12)
            {
                // fill
                for (int i = 0; i < 12; ++i)
                {
                    if (i > iled)
                    {
                        strip.setPixelColor(i, 0);
                    }
                    else
                    {
                        strip.setPixelColor(i, r, g, b);
                    }
                }
            }
            else
            {
                // erase
                iled -= 12;
                for (int i = 0; i < 12; ++i)
                {
                    if (i < iled)
                    {
                        strip.setPixelColor(i, 0);
                    }
                    else
                    {
                        strip.setPixelColor(i, r, g, b);
                    }
                }
            }
            strip.show();
        }

        PresentLeds(true);
        delay(12);
    }

    gLedPin_OrangeTriangle.SetLevel(0);
    gLedPin_BlueTriangle.SetLevel(0);
    gLedPin_RedTriangle.Trigger();
    gLedPin_MidiA_RX.Trigger();
    gLedPin_MidiB_TX.Trigger();
    gLedPin_MidiB_RX.Trigger();
    gLedPin_MidiA_TX.Trigger();

    for (int i = 0; i < 12; ++i)
    {
        strip.setPixelColor(i, 0);
    }
    strip.show();
    PresentLeds();
}

void setup()
{
    gGuiTimer.Restart();

    strip.begin();
    for (int i = 0; i < gWS2812Count; ++i)
    {
        strip.setPixelColor(i, 0);
    }
    strip.show();

    MIDIA.begin();
    MIDIB.begin();

    DoDemo1();
}

// in order to not conflict with the other device's own identification message.
bool waitingForIdentify = false;
constexpr int identifyWaitMS = 500;
clarinoid::Stopwatch identifyTimer;

// assumes there's a sysex message in the hopper
void HandleIdentify(decltype(MIDIA) &device, const byte *sysexData, const int sysexLen)
{
    if (waitingForIdentify) {
        if (identifyTimer.ElapsedTime().ElapsedMillisI() >= identifyWaitMS) {
            waitingForIdentify = false;
            //MIDIA.sendSysEx(SizeofStaticArray(gMIDIIdentifyResponse), gMIDIIdentifyResponse, true);
            //MIDIB.sendSysEx(SizeofStaticArray(gMIDIIdentifyResponse), gMIDIIdentifyResponse, true);
            gLedPin_MidiA_TX.Trigger();
            gLedPin_MidiB_TX.Trigger();
        }
        return;
    }
    if (!IsArrayEqual(sysexLen, sysexData, gMIDIIdentifyRequest))
    {
        identifyTimer.Restart();
        waitingForIdentify = true;
    }
}

// assumes there's a sysex message in the hopper
void HandleLEDCommand(decltype(MIDIA) &device, const byte *sysexData, int sysexLen)
{
    // const byte *sysexData = device.getSysExArray();
    // int sysexLen = device.getSysExArrayLength();
    if (!ArrayBeginsWith(sysexLen, sysexData, gMIDICommandPrefix))
    {
        // Serial.println("Not a LED command:");
        // Serial.println(byteArrayToString(sysexLen, sysexData));
        return;
    }

    // Serial.println("LED command...");

    // advance past the prefix
    sysexData += SizeofStaticArray(gMIDICommandPrefix);
    sysexLen -= SizeofStaticArray(gMIDICommandPrefix);

    while (true)
    {
        if (sysexLen < 1)
        {
            // Serial.println("LED batch EOF");
            break; // EOF
        }
        else if (sysexData[0] == 0xf7)
        {
            // Serial.println("LED batch EOF 0xf7");
            break; // normal end of msg. though this is not part of the array so should actually never happen.
        }
        else if (sysexData[0] == gMIDICommandSetOrange)
        {
            sysexData++; // advance past command byte
            sysexLen--;
            if (sysexLen < 1)
            {
                gLedPin_RedTriangle.Trigger();
                // Serial.println("Error: sysex data ended in the middle of LEDORANGE command.");
                break;
            }
            // read level
            byte level = sysexData[0];
            // Serial.println(String("LED ORANGE set level ") + (int)level);
            sysexData++; // advance past level byte
            sysexLen--;
            gLedPin_OrangeTriangle.SetLevel(float(level) / 127);
            continue;
        }
        else if (sysexData[0] == gMIDICommandSetBlue)
        {
            sysexData++; // advance past command byte
            sysexLen--;
            if (sysexLen < 1)
            {
                // Serial.println("Error: sysex data ended in the middle of BLUE command.");
                break;
            }
            // read level
            byte level = sysexData[0];
            // Serial.println(String("LED BLUE set level ") + (int)level);
            sysexData++; // advance past level byte
            sysexLen--;
            // perform action.
            gLedPin_BlueTriangle.SetLevel(float(level) / 127);
            continue;
        }
        else
        {
            int nled = sysexData[0];
            nled -= gMIDICommandSetNRGB;
            if (nled < 0 || nled >= gWS2812Count)
            {
                gLedPin_RedTriangle.Trigger();
                // Serial.println(String("ERROR: unknown command."));
                break;
            }
            // Serial.println(String("LED #") + nled);

            nled = gWS2812Count - nled - 1; // the way it's constructed, they're reversed.
            // Serial.println(String(" --> #") + nled);

            // sysexData++;                    // advance past command byte
            // sysexLen--;
            if (sysexLen < 4)
            {
                gLedPin_RedTriangle.Trigger();
                // Serial.println(String("ERROR: MALFORMED."));
                break; // malformed.
            }

            strip.setPixelColor(nled, sysexData[1], sysexData[2], sysexData[3]);
            gStripDirty = true;

            sysexData += 4;
            sysexLen -= 4;
            continue;
        }
        break; // if you get here nothing was processed therefore we don't understand the commend therefore cursor
               // not
               // advancing therefore endless loop therefore omg just stop
    }
}

TriggerLed gtriggerTest{400};

void loop()
{
    // READ MIDI A
    while (true)
    {
        bool activity = false;
        if (MIDIA.read())
        {
            activity = true;
            gLedPin_MidiA_TX.Trigger(); // if there's data, assume it goes through.
            gLedPin_MidiA_RX.Trigger();

            // Serial.println(String("MIDI A input: ") + (int)MIDIA.getType());

            if (MIDIA.getType() == midi::SystemExclusive)
            {
                const byte *sysexData = MIDIA.getSysExArray();
                const int sysexLen = MIDIA.getSysExArrayLength();
                HandleIdentify(MIDIA, sysexData, sysexLen);
                HandleLEDCommand(MIDIA, sysexData, sysexLen);
            }
        }

        // READ MIDI B
        if (MIDIB.read())
        {
            activity = true;
            gLedPin_MidiB_RX.Trigger();
            gLedPin_MidiB_TX.Trigger();

            // Serial.println(String("MIDI B input: ") + (int)MIDIB.getType());
            if (MIDIB.getType() == midi::SystemExclusive)
            {
                const byte *sysexData = MIDIB.getSysExArray();
                const int sysexLen = MIDIB.getSysExArrayLength();
                HandleIdentify(MIDIB, sysexData, sysexLen);
                HandleLEDCommand(MIDIB, sysexData, sysexLen);
            }
        }

        if (!activity)
            break;
    }

    // HANDLE BIG BUTTON NOTE INSERTION & SUBSEQUENT NOTE-OFF
    if (gBigButton.DidTrigger())
    {
        gLedPin_RedTriangle.Trigger();
        MIDIB.sendNoteOn(gGoNote, 126, 1);
        gLedPin_MidiB_TX.Trigger();
        // MIDIA.sendNoteOn(gGoNote, 126, 1);
        gNoteOffTimer.Restart();
        gWaitingForNoteOff = true;
        
        //Serial.println("you hit the big button");
        // gtriggerTest.Trigger();
    }
    if (gWaitingForNoteOff && gNoteOffTimer.ElapsedTime().ElapsedMillisI() >= gNoteOffDelayMS)
    {
        MIDIB.sendNoteOff(gGoNote, 0, 1);
        gWaitingForNoteOff = false;
        gLedPin_MidiB_TX.Trigger();
    }

    if (gGuiTimer.ElapsedTime().ElapsedMillisI() >= gGuiIntervalMS)
    {
        PresentLeds();
    }
}
