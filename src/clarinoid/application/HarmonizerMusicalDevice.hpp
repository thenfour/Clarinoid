/*
it's not a simple design, but it's robust.

user presses a key,
    USB keyboard device receives event
    USB keyboard has a held note tracker because it has a sustain pedal. events interpreted
        the event is sent to MusicalState as a "physical note on", where harmonizer is notified.
        the event is also sent to a voicing mode interpreter for synth patch A and B
        voicing mode interpreter
        the event is also sent to MusicalState as a device note on, where it's a MusicalVoice with synth patch A and B
applied. MusicalState sends to synth

    Harmonizer forwards the physical note event to each harm layer.
        the layer's harmonized note is calculated, and played virtually through a
*/

#pragma once

#include <clarinoid/synth/MusicalVoice.hpp>
#include <clarinoid/synth/MusicalDevice.hpp>
#include <clarinoid/loopstation/harmonizer.hpp>

namespace clarinoid
{

// harmonizerdevice will need to track polyphony across all harmonized layers.
// USB keyboard device does keep track of notes in heldnotetracker, and that kind of device represents a single layer.

// due to voicing etc, each harm voice should have its own tracker.

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// track which notes are playing, in order to know which notes to release on note off.
// each harmonizer layer should really not be responsible for playing many notes.
struct HarmonizerDeviceLayer
{
    size_t mLayerIndex = 0;

    IMusicalDeviceEvents *mpEventHandler;
    void *mpCapture = nullptr;
    AppSettings *mpAppSettings = nullptr;
    ISynthParamProvider *mpParamProvider = nullptr;
    MusicalVoice mModelVoice;
    VoicingMode mLastKnownVoicingMode = VoicingMode::Polyphonic;

    HarmonizerLayer mHarmonizerLayer;

    // calculate before processing mVoicingInterpreter,
    // to be used without having to process when it calls back

    HarmonizerDeviceLayer()
    {
    }

    void Init(size_t layerIndex,
              IMusicalDeviceEvents *pEventHandler,
              void *pCapture,
              AppSettings &appSettings,
              ISynthParamProvider *pParamProvider)
    {
        mLayerIndex = layerIndex;
        mpEventHandler = pEventHandler;
        mpCapture = pCapture;
        mpAppSettings = &appSettings;
        mpParamProvider = pParamProvider;

        mModelVoice.mSource.mHarmonizerVoiceIndex = layerIndex;
        mModelVoice.mSource.mLoopstationLayerIndex = 0; // todo ... need to figure out how to do this.
        mModelVoice.mSource.mType = MusicalEventSourceType::Harmonizer;
        mModelVoice.mpParamProvider = pParamProvider;

        mHarmonizerLayer.Init(appSettings, layerIndex);
    }

    void OnNoteOn(const HeldNoteInfo &liveNote, const HarmPatch &harmPatch)
    {
        MusicalVoice mv = mHarmonizerLayer.GetHarmonizedNote(
            harmPatch, liveNote, mModelVoice, HarmonizerLayer::HarmonizeFlags::BecauseOfNoteOn);
        if (!mv.mIsActive)
        {
            // note is not harmonized / dropped per harmonizer layer.
            //Serial.println(String("harm layer VM note on, harmonized musical voice is OFF"));
            return;
        }

        //Serial.println(String("harm layer #") + mLayerIndex + " NOTE ON: " + mv.ToString());
        mpEventHandler->IMusicalDeviceEvents_OnDeviceNoteOn(mv, mpCapture);
    }

    // user has lifted a key (or removed pedal with unpressed keys).
    // note-off all notes of the given source id. i think this should always incur <=1 note offs.
    void OnNoteOff(const HeldNoteInfo &noteInfo, const HeldNoteInfo *trillNote, const HarmPatch &harmPatch)
    {
        auto& synthPatch = mpAppSettings->GetSynthPresetForHarmonizerLayer(harmPatch, mLayerIndex);
        if ((synthPatch.mVoicingMode == VoicingMode::Monophonic) && !!trillNote) {
            // monophonic note off with a valid trill note = simple note on of trill note.
            OnNoteOn(*trillNote, harmPatch);
            return;
        }

        // harmonize the note, which is not 100% necessary but it's a reliable way to convert noteInfo to MusicalVoice.
        MusicalVoice mv = mHarmonizerLayer.GetHarmonizedNote(
            harmPatch, noteInfo, mModelVoice, HarmonizerLayer::HarmonizeFlags::None);

        mpEventHandler->IMusicalDeviceEvents_OnDeviceNoteOff(mv, mpCapture);
    }

