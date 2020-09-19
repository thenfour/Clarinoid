#pragma once

#include <clarinoid/harmonizer/harmonizer.hpp>
#include <clarinoid/loopstation/Loopstation.hpp>
#include "PhysicalState.hpp"

struct CCEWIMusicalState
{
  MusicalVoice mLiveVoice; // the voice you're literally physically playing.
  MusicalVoice mMusicalVoices[MAX_MUSICAL_VOICES]; // these are all the voices that WANT to be played (after transpose, harmonize, looping, etc). May be more than synth polyphony.
  size_t mVoiceCount = 0;

  LooperAndHarmonizer mLooper;

  // TODO: create a time-based smoother (LPF). throttling and taking samples like this is not very accurate. sounds fine today though.
  SimpleMovingAverage<15> breath01;// 0-1
  SimpleMovingAverage<60> pitchBendN11; // -1 to 1
  CCThrottlerT<1> mPressureSensingThrottle;
  int nUpdates = 0;
  int noteOns = 0;

  CCEWIMusicalState() {
    this->breath01.Update(0);
    this->pitchBendN11.Update(0);
  }

  void Update(const CCEWIPhysicalState& ps)
  {
    uint8_t prevNote = mLiveVoice.mMidiNote;
    bool wasPlayingNote = mLiveVoice.mIsNoteCurrentlyOn;
    bool isPlayingNote = wasPlayingNote;

    // convert that to musical state. i guess this is where the 
    // most interesting EWI-ish logic is.

    if (nUpdates == 0 || mPressureSensingThrottle.IsReady()) {

      float breathAdj = map((float)ps.breath01, gAppSettings.mBreathLowerBound, gAppSettings.mBreathUpperBound, 0.0f, 1.0f);
      breathAdj = constrain(breathAdj, 0.0f, 1.0f);
      this->breath01.Update(breathAdj);

      this->pitchBendN11.Update(constrain(map((float)(ps.pitchDown01), gAppSettings.mPitchDownMin, gAppSettings.mPitchDownMax, 0.0f, -1.0f), -1.0f, 0.0f));
    }

    mLiveVoice.mBreath01 = this->breath01.GetValue();
    isPlayingNote = mLiveVoice.mBreath01.GetFloatVal() > gAppSettings.mBreathNoteOnThreshold;
    mLiveVoice.mIsNoteCurrentlyOn = isPlayingNote;

    mLiveVoice.mPitchBendN11 = this->pitchBendN11.GetValue();
    mLiveVoice.mVelocity = 100; // TODO
    mLiveVoice.mSynthPatch = 0; // TODO
    mLiveVoice.mHarmPatch = 0;// todo

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
    mLiveVoice.mMidiNote = (uint8_t)newNote;

    mLiveVoice.mNeedsNoteOn = isPlayingNote && (!wasPlayingNote || prevNote != newNote);
    // send note off in these cases:
    // - you are not playing but were
    // - or, you are playing, but a different note than before.
    mLiveVoice.mNeedsNoteOff = (!isPlayingNote && wasPlayingNote) || (isPlayingNote && (prevNote != newNote));
    mLiveVoice.mNoteOffNote = prevNote;

    if (mLiveVoice.mNeedsNoteOn) {
      noteOns ++;
      mLiveVoice.mDuration.Restart();
    }

    nUpdates ++;

    // we have calculated mLiveVoice, converting physical to live musical state.
    // now take the live musical state, and fills out mMusicalVoices based on harmonizer & looper settings.
    mVoiceCount = mLooper.Update(mLiveVoice, mMusicalVoices, EndPtr(mMusicalVoices));
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

