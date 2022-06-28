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

// WS2812 leds
// 0-7 are free for callers to program
// 8 = ??
// 9 = ??
// 10 = identify status
// 11 = heartbeat

// TODO: long press = test + "screen saver" mode.

#include <Arduino.h>
#include <algorithm>
#include <Adafruit_NeoPixel.h>
#include <Bounce.h>
#include <MIDI.h>
#include "misc.hpp"
#include "midibus.hpp"
#include "anim.hpp"

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

constexpr int gGoNote = 14;

void ErrorCallbackA(int8_t n);
void ErrorCallbackB(int8_t n);

MidiBus midiBusA(Serial1, ErrorCallbackA);
MidiBus midiBusB(Serial1, ErrorCallbackB);

void ErrorCallbackA(int8_t n)
{
    midiBusA.ResetBus();
}

void ErrorCallbackA(int8_t n)
{
    midiBusA.ResetBus();
}

void setup()
{
    DoDemo1();
}

// in order to not conflict with the other device's own identification message.
bool waitingForIdentify = false;
constexpr int identifyWaitMS = 500;
clarinoid::Stopwatch identifyTimer;

// assumes there's a sysex message in the hopper
void HandleIdentify(MidiBus &midiBus, const byte *sysexData, const int sysexLen)
{
    if (IsArrayEqual(sysexLen, sysexData, gMIDIIdentifyRequest))
    {
        gLedstrip.SetIdentifyState(IdentifyState::Requested);
        identifyTimer.Restart();
        waitingForIdentify = true;
    }
}

// assumes there's a sysex message in the hopper
void HandleLEDCommand(MidiBus &midiBus, const byte *sysexData, int sysexLen)
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
            if (nled < 0 || nled >= Ledstrip::gWS2812Count) // this actually gives callers the possibility of setting
                                                            // our built-in LEDs as well; no worries
            {
                gLedPin_RedTriangle.Trigger();
                // Serial.println(String("ERROR: unknown command."));
                break;
            }
            // Serial.println(String("LED #") + nled);

            nled = Ledstrip::gWS2812Count - nled - 1; // the way it's constructed, they're reversed.
            // Serial.println(String(" --> #") + nled);

            // sysexData++;                    // advance past command byte
            // sysexLen--;
            if (sysexLen < 4)
            {
                gLedPin_RedTriangle.Trigger();
                // Serial.println(String("ERROR: MALFORMED."));
                break; // malformed.
            }

            gLedstrip.setPixelColor(nled, sysexData[1], sysexData[2], sysexData[3]);

            sysexData += 4;
            sysexLen -= 4;
            continue;
        }
        break; // if you get here nothing was processed therefore we don't understand the commend therefore cursor
               // not
               // advancing therefore endless loop therefore omg just stop
    }
}

// return true if there's any activity
bool ProcessMIDIBusActivity(MidiBus &bus, LEDPin &txLed, LEDPin &rxLed)
{
    if (!bus.mMIDI.read())
        return false;

    txLed.Trigger(); // if there's data, assume it goes thru.
    rxLed.Trigger();

    if (bus.mMIDI.getType() != midi::SystemExclusive)
        return true;
    const byte *sysexData = bus.mMIDI.getSysExArray();
    const int sysexLen = bus.mMIDI.getSysExArrayLength();
    HandleIdentify(bus, sysexData, sysexLen);
    HandleLEDCommand(bus, sysexData, sysexLen);
    return true;
}

void loop()
{
    bool activityA = false;
    bool activityB = false;
    do
    {
        // READ MIDI A
        activityA = ProcessMIDIBusActivity(midiBusA, gLedPin_MidiA_TX, gLedPin_MidiA_RX);
        activityB = ProcessMIDIBusActivity(midiBusB, gLedPin_MidiB_TX, gLedPin_MidiB_RX);
    } while (activityA || activityB);

    if (waitingForIdentify)
    {
        if (identifyTimer.ElapsedTime().ElapsedMillisI() >= identifyWaitMS)
        {
            gLedstrip.SetIdentifyState(IdentifyState::Sent);
            waitingForIdentify = false;
            midiBusA.mMIDI.sendSysEx(SizeofStaticArray(gMIDIIdentifyResponse), gMIDIIdentifyResponse, true);
            midiBusB.mMIDI.sendSysEx(SizeofStaticArray(gMIDIIdentifyResponse), gMIDIIdentifyResponse, true);
            gLedPin_MidiA_TX.Trigger();
            gLedPin_MidiB_TX.Trigger();
        }
    }

    // HANDLE BIG BUTTON NOTE INSERTION & SUBSEQUENT NOTE-OFF
    if (gBigButton.DidTrigger())
    {
        gLedPin_RedTriangle.Trigger();
        midiBusB.mMIDI.sendNoteOn(gGoNote, 126, 1);
        gLedPin_MidiB_TX.Trigger();
        gNoteOffTimer.Restart();
        gWaitingForNoteOff = true;
    }
    if (gWaitingForNoteOff && gNoteOffTimer.ElapsedTime().ElapsedMillisI() >= gNoteOffDelayMS)
    {
        midiBusB.mMIDI.sendNoteOff(gGoNote, 0, 1);
        gLedPin_MidiB_TX.Trigger();
        gWaitingForNoteOff = false;
    }

    delayMicroseconds(50);
}
