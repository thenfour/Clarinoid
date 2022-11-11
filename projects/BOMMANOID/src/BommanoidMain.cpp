// Teensy 4.1

//============================================================
/////////////////////////////////////////////////////////////////////////////////////////////////

// arduino-ish libraries
#define ENCODER_DO_NOT_USE_INTERRUPTS
#define AUDIO_BLOCK_SAMPLES 128 // other values are currently producing artifacts.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <Arduino.h>

#include <USBHost_t36.h>
#include <Wire.h>
//#include <EasyTransfer.h>
//#include <FastCRC.h>
#include <Bounce.h>
#include <SPI.h>
#include <MIDI.h>
//#include <SD.h>
//#include <SerialFlash.h>
#include <Encoder.h>
#include <Audio.h>
#include <WS2812Serial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPIDevice.h>
#include <Adafruit_SSD1306.h>

#include <vector> // note tracker uses vector
#include <algorithm>

#pragma GCC diagnostic pop

USBHost gUsbHost;
USBHub hub1(gUsbHost);
USBHub hub2(gUsbHost);
USBHub hub3(gUsbHost);
MIDIDevice_BigBuffer gUsbMidi(gUsbHost);

static constexpr size_t aosnetuhpch = sizeof(gUsbMidi);
static constexpr size_t aosnet9uhpch = sizeof(hub1);
static constexpr size_t aos8net9uhpch = sizeof(gUsbHost);

#include "bmApp.hpp"

// std::array<uint8_t, sizeof(clarinoid::BommanoidApp)> gAppStorage;
// static constexpr size_t aosnetuh8pch = sizeof(clarinoid::BommanoidApp);

void setup()
{
    clarinoid::gCrashHandlers[0] = &clarinoid::gSerialCrashHandler;
    clarinoid::gCrashHandlers[1] = &clarinoid::gDisplay;
    clarinoid::CheckCrashReport();

    AudioNoInterrupts();
    Serial.begin(9600);
    Wire.begin();
    Wire.setClock(clarinoid::gWireDataRate);

    clarinoid::BommanoidApp *app = new clarinoid::BommanoidApp;
    // clarinoid::BommanoidApp *app = new (gAppStorage.data()) clarinoid::BommanoidApp();
    app->Main();
}

void loop()
{
    // unreachable
}
