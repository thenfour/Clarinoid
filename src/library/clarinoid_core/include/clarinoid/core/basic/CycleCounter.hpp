#pragma once

#include "Teensy.hpp"

namespace clarinoid
{
// Simple accessor for the CPU cycle counter (DWT on Cortex-M7).
// NOTE: enabling resets the global counter (shared). Avoid repeated enable() calls if you need
// nested / concurrent measurements.
struct CycleCounter
{
  CycleCounter()
  {
    Teensy::cycles_enable();
  }
  inline uint32_t now() const
  {
    return Teensy::cycles_now();
  }
  inline void reset()
  {
    Teensy::cycles_enable(); /* reset the counter */
  }
};

// Stopwatch-like cycle timer. Similar intent to clarinoid::Stopwatch but measures raw CPU cycles.
// Designed for short benchmarking sections (< 2^32 cycles between pause/unpause) – wrap handling
// is via unsigned arithmetic so multiple pauses are accumulated in 64-bit space.
struct CycleTimer
{
  bool mIsRunning = true;
  uint32_t mStartCycle = 0;   // start of current running segment (32-bit, wraps naturally)
  uint64_t mAccumCycles = 0;  // accumulated cycles from prior segments + optional offset

  CycleTimer()
  {
    Restart();
  }

  // Ensure DWT is enabled only once if caller wants to avoid resetting the counter.
  // By default we reset (for isolation & immunity to wrap). Set resetCounter=false to just read.
  void Restart(uint64_t offsetCycles = 0, bool resetCounter = true)
  {
    if (resetCounter)
      Teensy::cycles_enable();
    mStartCycle = Teensy::cycles_now();
    mAccumCycles = offsetCycles;
    mIsRunning = true;
  }

  void PauseAndReset(uint64_t newOffsetCycles = 0)
  {
    mAccumCycles = newOffsetCycles;
    mStartCycle = Teensy::cycles_now();
    mIsRunning = false;
  }

  inline uint64_t ElapsedCycles() const
  {
    if (!mIsRunning)
      return mAccumCycles;
    uint32_t now = Teensy::cycles_now();
    // Unsigned wrap-safe delta accumulation.
    uint64_t delta = uint32_t(now - mStartCycle);
    return mAccumCycles + delta;
  }

  inline void Pause()
  {
    if (!mIsRunning)
      return;
    mAccumCycles = ElapsedCycles();  // fold in current segment
    mIsRunning = false;
  }

  inline void Unpause()
  {
    if (mIsRunning)
      return;
    mStartCycle = Teensy::cycles_now();
    mIsRunning = true;
  }

  // Convenience conversions (best-effort). Requires F_CPU or F_CPU_ACTUAL; fallback assumes 600 MHz.
  static constexpr double CpuHz()
  {
#if defined(F_CPU_ACTUAL)
    return double(F_CPU_ACTUAL);
#elif defined(F_CPU)
    return double(F_CPU);
#else
    return 600000000.0;  // fallback assumption
#endif
  }
  static inline double CyclesToSeconds(uint64_t cycles)
  {
    return cycles / CpuHz();
  }
  static inline double CyclesToMicros(uint64_t cycles)
  {
    return (cycles * 1e6) / CpuHz();
  }
  static inline double CyclesToNanos(uint64_t cycles)
  {
    return (cycles * 1e9) / CpuHz();
  }
};
}  // namespace clarinoid
