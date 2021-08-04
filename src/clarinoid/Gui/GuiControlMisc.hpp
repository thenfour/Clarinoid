#pragma once

#include "GuiControlBase.hpp"

namespace clarinoid
{

static constexpr int DOUBLE_CLICK_TIMEOUT_MS = 1000;

// ---------------------------------------------------------------------------------------
// For the little one-line editor on the top/bottom of the screen, this function
// sets up the mini editor state so the caller can just print the text or render or whatever it needs to do.
// returns true if the caller should proceed drawing the editor.
static bool GuiInitiateTooltip(bool isSelected, bool isEditing, DisplayApp &app)
{
    if (!isSelected)
        return false;
    app.mDisplay.ClearState();
    app.mDisplay.setCursor(1, 1 + app.mDisplay.GetClientHeight() - app.mDisplay.GetLineHeight());
    if (!isEditing)
    {
        app.mDisplay.DrawMarchingAntsFilledRect(
            3, 1, 1, 1, 0, app.mDisplay.GetClientHeight() - app.mDisplay.GetLineHeight(), app.mDisplay.width(), 1, 0);
        app.mDisplay.fillRect(0,
                              1 + app.mDisplay.GetClientHeight() - app.mDisplay.GetLineHeight(),
                              app.mDisplay.width(),
                              app.mDisplay.GetLineHeight(),
                              SSD1306_BLACK);
        app.mDisplay.setTextColor(SSD1306_WHITE);
        return true;
    }
    app.mDisplay.fillRect(0,
                          app.mDisplay.GetClientHeight() - app.mDisplay.GetLineHeight(),
                          app.mDisplay.width(),
                          app.mDisplay.GetLineHeight(),
                          SSD1306_WHITE);
    app.mDisplay.setTextColor(SSD1306_INVERSE);
    return true;
}

// ---------------------------------------------------------------------------------------
// just draws the value, as long as it can be converted to String()
template <typename T>
struct GuiValueAsTextRenderer : IGuiRenderer<T>
{
    typename cc::function<String(void *, const T &)>::ptr_t mValueFormatter = nullptr;
    void *mpCapture = nullptr;

    GuiValueAsTextRenderer() = default;

    GuiValueAsTextRenderer(typename cc::function<String(void *, const T &)>::ptr_t valueFormatter,
                           void *pCapture)
        : mValueFormatter(valueFormatter), //
          mpCapture(pCapture)
    {
    }

    virtual void IGuiRenderer_Render(IGuiControl &ctrl,
                                     const T &val,
                                     bool dblVal,
                                     bool isSelected,
                                     bool isEditing,
                                     DisplayApp &app) override
    {
        app.mDisplay.print(mValueFormatter ? mValueFormatter(mpCapture, val) : String(val));
    }
};

// ---------------------------------------------------------------------------------------
// renders a "Label: n" tooltip for types which are convertible directly to String.
template <typename T>
struct GuiLabelValueTooltipRenderer : IGuiRenderer<T>
{
    String mStaticCaption;
    typename cc::function<String(void *, const T &)>::ptr_t mValueFormatter = nullptr;
    void *mpCapture = nullptr;

    GuiLabelValueTooltipRenderer(const String &staticCaption) : mStaticCaption(staticCaption)
    {
    }

    GuiLabelValueTooltipRenderer(const String &staticCaption,
                                 typename cc::function<String(void *, const T &)>::ptr_t valueFormatter,
                                 void *pCapture)
        : mStaticCaption(staticCaption),   //
          mValueFormatter(valueFormatter), //
          mpCapture(pCapture)
    {
    }

