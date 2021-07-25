

#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/menu/MenuAppBase.hpp>

namespace clarinoid
{

static const uint8_t PROGMEM gNextPageBMP[] = {
    0b10000000,
    0b10000000,
    0b10000000,
    0b11000000,
    0b11000000,
    0b11000000,
    0b11100000,
    0b11100000,
    0b11100000,
    0b11000000,
    0b11000000,
    0b11000000,
    0b10000000,
    0b10000000,
    0b10000000,
};
static const uint8_t gNextPageBMP_BmpWidthBits = 8;
static const uint8_t gNextPageBMP_DisplayWidthBits = 3;
static const uint8_t gNextPageBMP_Height = SizeofStaticArray(gNextPageBMP);

static const uint8_t PROGMEM gPrevPageBMP[] = {
    0b00100000,
    0b00100000,
    0b00100000,
    0b01100000,
    0b01100000,
    0b01100000,
    0b11100000,
    0b11100000,
    0b11100000,
    0b01100000,
    0b01100000,
    0b01100000,
    0b00100000,
    0b00100000,
    0b00100000,
};
static const uint8_t gPrevPageBMP_BmpWidthBits = 8;
static const uint8_t gPrevPageBMP_Height = SizeofStaticArray(gPrevPageBMP);

// ---------------------------------------------------------------------------------------
// very much like ISettingItem.
// represents an INSTANCE of a gui control, not a control type.
struct IGuiControl
{
    virtual int IGuiControl_GetPage()
    {
        return mPage;
    }
    virtual RectI IGuiControl_GetBounds()
    {
        return mBounds;
    }
    virtual bool IGuiControl_IsSelectable()
    {
        return mIsSelectable;
    }
    virtual void IGuiControl_Render(CCDisplay &display) = 0;
    virtual void IGuiControl_Update(CCDisplay &display) = 0;

  protected:
    RectI mBounds;
    bool mIsSelectable = true;
    int mPage = 0;
};

// ---------------------------------------------------------------------------------------
struct GuiLabelControl : IGuiControl
{
    String mText;

    GuiLabelControl(int page, bool isSelectable, RectI bounds, const String &s) : mText(s)
    {
        IGuiControl::mPage = page;
        IGuiControl::mBounds = bounds;
        IGuiControl::mIsSelectable = isSelectable;
    }
    virtual void IGuiControl_Render(CCDisplay &display) override
    {
        display.mDisplay.setCursor(mBounds.x, mBounds.y);
        display.SetClipRect(mBounds);
        display.mDisplay.print(mText);
    }
    virtual void IGuiControl_Update(CCDisplay &display) override
    {
        //
    }
};

// ---------------------------------------------------------------------------------------
struct GuiControlList // very much like SettingsList. simpler though because we don't bother with multi items.
{
    IGuiControl **mItems;
    size_t mItemRawCount;

    template <size_t N>
    GuiControlList(IGuiControl *(&arr)[N]) : mItems(arr), mItemRawCount(N)
    {
    }

    size_t Count() const
    {
        return mItemRawCount;
    }

    IGuiControl *GetItem(size_t i)
    {
        CCASSERT(i < mItemRawCount);
        return mItems[i];
    }
};

// ---------------------------------------------------------------------------------------
struct GuiApp : public DisplayApp
{
    bool mTemporaryPageOnlyView = false; // set to true when you're viewing a page with no selectable controls.
    int32_t mSelectedIndex = 0;
    int32_t mPageIndexWhenNoSelection = 0; // the page index to display when there's no selected control. this way you
                                           // can still VIEW other pages without having to select controls.

    static constexpr int mTransitionXDeltaPerFrame = 8; // pixels to slide each frame.

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
            AdjustSelectedControl(mEnc.GetIntDelta());
        }

