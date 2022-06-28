#pragma once

#include <Arduino.h>
#include <algorithm>
#include <MIDI.h>
#include "misc.hpp"

static constexpr size_t MaxBufferSize = 512;

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
    uint8_t mSysexBuffer[MaxBufferSize] = {0};
    midi::ErrorCallback mErrorCallback;

    // limitations in callback styles mean the caller must provide the error callback method.
    MidiBus(HardwareSerial &transport, midi::ErrorCallback errorCallback) : mTransport(transport), mSerialMIDI(transport), mMIDI(mSerialMIDI), mErrorCallback(errorCallback)
    {
      transport.addMemoryForRead(mSysexBuffer, MaxBufferSize);
      ResetBus();
    }

    void ResetBus()
    {
        new (&mSerialMIDI) midi::SerialMIDI<HardwareSerial>(mTransport);
        new (&mMIDI) midi::MidiInterface<midi::SerialMIDI<HardwareSerial>, SlashKickSettings>(mSerialMIDI);
        mMIDI.begin();
        mMIDI.setHandleError(mErrorCallback);
    }
};
