// Teensy LC
// 48mhz

#define CLARINOID_PLATFORM_TEENSY
#define CLARINOID_MODULE_LHRH
#define LH
//#define RH

//============================================================

// arduino-ish libraries
#include <Arduino.h>
#include <EasyTransfer.h>
#include <FastCRC.h>
#include <Bounce.h>
#include <Adafruit_NeoPixel.h>
#include <touchablePin.h>

uint32_t gFrameNumber = 0;

#include <clarinoid/comm/ClarinoidTxRx.hpp>
#include <clarinoid/components/Switch.hpp>
#include <clarinoid/components/Leds.hpp>

static int gFocusedKeyIndex = -1;

int gFocusedKeyData = -1; // which key set the following data...
uint32_t gFocusedTouchReadMicros;
uint32_t gFocusedTouchReadValue;
uint32_t gFocusedTouchReadUntouchedMicros;
uint32_t gFocusedTouchReadThresholdMicros;

#include <clarinoid/components/TouchKey.hpp>
#include <clarinoid/components/AirPressorSensor.hpp>

// common
CCLeds leds(10, 2,
#ifdef LH
  false /* reversed */
#else // RH
  true /* reversed */
#endif // LH/RH
  );

#ifdef LH
CCTouchKey key1(15, KEY_LHX1);
CCTouchKey key2(16, KEY_LH1);
CCTouchKey key3(17, KEY_LH2);
CCTouchKey key4(18, KEY_LH3);
CCTouchKey key5(19, KEY_LH4);
CCTouchKey key6(22, KEY_LHX2);

CCBreathSensor wind(A0);
CCBiteSensor bite(A6);
CCOnOffSwitch backButton(6, 5);
CCTouchKey octave1(0, KEY_O1);
CCTouchKey octave2(1, KEY_O2);
CCTouchKey octave3(3, KEY_O3);
CCTouchKey octave4(4, KEY_O4);
#else // RH
CCTouchKey key1(22, KEY_RHX1);
CCTouchKey key2(15, KEY_RH1);
CCTouchKey key3(16, KEY_RH2);
CCTouchKey key4(17, KEY_RH3);
CCTouchKey key5(18, KEY_RH4);
CCTouchKey key6(19, KEY_RHX2);
CCPitchStripSensor pitchDown(A0);
CCOnOffSwitch oooButton1(11, 5);
CCOnOffSwitch oooButton2(12, 5);
#endif // LH/RH

CCThrottler ledThrottle(20);

CCThrottlerT<1> gCommThrottle;
CCLHRHTxRx gTxRx(Serial2);
LHRHPayload gPayload;

TransientActivityLED gTXIndicator(250, 250);
TransientActivityLED gRXIndicator(250, 250);

LHRHLEDMode gLEDMode = LHRHLEDMode::Debug;

// this cannot be a templated function, because you cannot bind bitfields like that.
inline void CaptureButton(CCTouchKey& key, uint8_t& dest)
{
  dest = key.IsPressed() ? 1 : 0;
}

void setup() {
  Serial.begin(9600);

  pinMode(13, OUTPUT);
  SetupUpdateObjects();
}

