// Teensy 4.0
// 600mhz

// Check synth & CCEWIControl for parameters

// https://forum.pjrc.com/threads/57932-Latency-what-s-new
//#define AUDIO_BLOCK_SAMPLES 1024 // this seems to have no effect; may need to fork

//============================================================

#include "Shared_CCSwitch.h"
#include "Shared_CCLeds.h"
#include "Shared_CCTxRx.h"

#include "CCPotentiometer.h"
#include "CCDisplay.h"
#include "CCEncoder.h"
#include "CCSynth.h"

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

CCMainTxRx gLHSerial(Serial1);
TransientActivityLED gLHRXIndicator(60, 500);
TransientEventLED gLHRXErrorIndicator(3000);
TransientActivityLED gLHTXIndicator(60, 500);

CCMainTxRx gRHSerial(Serial4);
ActivityLED gRHRXIndicator(60);
TransientEventLED gRHRXErrorIndicator(3000);
TransientActivityLED gRHTXIndicator(60, 500);

AsymmetricActivityLED gGeneralActivityIndicator(750, 250);
CCDisplay gDisplay;

framerateCalculator gFramerate;

bool prev_key_back = false;

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
  if (gLHSerial.mHaveNewData) {
    gLHRXIndicator.Touch();
  }
  if (gLHSerial.mErrorsDirty) {
    gLHRXErrorIndicator.Touch();
  }

  // gather up serial receive (RH)
  if (gRHSerial.mHaveNewData) {
    gRHRXIndicator.Touch();
  }
  if (gRHSerial.mErrorsDirty) {
    gRHRXErrorIndicator.Touch();
  }

  // combine the data to form the current state of things.
  // this converts incoming data to overall state and musical paramaters
  gEWIControl.Update(gLHSerial.mReceivedData, gRHSerial.mReceivedData);

  if (gEWIControl.mPhysicalState.key_back != prev_key_back) {
    // send a command to LHRH.
    prev_key_back = gEWIControl.mPhysicalState.key_back;
    MainPayload payload;
    payload.data.ledMode = gEWIControl.mPhysicalState.key_back ? LHRHLEDMode::Off : LHRHLEDMode::Debug;
    gLHSerial.Send(payload);
    gRHSerial.Send(payload);
    gLHTXIndicator.Touch();
    gRHTXIndicator.Touch();    
  }

  
  // convert state to MIDI events
  gSynth.Update(gEWIControl.mMusicalState);
  
  // output MIDI
  // output to synth

  if (ledThrottle.IsReady())
  {
    auto activityColor = col(gGeneralActivityIndicator.GetState(), 1, 4);
    leds.setPixelColor(0, 0, activityColor, activityColor); // cyan = MAIN
    leds.setPixelColor(1, col(gLHRXErrorIndicator.GetState(), 0, 4), 0, col(gLHRXIndicator.GetState(), 0, 4));
    leds.setPixelColor(2, 0, col(gLHTXIndicator.GetState(), 0, 4), 0);
    leds.setPixelColor(3, col(gRHRXErrorIndicator.GetState(), 0, 4), 0, col(gRHRXIndicator.GetState(), 0, 4));
    leds.setPixelColor(2, 0, col(gRHTXIndicator.GetState(), 0, 4), 0);
    // 5 midiTX
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

    auto pageRX = [&](CCMainTxRx& rx, const char *title){
      //CCMainTxRx& rx = (gEncButton.IsPressed()) ? gLHSerial : gRHSerial;
      gDisplay.mDisplay.println(String(title) + " ok:" + rx.mRxSuccess + " #" + rx.mReceivedData.serial);
      gDisplay.mDisplay.print(String("Err:") + rx.mChecksumErrors);
      gDisplay.mDisplay.println(String(" Skip: ") + rx.mSkippedPayloads);
      gDisplay.mDisplay.println(String("fps:") + (int)rx.mReceivedData.framerate + "  tx:" + rx.mTXSerial);
      gDisplay.mDisplay.println(String("myfps:") + (int)gFramerate.getFPS());
    };

    auto pageLHRX = [&](){
      pageRX(gLHSerial, "LH");
    };
    
    auto pageRHRX = [&](){
      pageRX(gRHSerial, "RH");
    };
    
    auto pageMusicalState = [&]() {
      gDisplay.mDisplay.println(String("note: ") + gEWIControl.mMusicalState.MIDINote + " (" + (gEWIControl.mMusicalState.isPlayingNote ? "ON" : "off" ) + ") freq=" + (int)MIDINoteToFreq(gEWIControl.mMusicalState.MIDINote));
      gDisplay.mDisplay.println(String("wind:") + gEWIControl.mMusicalState.breath01.GetValue());
      gDisplay.mDisplay.println(String("pitch:") + gEWIControl.mMusicalState.pitchBendN11.GetValue());
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
      gDisplay.mDisplay.println(String((gEncButton.IsPressed()) ? "LH" : "RH") + ToString((gEncButton.IsPressed()) ? gLHSerial.mReceivedData : gRHSerial.mReceivedData));
    };

    auto pageAudioStatus = [&]() {
      gDisplay.mDisplay.println(String("Memory: ") + AudioMemoryUsage());
      gDisplay.mDisplay.println(String("Memory Max: ") + AudioMemoryUsageMax());
      gDisplay.mDisplay.println(String("CPU: ") + AudioProcessorUsage());
      gDisplay.mDisplay.println(String("CPU Max: ") + AudioProcessorUsageMax());
    };

    int page = gEnc.GetValue() / 4;
    page = page % 5;
    switch(page) {
      case 0:
        pageLHRX();
        break;
      case 1:
        pageRHRX();
        break;
      case 2:
        pageMusicalState();
        break;
      case 3:
        pagePhysicalState();
        break;
      case 4:
        pageDebugRX();
        break;
      case 5:
        pageAudioStatus();
        break;
    }
    
    gDisplay.mDisplay.display();
  }

  //delay(50);
}
