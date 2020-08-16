// Teensy LC
// 48mhz

//#define LH
#define RH

//============================================================

#include "Shared_CCTxRx.h"
#include "Shared_CCSwitch.h"
#include "Shared_CCLeds.h"
#include "CCTouchKey.h"
#include "CCPressure.h"

// common
CCLeds leds(10, 2, 10,
#ifdef LH
  false /* reversed */
#else // RH
  true /* reversed */
#endif // LH/RH
  );

#ifdef LH
CCTouchKey key1(15);
CCTouchKey key2(16);
CCTouchKey key3(17);
CCTouchKey key4(18);
CCTouchKey key5(19);
CCTouchKey key6(22);

CCBreathSensor wind(A0);
CCBiteSensor bite(A6);
CCOnOffSwitch backButton(6, 10, 5);
CCTouchKey octave1(0);
CCTouchKey octave2(1);
CCTouchKey octave3(3);
CCTouchKey octave4(4);
#else // RH
CCTouchKey key1(22);
CCTouchKey key2(15);
CCTouchKey key3(16);
CCTouchKey key4(17);
CCTouchKey key5(18);
CCTouchKey key6(19);
CCPitchStripSensor pitchDown(A0);
CCOnOffSwitch oooButton1(11, 10, 5);
CCOnOffSwitch oooButton2(12, 10, 5);
#endif // LH/RH

CCThrottler ledThrottle(20);

CCLHRHTxRx gTxRx(Serial2);
LHRHPayload gPayload;
bool gDirty = false;
TransientActivityLED gTXIndicator(800, 800);
TransientActivityLED gRXIndicator(800, 800);

LHRHLEDMode gLEDMode = LHRHLEDMode::Debug;

// this cannot be a templated function, because you cannot bind bitfields like that.
#define CaptureButton(pb, dest) \
{ \
  gDirty |= pb.IsDirty(); \
  dest = pb.IsPressed(); \
}

void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  SetupUpdateObjects();
}

void loop() {
  UpdateUpdateObjects();

  // construct the payload & check if dirty / needs to be sent.
  // only unset "dirty" if we actually send the packet.
  CaptureButton(key1, gPayload.data.key1);
  CaptureButton(key2, gPayload.data.key2);
  CaptureButton(key3, gPayload.data.key3);
  CaptureButton(key4, gPayload.data.key4);
  CaptureButton(key5, gPayload.data.key5);
  CaptureButton(key6, gPayload.data.key6);
#ifdef LH
  gDirty = true;
  gPayload.data.pressure1 = wind.GetRawValue();
  gPayload.data.pressure2 = bite.GetRawValue();

  CaptureButton(backButton, gPayload.data.button1);
  CaptureButton(octave1, gPayload.data.octave1);
  CaptureButton(octave2, gPayload.data.octave2);
  CaptureButton(octave3, gPayload.data.octave3);
  CaptureButton(octave4, gPayload.data.octave4);
#else // RH
  gDirty = true;
  gPayload.data.pressure1 = pitchDown.GetRawValue();

  CaptureButton(oooButton1, gPayload.data.button1);
  CaptureButton(oooButton2, gPayload.data.button2);
#endif // LH/RH

  if (gDirty) {
    gTxRx.Send(gPayload);
    gTXIndicator.Touch();
    gDirty = false;
  }

  if (gTxRx.mHaveNewData) {
    gRXIndicator.Touch();
    gLEDMode = gTxRx.mReceivedData.data.ledMode;
  }

  if (ledThrottle.IsReady())
  {
    switch (gLEDMode) {
    case LHRHLEDMode::Off:
      for (int i = 0; i < 10; ++ i) {
        leds.setPixelColor(i, 0, 0, 0);
      }
      break;
      case LHRHLEDMode::Debug:
#ifdef LH
      leds.setPixelColor(0, col(gTXIndicator.GetState(), 0, 1), col(gRXIndicator.GetState(), 0, 1), col(gRXIndicator.GetState(), 0, 1));
      // 1 = off
      leds.setPixelColor(2, 0, col(bite.Value01Estimate(), 8), 0);
      leds.setPixelColor(3, 0, col(wind.Value01Estimate(), 8), 0);
      leds.setPixelColor(4, 0, 0, col(key1.IsPressed(), 0, 1));
      leds.setPixelColor(5, 0, 0, col(key2.IsPressed(), 0, 1));
      leds.setPixelColor(6, 0, col(octave4.IsPressed(), 0, 1), col(key3.IsPressed(), 0, 1));
      leds.setPixelColor(7, 0, col(octave3.IsPressed(), 0, 1), col(key4.IsPressed(), 0, 1));
      leds.setPixelColor(8, 0, col(octave2.IsPressed(), 0, 1), col(key5.IsPressed(), 0, 1));
      leds.setPixelColor(9, col(backButton.IsPressed(), 0, 12), col(octave1.IsPressed(), 0, 1), col(key6.IsPressed(), 0, 1));
#else // RH
      leds.setPixelColor(0, col(gTXIndicator.GetState(), 0, 1), col(gRXIndicator.GetState(), 0, 1), col(gRXIndicator.GetState(), 0, 1));
      // 1 = off
      // 2 = off
      leds.setPixelColor(3, 0, col(pitchDown.Value01Estimate(), 0, 8), 0);
      leds.setPixelColor(4, 0, 0, col(key1.IsPressed(), 0, 1));
      leds.setPixelColor(5, 0, 0, col(key2.IsPressed(), 0, 1));
      leds.setPixelColor(6, 0, 0, col(key3.IsPressed(), 0, 1));
      leds.setPixelColor(7, 0, 0, col(key4.IsPressed(), 0, 1));
      leds.setPixelColor(8, 0, col(oooButton1.IsPressed(), 0, 1), col(key5.IsPressed(), 0, 1));
      leds.setPixelColor(9, 0, col(oooButton2.IsPressed(), 0, 1), col(key6.IsPressed(), 0, 1));
#endif // LH/RH
      break;
    }
    leds.show();
  }

  //delay(50);
}
