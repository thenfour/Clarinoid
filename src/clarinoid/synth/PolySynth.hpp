
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{
static constexpr float pitchBendRange = 2.0f;
} // namespace clarinoid

#include "PolySynthVoice.hpp"

namespace clarinoid
{
static float gPeak = 0;


struct PolySynth : IIncomingMusicalEvents
{
    size_t mCurrentPolyphony = 0;
    AppSettings *mAppSettings;
    ISynthParamProvider* mParamProvider;

    void Init(AppSettings *appSettings, Metronome *metronome, ISynthParamProvider* paramProvider)
    {
        mAppSettings = appSettings;
        mParamProvider = paramProvider;
        gSynthGraphControl.Setup(appSettings, metronome, paramProvider);
    }

    virtual void IncomingMusicalEvents_OnNoteOn(MusicalEventSource source, const HeldNoteInfo& noteInfo, uint16_t synthPatchIndex, float extraGain, float extraPan) override
    {
        Serial.println(String("synth note on: ") + noteInfo.mMidiNote.GetNoteDesc().mNameWithCustomGlyphs);
    }
    virtual void IncomingMusicalEvents_OnNoteOff(MusicalEventSource source, MidiNote note)  override
    {
        Serial.println(String("synth note off: ") + note.GetNoteDesc().mNameWithCustomGlyphs);
    }
    virtual void IncomingMusicalEvents_OnAllNoteOff()  override
    {
        Serial.println(String("synth all notes off"));
    }


    // // returns a voice that's either already assigned to this voice, or the best one to free up for it.
    // Voice *FindAssignedOrAvailable(const MusicalVoice& mv)
    // {
    //     //static int i = 0;

    //     // decltype(mv.mReleaseTimestampMS) oldestReleaseTimeMS;
    //     // Voice* oldestReleaseVoice = nullptr;
    //     for (auto &v : gVoices)
    //     {
    //         if (v.mRunningVoice.IsSameSynthContext(mv))
    //         {
    //             return &v; // already assigned to this voice.
    //         }
    //     }
    //     //i = (i + 1) % SizeofStaticArray(gVoices);
    //     //return &gVoices[i];
    //     // // TODO. find the oldest non-playing voice.
    //     return &gVoices[0];
    // }

    // After musical state has been updated, call this to apply those changes to the synth state.
    void Update(USBMidiMusicalState &ms)
    {
        mCurrentPolyphony = 0;

        if (gpSynthGraph->peakL.available() && gpSynthGraph->peakR.available())
        {
            gPeak = std::max(gpSynthGraph->peakL.readPeakToPeak(), gpSynthGraph->peakR.readPeakToPeak());
        }
        else if (gpSynthGraph->peakL.available())
        {
            gPeak = std::max(gpSynthGraph->peakL.readPeakToPeak(), gPeak);
        }
        else if (gpSynthGraph->peakR.available())
        {
            gPeak = std::max(gpSynthGraph->peakR.readPeakToPeak(), gPeak);
        }

        // // there are potentially 127 notes playing in the incoming musical state.
        // // sort in priority, and take them one by one, feeding into our voices.
        // for (auto &v : gVoices)
        // {
        //     v.mTouched = false;
        // }

        // // todo: synth patch B.

        // //for (const MusicalVoice *pvoice = pVoicesBegin; pvoice != pVoicesEnd; ++pvoice)
        // for (auto it = ms.mHeldNotes.mHeldNotes.begin(); it != ms.mHeldNotes.mHeldNotes.end(); ++ it)
        // {
        //     const auto &mv = *it;//*pvoice;
        //     Voice *pv = FindAssignedOrAvailable(mv);
        //     CCASSERT(!!pv);
        //     pv->Update(mv);
        //     pv->mTouched = true;
        // }

        // // any voice that wasn't assigned (touched) should be "released".
        // for (auto &v : gVoices)
        // {
        //     if (v.IsPlaying())
        //         mCurrentPolyphony++; // also count polyphony here
        //     if (v.mTouched)
        //         continue;
        //     v.Release();
        // }

        gSynthGraphControl.UpdatePostFx();
    }

    static float GetPeakLevel()
    {
        return gPeak;
    }
};

template <uint32_t holdTimeMS, uint32_t falloffTimeMS>
struct PeakMeterUtility
{
    GenericPeakMeterUtility<holdTimeMS, falloffTimeMS> mUtil;

    void Update(float &peak, float &heldPeak)
    {
        peak = PolySynth::GetPeakLevel();
        heldPeak = mUtil.Update(peak);
    }
};

} // namespace clarinoid
