#pragma once

#include "GuiControlBase.hpp"
#include "GuiControlMisc.hpp"

namespace clarinoid
{
struct GuiKnobRenderer : IGuiRenderer<float>
{
    NumericEditRangeSpec<float> mRange;

    GuiKnobRenderer(const NumericEditRangeSpec<float> &range) : mRange(range)
    {
    }
    virtual void IGuiRenderer_Render(IGuiControl &ctrl,
                                     const float &val,
                                     bool isSelected,
                                     bool isEditing,
                                     DisplayApp &app) override
    {
        app.mDisplay.DrawBitmap(ctrl.mBounds.UpperLeft(), KnobDetails::gKnobOutlineSpec);
        float a0 = mRange.remap(val, KnobDetails::MinAngle, KnobDetails::MaxAngle);
        float a1 = mRange.remap(0.0f, KnobDetails::MinAngle, KnobDetails::MaxAngle);
        app.mDisplay.fillPie(KnobDetails::Origin.Add(ctrl.mBounds.UpperLeft()), KnobDetails::Radius, a0, a1 - a0);
    }
};

struct GuiGainKnobRenderer : IGuiRenderer<float>
{
    NumericEditRangeSpecWithBottom mRange;

    GuiGainKnobRenderer(const NumericEditRangeSpecWithBottom &range) : mRange(range)
    {
    }
    virtual void IGuiRenderer_Render(IGuiControl &ctrl,
                                     const float &val,
                                     bool isSelected,
                                     bool isEditing,
                                     DisplayApp &app) override
    {
        float aCenter = KnobDetails::CenterAngle; // Remap01ToRange(0.5f, KnobDetails::MinAngle, KnobDetails::MaxAngle);
        if (mRange.IsBottom(val))
        {
            // -inf db,
            app.mDisplay.DrawBitmap(ctrl.mBounds.UpperLeft(), KnobDetails::gKnobOutlineMutedSpec);
            return;
        }

        app.mDisplay.DrawBitmap(ctrl.mBounds.UpperLeft(), KnobDetails::gKnobOutlineSpec);
        if (mRange.mRangeMax >= 0) // user can GAIN in addition to attenuation; put zero point at the top.
        {
            // negative & positive poles are different scales
            float a0;
            if (val < 0)
            {
                float negPos = RemapTo01(val, mRange.mRangeMin, 0.0f);
                a0 = Remap01ToRange(negPos, KnobDetails::MinAngle, aCenter);
            }
            else
            {
                float posPos = RemapTo01(val, 0.0f, mRange.mRangeMax);
                a0 = Remap01ToRange(posPos, aCenter, KnobDetails::MaxAngle);
            }
            app.mDisplay.fillPie(
                KnobDetails::Origin.Add(ctrl.mBounds.UpperLeft()), KnobDetails::Radius, a0, aCenter - a0);
            return;
        }
        // ONLY attenuation allowed. -inf is full left, max is full right
        float a0;
        float negPos = RemapTo01(val, mRange.mRangeMin, 0.0f);
        a0 = Remap01ToRange(negPos, KnobDetails::MinAngle, aCenter);
        app.mDisplay.fillPie(KnobDetails::Origin.Add(ctrl.mBounds.UpperLeft()), KnobDetails::Radius, a0, aCenter - a0);
    }
};

// ---------------------------------------------------------------------------------------
struct GuiKnobControl : GuiCompositeControl<float>
{
    using T = float;
    GuiLabelValueTooltipRenderer<T> mTooltipRenderer;
    GuiKnobRenderer mValueRenderer;
    GuiRendererCombiner<T> mRenderer;
    GuiNumericEditor<T> mEditor;

    GuiKnobControl(int page,
                   PointI pos,
                   const NumericEditRangeSpec<float> &range,
                   const String &tooltipCaption,
                   const Property<T> &binding,
                   const Property<bool> &isSelectable)
        : GuiCompositeControl(page, RectI::Construct(pos, 16, 16), binding, &mRenderer, &mEditor, isSelectable), //
          mTooltipRenderer(tooltipCaption),                                                                      //
          mValueRenderer(range),                                                                                 //
          mRenderer(&mValueRenderer, &mTooltipRenderer),                                                         //
          mEditor(range)
    {
    }
};

// ---------------------------------------------------------------------------------------
// renders a "Label: n" tooltip for GAIN values (notably to display -inf properly and +/- signs and formatting)
template <typename T>
struct GuiLabelGainTooltipRenderer : IGuiRenderer<T>
{
    String mStaticCaption;
    GuiLabelGainTooltipRenderer(const String &staticCaption) : mStaticCaption(staticCaption)
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
            app.mDisplay.mDisplay.print(mStaticCaption + ": " + DecibelsToIntString(val));
        }
    }
};

// ---------------------------------------------------------------------------------------
struct GuiKnobGainControl : GuiCompositeControl<float>
{
    using T = float;
    GuiLabelGainTooltipRenderer<T> mTooltipRenderer;
    GuiGainKnobRenderer mValueRenderer;
    GuiRendererCombiner<T> mRenderer;
    GuiNumericEditor<T> mEditor;

    GuiKnobGainControl(int page,
                       PointI pos,
                       const NumericEditRangeSpecWithBottom &range,
                       const String &tooltipCaption,
                       const Property<T> &binding,
                       const Property<bool> &isSelectable)
        : GuiCompositeControl(page, RectI::Construct(pos, 16, 16), binding, &mRenderer, &mEditor, isSelectable), //
          mTooltipRenderer(tooltipCaption),                                                                      //
          mValueRenderer(range),                                                                                 //
          mRenderer(&mValueRenderer, &mTooltipRenderer),                                                         //
          mEditor(range)
    {
    }
};

} // namespace clarinoid
