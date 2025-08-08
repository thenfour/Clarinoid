
#pragma once


#if __cplusplus >= 201703L
  #define CL_NODISCARD [[nodiscard]]
#else
  #error C++17 or later is required for this project.
  #define CL_NODISCARD /*nothing*/
#endif

#ifdef CLARINOID_PLATFORM_X86
  #include "../x86/ArduinoEmu.hpp"
#endif

#ifdef CLARINOID_PLATFORM_TEENSY
  #include <Arduino.h>
#endif

#ifndef AUDIO_BLOCK_SAMPLES
  #define AUDIO_BLOCK_SAMPLES 128
#endif
