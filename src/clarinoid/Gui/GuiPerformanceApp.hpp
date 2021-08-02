

#pragma once

#include "GuiApp.hpp"

namespace clarinoid
{

// ---------------------------------------------------------------------------------------
struct GuiPerformanceApp : GuiApp
{
    virtual const char *DisplayAppGetName() override
    {
        return "GuiPerformanceApp";
    }

    GuiPerformanceApp(CCDisplay &display) : GuiApp(display)
    {
    }

    GuiLabelControl mLabel1 = {0,
                               PointI::Construct(0, 0),
                               "p aoesnut haoesntha oesnutha oesuntaoehu erf"};

    float floatParam1 = 0.0f;
    GuiKnobControl mKnob1 = {
        1,
        PointI::Construct(5, 30),
        StandardRangeSpecs::gFloat_N1_1,
        "Knob1", // formatter for edit
        floatParam1,
        AlwaysEnabled
    };

    // float floatParam4 = 0.0f;
    // GuiStereoSpreadControl mStereoSpread1 = {
    //     1,                               // page
    //     PointI::Construct(75, 10),       // pos
    //     StandardRangeSpecs::gFloat_N1_1, // range
    //     "Stereo spread",                 // tooltip
    //     floatParam4,                     // binding
    //     AlwaysEnabled                    // always en
    // };

    // GuiLabelControl mLabel7 = {2, false, RectI::Construct(5, 24, 50, 8), String("page 3")};

    // ClarinoidFilterType mEnumParam1 = ClarinoidFilterType::HP_K35;
    // GuiEnumControl<ClarinoidFilterType> mEnum1 = {2,                              // page
    //                                               RectI::Construct(5, 2, 50, 20), // bounds
    //                                               "Filter type",
    //                                               gClarinoidFilterTypeInfo, // enuminfo
    //                                               mEnumParam1,
    //                                               AlwaysEnabled};

    // bool mMuteParam = false;
    // GuiMuteControl mMute = {2, PointI::Construct(50, 2), "mute?", "yea", "nah", mMuteParam, AlwaysEnabled};

    // GuiLabelControl mLabel8 = {3, true, RectI::Construct(4, 34, 50, 8), String("page 4")};
    // GuiLabelControl mLabel9 = {4, false, RectI::Construct(4, 44, 50, 8), String("page 5")};

    // int param1 = 111;
    // GuiIntegerTextControl<int> mCtrl4a = {
    //     4,
    //     RectI::Construct(5, 10, 50, 20),
    //     StandardRangeSpecs::gMetronomeDecayRange,
    //     "Param1",
    //     param1,
    //     AlwaysEnabled // selectable
    // };

    // int param2 = 2;
    // GuiIntegerTextControl<int> mCtrl4b = {
    //     4,
    //     RectI::Construct(5, 35, 117, 20),
    //     StandardRangeSpecs::gMetronomeDecayRange,
    //     "param2",
    //     param2,
    //     AlwaysEnabled // selectable
    // };

    // GuiLabelControl mLabel10 = {5, false, RectI::Construct(106, 6, 50, 8), String("page 6 ~~")};

    int mSynthPatchAval = 2;
    GuiSynthPatchSelectControl mSynthPatchA = {
        5, false, RectI::Construct(6, 6, 59, 10), "Synth Patch A", mSynthPatchAval, AlwaysEnabled
    };

    int mSynthPatchBval = 2;
    GuiSynthPatchSelectControl mSynthPatchB = {
        5, true, RectI::Construct(6, 16, 59, 10), "Synth Patch B", mSynthPatchBval, AlwaysEnabled
    };

    int mHarmPatchVal = 2;
    GuiHarmPatchSelectControl mHarmPatch = {
        5, RectI::Construct(6, 36, 59, 10), "Harm Patch", mHarmPatchVal, AlwaysEnabled
    };



    IGuiControl *mArray[1] = {
        &mLabel1,
    };

    GuiControlList mList = {mArray};

    virtual GuiControlList *GetRootControlList() override
    {
        return &mList;
    }
};

} // namespace clarinoid
