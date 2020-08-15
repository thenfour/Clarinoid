// Teensy 4.0
// 600mhz

// Check synth & CCEWIControl for parameters

// https://forum.pjrc.com/threads/57932-Latency-what-s-new
//#define AUDIO_BLOCK_SAMPLES 1024 // this seems to have no effect; may need to fork

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

CCEWIControl gEWIControl;
CCSynth gSynth;

CCOnOffSwitch gEncButton(3, 10, 5);
CCEncoder gEnc(4, 5);
TransientActivityLED gEncIndicator(60, 200);

CCVolumePot gVolumePot(A8);
TransientActivityLED gVolIndicator(40, 200);

CCReceiver gLHReceiver(Serial1);
ActivityLED gLHRXIndicator(60);
TransientEventLED gLHRXErrorIndicator(3000);

CCReceiver gRHReceiver(Serial4);
ActivityLED gRHRXIndicator(60);
TransientEventLED gRHRXErrorIndicator(3000);

AsymmetricActivityLED gGeneralActivityIndicator(750, 250);
CCDisplay gDisplay;
//CCMenu gMenu(gDisplay, gEnc, gEncButton, gEWIControl);

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

  // gather up serial receive (LH)
  if (gLHReceiver.mHaveNewData) {
    gLHRXIndicator.Touch();
  }
  if (gLHReceiver.mErrorsDirty) {
    gLHRXErrorIndicator.Touch();
  }

  // gather up serial receive (RH)
  if (gRHReceiver.mHaveNewData) {
    gRHRXIndicator.Touch();
  }
  if (gRHReceiver.mErrorsDirty) {
    gRHRXErrorIndicator.Touch();
  }

  // combine the data to form the current state of things.
  // this converts incoming data to overall state and musical paramaters
  gEWIControl.Update(gLHReceiver.mData, gRHReceiver.mData);
  
  // convert state to MIDI events
  gSynth.Update(gEWIControl.mMusicalState);
  
  // output MIDI
  // output to synth

  if (ledThrottle.IsReady())
  {
    auto activityColor = col(gGeneralActivityIndicator.GetState(), 1, 4);
    leds.setPixelColor(0, 0, activityColor, activityColor); // cyan = MAIN
    leds.setPixelColor(1, col(gLHRXErrorIndicator.GetState(), 0, 4), 0, col(gLHRXIndicator.GetState(), 0, 4));
    leds.setPixelColor(2, col(gRHRXErrorIndicator.GetState(), 0, 4), 0, col(gRHRXIndicator.GetState(), 0, 4));
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

    auto pageRX = [&](){
      CCReceiver& rx = (gEncButton.IsPressed()) ? gLHReceiver : gRHReceiver;
      gDisplay.mDisplay.println(String((gEncButton.IsPressed()) ? "LH" : "RH") + " ok:" + rx.mRxSuccess + " #" + rx.mData.serial);
      gDisplay.mDisplay.print(String("Err:") + rx.mChecksumErrors);
      gDisplay.mDisplay.println(String(" Skip: ") + rx.mSkippedPayloads);
      gDisplay.mDisplay.println(String("fps:") + (int)rx.mData.framerate);
      gDisplay.mDisplay.println(String("myfps:") + (int)gFramerate.getFPS());
    };
    auto pageMusicalState = [&]() {
      gDisplay.mDisplay.println(String("note: ") + gEWIControl.mMusicalState.MIDINote + " (" + (gEWIControl.mMusicalState.isPlayingNote ? "ON" : "off" ) + ") freq=" + (int)MIDINoteToFreq(gEWIControl.mMusicalState.MIDINote));
      gDisplay.mDisplay.println(String("wind:") + gEWIControl.mMusicalState.breath01);
      gDisplay.mDisplay.println(String("pitch:") + gEWIControl.mMusicalState.pitchBendN11);
    };
    auto pagePhysicalState = [&]() {
      // LH: k:1234 o:1234 b:12
      // RH: k:1234 b:12
      // wind:0.455 // bite:0.11
      // pitch:
      gDisplay.mDisplay.println(String("LH k:") +
        (gEWIControl.mPhysicalState.key_lh1 ? "1" : "-") +
        (gEWIControl.mPhysicalState.key_lh2 ? "2" : "-") +
        (gEWIControl.mPhysicalState.key_lh3 ? "3" : "-") +
        (gEWIControl.mPhysicalState.key_lh4 ? "4" : "-") +
        " o:" +
        (gEWIControl.mPhysicalState.key_octave1 ? "1" : "-") +
        (gEWIControl.mPhysicalState.key_octave2 ? "2" : "-") +
        (gEWIControl.mPhysicalState.key_octave3 ? "3" : "-") +
        (gEWIControl.mPhysicalState.key_octave4 ? "4" : "-") +
        " b:" +
        (gEWIControl.mPhysicalState.key_lhExtra1 ? "1" : "-") +
        (gEWIControl.mPhysicalState.key_lhExtra2 ? "2" : "-"));

      gDisplay.mDisplay.println(String("RH k:") +
        (gEWIControl.mPhysicalState.key_rh1 ? "1" : "-") +
        (gEWIControl.mPhysicalState.key_rh2 ? "2" : "-") +
        (gEWIControl.mPhysicalState.key_rh3 ? "3" : "-") +
        (gEWIControl.mPhysicalState.key_rh4 ? "4" : "-") +
        "       " +
        " b:" +
        (gEWIControl.mPhysicalState.key_lhExtra1 ? "1" : "-") +
        (gEWIControl.mPhysicalState.key_lhExtra2 ? "2" : "-"));
      gDisplay.mDisplay.println(String("breath: ") + gEWIControl.mPhysicalState.breath01 + "  " +
        "bite: " + gEWIControl.mPhysicalState.bite01);
      gDisplay.mDisplay.println(String("pitch: ") + gEWIControl.mPhysicalState.pitchDown01);
    };

    auto pageDebugRX = [&]() {
      gDisplay.mDisplay.println(String((gEncButton.IsPressed()) ? "LH" : "RH") + ToString((gEncButton.IsPressed()) ? gLHReceiver.mData : gLHReceiver.mData));
    };

    int page = gEnc.GetValue() / 4;
    page = page % 4;
    switch(page) {
      case 0:
        pageRX();
        break;
      case 1:
        pageMusicalState();
        break;
      case 2:
        pagePhysicalState();
        break;
      case 3:
        pageDebugRX();
        break;
    }
    
    gDisplay.mDisplay.display();
  }

  //delay(50);
}
