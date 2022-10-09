
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
    size_t GetCurrentPolyphony_ForDisplay() const {
        size_t ret = 0;
        for (const auto& v : gVoices) {
            ret += v.IsConsideredPlaying_ForDisplay() ? 1 : 0;
        }
        return ret;
    }
    AppSettings *mAppSettings;
    ISynthParamProvider *mParamProvider;

    void Init(AppSettings *appSettings, Metronome *metronome, ISynthParamProvider *paramProvider)
    {
        mAppSettings = appSettings;
        mParamProvider = paramProvider;
        gSynthGraphControl.Setup(appSettings, metronome, paramProvider);
    }

    // always returns something.
    Voice *FindBestFreeVoice()
    {
        // find the voice with best releaseability.
        Voice *bestVoice = nullptr;
        //size_t bestVoiceIndex = 99;
        float bestReleaseabilityScore = 0;
        for (size_t i = 0; i < SizeofStaticArray(gVoices); ++i)
        {
            auto &v = gVoices[i];
            float r = v.GetReleaseability();
            if (r > bestReleaseabilityScore)
            {
                //bestVoiceIndex = i;
                bestVoice = &v;
                bestReleaseabilityScore = r;
            }
        }
                //Serial.println(String("best voice found: ") + bestVoiceIndex + "; rel:" + bestReleaseabilityScore);
        return bestVoice;
    }

    virtual void IncomingMusicalEvents_OnNoteOn(MusicalEventSource source,
                                                const HeldNoteInfo &noteInfo,
                                                uint16_t synthPatchIndex) override
    {
        //Serial.println(String("synth note on: ") + noteInfo.mMidiNote.GetNoteDesc().mName);

        auto &patch = mAppSettings->FindSynthPreset(synthPatchIndex);

        // monophonic wants to reuse the same voice for everything.
        // so find a voice already playing from this source
        if (patch.mVoicingMode == VoicingMode::Monophonic)
        {
            for (size_t i = 0; i < SizeofStaticArray(gVoices); ++i)
            {
                auto &v = gVoices[i];
                if (v.mRunningVoice.mSource.Equals(source))
                {
                    v.IncomingMusicalEvents_OnNoteOn(source, noteInfo, synthPatchIndex);
                    return;
                }
            }
            // if no existing voice is found, then just act like polyphonic for this 1 note. fall through.
        }

        auto bestVoice = FindBestFreeVoice();
        bestVoice->IncomingMusicalEvents_OnNoteOn(source, noteInfo, synthPatchIndex);
        return;
    }
    virtual void IncomingMusicalEvents_OnNoteOff(MusicalEventSource source, const HeldNoteInfo &noteInfo) override
    {
        //Serial.println(String("synth note off: ") + noteInfo.mMidiNote.GetNoteDesc().mName);
        // find this note
        for (auto& v : gVoices) {
            if (v.mRunningVoice.mNoteInfo.mLiveNoteSequenceID == noteInfo.mLiveNoteSequenceID) {
                v.IncomingMusicalEvents_OnNoteOff();
                return;
            }
        }
        //Serial.println("Note off sent to synth, but i didn't find any voices playing that note. i guess we culled it due to max voice polyphony.");
    }
    virtual void IncomingMusicalEvents_OnAllNoteOff() override
    {
        //Serial.println(String("synth all notes off"));
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

        // any voice that wasn't assigned (touched) should be "released".
        for (auto &v : gVoices)
        {
            v.Update(ms);
        }

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