    virtual void IGuiRenderer_Render(IGuiControl &ctrl,
                                     const T &val,
                                     bool dblVal,
                                     bool isSelected,
                                     bool isEditing,
                                     DisplayApp &app) override
    {
        if (GuiInitiateTooltip(isSelected, isEditing, app))
        {
            String valStr = (mValueFormatter ? mValueFormatter(mpCapture, val) : String(val));
            app.mDisplay.print(mStaticCaption + ": " + valStr);
        }
    }
};

// ---------------------------------------------------------------------------------------
template <typename T>
struct GuiStaticTooltipRenderer : IGuiRenderer<T>
{
    String mStaticCaption;
    GuiStaticTooltipRenderer(const String &staticCaption) : mStaticCaption(staticCaption)
    {
    }
    virtual void IGuiRenderer_Render(IGuiControl &ctrl,
                                     const T &val,
                                     bool dblVal,
                                     bool isSelected,
                                     bool isEditing,
                                     DisplayApp &app) override
    {
        if (GuiInitiateTooltip(isSelected, isEditing, app))
        {
            app.mDisplay.print(mStaticCaption);
        }
    }
};

// ---------------------------------------------------------------------------------------
// no modal, this edits in the tooltip area
template <typename Tparam>
struct GuiNumericEditor : IGuiEditor<Tparam>
{
    NumericEditRangeSpec<Tparam> mRange;
    Tparam mOldVal;
    bool mWasModified = false;
    Stopwatch mWhenEditingStarted;
    GuiNumericEditor(const NumericEditRangeSpec<Tparam> &range) : mRange(range)
    {
    }
    virtual bool IGuiEditor_StartEditing(IGuiControl &ctrl,
                                         Property<Tparam> &binding,
                                         Property<bool> &dblBinding,
                                         DisplayApp &app) override
    {
        mWasModified = false;
        mWhenEditingStarted.Restart();
        mOldVal = binding.GetValue();
        return true;
    }

    virtual void IGuiEditor_StopEditing(IGuiControl &ctrl,
                                        Property<Tparam> &binding,
                                        Property<bool> &dblBinding,
                                        DisplayApp &app,
                                        bool wasCancelled) override
    {
        if (wasCancelled)
        {
            binding.SetValue(mOldVal);
        }
        else if (!mWasModified && (mWhenEditingStarted.ElapsedTime() < TimeSpan::FromMillis(DOUBLE_CLICK_TIMEOUT_MS)))
        {
            // not cancelled (user clicked OK), but did not change value. that's a double click.
            dblBinding.SetValue(!dblBinding.GetValue());
        }
    }

    virtual void IGuiEditor_Render(IGuiControl &ctrl,
                                   Property<Tparam> &binding,
                                   Property<bool> &dblBinding,
                                   bool isSelected,
                                   bool isEditing,
                                   DisplayApp &app) override
    {
        if (isEditing)
        {
            Tparam oldVal = binding.GetValue();
            auto d = app.mEnc.GetIntDelta();
            if (d != 0)
            {
                mWasModified = true;
            }
            Tparam newVal = mRange.AdjustValue(
                oldVal, d, app.mInput->mModifierCourse.CurrentValue(), app.mInput->mModifierFine.CurrentValue());
            binding.SetValue(newVal);
        }
    }
};

struct GuiRectangleMarquee : IGuiControl
{
    Edges::Flags mEdgeFlags;
    GuiRectangleMarquee(int page, const RectI &bounds, Edges::Flags edges = Edges::All) : mEdgeFlags(edges)
    {
        this->mPage = page;
        this->mBounds = bounds;
        this->mIsSelectable = false;
    }
    virtual void IGuiControl_Render(bool isSelected, bool isEditing, DisplayApp &app, IDisplay &display) override
    {
        // display.ClearState();
        display.DrawMarchingAntsRectOutline(2,
                                            3 /*ant size*/,
                                            1,
                                            mBounds.x,
                                            mBounds.y,
                                            mBounds.width,
                                            mBounds.height,
                                            3,
                                            AntStyle::Continuous,
                                            mEdgeFlags);
    }
    virtual void IGuiControl_Update(bool isSelected, bool isEditing, DisplayApp &app, IDisplay &display) override
    {
    }
};

} // namespace clarinoid
