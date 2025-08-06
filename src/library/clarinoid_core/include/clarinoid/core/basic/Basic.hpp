
#pragma once

#if __cplusplus >= 201703L
  #define CL_NODISCARD [[nodiscard]]
#else
  #error C++17 or later is required for this project.
  #define CL_NODISCARD /*nothing*/
#endif

#include "CLArduino.hpp"

// language basics, syntax, types, fundamental helpers
#include "Array.hpp"
#include "BaseDefs.hpp"
#include "Binary.hpp"
#include "Memory.hpp"
#include "Teensy.hpp"
#include "assert.hpp"


#include "Enum.hpp"
#include "Tristate.hpp"
#include "function.hpp"


#include "CircularArray.hpp"
#include "MovingAverage.hpp"
#include "SortedArray.hpp"
#include "Util.hpp"


#include "./string/format.hpp"

#include "./Numeric.hpp"

#include "./Geometry.hpp"

#include "Gfx.hpp"

#include "Control.hpp"
#include "FPS.hpp"
#include "Music.hpp"
#include "NumericRanges.hpp"
#include "Profiler.hpp"
#include "Stopwatch.hpp"
#include "Taskman.hpp"
#include "Uptime.hpp"
#include "log.hpp"
