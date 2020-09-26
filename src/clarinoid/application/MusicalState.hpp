#pragma once

#include <clarinoid/harmonizer/harmonizer.hpp>
#include <clarinoid/loopstation/LooperHarmonizer.hpp>
#include "PhysicalState.hpp"

struct CCEWIMusicalState
{
  MusicalVoice mLiveVoice; // the voice you're literally physically playing.
  MusicalVoice mMusicalVoices[MAX_MUSICAL_VOICES]; // these are all the voices that WANT to be played (after transpose, harmonize, looping, etc). May be more than synth polyphony.
  size_t mVoiceCount = 0;

  LooperAndHarmonizer mLooper;

  // issue #26: TODO: create a time-based smoother (LPF). throttling and taking samples like this is not very accurate. sounds fine today though.
  SimpleMovingAverage<4> _incomingBreath01;// 0-1
  SimpleMovingAverage<4> _incomingPitchBendN11; // -1 to 1
  SimpleMovingAverage<15> mCurrentBreath01;
  SimpleMovingAverage<60> mCurrentPitchN11;

  CCThrottlerT<1> mPressureSensingThrottle;
  int nUpdates = 0;
  int noteOns = 0;

  CCEWIMusicalState() {
    this->_incomingBreath01.Update(0);
    this->mCurrentBreath01.Update(0);
    this->_incomingPitchBendN11.Update(0);
    this->mCurrentPitchN11.Update(0);
  }

  void Update(const CCEWIPhysicalState& ps)
  {
    MusicalVoice mNewState(mLiveVoice);

    // convert that to musical state. i guess this is where the 
    // most interesting EWI-ish logic is.

    if (nUpdates == 0 || mPressureSensingThrottle.IsReady())
    {
      {
        _incomingBreath01.Update(ps.breath01);
        float breath = _incomingBreath01.GetValue();
        breath = constrain(breath, gAppSettings.mBreathLowerBound, gAppSettings.mBreathUpperBound);
        breath = map(breath, gAppSettings.mBreathLowerBound, gAppSettings.mBreathUpperBound, 0.0f, 1.0f);
        breath = constrain(breath, 0.0f, 1.0f);
        mCurrentBreath01.Update(breath);
      }

      CCPlot(ps.pitchDown01);

      {
        _incomingPitchBendN11.Update(ps.pitchDown01);
        // see PitchStripSettings to understand the different regions.
        float pb = _incomingPitchBendN11.GetValue();

        if (pb <= gAppSettings.mPitchStrip.mHandsOffNoiseThresh) {
          // | IDLE | UP MAX | UP VARIABLE | ZERO | DOWN VARIABLE | DOWN MAX |
          // =======^
          mCurrentPitchN11.Update(0.0f);
        } else if (pb <= gAppSettings.mPitchStrip.mPitchUpMax) {
          // | IDLE | UP MAX | UP VARIABLE | ZERO | DOWN VARIABLE | DOWN MAX |
          //        ^========^
          mCurrentPitchN11.Update(1.0f);
        } else if (pb <= gAppSettings.mPitchStrip.mZeroMin) {
          // | IDLE | UP MAX | UP VARIABLE | ZERO | DOWN VARIABLE | DOWN MAX |
          //                 ^=============^
          float t = map(pb, gAppSettings.mPitchStrip.mZeroMin, gAppSettings.mPitchStrip.mPitchUpMax, 0, 1);
          mCurrentPitchN11.Update(t);
        } else if (pb <= gAppSettings.mPitchStrip.mZeroMax) {
          // | IDLE | UP MAX | UP VARIABLE | ZERO | DOWN VARIABLE | DOWN MAX |
          //                               ^======^
          mCurrentPitchN11.Update(0.0f);
        } else if (pb <= gAppSettings.mPitchStrip.mPitchDownMax) {
          // | IDLE | UP MAX | UP VARIABLE | ZERO | DOWN VARIABLE | DOWN MAX |
          //                                      ^===============^
          float t = map(pb, gAppSettings.mPitchStrip.mPitchDownMax, gAppSettings.mPitchStrip.mZeroMax, -1, 0);
          mCurrentPitchN11.Update(t);
        } else {
          // | IDLE | UP MAX | UP VARIABLE | ZERO | DOWN VARIABLE | DOWN MAX |
          //                                                      ^==========^
          mCurrentPitchN11.Update(-1);// full pitch down.
        }
      }

    }

    mNewState.mBreath01 = mCurrentBreath01.GetValue();
    bool isPlayingNote = mNewState.mBreath01.GetFloatVal() > gAppSettings.mBreathNoteOnThreshold;

    mNewState.mPitchBendN11 = mCurrentPitchN11.GetValue();
    mNewState.mVelocity = 100; // TODO

    mNewState.mHarmPatch = gAppSettings.mGlobalHarmPreset;
    mNewState.mSynthPatch = gAppSettings.mGlobalSynthPreset; 

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
    mNewState.mMidiNote = (uint8_t)newNote;

    if (!isPlayingNote) {
      mNewState.mVelocity = 0;
      mNewState.mMidiNote = 0;
    }

    auto transitionEvents = CalculateTransitionEvents(mLiveVoice, mNewState);

    mLiveVoice = mNewState;

    // we have calculated mLiveVoice, converting physical to live musical state.
    // now take the live musical state, and fills out mMusicalVoices based on harmonizer & looper settings.
    mVoiceCount = mLooper.Update(mLiveVoice, transitionEvents, mMusicalVoices, EndPtr(mMusicalVoices));
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

    // in addition to converting physical state to musical state, we interpret assignable controls here.
    if (mPhysicalState.key_rhExtra1.IsNewlyPressed()) {
      mMusicalState.mLooper.LoopIt(mMusicalState.mLiveVoice);
    }
    if (mPhysicalState.key_rhExtra2.IsNewlyPressed()) {
      mMusicalState.mLooper.Clear();
    }
  }
};

