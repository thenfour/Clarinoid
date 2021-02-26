
#pragma once

#include <clarinoid/harmonizer/harmonizer.hpp>
#include <clarinoid/loopstation/LooperHarmonizer.hpp>
#include <clarinoid/application/ControlMapper.hpp>

namespace clarinoid
{

  struct CCEWIMusicalState
  {
    AppSettings *mAppSettings;
    InputDelegator *mInput;
    Metronome *mMetronome;
    ScaleFollower *mScaleFollower;

    MusicalVoice mLiveVoice;                         // the voice you're literally physically playing.
    MusicalVoice mMusicalVoices[MAX_MUSICAL_VOICES]; // these are all the voices that WANT to be played (after transpose, harmonize, looping, etc). May be more than synth polyphony.
    size_t mVoiceCount = 0;

    LooperAndHarmonizer mLooper;
    CCEWIMIDIOut mMidiOut;

    // issue #26: TODO: create a time-based smoother (LPF). throttling and taking samples like this is not very accurate. sounds fine today though.
    SimpleMovingAverage<15> mCurrentBreath01;
    SimpleMovingAverage<60> mCurrentPitchN11;

    int nUpdates = 0;
    int noteOns = 0;

    CCEWIMusicalState(AppSettings *appSettings, InputDelegator *inputDelegator, Metronome *metronome, ScaleFollower *scaleFollower) : mAppSettings(appSettings),
                                                                                                                                      mInput(inputDelegator),
                                                                                                                                      mMetronome(metronome),
                                                                                                                                      mScaleFollower(scaleFollower),

                                                                                                                                      mLooper(appSettings, metronome, scaleFollower)
    {
      mMidiOut.setup();
      this->mCurrentBreath01.Update(0);
      this->mCurrentPitchN11.Update(0);
    }

    void Update()
    {
      MusicalVoice mNewState(mLiveVoice);

      // convert that to musical state. i guess this is where the
      // most interesting EWI-ish logic is.

      float _incomingBreath = mInput->mBreath.CurrentValue01(); // this is actually the transformed value.
      mCurrentBreath01.Update(_incomingBreath);
      mNewState.mBreath01 = mCurrentBreath01.GetValue();
      bool isPlayingNote = mNewState.mBreath01.GetFloatVal() > 0.005; //mAppSettings->mBreathCalibration.mNoteOnThreshold;

      mCurrentPitchN11.Update(mInput->mPitchBend.CurrentValueN11());
      mNewState.mPitchBendN11 = mCurrentPitchN11.GetValue();
      mNewState.mVelocity = 100;

      mNewState.mHarmPatch = mAppSettings->mGlobalHarmPreset;
      mNewState.mSynthPatch = mAppSettings->mGlobalSynthPreset;

      // the rules are rather weird for keys. open is a C#...
      // https://bretpimentel.com/flexible-ewi-fingerings/
      int newNote = 49; // C#2
      if (mInput->mKeyLH1.CurrentValue())
      {
        newNote -= 2;
      }
      if (mInput->mKeyLH2.CurrentValue())
      {
        newNote -= mInput->mKeyLH1.CurrentValue() ? 2 : 1;
      }
      if (mInput->mKeyLH3.CurrentValue())
      {
        newNote -= 2;
      }
      if (mInput->mKeyLH4.CurrentValue())
      {
        newNote += 1;
      }

      if (mInput->mKeyRH1.CurrentValue())
      {
        newNote -= mInput->mKeyLH3.CurrentValue() ? 2 : 1;
      }
      if (mInput->mKeyRH2.CurrentValue())
      {
        newNote -= 1;
      }
      if (mInput->mKeyRH3.CurrentValue())
      {
        newNote -= mInput->mKeyRH2.CurrentValue() ? 2 : 1; // deviation from akai. feels more flute-friendly.
      }
      if (mInput->mKeyRH4.CurrentValue())
      {
        newNote -= 2;
      }

#ifdef THREE_BUTTON_OCTAVES
      newNote += 0;
      if (mInput->mKeyOct1.CurrentValue())
      {
        if (mInput->mKeyOct2.CurrentValue())
        {
          newNote -= 24; // holding both 1&2 keys = sub-bass
        }
        else
        {
          newNote -= 12; // low key
        }
      }
      else if (mInput->mKeyOct2.CurrentValue())
      {
        newNote += 12; // high key
      }
      if (mInput->mKeyOct3.CurrentValue())
      {
        newNote += 24; // extra high key
      }
#else
      if (ps.key_octave4.IsCurrentlyPressed())
      {
        newNote += 12 * 4;
      }
      else if (ps.key_octave3.IsCurrentlyPressed())
      {
        newNote += 12 * 3;
      }
      else if (ps.key_octave2.IsCurrentlyPressed())
      {
        newNote += 12 * 2;
      }
      else if (ps.key_octave1.IsCurrentlyPressed())
      {
        newNote += 12 * 1;
      }
#endif

      // transpose
      newNote += mAppSettings->mTranspose;
      newNote = constrain(newNote, 1, 127);
      mNewState.mMidiNote = (uint8_t)newNote;

      if (!isPlayingNote)
      {
        mNewState.mVelocity = 0;
        mNewState.mMidiNote = 0;
      }

      auto transitionEvents = CalculateTransitionEvents(mLiveVoice, mNewState);

      mLiveVoice = mNewState;

      // we have calculated mLiveVoice, converting physical to live musical state.
      // now take the live musical state, and fills out mMusicalVoices based on harmonizer & looper settings.
      size_t newVoiceCount = mLooper.Update(mLiveVoice, transitionEvents, mMusicalVoices, EndPtr(mMusicalVoices));
      mVoiceCount = newVoiceCount;

      mMidiOut.Update(mLiveVoice, transitionEvents);
    }
  };

} // namespace clarinoid
