
#pragma once

#include <cstdint>

namespace clarinoid
{

extern uint32_t gUptimeLastMicrosCall;
extern int64_t gUptimeCurrentOffset;  // every time the 32-bit micros() call rolls over, this gets += 1<<32;

// for testing purposes we need to be able to act like the system is restarting from 0, uptime-wise.
inline void UptimeReset()
{
    gUptimeLastMicrosCall = 0;
    gUptimeCurrentOffset = 0;
}

inline int64_t UptimeMicros64()
{
    uint32_t m = micros();
    if (m < gUptimeLastMicrosCall)
    {
        gUptimeCurrentOffset += ((int64_t)1) << 32;
    }
    gUptimeLastMicrosCall = m;
    return gUptimeCurrentOffset + m;
}

} // namespace clarinoid
