

#pragma once

#include "GuiControlBase.hpp"
#include "GuiControlMisc.hpp"

namespace clarinoid
{

// ---------------------------------------------------------------------------------------
struct GuiBreathMinRenderer : IGuiRenderer<float>
{
    virtual void IGuiRenderer_Render(IGuiControl &ctrl,
                                     const float &val,
                                     bool dblVal,
                                     bool isSelected,
                                     bool isEditing,
                                     DisplayApp &app) override
    {
        app.mDisplay.fillRect(ctrl.mBounds.x, ctrl.mBounds.y, 5, 5, SSD1306_WHITE);
    }
};

// ---------------------------------------------------------------------------------------
struct GuiBreathMinControl : GuiCompositeControl<float>
{
    using T = float;
    GuiLabelValueTooltipRenderer<T> mTooltipRenderer;
    GuiBreathMinRenderer mValueRenderer;
    GuiRendererCombiner<T> mRenderer;
    GuiNumericEditor<T> mEditor;

    GuiBreathMinControl(int page,
                        PointI pos,
                        const NumericEditRangeSpec<T> &range,
                        const String &tooltipCaption,
                        const Property<T> &binding,
                        const Property<bool> &isSelectable)
        : GuiCompositeControl(page,
                              RectI::Construct(pos, 5, 5),
                              binding,
                              NullBoolBinding,
                              &mRenderer,
                              &mEditor,
                              isSelectable),             //
          mTooltipRenderer(tooltipCaption, [](void* cap, const float& val) {
              auto ret = String(val, 4);
              return ret;
          }, this),              //
          mRenderer(&mValueRenderer, &mTooltipRenderer), //
          mEditor(range)
    {
    }
};

} // namespace clarinoid
