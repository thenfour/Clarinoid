
#pragma once

#include "log.hpp"

namespace clarinoid
{

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
    log(msg.mStr.str().c_str());
    DebugBreak();
}

#else

#error No Clarinoid platform is defined...

#endif

#define CCASSERT(x)                                                                                                    \
    if (!(x))                                                                                                          \
    {                                                                                                                  \
        ::clarinoid::Die(String("E") + __COUNTER__ + (":" #x) + __FILE__ + ":" + (int)__LINE__);                       \
    }
#define CCDIE(msg)                                                                                                     \
    {                                                                                                                  \
        Die(String("E") + __COUNTER__ + ":" + msg + __FILE__ + ":" + (int)__LINE__);                                   \
    }

} // namespace clarinoid
