
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
#include <Audio.h>
//#include <polyBlepOscillator.h> // https://gitlab.com/flojawi/teensy-polyblep-oscillator/-/tree/master/polySynth
#include <Encoder.h>
#include <WS2812Serial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPIDevice.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPR121.h>
#include <Adafruit_MCP23017.h>

#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeMono18pt7b.h>
#include <Fonts/FreeMono24pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeMonoBoldOblique18pt7b.h>
#include <Fonts/FreeMonoBoldOblique24pt7b.h>
#include <Fonts/FreeMonoBoldOblique9pt7b.h>
#include <Fonts/FreeMonoOblique12pt7b.h>
#include <Fonts/FreeMonoOblique18pt7b.h>
#include <Fonts/FreeMonoOblique24pt7b.h>
#include <Fonts/FreeMonoOblique9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBoldOblique12pt7b.h>
#include <Fonts/FreeSansBoldOblique18pt7b.h>
#include <Fonts/FreeSansBoldOblique24pt7b.h>
#include <Fonts/FreeSansBoldOblique9pt7b.h>
#include <Fonts/FreeSansOblique12pt7b.h>
#include <Fonts/FreeSansOblique18pt7b.h>
#include <Fonts/FreeSansOblique24pt7b.h>
#include <Fonts/FreeSansOblique9pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSerif18pt7b.h>
#include <Fonts/FreeSerif24pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerifBold12pt7b.h>
#include <Fonts/FreeSerifBold18pt7b.h>
#include <Fonts/FreeSerifBold24pt7b.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/FreeSerifBoldItalic12pt7b.h>
#include <Fonts/FreeSerifBoldItalic18pt7b.h>
#include <Fonts/FreeSerifBoldItalic24pt7b.h>
#include <Fonts/FreeSerifBoldItalic9pt7b.h>
#include <Fonts/FreeSerifItalic12pt7b.h>
#include <Fonts/FreeSerifItalic18pt7b.h>
#include <Fonts/FreeSerifItalic24pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>
#include <Fonts/Org_01.h>
#include <Fonts/Picopixel.h>
#include <Fonts/Tiny3x3a2pt7b.h>
#include <Fonts/TomThumb.h>


#pragma GCC diagnostic pop

#include "testDeviceApp.hpp"

void setup()
{
    Serial.begin(9600);
    //while(!Serial) {} // when you are debugging with serial, uncomment this to ensure you see startup msgs

    clarinoid::TestDeviceApp app;
    app.Main();
}

void loop()
{
    // unreachable
}
