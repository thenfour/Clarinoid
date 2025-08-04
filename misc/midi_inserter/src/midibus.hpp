#pragma once

#include <Arduino.h>
#include <algorithm>
#include <MIDI.h>
#include "misc.hpp"

static constexpr size_t MaxBufferSize = 2048;

struct SlashKickSettings
{
    static const bool UseRunningStatus = false;
    static const bool HandleNullVelocityNoteOnAsNoteOff = true;
    static const bool Use1ByteParsing = true;
    static const unsigned SysExMaxSize =
        MaxBufferSize; // big payloads to set all launchpad buttons are like 350 bytes long.

    static const bool UseSenderActiveSensing = false;
    static const bool UseReceiverActiveSensing = false;
    static const uint16_t SenderActiveSensingPeriodicity = 0;
};

struct MidiBus
{
    HardwareSerial &mTransport;
    midi::SerialMIDI<HardwareSerial> mSerialMIDI;
    midi::MidiInterface<midi::SerialMIDI<HardwareSerial>, SlashKickSettings> mMIDI;
    // midi::MidiInterface<midi::SerialMIDI<HardwareSerial>, midi::DefaultSettings> mMIDI;
    uint8_t mSysexBuffer[MaxBufferSize] = {0};
    midi::ErrorCallback mErrorCallback;

    // limitations in callback styles mean the caller must provide the error callback method.
    MidiBus(HardwareSerial &transport, midi::ErrorCallback errorCallback)
        : mTransport(transport), mSerialMIDI(transport), mMIDI(mSerialMIDI), mErrorCallback(errorCallback)
    {
        transport.addMemoryForRead(mSysexBuffer, MaxBufferSize);
        ResetBus();
    }

    void ResetBus()
    {
        new (&mSerialMIDI) midi::SerialMIDI<HardwareSerial>(mTransport);
        new (&mMIDI) midi::MidiInterface<midi::SerialMIDI<HardwareSerial>, SlashKickSettings>(mSerialMIDI);
        // new (&mMIDI) midi::MidiInterface<midi::SerialMIDI<HardwareSerial>, midi::DefaultSettings>(mSerialMIDI);
        mMIDI.begin();
        mMIDI.setHandleError(mErrorCallback);
    }
};

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
