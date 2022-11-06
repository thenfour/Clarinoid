

#pragma once

namespace clarinoid
{

// DMAMEM byte gLED1DisplayMemory[20 * 12]; // 12 bytes per LED

struct ILEDDataProvider
{
    virtual Metronome *ILEDDataProvider_GetMetronomeBeat() = 0;
    virtual InputDelegator *ILEDDataProvider_GetInput() = 0;
    virtual MusicalState *ILEDDataProvider_GetMusicalState() = 0;
};

// this is a task which performs the meat of the musical state.
// 1. converting input data to musical data
// 2. updating the synth state
struct TheShinies : ITask
{

    static constexpr uint8_t pinBlink = 28;
    static constexpr uint8_t pinAct1 = 27;
    static constexpr uint8_t pinAct2 = 16;
    static constexpr uint8_t pinAct3 = 5;
    static constexpr uint8_t pinRJ45green = 29;
    static constexpr uint8_t pinRJ45yellow = 4;

    static constexpr uint8_t Digit1Select = 24;
    static constexpr uint8_t Digit2Select = 23;
    static constexpr uint8_t A = 6;
    static constexpr uint8_t B = 45;
    static constexpr uint8_t C = 17;
    static constexpr uint8_t D = 15;
    static constexpr uint8_t E = 22;
    static constexpr uint8_t F = 33;
    static constexpr uint8_t G = 36;
    static constexpr uint8_t H = 32;
    static constexpr uint8_t J = 26;
    static constexpr uint8_t K = 42;
    static constexpr uint8_t L = 43;
    static constexpr uint8_t M = 44;
    static constexpr uint8_t N = 25;
    static constexpr uint8_t P = 9;
    static constexpr uint8_t DP = 3;

    AppSettings *mAppSettings = nullptr;
    int mFrame = 0;

    TheShinies(AppSettings *appSettings) : mAppSettings(appSettings)
    {
        pinMode(Digit1Select, OUTPUT);
        pinMode(Digit2Select, OUTPUT);

        pinMode(pinBlink, OUTPUT);
        pinMode(pinAct1, OUTPUT);
        pinMode(pinAct2, OUTPUT);
        pinMode(pinAct3, OUTPUT);
        pinMode(pinRJ45green, OUTPUT);
        pinMode(pinRJ45yellow, OUTPUT);
    }

#define rot(x, k) (((x) << (k)) | ((x) >> (32 - (k))))

#define mix(a, b, c)                                                                                                   \
    {                                                                                                                  \
        a -= c;                                                                                                        \
        a ^= rot(c, 4);                                                                                                \
        c += b;                                                                                                        \
        b -= a;                                                                                                        \
        b ^= rot(a, 6);                                                                                                \
        a += c;                                                                                                        \
        c -= b;                                                                                                        \
        c ^= rot(b, 8);                                                                                                \
        b += a;                                                                                                        \
        a -= c;                                                                                                        \
        a ^= rot(c, 16);                                                                                               \
        c += b;                                                                                                        \
        b -= a;                                                                                                        \
        b ^= rot(a, 19);                                                                                               \
        a += c;                                                                                                        \
        c -= b;                                                                                                        \
        c ^= rot(b, 4);                                                                                                \
        b += a;                                                                                                        \
    }

#define final(a, b, c)                                                                                                 \
    {                                                                                                                  \
        c ^= b;                                                                                                        \
        c -= rot(b, 14);                                                                                               \
        a ^= c;                                                                                                        \
        a -= rot(c, 11);                                                                                               \
        b ^= a;                                                                                                        \
        b -= rot(a, 25);                                                                                               \
        c ^= b;                                                                                                        \
        c -= rot(b, 16);                                                                                               \
        a ^= c;                                                                                                        \
        a -= rot(c, 4);                                                                                                \
        b ^= a;                                                                                                        \
        b -= rot(a, 14);                                                                                               \
        c ^= b;                                                                                                        \
        c -= rot(b, 24);                                                                                               \
    }

