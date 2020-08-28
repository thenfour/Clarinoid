
#ifndef CCEWICONTROL_H
#define CCEWICONTROL_H

#include "Shared_CCUtil.h"
#include "Shared_CCTxRx.h"

static constexpr size_t MAX_MUSICAL_VOICES = LOOP_LAYERS * HARM_VOICES;

struct CCEWIPhysicalState
{
  CCVirtualSwitch key_lh1;
  CCVirtualSwitch key_lh2;
  CCVirtualSwitch key_lh3;
  CCVirtualSwitch key_lh4;

  CCVirtualSwitch key_rh1;
  CCVirtualSwitch key_rh2;
  CCVirtualSwitch key_rh3;
  CCVirtualSwitch key_rh4;

  CCVirtualSwitch key_octave1;
  CCVirtualSwitch key_octave2;
  CCVirtualSwitch key_octave3;
  CCVirtualSwitch key_octave4;

  CCVirtualSwitch key_lhExtra1;
  CCVirtualSwitch key_lhExtra2;
  CCVirtualSwitch key_rhExtra1;
  CCVirtualSwitch key_rhExtra2;

  // match the order of gKeyDesc
  CCVirtualSwitch* mOrderedKeys[16] = {
    &key_lh1,
    &key_lh2,
    &key_lh3,
    &key_lh4,

    &key_rh1,
    &key_rh2,
    &key_rh3,
    &key_rh4,

    &key_octave1,
    &key_octave2,
    &key_octave3,
    &key_octave4,

    &key_lhExtra1,
    &key_lhExtra2,
    &key_rhExtra1,
    &key_rhExtra2,
  };

  float breath01;
  float bite01;
  float pitchDown01;

  // yea these are sorta oddballs; not really sure they should be lumped in here
  // but it's convenient to do so because they come from the LHRH payloads.
  CCVirtualSwitch key_back;
  Tristate key_triState; // same.
  bool key_triState_is_dirty = false;
  
  void Update(const LHRHPayload& lh, const LHRHPayload& rh)
  {
    this->key_lh1.Update(lh.data.keys[1].IsPressed);
    this->key_lh2.Update(lh.data.keys[2].IsPressed);
    this->key_lh3.Update(lh.data.keys[3].IsPressed);
    this->key_lh4.Update(lh.data.keys[4].IsPressed);

    this->key_octave1.Update(lh.data.octaveKeys[0].IsPressed);
    this->key_octave2.Update(lh.data.octaveKeys[1].IsPressed);
    this->key_octave3.Update(lh.data.octaveKeys[2].IsPressed);
    this->key_octave4.Update(lh.data.octaveKeys[3].IsPressed);

    this->key_lhExtra1.Update(lh.data.keys[0].IsPressed);
    this->key_lhExtra2.Update(lh.data.keys[5].IsPressed);
    
    this->key_rh1.Update(rh.data.keys[1].IsPressed);
    this->key_rh2.Update(rh.data.keys[2].IsPressed);
    this->key_rh3.Update(rh.data.keys[3].IsPressed);
    this->key_rh4.Update(rh.data.keys[4].IsPressed);

    this->key_rhExtra1.Update(rh.data.keys[0].IsPressed);
    this->key_rhExtra2.Update(rh.data.keys[5].IsPressed);

    this->breath01 = (float)lh.data.pressure1 / 1024;
    this->bite01 = (float)lh.data.pressure2 / 1024;
    this->pitchDown01 = (float)rh.data.pressure1 / 1024;

    this->key_back.Update(lh.data.button1);

    Tristate oldState = key_triState;
    if (rh.data.button1) {
      this->key_triState = Tristate::Position1;
    } else if (rh.data.button2) {
      this->key_triState = Tristate::Position3;
    } else {
      this->key_triState = Tristate::Position2;
    }
    key_triState_is_dirty = oldState != key_triState;
  }
};

struct MusicalVoice
{
  MusicalVoice() = default;
  MusicalVoice(const MusicalVoice& rhs) = delete; // no copy ctor.
  MusicalVoice(MusicalVoice&&) = delete;// no move.
  MusicalVoice& operator =(const MusicalVoice& rhs) // assignment is fine for an existing voice obj.
  {
    mIsNoteCurrentlyOn = rhs.mIsNoteCurrentlyOn;
    mNote = rhs.mNote;
    mVelocity = rhs.mVelocity;
    mSynthPatch = rhs.mSynthPatch;
    return *this;
  }
  MusicalVoice& operator =(MusicalVoice&&) = delete; // no move assignment because i want these objects to stay static.
  
  int16_t mVoiceId = -1;
  bool mIsNoteCurrentlyOn = false;
  uint8_t mNote = 0;
  uint8_t mVelocity = 0;
  int16_t mSynthPatch = -1;
};

struct Harmonizer
{
  // state & processing for harmonizer.

  // called each frame to add harmonizer voices to the output, given the live playing voice.
  // return the next available voice.
  MusicalVoice* Update(const MusicalVoice& liveVoice, MusicalVoice* outp, const MusicalVoice* end) {
    *outp = liveVoice;
    outp ++;
    *outp = liveVoice;
    outp->mNote += 7;
    return outp;
  }
};

struct Looper
{
  // here you can record a loop or insert notes.
  // return the next item
  MusicalVoice* Update(const MusicalVoice* playingVoices, const MusicalVoice* playingEnd, MusicalVoice* outp, const MusicalVoice* outpEnd) {
    return outp;
  }
};

