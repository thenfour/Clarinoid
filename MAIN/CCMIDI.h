
#ifndef CCCCEWIMIDIOut_H
#define CCCCEWIMIDIOut_H

#include <MIDI.h>

#include "Shared_CCUtil.h"

class CCEWIMIDIOut : IUpdateObject
{
  //MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, MIDI);

public:
  virtual void setup() {
    //MIDI.begin();
  }

  virtual void loop() {
//    int note;
//    for (note=10; note <= 127; note++) {
//      MIDI.sendNoteOn(note, 100, channel);
//      delay(200);
//      MIDI.sendNoteOff(note, 100, channel);
//    }
//    delay(2000);
  }
};


#endif
