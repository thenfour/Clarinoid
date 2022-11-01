// musical note data flows through multiple stages of processing.
// 1. device level
//   - loopstation memory gets converted to note on/ off / etc events
//   - physical keyboard similarly get converted to noteon/off/etc events
//   at this stage, no synth patch data or harmonizing processing is applied. these are raw device notes.
//   MIDI parameters like CC values, pedal, pitch bend, are all stored by device.
// 2. musical state
//   takes device data and applies synth patch, harmonization.
// 3. synthesizer
//   takes "idealized" musical state and distributes to synth voices.

// Despite these 3 separate stages, MusicalState is the orchestrator of all this.
// it tells devices to update, handles their events, and then passes to synthesizer.

#pragma once

#include <clarinoid/synth/MusicalVoice.hpp>
#include <clarinoid/synth/MusicalDevice.hpp>
#include "USBKeyboardMusicalDevice.hpp"
#include "HarmonizerMusicalDevice.hpp"

namespace clarinoid
{

// orchestrates propagation of musical events from physical device to synthesizer
struct MusicalState : IMusicalDeviceEvents // for receiving musical event data from various devices
{
    AppSettings &mAppSettings;

    USBKeyboardMusicalDevice mUsbKeyboard;
    HarmonizerMusicalDevice mLiveHarmonizer;

    IMusicalEventsForSynth *mpSynth = nullptr;

    MusicalState(AppSettings &appSettings, IMusicalEventsForSynth *pSynth)
        : mAppSettings(appSettings),                                           //
          mUsbKeyboard(&appSettings, this, &mUsbKeyboard),                     //
          mLiveHarmonizer(appSettings, &mUsbKeyboard, this, &mLiveHarmonizer), //
          mpSynth(pSynth)
    {
    }

    virtual void IMusicalDeviceEvents_OnNoteOn(const MusicalVoice &mv, void *cap) override
    {
        mpSynth->IMusicalEventsForSynth_OnNoteOn(mv);
    }

    virtual void IMusicalDeviceEvents_OnNoteOff(const MusicalVoice &mv, void *cap) override
    {
        mpSynth->IMusicalEventsForSynth_OnNoteOff(mv);
    }

    virtual void IMusicalDeviceEvents_OnAllNotesOff(void *cap) override
    {
        // TODO: well this isn't really used, but i think if loopstation gives a "all notes off" event,
        // then probably means we should only cut loopstation notes. not sure.
        mpSynth->IMusicalEventsForSynth_OnAllNoteOff();
    }

    virtual void IMusicalDeviceEvents_OnPhysicalNoteOn(const HeldNoteInfo &noteInfo, void *cap) override
    {
        CCASSERT(cap == &mUsbKeyboard);
        // forward to things that care about physical musical events
        mLiveHarmonizer.OnSourceNoteOn(
            noteInfo, mAppSettings.FindHarmPreset(mAppSettings.GetCurrentPerformancePatch().mHarmPreset));
    }
    virtual void IMusicalDeviceEvents_OnPhysicalNoteOff(const HeldNoteInfo &noteInfo,
                                                        const HeldNoteInfo *trillNote,
                                                        void *cap) override
    {
        CCASSERT(cap == &mUsbKeyboard);
        // forward to things that care about physical musical events
        mLiveHarmonizer.OnSourceNoteOff(
            noteInfo, trillNote, mAppSettings.FindHarmPreset(mAppSettings.GetCurrentPerformancePatch().mHarmPreset));
    }
    virtual void IMusicalDeviceEvents_OnPhysicalAllNotesOff(void *cap) override
    {
        CCASSERT(cap == &mUsbKeyboard);
        // forward to things that care about physical musical events
        mLiveHarmonizer.OnSourceAllNotesOff(
            mAppSettings.FindHarmPreset(mAppSettings.GetCurrentPerformancePatch().mHarmPreset));
    }

    // synth voices need to be able to get any updates to the notes they're playing.
    // replace their existing copy with a live version.
    MusicalVoice GetLiveMusicalVoice(
        const MusicalVoice &existing) // non-const because we assign the non-const param provider lel
    {
        switch (existing.mSource.mType)
        {
        case MusicalEventSourceType::Null: {
            return {};
        }
        case MusicalEventSourceType::LivePlayA:
        case MusicalEventSourceType::LivePlayB:
            return mUsbKeyboard.GetLiveMusicalVoice(existing);
        case MusicalEventSourceType::Harmonizer:
            return mLiveHarmonizer.GetLiveMusicalVoice(existing);
        case MusicalEventSourceType::Loopstation:
            // TODO
            break;
        default:
            CCASSERT(!"unknown musical voice source");
            break;
        }
        return {};
    }

