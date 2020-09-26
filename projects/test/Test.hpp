

#pragma once

//#define CLARINOID_UNIT_TESTS
#include <clarinoid/basic/Basic.hpp>

static size_t gTestCount = 0;
static size_t gTestPassCount = 0;

inline void Test_(bool b, const char *str, bool expectFailure, const char *explanation) {
  gTestCount++;
  if (!b) {
    cc::log("[%d] THIS FAILED: >>> %s <<<", gTestCount, str);
    if (expectFailure) {
      cc::log("  but we expected failure: %s", explanation);
    }
    else {
      DebugBreak();
    }
  }
  else {
    gTestPassCount++;
    cc::log("[%d] Pass: %s", gTestCount, str);
  }
}

#define Test(x) (Test_(x, #x, false, ""))

#define TestExpectFailure(x,y) (Test_(x, #x, true, y))
