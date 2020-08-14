// Teensy LC
// 48mhz

//#define LH
#define RH
#define EWI_LHRH_VERSION_MAJOR 0
#define EWI_LHRH_VERSION_MINOR 1

//============================================================

#include "CCSwitch.h"
#include "CCLeds.h"
#include "CCTouchKey.h"
#include "CCPressure.h"
#include "CCTransmit.h"

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

CCPressure wind(A0);
CCPressure bite(A6);
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
CCPressure pitchDown(A0);
CCOnOffSwitch oooButton1(11, 10, 5);
CCOnOffSwitch oooButton2(12, 10, 5);
#endif // LH/RH

CCThrottler ledThrottle(20);

CCTransmit gTransmitter(Serial2);
LHRHPayload gPayload;
bool gDirty = false;
ActivityLED gTXIndicator(60/* should be no smaller than ledThrottle*/);

AsymmetricActivityLED gGeneralActivityIndicator(750, 250);

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
  //gPayload.payloadSize = sizeof(gPayload);
  //gPayload.versionMajor = EWI_LHRH_VERSION_MAJOR;
  //gPayload.versionMinor = EWI_LHRH_VERSION_MINOR;
  /*gPayload.module =
#ifdef LH
    LHRHPAYLOAD_MODULE_LH;
#else // RH
    LHRHPAYLOAD_MODULE_RH;
#endif // LH/RH
*/
}

void loop() {
  UpdateUpdateObjects();
  gGeneralActivityIndicator.Touch();

  // construct the payload & check if dirty / needs to be sent.
  // only unset "dirty" if we actually send the packet.
//  CaptureButton(key1, gPayload.data.key1);
//  CaptureButton(key2, gPayload.data.key2);
//  CaptureButton(key3, gPayload.data.key3);
//  CaptureButton(key4, gPayload.data.key4);
//  CaptureButton(key5, gPayload.data.key5);
//  CaptureButton(key6, gPayload.data.key6);
#ifdef LH
  gDirty = true;
  gPayload.data.pressure1 = wind.Value01();
  gPayload.data.pressure2 = bite.Value01();

  CaptureButton(backButton, gPayload.data.button1);
  CaptureButton(octave1, gPayload.data.octave1);
  CaptureButton(octave1, gPayload.data.octave2);
  CaptureButton(octave1, gPayload.data.octave3);
  CaptureButton(octave1, gPayload.data.octave4);
#else // RH
  gDirty = true;
  //gPayload.data.pressure1 = pitchDown.Value01();

  //CaptureButton(oooButton1, gPayload.data.button1);
  //CaptureButton(oooButton2, gPayload.data.button2);
#endif // LH/RH

  if (gDirty) {
    // collect remaining stats before sending
    //gPayload.capGlobalMin = mTouchKeyGlobalMin;
    //gPayload.capGlobalMax = mTouchKeyGlobalMax;
    
    gTransmitter.Send(gPayload);
    gTXIndicator.Touch();
    
    Serial.println(String("sent ") + ToString(gPayload));
    gDirty = false;
  }

  if (ledThrottle.IsReady())
  {
#ifdef LH
    leds.setPixelColor(0, 0, col(gGeneralActivityIndicator.GetState(), 1, 4), 0); // green = LH
    leds.setPixelColor(1, col(gTXIndicator.GetState(), 0, 1), 0, col(!gTXIndicator.GetState(), 0, 1));
    leds.setPixelColor(2, 0, col(bite.Value01()), 0);
    leds.setPixelColor(3, 0, col(wind.Value01()), 0);
    leds.setPixelColor(4, 0, 0, col(key1.IsPressed()));
    leds.setPixelColor(5, 0, 0, col(key2.IsPressed()));
    leds.setPixelColor(6, 0, col(octave4.IsPressed()), col(key3.IsPressed()));
    leds.setPixelColor(7, 0, col(octave3.IsPressed()), col(key4.IsPressed()));
    leds.setPixelColor(8, 0, col(octave2.IsPressed()), col(key5.IsPressed()));
    leds.setPixelColor(9, col(backButton.IsPressed()), col(octave1.IsPressed()), col(key6.IsPressed()));
#else // RH
    leds.setPixelColor(0, 0, 0, col(gGeneralActivityIndicator.GetState(), 1, 4)); // blue = RH
    leds.setPixelColor(1, col(gTXIndicator.GetState(), 0, 1), 0, col(!gTXIndicator.GetState(), 0, 1));
    leds.setPixelColor(3, 0, col(pitchDown.Value01(), 0, 10), 0);
    leds.setPixelColor(4, 0, 0, col(key1.IsPressed()));
    leds.setPixelColor(5, 0, 0, col(key2.IsPressed()));
    leds.setPixelColor(6, 0, 0, col(key3.IsPressed()));
    leds.setPixelColor(7, 0, 0, col(key4.IsPressed()));
    leds.setPixelColor(8, 0, col(oooButton1.IsPressed()), col(key5.IsPressed()));
    leds.setPixelColor(9, 0, col(oooButton2.IsPressed()), col(key6.IsPressed()));
#endif // LH/RH
    leds.show();
  }

  delay(300); // yield interrupts
}