    void Update()
    {
        // update physical device
        mUsbKeyboard.Update();

        // update harmonizer, loopstation
    }
};

//     // updates devices, converts device notes into musical notes
// struct MusicalStateProcessor
// {
//     IDisplay *mpDisplay;
//     AppSettings *mAppSettings;
//     InputDelegator *mInput;
//     Metronome *mMetronome;
//     ScaleFollower *mScaleFollower;
//     IInputSource *mInputSrc;

//     int mLastInterval = 0;
//     int mIncomingMidiNote = 0;
//     int mIncomingMidiNoteAgeFrames = 0;

//     // MusicalVoice mIncomingVoice; // the voice that you are physically playing, but with age = 0. In order to
//     // avoid very
//     //                              // small glitchy notes, we don't apply a note change unless it's of a
//     //                              certain age.
//     MusicalVoice mLiveVoice; // the voice that's actually playing that represents what the player is playing
//                              // (not a harmonized voice or looped voice)
//     MusicalVoice
//         mMusicalVoices[MAX_MUSICAL_VOICES]; // these are all the voices that WANT to be played (after transpose,
//                                             // harmonize, looping, etc). May be more than synth polyphony.
//     size_t mVoiceCount = 0;

//     LooperAndHarmonizer mLooper;
//     CCEWIMIDIOut mMidiOut;

//     // - too many samples: you lose attack and things are sluggish
//     // - too few samples: noisy
//     SimpleMovingAverage<BREATH_SIGNAL_SMOOTHING_FRAMES> mCurrentBreath01;
//     SimpleMovingAverage<BREATH_SIGNAL_SMOOTHING_FRAMES> mCurrentPitchN11;

//     SwitchControlReader mLoopGoReader;
//     SwitchControlReader mLoopStopReader;
//     SwitchControlReader mHoldBasePitchReader;

//     SwitchControlReader mMetronomeLEDToggleReader;

//     SwitchControlReader mHarmPresetOnOffToggleReader;

//     int nUpdates = 0;
//     int noteOns = 0;

//     int mLastPlayedNote = 0; // valid even when mLiveVoice is not.
//     int mLiveOctave = 0; // whether or not you're playing a note, indicates the octave # you're fingering. used for
//     LED indication.

//     int mDefaultBaseNote = 49; // C#2
//     bool mHoldingBaseNote = false;
//     bool mWaitingForRelativePitchToChange = false;
//     int mBaseNoteToUseAfterRelativePitchChanges = 0;
//     int mRelativeNoteWhenPitchHeld = 0;
//     int mCurrentBaseNote = mDefaultBaseNote;

//     CCEWIMusicalState(IDisplay *pDisplay,
//                       AppSettings *appSettings,
//                       InputDelegator *inputDelegator,
//                       Metronome *metronome,
//                       ScaleFollower *scaleFollower,
//                       IInputSource *inputSrc)
//         : mpDisplay(pDisplay), mAppSettings(appSettings), mInput(inputDelegator), mMetronome(metronome),
//           mScaleFollower(scaleFollower), mInputSrc(inputSrc),

//           mLooper(appSettings, metronome, scaleFollower)
//     {
//         mMidiOut.setup();

//         this->mCurrentBreath01.Update(0);
//         this->mCurrentPitchN11.Update(0);
//     }

//     CCThrottlerT<600> mth;

//     void Update()
//     {
//         mIncomingMidiNoteAgeFrames++;
//         // mLiveVoice.mAgeFrames++;
//         MusicalVoice mNewState(mLiveVoice);

//         float _incomingBreath = mInput->mBreath.CurrentValue01(); // this is actually the transformed value.
//         mCurrentBreath01.Update(_incomingBreath);
//         mNewState.mBreath01 = mCurrentBreath01.GetValue();
//         bool isPlayingNote = mNewState.mBreath01.GetFloatVal() >
//                              (.000001f); // rounding errors amplify; FLT_EPSILON would be too precise for this check.

//         mCurrentPitchN11.Update(mInput->mPitchBend.CurrentValueN11());
//         mNewState.mPitchBendN11 = mCurrentPitchN11.GetValue();
//         mNewState.mVelocity = 100;

//         auto &perf = mAppSettings->GetCurrentPerformancePatch();
//         mNewState.mHarmPatch = perf.mHarmPreset;
//         mNewState.mSynthPatchA = perf.mSynthPresetA;
//         mNewState.mSynthPatchB = perf.mSynthPresetB;
//         mNewState.mPan = 0; // panning is part of musical state because harmonizer needs it per voice, but it's
//                             // actually calculated by the synth based on synth preset.

//         // the rules are rather weird for keys. open is a C#...
//         // https://bretpimentel.com/flexible-ewi-fingerings/
//         int relativeNote = KeyStateToRelativeNote(mInput->mKeyLH1.CurrentValue(),
//                                                   mInput->mKeyLH2.CurrentValue(),
//                                                   mInput->mKeyLH3.CurrentValue(),
//                                                   mInput->mKeyLH4.CurrentValue(),
//                                                   mInput->mKeyRH1.CurrentValue(),
//                                                   mInput->mKeyRH2.CurrentValue(),
//                                                   mInput->mKeyRH3.CurrentValue(),
//                                                   mInput->mKeyRH4.CurrentValue());

//         mLiveOctave = 2;

// #ifdef THREE_BUTTON_OCTAVES
//         // Buttons   Transpose:   mLiveOctave
//         //     1+2   -24               0
//         //     1     -12               1
//         //           0                 2
//         //     2     +12               3
//         //     2+3   +24               4
//         //     3     +36               5
//         if (mInput->mKeyOct1.CurrentValue())
//         {
//             if (mInput->mKeyOct2.CurrentValue())
//             {
//                 mLiveOctave = 0;
//                 relativeNote -= 24; // holding both 1&2 keys = sub-bass
//             }
//             else
//             {
//                 mLiveOctave = 1;
//                 relativeNote -= 12; // button 1 only
//             }
//         }
//         else if (mInput->mKeyOct2.CurrentValue())
//         {
//             if (mInput->mKeyOct3.CurrentValue())
//             {
//                 mLiveOctave = 4;
//                 relativeNote += 24; // holding 2+3
//             }
//             else
//             {
//                 mLiveOctave = 3;
//                 relativeNote += 12; // holding only 2
//             }
//         }
//         else if (mInput->mKeyOct3.CurrentValue())
//         {
//             mLiveOctave = 5;
//             relativeNote += 36; // holding only 3
//         }
// #endif
// #ifdef SIX_OCTAVE_SEQ_BUTTONS
//         if (mInput->mKeyOct6.CurrentValue())
//         {
//             mLiveOctave = 5;
//             relativeNote += 36;
//         }
//         else if (mInput->mKeyOct5.CurrentValue())
//         {
//             mLiveOctave = 4;
//             relativeNote += 24;
//         }
//         else if (mInput->mKeyOct4.CurrentValue())
//         {
//             mLiveOctave = 3;
//             relativeNote += 12;
//         }
//         else if (mInput->mKeyOct3.CurrentValue())
//         {
//             // no change. act like nothing is pressed.
//         }
//         else if (mInput->mKeyOct2.CurrentValue())
//         {
//             mLiveOctave = 1;
//             relativeNote -= 12;
//         }
//         else if (mInput->mKeyOct1.CurrentValue())
//         {
//             mLiveOctave = 0;
//             relativeNote -= 24;
//         }
// #endif

//         // transpose
//         relativeNote += mAppSettings->GetCurrentPerformancePatch().mTranspose;

//         // hold pitch is cool, but if we set the new base pitch while you're holding keys down (which is kinda
//         // 100%), then you immediately start playing relative to the existing pitch. very ugly, and literally
//         // never intended. so continue playing the existing note until the relative pitch changes.
//         int newNote = 0;
//         if (mWaitingForRelativePitchToChange && (mRelativeNoteWhenPitchHeld != relativeNote))
//         {
//             mWaitingForRelativePitchToChange = false;
//             this->mCurrentBaseNote = mBaseNoteToUseAfterRelativePitchChanges;
//         }

//         newNote = relativeNote + this->mCurrentBaseNote;
//         newNote = constrain(newNote, 1, 127);
//         mNewState.mMidiNote = (uint8_t)newNote;

//         if (!isPlayingNote)
//         {
//             mNewState.mVelocity = 0;
//             mNewState.mMidiNote = 0;
//         }
//         else
//         {
//             mLastPlayedNote = mNewState.mMidiNote;
//         }

//         if (mNewState.IsPlaying() && (mIncomingMidiNote != mNewState.mMidiNote))
//         {
//             mLastInterval = std::abs(mLiveVoice.mMidiNote - mNewState.mMidiNote);
//             mIncomingMidiNote = mNewState.mMidiNote;
//             mIncomingMidiNoteAgeFrames = 0;
//         }

//         int smoothingFrames = mAppSettings->mNoteChangeSmoothingFrames +
//                               (mLastInterval * mAppSettings->mNoteChangeSmoothingIntervalFrameFactor);

//         // force continuation of live note if this note is just too damned freshh
//         if (mIncomingMidiNoteAgeFrames <= smoothingFrames)
//         {
//             // only do this if it's a note transition, not from off to on.
//             if (mLiveVoice.IsPlaying())
//             {
//                 mNewState.mMidiNote = mLiveVoice.mMidiNote;
//             }
//         }

//         auto transitionEvents = CalculateTransitionEvents(mLiveVoice, mNewState);

//         mLiveVoice = mNewState;

//         mLoopGoReader.Update(&mInput->mLoopGoButton);
//         mLoopStopReader.Update(&mInput->mLoopStopButton);
//         if (mLoopGoReader.IsNewlyPressed())
//         {
//             mLooper.LoopIt(mLiveVoice);
//         }
//         if (mLoopStopReader.IsNewlyPressed())
//         {
//             mLooper.Clear();
//         }

//         mHoldBasePitchReader.Update(&mInput->mBaseNoteHoldToggle);
//         if (mHoldBasePitchReader.IsNewlyPressed())
//         {
//             mHoldingBaseNote = !mHoldingBaseNote;
//             if (mHoldingBaseNote)
//             {
//                 // set up the pitch hold scenario.
//                 mBaseNoteToUseAfterRelativePitchChanges = mLastPlayedNote;
//                 mRelativeNoteWhenPitchHeld = relativeNote;
//                 mWaitingForRelativePitchToChange = true;
//                 MidiNote note(mLastPlayedNote);
//                 mInputSrc->InputSource_ShowToast(String("HOLD: ") + note.ToString());
//             }
//             else
//             {
//                 mInputSrc->InputSource_ShowToast(String("Release note"));
//                 mRelativeNoteWhenPitchHeld = false;
//                 mCurrentBaseNote = mDefaultBaseNote;
//             }
//         }

//         // this really doesn't belong here but while we're on the topic of reading inputs...
//         mMetronomeLEDToggleReader.Update(&mInput->mMetronomeLEDToggle);
//         if (mMetronomeLEDToggleReader.IsNewlyPressed())
//         {
//             mAppSettings->mMetronomeLED = !mAppSettings->mMetronomeLED;
//             mInputSrc->InputSource_ShowToast(String("Metronome LED: ") +
//                                              ((mAppSettings->mMetronomeLED) ? "on" : "off"));
//         }

//         mHarmPresetOnOffToggleReader.Update(&mInput->mHarmPresetOnOffToggle);
//         if (mHarmPresetOnOffToggleReader.IsNewlyPressed())
//         {
//             if (perf.mHarmEnabled && perf.mHarmPreset)
//             {
//                 perf.mHarmEnabled = false;
//                 mpDisplay->ShowToast(String("Harmonizer OFF\r\n") +
//                                      mAppSettings->mHarmSettings.mPresets[perf.mHarmPreset].ToString(perf.mHarmPreset));
//             }
//             else if (!perf.mHarmEnabled && perf.mHarmPreset)
//             {
//                 perf.mHarmEnabled = true;
//                 mpDisplay->ShowToast(String("Harmonizer ON\r\n") + mAppSettings->GetHarmPatchName(perf.mHarmPreset));
//             }
//         }

//         // we have calculated mLiveVoice, converting physical to live musical state.
//         // now take the live musical state, and fills out mMusicalVoices based on harmonizer & looper settings.
//         size_t newVoiceCount = mLooper.Update(mLiveVoice, transitionEvents, mMusicalVoices, EndPtr(mMusicalVoices));
//         mVoiceCount = newVoiceCount;

//         mMidiOut.Update(mLiveVoice, transitionEvents);
//     }
// };

// static constexpr auto aoechusnthcphic = sizeof(CCEWIMusicalState);

} // namespace clarinoid
