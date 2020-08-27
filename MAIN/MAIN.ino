// Teensy 4.0
// 600mhz
// "Faster"

// https://forum.pjrc.com/threads/57932-Latency-what-s-new
//#define AUDIO_BLOCK_SAMPLES 1024 // this seems to have no effect; may need to fork

//============================================================
/////////////////////////////////////////////////////////////////////////////////////////////////

// a sort of exception display mechanism.
bool gIsCrashed = false;
String gCrashMessage;

static inline void Die(const String& msg) {
  gCrashMessage = msg;
  gIsCrashed = true;
}

#define CCASSERT(x) if (!(x)) { Die(String("!Assert! ") + __FILE__ + ":" + (int)__LINE__); }

#include "Shared_CCSwitch.h"
#include "Shared_CCLeds.h"

#include "AppSettings.h"

#include "Shared_CCTxRx.h"

#include "CCEWIApplication.h"
CCEWIApp gApp;

#include "CCDisplay.h"

#include "CCMenuDebug.h"
#include "CCMenu.h"

HarmSettingsApp gHarmSettingsApp;
SystemSettingsApp gSystemSettingsApp;
TouchKeyGraphs gTouchKeyApp;
MetronomeSettingsApp gMetronomeApp;
DebugMenuApp gDebugApp;
SynthSettingsApp gSynthSettingsApp;


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
    Serial.println(gCrashMessage);
    gDisplay.mDisplay.println("!EXCEPTION!");
    gDisplay.mDisplay.println(gCrashMessage);
    gDisplay.mDisplay.display();
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

  if (UpdateObjectCount > MaxUpdateObjects) {
    CCPlot(String("Too many update objects!") + UpdateObjectCount);
  }

  UpdateUpdateObjects();

  uint32_t n = micros();
  if (n > m && (n - m) > gLongestLoopMicros) {
    gLongestLoopMicros = n - m;
  }
  
  gLoopExitMicros = micros();
}
