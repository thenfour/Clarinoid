// TODO: generate PROGMEM lut header file
// TODO: some tests

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
#include <Encoder.h>
#include <WS2812Serial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPIDevice.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPR121.h>
#include <Adafruit_MCP23017.h>

#pragma GCC diagnostic pop

#include "testDeviceApp.hpp"

void setup()
{
    Serial.begin(9600);
    while (!Serial)
    {
    } // when you are debugging with serial, uncomment this to ensure you see startup msgs
    Serial.println("starting normally....");

    clarinoid::Stopwatch sw;
    srand(micros());

    clarinoid::CCEncoder<4 /* step increment each increment */, 14 /* pin #1 */, 15 /* pin #2 */> encoder;
    CCAdafruitSSD1306 display = {128, 64, &SPI, 9 /*DC*/, 8 /*RST*/, 10 /*CS*/, 10 * 1000000UL};
    display.begin(SSD1306_SWITCHCAPVCC);

    float k = 0.5f;
    static constexpr int graphWidth = 60;
    static constexpr int graphHeight = 60;

    decltype(gModCurveLUT)& lut = gModCurveLUT;

    int16_t q15[128];
    static constexpr size_t iterations = 70;
    sw.Pause();
    sw.Restart();
    sw.Pause();
    for (size_t i = 0; i < iterations; ++i)
    {
        for (int16_t &q : q15)
        {
            q = rand() - RAND_MAX / 2;
        }
        float k = ((float(rand()) / RAND_MAX) * 2) - 1;
        sw.Unpause();
        auto state = lut.BeginLookup(k);
        for (int16_t &q : q15)
        {
            q += lut.Transfer(q, state);
        }
        sw.Pause();
    }
    Serial.println(String("micros: ") + (int)sw.ElapsedTime().ElapsedMicros());

    while (true)
    {
        display.clearDisplay();
        display.DrawDottedRect(0, 0, graphWidth + 3, graphHeight + 1, SSD1306_WHITE);
        display.fillRect(1, 1, graphWidth, graphHeight - 2, SSD1306_BLACK);

        delay(5);
        encoder.Update();
        k = clarinoid::Frac(0.5 + (encoder.CurrentValue() / 25)) * 2 - 1;
        display.setCursor(127 - 7 * 8, 0);
        display.setTextColor(SSD1306_WHITE);
        display.print(String("k=") + k);

        auto lookupState = lut.BeginLookup(k);

        display.setCursor(127 - 8 * 8, 8);
        display.println(String("1=") + lut.Transfer(32767, lookupState));
        display.setCursor(127 - 8 * 8, 16);
        display.println(String("0=") + lut.Transfer(0, lookupState));
        display.setCursor(127 - 8 * 8, 24);
        display.println(String("-1=") + lut.Transfer(-32768, lookupState));

        for (size_t ix = 1; ix < graphWidth + 1; ++ix)
        {
            // static line
            //int16_t s1 = float(ix - 1) * 7 / graphWidth)) * 32767;
            int16_t s1 = ((float(ix-1) / graphWidth) * 2 - 1) * 32767;
            int16_t s2 = ((float(ix) / graphWidth) * 2 - 1) * 32767;

            // moving sine
            // int16_t s1 = ::sinf(float(millis()) / 1000 + (float(ix - 1) * 7 / graphWidth)) * 32767;
            // int16_t s2 = ::sinf(float(millis()) / 1000 + (float(ix) * 7 / graphWidth)) * 32767;
            int y1x = ((float(lut.Transfer(s1, lookupState)) / 32767) * .5 + .5) * graphHeight;
            int y2x = ((float(lut.Transfer(s2, lookupState)) / 32767) * .5 + .5) * graphHeight;
            display.drawLine(ix, graphHeight - y1x, ix + 1, graphHeight - y2x, SSD1306_WHITE);
        }

        display.display();
    }

    // auto* app = new clarinoid::TestDeviceApp;
    // app->Main();
}

void loop()
{
    // unreachable
}
