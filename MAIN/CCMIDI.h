
#ifndef CCCCEWIMIDIOut_H
#define CCCCEWIMIDIOut_H

#include <MIDI.h>

#include "Shared_CCUtil.h"

const int CCEWI_MIDICHANNEL = 1;

const float MIDI_BREATH_CURVE = 0.3f; // pow() - if <1, increases values

class CCEWIMIDIOut : IUpdateObject
{
  CCThrottlerT<30> gCCThrottle;
  int currentPitchBendRaw = 0;
  int currentBreathCC14Bit = 0;

  // from MIDI.hpp
  static int CalcRawPitchBend(double inPitchValue) {
    const int scale = inPitchValue > 0.0 ? MIDI_PITCHBEND_MAX : - MIDI_PITCHBEND_MIN;
    const int value = int(inPitchValue * double(scale));
    return value;
  }

  static int Calc14BitBreath(double inBreath01) {
    int ret = (int)(inBreath01 * 0x3fff);
    return ret & 0x3fff;
  }
  // splits 14-bit x into separate 7-bit MSB / LSB.
  static void Breath14BitToMSBLSB(int inValue, uint8_t& outMSB, uint8_t& outLSB) {
    // from MIDI.hpp
    outMSB = 0x7f & (inValue >> 7);
    outLSB = 0x7f & inValue;
  }

  MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial>>& mMidi;
public:
  CCEWIMIDIOut(MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial>>& _midi) :
    mMidi(_midi)
  {
  }
  
  virtual void setup() {
    mMidi.begin();
  }

  void Update(const CCEWIMusicalState& ms) {

    // important: send note on before note off, to make portamento work.
    if (ms.needsNoteOn) {
      mMidi.sendNoteOn(ms.MIDINote, 100, CCEWI_MIDICHANNEL);
    }
    if (ms.needsNoteOff) {
      mMidi.sendNoteOff(ms.noteOffNote, 0, CCEWI_MIDICHANNEL);
    }

    if (gCCThrottle.IsReady()) {
      int pb = CalcRawPitchBend(ms.pitchBendN11.GetValue());
      if (pb != currentPitchBendRaw) {
        mMidi.sendPitchBend(pb, CCEWI_MIDICHANNEL);
        currentPitchBendRaw = pb;
      }
      
      float breathAdj = powf(ms.breath01.GetValue(), MIDI_BREATH_CURVE);
      int breath = Calc14BitBreath(breathAdj);
      uint8_t msb, lsb;
      Breath14BitToMSBLSB(breath, msb, lsb);

      //CCPlot(breath);
      //CCPlot(msb);
      if (breath != currentBreathCC14Bit) {
        currentBreathCC14Bit = breath;
        
        mMidi.sendControlChange(midi::MidiControlChangeNumber::BreathController, msb, CCEWI_MIDICHANNEL);
        mMidi.sendControlChange(midi::MidiControlChangeNumber::BreathController + 32, lsb, CCEWI_MIDICHANNEL);
      }

    }
  }
};


#endif
