

#pragma once

#include <clarinoid/menu/MenuListControl.hpp>

#include "GuiControlBase.hpp"
#include "GuiControlMisc.hpp"

namespace clarinoid
{

// ---------------------------------------------------------------------------------------
struct GuiSynthPatchAsTextRenderer : IGuiRenderer<int>
{
    virtual void IGuiRenderer_Render(IGuiControl &ctrl,
                                     const int &val,
                                     bool isSelected,
                                     bool isEditing,
                                     DisplayApp &app) override
    {
        app.mDisplay.SelectEightiesFont();
        app.mDisplay.mDisplay.SetClipRect(ctrl.mBounds.x, ctrl.mBounds.y, ctrl.mBounds.right(), ctrl.mBounds.bottom());
        app.mDisplay.mDisplay.print(app.mAppSettings->GetSynthPatchName(val));
    }
};

// ---------------------------------------------------------------------------------------
struct GuiSynthPatchSelectorEditor : IGuiEditor<int>
{
    int mOldVal;
    const bool mIsNullable;
    Property<int> mBinding;

    // list index does not always correspond to the external binding. EG: NULL synth
    // patch is -1, but -1 is not a valid list index.
    Property<int> mListIndex;

    ListControl2 mListControl;
    DisplayApp *mpApp = nullptr; // dirty holder for state during lambda call

    int PatchIndexToListIndex(int pi)
    {
        return pi + (mIsNullable ? 1 : 0);
    }

    int ListIndexToPatchIndex(int li)
    {
        return li - (mIsNullable ? 1 : 0);
    }

    GuiSynthPatchSelectorEditor(bool isNullable, const Property<int> &binding)
        : mIsNullable(isNullable), mBinding(binding)
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
        mListControl.Update(&app.mInput->mMenuScrollA, SYNTH_PRESET_COUNT + (mIsNullable ? 1 : 0), mListIndex);
        mBinding.SetValue(ListIndexToPatchIndex(mListIndex.GetValue()));
    }

    virtual void IGuiEditor_Render(IGuiControl &ctrl,
                                   Property<int> &binding,
                                   bool isSelected,
                                   bool isEditing,
                                   DisplayApp &app) override
    {
        if (!isEditing)
            return;
        mListIndex.SetValue(PatchIndexToListIndex(mBinding.GetValue()));
        auto rc = app.mDisplay.SetupModal();
        mpApp = &app;
        mListControl.Render(
            &app.mDisplay,
            rc,
            SYNTH_PRESET_COUNT + (mIsNullable ? 1 : 0),
            mListIndex,
            [](void *cap, int li) {
                auto *pThis = (GuiSynthPatchSelectorEditor *)cap;
                int pi = pThis->ListIndexToPatchIndex(li);
                //Serial.println(String("li=") + li + " -> pi=" + pi);
                return pThis->mpApp->mAppSettings->GetSynthPatchName(pi);
            },
            this);
    }
};

// ---------------------------------------------------------------------------------------
struct GuiSynthPatchSelectControl : GuiCompositeControl<int>
{
    using T = int;
    GuiSynthPatchSelectorEditor mEditor;
    GuiStaticTooltipRenderer<T> mTooltipRenderer;
    GuiSynthPatchAsTextRenderer mValueRenderer;
    GuiRendererCombiner<T> mRenderer;

    GuiSynthPatchSelectControl(int page,
                               bool nullable,
                               RectI bounds,
                               const String &tooltipCaption,
                               const Property<T> &binding,
                               const Property<bool> &isSelectable)
        : GuiCompositeControl(page, bounds, binding, &mRenderer, &mEditor, isSelectable), //
          mEditor(nullable, binding),                                                     //
          mTooltipRenderer(tooltipCaption),                                               //
          mValueRenderer(),                                                               //
          mRenderer(&mValueRenderer, &mTooltipRenderer)
    {
    }
};

} // namespace clarinoid
