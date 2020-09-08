
#ifndef CCMIDIAPP_H
#define CCMIDIAPP_H

#include <MIDI.h>

#include "Shared_CCSwitch.h"
#include "Shared_CCLeds.h"
#include "Shared_CCTxRx.h"

#include "CCPotentiometer.h"
#include "CCEncoder.h"
#include "CCSynth.h"
#include "CCMIDI.h"



uint32_t gLongestLoopMicros = 0;
uint32_t gLongestBetweenLoopMicros = 0;

// MIDI library is touchy about how you instantiate.
// Simplest is to do it the way it's designed for: in the main sketch, global scope.
MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, gMIDI);

CCLeds gLeds(10, 2, true);
CCThrottlerT<10> gLedThrottle;

CCOnOffSwitch gEncButton(3, 5);
CCEncoder<4> gEnc(4, 5);
TransientActivityLED gEncIndicator(60, 200);

CCVolumePot gVolumePot(A8);
TransientActivityLED gVolIndicator(40, 200);

CCMainTxRx gLHSerial(Serial1);
TransientActivityLED gLHRXIndicator(60, 500);
TransientEventLED gLHRXErrorIndicator(700);
TransientActivityLED gLHTXIndicator(60, 500);

CCMainTxRx gRHSerial(Serial4);
ActivityLED gRHRXIndicator(60);
TransientEventLED gRHRXErrorIndicator(700);
TransientActivityLED gRHTXIndicator(60, 500);

CCEWIControl gEWIControl;

CCSynth gSynth;

CCEWIMIDIOut gMidiOut(gMIDI);
TransientActivityLED gMidiActivityIndicator(40, 200);

// non-tactile keys ugh
BoolKeyWithRepeat<1000, 1000> gRHButton1Key;
BoolKeyWithRepeat<1000, 1000> gRHButton2Key;
Tristate gOldTristateVal = Tristate::Null;

bool gSettingsLoaded = false;

// contains all non-GUI application stuff.
class CCEWIApp : UpdateObjectT<ProfileObjectType::EWIApp>
{
  bool mTxScheduled = false;
public:

  MainPayload mPayload;

  CCEWIApp()
  {
    mPayload.data.focusedTouchKey = -1;
    mPayload.data.ledMode = LHRHLEDMode::Debug;
  }

  int8_t mLastKeyFocused = -33;// something you'll never send, so the 1st is always sent.

  // schedules notifying LHRH to send debug data for a cap touch key.
  void FocusKeyDebug(int8_t key /* index into gKeyDesc */) {
    if (mLastKeyFocused == key)
      return;
    mLastKeyFocused = key;
    mTxScheduled = true;
    mPayload.data.focusedTouchKey = key;
  }

  void SendCmd(CommandFromMain cmd) {
    mPayload.data.cmd = cmd;
    mTxScheduled = true;    
    //Serial.println("Sendingcmd");
  }

  void SendCmd(CommandFromMain cmd, float param1) {
    mPayload.data.cmd = cmd;
    mPayload.data.cmdFloatParam1 = param1;
    mTxScheduled = true;    
    //Serial.println("Sendingcmd2");
  }

  void loop() {

    if (!gSettingsLoaded) {
      SendCmd(gAppSettings.mOrangeLEDs ? CommandFromMain::EnableOrangeLED : CommandFromMain::DisableOrangeLED);
      gSettingsLoaded = true;
    }

   
    if (gEnc.IsDirty()) {
      gEncIndicator.Touch();
    }
    
    if (gVolumePot.IsDirty()) {
      gVolIndicator.Touch();
      gSynth.SetGain(gVolumePot.GetValue01());
    }
    
    gRHButton1Key.Update(gEWIControl.mPhysicalState.key_rhExtra1.IsCurrentlyPressed());
    gRHButton2Key.Update(gEWIControl.mPhysicalState.key_rhExtra2.IsCurrentlyPressed());
  
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

    if (!gTouchKeyGraphsIsRunning) {
      if (mPayload.data.focusedTouchKey != -1) {
        FocusKeyDebug(-1);
      }
    }
    gTouchKeyGraphsIsRunning = false; // between frames this gets set to true if it's running, so we can detect if it's not running like this.


    if (mTxScheduled) {
      mTxScheduled = false;
      gLHSerial.Send(mPayload);
      gRHSerial.Send(mPayload);
      gLHTXIndicator.Touch();
      gRHTXIndicator.Touch();    
      // reset the payload.
      mPayload.data.cmd = CommandFromMain::None;
    }

    // convert state to MIDI events
    gMidiOut.Update(gEWIControl.mMusicalState);
    
    if (gMidiOut.activityHappened) {
      gMidiActivityIndicator.Touch();
    }
    
    // output to synth. this synth doesn't operate on the MIDI data because MIDI reduces precision a bit.
    // this can be realtime.
    gSynth.Update(gEWIControl.mMusicalState);

    if (gLedThrottle.IsReady())
    {
      gLeds.setPixelColor(0, 0, 0, 0);
      gLeds.setPixelColor(1, col(gLHRXErrorIndicator.GetState()), col(gLHTXIndicator.GetState()), col(gLHRXIndicator.GetState()));
      gLeds.setPixelColor(2, col(gRHRXErrorIndicator.GetState()), col(gRHTXIndicator.GetState()), col(gRHRXIndicator.GetState()));
      gLeds.setPixelColor(3, 0, col(gMidiActivityIndicator.GetState()), col(gMidiActivityIndicator.GetState()));
      
      gLeds.setPixelColor(4, col(gAppSettings.mTranspose != 0), 0, 0);
      gLeds.setPixelColor(5, col(gAppSettings.mTranspose < 0), col(gAppSettings.mTranspose > 0), 0);
  
      // 6 = off
      // 7 = off
  
      gLeds.setPixelColor(8, col(gEncButton.IsCurrentlyPressed()), 0, col(gEncIndicator.GetState()));
      gLeds.setPixelColor(9, 0, gVolumePot.GetValue01() * 6, col(gVolIndicator.GetState()));
      gLeds.show();
    }
  
  } // loop();
};

CCEWIApp gApp;

#endif
