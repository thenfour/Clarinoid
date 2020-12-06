

#pragma once

//#define CLARINOID_UNIT_TESTS
#include <clarinoid/basic/Basic.hpp>

static size_t gTestCount = 0;
static size_t gTestPassCount = 0;

inline void Test_(bool b, const char *str, bool expectFailure, const char *explanation) {
  gTestCount++;
  if (!b) {
    clarinoid::log("[%d] THIS FAILED: >>> %s <<<", gTestCount, str);
    if (expectFailure) {
      clarinoid::log("  but we expected failure: %s", explanation);
    }
    else {
      DebugBreak();
    }
  }
  else {
    gTestPassCount++;
    clarinoid::log("[%d (%d)] Pass: %s", gTestCount, gTestPassCount, str);
  }
}
template<typename Tx, typename Ty>
inline void TestEq_(const Tx& x, const Ty& y, const char *str, bool expectFailure, const char *explanation) {
  Test_(x == y, str, expectFailure, explanation);
}

#define Test(x) (Test_(x, #x, false, ""))
#define TestEq(x, y) (TestEq_(x, y, #x " EQ " #y, false, ""))
#define TestExpectFailure(x,y) (Test_(x, #x, true, y))
