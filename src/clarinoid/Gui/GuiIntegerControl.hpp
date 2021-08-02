

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

// contains a special feature that if you commit without changing bpm, then we call a fn.
// that fn is designed to toggle metronome on/off.
struct GuiTempoControl : GuiCompositeControl<float>
{
    using T = float;
    GuiLabelValueTooltipRenderer<T> mTooltipRenderer;
    GuiValueAsTextRenderer<T> mValueRenderer;
    GuiRendererCombiner<T> mRenderer;
    GuiNumericEditor<T> mEditor;
    typename cc::function<void(void *, DisplayApp &app)>::ptr_t mOnToggleHandler = nullptr;
    void *mpCapture = nullptr;

    GuiTempoControl(int page,
                    RectI bounds,
                    const NumericEditRangeSpec<T> &range,
                    const String &tooltipCaption,
                    const Property<T> &binding,
                    const Property<bool> &isSelectable,
                    typename cc::function<void(void *, DisplayApp &app)>::ptr_t onToggleHandler = nullptr,
                    void *capture = nullptr)
        : GuiCompositeControl<T>(page, bounds, binding, &mRenderer, &mEditor, isSelectable),
          mTooltipRenderer(
              tooltipCaption,
              [](void *cap, const float &bpm) { return String(String(CHARSTR_QEQ) + (int)std::round(bpm)); },
              this), //
          mValueRenderer([](void *cap, const float &bpm) { return String(String(CHARSTR_QEQ) + (int)std::round(bpm)); },
                         this),                          //
          mRenderer(&mValueRenderer, &mTooltipRenderer), //
          mEditor(range), mOnToggleHandler(onToggleHandler), mpCapture(capture)
    {
    }

    T oldVal;

    virtual bool IGuiControl_EditBegin(DisplayApp &app) override
    {
        oldVal = mBinding.GetValue();
        return GuiCompositeControl<float>::IGuiControl_EditBegin(app);
    }

    virtual void IGuiControl_EditEnd(DisplayApp &app, bool wasCancelled) override
    {
        if (!wasCancelled && mOnToggleHandler)
        {
            // if you enter edit mode with no changes, toggle metronome on.
            if (FloatEquals(oldVal, mBinding.GetValue()))
            {
                mOnToggleHandler(mpCapture, app);
            }
        }
        GuiCompositeControl<float>::IGuiControl_EditEnd(app, wasCancelled);
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
                        const Property<bool> &isSelectable)
        : GuiCompositeControl<T>(page, bounds, binding, &mRenderer, &mEditor, isSelectable),
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
