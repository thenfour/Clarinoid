

#pragma once

//#define CLARINOID_UNIT_TESTS

namespace TestUtils
{

template <typename T, size_t N>
constexpr size_t SizeofStaticArray(const T (&x)[N])
{
    return N;
}

static constexpr float FloatEpsilon = 0.000001f;

inline bool FloatEquals(float f1, float f2, float eps = FloatEpsilon)
{
    return fabs(f1 - f2) < eps;
}

} // namespace TestUtils


String ToString(void *p)
{
    static char x[20];
#ifdef CLARINOID_PLATFORM_X86 // for some reason snprintf() is not available in teensyduino
    std::snprintf(x, TestUtils::SizeofStaticArray(x), "%p", p);
#else
    sprintf(x, "%p", p);
#endif
    return String(x);
}

const char *ToString(bool p)
{
    if (p)
        return "true";
    return "false";
}

#ifdef CLARINOID_PLATFORM_X86 // for some reason snprintf() is not available in teensyduino
static int gLogIndent = 0;
template <typename... Args>
static void log(const std::string &format, Args &&...args)
{
    std::string fmt = std::string("[%x:%x] %s") + format + "\r\n";
    auto size = std::snprintf(nullptr,
                              0,
                              fmt.c_str(),
                              GetCurrentProcessId(),
                              GetCurrentThreadId(),
                              std::string(gLogIndent * 2, ' ').c_str(),
                              std::forward<Args>(args)...);
    std::string output(size + 2, '\0'); // to ensure the null-terminator
    output.resize(size);                // so the reported length is correct.
    std::snprintf(&output[0],
                  size,
                  fmt.c_str(),
                  GetCurrentProcessId(),
                  GetCurrentThreadId(),
                  std::string(gLogIndent * 2, ' ').c_str(),
                  std::forward<Args>(args)...);
    // OutputDebugStringA(output.c_str());
    Serial.println(output.c_str());
}

struct ScopeLog
{
    String mMsg;
    ScopeLog(const String &msg) : mMsg(msg)
    {
        // Serial.print("{ ");
        // Serial.println(msg);
        log("{ %s", msg.c_str());
        gLogIndent++;
    }
    ~ScopeLog()
    {
        gLogIndent--;
        log("} %s", mMsg.c_str());
        // Serial.print("} ");
        // Serial.println(mMsg);
    }
};

#else

void log(const String &s)
{
    if (!Serial)
    {
        Serial.begin(9600);
        while (!Serial)
            ;
    }
    Serial.println(s);
    delay(10);
}

#endif



static size_t gTestCount = 0;
static size_t gTestPassCount = 0;

void TestSummary()
{
    log("SUMMARY:");
    log("  %d PASS", gTestPassCount);
    log("  %d FAIL", gTestCount - gTestPassCount);
    log("  %d total", gTestCount);
}


inline void Test_(bool b, const char *str, bool expectFailure, const char *explanation) {
  gTestCount++;
  if (!b) {
    log("[%d] THIS FAILED: >>> %s <<<", gTestCount, str);
    if (expectFailure) {
      log("  but we expected failure: %s", explanation);
    }
    else {
      DebugBreak();
    }
  }
  else {
    gTestPassCount++;
    log("[%d (%d)] Pass: %s", gTestCount, gTestPassCount, str);
  }
}
template<typename Tx, typename Ty>
inline void TestEq_(const Tx& x, const Ty& y, const char *str, bool expectFailure, const char *explanation) {
  Test_(x == y, str, expectFailure, explanation);
}

inline void TestFloatEq_(const float& x, const float& y, const char *str, bool expectFailure, const char *explanation) {
    Test_(TestUtils::FloatEquals(x, y), str, expectFailure, explanation);
}

inline void TestFloatClose_(const float& x, const float& y, const float& eps, const char *str, bool expectFailure, const char *explanation) {
    Test_(TestUtils::FloatEquals(x, y, eps), str, expectFailure, explanation);
}

#define Test(x) (Test_(x, #x, false, ""))
#define TestEq(x, y) (TestEq_(x, y, #x " EQ " #y, false, ""))
#define TestFloatEq(x, y) (TestFloatEq_(x, y, #x " FLEQ " #y, false, ""))
#define TestFloatClose(x, y, eps) (TestFloatClose_(x, y, eps, #x " FLCLOSE " #y, false, ""))
#define TestExpectFailure(x,y) (Test_(x, #x, true, y))
