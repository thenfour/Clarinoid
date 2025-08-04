
#include <Arduino.h>
#include <algorithm>
#include <MIDI.h>
#include <Bounce.h>
#include <cmath>
#include <WS2812Serial.h>

// setting analog resolution causes other problems and ugliness. don't.
// see https://www.pjrc.com/teensy/td_pulse.html
// on how to balance these values.
constexpr float gLEDPWMFrequency = 36621.09f;
constexpr int gLEDPWMResolution = 12;

constexpr int gLEDPWMMaxValue = (1 << gLEDPWMResolution) - 1;

constexpr int gFrameRate = 125;

#include "midibus.hpp"
#include "gui.hpp"
#include "misc.hpp"

// command: 0x10: set orange level (0-255)
// command: 0x11: set blue level (0-255)
// command: 0x20+N: set led N color (0-255 R, 0-255 G, 0-255 B)
constexpr byte gMIDICommandSetOrange = 0x10;
constexpr byte gMIDICommandSetBlue = 0x11;
constexpr byte gMIDICommandSetNRGB = 0x20;
constexpr byte gMIDICommandSetNRGBCount = 8;
constexpr byte gMIDICommandSimulateError = 0x30;
constexpr byte gMIDICommandBeatTrigger = 0x40;

clarinoid::Stopwatch gNoteOffTimer; // when a noteon happens, set a timer to send note off.
bool gWaitingForNoteOff = false;
static constexpr int gNoteOffDelayMS = 50;

constexpr int gGoNote = 14;

BigButtonReader gBigButton{16, 30, 200};

MidiPerformerGui gGui;

void ErrorCallbackA(int8_t n);
void ErrorCallbackB(int8_t n);

MidiBus gMidiBusA(Serial1, ErrorCallbackA);
MidiBus gMidiBusB(Serial2, ErrorCallbackB);

void ErrorCallbackA(int8_t n)
{
    gMidiBusA.ResetBus();
}

void ErrorCallbackB(int8_t n)
{
    gMidiBusB.ResetBus();
}

void setup()
{
    Serial.begin(9600);
    // delay(250);
    // while (!Serial);
    // Serial.println("beginning.");
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
        gGui.SetIdentifyState(IdentifyState::Requested);
        identifyTimer.Restart();
        waitingForIdentify = true;
    }
}

// assumes there's a sysex message in the hopper
void HandleLEDCommand(MidiBus &midiBus, const byte *sysexData, int sysexLen)
{
    if (!ArrayBeginsWith(sysexLen, sysexData, gMIDICommandPrefix))
    {
        return;
    }

    // advance past the prefix
    sysexData += SizeofStaticArray(gMIDICommandPrefix);
    sysexLen -= SizeofStaticArray(gMIDICommandPrefix);

    while (true)
    {
        if (sysexLen < 1)
        {
            break; // EOF
        }
        else if (sysexData[0] == 0xf7)
        {
            break; // normal end of msg. though this is not part of the array so should actually never happen.
        }

        if (sysexData[0] == gMIDICommandBeatTrigger)
        {
            sysexData++; // advance past command byte
            sysexLen--;
            gGui.OnBeatTrigger();
            continue;
        }

        if (sysexData[0] == gMIDICommandSetOrange)
        {
            sysexData++; // advance past command byte
            sysexLen--;
            if (sysexLen < 1)
            {
                gGui.OnError();
                // Serial.println("Error: sysex data ended in the middle of LEDORANGE command.");
                break;
            }
            // read level
            byte level = sysexData[0];
            sysexData++; // advance past level byte
            sysexLen--;
            gGui.OnSetOrangeLevel(float(level) / 127);
            continue;
        }

        if (sysexData[0] == gMIDICommandSetBlue)
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
            sysexData++; // advance past level byte
            sysexLen--;
            gGui.OnSetBlueLevel(float(level) / 127);
            continue;
        }
        else if (sysexData[0] == gMIDICommandSimulateError)
        {
            gGui.OnError();
            break; // malformed.
        }
        else
        {
            if (sysexLen < 4)
            {
                gGui.OnError();
                break; // malformed.
            }

            int nled = sysexData[0];
            nled -= gMIDICommandSetNRGB;

            if (nled >= gMIDICommandSetNRGBCount)
            {
                gGui.OnError();
                break;
            }

            gGui.SetLedStripColor(
                nled, Color{float(sysexData[1]) / 127, float(sysexData[2]) / 127, float(sysexData[3]) / 127});

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
bool ProcessMIDIBusActivity(MidiBus &bus)
{
    if (!bus.mMIDI.read())
        return false;

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
        activityA = ProcessMIDIBusActivity(gMidiBusA);
        if (activityA)
        {
            gGui.OnMidiARXTX();
        }
        activityB = ProcessMIDIBusActivity(gMidiBusB);
        if (activityB)
        {
            gGui.OnMidiBRXTX();
        }
    } while (activityA || activityB);

    if (waitingForIdentify)
    {
        if (identifyTimer.ElapsedTime().ElapsedMillisI() >= identifyWaitMS)
        {
            waitingForIdentify = false;
            gMidiBusA.mMIDI.sendSysEx(SizeofStaticArray(gMIDIIdentifyResponse), gMIDIIdentifyResponse, true);
            gMidiBusB.mMIDI.sendSysEx(SizeofStaticArray(gMIDIIdentifyResponse), gMIDIIdentifyResponse, true);
            gGui.SetIdentifyState(IdentifyState::Sent);
            gGui.OnMidiABTX();
        }
    }

    // HANDLE BIG BUTTON NOTE INSERTION & SUBSEQUENT NOTE-OFF
    if (gBigButton.DidTrigger())
    {
        gGui.OnBigButtonSendNoteViaBusB();
        gMidiBusB.mMIDI.sendNoteOn(gGoNote, 126, 1);
        gNoteOffTimer.Restart();
        gWaitingForNoteOff = true;
    }
    if (gWaitingForNoteOff && gNoteOffTimer.ElapsedTime().ElapsedMillisI() >= gNoteOffDelayMS)
    {
        gMidiBusB.mMIDI.sendNoteOff(gGoNote, 0, 1);
        gGui.OnMidiBTX();
        gWaitingForNoteOff = false;
    }

    gGui.OnFrame();

    delayMicroseconds(100);
}
