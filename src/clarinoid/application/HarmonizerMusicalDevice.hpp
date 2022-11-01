
#pragma once

#include <clarinoid/synth/MusicalVoice.hpp>
#include <clarinoid/synth/MusicalDevice.hpp>
#include <clarinoid/loopstation/harmonizer.hpp>

namespace clarinoid
{
/*

let's talk about harmonized polyphony. i don't actually think this should be the primary use, but it should be
supported. a vexing scenario: note ons get harmonized, great. now all those notes are not tracked anywhere though. so
the harmonizerdevice will need to track them. USB keyboard device does keep track of notes in heldnotetracker.

due to voicing etc, each harm voice should have its own tracker.

*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// track which notes are playing, in order to know which notes to release on note off.
// each harmonizer layer should really not be responsible for playing many notes.
struct HarmonizerDeviceLayer : IVoicingModeResultEvents
{
    size_t mLayerIndex = 0;
    // held notes
    // voicing interpreter
    VoicingModeInterpreter mVoicingInterpreter;

    HarmonizerDeviceLayer() : mVoicingInterpreter(this)
    {
    }

    void Init(size_t layerIndex)
    {
        mLayerIndex = layerIndex;
    }

    void OnNoteOn(const MusicalVoice &noteInfo, const HarmPreset &harmPatch)
    {
        //
    }

    void OnNoteOff(const HeldNoteInfo &noteInfo, const HeldNoteInfo *trillNote, const HarmPreset &harmPatch)
    {
        // note-off all notes of the given source id.
    }

    void OnAllNotesOff(const HarmPreset &harmPatch)
    {
        //
    }

    //
    virtual void IVoicingModeResultEvents_OnNoteOn(const HeldNoteInfo &noteInfo) override
    {
        //
    }
    virtual void IVoicingModeResultEvents_OnNoteOff(const HeldNoteInfo &noteInfo) override
    {
        //
    }
    virtual void IVoicingModeResultEvents_OnAllNotesOff() override
    {
        //
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct HarmonizerMusicalDevice
{
    AppSettings &mAppSettings;

    // harmonized notes don't carry their own params; caller must provide.
    ISynthParamProvider *mpParamProvider = nullptr;
    // need a way to know which harmonizer preset we're using.

    IMusicalDeviceEvents *mpEventHandler;
    void *mpCapture = nullptr;

    HarmonizerDeviceLayer mLayers[HARM_VOICES];
    Harmonizer mHarmonizer;

    HarmonizerMusicalDevice(AppSettings &appSettings,
                            ISynthParamProvider *pParamProvider,
                            IMusicalDeviceEvents *pEventHandler,
                            void *pCapture)
        : mAppSettings(appSettings),       //
          mpParamProvider(pParamProvider), //
          mpEventHandler(pEventHandler),   //
          mpCapture(pCapture),             //
          mHarmonizer(&appSettings)
    {
        for (size_t i = 0; i < SizeofStaticArray(mLayers); ++i)
        {
            mLayers[i].Init(i);
        }
    }

    void OnSourceNoteOn(const HeldNoteInfo &noteInfo, const HarmPreset &harmPatch)
    {
        // adds to the playing notes, by harmonizing the input
    }

    void OnSourceNoteOff(const HeldNoteInfo &noteInfo, const HeldNoteInfo *trillNote, const HarmPreset &harmPatch)
    {
        for (auto &layer : mLayers)
        {
            layer.OnNoteOff(noteInfo, trillNote, harmPatch);
        }
    }

    void OnSourceAllNotesOff(const HarmPreset &harmPatch)
    {
        for (auto &layer : mLayers)
        {
            layer.OnAllNotesOff(harmPatch);
        }
    }

    const MusicalVoice GetLiveMusicalVoice(const MusicalVoice &existing) const
    {
        // TODO
        return existing;
    }

};

} // namespace clarinoid
