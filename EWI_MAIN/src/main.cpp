// Teensy 4.0
// 600mhz
// "Faster"

// https://forum.pjrc.com/threads/57932-Latency-what-s-new
// This does have an effect, but you need to make sure you restart arduino to do a full rebuild (there's probably a way to do it in the IDE i dunno).
// BUT, 128 is nice and stable. Anything less and I find random artifacts, sometimes things get ugly. When it comes to reducing latency, I prefer
// to try and optimize elsewhere, because this is a bit unruly. I'd rather make changes to the more controlled code.
#define AUDIO_BLOCK_SAMPLES 128

static constexpr int RESOLUTION_X = 128;
static constexpr int RESOLUTION_Y = 32;

//============================================================
/////////////////////////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>

// a sort of exception display mechanism.
bool gIsCrashed = false;
String gCrashMessage;

static inline void Die(const String& msg) {
  gCrashMessage = msg;
  gIsCrashed = true;
}

#define CCASSERT(x) if (!(x)) { Die(String("!Assert! ") + __FILE__ + ":" + (int)__LINE__); }

#include <Shared_CCSwitch.h>
#include <Shared_CCLeds.h>
#include <Shared_CCTxRx.h>

#include "AppSettings.h"
#include "CCEWIApplication.h"
#include "CCDisplay.h"
#include "MenuApps.hpp"

void setup() {
  Serial.begin(9600);
  SetupUpdateObjects();
}

bool firstLoop = true;
uint32_t gLoopExitMicros = micros();

void loop() {

  if (gIsCrashed) {
    if (!gDisplay.mIsSetup) {
      gDisplay.setup();
    }
    gDisplay.mDisplay.clearDisplay();
    gDisplay.mDisplay.setCursor(0,0);
    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
    gDisplay.mDisplay.println("!EXCEPTION!");
    gDisplay.mDisplay.println(gCrashMessage);
    gDisplay.mDisplay.display();
    Serial.begin(9600);
    while (!Serial);
    Serial.println(gCrashMessage);
    while(true) {
      delay(100);
    }
    return;
  }
  
  uint32_t m = micros();
  if (firstLoop) {
    firstLoop = false;
  } else {
    if (m > gLoopExitMicros && (m - gLoopExitMicros) > gLongestBetweenLoopMicros) {
      gLongestBetweenLoopMicros = m - gLoopExitMicros;
    }
  }

  UpdateUpdateObjects();

  uint32_t n = micros();
  if (n > m && (n - m) > gLongestLoopMicros) {
    gLongestLoopMicros = n - m;
  }
  
  gLoopExitMicros = micros();
}
