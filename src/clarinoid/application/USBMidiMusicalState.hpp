
#pragma once

#include <clarinoid/harmonizer/harmonizer.hpp>
#include <clarinoid/loopstation/LooperHarmonizer.hpp>
#include <clarinoid/application/ControlMapper.hpp>

namespace clarinoid
{
    struct USBMidiReader
    {
        USBHost mUsb;
        MIDIDevice mMidiDevice;

        bool mIsPlaying = false;
        uint8_t mCurrentNote = 0; // ignore if mIsPlaying = false
        uint8_t mCurrentVelocity = 0; // ignore if mIsPlaying = false
        float mCurrentPitchBendN11 = 0;
        float mCurrentMod01 = 0;

        USBMidiReader() : mUsb(), mMidiDevice(mUsb)
        {
            mUsb.begin();
            // midi1.setHandleNoteOff(OnNoteOff);
            // midi1.setHandleNoteOn(OnNoteOn);
            // midi1.setHandleControlChange(OnControlChange);
        }

        void Update()
        {
            mUsb.Task();
            mMidiDevice.read();
        }
    };

// this serves the same function as CCEWIMusicalState, but instead of
// musical state coming from breath control, pitch device, and touch keys,
// the musical state comes from a USB MIDI device.
struct USBMidiMusicalState
{
    IDisplay *mpDisplay;
    AppSettings *mAppSettings;
    InputDelegator *mInput;
    Metronome *mMetronome;
    ScaleFollower *mScaleFollower;
    IInputSource *mInputSrc;
    USBMidiReader mMidiDevice;

    // MusicalVoice mIncomingVoice; // the voice that you are physically playing, but with age = 0. In order to
    // avoid very
    //                              // small glitchy notes, we don't apply a note change unless it's of a
    //                              certain age.
    MusicalVoice mLiveVoice; // the voice that's actually playing that represents what the player is playing
                             // (not a harmonized voice or looped voice)
    MusicalVoice
        mMusicalVoices[MAX_MUSICAL_VOICES]; // these are all the voices that WANT to be played (after transpose,
                                            // harmonize, looping, etc). May be more than synth polyphony.
    size_t mVoiceCount = 0;

    LooperAndHarmonizer mLooper;

    int nUpdates = 0;
    int noteOns = 0;

    int mLastPlayedNote = 0; // valid even when mLiveVoice is not.

    USBMidiMusicalState(IDisplay *pDisplay,
                      AppSettings *appSettings,
                      InputDelegator *inputDelegator,
                      Metronome *metronome,
                      ScaleFollower *scaleFollower,
                      IInputSource *inputSrc)
        : mpDisplay(pDisplay), mAppSettings(appSettings), mInput(inputDelegator), mMetronome(metronome),
          mScaleFollower(scaleFollower), mInputSrc(inputSrc),

          mLooper(appSettings, metronome, scaleFollower)
    {
    }

    CCThrottlerT<600> mth;

    void Update()
    {
        mMidiDevice.Update();
        MusicalVoice mNewState(mLiveVoice);

        mNewState.mBreath01 = mMidiDevice.mCurrentMod01;
        mNewState.mPitchBendN11 = mMidiDevice.mCurrentPitchBendN11;
        mNewState.mVelocity = mMidiDevice.mCurrentVelocity;

        auto &perf = mAppSettings->GetCurrentPerformancePatch();
        mNewState.mHarmPatch = perf.mHarmPreset;
        mNewState.mSynthPatchA = perf.mSynthPresetA;
        mNewState.mSynthPatchB = perf.mSynthPresetB;
        mNewState.mPan = 0; // panning is part of musical state because harmonizer needs it per voice, but it's
                            // actually calculated by the synth based on synth preset.

        mNewState.mMidiNote = ClampInclusive(mMidiDevice.mCurrentNote + mAppSettings->GetCurrentPerformancePatch().mTranspose, 0, 127);

        if (!mMidiDevice.mIsPlaying)
        {
            // enforce some later assumptions
            mNewState.mVelocity = 0;
            mNewState.mMidiNote = 0;
        }
        else
        {
            mLastPlayedNote = mNewState.mMidiNote;
        }

        auto transitionEvents = CalculateTransitionEvents(mLiveVoice, mNewState);

        mLiveVoice = mNewState;

        // we have calculated mLiveVoice, converting physical to live musical state.
        // now take the live musical state, and fills out mMusicalVoices based on harmonizer & looper settings.
        size_t newVoiceCount = mLooper.Update(mLiveVoice, transitionEvents, mMusicalVoices, EndPtr(mMusicalVoices));
        mVoiceCount = newVoiceCount;
    }
};

static constexpr auto aoeusnthcphic = sizeof(USBMidiMusicalState);

} // namespace clarinoid
