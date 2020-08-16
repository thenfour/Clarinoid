// Teensy 4.0
// 600mhz

// Check synth & CCEWIControl for parameters

// This main sketch pretty much just glues the application to the display & LEDs

// https://forum.pjrc.com/threads/57932-Latency-what-s-new
//#define AUDIO_BLOCK_SAMPLES 1024 // this seems to have no effect; may need to fork

//============================================================

#include "Shared_CCSwitch.h"
#include "Shared_CCLeds.h"
#include "Shared_CCTxRx.h"

#include "CCEWIApplication.h"
#include "CCDisplay.h"
#include "CCMenuDebug.h"

CCEWIApp gApp;
CCDisplay gDisplay(gApp);

ScreensaverMenuApp gScreensaverApp(gDisplay, gApp);
DebugMenuApp gDebugApp(gDisplay, gApp);

CCLeds leds(10, 2, 10, true);
CCThrottlerT<20> ledThrottle;


void setup() {
  Serial.begin(9600);
  SetupUpdateObjects();
}

bool firstLoop = true;
uint32_t gLoopExitMicros = micros();

void loop() {
  uint32_t m = micros();
  if (firstLoop) {
    firstLoop = false;
  } else {
    if (m > gLoopExitMicros && (m - gLoopExitMicros) > gLongestBetweenLoopMicros) {
      gLongestBetweenLoopMicros = m - gLoopExitMicros;
    }
  }

  UpdateUpdateObjects();

  if (ledThrottle.IsReady())
  {
    //auto activityColor = col(gGeneralActivityIndicator.GetState(), 1, 4);
    leds.setPixelColor(0, 0, 0, 0);
    leds.setPixelColor(1, col(gLHRXErrorIndicator.GetState(), 0, 1), col(gLHTXIndicator.GetState(), 0, 1), col(gLHRXIndicator.GetState(), 0, 1));
    leds.setPixelColor(2, col(gRHRXErrorIndicator.GetState(), 0, 1), col(gRHTXIndicator.GetState(), 0, 1), col(gRHRXIndicator.GetState(), 0, 1));
    leds.setPixelColor(3, col(gSynth.IsHarmonizerEnabled(), 0, 1), 0, col(gMidiActivityIndicator.GetState(), 0, 1));
    
    leds.setPixelColor(4, col(gEWIControl.mTranspose != 0, 0, 1), 0, 0);
    leds.setPixelColor(5, col(gEWIControl.mTranspose < 0, 0, 1), col(gEWIControl.mTranspose > 0, 0, 1), 0);

    // 6 = off
    // 7 = off

    leds.setPixelColor(8, col(gEncButton.IsPressed()), 0, col(gEncIndicator.GetState(), 0, 1));
    leds.setPixelColor(9, 0, gVolumePot.GetValue01() * 6, col(gVolIndicator.GetState(), 0, 6));
    leds.show();
  }

  uint32_t n = micros();
  if (n > m && (n - m) > gLongestLoopMicros) {
    gLongestLoopMicros = n - m;
  }
  
  gLoopExitMicros = micros();
}
