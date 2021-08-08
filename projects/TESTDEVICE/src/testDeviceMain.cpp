
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
    // Serial.println("starting normally....");

    int16_t q15[128];
    float f31[128];
    auto reset = [&]() {
        for (int16_t &e : q15)
        {
            e = (int16_t)((float(rand()) / RAND_MAX * 2 - 1) * 32767);
        }
        for (float &e : f31)
        {
            e = (float(rand()) / RAND_MAX * 2 - 1);
        }
    };
    clarinoid::Stopwatch sw;
    static constexpr size_t iterations = 100000;
    srand(micros());

    {
        reset();
        sw.Restart();
        sw.Pause();
        for (size_t i = 0; i < iterations; ++i)
        {
        reset();
            float s = float(i) / iterations * 2 - 1;
            sw.Unpause();
            arm_scale_f32(f31, s, f31, 128);
            sw.Pause();
        }
        Serial.println(String("arm_scale_f32  micros=") + (int)sw.ElapsedTime().ElapsedMicros());
    }

    {
        reset();
        sw.Restart();
        sw.Pause();
        for (size_t i = 0; i < iterations; ++i)
        {
        reset();
            float s = float(i) / iterations * 2 - 1;
            sw.Unpause();
            arm_scale_q15(q15, s, 0, q15, 128);
            sw.Pause();
        }
        Serial.println(String("arm_scale_q15  micros=") + (int)sw.ElapsedTime().ElapsedMicros());
    }

    {
        reset();
        sw.Restart();
        sw.Pause();
        for (size_t i = 0; i < iterations; ++i)
        {
        reset();
            float s = float(i) / iterations * 2 - 1;
            sw.Unpause();
            for (float& f : f31) {
                f *= s;
            }
            sw.Pause();
        }
        Serial.println(String("manual float  micros=") + (int)sw.ElapsedTime().ElapsedMicros());
    }

    {
        reset();
        sw.Restart();
        sw.Pause();
        for (size_t i = 0; i < iterations; ++i)
        {
        reset();
            float s = float(i) / iterations * 2 - 1;
            sw.Unpause();
            auto mul = clarinoid::gainToSignedMultiply32x16(s);
            clarinoid::audioBufferCopyAndApplyGain(q15, q15, mul);
            sw.Pause();
        }
        Serial.println(String("audioBufferCopyAndApplyGain  micros=") + (int)sw.ElapsedTime().ElapsedMicros());
    }



    {
        sw.Restart();
        sw.Pause();
        for (size_t i = 0; i < iterations; ++i)
        {
        reset();
            float s = float(i) / iterations * 2 - 1;
            sw.Unpause();
            clarinoid::fast::Sample16To32Buffer(q15, f31);
            arm_scale_f32(f31, s, f31, 128);
            clarinoid::fast::Sample32To16Buffer(f31, q15);
            sw.Pause();
        }
        Serial.println(String("arm_scale_f32 with 16-bit roundtrip  micros=") + (int)sw.ElapsedTime().ElapsedMicros());
    }


    // auto* app = new clarinoid::TestDeviceApp;
    // app->Main();
}

void loop()
{
    // unreachable
}
