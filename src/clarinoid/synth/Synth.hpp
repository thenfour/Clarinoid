
#pragma once

#include <clarinoid/basic/Basic.hpp>

#ifdef CLARINOID_MODULE_TEST
// #include "MockSynthVoice.hpp"
#else
#include "SynthVoice.hpp"
#endif

namespace clarinoid
{
AudioAnalysisState gAnalysisStateL;
AudioAnalysisState gAnalysisStateC;
AudioAnalysisState gAnalysisStateR;

struct CCSynth
{
    size_t mCurrentPolyphony = 0;
    AppSettings *mAppSettings;
    Metronome *mMetronome;

    MusicalVoice mUnassignedVoice;

    void Init(AppSettings *appSettings, Metronome *metronome /*, IModulationSourceSource *modulationSourceSource*/)
    {
        mAppSettings = appSettings;
        mMetronome = metronome;
        gSynthGraphControl.Setup(appSettings, metronome /*, modulationSourceSource*/);

        mUnassignedVoice.mVoiceId = MAGIC_VOICE_ID_UNASSIGNED;
    }

    // returns a voice that's either already assigned to this voice, or the best one to free up for it.
    Voice *FindAssignedOrAvailable(int16_t musicalVoiceId)
    {
        Voice *firstFree = nullptr;
        for (auto &v : gVoices)
        {
            if (v.mRunningVoice.mVoiceId == musicalVoiceId)
            {
                return &v; // already assigned to this voice.
            }
            if (!firstFree && (v.mRunningVoice.mVoiceId == MAGIC_VOICE_ID_UNASSIGNED))
            {
                firstFree = &v;
            }
        }
        if (firstFree)
        {
            return firstFree;
        }
        // no free voices. in this case find the oldest.
        // TODO.
        return &gVoices[0];
    }

    // After musical state has been updated, call this to apply those changes to the synth state.
    void Update(const MusicalVoice *pVoicesBegin, const MusicalVoice *pVoicesEnd)
    {
        mCurrentPolyphony = 0;

        if (gpSynthGraph)
        {
            gAnalysisStateL = gpSynthGraph->analysisL.GetState();
            gAnalysisStateR = gpSynthGraph->analysisR.GetState();
            gAnalysisStateC = IAnalysisStream::Combine(gAnalysisStateL, gAnalysisStateR);
        }

        for (auto &v : gVoices)
        {
            v.mTouched = false;
        }

        for (const MusicalVoice *pvoice = pVoicesBegin; pvoice != pVoicesEnd; ++pvoice)
        {
            auto &mv = *pvoice;
            Voice *pv = FindAssignedOrAvailable(mv.mVoiceId);
            CCASSERT(!!pv);
            pv->Update(mv);
            pv->mTouched = true;
        }

        // any voice that wasn't assigned should be muted.
        for (auto &v : gVoices)
        {
            if (v.IsPlaying())
                mCurrentPolyphony++;
            if (v.mTouched)
                continue;
            v.Update(mUnassignedVoice);
        }

        for (auto &v : gVoices)
        {
            if (!v.IsPlaying())
            {
                v.Unassign();
            }
        }

        gSynthGraphControl.UpdatePostFx();
    }

    // static float GetPeakLevel()
    // {
    //     return gPeak;
    // }

    // static float GetHeldPeakLevel()
    // {
    //     if (!gpSynthGraph)
    //     {
    //         return 0.0f;
    //     }
    //     return gpSynthGraph->analysis.CurrentHeldPeak();
    // }

    // static float GetRMSLevel()
    // {
    //     if (!gpSynthGraph)
    //     {
    //         return 0.0f;
    //     }
    //     return gpSynthGraph->analysis.CurrentRMS();
    // }

    // static const IAnalysisStream *GetAnalysisStream()
    // {
    //     if (!gpSynthGraph)
    //     {
    //         return nullptr;
    //     }
    //     return &gpSynthGraph->analysis;
    // }
};

// template <uint32_t holdTimeMS, uint32_t falloffTimeMS>
// struct PeakMeterUtility
// {
//     GenericPeakMeterUtility<holdTimeMS, falloffTimeMS> mUtil;

//     void Update(float &peak, float &heldPeak)
//     {
//         peak = CCSynth::GetPeakLevel();
//         heldPeak = mUtil.Update(peak);
//     }
// };

} // namespace clarinoid
