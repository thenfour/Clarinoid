

#pragma once

#include "GuiControlBase.hpp"
#include "GuiControlMisc.hpp"

namespace clarinoid
{

// integer parameter, string formatting,
// mini string-based editor.
template <typename T>
struct GuiIntegerTextControl : GuiCompositeControl<T>
{
    GuiLabelValueTooltipRenderer<T> mTooltipRenderer;
    GuiValueAsTextRenderer<T> mValueRenderer;
    GuiRendererCombiner<T> mRenderer;
    GuiNumericEditor<T> mEditor;

    GuiIntegerTextControl(int page,
                          RectI bounds,
                          const NumericEditRangeSpec<T> &range,
                          const String &tooltipCaption,
                          const Property<T> &binding,
                          const Property<bool> &isSelectable)
        : GuiCompositeControl<T>(page, bounds, binding, &mRenderer, &mEditor, isSelectable),
          mTooltipRenderer(tooltipCaption), mRenderer(&mValueRenderer, &mTooltipRenderer), mEditor(range)
    {
    }
};

} // namespace clarinoid
