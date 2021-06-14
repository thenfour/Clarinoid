
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
#include <polyBlepOscillator.h>// https://gitlab.com/flojawi/teensy-polyblep-oscillator/-/tree/master/polySynth
#include <Encoder.h>
#include <WS2812Serial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPIDevice.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPR121.h>
#include <Adafruit_MCP23017.h>

#pragma GCC diagnostic pop

#include "clarinoid2App.hpp"

void setup()
{
  Serial.begin(9600);
  //while(!Serial) {} // when you are debugging with serial, uncomment this to ensure you see startup msgs

  clarinoid::Clarinoid2App app;
  app.Main();
}

void loop() {
  // unreachable
}
