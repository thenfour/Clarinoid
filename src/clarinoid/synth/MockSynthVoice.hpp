
#pragma once

//#include <clarinoid/basic/Basic.hpp>
//#include <clarinoid/application/MusicalState.hpp>

//namespace clarinoid
//{
//
//struct MockPeakNode
//{
//    bool available() const
//    {
//        return true;
//    }
//    float readPeakToPeak() const
//    {
//        return 0.0f;
//    }
//};
//namespace CCSynthGraph
//{
//MockPeakNode peakL;
//MockPeakNode peakR;
//} // namespace CCSynthGraph
//
//struct Voice
//{
//    // int16_t mMusicalVoiceId = MAGIC_VOICE_ID_UNASSIGNED;
//    bool mTouched = false;
//
//    MusicalVoice mRunningVoice;
//
//    void EnsurePatchConnections()
//    {
//    }
//
//    void Update(const MusicalVoice &mv)
//    {
//        mRunningVoice = mv;
//    }
//
//    bool IsPlaying() const
//    {
//        return mRunningVoice.IsPlaying();
//    }
//    void Unassign()
//    {
//        mRunningVoice.mVoiceId = MAGIC_VOICE_ID_UNASSIGNED;
//    }
//};
//
//std::array<Voice, MAX_SYNTH_VOICES> gVoices = initialize_array_with_indices<Voice, MAX_SYNTH_VOICES>();
//
//struct SynthGraphControl
//{
//    CCThrottlerT<500> mMetronomeTimer;
//
//    void Setup(...)
//    {
//    }
//
//    void SetGain(float f)
//    {
//    }
//
//    void BeginUpdate()
//    {
//    }
//
//    void EndUpdate()
//    {
//    }
//
//    void UpdatePostFx()
//    {
//    }
//};
//
//SynthGraphControl gSynthGraphControl;
//
//} // namespace clarinoid
