

#pragma once

#include <clarinoid/menu/MenuListControl.hpp>

#include "GuiControlBase.hpp"
#include "GuiControlMisc.hpp"

namespace clarinoid
{

// ---------------------------------------------------------------------------------------
template<typename T>
struct GuiHarmPatchAsTextRenderer : IGuiRenderer<T>
{
    virtual void IGuiRenderer_Render(IGuiControl &ctrl,
                                     const T &val,
                                     bool isSelected,
                                     bool isEditing,
                                     DisplayApp &app) override
    {
        app.mDisplay.SelectEightiesFont();
        app.mDisplay.mDisplay.SetClipRect(ctrl.mBounds.x, ctrl.mBounds.y, ctrl.mBounds.right(), ctrl.mBounds.bottom());
        app.mDisplay.mDisplay.print(app.mAppSettings->GetHarmPatchName(val));
    }
};

// ---------------------------------------------------------------------------------------
// does not support NULL
template<typename T>
struct GuiHarmPatchSelectorEditor : IGuiEditor<T>
{
    T mOldVal;
    Property<T> mBinding;
    ListControl2 mListControl;

    GuiHarmPatchSelectorEditor(const Property<T> &binding) : mBinding(binding)
    {
    }

    virtual bool IGuiEditor_StartEditing(IGuiControl &ctrl, Property<T> &binding, DisplayApp &app) override
    {
        mListControl.OnShow();
        mOldVal = mBinding.GetValue();
        return true;
    }

    virtual void IGuiEditor_StopEditing(IGuiControl &ctrl,
                                        Property<T> &binding,
                                        DisplayApp &app,
                                        bool wasCancelled) override
    {
        if (wasCancelled)
            mBinding.SetValue(mOldVal);
    }

    virtual void IGuiEditor_Update(IGuiControl &ctrl,
                                   Property<T> &binding,
                                   bool isSelected,
                                   bool isEditing,
                                   DisplayApp &app) override
    {
        if (!isEditing)
            return;
        mListControl.Update(&app.mInput->mMenuScrollA, HARM_PRESET_COUNT, mBinding);
    }

    virtual void IGuiEditor_Render(IGuiControl &ctrl,
                                   Property<T> &binding,
                                   bool isSelected,
                                   bool isEditing,
                                   DisplayApp &app) override
    {
        if (!isEditing)
            return;
        auto rc = app.mDisplay.SetupModal();
        mListControl.Render(
            &app.mDisplay,
            rc,
            HARM_PRESET_COUNT,
            mBinding,
            [](void *cap, T li) {
                auto *pApp = (DisplayApp *)cap;
                return pApp->mAppSettings->GetHarmPatchName(li);
            },
            &app);
    }
};

// ---------------------------------------------------------------------------------------
template<typename T>
struct GuiHarmPatchSelectControl : GuiCompositeControl<T>
{
    GuiHarmPatchSelectorEditor<T> mEditor;
    GuiStaticTooltipRenderer<T> mTooltipRenderer;
    GuiHarmPatchAsTextRenderer<T> mValueRenderer;
    GuiRendererCombiner<T> mRenderer;

    GuiHarmPatchSelectControl(int page,
                              RectI bounds,
                              const String &tooltipCaption,
                              const Property<T> &binding,
                              const Property<bool> &isSelectable)
        : GuiCompositeControl<T>(page, bounds, binding, &mRenderer, &mEditor, isSelectable), //
          mEditor(binding),                                                               //
          mTooltipRenderer(tooltipCaption),                                               //
          mValueRenderer(),                                                               //
          mRenderer(&mValueRenderer, &mTooltipRenderer)
    {
    }
};

} // namespace clarinoid
