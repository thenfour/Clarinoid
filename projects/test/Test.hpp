

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

inline void TestFloatEq_(const float& x, const float& y, const char *str, bool expectFailure, const char *explanation) {
  Test_(clarinoid::FloatEquals(x, y), str, expectFailure, explanation);
}

inline void TestFloatClose_(const float& x, const float& y, const float& eps, const char *str, bool expectFailure, const char *explanation) {
  Test_(clarinoid::FloatEquals(x, y, eps), str, expectFailure, explanation);
}

#define Test(x) (Test_(x, #x, false, ""))
#define TestEq(x, y) (TestEq_(x, y, #x " EQ " #y, false, ""))
#define TestFloatEq(x, y) (TestFloatEq_(x, y, #x " FLEQ " #y, false, ""))
#define TestFloatClose(x, y, eps) (TestFloatClose_(x, y, eps, #x " FLCLOSE " #y, false, ""))
#define TestExpectFailure(x,y) (Test_(x, #x, true, y))
