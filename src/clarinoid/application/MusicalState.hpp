#pragma once

#include <clarinoid/harmonizer/harmonizer.hpp>
#include <clarinoid/loopstation/LooperHarmonizer.hpp>
#include <clarinoid/application/ControlMapper.hpp>

namespace clarinoid
{

struct CCEWIMusicalState
{
  AppSettings* mAppSettings;
  IControlMapper* mControlMapper;
  Metronome* mMetronome;
  ScaleFollower* mScaleFollower;

  MusicalVoice mLiveVoice; // the voice you're literally physically playing.
  MusicalVoice mMusicalVoices[MAX_MUSICAL_VOICES]; // these are all the voices that WANT to be played (after transpose, harmonize, looping, etc). May be more than synth polyphony.
  size_t mVoiceCount = 0;

  LooperAndHarmonizer mLooper;

  // issue #26: TODO: create a time-based smoother (LPF). throttling and taking samples like this is not very accurate. sounds fine today though.
  //SimpleMovingAverage<4> _incomingBreath01;// 0-1
  //SimpleMovingAverage<4> _incomingPitchBendN11; // -1 to 1
  SimpleMovingAverage<15> mCurrentBreath01;
  SimpleMovingAverage<60> mCurrentPitchN11;

  //CCThrottlerT<1> mPressureSensingThrottle;
  int nUpdates = 0;
  int noteOns = 0;

  CCEWIMusicalState(AppSettings* appSettings, IControlMapper* controlMapper, Metronome* metronome, ScaleFollower* scaleFollower) :
    mAppSettings(appSettings),
    mControlMapper(controlMapper),
    mMetronome(metronome),
    mScaleFollower(scaleFollower),

    mLooper(appSettings, metronome, scaleFollower)
  {
    //this->_incomingBreath01.Update(0);
    this->mCurrentBreath01.Update(0);
    //this->_incomingPitchBendN11.Update(0);
    this->mCurrentPitchN11.Update(0);
  }

