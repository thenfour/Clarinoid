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
// some musical state depends directly on hardware, like sustain pedal and analog controls; those are also handled here
struct MusicalState : IMusicalDeviceEvents // for receiving musical event data from various devices
{
    AppSettings &mAppSettings;
    InputDelegator &mInput;

    USBKeyboardMusicalDevice mUsbKeyboard;
    HarmonizerMusicalDevice mLiveHarmonizer;

    IMusicalEventsForSynth *mpSynth = nullptr;

    MusicalState(AppSettings &appSettings, InputDelegator &input, IMusicalEventsForSynth *pSynth)
        : mAppSettings(appSettings),                                           //
          mInput(input),                                                       //
          mUsbKeyboard(&appSettings, this, &mUsbKeyboard),                     //
          mLiveHarmonizer(appSettings, &mUsbKeyboard, this, &mLiveHarmonizer), //
          mpSynth(pSynth)
    {
    }

    virtual void IMusicalDeviceEvents_OnDeviceNoteOn(const MusicalVoice &mv, void *cap) override
    {
        mpSynth->IMusicalEventsForSynth_OnNoteOn(mv);
    }

    virtual void IMusicalDeviceEvents_OnDeviceNoteOff(const MusicalVoice &mv, void *cap) override
    {
        mpSynth->IMusicalEventsForSynth_OnNoteOff(mv);
    }

    virtual void IMusicalDeviceEvents_OnDeviceAllNotesOff(const MusicalEventSource &source, void *cap) override
    {
        // TODO: well this isn't really used, but i think if loopstation gives a "all notes off" event,
        // then probably means we should only cut loopstation notes. not sure.
        mpSynth->IMusicalEventsForSynth_OnAllNoteOff(source);
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
            return mLiveHarmonizer.GetLiveMusicalVoice(
                mAppSettings.FindHarmPreset(mAppSettings.GetCurrentPerformancePatch().mHarmPreset),
                existing,
                mUsbKeyboard.mHeldNotes);
        case MusicalEventSourceType::Loopstation:
            // TODO
            break;
        case MusicalEventSourceType::LoopstationHarmonizer:
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
        // update physical devices
        mUsbKeyboard.Update();

        // maybe a "better" design is if THIS class is a param source, and for certain params we get from the inputdelegator.
        // or we can keep the USB Keyboard as the param provider, and just tell it some stuff.
        for (size_t i = 0; i < 4; ++ i) {
            mUsbKeyboard.mMacroValues[i] = mInput.mMacroPots[i].CurrentValue01();
        }

        // update harmonizer, loopstation
    }
};

} // namespace clarinoid
