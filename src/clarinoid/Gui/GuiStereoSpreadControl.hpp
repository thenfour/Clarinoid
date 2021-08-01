

#pragma once

#include "GuiControlBase.hpp"
#include "GuiControlMisc.hpp"

namespace clarinoid
{

// ---------------------------------------------------------------------------------------
struct GuiStereoSpreadRenderer : IGuiRenderer<float>
{
    virtual void IGuiRenderer_Render(IGuiControl &ctrl,
                                     const float &val,
                                     bool isSelected,
                                     bool isEditing,
                                     DisplayApp &app) override
    {
        auto &spec = GetStereoSpreadBitmapSpec(val);
        app.mDisplay.DrawBitmap(ctrl.mBounds.UpperLeft(), spec);
    }
};

// ---------------------------------------------------------------------------------------
struct GuiStereoSpreadControl : GuiCompositeControl<float>
{
    using T = float;
    GuiLabelValueTooltipRenderer<T> mTooltipRenderer;
    GuiStereoSpreadRenderer mValueRenderer;
    GuiRendererCombiner<T> mRenderer;
    GuiNumericEditor<T> mEditor;

    GuiStereoSpreadControl(int page,
                           PointI pos,
                           const NumericEditRangeSpec<T> &range,
                           const String &tooltipCaption,
                           const Property<T> &binding,
                           const Property<bool> &isSelectable)
        : GuiCompositeControl(page, RectI::Construct(pos, 17, 7), binding, &mRenderer, &mEditor, isSelectable), //
          mTooltipRenderer(tooltipCaption),                                               //
          mRenderer(&mValueRenderer, &mTooltipRenderer),                                  //
          mEditor(range)
    {
    }
};

} // namespace clarinoid
