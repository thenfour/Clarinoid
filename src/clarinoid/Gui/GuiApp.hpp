

#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/menu/MenuAppBase.hpp>
#include "GuiNavigation.hpp"
#include "GuiControlBase.hpp"

#include "GuiBoolControl.hpp"
#include "GuiIntegerControl.hpp"
#include "GuiKnobControl.hpp"
#include "GuiEnumControl.hpp"
#include "GuiStereoSpreadControl.hpp"
#include "GuiSynthPatchSelectControl.hpp"
#include "GuiHarmPatchSelectControl.hpp"

namespace clarinoid
{

// ---------------------------------------------------------------------------------------
struct GuiApp : public DisplayApp
{
    GuiNavigationLogic mNavigator;
    bool mIsEditing = false;

    GuiApp(CCDisplay &d) : DisplayApp(d)
    {
    }

    virtual void UpdateApp() override
    {
        auto *list = GetRootControlList();
        auto navState = mNavigator.GetNavState(list);
        if (IsShowingFrontPage())
        {
            return;
        }

        bool mWasEditing = mIsEditing;
        if (mIsEditing)
        {
            if (mBack.IsNewlyPressed())
            {
                if (navState.mSelectedControl)
                {
                    navState.mSelectedControl->IGuiControl_EditEnd(*this, true);
                }
                mIsEditing = false;
            }
            else if (mOK.IsNewlyPressed())
            {
                if (navState.mSelectedControl)
                {
                    navState.mSelectedControl->IGuiControl_EditEnd(*this, false);
                }
                mIsEditing = false;
            }
        }
        else
        {
            // not editing
            if (mOK.IsNewlyPressed() && navState.mSelectedControl)
            {
                mIsEditing = navState.mSelectedControl->IGuiControl_EditBegin(*this);
                // mIsEditing = true;
            }
            else
            {
                auto *oldSelection = navState.mSelectedControl;
                navState = mNavigator.AdjustSelectedControl(list, mEnc.GetIntDelta());
                if (oldSelection != navState.mSelectedControl)
                {
                    if (oldSelection)
                    {
                        oldSelection->IGuiControl_SelectEnd(*this);
                    }
                    if (navState.mSelectedControl)
                    {
                        navState.mSelectedControl->IGuiControl_SelectBegin(*this);
                    }
                }
            }
        }
        for (size_t i = 0; i < list->Count(); ++i)
        {
            auto *ctrl = list->GetItem(i);
            ctrl->IGuiControl_Update(
                ctrl == navState.mSelectedControl, mIsEditing && ctrl == navState.mSelectedControl, *this, mDisplay);
        }
        // back/up when not editing
        if (!mWasEditing && mBack.IsNewlyPressed())
        {
            GoToFrontPage();
        }
    }

    virtual void RenderFrontPage() override
    {
        auto *list = GetRootControlList();
        auto navState = mNavigator.GetNavState(list);
        for (size_t i = 0; i < list->Count(); ++i)
        {
            auto *ctrl = list->GetItem(i);
            if (ctrl->IGuiControl_GetPage() != navState.mSelectedPage)
                continue;
            ctrl->IGuiControl_Render(false, false,
                                        *this,
                                        mDisplay);
        }

        mDisplay.DrawSelectionRect(mDisplay.GetClientRect());

        DrawPageIndicators(navState);
    }

    void DrawPageIndicators(const GuiNavigationState& navState) {
        mDisplay.ResetClip();
        if (navState.mSelectedPage < navState.mPageCount - 1)
        {
            mDisplay.DrawBitmap(PointI::Construct(mDisplay.mDisplay.width() - gNextPageBitmapSpec.widthPixels,
                                                  (mDisplay.GetClientHeight() - gNextPageBitmapSpec.heightPixels) / 2),
                                gNextPageBitmapSpec);
        }
        if (navState.mSelectedPage > 0)
        {
            mDisplay.DrawBitmap(
                PointI::Construct(0, (mDisplay.GetClientHeight() - gPrevPageBitmapSpec.heightPixels) / 2),
                gPrevPageBitmapSpec);
        }
    }

    virtual void RenderApp() override
    {
        auto *list = GetRootControlList();
        auto navState = mNavigator.GetNavState(list);
        IGuiControl *ctrlToRenderLast = nullptr;
        for (size_t i = 0; i < list->Count(); ++i)
        {
            auto *ctrl = list->GetItem(i);
            if (ctrl->IGuiControl_GetPage() != navState.mSelectedPage)
                continue;
            if (ctrl == navState.mSelectedControl)
            {
                ctrlToRenderLast = ctrl;
            }
            else
            {
                ctrl->IGuiControl_Render(ctrl == navState.mSelectedControl,
                                         mIsEditing && ctrl == navState.mSelectedControl,
                                         *this,
                                         mDisplay);
            }
        }

        DrawPageIndicators(navState);

        if (ctrlToRenderLast)
        {
            mDisplay.DrawSelectionRect(ctrlToRenderLast->IGuiControl_GetBounds().Inflate(2));
            ctrlToRenderLast->IGuiControl_Render(ctrlToRenderLast == navState.mSelectedControl,
                                                 mIsEditing && ctrlToRenderLast == navState.mSelectedControl,
                                                 *this,
                                                 mDisplay);
        }
    }
    virtual GuiControlList *GetRootControlList() = 0;
};

// ---------------------------------------------------------------------------------------
struct GuiTestApp : GuiApp
{
    virtual const char *DisplayAppGetName() override
    {
        return "GuiTestApp";
    }