    uint32_t lookup3(const void *key, size_t length, uint32_t initval)
    {
        uint32_t a, b, c;
        const uint8_t *k;
        const uint32_t *data32Bit = (const uint32_t *)key;

        a = b = c = 0xdeadbeef + (((uint32_t)length) << 2) + initval;

        while (length > 12)
        {
            a += *(data32Bit++);
            b += *(data32Bit++);
            c += *(data32Bit++);
            mix(a, b, c);
            length -= 12;
        }

        k = (const uint8_t *)data32Bit;
        switch (length)
        {
        case 12:
            c += ((uint32_t)k[11]) << 24;
        case 11:
            c += ((uint32_t)k[10]) << 16;
        case 10:
            c += ((uint32_t)k[9]) << 8;
        case 9:
            c += k[8];
        case 8:
            b += ((uint32_t)k[7]) << 24;
        case 7:
            b += ((uint32_t)k[6]) << 16;
        case 6:
            b += ((uint32_t)k[5]) << 8;
        case 5:
            b += k[4];
        case 4:
            a += ((uint32_t)k[3]) << 24;
        case 3:
            a += ((uint32_t)k[2]) << 16;
        case 2:
            a += ((uint32_t)k[1]) << 8;
        case 1:
            a += k[0];
            break;
        case 0:
            return c;
        }
        final(a, b, c);
        return c;
    }

    virtual void TaskRun() override
    {
        uint32_t t = millis() / ((mFrame & 1) ? 300 : 133);
        uint32_t ts[4] = {t, t, t, t};
        // hash it.
        t = lookup3(ts, sizeof(ts), 0xa3a3a3a3);

        // turn off all display to prevent bleed. from here make the code as fast as possible
        // to reduce the duration of darkness.
        digitalWrite(Digit1Select, LOW);
        digitalWrite(Digit2Select, LOW);

        // set up digit and present.
        pinMode(A, (t & 1) ? INPUT : OUTPUT);
        pinMode(B, (t & 2) ? INPUT : OUTPUT);
        pinMode(C, (t & 4) ? INPUT : OUTPUT);
        pinMode(D, (t & 8) ? INPUT : OUTPUT);
        pinMode(E, (t & 16) ? INPUT : OUTPUT);
        pinMode(F, (t & 32) ? INPUT : OUTPUT);
        pinMode(G, (t & 64) ? INPUT : OUTPUT);

        pinMode(H, (t & 128) ? INPUT : OUTPUT);
        pinMode(J, (t & 256) ? INPUT : OUTPUT);
        pinMode(K, (t & 512) ? INPUT : OUTPUT);
        pinMode(L, (t & 1024) ? INPUT : OUTPUT);

        pinMode(M, (t & 2048) ? INPUT : OUTPUT);
        pinMode(N, (t & 4096) ? INPUT : OUTPUT);
        pinMode(P, (t & 8192) ? INPUT : OUTPUT);
        pinMode(DP, (t & 16384) ? INPUT : OUTPUT);
        uint32_t y = 16384 * 2;
        digitalWrite(pinBlink, (t & y) ? HIGH : LOW);
        y <<= 1;
        digitalWrite(pinAct1, (t & y) ? HIGH : LOW);
        y <<= 1;
        digitalWrite(pinAct2, (t & y) ? HIGH : LOW);
        y <<= 1;
        digitalWrite(pinAct3, (t & y) ? HIGH : LOW);
        y <<= 1;
        digitalWrite(pinRJ45green, (t & y) ? HIGH : LOW);
        y <<= 1;
        digitalWrite(pinRJ45yellow, (t & y) ? HIGH : LOW);

        digitalWrite(Digit1Select, (mFrame & 1) ? HIGH : LOW);
        digitalWrite(Digit2Select, !(mFrame & 1) ? HIGH : LOW);

        mFrame++;
    }
};

} // namespace clarinoid
