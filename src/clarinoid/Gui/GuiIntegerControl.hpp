

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
        : GuiCompositeControl<T>(page, bounds, binding, NullBoolBinding, &mRenderer, &mEditor, isSelectable),
          mTooltipRenderer(tooltipCaption), mRenderer(&mValueRenderer, &mTooltipRenderer), mEditor(range)
    {
    }
};

// contains a special feature that if you commit without changing bpm, then we call a fn.
// that fn is designed to toggle metronome on/off.
struct GuiTempoControl : GuiCompositeControl<float>
{
    using T = float;
    GuiLabelValueTooltipRenderer<T> mTooltipRenderer;
    GuiValueAsTextRenderer<T> mValueRenderer;
    GuiRendererCombiner<T> mRenderer;
    GuiNumericEditor<T> mEditor;

    GuiTempoControl(int page,
                    RectI bounds,
                    const NumericEditRangeSpec<T> &range,
                    const String &tooltipCaption,
                    const Property<T> &binding,
                    const Property<bool> &dblBinding,
                    const Property<bool> &isSelectable)
        : GuiCompositeControl<T>(page, bounds, binding, dblBinding, &mRenderer, &mEditor, isSelectable),
          mTooltipRenderer(
              tooltipCaption,
              [](void *cap, const float &bpm) { return String(String(CHARSTR_QEQ) + FloatRoundToInt(bpm)); },
              this), //
          mValueRenderer([](void *cap, const float &bpm) { return String(String(CHARSTR_QEQ) + FloatRoundToInt(bpm)); },
                         this),                          //
          mRenderer(&mValueRenderer, &mTooltipRenderer), //
          mEditor(range)
    {
    }
};

template <typename T>
struct GuiTransposeControl : GuiCompositeControl<T>
{
    GuiLabelValueTooltipRenderer<T> mTooltipRenderer;
    GuiValueAsTextRenderer<T> mValueRenderer;
    GuiRendererCombiner<T> mRenderer;
    GuiNumericEditor<T> mEditor;

    GuiTransposeControl(int page,
                        RectI bounds,
                        const NumericEditRangeSpec<T> &range,
                        const String &tooltipCaption,
                        const Property<T> &binding,
                        const Property<bool> &dblBinding,
                        const Property<bool> &isSelectable)
        : GuiCompositeControl<T>(page, bounds, binding, dblBinding, &mRenderer, &mEditor, isSelectable),
          mTooltipRenderer(
              tooltipCaption,
              [](void *cap, const T &val) -> String { return String("t") + GetSignStr(val) + val; },
              this), //
          mValueRenderer([](void *cap, const T &val) -> String { return String("t") + GetSignStr(val) + val; },
                         this),                          //
          mRenderer(&mValueRenderer, &mTooltipRenderer), //
          mEditor(range)
    {
    }
};

} // namespace clarinoid