    void OnAllNotesOff(const HarmPatch &harmPatch)
    {
        mpEventHandler->IMusicalDeviceEvents_OnDeviceAllNotesOff(mModelVoice.mSource, mpCapture);
    }

    const MusicalVoice GetLiveMusicalVoice(const HarmPatch &harmPatch,
                                           const MusicalVoice &synthCurrentlyPlaying, // from synth
                                           const HeldNoteTracker &noteSource)
    {
        // if voicing mode changes, all notes off is critical.
        const auto& synthPatch = mpAppSettings->GetSynthPresetForHarmonizerLayer(harmPatch, mLayerIndex);
        if (mLastKnownVoicingMode != synthPatch.mVoicingMode) {
            mLastKnownVoicingMode = synthPatch.mVoicingMode;
            this->OnAllNotesOff(harmPatch);
        }

        const HeldNoteInfo *pSourceNote = noteSource.FindExisting(synthCurrentlyPlaying.mHarmonizerSourceNoteID);
        if (!pSourceNote)
        {
            // no longer playing
            MusicalVoice ret = synthCurrentlyPlaying;
            ret.mIsActive = false;
            return ret;
        }

        auto ret = mHarmonizerLayer.GetHarmonizedNote(
            harmPatch, *pSourceNote, synthCurrentlyPlaying, HarmonizerLayer::HarmonizeFlags::None);
        if (!ret.mIsActive)
            return ret;
        return ret;
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct HarmonizerMusicalDevice
{
    AppSettings &mAppSettings;

    // harmonized notes don't carry their own params; caller must provide.
    ISynthParamProvider *mpParamProvider = nullptr;
    IMusicalDeviceEvents *mpEventHandler;
    void *mpCapture = nullptr;

    HarmonizerDeviceLayer mLayers[HARM_VOICES];

    HarmonizerMusicalDevice(AppSettings &appSettings,
                            ISynthParamProvider *pParamProvider,
                            IMusicalDeviceEvents *pEventHandler,
                            void *pCapture)
        : mAppSettings(appSettings),       //
          mpParamProvider(pParamProvider), //
          mpEventHandler(pEventHandler),   //
          mpCapture(pCapture)              //
    {
        for (size_t i = 0; i < SizeofStaticArray(mLayers); ++i)
        {
            mLayers[i].Init(i, mpEventHandler, mpCapture, mAppSettings, pParamProvider);
        }
    }

    void OnSourceNoteOn(const HeldNoteInfo &noteInfo, const HarmPatch &harmPatch)
    {
        //Serial.println("harmonizer device source note on");
        for (auto &layer : mLayers)
        {
            layer.OnNoteOn(noteInfo, harmPatch);
        }
    }

    void OnSourceNoteOff(const HeldNoteInfo &noteInfo, const HeldNoteInfo *trillNote, const HarmPatch &harmPatch)
    {
        for (auto &layer : mLayers)
        {
            layer.OnNoteOff(noteInfo, trillNote, harmPatch);
        }
    }

    void OnSourceAllNotesOff(const HarmPatch &harmPatch)
    {
        for (auto &layer : mLayers)
        {
            layer.OnAllNotesOff(harmPatch);
        }
    }

    const MusicalVoice GetLiveMusicalVoice(const HarmPatch &harmPatch,
                                           const MusicalVoice &existing,
                                           HeldNoteTracker &noteSource)
    {
        CCASSERT(existing.mSource.mType == MusicalEventSourceType::Harmonizer);
        CCASSERT(existing.mSource.mHarmonizerVoiceIndex < HARM_VOICES);
        return mLayers[existing.mSource.mHarmonizerVoiceIndex].GetLiveMusicalVoice(harmPatch, existing, noteSource);
    }
};

static constexpr auto airchp99i = sizeof(MusicalVoice);
static constexpr auto airchpi = sizeof(HarmonizerMusicalDevice);

} // namespace clarinoid
