// IGuiControl is what the GuiApp interacts with for navigation, rendering, update.
// i started out writing stuff from scratch, based only on IGuiControl.
// but it became clear that a modular approach is necessary.
// thus,
// IGuiRenderer and IGuiEditor are used by GuiCompositeControl to provide a modular system wrt render, update, edit

#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/menu/MenuAppBase.hpp>

#include "StereoSpreadIcons.hpp"
#include "Bitmaps.hpp"

namespace clarinoid
{

// ---------------------------------------------------------------------------------------
// very much like ISettingItem.
// represents an INSTANCE of a gui control, not a control type.
struct IGuiControl
{
    int mPage = 0;
    RectI mBounds;
    bool mIsSelectable = true;

    IGuiControl()
    {
    }
    explicit IGuiControl(int page, const RectI &bounds) : mPage(page), mBounds(bounds)
    {
    }
    virtual int IGuiControl_GetPage()
    {
        return mPage;
    }
    virtual RectI IGuiControl_GetBounds()
    {
        return mBounds;
    }
    virtual bool IGuiControl_IsSelectable()
    {
        return mIsSelectable;
    }

    virtual void IGuiControl_SelectBegin(DisplayApp &app)
    {
    }
    virtual void IGuiControl_SelectEnd(DisplayApp &app)
    {
    }
    virtual bool IGuiControl_EditBegin(DisplayApp &app) // if editing should be entered, return true.
    {
        return false;
    }
    virtual void IGuiControl_EditEnd(DisplayApp &app, bool wasCancelled)
    {
    }

    virtual void IGuiControl_Render(bool isSelected, bool isEditing, DisplayApp &app, IDisplay &display) = 0;
    virtual void IGuiControl_Update(bool isSelected, bool isEditing, DisplayApp &app, IDisplay &display) = 0;
};

// ---------------------------------------------------------------------------------------
struct GuiControlList // very much like SettingsList. simpler though because we don't bother with multi items.
{
    IGuiControl **mItems;
    size_t mItemRawCount;

    template <size_t N>
    GuiControlList(IGuiControl *(&arr)[N]) : mItems(arr), mItemRawCount(N)
    {
    }

    size_t Count() const
    {
        return mItemRawCount;
    }

    IGuiControl *GetItem(size_t i)
    {
        CCASSERT(i < mItemRawCount);
        return mItems[i];
    }

    int GetPageCount()
    {
        int ret = 1;
        for (size_t i = 0; i < this->Count(); ++i)
        {
            auto *ctrl = this->GetItem(i);
            ret = std::max(ctrl->IGuiControl_GetPage() + 1, ret);
        }
        return ret;
    }
};

// ---------------------------------------------------------------------------------------
struct GuiLabelControl : IGuiControl
{
    bool mClip = false;
    String mText;

    GuiLabelControl(int page, RectI clipBounds, const String &s)
        : mClip(true), //
          mText(s)
    {
        IGuiControl::mPage = page;
        IGuiControl::mBounds = clipBounds;
        IGuiControl::mIsSelectable = false;
    }

    GuiLabelControl(int page, PointI pt, const String &s)
        : mClip(false), //
          mText(s)
    {
        IGuiControl::mPage = page;
        IGuiControl::mBounds =
            RectI::Construct(pt, 1, 1); // since we don't clip text, don't require specifying width/height.
        IGuiControl::mIsSelectable = false;
    }

    virtual void IGuiControl_Render(bool isSelected, bool isEditing, DisplayApp &app, IDisplay &display) override
    {
        display.ClearState();
        display.setTextWrap(false);
        display.setCursor(mBounds.x, mBounds.y);
        if (mClip)
        {
            display.SetClipRect(mBounds);
        }
        display.print(mText);
    }
    virtual void IGuiControl_Update(bool isSelected, bool isEditing, DisplayApp &app, IDisplay &display) override
    {
    }
};

// ---------------------------------------------------------------------------------------
// for rendering the GUI control, tooltip area, etc, this is a common
template <typename T>
struct IGuiRenderer
{
    virtual void IGuiRenderer_Render(IGuiControl &ctrl,
                                     const T &val,
                                     bool isSelected,
                                     bool isEditing,
                                     DisplayApp &app) = 0;
};

// ---------------------------------------------------------------------------------------
template <typename Tparam>
struct IGuiEditor
{
    // return true if the editor should be shown (for example simple toggles don't enter an editing state)
    virtual bool IGuiEditor_StartEditing(IGuiControl &ctrl, Property<Tparam> &binding, DisplayApp &app)
    {
        return false;
    }

