

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

    int GetPageCount() {
        int ret = 1;
        for (size_t i = 0; i < this->Count(); ++i)
        {
            auto *ctrl = this->GetItem(i);
            ret = std::max(ctrl->IGuiControl_GetPage() + 1, ret);
        }
        return ret;
    }
};

// select patch
// select harm patch
// knob (N11 or 01)
// stereo width (N11 or 01)
// text
// filter type
// waveform
// IntString ("transpose +12")


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

} // namespace clarinoid
