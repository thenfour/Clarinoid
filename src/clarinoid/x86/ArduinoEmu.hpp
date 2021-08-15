
#pragma once

#ifndef CLARINOID_PLATFORM_X86
#error This is only for x86 unit test stuff.
#endif

#define PROGMEM
#define EXTMEM

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdio.h>
#include <Windows.h>
#undef MIN
#undef MAX
#undef min
#undef max

#include <stdint.h>
#include <string>
#include <sstream>
#include <algorithm>

#ifdef CLARINOID_MODULE_TEST

static uint64_t gTestClockMicros = 0;

uint32_t millis()
{
    return (uint32_t)((gTestClockMicros / 1000) & 0xffffffff);
}

uint32_t micros()
{
    return (uint32_t)(gTestClockMicros & 0xffffffff);
}

#else
#error huh
#endif

#include "../basic/Uptime.hpp"

#ifdef CLARINOID_MODULE_TEST

void SetTestClockMillis(int64_t ms)
{
    clarinoid::UptimeReset();
    gTestClockMicros = ms * 1000;
}
void SetTestClockMicros(int64_t m)
{
    clarinoid::UptimeReset();
    gTestClockMicros = m;
}

void delay(uint32_t ms)
{
    gTestClockMicros += ((uint64_t)ms) * 1000;
}
void delayMicroseconds(uint32_t m)
{
    gTestClockMicros += m;
}

void yield()
{
}

#else
#error huh
#endif

struct String
{
    std::stringstream mStr;
    String()
    {
    }
    String(const char *s) : mStr(s)
    {
    }
    String(const String &rhs)
    {
        mStr << rhs.mStr.str();
    }
    template <typename T>
    String(const T &rhs)
    {
        mStr << rhs;
    }
    size_t length() const
    {
        return mStr.str().length();
    }
    template <typename T>
    String &operator+=(const T &rhs)
    {
        append(rhs);
        return *this;
    }
    template <typename T>
    String operator+(const T &rhs)
    {
        String ret(*this);
        ret += rhs;
        return ret;
    }
    template <typename T>
    String &append(const T &n)
    {
        mStr << n;
        return *this;
    }
    String &append(const String &n)
    {
        mStr << n.mStr.str();
        return *this;
    }
    String &operator=(const String &s)
    {
        mStr.str(std::string());
        mStr << s.mStr.str();
        return *this;
    }
};

struct
{
    void println(const String &str)
    {
        print(str);
        print("\r\n");
    }
    void print(const String &str)
    {
        ::OutputDebugStringA(str.mStr.str().c_str());
    }
    void begin(uint32_t baud)
    {
    }
    bool operator!() const
    {
        return true;
    }
    operator bool() const
    {
        return true;
    }
} Serial;

// from core_pins.h
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define INPUT_PULLDOWN 3
#define OUTPUT_OPENDRAIN 4
#define INPUT_DISABLE 5
#define LSBFIRST 0
#define MSBFIRST 1
#define _BV(n) (1 << (n))
#define CHANGE 4
#define FALLING 2
#define RISING 3

void pinMode(uint8_t pin, uint8_t mode)
{
}
void init_pins(void)
{
}
void analogWrite(uint8_t pin, int val)
{
}
int analogRead(uint8_t pin)
{
    return 0;
}
uint32_t analogWriteRes(uint32_t bits)
{
    return 0;
}
static inline uint32_t analogWriteResolution(uint32_t bits)
{
    return analogWriteRes(bits);
}

bool gPinStates[256];

void digitalWrite(uint8_t pin, uint8_t val)
{
    gPinStates[pin] = val;
}

bool digitalReadFast(uint8_t pin)
{
    return gPinStates[pin];
}

#define DMAMEM
#define F

struct Encoder
{
    void write(int)
    {
    }
    int read()
    {
        return 0;
    }
};

#define AUDIO_BLOCK_SAMPLES 128

struct audio_block_t
{
    int16_t data[AUDIO_BLOCK_SAMPLES];
    int transmittedAsIndex = 0;
};

static audio_block_t gTestSrcBuffers[100];
static audio_block_t gTestDestBuffers[100];
static audio_block_t gTestTransmittedBuffers[100];
static size_t gAllocatedDestBuffers = 0;

void FillAudioBuffer(audio_block_t &b, int16_t val)
{
    for (int16_t &s : b.data)
    {
        s = val;
    }
}

void TestResetAudioStreams()
{
    gAllocatedDestBuffers = 0;
    for (auto &b : gTestSrcBuffers)
    {
        FillAudioBuffer(b, 0);
    }
    for (auto &b : gTestDestBuffers)
    {
        FillAudioBuffer(b, 0);
    }
    for (auto &b : gTestTransmittedBuffers)
    {
        FillAudioBuffer(b, 0);
    }
}

struct AudioStream
{
    AudioStream(unsigned char ninput, audio_block_t **iqueue)
    {
    }
    static audio_block_t *allocate(void)
    {
        auto *ret = &gTestDestBuffers[gAllocatedDestBuffers];
        gAllocatedDestBuffers++;
        return ret;
    }
    static void release(audio_block_t *block)
    {
    }
    void transmit(audio_block_t *block, unsigned char index)
    {
        block->transmittedAsIndex = index;
        gTestTransmittedBuffers[index] = *block;
    }
    audio_block_t *receiveReadOnly(unsigned int index)
    {
        return &gTestSrcBuffers[index];
    }
    audio_block_t *receiveWritable(unsigned int index)
    {
        return &gTestSrcBuffers[index];
    }

    virtual void update() = 0;
};
