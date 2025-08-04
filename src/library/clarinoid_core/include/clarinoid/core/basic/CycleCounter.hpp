
#include "Teensy.hpp"

namespace clarinoid
{
struct CyleCounter
{
  CyleCounter()
  {
    Teensy::cycles_enable();
  }
  inline uint32_t now() const
  {
    return Teensy::cycles_now();
  }
  inline void reset()
  {
    Teensy::cycles_enable();  // reset the counter
  }
};

// todo: stopwatch that uses this cycle counter.

}  // namespace clarinoid