

#pragma once

#include "GuiControlBase.hpp"
#include "GuiControlMisc.hpp"

namespace clarinoid
{

// ---------------------------------------------------------------------------------------
// just draws the value text
template <typename T>
struct GuiEnumAsTextRenderer : IGuiRenderer<T>
{
    const EnumInfo<T> &mEnumInfo;
    GuiEnumAsTextRenderer(const EnumInfo<T> &enumInfo) : mEnumInfo(enumInfo)
    {
    }
    virtual void IGuiRenderer_Render(IGuiControl &ctrl,
                                     const T &val,
                                     bool isSelected,
                                     bool isEditing,
                                     DisplayApp &app) override
    {
        app.mDisplay.print(mEnumInfo.GetValueString(val));
    }
};

// ---------------------------------------------------------------------------------------
// renders a "Label: n" tooltip for enum values
template <typename T>
struct GuiLabelEnumTooltipRenderer : IGuiRenderer<T>
{
    const EnumInfo<T> &mEnumInfo;
    String mStaticCaption;
    GuiLabelEnumTooltipRenderer(const EnumInfo<T> &enumInfo, const String &staticCaption)
        : mEnumInfo(enumInfo), mStaticCaption(staticCaption)
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
            app.mDisplay.print(mStaticCaption + ": " + mEnumInfo.GetValueString(val));
        }
    }
};

// ---------------------------------------------------------------------------------------
template <typename T>
struct GuiEnumEditor : IGuiEditor<T>
{
    T mOldVal;

    Property<T> mBinding;
    const EnumInfo<T> &mEnumInfo;

    Property<int> mListSelectedItem = {[](void *capture) {
                                           GuiEnumEditor<T> *pthis = (GuiEnumEditor<T> *)capture;
                                           return (int)pthis->mBinding.GetValue();
                                       },
                                       [](void *capture, const int &val) {
                                           GuiEnumEditor<T> *pthis = (GuiEnumEditor<T> *)capture;
                                           pthis->mBinding.SetValue((T)val);
                                       },
                                       this};

    ListControl mListControl;

    GuiEnumEditor(const EnumInfo<T> &enumInfo, const Property<T> &binding) : mBinding(binding), mEnumInfo(enumInfo)
    {
    }

    virtual bool IGuiEditor_StartEditing(IGuiControl &ctrl, Property<T> &binding, DisplayApp &app) override
    {
        mListControl.Init(&mEnumInfo, &app.mInput->mMenuScrollA, mListSelectedItem); //
        mOldVal = mBinding.GetValue();
        return true;
    }

    virtual void IGuiEditor_StopEditing(IGuiControl &ctrl,
                                        Property<T> &binding,
                                        DisplayApp &app,
                                        bool wasCancelled) override
    {
        if (wasCancelled)
        {
            mBinding.SetValue(mOldVal);
        }
    }

    virtual void IGuiEditor_Update(IGuiControl &ctrl,
                                   Property<T> &binding,
                                   bool isSelected,
                                   bool isEditing,
                                   DisplayApp &app) override
    {
        if (!isEditing)
            return;
        mListControl.Update();
    }

    virtual void IGuiEditor_Render(IGuiControl &ctrl,
                                   Property<T> &binding,
                                   bool isSelected,
                                   bool isEditing,
                                   DisplayApp &app) override
    {
        if (!isEditing)
            return;
        app.mDisplay.SetupModal();
        int nitems = app.mDisplay.ClippedAreaHeight() / app.mDisplay.GetLineHeight();
        mListControl.Render(&app.mDisplay, 12, 12, nitems + 1);
    }
};

template <typename T>
struct GuiEnumControl : GuiCompositeControl<T>
{
    GuiEnumEditor<T> mEditor;
    GuiLabelEnumTooltipRenderer<T> mTooltipRenderer;
    GuiEnumAsTextRenderer<T> mValueRenderer;
    GuiRendererCombiner<T> mRenderer;

    GuiEnumControl(int page,
                   RectI bounds,
                   const String &tooltipCaption,
                   const EnumInfo<T> &enumInfo,
                   const Property<T> &binding,
                   const Property<bool> &isSelectable)
        : GuiCompositeControl<T>(page, bounds, binding, &mRenderer, &mEditor, isSelectable), //
          mEditor(enumInfo, binding),                                                        //
          mTooltipRenderer(enumInfo, tooltipCaption),                                        //
          mValueRenderer(enumInfo), mRenderer(&mValueRenderer, &mTooltipRenderer)
    {
    }
};

} // namespace clarinoid
