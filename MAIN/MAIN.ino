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

CCEWIApp gApp;
CCDisplay gDisplay(gApp);

CCLeds leds(10, 2, 10, true);
CCThrottlerT<20> ledThrottle;

//============================================================

//============================================================

void setup() {
  Serial.begin(9600);
  SetupUpdateObjects();
}

void loop() {
  UpdateUpdateObjects();

  if (ledThrottle.IsReady())
  {
    //auto activityColor = col(gGeneralActivityIndicator.GetState(), 1, 4);
    leds.setPixelColor(0, 0, 0, 0);
    leds.setPixelColor(1, col(gLHRXErrorIndicator.GetState(), 0, 4), 0, col(gLHRXIndicator.GetState(), 0, 4));
    leds.setPixelColor(2, 0, col(gLHTXIndicator.GetState(), 0, 4), 0);
    leds.setPixelColor(3, col(gRHRXErrorIndicator.GetState(), 0, 4), 0, col(gRHRXIndicator.GetState(), 0, 4));
    leds.setPixelColor(4, 0, col(gRHTXIndicator.GetState(), 0, 4), 0);
    leds.setPixelColor(5, 0, col(gMidiActivityIndicator.GetState(), 0, 4), col(gMidiActivityIndicator.GetState(), 0, 4));
    // 6 (off)
    // 7 (off)
    leds.setPixelColor(8, 0, gVolumePot.GetValue01() * 16, col(gVolIndicator.GetState(), 0, 32));
    leds.setPixelColor(9, col(gEncButton.IsPressed()), 0, col(gEncIndicator.GetState(), 0, 16));
    leds.show();
  }

  //delay(50);
}
