

#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/menu/MenuAppBase.hpp>

#include "GuiControl.hpp"

namespace clarinoid
{

// ---------------------------------------------------------------------------------------
static constexpr size_t MaxScrollSequenceLen =
    50; // should be enough to represent the maximum complete # of selectable items on the menu.

struct ScrollSequenceItem
{
    IGuiControl *mpCtrl = nullptr; // when null, show the page but no selected control.
    int mPage = -1;                // represents an invalid item.
};
struct GuiNavigationState
{
    int mPageCount;
    int mSelectedScrollSequenceIndex;
    int mScrollSequenceLength;
    int mSelectedPage;
    IGuiControl *mSelectedControl = nullptr;
    ScrollSequenceItem mScrollSequence[MaxScrollSequenceLen];
};
// static constexpr size_t scrollsequencesize = sizeof(ScrollSequence);

// ---------------------------------------------------------------------------------------
// this is kinda convoluted so want to stick it in its own object.
// in general you want to scroll through selectable controls only, and the current page
// will follow your selection. but you also want to be able to see pages even when
// there are no selectable controls on them. so as you scroll through, you are selecting
// either a page or a control.
struct GuiNavigationLogic
{
    int mSelectedScrollSequenceIndex = 0;

    // generate a list of items you can scroll through. that includes selectable controls,
    // or in the case where a page contains no selectable controls, an item representing the page with no selected
    // control. assumes that the control list is in tab order.
    GuiNavigationState GetNavState(GuiControlList *list)
    {
        GuiNavigationState ret;
        int iSeq = 0;

        // determine page count.
        ret.mPageCount = list->GetPageCount();

        for (int iPage = 0; iPage < ret.mPageCount; ++iPage)
        {
            CCASSERT(iSeq < (int)MaxScrollSequenceLen);
            int nSelectableControls = 0;
            for (size_t iCtrl = 0; iCtrl < list->Count(); ++iCtrl)
            {
                auto *pctrl = list->GetItem(iCtrl);
                if (pctrl->IGuiControl_GetPage() != iPage)
                    continue;
                if (!pctrl->IGuiControl_IsSelectable())
                    continue;
                ret.mScrollSequence[iSeq].mPage = iPage;
                ret.mScrollSequence[iSeq].mpCtrl = pctrl;
                ++iSeq;
                ++nSelectableControls;
            }
            if (nSelectableControls < 1)
            {
                ret.mScrollSequence[iSeq].mPage = iPage;
                ret.mScrollSequence[iSeq].mpCtrl = nullptr;
                ++iSeq;
            }
        }

        ret.mScrollSequenceLength = iSeq;
        ret.mSelectedScrollSequenceIndex = RotateIntoRange(mSelectedScrollSequenceIndex, ret.mScrollSequenceLength);

        auto &si = ret.mScrollSequence[ret.mSelectedScrollSequenceIndex];
        ret.mSelectedControl = si.mpCtrl;
        ret.mSelectedPage = si.mPage;

        return ret;
    }

    // returns new state.
    GuiNavigationState AdjustSelectedControl(GuiControlList *list, int delta)
    {
        auto navState = this->GetNavState(list);
        mSelectedScrollSequenceIndex =
            RotateIntoRange(navState.mSelectedScrollSequenceIndex + delta, navState.mScrollSequenceLength);
        return this->GetNavState(list); // yea this could be optimized but not critical.
    }

};

} // namespace clarinoid