        // back/up when not editing
        if (mBack.IsNewlyPressed())
        {
            GoToFrontPage();
        }
    }

    int GetSelectedPage()
    {
        if (mTemporaryPageOnlyView)
        {
            return mPageIndexWhenNoSelection;
        }
        auto *ctrl = GetSelectedControl();
        if (ctrl == nullptr)
        {
            return mPageIndexWhenNoSelection;
        }
        return ctrl->IGuiControl_GetPage();
    }

    virtual void RenderFrontPage() override
    {
        mDisplay.mDisplay.print("gui?");
    }

    int32_t GetPageCount()
    {
        auto *list = GetRootControlList();
        int32_t ret = 1;
        for (size_t i = 0; i < list->Count(); ++i)
        {
            auto *ctrl = list->GetItem(i);
            ret = std::max((int32_t)ctrl->IGuiControl_GetPage() + 1, ret);
        }
        return ret;
    }

    IGuiControl *IncrementSelectedControl()
    {
        auto *list = GetRootControlList();
        if (!list->Count())
            return nullptr;

        if (mTemporaryPageOnlyView)
        {
            // increment the temporary page until we get in sync with the selected item
            mPageIndexWhenNoSelection = RotateIntoRange(mPageIndexWhenNoSelection + 1, GetPageCount());
            int targetPage = list->GetItem(mSelectedIndex)->IGuiControl_GetPage();
            if (ModularDistance(GetPageCount(), targetPage, mPageIndexWhenNoSelection) > 0)
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

        int oldPage = GetSelectedPage();

        for (size_t i = 0; i < list->Count(); ++i)
        {
            size_t adjindex = RotateIntoRange(i + mSelectedIndex + 1, list->Count());
            auto *ctrl = list->GetItem(adjindex);
            if (ctrl->IGuiControl_IsSelectable())
            {
                mSelectedIndex = adjindex;
                int newPage = GetSelectedPage();
                if (ModularDistance(GetPageCount(), newPage, oldPage) > 1)
                {
                    mPageIndexWhenNoSelection = RotateIntoRange(oldPage + 1, GetPageCount());
                    mTemporaryPageOnlyView = true;
                }
                else
                {
                    mTemporaryPageOnlyView = false;
                }
                return ctrl;
            }
        }
        mPageIndexWhenNoSelection = RotateIntoRange(mPageIndexWhenNoSelection + 1, GetPageCount());
        return nullptr;
    }

    IGuiControl *DecrementSelectedControl()
    {
        auto *list = GetRootControlList();
        if (!list->Count())
            return nullptr;

        if (mTemporaryPageOnlyView)
        {
            // increment the temporary page until we get in sync with the selected item
            mPageIndexWhenNoSelection = RotateIntoRange(mPageIndexWhenNoSelection - 1, GetPageCount());
            int targetPage = list->GetItem(mSelectedIndex)->IGuiControl_GetPage();
            if (ModularDistance(GetPageCount(), targetPage, mPageIndexWhenNoSelection) > 0)
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

        int oldPage = GetSelectedPage();
        for (size_t i = 0; i < list->Count(); ++i)
        {
            size_t adjindex = RotateIntoRange(mSelectedIndex - i - 1, list->Count());
            auto *ctrl = list->GetItem(adjindex);
            if (ctrl->IGuiControl_IsSelectable())
            {
                mSelectedIndex = adjindex;
                int newPage = GetSelectedPage();
                if (ModularDistance(GetPageCount(), newPage, oldPage) > 1)
                {
                    mPageIndexWhenNoSelection = RotateIntoRange(oldPage - 1, GetPageCount());
                    mTemporaryPageOnlyView = true;
                }
                else
                {
                    mTemporaryPageOnlyView = false;
                }
                return ctrl;
            }
        }
        mPageIndexWhenNoSelection = RotateIntoRange(mPageIndexWhenNoSelection - 1, GetPageCount());
        return nullptr;
    }

    void AdjustSelectedControl(int delta)
    {
        if (delta >= 0)
        {
            for (int n = 0; n < delta; ++n) // for each encoder increment, select the next
            {
                IncrementSelectedControl();
            }
        }
        else
        {
            for (int n = 0; n < -delta; ++n)
            {
                DecrementSelectedControl();
            }
        }
    }

    IGuiControl *GetSelectedControl()
    {
        auto *list = GetRootControlList();
        if (list->Count() == 0)
            return nullptr;

        auto *ret = list->GetItem(mSelectedIndex);

        if (!ret->IGuiControl_IsSelectable())
        {
            return IncrementSelectedControl();
        }
        return ret;
    }

    virtual void RenderApp() override
    {
        auto *list = GetRootControlList();
        auto *selectedCtrl = GetSelectedControl();
        int selectedPage = GetSelectedPage();
        for (size_t i = 0; i < list->Count(); ++i)
        {
            auto *ctrl = list->GetItem(i);
            if (ctrl->IGuiControl_GetPage() != selectedPage)
                continue;
            ctrl->IGuiControl_Render(mDisplay);
            if (ctrl == selectedCtrl)
            {
                mDisplay.DrawSelectionRect(ctrl->IGuiControl_GetBounds().Inflate(1));
                mDisplay.DrawSelectionRect(ctrl->IGuiControl_GetBounds());
            }
        }

        mDisplay.ResetClip();
        if (selectedPage < GetPageCount() - 1)
        {
            mDisplay.mDisplay.drawBitmap(mDisplay.mDisplay.width() - gNextPageBMP_DisplayWidthBits,
                                         (mDisplay.GetClientHeight() - gNextPageBMP_Height) / 2,
                                         gNextPageBMP,
                                         gNextPageBMP_BmpWidthBits,
                                         gNextPageBMP_Height,
                                         SSD1306_WHITE);
        }
        if (selectedPage > 0)
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
