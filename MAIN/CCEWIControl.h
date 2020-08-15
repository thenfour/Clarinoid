// converts state to 

#ifndef CCEWICONTROL_H
#define CCEWICONTROL_H

#include "CCUtil.h"
#include "CCTransmit.h"

#define NOTE_ON_BREATH_THRESHOLD 0.1

enum class Tristate
{
  Position1,
  Position2,
  Position3
};

struct CCEWIPhysicalState
{
  bool key_lh1;
  bool key_lh2;
  bool key_lh3;
  bool key_lh4;

  bool key_rh1;
  bool key_rh2;
  bool key_rh3;
  bool key_rh4;

  bool key_octave1;
  bool key_octave2;
  bool key_octave3;
  bool key_octave4;

  bool key_lhExtra1;
  bool key_lhExtra2;
  bool key_rhExtra1;
  bool key_rhExtra2;

  float breath01;
  float bite01;
  float pitchDown01;

  // yea these are sorta oddballs; not really sure they should be lumped in here
  // but it's convenient to do so because they come from the LHRH payloads.
  bool key_back;
  Tristate key_triState; // same.
  
  CCEWIPhysicalState() = default;
  explicit CCEWIPhysicalState(const LHRHPayload& lh, const LHRHPayload& rh)
  {
    this->key_lh1 = lh.data.key2;
    this->key_lh2 = lh.data.key3;
    this->key_lh3 = lh.data.key4;
    this->key_lh4 = lh.data.key5;

    this->key_octave1 = lh.data.octave1;
    this->key_octave2 = lh.data.octave2;
    this->key_octave3 = lh.data.octave3;
    this->key_octave4 = lh.data.octave4;

    this->key_lhExtra1 = lh.data.key1;
    this->key_lhExtra2 = lh.data.key6;
    
    this->key_rh1 = rh.data.key2;
    this->key_rh2 = rh.data.key3;
    this->key_rh3 = rh.data.key4;
    this->key_rh4 = rh.data.key5;

    this->key_rhExtra1 = rh.data.key1;
    this->key_rhExtra2 = rh.data.key6;

    this->breath01 = (float)lh.data.pressure1 / 1024;
    this->bite01 = (float)lh.data.pressure2 / 1024;
    this->pitchDown01 = (float)rh.data.pressure1 / 1024;

    this->key_back = lh.data.button1;
    if (rh.data.button1) {
      this->key_triState = Tristate::Position1;
    } else if (rh.data.button2) {
      this->key_triState = Tristate::Position3;
    } else {
      this->key_triState = Tristate::Position2;
    }
  }
};

struct CCEWIMusicalState
{
  bool isPlayingNote = false;
  uint8_t MIDINote = 0;
  float pitchBendN11 = 0; // -1 to 1
  float breath01 = 0;// 0-1

  CCEWIMusicalState() = default;
  
  explicit CCEWIMusicalState(const CCEWIPhysicalState& ps)
  {
    // now convert that to musical state. i guess this is where the 
    // most interesting EWI-ish logic is.

    // the easy stuff
    this->breath01 = constrain(ps.breath01, 0, 1);
    this->pitchBendN11 = constrain(ps.bite01 - ps.pitchDown01, -1, 1);

    this->isPlayingNote = this->breath01 > NOTE_ON_BREATH_THRESHOLD;

    // the rules are rather weird for keys. open is a C#
    // https://bretpimentel.com/flexible-ewi-fingerings/
    this->MIDINote = 49; // C#3
    if (ps.key_lh1){
      this->MIDINote -= 2;
    }
    if (ps.key_lh2) {
      this->MIDINote -= ps.key_lh1 ? 2 : 1;
    }
    if (ps.key_lh3) {
      this->MIDINote -= 2;
    }
    if (ps.key_lh4) {
      this->MIDINote += 1;
    }

    if (ps.key_rh1) {
      this->MIDINote -= ps.key_lh3 ? 2 : 1;
    }
    if (ps.key_rh2) {
      this->MIDINote -= 1;
    }
    if (ps.key_rh3) {
      this->MIDINote -= 2;
    }
    if (ps.key_rh4) {
      this->MIDINote -= 2;
    }

    if (ps.key_octave4) {
      this->MIDINote += 12 * 4;  
    } else if (ps.key_octave3) {
      this->MIDINote += 12 * 3;
    } else if (ps.key_octave2) {
      this->MIDINote += 12 * 2;
    } else if (ps.key_octave1) {
      this->MIDINote += 12 * 1;
    }
  }
};

class CCEWIControl
{
public:
  CCEWIPhysicalState mPhysicalState;
  CCEWIMusicalState mMusicalState;
  
  void Update(const LHRHPayload& lh, const LHRHPayload& rh) {
    mPhysicalState = CCEWIPhysicalState(lh, rh);
    mMusicalState = CCEWIMusicalState(mPhysicalState);
  }
};

#endif
