
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

// MIDI library is touchy about how you instantiate.
// Simplest is to do it the way it's designed for: in the main sketch, global scope.
MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, gMIDI);

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

CCEWIControl gEWIControl;

CCSynth gSynth;

CCEWIMIDIOut gMidiOut(gMIDI);
TransientActivityLED gMidiActivityIndicator(40, 200);

// non-tactile keys ugh
BoolKeyWithRepeat<500, 200> gRHButton1Key;
BoolKeyWithRepeat<500, 200> gRHButton2Key;
Tristate gOldTristateVal = Tristate::Null;

// contains all non-GUI application stuff.
class CCEWIApp : IUpdateObject
{
public:
  void loop() {
   
    if (gEnc.IsDirty()) {
      gEncIndicator.Touch();
    }
    
    if (gVolumePot.IsDirty()) {
      gVolIndicator.Touch();
      gSynth.SetGain(gVolumePot.GetValue01());
    }

    if (gOldTristateVal != gEWIControl.mPhysicalState.key_triState) {
      // tristate will control harmonizer.
      switch (gEWIControl.mPhysicalState.key_triState) {
      case Tristate::Position2:
      case Tristate::Position3:
        gSynth.SetHarmonizer(true);
        break;
      default:
        gSynth.SetHarmonizer(false);
        break;
      }
    }
    
    gRHButton1Key.Update(gEWIControl.mPhysicalState.key_rhExtra1);
    gRHButton2Key.Update(gEWIControl.mPhysicalState.key_rhExtra2);
    
    if (gRHButton1Key.IsTriggered()) {
      // todo: this logic belongs elsewhere.
      if (gEWIControl.mTranspose < 48)
        gEWIControl.mTranspose += 12;
    }
    if (gRHButton2Key.IsTriggered()) {
      // todo: this logic belongs elsewhere.
      if (gEWIControl.mTranspose > -48)
        gEWIControl.mTranspose -= 12;
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
  
//    if (gEWIControl.mPhysicalState.key_back != prev_key_back) {
//      // send a command to LHRH.
//      prev_key_back = gEWIControl.mPhysicalState.key_back;
//      MainPayload payload;
//      payload.data.ledMode = gEWIControl.mPhysicalState.key_back ? LHRHLEDMode::Off : LHRHLEDMode::Debug;
//      gLHSerial.Send(payload);
//      gRHSerial.Send(payload);
//      gLHTXIndicator.Touch();
//      gRHTXIndicator.Touch();    
//    }
//    
    // convert state to MIDI events
    gMidiOut.Update(gEWIControl.mMusicalState);
    
    if (gMidiOut.activityHappened) {
      gMidiActivityIndicator.Touch();
    }
    
    // output to synth. this synth doesn't operate on the MIDI data because MIDI reduces precision a bit.
    // this can be realtime.
    gSynth.Update(gEWIControl.mMusicalState);
  }
};


#endif
