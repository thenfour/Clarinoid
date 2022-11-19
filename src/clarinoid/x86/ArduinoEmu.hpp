
#pragma once

#ifndef CLARINOID_PLATFORM_X86
#error This is only for x86 unit test stuff.
#endif

#define PROGMEM
#define EXTMEM
#define FLASHMEM

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
#include <vector>

#ifdef CLARINOID_MODULE_TEST

#define AUDIO_SAMPLE_RATE 44100

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
#include "String.hpp"

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

//struct String
//{
//    std::string mStr;
//    // std::stringstream mStr;
//    String()
//    {
//    }
//    String(const char *s) : mStr(s)
//    {
//    }
//    String(const String &rhs) : mStr(rhs.mStr)
//    {
//    }
//    template <typename T>
//    String(const T &rhs)
//    {
//        std::stringstream ss;
//        ss << rhs;
//        mStr = ss.str();
//    }
//    size_t length() const
//    {
//        return mStr.length();
//    }
//    char &operator[](size_t i)
//    {
//        return mStr[i];
//    }
//    char operator[](size_t i) const
//    {
//        return mStr[i];
//    }
//    const char *c_str() const
//    {
//        return mStr.c_str();
//    }
//    template <typename T>
//    String &operator+=(const T &rhs)
//    {
//        append(rhs);
//        return *this;
//    }
//
//    String &trim(void)
//    {
//        if (mStr.length() == 0)
//            return *this;
//        const char *buffer = mStr.c_str();
//        const char *begin = buffer;
//        while (isspace(*begin))
//            begin++;
//        const char *end = buffer + mStr.length() - 1;
//        while (isspace(*end) && end >= begin)
//            end--;
//
//        mStr = std::string{begin, end};
//        // len = end + 1 - begin;
//        // if (begin > buffer)
//        //     memcpy(buffer, begin, len);
//        // buffer[len] = 0;
//        return *this;
//    }
//
//    int indexOf(char ch, unsigned int fromIndex = 0) const
//    {
//        size_t x = mStr.find(ch, fromIndex);
//        return x == std::string::npos ? (int)-1 : (int)x;
//        // if (fromIndex >= len)
//        //     return -1;
//        // const char *temp = strchr(buffer + fromIndex, ch);
//        // if (temp == NULL)
//        //     return -1;
//        // return temp - buffer;
//    }
//
//    template <typename T>
//    String operator+(const T &rhs) const
//    {
//        String ret(*this);
//        ret += rhs;
//        return ret;
//    }
//    template <typename T>
//    String &append(const T &n)
//    {
//        std::stringstream ss;
//        ss << mStr << n;
//        mStr = ss.str();
//        return *this;
//    }
//    String &append(const String &n)
//    {
//        mStr.append(n.mStr);
//        return *this;
//    }
//    String &operator=(const String &s)
//    {
//        mStr = s.mStr;
//        return *this;
//    }
//
//    bool operator==(const char *rhs) const
//    {
//        return mStr == rhs;
//    }
//
//    String substring(unsigned int left, unsigned int right) const
//    {
//        return mStr.substr(left, right - left);
//    }
//
//    String substring(unsigned int left) const
//    {
//        return mStr.substr(left);
//    }
//};
//
struct
{
    void println(const String &str)
    {
        print(str);
        print("\r\n");
    }
    void print(const String &str)
    {
        std::cout << str.c_str();
        ::OutputDebugStringA(str.c_str());
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
//#define F

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

class Stream // : public Print
{
  public:
    constexpr Stream() : _timeout(1000), read_error(0)
    {
    }
    virtual int available() = 0;
    virtual int read() = 0;
    virtual int peek() = 0;

    virtual size_t write(const uint8_t *buf, size_t bytes) = 0;
    virtual size_t read(uint8_t *buf, size_t bytes) = 0;

    void setTimeout(unsigned long timeout);
    bool find(const char *target);
    bool find(const uint8_t *target)
    {
        return find((const char *)target);
    }
    bool find(const String &target)
    {
        return find(target.c_str());
    }
    bool find(const char *target, size_t length);
    bool find(const uint8_t *target, size_t length)
    {
        return find((const char *)target, length);
    }
    bool find(const String &target, size_t length)
    {
        return find(target.c_str(), length);
    }
    bool findUntil(const char *target, const char *terminator);
    bool findUntil(const uint8_t *target, const char *terminator)
    {
        return findUntil((const char *)target, terminator);
    }
    bool findUntil(const String &target, const char *terminator)
    {
        return findUntil(target.c_str(), terminator);
    }
    bool findUntil(const char *target, const String &terminator)
    {
        return findUntil(target, terminator.c_str());
    }
    bool findUntil(const String &target, const String &terminator)
    {
        return findUntil(target.c_str(), terminator.c_str());
    }
    bool findUntil(const char *target, size_t targetLen, const char *terminate, size_t termLen);
    bool findUntil(const uint8_t *target, size_t targetLen, const char *terminate, size_t termLen)
    {
        return findUntil((const char *)target, targetLen, terminate, termLen);
    }
    bool findUntil(const String &target, size_t targetLen, const char *terminate, size_t termLen);
    bool findUntil(const char *target, size_t targetLen, const String &terminate, size_t termLen);
    bool findUntil(const String &target, size_t targetLen, const String &terminate, size_t termLen);
    long parseInt();
    long parseInt(char skipChar);
    float parseFloat();
    float parseFloat(char skipChar);
    size_t readBytes(char *buffer, size_t length);
    size_t readBytes(uint8_t *buffer, size_t length)
    {
        return readBytes((char *)buffer, length);
    }
    size_t readBytesUntil(char terminator, char *buffer, size_t length);
    size_t readBytesUntil(char terminator, uint8_t *buffer, size_t length)
    {
        return readBytesUntil(terminator, (char *)buffer, length);
    }
    String readString(size_t max = 120);
    String readStringUntil(char terminator, size_t max = 120);
    int getReadError()
    {
        return read_error;
    }
    void clearReadError()
    {
        setReadError(0);
    }

  protected:
    void setReadError(int err = 1)
    {
        read_error = err;
    }
    int timedRead();
    int timedPeek();
    int peekNextDigit();

    unsigned long _timeout;

  private:
    char read_error;
};


