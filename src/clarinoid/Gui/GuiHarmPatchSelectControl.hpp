

#pragma once

#include <clarinoid/menu/MenuListControl.hpp>

#include "GuiControlBase.hpp"
#include "GuiControlMisc.hpp"

namespace clarinoid
{

// ---------------------------------------------------------------------------------------
struct GuiHarmPatchAsTextRenderer : IGuiRenderer<int>
{
    virtual void IGuiRenderer_Render(IGuiControl &ctrl,
                                     const int &val,
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
struct GuiHarmPatchSelectorEditor : IGuiEditor<int>
{
    int mOldVal;
    Property<int> mBinding;
    ListControl2 mListControl;

    GuiHarmPatchSelectorEditor(const Property<int> &binding) : mBinding(binding)
    {
    }

    virtual bool IGuiEditor_StartEditing(IGuiControl &ctrl, Property<int> &binding, DisplayApp &app) override
    {
        mListControl.OnShow();
        mOldVal = mBinding.GetValue();
        return true;
    }

    virtual void IGuiEditor_StopEditing(IGuiControl &ctrl,
                                        Property<int> &binding,
                                        DisplayApp &app,
                                        bool wasCancelled) override
    {
        if (wasCancelled)
            mBinding.SetValue(mOldVal);
    }

    virtual void IGuiEditor_Update(IGuiControl &ctrl,
                                   Property<int> &binding,
                                   bool isSelected,
                                   bool isEditing,
                                   DisplayApp &app) override
    {
        if (!isEditing)
            return;
        mListControl.Update(&app.mInput->mMenuScrollA, HARM_PRESET_COUNT, mBinding);
    }

    virtual void IGuiEditor_Render(IGuiControl &ctrl,
                                   Property<int> &binding,
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
            [](void *cap, int li) {
                auto *pApp = (DisplayApp *)cap;
                return pApp->mAppSettings->GetHarmPatchName(li);
            },
            &app);
    }
};

// ---------------------------------------------------------------------------------------
struct GuiHarmPatchSelectControl : GuiCompositeControl<int>
{
    using T = int;
    GuiHarmPatchSelectorEditor mEditor;
    GuiStaticTooltipRenderer<T> mTooltipRenderer;
    GuiHarmPatchAsTextRenderer mValueRenderer;
    GuiRendererCombiner<T> mRenderer;

    GuiHarmPatchSelectControl(int page,
                              RectI bounds,
                              const String &tooltipCaption,
                              const Property<T> &binding,
                              const Property<bool> &isSelectable)
        : GuiCompositeControl(page, bounds, binding, &mRenderer, &mEditor, isSelectable), //
          mEditor(binding),                                                               //
          mTooltipRenderer(tooltipCaption),                                               //
          mValueRenderer(),                                                               //
          mRenderer(&mValueRenderer, &mTooltipRenderer)
    {
    }
};

} // namespace clarinoid