  void Update()
  {
    MusicalVoice mNewState(mLiveVoice);

    // convert that to musical state. i guess this is where the 
    // most interesting EWI-ish logic is.

    float _incomingBreath = mControlMapper->BreathSensor()->CurrentValue01();
    _incomingBreath = map(_incomingBreath, 0.1f, 7.0f, 0.0f, 1.0f);
    mCurrentBreath01.Update(_incomingBreath);

    // if (nUpdates == 0 || mPressureSensingThrottle.IsReady())
    // {
    //   {
    //     _incomingBreath01.Update(mControlMapper->BreathSensor()->CurrentValue01());
    //     float breath = _incomingBreath01.GetValue();
    //     breath = constrain(breath, mAppSettings->mBreathLowerBound, mAppSettings->mBreathUpperBound);
    //     breath = map(breath, mAppSettings->mBreathLowerBound, mAppSettings->mBreathUpperBound, 0.0f, 1.0f);
    //     breath = constrain(breath, 0.0f, 1.0f);
    //     mCurrentBreath01.Update(breath);
    //   }

    //   {
    //     _incomingPitchBendN11.Update(gControlMapper.PitchStrip().GetValue01());
    //     // see PitchStripSettings to understand the different regions.
    //     float pb = _incomingPitchBendN11.GetValue();

    //     // #30: idle zone should LATCH existing pitch bend (i.e. just don't touch it) value.
    //     // to make that as quick as possible, bypass any signal filtering for this check. worst case we miss some subtle pitch-up change events.
    //     if (ps.pitchDown01 <= gAppSettings.mPitchStrip.mHandsOffNoiseThresh || pb <= gAppSettings.mPitchStrip.mHandsOffNoiseThresh) {
    //       // | IDLE | UP MAX | UP VARIABLE | ZERO | DOWN VARIABLE | DOWN MAX |
    //       // =======^
    //     } else if (pb <= gAppSettings.mPitchStrip.mPitchUpMax) {
    //       // | IDLE | UP MAX | UP VARIABLE | ZERO | DOWN VARIABLE | DOWN MAX |
    //       //        ^========^
    //       mCurrentPitchN11.Update(1.0f);
    //     } else if (pb <= gAppSettings.mPitchStrip.mZeroMin) {
    //       // | IDLE | UP MAX | UP VARIABLE | ZERO | DOWN VARIABLE | DOWN MAX |
    //       //                 ^=============^
    //       float t = map(pb, gAppSettings.mPitchStrip.mZeroMin, gAppSettings.mPitchStrip.mPitchUpMax, 0, 1);
    //       mCurrentPitchN11.Update(t);
    //     } else if (pb <= gAppSettings.mPitchStrip.mZeroMax) {
    //       // | IDLE | UP MAX | UP VARIABLE | ZERO | DOWN VARIABLE | DOWN MAX |
    //       //                               ^======^
    //       mCurrentPitchN11.Update(0.0f);
    //     } else if (pb <= gAppSettings.mPitchStrip.mPitchDownMax) {
    //       // | IDLE | UP MAX | UP VARIABLE | ZERO | DOWN VARIABLE | DOWN MAX |
    //       //                                      ^===============^
    //       float t = map(pb, gAppSettings.mPitchStrip.mPitchDownMax, gAppSettings.mPitchStrip.mZeroMax, -1, 0);
    //       mCurrentPitchN11.Update(t);
    //     } else {
    //       // | IDLE | UP MAX | UP VARIABLE | ZERO | DOWN VARIABLE | DOWN MAX |
    //       //                                                      ^==========^
    //       mCurrentPitchN11.Update(-1);// full pitch down.
    //     }
    //   }

    //}

    mNewState.mBreath01 = mCurrentBreath01.GetValue();
    bool isPlayingNote = mNewState.mBreath01.GetFloatVal() > mAppSettings->mBreathNoteOnThreshold;

    mNewState.mPitchBendN11 = mCurrentPitchN11.GetValue();
    mNewState.mVelocity = 100; // TODO

    mNewState.mHarmPatch = mAppSettings->mGlobalHarmPreset;
    mNewState.mSynthPatch = mAppSettings->mGlobalSynthPreset; 

    // the rules are rather weird for keys. open is a C#...
    // https://bretpimentel.com/flexible-ewi-fingerings/
    int newNote = 49; // C#2
    if (mControlMapper->KeyLH1()->CurrentValue()){
      newNote -= 2;
    }
    if (mControlMapper->KeyLH2()->CurrentValue()) {
      newNote -= mControlMapper->KeyLH1()->CurrentValue() ? 2 : 1;
    }
    if (mControlMapper->KeyLH3()->CurrentValue()) {
      newNote -= 2;
    }
    if (mControlMapper->KeyLH4()->CurrentValue()) {
      newNote += 1;
    }

    if (mControlMapper->KeyRH1()->CurrentValue()) {
      newNote -= mControlMapper->KeyLH3()->CurrentValue() ? 2 : 1;
    }
    if (mControlMapper->KeyRH2()->CurrentValue()) {
      newNote -= 1;
    }
    if (mControlMapper->KeyRH3()->CurrentValue()) {
      newNote -= mControlMapper->KeyRH2()->CurrentValue() ? 2 : 1; // deviation from akai. feels more flute-friendly.
    }
    if (mControlMapper->KeyRH4()->CurrentValue()) {
      newNote -= 2;
    }

#ifdef THREE_BUTTON_OCTAVES
    // todo.
    newNote += 0;
    if (mControlMapper->KeyOct1()->CurrentValue()) {
      newNote -= 12;
    }
    if (mControlMapper->KeyOct3()->CurrentValue()) {
      newNote += 12;
    }
#else
    if (ps.key_octave4.IsCurrentlyPressed()) {
      newNote += 12 * 4;  
    } else if (ps.key_octave3.IsCurrentlyPressed()) {
      newNote += 12 * 3;
    } else if (ps.key_octave2.IsCurrentlyPressed()) {
      newNote += 12 * 2;
    } else if (ps.key_octave1.IsCurrentlyPressed()) {
      newNote += 12 * 1;
    }
#endif

    // transpose
    newNote += mAppSettings->mTranspose;
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
    size_t newVoiceCount = mLooper.Update(mLiveVoice, transitionEvents, mMusicalVoices, EndPtr(mMusicalVoices));
    // if (newVoiceCount != mVoiceCount)
    // {
    //   Serial.println(String("VOICE COUNT: ") + newVoiceCount);
    // }
    mVoiceCount = newVoiceCount;
  }
};

} // namespace clarinoid


