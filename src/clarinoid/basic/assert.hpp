
#pragma once

#include "log.hpp"

namespace clarinoid
{

struct IClarinoidCrashReportOutput
{
    virtual void IClarinoidCrashReportOutput_Init() = 0;
    virtual void IClarinoidCrashReportOutput_Blink() = 0;
    virtual void IClarinoidCrashReportOutput_Print(const char *) = 0;
};

IClarinoidCrashReportOutput *gCrashHandlers[10] = {0};

struct SerialCrashHandler : IClarinoidCrashReportOutput
{
    virtual void IClarinoidCrashReportOutput_Init() override
    {
    }
    virtual void IClarinoidCrashReportOutput_Blink() override
    {
    }
    virtual void IClarinoidCrashReportOutput_Print(const char *s) override
    {
        Serial.begin(9600);
        Serial.print(s);
    }
};

SerialCrashHandler gSerialCrashHandler;

#ifdef CLARINOID_PLATFORM_X86
struct PrintToString
{
    std::string acc;
    size_t write(uint8_t b)
    {
        acc.append(1, b);
        return 1;
    }
};

struct
{
    operator bool() const
    {
        return false;
    }
    template<typename T>
    void printTo(const T &s)
    {

    }
} CrashReport;


#else
struct PrintToString : Print
{
    String acc;
    virtual size_t write(uint8_t b) override
    {
        acc.append((char)b);
        return 1;
    }
};
#endif // #ifndef CLARINOID_PLATFORM_X86


inline void CheckCrashReport()
{
    if (CrashReport)
    {
        for (auto *p : gCrashHandlers)
        {
            if (p)
            {
                p->IClarinoidCrashReportOutput_Init();
            }
        }
        for (int i = 0; i < 4; ++i)
        {
            for (auto *p : gCrashHandlers)
            {
                if (p)
                {
                    p->IClarinoidCrashReportOutput_Blink();
                }
            }
        }
        PrintToString s;
        CrashReport.printTo(s);

        while (true)
        {
            for (auto *p : gCrashHandlers)
            {
                if (p)
                {
                    p->IClarinoidCrashReportOutput_Blink();
                    p->IClarinoidCrashReportOutput_Print(s.acc.c_str());
                }
            }
            delay(1500);
        }
    }
}

inline void DebugBlink(int n, int period = 120)
{
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);
    delay(period * 3);
    for (int i = 0; i < n; ++i)
    {
        digitalWrite(13, HIGH);
        delay(period);
        digitalWrite(13, LOW);
        delay(period);
    }
    digitalWrite(13, LOW);
    delay(period * 3);
}

inline void BlinkyDeath(int n)
{
    while (true)
    {
        DebugBlink(n, 400);
        delay(700);
    }
}

//#ifndef CLARINOID_PLATFORM_X86
//#endif // #ifndef CLARINOID_PLATFORM_X86

#if defined(CLARINOID_MODULE_MAIN)

// a sort of exception display mechanism.
bool gIsCrashed = false;
String gCrashMessage;

void DefaultCrashHandler()
{
    Serial.begin(9600);
    while (!Serial)
        ;
    while (true)
    {
        Serial.println(gCrashMessage);
        delay(1000);
    }
}

void (*pfnCrashHandler)() = DefaultCrashHandler;

static inline void Die(const String &msg)
{
    DebugBlink(4);
    gCrashMessage = msg;
    gIsCrashed = true;
    pfnCrashHandler();
}

#elif defined(CLARINOID_PLATFORM_TEENSY)

static inline void Die(const String &msg)
{
    DebugBlink(4);
    Serial.begin(9600);
    while (!Serial)
        ;
    Serial.println(msg);
    while (true)
    {
        delay(500);
    }
}

#elif defined(CLARINOID_PLATFORM_X86)

static inline void Die(const String &msg)
{
    log(msg.c_str());
    DebugBreak();
}

#else

#error No Clarinoid platform is defined...

#endif

#define CCASSERT(x)                                                                                                    \
    if (!(x))                                                                                                          \
    {                                                                                                                  \
        ::clarinoid::Die(String(__COUNTER__) + (#x));                                                                                 \
    }

#define CCASSERT2(x, msg)                                                                                                    \
    if (!(x))                                                                                                          \
    {                                                                                                                  \
        ::clarinoid::Die(String(__COUNTER__) + (msg) + (#x));                                                                                 \
    }

// an assert which is intended to stay in release build
#define REQUIRE(x)                                                                                                     \
    if (!(x))                                                                                                          \
    {                                                                                                                  \
        ::clarinoid::Die(__COUNTER__);                                                                                 \
    }

} // namespace clarinoid
