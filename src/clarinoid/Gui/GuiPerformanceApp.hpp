

#pragma once

#include "GuiApp.hpp"

namespace clarinoid
{

// ---------------------------------------------------------------------------------------
struct GuiPerformanceApp : GuiApp
{
    Metronome &mMetronome;

    virtual const char *DisplayAppGetName() override
    {
        return "GuiPerformanceApp";
    }

    GuiPerformanceApp(IDisplay &display, Metronome &m) : GuiApp(display), mMetronome(m)
    {
    }

    GuiPatchMuteControl<false> mSynthAEnable = {
        0,
        PointI::Construct(0, 5),
        "Synth A",
        "Enabled",
        "Muted",
        {[](void *cap) -> bool & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mSynthAEnabled;
         },
         this},
        AlwaysEnabled};

    GuiPatchMuteControl<false> mSynthBEnable = {
        0,
        PointI::Construct(0, 21),
        "Synth B",
        "Enabled",
        "Muted",
        {[](void *cap) -> bool & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mSynthBEnabled;
         },
         this},
        AlwaysEnabled};

    GuiPatchMuteControl<false> mHarmonizerEnable = {
        0,
        PointI::Construct(0, 37),
        "Harmonizer",
        "Enabled",
        "Muted",
        {[](void *cap) -> bool & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mHarmEnabled;
         },
         this},
        AlwaysEnabled};

    GuiKnobGainControl mSynthAGain = {
        0,
        PointI::Construct(8, 1),
        StandardRangeSpecs::gGeneralGain,
        "Synth A Gain",
        {[](void *cap) -> float & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mSynthAGain;
         },
         this},
        AlwaysEnabled};

    GuiKnobGainControl mSynthBGain = {
        0,
        PointI::Construct(8, 17),
        StandardRangeSpecs::gGeneralGain,
        "Synth B Gain",
        {[](void *cap) -> float & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mSynthBGain;
         },
         this},
        AlwaysEnabled};

    GuiKnobGainControl mHarmGain = {
        0,
        PointI::Construct(8, 32),
        StandardRangeSpecs::gGeneralGain,
        "Harm Gain",
        {[](void *cap) -> float & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mHarmGain;
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
             return pThis->mAppSettings->GetCurrentPerformancePatch().mSynthPresetA;
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
             return pThis->mAppSettings->GetCurrentPerformancePatch().mSynthPresetB;
         },
         this},
        AlwaysEnabled};

    GuiHarmPatchSelectControl<int16_t> mHarmPatch = {
        0,
        RectI::Construct(24, 35, 45, 9),
        "Harm Patch",
        {[](void *cap) -> int16_t & {
             auto *pThis = (GuiPerformanceApp *)cap;
             return pThis->mAppSettings->GetCurrentPerformancePatch().mHarmPreset;
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
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mMasterGain;
         },
         this},
        AlwaysEnabled};

    GuiStereoSpreadControl mStereoSpread = {
        0,                               // page
        PointI::Construct(107, 4),       // pos
        StandardRangeSpecs::gFloat_N1_1, // range
        "Stereo spread",                 // tooltip
        {[](void *cap) -> float & {
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mSynthStereoSpread;
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
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mMasterFXGain;
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
             return ((GuiPerformanceApp *)cap)->mAppSettings->GetCurrentPerformancePatch().mMasterFXEnable; //
         },
         this},
        AlwaysEnabled};

    GuiTempoControl mTempo = {0,
                              RectI::Construct(70, 29, 26, 10),
                              StandardRangeSpecs::gBPMRange,
                              "Master BPM",
                              {[](void *cap) -> float {
                                   auto *pThis = (GuiPerformanceApp *)cap;
                                   return pThis->mAppSettings->GetCurrentPerformancePatch().mBPM;
                               },
                               [](void *cap, const float &val) -> void {
                                   auto *pThis = (GuiPerformanceApp *)cap;
                                   pThis->mAppSettings->GetCurrentPerformancePatch().mBPM = val;
                                   pThis->mMetronome.OnBPMChanged();
                               },
                               this},
                              AlwaysEnabled,
                              [](void *cap, DisplayApp &app) { // on toggle handler
                                  app.mAppSettings->mMetronomeSoundOn = !app.mAppSettings->mMetronomeSoundOn;
                              }};

    GuiTransposeControl<int8_t> mTranspose = {0,
                                              RectI::Construct(98, 29, 24, 10),
                                              StandardRangeSpecs::gTransposeRange.Cast<int8_t>(),
                                              "Transpose",
                                              {[](void *cap) -> int8_t & {
                                                   auto *pThis = (GuiPerformanceApp *)cap;
                                                   return pThis->mAppSettings->GetCurrentPerformancePatch().mTranspose;
                                               },
                                               this},
                                              AlwaysEnabled};

    GuiLabelControl mGlobalScale = {0, PointI::Construct(70, 39), "F# hw dim"};

    GuiTransposeControl<int8_t> mTranspose2 = {1,
                                               RectI::Construct(96, 31, 27, 10),
                                               StandardRangeSpecs::gTransposeRange.Cast<int8_t>(),
                                               "Transpose",
                                               {[](void *cap) -> int8_t & {
                                                    auto *pThis = (GuiPerformanceApp *)cap;
                                                    return pThis->mAppSettings->GetCurrentPerformancePatch().mTranspose;
                                                },
                                                this},
                                               AlwaysEnabled};

    IGuiControl *mArray[19] = {
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

        &mLabelFx,
        &mFXGain,
        &mFXEnable,

        &mTempo,
        &mTranspose,
        &mGlobalScale,

        // PAGE 2
        &mTranspose2,
    };

    GuiControlList mList = {mArray};

    virtual GuiControlList *GetRootControlList() override
    {
        return &mList;
    }
};

} // namespace clarinoid
