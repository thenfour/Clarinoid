#pragma once

#include <cstdio>
#include <stdio.h>

namespace clarinoid
{

//String ToString(void *p)
//{
//    static char x[20];
//#ifdef CLARINOID_PLATFORM_X86 // for some reason snprintf() is not available in teensyduino
//    std::snprintf(x, SizeofStaticArray(x), "%p", p);
//#else
//    sprintf(x, "%p", p);
//#endif
//    return String(x);
//}
//
//const char *ToString(bool p)
//{
//    if (p)
//        return "true";
//    return "false";
//}
//
//#ifdef CLARINOID_PLATFORM_X86 // for some reason snprintf() is not available in teensyduino
//static int gLogIndent = 0;
//template <typename... Args>
//static void log(const std::string &format, Args &&... args)
//{
//    std::string fmt = std::string("[%x:%x] %s") + format + "\r\n";
//    auto size = std::snprintf(nullptr,
//                              0,
//                              fmt.c_str(),
//                              GetCurrentProcessId(),
//                              GetCurrentThreadId(),
//                              std::string(gLogIndent * 2, ' ').c_str(),
//                              std::forward<Args>(args)...);
//    std::string output(size + 2, '\0'); // to ensure the null-terminator
//    output.resize(size);                // so the reported length is correct.
//    std::snprintf(&output[0],
//                  size,
//                  fmt.c_str(),
//                  GetCurrentProcessId(),
//                  GetCurrentThreadId(),
//                  std::string(gLogIndent * 2, ' ').c_str(),
//                  std::forward<Args>(args)...);
//    // OutputDebugStringA(output.c_str());
//    Serial.println(output.c_str());
//}
//
//struct ScopeLog
//{
//    String mMsg;
//    ScopeLog(const String &msg) : mMsg(msg)
//    {
//        // Serial.print("{ ");
//        // Serial.println(msg);
//        log("{ %s", msg.c_str());
//        gLogIndent++;
//    }
//    ~ScopeLog()
//    {
//        gLogIndent--;
//        log("} %s", mMsg.c_str());
//        // Serial.print("} ");
//        // Serial.println(mMsg);
//    }
//};
//
//#else
//
//void log(const String &s)
//{
//    if (!Serial)
//    {
//        Serial.begin(9600);
//        while (!Serial)
//            ;
//    }
//    Serial.println(s);
//    delay(10);
//}
//
//#endif

} // namespace clarinoid