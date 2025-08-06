
#pragma once

#ifdef CLARINOID_PLATFORM_X86
  #include "../x86/ArduinoEmu.hpp"
#endif

#ifdef CLARINOID_PLATFORM_TEENSY
  #include <Arduino.h>
#endif
