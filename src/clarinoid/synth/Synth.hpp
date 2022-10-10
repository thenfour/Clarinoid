
// #pragma once

// #include <clarinoid/basic/Basic.hpp>

// namespace clarinoid
// {
// static constexpr float pitchBendRange = 2.0f;
// } // namespace clarinoid

// #ifdef CLARINOID_MODULE_TEST
// #include "MockSynthVoice.hpp"
// #else
// #include "SynthVoice.hpp"
// #endif

// namespace clarinoid
// {
// static float gPeak = 0;

// struct CCSynth
// {
//     size_t mCurrentPolyphony = 0;
//     AppSettings *mAppSettings;
//     Metronome *mMetronome;

//     MusicalVoice mUnassignedVoice;

//     void Init(AppSettings *appSettings, Metronome *metronome /*, IModulationSourceSource *modulationSourceSource*/)
//     {
//         mAppSettings = appSettings;
//         mMetronome = metronome;
//         gSynthGraphControl.Setup(appSettings, metronome /*, modulationSourceSource*/);
//         mUnassignedVoice.mVoiceId = MAGIC_VOICE_ID_UNASSIGNED;
//     }

//     // returns a voice that's either already assigned to this voice, or the best one to free up for it.
//     Voice *FindAssignedOrAvailable(int16_t musicalVoiceId)
//     {
//         Voice *firstFree = nullptr;
//         for (auto &v : gVoices)
//         {
//             if (v.mRunningVoice.mVoiceId == musicalVoiceId)
//             {
//                 return &v; // already assigned to this voice.
//             }
//             if (!firstFree && (v.mRunningVoice.mVoiceId == MAGIC_VOICE_ID_UNASSIGNED))
//             {
//                 firstFree = &v;
//             }
//         }
//         if (firstFree)
//         {
//             return firstFree;
//         }
//         // no free voices. in this case find the oldest.
//         // TODO.
//         return &gVoices[0];
//     }

//     // After musical state has been updated, call this to apply those changes to the synth state.
//     void Update(const MusicalVoice *pVoicesBegin, const MusicalVoice *pVoicesEnd)
//     {
//         mCurrentPolyphony = 0;

//         if (gpSynthGraph->peakL.available() && gpSynthGraph->peakR.available())
//         {
//             gPeak = std::max(gpSynthGraph->peakL.readPeakToPeak(), gpSynthGraph->peakR.readPeakToPeak());
//         }
//         else if (gpSynthGraph->peakL.available())
//         {
//             gPeak = std::max(gpSynthGraph->peakL.readPeakToPeak(), gPeak);
//         }
//         else if (gpSynthGraph->peakR.available())
//         {
//             gPeak = std::max(gpSynthGraph->peakR.readPeakToPeak(), gPeak);
//         }

//         for (auto &v : gVoices)
//         {
//             v.mTouched = false;
//         }

//         for (const MusicalVoice *pvoice = pVoicesBegin; pvoice != pVoicesEnd; ++pvoice)
//         {
//             auto &mv = *pvoice;
//             Voice *pv = FindAssignedOrAvailable(mv.mVoiceId);
//             CCASSERT(!!pv);
//             pv->Update(mv);
//             pv->mTouched = true;
//         }

//         // any voice that wasn't assigned (touched) should be "released".
//         for (auto &v : gVoices)
//         {
//             if (v.IsPlaying())
//                 mCurrentPolyphony++;
//             if (v.mTouched)
//                 continue;
//             v.Update(mUnassignedVoice);
//         }

//         for (auto &v : gVoices)
//         {
//             if (!v.IsPlaying())
//             {
//                 v.Unassign();
//             }
//         }

//         gSynthGraphControl.UpdatePostFx();
//     }

//     static float GetPeakLevel()
//     {
//         return gPeak;
//     }
// };


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

// } // namespace clarinoid
