// converts state to 

#ifndef CCEWICONTROL_H
#define CCEWICONTROL_H

#include "Shared_CCUtil.h"
#include "Shared_CCTxRx.h"

const float BREATH_LOWER_DEADZONE = 0.07f;
const float BREATH_UPPER_DEADZONE = 0.6f;
const float PITCHDOWN_DEADZONE = 0.8f;
const float BREATH_NOTEON_THRESHOLD = 0.25;

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
  
  void Update(const LHRHPayload& lh, const LHRHPayload& rh)
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

enum class NoteTransitionType
{
  WasNeverPlaying, // nothing (isplaying=false)
  NoteOff,// was playing note, no longer playing note. (isplaying = false)
  Legato, // was playing note, now playing this different note (isplaying = true)
  PlayingNewNote
};

struct CCEWIMusicalState
{
  // stuff helpful for midi processing
  bool isPlayingNote = false;
  
  uint8_t MIDINote = 0;
  // TODO: create a time-based smoother (LPF). throttling and taking samples like this is not very accurate. sounds fine today though.
  SimpleMovingAverage<20, true> breath01;// 0-1
  SimpleMovingAverage<120, false> pitchBendN11; // -1 to 1
  CCThrottlerT<1> mPressureSensingThrottle;
  int nUpdates = 0;

  // { valid for 1 frame only.
  bool needsNoteOn = false;
  bool needsNoteOff = false;
  uint8_t noteOffNote = 0; // ignore this if needsNoteOff is false.
  // } valid for 1 frame only.

  CCEWIMusicalState() {
    this->breath01.Update(0);
    this->pitchBendN11.Update(0);
  }
  

  void Update(const CCEWIPhysicalState& ps)
  {
    uint8_t prevNote = MIDINote;
    bool wasPlayingNote = isPlayingNote;

    // convert that to musical state. i guess this is where the 
    // most interesting EWI-ish logic is.

    if (nUpdates == 0 || mPressureSensingThrottle.IsReady()) {

      float breathAdj = map((float)ps.breath01, BREATH_LOWER_DEADZONE, BREATH_UPPER_DEADZONE, 0.0f, 1.0f);
      breathAdj = constrain(breathAdj, 0.0f, 1.0f);
      //breathAdj = powf(breathAdj, BREATH_CURVE);
      //breathAdj = constrain(breathAdj, 0.0f, 1.0f);
      this->breath01.Update(breathAdj);
      
      //this->pitchBendN11.Update(constrain(map((float)(ps.pitchDown01), 0, PITCHDOWN_DEADZONE, -1.0f, 0.0f), -1.0f, 0.0f));
      this->isPlayingNote = this->breath01.GetValue() > BREATH_NOTEON_THRESHOLD;
    }

    //CCPlot(this->breath01.GetValue() * 100);
    //CCPlot(ps.breath01 * 100);
    //CCPlot(this->pitchBendN11.GetValue() * 100);
    //CCPlot(ps.pitchDown01 * 100);

    // the rules are rather weird for keys. open is a C#...
    // https://bretpimentel.com/flexible-ewi-fingerings/
    this->MIDINote = 49-12; // C#2
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

    needsNoteOn = isPlayingNote && (!wasPlayingNote || prevNote != MIDINote);
    // send note off in these cases:
    // - you are not playing but were
    // - or, you are playing, but a different note than before.
    needsNoteOff = (!isPlayingNote && wasPlayingNote) || (isPlayingNote && (prevNote != MIDINote));
    noteOffNote = prevNote;

//    if (needsNoteOn) {
//      Serial.println(String("note on: ") + MIDINote + " ; wasplaying=" + wasPlayingNote + " prevnote=" + prevNote + " isplaying=" + isPlayingNote + " currentnote=" + MIDINote);
//    }
//    if (needsNoteOff) {
//      Serial.println(String("note off: ") + noteOffNote + " ; wasplaying=" + wasPlayingNote + " prevnote=" + prevNote + " isplaying=" + isPlayingNote + " currentnote=" + MIDINote);
//    }

    nUpdates ++;
  }
};

class CCEWIControl
{
public:
  CCEWIPhysicalState mPhysicalState;
  CCEWIMusicalState mMusicalState;
  
  void Update(const LHRHPayload& lh, const LHRHPayload& rh) {
    mPhysicalState.Update(lh, rh);
    mMusicalState.Update(mPhysicalState);
  }
};

#endif
