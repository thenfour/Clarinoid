
#pragma once

// todo.

// #include <clarinoid/basic/Basic.hpp>
// // #include <clarinoid/application/MusicalState.hpp>

// namespace clarinoid
// {

// // struct IAnalysisStream
// // {
// //     virtual ~IAnalysisStream() = default;
// //     virtual void Reset() = 0;

// //     float CurrentRMS() const
// //     {
// //         return mCurrentRMSValue;
// //     }

// //     bool ClipIndicator() const
// //     {
// //         return mClipIndicator;
// //     }

// //     float CurrentPeak() const
// //     {
// //         return mCurrentPeak;
// //     }

// //     float CurrentHeldPeak() const
// //     {
// //         return mCurrentHeldPeak;
// //     }

// //   protected:
// //     float mCurrentRMSValue = 0.0f;
// //     bool mClipIndicator = false;
// //     float mCurrentPeak = 0.0f;
// //     float mCurrentHeldPeak = 0.0f;
// // };

// // struct MockAnalysisStream : IAnalysisStream
// // {
// //     virtual void Reset() override
// //     {
// //         mCurrentRMSValue = 0.0f;
// //         mClipIndicator = false;
// //         mCurrentPeak = 0.0f;
// //         mCurrentHeldPeak = 0.0f;
// //     }

// //     void SetValues(float peak, float heldPeak, float rms, bool clip)
// //     {
// //         mCurrentPeak = peak;
// //         mCurrentHeldPeak = heldPeak;
// //         mCurrentRMSValue = rms;
// //         mClipIndicator = clip;
// //     }
// // };

// // struct SynthGraph
// // {
// //     MockAnalysisStream analysis;
// // };

// SynthGraph gMockSynthGraphInstance;
// SynthGraph *gpSynthGraph = &gMockSynthGraphInstance;

// struct Voice
// {
//     // int16_t mMusicalVoiceId = MAGIC_VOICE_ID_UNASSIGNED;
//     bool mTouched = false;

//     MusicalVoice mRunningVoice;

//     void EnsurePatchConnections()
//     {
//     }

//     void Update(const MusicalVoice &mv)
//     {
//         mRunningVoice = mv;
//     }

//     bool IsPlaying() const
//     {
//         return mRunningVoice.IsPlaying();
//     }
//     void Unassign()
//     {
//         mRunningVoice.mVoiceId = MAGIC_VOICE_ID_UNASSIGNED;
//     }
// };

// Voice gVoices[MAX_SYNTH_VOICES];

// struct SynthGraphControl
// {
//     CCThrottlerT<500> mMetronomeTimer;

//     void Setup(...)
//     {
//     }

//     void SetGain(float f)
//     {
//     }

//     void BeginUpdate()
//     {
//     }

//     void EndUpdate()
//     {
//     }

//     void UpdatePostFx()
//     {
//     }
// };

// SynthGraphControl gSynthGraphControl;

// } // namespace clarinoid