struct CCEWIMusicalState
{
  MusicalVoice mLiveVoice;
  MusicalVoice mMusicalVoices[MAX_MUSICAL_VOICES]; // these are all the voices that WANT to be played. May be more than synth polyphony.

  Harmonizer mHarmonizer;
  Looper mLooper;

  // TODO: create a time-based smoother (LPF). throttling and taking samples like this is not very accurate. sounds fine today though.
  SimpleMovingAverage<20> breath01;// 0-1
  SimpleMovingAverage<120> pitchBendN11; // -1 to 1
  CCThrottlerT<1> mPressureSensingThrottle;
  int nUpdates = 0;
  int noteOns = 0;

  // { valid for 1 frame only.
  bool needsNoteOn = false;
  bool needsNoteOff = false;
  uint8_t noteOffNote = 0; // ignore this if needsNoteOff is false.
  // } valid for 1 frame only.

  CCEWIMusicalState() {
    this->breath01.Update(0);
    this->pitchBendN11.Update(0);

    for (int16_t i = 0; i < (int16_t)SizeofStaticArray(mMusicalVoices); ++ i) {
      mMusicalVoices[i].mVoiceId = i;
    }
  }

  // Update() determines the mLiveVoice, converting physical to live musical state.
  // but this function then takes the live musical state, and fills out mMusicalVoices based on harmonizer & looper settings.
  void DistributeVoices() {
    MusicalVoice* p = mMusicalVoices;
    if (gAppSettings.mHarmSettings.mIsEnabled) {
      p = mHarmonizer.Update(mLiveVoice, p, EndPtr(mMusicalVoices));
    } else {
      mMusicalVoices[0] = mLiveVoice;
      p = &mMusicalVoices[1];
      for (int16_t i = 1; i < (int16_t)SizeofStaticArray(mMusicalVoices); ++ i) {
        mMusicalVoices[i].mIsNoteCurrentlyOn = false;
      }
    }

    p = mLooper.Update(mMusicalVoices, p, p, EndPtr(mMusicalVoices));
  }

  void Update(const CCEWIPhysicalState& ps)
  {
    uint8_t prevNote = mLiveVoice.mNote;
    bool wasPlayingNote = mLiveVoice.mIsNoteCurrentlyOn;
    bool isPlayingNote = wasPlayingNote;

    // convert that to musical state. i guess this is where the 
    // most interesting EWI-ish logic is.

    if (nUpdates == 0 || mPressureSensingThrottle.IsReady()) {

      float breathAdj = map((float)ps.breath01, gAppSettings.mBreathLowerBound, gAppSettings.mBreathUpperBound, 0.0f, 1.0f);
      breathAdj = constrain(breathAdj, 0.0f, 1.0f);
      this->breath01.Update(breathAdj);
      
      //this->pitchBendN11.Update(constrain(map((float)(ps.pitchDown01), 0, PITCHDOWN_DEADZONE, -1.0f, 0.0f), -1.0f, 0.0f));
      isPlayingNote = this->breath01.GetValue() > gAppSettings.mBreathNoteOnThreshold;
      mLiveVoice.mIsNoteCurrentlyOn = isPlayingNote;
      mLiveVoice.mVelocity = 100; // TODO
      mLiveVoice.mSynthPatch = 0; // TODO
    }

    // the rules are rather weird for keys. open is a C#...
    // https://bretpimentel.com/flexible-ewi-fingerings/
    int newNote = 49; // C#2
    if (ps.key_lh1.IsCurrentlyPressed()){
      newNote -= 2;
    }
    if (ps.key_lh2.IsCurrentlyPressed()) {
      newNote -= ps.key_lh1.IsCurrentlyPressed() ? 2 : 1;
    }
    if (ps.key_lh3.IsCurrentlyPressed()) {
      newNote -= 2;
    }
    if (ps.key_lh4.IsCurrentlyPressed()) {
      newNote += 1;
    }

    if (ps.key_rh1.IsCurrentlyPressed()) {
      newNote -= ps.key_lh3.IsCurrentlyPressed() ? 2 : 1;
    }
    if (ps.key_rh2.IsCurrentlyPressed()) {
      newNote -= 1;
    }
    if (ps.key_rh3.IsCurrentlyPressed()) {
      newNote -= 2;
    }
    if (ps.key_rh4.IsCurrentlyPressed()) {
      newNote -= 2;
    }

    if (ps.key_octave4.IsCurrentlyPressed()) {
      newNote += 12 * 4;  
    } else if (ps.key_octave3.IsCurrentlyPressed()) {
      newNote += 12 * 3;
    } else if (ps.key_octave2.IsCurrentlyPressed()) {
      newNote += 12 * 2;
    } else if (ps.key_octave1.IsCurrentlyPressed()) {
      newNote += 12 * 1;
    }

    // transpose
    newNote += gAppSettings.mTranspose;
    newNote = constrain(newNote, 1, 127);
    mLiveVoice.mNote = (uint8_t)newNote;

    needsNoteOn = isPlayingNote && (!wasPlayingNote || prevNote != newNote);
    // send note off in these cases:
    // - you are not playing but were
    // - or, you are playing, but a different note than before.
    needsNoteOff = (!isPlayingNote && wasPlayingNote) || (isPlayingNote && (prevNote != newNote));
    noteOffNote = prevNote;

    if (needsNoteOn) {
      noteOns ++;
    }

    nUpdates ++;

    DistributeVoices();
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
