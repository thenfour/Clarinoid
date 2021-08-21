
#pragma once

#include <clarinoid/harmonizer/harmonizer.hpp>
#include <clarinoid/loopstation/LooperHarmonizer.hpp>
#include <clarinoid/application/ControlMapper.hpp>

namespace clarinoid
{

struct CCEWIMusicalState
{
    IDisplay *mpDisplay;
    AppSettings *mAppSettings;
    InputDelegator *mInput;
    Metronome *mMetronome;
    ScaleFollower *mScaleFollower;
    IInputSource *mInputSrc;

    int mLastInterval = 0;
    int mIncomingMidiNote = 0;
    int mIncomingMidiNoteAgeFrames = 0;

    // MusicalVoice mIncomingVoice; // the voice that you are physically playing, but with age = 0. In order to avoid
    // very
    //                              // small glitchy notes, we don't apply a note change unless it's of a certain age.
    MusicalVoice mLiveVoice; // the voice that's actually playing that represents what the player is playing (not a
                             // harmonized voice or looped voice)
    MusicalVoice
        mMusicalVoices[MAX_MUSICAL_VOICES]; // these are all the voices that WANT to be played (after transpose,
                                            // harmonize, looping, etc). May be more than synth polyphony.
    size_t mVoiceCount = 0;

    LooperAndHarmonizer mLooper;
    CCEWIMIDIOut mMidiOut;

    // - too many samples: you lose attack and things are sluggish
    // - too few samples: noisy
    SimpleMovingAverage<BREATH_SIGNAL_SMOOTHING_FRAMES> mCurrentBreath01;
    SimpleMovingAverage<BREATH_SIGNAL_SMOOTHING_FRAMES> mCurrentPitchN11;

    SwitchControlReader mLoopGoReader;
    SwitchControlReader mLoopStopReader;
    SwitchControlReader mHoldBasePitchReader;

    SwitchControlReader mMetronomeLEDToggleReader;

    SwitchControlReader mHarmPresetOnOffToggleReader;

    int nUpdates = 0;
    int noteOns = 0;

    int mLastPlayedNote = 0; // valid even when mLiveVoice is not.

    int mDefaultBaseNote = 49; // C#2
    bool mHoldingBaseNote = false;
    bool mWaitingForRelativePitchToChange = false;
    int mBaseNoteToUseAfterRelativePitchChanges = 0;
    int mRelativeNoteWhenPitchHeld = 0;
    int mCurrentBaseNote = mDefaultBaseNote;

    CCEWIMusicalState(IDisplay *pDisplay,
                      AppSettings *appSettings,
                      InputDelegator *inputDelegator,
                      Metronome *metronome,
                      ScaleFollower *scaleFollower,
                      IInputSource *inputSrc)
        : mpDisplay(pDisplay), mAppSettings(appSettings), mInput(inputDelegator), mMetronome(metronome),
          mScaleFollower(scaleFollower), mInputSrc(inputSrc),

          mLooper(appSettings, metronome, scaleFollower)
    {
        mMidiOut.setup();

        this->mCurrentBreath01.Update(0);
        this->mCurrentPitchN11.Update(0);
    }

    CCThrottlerT<600> mth;

