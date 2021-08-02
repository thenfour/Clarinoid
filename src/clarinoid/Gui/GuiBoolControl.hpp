
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

template<bool invertVal>
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
                                     const T &val_,
                                     bool isSelected,
                                     bool isEditing,
                                     DisplayApp &app) override
    {
        bool val = invertVal ? !val_ : val_;
        app.mDisplay.DrawBitmap(ctrl.mBounds.UpperLeft(), val ? gMuteOnBitmapSpec : gMuteOffBitmapSpec);
        if (GuiInitiateTooltip(isSelected, isEditing, app))
        {
            app.mDisplay.mDisplay.print(mStaticCaption + ": " + (val_ ? mTrueValue : mFalseValue));
        }
    }
};

template<bool invertVal>
struct GuiMuteControl : GuiCompositeControl<bool>
{
    GuiMuteControlRenderer<invertVal> mRenderer;
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

template<bool invertVal>
struct GuiPatchMuteControlRenderer : IGuiRenderer<bool>
{
    using T = bool;
    String mStaticCaption;
    String mTrueValue;
    String mFalseValue;
    GuiPatchMuteControlRenderer(const String &staticCaption, //
                                const String &trueValue,     //
                                const String &falseValue)
        : mStaticCaption(staticCaption), mTrueValue(trueValue), mFalseValue(falseValue)
    {
    }
    virtual void IGuiRenderer_Render(IGuiControl &ctrl,
                                     const T &val_,
                                     bool isSelected,
                                     bool isEditing,
                                     DisplayApp &app) override
    {
        bool val = (invertVal ? !val_ : val_);
        app.mDisplay.DrawBitmap(ctrl.mBounds.UpperLeft(), val ? gPatchEnabledSpec : gPatchDisabledSpec);
        if (GuiInitiateTooltip(isSelected, isEditing, app))
        {
            app.mDisplay.mDisplay.print(mStaticCaption + ": " + (val_ ? mTrueValue : mFalseValue));
        }
    }
};

template<bool invertVal>
struct GuiPatchMuteControl : GuiCompositeControl<bool>
{
    GuiPatchMuteControlRenderer<invertVal> mRenderer;
    GuiToggleEditor mEditor;

    GuiPatchMuteControl(int page, //
                        PointI pt,                         //
                        const String &tooltipCaption,      //
                        const String &trueValue,           //
                        const String &falseValue,          //
                        const Property<bool> &binding,     //
                        const Property<bool> &isSelectable //
                        )
        : GuiCompositeControl(page, RectI::Construct(pt, 7, 7), binding, &mRenderer, &mEditor, isSelectable),
          mRenderer(tooltipCaption, trueValue, falseValue)
    {
    }
};

} // namespace clarinoid
