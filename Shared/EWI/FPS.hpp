#pragma once

//////////////////////////////////////////////////////////////////////
// from https://github.com/thenfour/demos/blob/fa35a727549c67222d2829310e2c427c09e347e7/demo3/src/core/algorithm.hpp
struct framerateCalculator {

  framerateCalculator() {
    g_TicksPerSecond = 1000;
    g_Time = millis();
    Framerate = 0.f;
    set(FramerateSecPerFrame, SizeofStaticArray(FramerateSecPerFrame), 0.f);
    FramerateSecPerFrameIdx = 0;
    FramerateSecPerFrameAccum = 0.0f;
  }

  void onFrame() {
    uint32_t current_time = millis();
    float DeltaTime = (float)(current_time - g_Time) / g_TicksPerSecond;
    g_Time = current_time;
    FramerateSecPerFrameAccum += DeltaTime - FramerateSecPerFrame[FramerateSecPerFrameIdx];
    FramerateSecPerFrame[FramerateSecPerFrameIdx] = DeltaTime;
    FramerateSecPerFrameIdx = (FramerateSecPerFrameIdx + 1) % SizeofStaticArray(FramerateSecPerFrame);
    Framerate = 1.0f / (FramerateSecPerFrameAccum / (float)SizeofStaticArray(FramerateSecPerFrame));
  }

  float getFPS() {
    return Framerate;
  }

private:

  uint32_t g_TicksPerSecond;
  uint32_t g_Time;
  float Framerate;
  float                   FramerateSecPerFrame[120];
  int                     FramerateSecPerFrameIdx;
  float                   FramerateSecPerFrameAccum;
};
