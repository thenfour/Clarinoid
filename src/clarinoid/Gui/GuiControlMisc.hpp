#pragma once

#include "GuiControlBase.hpp"

namespace clarinoid
{

// ---------------------------------------------------------------------------------------
// For the little one-line editor on the top/bottom of the screen, this function
// sets up the mini editor state so the caller can just print the text or render or whatever it needs to do.
// returns true if the caller should proceed drawing the editor.
static bool GuiInitiateTooltip(bool isSelected, bool isEditing, DisplayApp &app)
{
    if (!isSelected)
        return false;
    app.mDisplay.ClearState();
    app.mDisplay.mDisplay.setCursor(1, 1 + app.mDisplay.GetClientHeight() - app.mDisplay.mDisplay.GetLineHeight());
    if (!isEditing)
    {
        app.mDisplay.mDisplay.DrawMarchingAntsFilledRect<3, 1, 1, 1>(0,
                                                                     app.mDisplay.GetClientHeight() -
                                                                         app.mDisplay.mDisplay.GetLineHeight(),
                                                                     app.mDisplay.mDisplay.width(),
                                                                     1,
                                                                     0);
        app.mDisplay.mDisplay.fillRect(0,
                                       1 + app.mDisplay.GetClientHeight() - app.mDisplay.mDisplay.GetLineHeight(),
                                       app.mDisplay.mDisplay.width(),
                                       app.mDisplay.mDisplay.GetLineHeight(),
                                       SSD1306_BLACK);
        app.mDisplay.mDisplay.setTextColor(SSD1306_WHITE);
        return true;
    }
    app.mDisplay.mDisplay.fillRect(0,
                                   app.mDisplay.GetClientHeight() - app.mDisplay.mDisplay.GetLineHeight(),
                                   app.mDisplay.mDisplay.width(),
                                   app.mDisplay.mDisplay.GetLineHeight(),
                                   SSD1306_WHITE);
    app.mDisplay.mDisplay.setTextColor(SSD1306_INVERSE);
    return true;
}

// ---------------------------------------------------------------------------------------
// just draws the value, as long as it can be converted to String()
template <typename T>
struct GuiValueAsTextRenderer : IGuiRenderer<T>
{
    virtual void IGuiRenderer_Render(IGuiControl &ctrl,
                                     const T &val,
                                     bool isSelected,
                                     bool isEditing,
                                     DisplayApp &app) override
    {
        app.mDisplay.mDisplay.print(String(val));
    }
};

// ---------------------------------------------------------------------------------------
// renders a "Label: n" tooltip for types which are convertible directly to String.
template <typename T>
struct GuiLabelValueTooltipRenderer : IGuiRenderer<T>
{
    String mStaticCaption;
    GuiLabelValueTooltipRenderer(const String &staticCaption) : mStaticCaption(staticCaption)
    {
    }
    virtual void IGuiRenderer_Render(IGuiControl &ctrl,
                                     const T &val,
                                     bool isSelected,
                                     bool isEditing,
                                     DisplayApp &app) override
    {
        if (GuiInitiateTooltip(isSelected, isEditing, app))
        {
            app.mDisplay.mDisplay.print(mStaticCaption + ": " + String(val));
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
                                     bool isSelected,
                                     bool isEditing,
                                     DisplayApp &app) override
    {
        if (GuiInitiateTooltip(isSelected, isEditing, app))
        {
            app.mDisplay.mDisplay.print(mStaticCaption);
        }
    }
};


// ---------------------------------------------------------------------------------------
// no modal, this edits in the tooltip area
template <typename Tparam>
struct GuiNumericEditor : IGuiEditor<Tparam>
{
    const NumericEditRangeSpec<Tparam> &mRange;
    Tparam mOldVal;
    GuiNumericEditor(const NumericEditRangeSpec<Tparam> &range) : mRange(range)
    {
    }
    virtual bool IGuiEditor_StartEditing(IGuiControl &ctrl, Property<Tparam> &binding, DisplayApp &app)
    {
        mOldVal = binding.GetValue();
        return true;
    }

    virtual void IGuiEditor_StopEditing(IGuiControl &ctrl,
                                        Property<Tparam> &binding,
                                        DisplayApp &app,
                                        bool wasCancelled)
    {
        if (wasCancelled)
        {
            binding.SetValue(mOldVal);
        }
    }

    virtual void IGuiEditor_Render(IGuiControl &ctrl,
                                   Property<Tparam> &binding,
                                   bool isSelected,
                                   bool isEditing,
                                   DisplayApp &app)
    {
        if (isEditing)
        {
            Tparam oldVal = binding.GetValue();
            Tparam newVal = mRange.AdjustValue(oldVal,
                                            app.mEnc.GetIntDelta(),
                                            app.mInput->mModifierCourse.CurrentValue(),
                                            app.mInput->mModifierFine.CurrentValue());
            binding.SetValue(newVal);
        }
    }
};


} // namespace clarinoid
