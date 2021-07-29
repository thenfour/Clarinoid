

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
    IGuiControl()
    {
    }
    explicit IGuiControl(int page, const RectI &bounds) : mPage(page), mBounds(bounds)
    {
    }
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

    virtual void IGuiControl_SelectBegin(DisplayApp &app)
    {
    }
    virtual void IGuiControl_SelectEnd(DisplayApp &app)
    {
    }
    virtual void IGuiControl_EditBegin(DisplayApp &app)
    {
    }
    virtual void IGuiControl_EditEnd(DisplayApp &app, bool wasCancelled)
    {
    }

    virtual void IGuiControl_Render(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) = 0;
    virtual void IGuiControl_Update(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) = 0;

  protected:
    int mPage = 0;
    RectI mBounds;
    bool mIsSelectable = true;
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

    int GetPageCount()
    {
        int ret = 1;
        for (size_t i = 0; i < this->Count(); ++i)
        {
            auto *ctrl = this->GetItem(i);
            ret = std::max(ctrl->IGuiControl_GetPage() + 1, ret);
        }
        return ret;
    }
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
    virtual void IGuiControl_Render(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) override
    {
        display.mDisplay.setCursor(mBounds.x, mBounds.y);
        display.SetClipRect(mBounds);
        display.mDisplay.print(mText);
    }
    virtual void IGuiControl_Update(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) override
    {
        //
    }
};

// ---------------------------------------------------------------------------------------
struct GuiIntegerTextControl : IGuiControl
{
    NumericEditRangeSpec<int> mRange;
    cc::function<String(void *, const int &)>::ptr_t mValueFormatter;
    cc::function<String(void *, const int &)>::ptr_t mValueFormatterForEdit;
    Property<int> mValue;
    Property<bool> mIsSelectable;
    void *mCapture;
    // editing label placement (top/bottom)
    // text size is not a good idea because it has bugs

    int mEditingOriginalVal = 0;

    GuiIntegerTextControl(int page,
                          RectI bounds,
                          const NumericEditRangeSpec<int> &range,
                          cc::function<String(void *, const int &)>::ptr_t valueFormatter,
                          cc::function<String(void *, const int &)>::ptr_t valueFormatterForEdit,
                          const Property<int> &value,
                          const Property<bool> &isSelectable,
                          void *capture)
        : IGuiControl(page, bounds), mRange(range), mValueFormatter(valueFormatter),
          mValueFormatterForEdit(valueFormatterForEdit), mValue(value), mIsSelectable(isSelectable), mCapture(capture)
    {
    }
    virtual void IGuiControl_Render(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) override
    {
        display.ClearState();
        if (isEditing)
        {
            display.mDisplay.fillRect(0,
                                      display.GetClientHeight() - display.mDisplay.GetLineHeight(),
                                      display.mDisplay.width(),
                                      display.mDisplay.GetLineHeight(),
                                      SSD1306_WHITE);
            //Serial.println(String("y cursor setting to ") + (display.GetClientHeight() - display.mDisplay.GetLineHeight()));
            display.mDisplay.setCursor(0, display.GetClientHeight() - display.mDisplay.GetLineHeight());
            display.mDisplay.setTextColor(SSD1306_INVERSE);
            //Serial.println(String("y cursor ? ") + (display.mDisplay.getCursorY()));
            display.mDisplay.print(mValueFormatterForEdit(mCapture, mValue.GetValue()));
            //Serial.println(String("y cursor after print = ") + (display.mDisplay.getCursorY()));
        }
        display.ClearState();
        display.mDisplay.setCursor(mBounds.x + 2, mBounds.y + 2);
        display.SetClipRect(mBounds);
        display.mDisplay.print(mValueFormatter(mCapture, mValue.GetValue()));
    }
    virtual void IGuiControl_EditBegin(DisplayApp &app) override
    {
        mEditingOriginalVal = mValue.GetValue();
    }
    virtual void IGuiControl_EditEnd(DisplayApp &app, bool wasCancelled) override
    {
        if (wasCancelled)
        {
            mValue.SetValue(mEditingOriginalVal);
        }
    }
    virtual void IGuiControl_Update(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) override
    {
        if (isEditing)
        {
            int oldVal = mValue.GetValue();
            int newVal = mRange.AdjustValue(oldVal,
                                            app.mEnc.GetIntDelta(),
                                            app.mInput->mModifierCourse.CurrentValue(),
                                            app.mInput->mModifierFine.CurrentValue());
            // Serial.printf("editing param: delta=%d, old=%d, new=%d\n", app.mEnc.GetIntDelta(), oldVal, newVal);
            mValue.SetValue(newVal);
        }
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

} // namespace clarinoid
