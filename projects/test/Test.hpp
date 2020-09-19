

#pragma once

//#define CLARINOID_UNIT_TESTS
#include <clarinoid/basic/Basic.hpp>

static size_t gTestCount = 0;
static size_t gTestPassCount = 0;

inline void Test_(bool b, const char *str) {
  gTestCount++;
  if (!b) {
    cc::log("[%d] THIS FAILED: >>> %s <<<", gTestCount, str);
    DebugBreak();
  }
  else {
    gTestPassCount++;
    cc::log("[%d] Pass: %s", gTestCount, str);
  }
}

#define Test(x) (Test_(x, #x))