void loop() {
  gFocusedKeyData = -1;
  gFrameNumber ++;
  UpdateUpdateObjects();

  if (gTxRx.mHaveNewData) {
    gRXIndicator.Touch();
    gLEDMode = gTxRx.mReceivedData.data.ledMode;
    gFocusedKeyIndex = gTxRx.mReceivedData.data.focusedTouchKey;
    switch(gTxRx.mReceivedData.data.cmd) {
    case CommandFromMain::None:
      break;
    case CommandFromMain::EnableOrangeLED:
      digitalWrite(13, HIGH);
      break;
    case CommandFromMain::DisableOrangeLED:
      digitalWrite(13, LOW);
      break;
    case CommandFromMain::ResetTouchKeys:
      gTouchKeyCalibrator.ResetAllKeys();
      break;
    case CommandFromMain::SetTouchMaxFactor:
      gTouchKeyCalibrator.SetMaxFactorOfAllKeys(gTxRx.mReceivedData.data.cmdFloatParam1);
      break;
    }
  }

  if (gFocusedKeyIndex >= 0 && gFocusedKeyIndex < (int)SizeofStaticArray(gKeyDesc)) {
#ifdef LH
    if (gKeyDesc[gFocusedKeyIndex].mLH) {
#else // RH
    if (!gKeyDesc[gFocusedKeyIndex].mLH) {
#endif // LHRH
      if (gFocusedKeyData == gFocusedKeyIndex) {
        gPayload.data.focusedKey = gFocusedKeyData;
        gPayload.data.focusedTouchReadMicros = ClampUint32ToUint16(gFocusedTouchReadMicros);
        gPayload.data.focusedTouchReadValue = ClampUint32ToUint16(gFocusedTouchReadValue);
        gPayload.data.focusedTouchReadUntouchedMicros = ClampUint32ToUint16(gFocusedTouchReadUntouchedMicros);
        gPayload.data.focusedTouchReadThresholdMicros = ClampUint32ToUint16(gFocusedTouchReadThresholdMicros);
      }
    }
  }

  // construct the payload & check if dirty / needs to be sent.
  // only unset "dirty" if we actually send the packet.
  BigKeyData keyData;
  CaptureButton(key1, keyData.keys[0]);
  CaptureButton(key2, keyData.keys[1]);
  CaptureButton(key3, keyData.keys[2]);
  CaptureButton(key4, keyData.keys[3]);
  CaptureButton(key5, keyData.keys[4]);
  CaptureButton(key6, keyData.keys[5]);
#ifdef LH
//  gDirty = true;
  gPayload.data.pressure1 = wind.GetRawValue();
  gPayload.data.pressure2 = bite.GetRawValue();

  keyData.SetButton1(backButton.IsCurrentlyPressed());
  CaptureButton(octave1, keyData.oct[0]);
  CaptureButton(octave2, keyData.oct[1]);
  CaptureButton(octave3, keyData.oct[2]);
  CaptureButton(octave4, keyData.oct[3]);
#else // RH
//  gDirty = true;
  gPayload.data.pressure1 = pitchDown.GetRawValue();

  keyData.SetButton1(oooButton1.IsCurrentlyPressed());
  keyData.SetButton2(oooButton2.IsCurrentlyPressed());
  //gPayload.data.button1 = oooButton1.IsCurrentlyPressed();
  //gPayload.data.button2 = oooButton2.IsCurrentlyPressed();
#endif // LH/RH

  gPayload.data.packedKeys = keyData.Serialize();

  //if (gCommThrottle.IsReady()) {
    gTxRx.Send(gPayload);
    gTXIndicator.Touch();
  //}

  if (ledThrottle.IsReady())
  {
    switch (gLEDMode) {
    case LHRHLEDMode::Off:
      for (int i = 0; i < 10; ++ i) {
        leds.setPixelColor(i, 0, 0, 0);
      }
      break;
    case LHRHLEDMode::Minimal:
      leds.setPixelColor(0, col(gTXIndicator.GetState()), col(gRXIndicator.GetState()), col(gRXIndicator.GetState()));
      break;
    case LHRHLEDMode::MainControlled:
    {
      auto& d = gTxRx.mReceivedData.data;
      for (int i = 0; i < 10; ++ i) {
        leds.setPixelColor(i, d.leds[i][0], d.leds[i][1], d.leds[i][2]);
      }
      break;
    }
    case LHRHLEDMode::Debug:
#ifdef LH
      leds.setPixelColor(0, col(gTXIndicator.GetState()), col(gRXIndicator.GetState()), col(gRXIndicator.GetState()));
      // 1 = off
      leds.setPixelColor(2, 0, col(bite.Value01Estimate()), 0);
      leds.setPixelColor(3, 0, col(wind.Value01Estimate()), 0);
      leds.setPixelColor(4, 0, 0, col(key1.IsPressed()));
      leds.setPixelColor(5, 0, 0, col(key2.IsPressed()));
      leds.setPixelColor(6, 0, col(octave4.IsPressed()), col(key3.IsPressed()));
      leds.setPixelColor(7, 0, col(octave3.IsPressed()), col(key4.IsPressed()));
      leds.setPixelColor(8, 0, col(octave2.IsPressed()), col(key5.IsPressed()));
      leds.setPixelColor(9, col(backButton.IsCurrentlyPressed()), col(octave1.IsPressed()), col(key6.IsPressed()));
#else // RH
      leds.setPixelColor(0, col(gTXIndicator.GetState()), col(gRXIndicator.GetState()), col(gRXIndicator.GetState()));
      // 1 = off
      // 2 = off
      leds.setPixelColor(3, 0, col(pitchDown.Value01Estimate()), 0);
      leds.setPixelColor(4, 0, 0, col(key1.IsPressed()));
      leds.setPixelColor(5, 0, 0, col(key2.IsPressed()));
      leds.setPixelColor(6, 0, 0, col(key3.IsPressed()));
      leds.setPixelColor(7, 0, 0, col(key4.IsPressed()));
      leds.setPixelColor(8, 0, col(oooButton1.IsCurrentlyPressed()), col(key5.IsPressed()));
      leds.setPixelColor(9, 0, col(oooButton2.IsCurrentlyPressed()), col(key6.IsPressed()));
#endif // LH/RH
      break;
    }
    leds.show();
  }
}