    virtual void IGuiEditor_StopEditing(IGuiControl &ctrl,
                                        Property<Tparam> &binding,
                                        DisplayApp &app,
                                        bool wasCancelled)
    {
    }

    // called ALWAYS, not only when editing. so check if editing.
    virtual void IGuiEditor_Update(IGuiControl &ctrl,
                                   Property<Tparam> &binding,
                                   bool isSelected,
                                   bool isEditing,
                                   DisplayApp &app)
    {
    }
    virtual void IGuiEditor_Render(IGuiControl &ctrl,
                                   Property<Tparam> &binding,
                                   bool isSelected,
                                   bool isEditing,
                                   DisplayApp &app)
    {
    }
};

// so these renderers & editors are great, but the problem is that you can't actually INSTANTIATE them in a constructor
// like we want. maybe it's OK; we just instantiate them next to the thing. so instead of
/*

GuiMuteControl mMute = {
    []() { display.println(); }
};

it's

GuiMuteControlRenderer mMuteRenderer = { ... };
GuiMuteControlEditor mMuteEditor = { ... };
GuiParamControl mMute = {
    &mMuteRenderer,
    &mMuteEditor,
    ...
}

and then you can compose these into a single mute control.
struct CompositeMuteCtrl : GuiParamControl
{
    GuiMuteControlRenderer mMuteRenderer = { ... };
    GuiMuteControlEditor mMuteEditor = { ... };
}

*/

// IGuiControl is very generic. This is more specific, with facilities for binding, render, update, selectable,
// editing... but they can all be plugged with your own thing. inherit this class to create specific controls and plug
// other renderers / editors etc.
template <typename Tparam>
struct GuiCompositeControl : IGuiControl
{
    IGuiRenderer<Tparam> *mRenderCtrl = nullptr;
    IGuiEditor<Tparam> *mEditor = nullptr;
    Property<Tparam> mBinding;
    Property<bool> mIsSelectable;

    GuiCompositeControl(int page,
                        RectI bounds,
                        const Property<Tparam> &binding,
                        IGuiRenderer<Tparam> *renderFn,
                        IGuiEditor<Tparam> *editor,
                        const Property<bool> &isSelectable)
        : IGuiControl(page, bounds), mRenderCtrl(renderFn), mEditor(editor), mBinding(binding),
          mIsSelectable(isSelectable)
    {
    }

    virtual void IGuiControl_Render(bool isSelected, bool isEditing, DisplayApp &app, IDisplay &display) override
    {
        display.ClearState();
        display.setCursor(mBounds.x, mBounds.y);
        mRenderCtrl->IGuiRenderer_Render(*this, mBinding.GetValue(), isSelected, isEditing, app);
        mEditor->IGuiEditor_Render(*this, mBinding, isSelected, isEditing, app);
    }
    virtual bool IGuiControl_EditBegin(DisplayApp &app) override
    {
        return mEditor->IGuiEditor_StartEditing(*this, mBinding, app);
    }
    virtual void IGuiControl_EditEnd(DisplayApp &app, bool wasCancelled) override
    {
        mEditor->IGuiEditor_StopEditing(*this, mBinding, app, wasCancelled);
    }
    virtual void IGuiControl_Update(bool isSelected, bool isEditing, DisplayApp &app, IDisplay &display) override
    {
        mEditor->IGuiEditor_Update(*this, mBinding, isSelected, isEditing, app);
    }
};

// ---------------------------------------------------------------------------------------
// allows chaining renderers together
template <typename T>
struct GuiRendererCombiner : IGuiRenderer<T>
{
    IGuiRenderer<T> *mRenderer1 = nullptr;
    IGuiRenderer<T> *mRenderer2 = nullptr;

    GuiRendererCombiner(IGuiRenderer<T> *a, IGuiRenderer<T> *b) : mRenderer1(a), mRenderer2(b)
    {
    }
    virtual void IGuiRenderer_Render(IGuiControl &ctrl, const T &val, bool isSelected, bool isEditing, DisplayApp &app)
    {
        if (mRenderer1)
        {
            mRenderer1->IGuiRenderer_Render(ctrl, val, isSelected, isEditing, app);
        }
        if (mRenderer2)
        {
            mRenderer2->IGuiRenderer_Render(ctrl, val, isSelected, isEditing, app);
        }
    }
};

} // namespace clarinoid
