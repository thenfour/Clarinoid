

#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/menu/MenuAppBase.hpp>
#include "GuiNavigation.hpp"

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
        mDisplay.mDisplay.print("gui?");
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

        if (ctrlToRenderLast)
        {
            mDisplay.DrawSelectionRect(ctrlToRenderLast->IGuiControl_GetBounds());
            ctrlToRenderLast->IGuiControl_Render(ctrlToRenderLast == navState.mSelectedControl,
                                                 mIsEditing && ctrlToRenderLast == navState.mSelectedControl,
                                                 *this,
                                                 mDisplay);
        }
    }
    virtual GuiControlList *GetRootControlList() = 0;
};

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
                               true,
                               RectI::Construct(0, 0, 50, 8),
                               String("p aoesnut haoesntha oesnutha oesuntaoehu erf")};

    GuiLabelControl mLabel2 = {0,
                               false,
                               RectI::Construct(12, 16, 90, 16),
                               String("p aoesnut haoesntha oesnutha oesuntaoehu erf")};
    GuiLabelControl mLabel3 = {0, true, RectI::Construct(20, 4, 50, 8), String("aight")};
    GuiLabelControl mLabel4 = {0,
                               false,
                               RectI::Construct(26, 48, 111, 4),
                               String("p aoesnut haoesntha oesnutha oesuntaoehu erf")};
    GuiLabelControl mLabel5 = {1, false, RectI::Construct(5, 4, 50, 8), String("page 2")};
    GuiLabelControl mLabel6 = {1, true, RectI::Construct(5, 14, 50, 8), String("xyz")};

    float floatParam1 = 0.0f;
    GuiKnobControl mKnob1 = {
        1,
        RectI::Construct(5, 30, 16, 16),
        StandardRangeSpecs::gFloat_N1_1,
        [](void *, const float &val) { return String(String("Knob1: ") + val); }, // formatter for edit
        Property<float>{[](void *cap) {
                            auto *pThis = (GuiPerformanceApp *)cap;
                            return pThis->floatParam1;
                        },
                        [](void *cap, const float &i) {
                            auto *pThis = (GuiPerformanceApp *)cap;
                            pThis->floatParam1 = i;
                        },
                        this},                                                                 // value
        Property<bool>{[](void *cap) { return true; }, [](void *cap, const bool &b) {}, this}, // selectable
        this                                                                                   // cap

    };

    float floatParam2 = 0.0f;
    GuiKnobControl mKnob2 = {
        1,
        RectI::Construct(55, 30, 15, 15),
        StandardRangeSpecs::gFloat_0_2,
        [](void *, const float &val) { return String(String("Knob2: ") + val); }, // formatter for edit
        Property<float>{[](void *cap) {
                            auto *pThis = (GuiPerformanceApp *)cap;
                            return pThis->floatParam2;
                        },
                        [](void *cap, const float &i) {
                            auto *pThis = (GuiPerformanceApp *)cap;
                            pThis->floatParam2 = i;
                        },
                        this},                                                                 // value
        Property<bool>{[](void *cap) { return true; }, [](void *cap, const bool &b) {}, this}, // selectable
        this                                                                                   // cap

    };

    float floatParam3 = 0.0f;
    GuiKnobGainControl mKnob3 = {
        1,
        RectI::Construct(75, 30, 15, 15),
        StandardRangeSpecs::gMasterGainDb,
        [](void *, const float &val) {
            return String(String("Master Gain: ") + DecibelsToIntString(val));
        }, // formatter for edit
        Property<float>{[](void *cap) {
                            auto *pThis = (GuiPerformanceApp *)cap;
                            return pThis->floatParam3;
                        },
                        [](void *cap, const float &i) {
                            auto *pThis = (GuiPerformanceApp *)cap;
                            pThis->floatParam3 = i;
                        },
                        this},                                                                 // value
        Property<bool>{[](void *cap) { return true; }, [](void *cap, const bool &b) {}, this}, // selectable
        this                                                                                   // cap

    };

    float floatParam4 = 0.0f;
    GuiStereoSpreadControl mStereoSpread1 = {
        1,
        RectI::Construct(75, 10, 17, 7),
        StandardRangeSpecs::gFloat_N1_1,
        [](void *, const float &val) { return String(String("Stereo spread: ") + val); }, // formatter for edit
        Property<float>{[](void *cap) {
                            auto *pThis = (GuiPerformanceApp *)cap;
                            return pThis->floatParam4;
                        },
                        [](void *cap, const float &i) {
                            auto *pThis = (GuiPerformanceApp *)cap;
                            pThis->floatParam4 = i;
                        },
                        this},                                                                 // value
        Property<bool>{[](void *cap) { return true; }, [](void *cap, const bool &b) {}, this}, // selectable
        this                                                                                   // cap

    };

    GuiLabelControl mLabel7 = {2, false, RectI::Construct(5, 24, 50, 8), String("page 3")};

    ClarinoidFilterType mEnumParam1 = ClarinoidFilterType::HP_K35;
    GuiEnumControl<ClarinoidFilterType> mEnum1 = {
        2,                                            // page
        RectI::Construct(5, 2, 100, 20),              // bounds
        gClarinoidFilterTypeInfo,                     // enuminfo
        Property<ClarinoidFilterType>{[](void *cap) { // binding
                                          auto *pThis = (GuiPerformanceApp *)cap;
                                          return pThis->mEnumParam1;
                                      },
                                      [](void *cap, const ClarinoidFilterType &i) {
                                          auto *pThis = (GuiPerformanceApp *)cap;
                                          pThis->mEnumParam1 = i;
                                      },
                                      this},
        [](void *cap, const ClarinoidFilterType &val, bool isSelected, bool isEditing, DisplayApp &app) { // render fn
            app.mDisplay.mDisplay.print(String(gClarinoidFilterTypeInfo.GetValueString(val)));
            //app.mDisplay.mDisplay.print("hi.");
        },
        Property<bool>{
            [](void *cap) { return true; }, // selectable
        },
        this};

    GuiLabelControl mLabel8 = {3, true, RectI::Construct(4, 34, 50, 8), String("page 4")};
    GuiLabelControl mLabel9 = {4, false, RectI::Construct(4, 44, 50, 8), String("page 5")};

    int param1 = 111;
    GuiIntegerTextControl mCtrl4a = {
        4,
        RectI::Construct(5, 10, 50, 20),
        StandardRangeSpecs::gMetronomeDecayRange,
        [](void *, const int &val) { return String(val); },                      // formatter
        [](void *, const int &val) { return String(String("Param1: ") + val); }, // formatter
        Property<int>{[](void *cap) {
                          auto *pThis = (GuiPerformanceApp *)cap;
                          return pThis->param1;
                      },
                      [](void *cap, const int &i) {
                          auto *pThis = (GuiPerformanceApp *)cap;
                          pThis->param1 = i;
                      },
                      this},                                                                   // value
        Property<bool>{[](void *cap) { return true; }, [](void *cap, const bool &b) {}, this}, // selectable
        this                                                                                   // cap
    };

    int param2 = 2;
    GuiIntegerTextControl mCtrl4b = {
        4,
        RectI::Construct(5, 35, 117, 20),
        StandardRangeSpecs::gMetronomeDecayRange,
        [](void *, const int &val) { return String(val); },                         // formatter
        [](void *, const int &val) { return String(String("Param two: ") + val); }, // formatter
        Property<int>{[](void *cap) {
                          auto *pThis = (GuiPerformanceApp *)cap;
                          return pThis->param2;
                      },
                      [](void *cap, const int &i) {
                          auto *pThis = (GuiPerformanceApp *)cap;
                          pThis->param2 = i;
                      },
                      this},                                                                    // value
        Property<bool>{[](void *cap) { return false; }, [](void *cap, const bool &b) {}, this}, // selectable
        this                                                                                    // cap
    };
    GuiLabelControl mLabel10 = {5, false, RectI::Construct(14, 34, 50, 8), String("page 6 ~~")};

    IGuiControl *mArray[17] = {
        &mLabel1,
        &mLabel2,
        &mLabel3,
        &mLabel4,
        &mLabel5,
        &mLabel6,
        &mKnob1,
        &mKnob2,
        &mKnob3,
        &mStereoSpread1,
        &mLabel7,
        &mEnum1,
        &mLabel8,
        &mLabel9,
        &mCtrl4a,
        &mCtrl4b,
        &mLabel10,
    };

    GuiControlList mList = {mArray};

    virtual GuiControlList *GetRootControlList() override
    {
        return &mList;
    }
};

} // namespace clarinoid
