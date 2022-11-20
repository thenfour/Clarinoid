

#pragma once

#include "GuiApp.hpp"

namespace clarinoid
{
    float gThrowawayFloatValue = 0; // use when you need a reference

// ---------------------------------------------------------------------------------------
struct GuiPerformanceApp : GuiApp
{
    Metronome &mMetronome;
    UnipolarMapping *mBreathCalibration = nullptr;

    virtual const char *DisplayAppGetName() override
    {
        return "GuiPerformanceApp";
    }

    GuiPerformanceApp(IDisplay &display, Metronome &m /* PROMISE ME YOU WILL SET mBreathCalibration */, AppSettings& appSettings, InputDelegator& input)
        : GuiApp(display, appSettings, input), mMetronome(m)
    {
    }

    GuiPatchMuteControl<false> mSynthAEnable = {
        0,
        PointI::Construct(0, 5),
        "Synth A",
        "Enabled",
        "Muted",
        {[](void *cap) -> bool & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mSynthAEnabled.mValue;
         },
         this},
        NeverEnabled};

    GuiPatchMuteControl<false> mSynthBEnable = {
        0,
        PointI::Construct(0, 21),
        "Synth B",
        "Enabled",
        "Muted",
        {[](void *cap) -> bool & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mSynthBEnabled.mValue;
         },
         this},
        NeverEnabled};

    GuiPatchMuteControl<false> mHarmonizerEnable = {
        0,
        PointI::Construct(0, 37),
        "Harmonizer",
        "Enabled",
        "Muted",
        {[](void *cap) -> bool & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mHarmEnabled.mValue;
         },
         this},
        NeverEnabled};

    GuiKnobGainControl mSynthAGain = {
        0,
        PointI::Construct(8, 1),
        StandardRangeSpecs::gGeneralGain,
        "Synth A Gain",
        {[](void *cap) -> float & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mSynthAGain.mValue;
         },
         this},
        {[](void *cap) -> bool & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mSynthAEnabled.mValue;
         },
         this},
        AlwaysEnabled};

    GuiKnobGainControl mSynthBGain = {
        0,
        PointI::Construct(8, 17),
        StandardRangeSpecs::gGeneralGain,
        "Synth B Gain",
        {[](void *cap) -> float & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mSynthBGain.mValue;
         },
         this},
        {[](void *cap) -> bool & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mSynthBEnabled.mValue;
         },
         this},
        AlwaysEnabled};

    GuiKnobGainControl mHarmGain = {
        0,
        PointI::Construct(8, 32),
        StandardRangeSpecs::gGeneralGain,
        "Harm Gain",
        {[](void *cap) -> float & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mHarmGain.mValue;
         },
         this},
        {[](void *cap) -> bool & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mHarmEnabled.mValue;
         },
         this},
        AlwaysEnabled};

    GuiSynthPatchSelectControl<int16_t> mSynthPatchA = {
        0,
        false, // nullable
        RectI::Construct(24, 5, 45, 9),
        "Synth Patch A", //
        {[](void *cap) -> int16_t & {
             auto *pThis = (GuiPerformanceApp *)cap;
             return pThis->mAppSettings->GetCurrentPerformancePatch().mSynthPatchA.mValue;
         },
         this},
        AlwaysEnabled};

    GuiSynthPatchSelectControl<int16_t> mSynthPatchB = {
        0,
        true, // nullable
        RectI::Construct(24, 20, 45, 9),
        "Synth Patch B", //
        {[](void *cap) -> int16_t & {
             auto *pThis = (GuiPerformanceApp *)cap;
             return pThis->mAppSettings->GetCurrentPerformancePatch().mSynthPatchB.mValue;
         },
         this},
        AlwaysEnabled};

    GuiHarmPatchSelectControl<int16_t> mHarmPatch = {
        0,
        RectI::Construct(24, 35, 45, 9),
        "Harm Patch",
        {[](void *cap) -> int16_t & {
             auto *pThis = (GuiPerformanceApp *)cap;
             return pThis->mAppSettings->GetCurrentPerformancePatch().mHarmPreset.mValue;
         },
         this},
        AlwaysEnabled};

    GuiLabelControl mLabelMst = {0, PointI::Construct(70, 4), "Mst"};

    GuiKnobGainControl mGlobalGain = {
        0,
        PointI::Construct(90, 0),
        StandardRangeSpecs::gMasterGainDb,
        "Master gain",
        {[](void *cap) -> float & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mMasterGain.mValue;
         },
         this},
        NullBoolBinding,
        AlwaysEnabled};

    GuiStereoSpreadControl mStereoSpread = {
        0,                               // page
        PointI::Construct(107, 4),       // pos
        StandardRangeSpecs::gFloat_N1_1, // range
        "Stereo spread",                 // tooltip
        {[](void *cap) -> float & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mSynthStereoSpread.mValue;
         },
         this},
        AlwaysEnabled // always en
    };

    GuiBreathMinControl mBreathMin = {
        0,                              // page
        PointI::Construct(120, 7),      // pos
        StandardRangeSpecs::gBreathMin, // range
        "Breath min",                   // tooltip
        {[](void *cap) -> float & {
            auto* bc = ((GuiPerformanceApp *)cap)->mBreathCalibration;
            if(!bc) {
                return gThrowawayFloatValue;
            }
            return ((GuiPerformanceApp *)cap)->mBreathCalibration->mSrcMin;
         },
         this},
        AlwaysEnabled // always en
    };

    GuiLabelControl mLabelFx = {0, PointI::Construct(70, 18), "Fx"};

    GuiKnobGainControl mFXGain = {
        0,
        PointI::Construct(90, 14),
        StandardRangeSpecs::gGeneralGain,
        "Master gain",
        {[](void *cap) -> float & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mMasterFXGain.mValue;
         },
         this},
        {[](void *cap) -> bool & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mMasterFXEnable.mValue;
         },
         this},
        AlwaysEnabled};

    GuiMuteControl<true> mFXEnable = {
        0,
        PointI::Construct(107, 17),
        "FX",
        "Enabled",
        "Disabled", //
        {[](void *cap) -> bool & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mMasterFXEnable.mValue;
         },
         this},
        NeverEnabled};

    GuiTempoControl mTempo = {0,
                              RectI::Construct(70, 29, 26, 10),
                              StandardRangeSpecs::gBPMRange,
                              "Master BPM",
                              {[](void *cap) -> float {
                                   auto *pThis = (GuiPerformanceApp *)cap;
                                   return pThis->mAppSettings->GetCurrentPerformancePatch().mBPM.GetValue();
                               },
                               [](void *cap, const float &val) -> void {
                                   auto *pThis = (GuiPerformanceApp *)cap;
                                   pThis->mAppSettings->GetCurrentPerformancePatch().mBPM.SetValue(val);
                                   pThis->mMetronome.OnBPMChanged();
                               },
                               this},
                              {[](void *cap) -> bool & { // on toggle handler
                                   auto *pThis = (GuiPerformanceApp *)cap;
                                   return pThis->mAppSettings->mMetronome.mSoundOn.mValue;
                               },
                               this},
                              AlwaysEnabled};

    GuiTransposeControl<int8_t> mTranspose = {
        0,
        RectI::Construct(98, 29, 24, 10),
        StandardRangeSpecs::gTransposeRange.Cast<int8_t>(),
        "Transpose",
        {[](void *cap) -> int8_t & {
             auto *pThis = (GuiPerformanceApp *)cap;
             return pThis->mAppSettings->GetCurrentPerformancePatch().mTranspose.mValue;
         },
         this},
        {[](void *cap) -> bool { return true; }, // double-click getter (not used)
         [](void *cap, const bool &val) {        // double-click setter (set transpose to 0)
             auto *pThis = (GuiPerformanceApp *)cap;
             pThis->mAppSettings->GetCurrentPerformancePatch().mTranspose.SetValue(0);
         },
         this},
        AlwaysEnabled};

    GuiLabelControl mGlobalScale = {0, PointI::Construct(70, 39), "F# hw dim"};

    // verb
    GuiLabelControl mReverbCaption = {1, PointI::Construct(0, 0), "Reverb Fx"};

    // rectangle marquee
    GuiRectangleMarquee mReverbMarquee = {1, RectI::Construct(0, 0, 68, 12), Edges::Right | Edges::Bottom};

    // size
    GuiKnobControl mVerbSize = {
        1,
        PointI::Construct(19, 16),
        StandardRangeSpecs::gFloat_0_1,
        "Size",
        {[](void *cap) -> float & { //
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mReverb.mSize.mValue;
         },
         this},
        NullBoolBinding,
        AlwaysEnabled};

    GuiLabelControl mReverbSizeLabel = {1, PointI::Construct(37, 20), "Size"};

    // damp
    GuiKnobControl mVerbDamp = {
        1,
        PointI::Construct(19, 33),
        StandardRangeSpecs::gFloat_0_1,
        "Damp",
        {[](void *cap) -> float & { //
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mReverb.mDamping.mValue;
         },
         this},
        NullBoolBinding,
        AlwaysEnabled};
    GuiLabelControl mReverbDampLabel = {1, PointI::Construct(37, 37), "Damp"};

    // gain
    GuiKnobGainControl mVerbGain = {
        1,
        PointI::Construct(72, 16),
        StandardRangeSpecs::gGeneralGain,
        "Gain",
        {[](void *cap) -> float & { //
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mReverb.mGain.mValue;
         },
         this},
        {[](void *cap) -> bool & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mReverb.mEnabled.mValue;
         },
         this},
        AlwaysEnabled};

    GuiLabelControl mReverbGainLabel = {1, PointI::Construct(90, 20), "Gain"};

    GuiMuteControl<true> mVerbEnable = {
        1,
        PointI::Construct(73, 33),
        "",
        "",
        "", //
        {[](void *cap) -> bool & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mReverb.mEnabled.mValue;
         },
         this},
        NeverEnabled};

    GuiLabelControl mDelayCaption = {2, PointI::Construct(0, 0), "Delay Fx"};
    GuiRectangleMarquee mDelayMarquee = {2, RectI::Construct(0, 0, 62, 12), Edges::Right | Edges::Bottom};

    GuiLabelControl mDelayGainLabel = {2, PointI::Construct(6, 13), "Gain"};

    GuiKnobGainControl mDelayGain = {
        2,
        PointI::Construct(10, 23),
        StandardRangeSpecs::gGeneralGain,
        "Gain",
        {[](void *cap) -> float & { //
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mDelay.mGain.mValue;
         },
         this},
        {[](void *cap) -> bool & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mDelay.mEnabled.mValue;
         },
         this},
        AlwaysEnabled};

    GuiMuteControl<true> mDelayEnable = {
        2,
        PointI::Construct(10, 39),
        "",
        "",
        "", //
        {[](void *cap) -> bool & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mDelay.mEnabled.mValue;
         },
         this},
        NeverEnabled};

    GuiLabelControl mDelayTimeLabel = {2, PointI::Construct(34, 13), "Time"};

    GuiKnobControl mDelayTime = {
        2,
        PointI::Construct(36, 23),
        NumericEditRangeSpec<float>(0, MAX_DELAY_MS),
        "Time(MS)",
        {[](void *cap) -> float & { //
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mDelay.mTime.mParamValue.mValue;
         },
         this},
        NullBoolBinding,
        AlwaysEnabled};

    GuiStereoSpreadControl mDelayStereoSpread = {
        2,                              // page
        PointI::Construct(35, 39),      // pos
        StandardRangeSpecs::gFloat_0_1, // range
        "Stereo spread",                // tooltip
        {[](void *cap) -> float {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mDelay.mStereoSeparationDelayMS.mValue /
                    StandardRangeSpecs::gDelayStereoSpread.mRangeMax;
         },
         [](void *cap, const float &val) {
             ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mDelay.mStereoSeparationDelayMS.mValue =
                 val * StandardRangeSpecs::gDelayStereoSpread.mRangeMax;
         },
         this},
        AlwaysEnabled //
    };

    GuiLabelControl mDelayFeedbackLabel = {2, PointI::Construct(69, 0), "Feedback"};

    GuiKnobGainControl mDelayFeedbackLevel = {
        2,
        PointI::Construct(72, 10),
        StandardRangeSpecs::gGeneralGain,
        "Feedback level",
        {[](void *cap) -> float & { //
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mDelay.mFeedbackGain.mValue;
         },
         this},
        NullBoolBinding,
        AlwaysEnabled};

    GuiLabelControl mDelayHzLabel = {2, PointI::Construct(71, 25), "Hz"};

    GuiKnobControl mDelayFeedbackHz = {
        2,
        PointI::Construct(70, 33),
        StandardRangeSpecs::gFloat_0_1,
        "Cutoff Hz",
        {[](void *cap) -> float & { //
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mDelay.mFilter.mFrequency.mValue.mValue;
         },
         this},
        NullBoolBinding,
        AlwaysEnabled};

    GuiLabelControl mDelaySatLabel = {2, PointI::Construct(89, 25), "Sat"};

    GuiKnobControl mDelaySat = {
        2,
        PointI::Construct(89, 33),
        StandardRangeSpecs::gFloat_0_1,
        "Saturation",
        {[](void *cap) -> float & { //
            auto& patch = ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch();
            //Serial.println(String("freq hz: ") + patch.mDelay.mFilter.mFrequency.GetFrequency(64, 0));
             return patch.mDelay.mFilter.mSaturation.mValue;
         },
         this},
        NullBoolBinding,
        AlwaysEnabled};

    GuiLabelControl mDelayQLabel = {2, PointI::Construct(113, 25), "Q"};

    GuiKnobControl mDelayQ = {
        2,
        PointI::Construct(108, 33),
        StandardRangeSpecs::gFloat_0_1,
        "Resonance",
        {[](void *cap) -> float & { //
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mDelay.mFilter.mQ.mValue;
         },
         this},
        NullBoolBinding,
        AlwaysEnabled};

    IGuiControl *mArray[44] = {
        // PAGE 1
        &mSynthAEnable,
        &mSynthBEnable,

        &mSynthAGain,
        &mSynthBGain,

        &mSynthPatchA,
        &mSynthPatchB,

        &mHarmonizerEnable,
        &mHarmGain,
        &mHarmPatch,

        &mLabelMst,
        &mGlobalGain,
        &mStereoSpread,

        &mBreathMin,

        &mLabelFx,
        &mFXGain,
        &mFXEnable,

        &mTempo,
        &mTranspose,
        &mGlobalScale,

        // PAGE 2
        &mReverbCaption,
        &mReverbMarquee,
        &mVerbSize,
        &mReverbSizeLabel,
        &mVerbDamp,
        &mReverbDampLabel,
        &mVerbGain,
        &mReverbGainLabel,
        &mVerbEnable,

        // PAGE 3
        &mDelayCaption,
        &mDelayMarquee,
        &mDelayGainLabel,
        &mDelayGain,
        &mDelayEnable,
        &mDelayTimeLabel,
        &mDelayTime,
        &mDelayStereoSpread,
        &mDelayFeedbackLabel,
        &mDelayFeedbackLevel,
        &mDelayHzLabel,
        &mDelayFeedbackHz,
        &mDelaySatLabel,
        &mDelaySat,
        &mDelayQLabel,
        &mDelayQ,
    };

    GuiControlList mList = {mArray};

    virtual GuiControlList *GetRootControlList() override
    {
        return &mList;
    }
};

} // namespace clarinoid
