
#pragma once

namespace clarinoid
{

  static uint32_t gUptimeLastMicrosCall = 0;
  static int64_t gUptimeCurrentOffset = 0; // every time the 32-bit micros() call rolls over, this gets += 1<<32;

  // for testing purposes we need to be able to act like the system is restarting from 0, uptime-wise.
  void UptimeReset()
  {
    gUptimeLastMicrosCall = 0;
    gUptimeCurrentOffset = 0;
  }

  int64_t UptimeMicros64()
  {
    uint32_t m = micros();
    if (m < gUptimeLastMicrosCall) {
      gUptimeCurrentOffset += ((int64_t)1) << 32;
    }
    gUptimeLastMicrosCall = m;
    return gUptimeCurrentOffset + m;
  }

} // namespace clarinoid
