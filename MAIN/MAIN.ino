// Teensy 4.0
// 600mhz
// 

#define EWI_MAIN_VERSION_NUMBER "0.1"
#define EWI_LHRH_VERSION EWI_LHRH_VERSION_NUMBER "_LH" // only needed for size calculation

//============================================================

#include "CCSwitch.h"
#include "CCLeds.h"
#include "CCPotentiometer.h"
#include "CCDisplay.h"
#include "CCEncoder.h"
#include "CCSynth.h"
#include "CCReceive.h"

//============================================================
CCLeds leds(10, 2, 10, true);
CCThrottler ledThrottle(20);

CCSynth gSynth;

CCOnOffSwitch gEncButton(3, 10, 5);
CCEncoder gEnc(4, 5);
TransientActivityLED gEncIndicator(60, 200);

CCVolumePot gVolumePot(A8);
TransientActivityLED gVolIndicator(40, 200);

//CCReceiver gLHReceiver(Serial1);
ActivityLED gLHRXIndicator(60);

CCReceiver gRHReceiver(Serial4);
ActivityLED gRHRXIndicator(60);

AsymmetricActivityLED gGeneralActivityIndicator(750, 250);
CCDisplay gDisplay;

framerateCalculator gFramerate;

//============================================================

void setup() {
  Serial.begin(9600);
  SetupUpdateObjects();
}

void loop() {
  // if the serial RX buffer fills up, you need to search for the marker of the beginning of the payload.
  UpdateUpdateObjects();
  gFramerate.onFrame();
  
  gGeneralActivityIndicator.Touch();
  if (gEnc.IsDirty()) {
    gEncIndicator.Touch();
  }
  if (gVolumePot.IsDirty()) {
    gVolIndicator.Touch();
    gSynth.SetGain(gVolumePot.GetValue01());
  }

  // gather up serial receive

  // now drive synth

  // drive MIDI

  if (ledThrottle.IsReady())
  {
    auto activityColor = col(gGeneralActivityIndicator.GetState(), 1, 4);
    leds.setPixelColor(0, 0, activityColor, activityColor); // cyan = MAIN
    leds.setPixelColor(1, 0, 0, col(gLHRXIndicator.GetState(), 0, 4)); // TODO: red = errors
    leds.setPixelColor(2, 0, 0, col(gRHRXIndicator.GetState(), 0, 4)); // TODO: red = errors
    // 3 midiTX
    // 4 (off)
    // 5 (off)
    // 6 (off)
    // 7 (off)
    leds.setPixelColor(8, 0, gVolumePot.GetValue01() * 16, col(gVolIndicator.GetState(), 0, 32));
    leds.setPixelColor(9, col(gEncButton.IsPressed()), 0, col(gEncIndicator.GetState(), 0, 16));
    leds.show();

    // BEWARE: since this is not run every frame, you can miss events.
    gDisplay.mDisplay.clearDisplay();
    gDisplay.mDisplay.setTextSize(1);
    gDisplay.mDisplay.setTextColor(WHITE);
    gDisplay.mDisplay.setCursor(0,0);
    //gDisplay.mDisplay.println(String("fps:") + gFramerate.getFPS());
    CCReceiver& rx = (gEncButton.IsPressed()) ? gRHReceiver : gRHReceiver;
    gDisplay.mDisplay.println(String((gEncButton.IsPressed()) ? "LH" : "RH") + " ok: " + rx.mRxSuccess + " #" + rx.mLatestData.serial);
    gDisplay.mDisplay.print(String("E:sz  ") + rx.mSizeErrors);
    gDisplay.mDisplay.println(String("E:chk ") + rx.mChecksumErrors);
    gDisplay.mDisplay.display();
  }

  delay(100); // yield for serial data
}
