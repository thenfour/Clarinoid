
#pragma once

#include <clarinoid/basic/Basic.hpp>

#include "SynthGraph.hpp"
#include "PolySynthVoice.hpp"

namespace clarinoid
{
static float gPeak = 0;

struct PolySynth : IMusicalEventsForSynth
{
    size_t GetCurrentPolyphony_ForDisplay() const
    {
        size_t ret = 0;
        for (const auto &v : gVoices)
        {
            ret += v.IsConsideredPlaying_ForDisplay() ? 1 : 0;
        }
        return ret;
    }
    AppSettings *mAppSettings = nullptr;
    MusicalState *mpMusicalState = nullptr;

    void Init(AppSettings *appSettings, Metronome *metronome, MusicalState *pMusicalState)
    {
        mAppSettings = appSettings;
        mpMusicalState = pMusicalState;
        gSynthGraphControl.Setup(appSettings, metronome);

        // for some reason patches really don't like to connect unless they are
        // last in the initialization order. Here's a workaround to force them to
        // connect.
        for (auto &v : gVoices)
        {
            v.EnsurePatchConnections(appSettings, pMusicalState);
        }
    }

    // always returns something.
    Voice *FindBestFreeVoice()
    {
        // find the voice with best releaseability.
        Voice *bestVoice = nullptr;
        // size_t bestVoiceIndex = 99;
        float bestReleaseabilityScore = 0;
        for (size_t i = 0; i < SizeofStaticArray(gVoices); ++i)
        {
            auto &v = gVoices[i];
            float r = v.GetReleaseability();
            if (r > bestReleaseabilityScore)
            {
                // bestVoiceIndex = i;
                bestVoice = &v;
                bestReleaseabilityScore = r;
            }
        }
        // Serial.println(String("best voice found: ") + bestVoiceIndex + "; rel:" + bestReleaseabilityScore);
        return bestVoice;
    }

    virtual void IMusicalEventsForSynth_OnNoteOn(const MusicalVoice &mv) override
    {
        // if it's not active, why is this being called? handle that outside of the synth please.
        CCASSERT(mv.mIsActive);

        // Serial.println(String("synth note on: ") + mv.ToString());

        // monophonic wants to reuse the same voice for everything.
        // so find a voice already playing from this source
        if (mv.mpSynthPatch->mVoicingMode == VoicingMode::Monophonic)
        {
            for (size_t i = 0; i < SizeofStaticArray(gVoices); ++i)
            {
                auto &v = gVoices[i];
                if (v.mRunningVoice.mSource.Equals(mv.mSource))
                {
                    v.IncomingMusicalEvents_OnNoteOn(mv);
                    return;
                }
            }
            // if no existing voice is found, then just act like polyphonic for this 1 note. fall through.
        }

        auto bestVoice = FindBestFreeVoice();
        bestVoice->IncomingMusicalEvents_OnNoteOn(mv);
        return;
    }

    // you're effectively saying to note off for the given
    // mLiveNoteSequenceID.
    // in the case of harmonizer
    virtual void IMusicalEventsForSynth_OnNoteOff(const MusicalVoice &mv) override
    {
        // Serial.println(String("synth note off: ") + mv.mSource.ToString() + ", note:" + mv.mNoteInfo.ToString());
        //   find this note
        switch (mv.mSource.mType)
        {
            default:
            CCASSERT(!"unexpected source types");
        case MusicalEventSourceType::LivePlayA:
        case MusicalEventSourceType::LivePlayB:
        case MusicalEventSourceType::Loopstation:
            for (auto &v : gVoices)
            {
                if (mv.mSource.Equals(v.mRunningVoice.mSource) &&
                    (v.mRunningVoice.mNoteInfo.mLiveNoteSequenceID == mv.mNoteInfo.mLiveNoteSequenceID))
                {
                    v.IncomingMusicalEvents_OnNoteOff();
                    // Serial.println(" -> success");
                    return;
                }
            }
            break;
        case MusicalEventSourceType::Harmonizer:
        case MusicalEventSourceType::LoopstationHarmonizer:
            for (auto &v : gVoices)
            {
                // harmonizer noteoffs will not know the actual noteid. they'll specify the source note id instead.
                if (mv.mSource.Equals(v.mRunningVoice.mSource) &&
                    (v.mRunningVoice.mHarmonizerSourceNoteID == mv.mHarmonizerSourceNoteID))
                {
                    v.IncomingMusicalEvents_OnNoteOff();
                    //Serial.println(" -> success");
                    return;
                }
            }
            break;
        }
        // Serial.println(" -> Note off sent to synth, but i didn't find any voices playing that note. i guess we culled
        // it due to max voice polyphony."); Serial.println("{"); for (auto &v : gVoices)
        // {
        //     Serial.println(String("  ") + v.mRunningVoice.mSource.ToString() + ", note:" +
        //     v.mRunningVoice.mNoteInfo.ToString());
        // }
        // Serial.println("}");
    }
    virtual void IMusicalEventsForSynth_OnAllNoteOff(MusicalEventSource src) override
    {
        for (auto &v : gVoices)
        {
            if (src.Equals(v.mRunningVoice.mSource))
            {
                v.IncomingMusicalEvents_OnNoteKill();
            }
        }
    }

    // After musical state has been updated, call this to apply those changes to the synth state.
    void Update(MusicalState &ms)
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

        for (auto &v : gVoices)
        {
            v.Update();
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
