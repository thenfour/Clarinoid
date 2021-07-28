

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

    GuiApp(CCDisplay &d) : DisplayApp(d)
    {
    }

    virtual void UpdateApp() override
    {
        auto *list = GetRootControlList();
        for (size_t i = 0; i < list->Count(); ++i)
        {
            list->GetItem(i)->IGuiControl_Update(mDisplay);
        }

        if (!IsShowingFrontPage())
        {
            mNavigator.AdjustSelectedControl(list, mEnc.GetIntDelta());
        }

        // back/up when not editing
        if (mBack.IsNewlyPressed())
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
        for (size_t i = 0; i < list->Count(); ++i)
        {
            auto *ctrl = list->GetItem(i);
            if (ctrl->IGuiControl_GetPage() != navState.mSelectedPage)
                continue;
            ctrl->IGuiControl_Render(mDisplay);
            if (ctrl == navState.mSelectedControl)
            {
                mDisplay.DrawSelectionRect(ctrl->IGuiControl_GetBounds().Inflate(1));
                mDisplay.DrawSelectionRect(ctrl->IGuiControl_GetBounds());
            }
        }

        mDisplay.ResetClip();
        if (navState.mSelectedPage < navState.mPageCount - 1)
        {
            mDisplay.mDisplay.drawBitmap(mDisplay.mDisplay.width() - gNextPageBMP_DisplayWidthBits,
                                         (mDisplay.GetClientHeight() - gNextPageBMP_Height) / 2,
                                         gNextPageBMP,
                                         gNextPageBMP_BmpWidthBits,
                                         gNextPageBMP_Height,
                                         SSD1306_WHITE);
        }
        if (navState.mSelectedPage > 0)
        {
            mDisplay.mDisplay.drawBitmap(0,
                                         (mDisplay.GetClientHeight() - gPrevPageBMP_Height) / 2,
                                         gPrevPageBMP,
                                         gPrevPageBMP_BmpWidthBits,
                                         gPrevPageBMP_Height,
                                         SSD1306_WHITE);
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
    GuiLabelControl mLabel5 = {1, false, RectI::Construct(4, 4, 50, 8), String("page 2")};
    GuiLabelControl mLabel6 = {1, true, RectI::Construct(4, 14, 50, 8), String("xyz")};
    GuiLabelControl mLabel7 = {2, false, RectI::Construct(4, 24, 50, 8), String("page 3")};
    GuiLabelControl mLabel8 = {3, true, RectI::Construct(4, 34, 50, 8), String("page 4")};
    GuiLabelControl mLabel9 = {4, false, RectI::Construct(4, 44, 50, 8), String("page 5")};
    GuiLabelControl mLabel10 = {5, false, RectI::Construct(14, 34, 50, 8), String("page 6 ~~")};

    IGuiControl *mArray[10] = {
        &mLabel1,
        &mLabel2,
        &mLabel3,
        &mLabel4,
        &mLabel5,
        &mLabel6,
        &mLabel7,
        &mLabel8,
        &mLabel9,
        &mLabel10,
    };

    GuiControlList mList = {mArray};

    virtual GuiControlList *GetRootControlList() override
    {
        return &mList;
    }
};

} // namespace clarinoid
