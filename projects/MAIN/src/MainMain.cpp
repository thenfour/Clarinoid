// Teensy 4.0

#define CLARINOID_PLATFORM_TEENSY
#define CLARINOID_MODULE_MAIN

static constexpr int RESOLUTION_X = 128;
static constexpr int RESOLUTION_Y = 32;

//============================================================
/////////////////////////////////////////////////////////////////////////////////////////////////

// arduino-ish libraries
#define ENCODER_DO_NOT_USE_INTERRUPTS
#define AUDIO_BLOCK_SAMPLES 128 // other values are currently producing artifacts.
#include <Arduino.h>

PROGMEM const char gClarinoidVersion[] = "Clarinoid v0.01";

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
#include <Adafruit_GFX.h>
#include <Adafruit_SPIDevice.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_SSD1306.h>

//#pragma pack(push, 1)

// MIDI library is touchy about how you instantiate.
// Simplest is to do it the way it's designed for: in the main sketch, global scope.
MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, gMIDI);

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/components/Switch.hpp>
#include <clarinoid/components/Leds.hpp>
#include <clarinoid/comm/ClarinoidTxRx.hpp>

#include <clarinoid/settings/AppSettings.hpp>
#include <clarinoid/application/ClarinoidApplication.hpp>

#include <clarinoid/components/Display.hpp>
#include <clarinoid/menu/MenuApps.hpp>

void setup() {
  Serial.begin(9600);
  DebugBlink(5);
  SetupUpdateObjects();
}

bool firstLoop = true;
uint32_t gLoopExitMicros = micros();

 void loop() {
  if (gIsCrashed) {
    if (!gDisplay.mIsSetup) {
      gDisplay.setup();
    }
    gDisplay.mDisplay.clearDisplay();
    gDisplay.mDisplay.setCursor(0,0);
    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
    gDisplay.mDisplay.println("!EXCEPTION!");
    gDisplay.mDisplay.println(gCrashMessage);
    gDisplay.mDisplay.display();
    Serial.begin(9600);
    while (!Serial);
    Serial.println(gCrashMessage);
    while(true) {
      delay(100);
    }
    return;
  }
  
  uint32_t m = micros();
  if (firstLoop) {
    firstLoop = false;
  } else {
    if (m > gLoopExitMicros && (m - gLoopExitMicros) > gLongestBetweenLoopMicros) {
      gLongestBetweenLoopMicros = m - gLoopExitMicros;
    }
  }

  UpdateUpdateObjects();

  uint32_t n = micros();
  if (n > m && (n - m) > gLongestLoopMicros) {
    gLongestLoopMicros = n - m;
  }
  
  gLoopExitMicros = micros();
}
