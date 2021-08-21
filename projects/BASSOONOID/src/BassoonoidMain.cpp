// Teensy 4.1

//============================================================
/////////////////////////////////////////////////////////////////////////////////////////////////

// arduino-ish libraries
#define ENCODER_DO_NOT_USE_INTERRUPTS
#define AUDIO_BLOCK_SAMPLES 128 // other values are currently producing artifacts.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <Arduino.h>

#include <Wire.h>
#include <EasyTransfer.h>
#include <FastCRC.h>
#include <Bounce.h>
#include <SPI.h>
#include <MIDI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Encoder.h>
#include <Audio.h>
#include <WS2812Serial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPIDevice.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MCP23017.h>

#pragma GCC diagnostic pop

#include "bsApp.hpp"

void setup()
{
    clarinoid::gCrashHandlers[0] = &clarinoid::gSerialCrashHandler;
    clarinoid::gCrashHandlers[1] = &clarinoid::gDisplay;
    clarinoid::CheckCrashReport();

    AudioNoInterrupts();
    Serial.begin(9600);
    // while(!Serial) {} // when you are debugging with serial, uncomment this to ensure you see startup msgs

    auto *app = new clarinoid::BassoonoidApp;
    app->Main();
}

void loop()
{
    // unreachable
}