    void Update()
    {
        mIncomingMidiNoteAgeFrames++;
        // mLiveVoice.mAgeFrames++;
        MusicalVoice mNewState(mLiveVoice);

        float _incomingBreath = mInput->mBreath.CurrentValue01(); // this is actually the transformed value.
        mCurrentBreath01.Update(_incomingBreath);
        mNewState.mBreath01 = mCurrentBreath01.GetValue();
        bool isPlayingNote = mNewState.mBreath01.GetFloatVal() >
                             (.000001f); // rounding errors amplify; FLT_EPSILON would be too precise for this check.

        mCurrentPitchN11.Update(mInput->mPitchBend.CurrentValueN11());
        mNewState.mPitchBendN11 = mCurrentPitchN11.GetValue();
        mNewState.mVelocity = 100;

        auto &perf = mAppSettings->GetCurrentPerformancePatch();
        mNewState.mHarmPatch = perf.mHarmPreset;
        mNewState.mSynthPatchA = perf.mSynthPresetA;
        mNewState.mSynthPatchB = perf.mSynthPresetB;
        mNewState.mPan = 0; // panning is part of musical state because harmonizer needs it per voice, but it's actually
                            // calculated by the synth based on synth preset.

        // the rules are rather weird for keys. open is a C#...
        // https://bretpimentel.com/flexible-ewi-fingerings/
        int relativeNote = 0;
        if (mInput->mKeyLH1.CurrentValue())
        {
            relativeNote -= 2;
        }
        if (mInput->mKeyLH2.CurrentValue())
        {
            relativeNote -= mInput->mKeyLH1.CurrentValue() ? 2 : 1;
        }
        if (mInput->mKeyLH3.CurrentValue())
        {
            relativeNote -= 2;
        }
        if (mInput->mKeyLH4.CurrentValue())
        {
            relativeNote += 1;
        }

        if (mInput->mKeyRH1.CurrentValue())
        {
            // naturally we expect this to be -2 (for G-F trill for example)
            // but we need to support Bb.
            if (mInput->mKeyLH1.CurrentValue() && !mInput->mKeyLH2.CurrentValue())
            {
                relativeNote -= 1;
            }
            else
            {
                relativeNote -= 2;
            }
        }
        if (mInput->mKeyRH2.CurrentValue())
        {
            relativeNote -= 1;
        }
        if (mInput->mKeyRH3.CurrentValue())
        {
            relativeNote -= mInput->mKeyRH2.CurrentValue() ? 2 : 1; // deviation from akai. feels more flute-friendly.
        }
        if (mInput->mKeyRH4.CurrentValue())
        {
            relativeNote -= 2;
        }

#ifdef THREE_BUTTON_OCTAVES
        // Buttons   Transpose:
        //     1+2   -24
        //     1     -12
        //           0
        //     2     +12
        //     2+3   +24
        //     3     +36
        if (mInput->mKeyOct1.CurrentValue())
        {
            if (mInput->mKeyOct2.CurrentValue())
            {
                relativeNote -= 24; // holding both 1&2 keys = sub-bass
            }
            else
            {
                relativeNote -= 12; // button 1 only
            }
        }
        else if (mInput->mKeyOct2.CurrentValue())
        {
            if (mInput->mKeyOct3.CurrentValue())
            {
                relativeNote += 24; // holding 2+3
            }
            else
            {
                relativeNote += 12; // holding only 2
            }
        }
        else if (mInput->mKeyOct3.CurrentValue())
        {
            relativeNote += 36; // holding only 3
        }
#endif
#ifdef SIX_OCTAVE_SEQ_BUTTONS
        if (mInput->mKeyOct6.CurrentValue())
        {
            relativeNote += 36;
        }
        else if (mInput->mKeyOct5.CurrentValue())
        {
            relativeNote += 24;
        }
        else if (mInput->mKeyOct4.CurrentValue())
        {
            relativeNote += 12;
        }
        else if (mInput->mKeyOct3.CurrentValue())
        {
            // no change. act like nothing is pressed.
        }
        else if (mInput->mKeyOct2.CurrentValue())
        {
            relativeNote -= 12;
        }
        else if (mInput->mKeyOct1.CurrentValue())
        {
            relativeNote -= 24;
        }
#endif

        // transpose
        relativeNote += mAppSettings->GetCurrentPerformancePatch().mTranspose;

        // hold pitch is cool, but if we set the new base pitch while you're holding keys down (which is kinda 100%),
        // then you immediately start playing relative to the existing pitch. very ugly, and literally never intended.
        // so continue playing the existing note until the relative pitch changes.
        int newNote = 0;
        if (mWaitingForRelativePitchToChange && (mRelativeNoteWhenPitchHeld != relativeNote))
        {
            mWaitingForRelativePitchToChange = false;
            this->mCurrentBaseNote = mBaseNoteToUseAfterRelativePitchChanges;
        }

        newNote = relativeNote + this->mCurrentBaseNote;
        newNote = constrain(newNote, 1, 127);
        mNewState.mMidiNote = (uint8_t)newNote;

        if (!isPlayingNote)
        {
            mNewState.mVelocity = 0;
            mNewState.mMidiNote = 0;
        }
        else
        {
            mLastPlayedNote = mNewState.mMidiNote;
        }

        if (mNewState.IsPlaying() && (mIncomingMidiNote != mNewState.mMidiNote))
        {
            mLastInterval = std::abs(mLiveVoice.mMidiNote - mNewState.mMidiNote);
            mIncomingMidiNote = mNewState.mMidiNote;
            mIncomingMidiNoteAgeFrames = 0;
        }

        int smoothingFrames = mAppSettings->mNoteChangeSmoothingFrames +
                              (mLastInterval * mAppSettings->mNoteChangeSmoothingIntervalFrameFactor);

        // force continuation of live note if this note is just too damned freshh
        if (mIncomingMidiNoteAgeFrames <= smoothingFrames)
        {
            // only do this if it's a note transition, not from off to on.
            if (mLiveVoice.IsPlaying())
            {
                mNewState.mMidiNote = mLiveVoice.mMidiNote;
            }
        }

        auto transitionEvents = CalculateTransitionEvents(mLiveVoice, mNewState);

        mLiveVoice = mNewState;

        mLoopGoReader.Update(&mInput->mLoopGoButton);
        mLoopStopReader.Update(&mInput->mLoopStopButton);
        if (mLoopGoReader.IsNewlyPressed())
        {
            mLooper.LoopIt(mLiveVoice);
        }
        if (mLoopStopReader.IsNewlyPressed())
        {
            mLooper.Clear();
        }

        mHoldBasePitchReader.Update(&mInput->mBaseNoteHoldToggle);
        if (mHoldBasePitchReader.IsNewlyPressed())
        {
            mHoldingBaseNote = !mHoldingBaseNote;
            if (mHoldingBaseNote)
            {
                // set up the pitch hold scenario.
                mBaseNoteToUseAfterRelativePitchChanges = mLastPlayedNote;
                mRelativeNoteWhenPitchHeld = relativeNote;
                mWaitingForRelativePitchToChange = true;
                MidiNote note(mLastPlayedNote);
                mInputSrc->InputSource_ShowToast(String("HOLD: ") + note.ToString());
            }
            else
            {
                mInputSrc->InputSource_ShowToast(String("Release note"));
                mRelativeNoteWhenPitchHeld = false;
                mCurrentBaseNote = mDefaultBaseNote;
            }
        }

        // this really doesn't belong here but while we're on the topic of reading inputs...
        mMetronomeLEDToggleReader.Update(&mInput->mMetronomeLEDToggle);
        if (mMetronomeLEDToggleReader.IsNewlyPressed())
        {
            mAppSettings->mMetronomeLED = !mAppSettings->mMetronomeLED;
            mInputSrc->InputSource_ShowToast(String("Metronome LED: ") +
                                             ((mAppSettings->mMetronomeLED) ? "on" : "off"));
        }

        mHarmPresetOnOffToggleReader.Update(&mInput->mHarmPresetOnOffToggle);
        if (mHarmPresetOnOffToggleReader.IsNewlyPressed())
        {
            if (perf.mHarmEnabled && perf.mHarmPreset)
            {
                perf.mHarmEnabled = false;
                mpDisplay->ShowToast(String("Harmonizer OFF\r\n") +
                                     mAppSettings->mHarmSettings.mPresets[perf.mHarmPreset].ToString(perf.mHarmPreset));
            }
            else if (!perf.mHarmEnabled && perf.mHarmPreset)
            {
                perf.mHarmEnabled = true;
                mpDisplay->ShowToast(String("Harmonizer ON\r\n") + mAppSettings->GetHarmPatchName(perf.mHarmPreset));
            }
        }

        // we have calculated mLiveVoice, converting physical to live musical state.
        // now take the live musical state, and fills out mMusicalVoices based on harmonizer & looper settings.
        size_t newVoiceCount = mLooper.Update(mLiveVoice, transitionEvents, mMusicalVoices, EndPtr(mMusicalVoices));
        mVoiceCount = newVoiceCount;

        mMidiOut.Update(mLiveVoice, transitionEvents);
    }
};

static constexpr auto aoeusnthcphic = sizeof (CCEWIMusicalState);

} // namespace clarinoid
