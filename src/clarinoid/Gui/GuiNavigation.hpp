

#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/menu/MenuAppBase.hpp>

#include "GuiControl.hpp"

namespace clarinoid
{

// ---------------------------------------------------------------------------------------
struct GuiNavigationState
{
    int mSelectedPage;
    IGuiControl *mSelectedCtrl; // may be null!
    int mPageCount;
};


// this is kinda convoluted so want to stick it in its own object.
// in general you want to scroll through selectable controls only, and the current page
// will follow your selection. but you also want to be able to see pages even when
// there are no selectable controls on them. so as you scroll through, you are selecting
// either a page or a control.
//
// In fact it should eventually be refactored a bit so instead of doing everything in a
// live line-by-line update, just create a complete list of nav states from the given
// control list, and then nav from that list.
struct GuiNavigationLogic
{
    bool mTemporaryPageOnlyView = false; // set to true when you're viewing a page with no selectable controls.
    int32_t mSelectedIndex = 0;
    int32_t mPageIndexWhenNoSelection = 0; // the page index to display when there's no selected control. this way you
                                           // can still VIEW other pages without having to select controls.

    IGuiControl *IncrementSelectedControl(GuiControlList *list)
    {
        // auto *list = GetRootControlList();
        if (!list->Count())
            return nullptr;

        if (mTemporaryPageOnlyView)
        {
            // increment the temporary page until we get in sync with the selected item
            mPageIndexWhenNoSelection = RotateIntoRange(mPageIndexWhenNoSelection + 1, GetPageCount(list));
            int targetPage = list->GetItem(mSelectedIndex)->IGuiControl_GetPage();
            if (ModularDistance(GetPageCount(list), targetPage, mPageIndexWhenNoSelection) > 0)
            {
                // still no good; stay on page view.
                return nullptr;
            }
            else
            {
                // we hit it; break out of temp view.
                mTemporaryPageOnlyView = false;
            }
            return list->GetItem(mSelectedIndex);
        }

        int oldPage = GetSelectedPage(list);

        for (size_t i = 0; i < list->Count(); ++i)
        {
            size_t adjindex = RotateIntoRange(i + mSelectedIndex + 1, list->Count());
            auto *ctrl = list->GetItem(adjindex);
            if (ctrl->IGuiControl_IsSelectable())
            {
                mSelectedIndex = adjindex;
                int newPage = GetSelectedPage(list);
                if (ModularDistance(GetPageCount(list), newPage, oldPage) > 1)
                {
                    mPageIndexWhenNoSelection = RotateIntoRange(oldPage + 1, GetPageCount(list));
                    mTemporaryPageOnlyView = true;
                }
                else
                {
                    mTemporaryPageOnlyView = false;
                }
                return ctrl;
            }
        }
        mPageIndexWhenNoSelection = RotateIntoRange(mPageIndexWhenNoSelection + 1, GetPageCount(list));
        return nullptr;
    }

    IGuiControl *DecrementSelectedControl(GuiControlList *list)
    {
        // auto *list = GetRootControlList();
        if (!list->Count())
            return nullptr;

        if (mTemporaryPageOnlyView)
        {
            // increment the temporary page until we get in sync with the selected item
            mPageIndexWhenNoSelection = RotateIntoRange(mPageIndexWhenNoSelection - 1, GetPageCount(list));
            int targetPage = list->GetItem(mSelectedIndex)->IGuiControl_GetPage();
            if (ModularDistance(GetPageCount(list), targetPage, mPageIndexWhenNoSelection) > 0)
            {
                // still no good.
            }
            else
            {
                // we hit it; break out of temp view.
                mTemporaryPageOnlyView = false;
            }
            return list->GetItem(mSelectedIndex);
        }

        int oldPage = GetSelectedPage(list);
        for (size_t i = 0; i < list->Count(); ++i)
        {
            size_t adjindex = RotateIntoRange(mSelectedIndex - i - 1, list->Count());
            auto *ctrl = list->GetItem(adjindex);
            if (ctrl->IGuiControl_IsSelectable())
            {
                mSelectedIndex = adjindex;
                int newPage = GetSelectedPage(list);
                if (ModularDistance(GetPageCount(list), newPage, oldPage) > 1)
                {
                    mPageIndexWhenNoSelection = RotateIntoRange(oldPage - 1, GetPageCount(list));
                    mTemporaryPageOnlyView = true;
                }
                else
                {
                    mTemporaryPageOnlyView = false;
                }
                return ctrl;
            }
        }
        mPageIndexWhenNoSelection = RotateIntoRange(mPageIndexWhenNoSelection - 1, GetPageCount(list));
        return nullptr;
    }

    void AdjustSelectedControl(GuiControlList *list, int delta)
    {
        if (delta >= 0)
        {
            for (int n = 0; n < delta; ++n) // for each encoder increment, select the next
            {
                IncrementSelectedControl(list);
            }
        }
        else
        {
            for (int n = 0; n < -delta; ++n)
            {
                DecrementSelectedControl(list);
            }
        }
    }

    IGuiControl *GetSelectedControl(GuiControlList *list)
    {
        // auto *list = GetRootControlList();
        if (list->Count() == 0)
            return nullptr;

        auto *ret = list->GetItem(mSelectedIndex);

        if (!ret->IGuiControl_IsSelectable())
        {
            return IncrementSelectedControl(list);
        }
        return ret;
    }

    int GetSelectedPage(GuiControlList *list)
    {
        if (mTemporaryPageOnlyView)
        {
            return mPageIndexWhenNoSelection;
        }
        auto *ctrl = GetSelectedControl(list);
        if (ctrl == nullptr)
        {
            return mPageIndexWhenNoSelection;
        }
        return ctrl->IGuiControl_GetPage();
    }

    int32_t GetPageCount(GuiControlList *list)
    {
        // auto *list = GetRootControlList();
        int32_t ret = 1;
        for (size_t i = 0; i < list->Count(); ++i)
        {
            auto *ctrl = list->GetItem(i);
            ret = std::max((int32_t)ctrl->IGuiControl_GetPage() + 1, ret);
        }
        return ret;
    }

    GuiNavigationState GetNavState(GuiControlList *list)
    {
        GuiNavigationState ret;
        ret.mPageCount = GetPageCount(list);
        ret.mSelectedCtrl = GetSelectedControl(list);
        ret.mSelectedPage = GetSelectedPage(list);
        return ret;
    }
};

} // namespace clarinoid
