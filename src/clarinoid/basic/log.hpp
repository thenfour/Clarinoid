#pragma once

#include <cstdio>
#include <stdio.h>





String ToString(void* p) {
  static char x[20];
  sprintf(x, "%p", p);
  return String(x);
}

const char *ToString(bool p) {
  if (p) return "true";
  return "false";
}

 namespace cc
 {
   template <typename ...Args>
   static void log(const std::string& format, Args && ...args)
   {
     std::string fmt = std::string("[%x:%x] ") + format + "\r\n";
     auto size = std::snprintf(nullptr, 0, fmt.c_str(), GetCurrentProcessId(), GetCurrentThreadId(), std::forward<Args>(args)...);
     std::string output(size + 2, '\0');// to ensure the null-terminator
     output.resize(size);// so the reported length is correct.
     std::sprintf(&output[0], fmt.c_str(), GetCurrentProcessId(), GetCurrentThreadId(), std::forward<Args>(args)...);
     //OutputDebugStringA(output.c_str());
     Serial.print(output.c_str());
   }
}

struct ScopeLog
{
  String mMsg;
  ScopeLog(const String& msg) : mMsg(msg)
  {
    Serial.print("{ ");
    Serial.println(msg);
  }
  ~ScopeLog()
  {
    Serial.print("} ");
    Serial.println(mMsg);
  }
};
