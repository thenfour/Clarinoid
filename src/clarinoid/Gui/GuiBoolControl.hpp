
#pragma once

#include "GuiControlBase.hpp"
#include "GuiControlMisc.hpp"

namespace clarinoid
{

// ---------------------------------------------------------------------------------------
struct GuiToggleEditor : IGuiEditor<bool>
{
    virtual bool IGuiEditor_StartEditing(IGuiControl &ctrl, Property<bool> &binding, DisplayApp &app) override
    {
        binding.SetValue(!binding.GetValue());
        return false;
    }
};

struct GuiMuteControlRenderer : IGuiRenderer<bool>
{
    using T = bool;
    String mStaticCaption;
    String mTrueValue;
    String mFalseValue;
    GuiMuteControlRenderer(const String &staticCaption, const String &trueValue, const String &falseValue)
        : mStaticCaption(staticCaption), mTrueValue(trueValue), mFalseValue(falseValue)
    {
    }
    virtual void IGuiRenderer_Render(IGuiControl &ctrl,
                                     const T &val,
                                     bool isSelected,
                                     bool isEditing,
                                     DisplayApp &app) override
    {
        app.mDisplay.DrawBitmap(ctrl.mBounds.UpperLeft(), val ? gMuteOnBitmapSpec : gMuteOffBitmapSpec);
        if (GuiInitiateTooltip(isSelected, isEditing, app))
        {
            app.mDisplay.mDisplay.print(mStaticCaption + ": " + (val ? mTrueValue : mFalseValue));
        }
    }
};

struct GuiMuteControl : GuiCompositeControl<bool>
{
    GuiMuteControlRenderer mRenderer;
    GuiToggleEditor mEditor;

    GuiMuteControl(int page,                          //
                   PointI pt,                         //
                   const String &tooltipCaption,      //
                   const String &trueValue,           //
                   const String &falseValue,          //
                   const Property<bool> &binding,     //
                   const Property<bool> &isSelectable //
                   )
        : GuiCompositeControl(page, RectI::Construct(pt, 17, 7), binding, &mRenderer, &mEditor, isSelectable),
          mRenderer(tooltipCaption, trueValue, falseValue)
    {
    }
};

} // namespace clarinoid
