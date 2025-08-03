
#pragma once

#ifdef CLARINOID_PLATFORM_X86
#include "../x86/ArduinoEmu.hpp"
#endif


// language basics, syntax, types, fundamental helpers
#include "Binary.hpp"
#include "Teensy.hpp"
#include "Array.hpp"
#include "assert.hpp"
#include "BaseDefs.hpp"
#include "Memory.hpp"

#include "function.hpp"
#include "Enum.hpp"
#include "Tristate.hpp"

#include "Util.hpp"
#include "MovingAverage.hpp"
#include "CircularArray.hpp"
#include "SortedArray.hpp"

#include "string/format.hpp"

#include "./Numeric.hpp"

#include "./Geometry.hpp"

#include "Gfx.hpp"

#include "log.hpp"
#include "Uptime.hpp"
#include "FPS.hpp"
#include "Stopwatch.hpp"
#include "Music.hpp"
#include "Profiler.hpp"
#include "Taskman.hpp"
#include "Control.hpp"
#include "NumericRanges.hpp"