    GuiTestApp(CCDisplay &display) : GuiApp(display)
    {
    }

    GuiLabelControl mLabel1 = {0, PointI::Construct(0, 0), String("p aoesnut haoesntha oesnutha oesuntaoehu erf")};

    GuiLabelControl mLabel2 = {0, PointI::Construct(12, 16), String("p aoesnut haoesntha oesnutha oesuntaoehu erf")};
    GuiLabelControl mLabel3 = {0, PointI::Construct(20, 4), String("aight")};
    GuiLabelControl mLabel4 = {0, PointI::Construct(26, 48), String("p aoesnut haoesntha oesnutha oesuntaoehu erf")};
    GuiLabelControl mLabel5 = {1, PointI::Construct(5, 4), String("page 2")};
    GuiLabelControl mLabel6 = {1, PointI::Construct(5, 14), String("xyz")};

    float floatParam1 = 0.0f;
    GuiKnobControl mKnob1 = {1,
                             PointI::Construct(5, 30),
                             StandardRangeSpecs::gFloat_N1_1,
                             "Knob1", // formatter for edit
                             floatParam1,
                             AlwaysEnabled};

    float floatParam2 = 0.0f;
    GuiKnobControl mKnob2 = {1,
                             PointI::Construct(55, 30),
                             StandardRangeSpecs::gFloat_0_2,
                             "Knob2", // formatter for edit
                             floatParam2,
                             AlwaysEnabled};

    float floatParam3 = 0.0f;
    GuiKnobGainControl mKnob3 = {1,
                                 PointI::Construct(75, 30),
                                 StandardRangeSpecs::gMasterGainDb,
                                 "Master Gain", // formatter for edit
                                 floatParam3,
                                 AlwaysEnabled};

    float floatParam4 = 0.0f;
    GuiStereoSpreadControl mStereoSpread1 = {
        1,                               // page
        PointI::Construct(75, 10),       // pos
        StandardRangeSpecs::gFloat_N1_1, // range
        "Stereo spread",                 // tooltip
        floatParam4,                     // binding
        AlwaysEnabled                    // always en
    };

    GuiLabelControl mLabel7 = {2, PointI::Construct(5, 24), String("page 3")};

    ClarinoidFilterType mEnumParam1 = ClarinoidFilterType::HP_K35;
    GuiEnumControl<ClarinoidFilterType> mEnum1 = {2,                              // page
                                                  RectI::Construct(5, 2, 50, 20), // bounds
                                                  "Filter type",
                                                  gClarinoidFilterTypeInfo, // enuminfo
                                                  mEnumParam1,
                                                  AlwaysEnabled};

    bool mMuteParam = false;
    GuiMuteControl<true> mMute = {2, PointI::Construct(50, 2), "mute?", "yea", "nah", mMuteParam, AlwaysEnabled};

    GuiLabelControl mLabel8 = {3, PointI::Construct(4, 34), String("page 4")};
    GuiLabelControl mLabel9 = {4, PointI::Construct(4, 44), String("page 5")};

    int param1 = 111;
    GuiIntegerTextControl<int> mCtrl4a = {
        4,
        RectI::Construct(5, 10, 50, 20),
        StandardRangeSpecs::gMetronomeDecayRange,
        "Param1",
        param1,
        AlwaysEnabled // selectable
    };

    int param2 = 2;
    GuiIntegerTextControl<int> mCtrl4b = {
        4,
        RectI::Construct(5, 35, 117, 20),
        StandardRangeSpecs::gMetronomeDecayRange,
        "param2",
        param2,
        AlwaysEnabled // selectable
    };

    GuiLabelControl mLabel10 = {5, PointI::Construct(106, 6), String("page 6 ~~")};

    int16_t mSynthPatchAval = 2;
    GuiSynthPatchSelectControl<int16_t> mSynthPatchA =
        {5, false, RectI::Construct(6, 6, 59, 10), "Synth Patch A", mSynthPatchAval, AlwaysEnabled};

    int16_t mSynthPatchBval = 2;
    GuiSynthPatchSelectControl<int16_t> mSynthPatchB =
        {5, true, RectI::Construct(6, 16, 59, 10), "Synth Patch B", mSynthPatchBval, AlwaysEnabled};

    int16_t mSynthPatchCval = 2;
    GuiSynthPatchSelectControl<int16_t> mSynthPatchC =
        {5, true, RectI::Construct(6, 26, 59, 10), "Synth Patch C", mSynthPatchCval, AlwaysEnabled};

    int mHarmPatchVal = 2;
    GuiHarmPatchSelectControl<int> mHarmPatch = {5,
                                            RectI::Construct(6, 36, 59, 10),
                                            "Harm Patch",
                                            mHarmPatchVal,
                                            AlwaysEnabled};

    IGuiControl *mArray[22] = {
        &mLabel1, &mLabel2,        &mLabel3,      &mLabel4,      &mLabel5,      &mLabel6,    &mKnob1,  &mKnob2,
        &mKnob3,  &mStereoSpread1, &mLabel7,      &mEnum1,       &mMute,        &mLabel8,    &mLabel9, &mCtrl4a,
        &mCtrl4b, &mLabel10,       &mSynthPatchA, &mSynthPatchB, &mSynthPatchC, &mHarmPatch,
    };

    GuiControlList mList = {mArray};

    virtual GuiControlList *GetRootControlList() override
    {
        return &mList;
    }
};

} // namespace clarinoid
