

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

    virtual void IGuiControl_Render(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) = 0;
    virtual void IGuiControl_Update(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) = 0;

    int mPage = 0;
    RectI mBounds;
    bool mIsSelectable = true;
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
    String mText;

    GuiLabelControl(int page, bool isSelectable, RectI bounds, const String &s) : mText(s)
    {
        IGuiControl::mPage = page;
        IGuiControl::mBounds = bounds;
        IGuiControl::mIsSelectable = isSelectable;
    }
    virtual void IGuiControl_Render(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) override
    {
        display.mDisplay.setCursor(mBounds.x, mBounds.y);
        display.SetClipRect(mBounds);
        display.mDisplay.print(mText);
    }
    virtual void IGuiControl_Update(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) override
    {
    }
};

// For the little one-line editor on the top/bottom of the screen, this function
// sets up the mini editor state so the caller can just print the text or render or whatever it needs to do.
// returns true if the caller should proceed drawing the editor.
static bool GuiInitiateMiniEditor(bool isEditing, DisplayApp &app)
{
    if (!isEditing)
        return false;
    app.mDisplay.ClearState();
    app.mDisplay.mDisplay.fillRect(0,
                                   app.mDisplay.GetClientHeight() - app.mDisplay.mDisplay.GetLineHeight(),
                                   app.mDisplay.mDisplay.width(),
                                   app.mDisplay.mDisplay.GetLineHeight(),
                                   SSD1306_WHITE);
    app.mDisplay.mDisplay.setCursor(1, app.mDisplay.GetClientHeight() - app.mDisplay.mDisplay.GetLineHeight());
    app.mDisplay.mDisplay.setTextColor(SSD1306_INVERSE);
    return true;
}

// ---------------------------------------------------------------------------------------
// integer parameter, string formatting,
// mini string-based editor.
struct GuiIntegerTextControl : IGuiControl
{
    NumericEditRangeSpec<int> mRange;
    cc::function<String(void *, const int &)>::ptr_t mValueFormatter;
    cc::function<String(void *, const int &)>::ptr_t mValueFormatterForEdit;
    Property<int> mValue;
    Property<bool> mIsSelectable;
    void *mCapture;

    int mEditingOriginalVal = 0;

    GuiIntegerTextControl(int page,
                          RectI bounds,
                          const NumericEditRangeSpec<int> &range,
                          cc::function<String(void *, const int &)>::ptr_t valueFormatter,
                          cc::function<String(void *, const int &)>::ptr_t valueFormatterForEdit,
                          const Property<int> &value,
                          const Property<bool> &isSelectable,
                          void *capture)
        : IGuiControl(page, bounds), mRange(range), mValueFormatter(valueFormatter),
          mValueFormatterForEdit(valueFormatterForEdit), mValue(value), mIsSelectable(isSelectable), mCapture(capture)
    {
    }
    virtual void IGuiControl_Render(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) override
    {
        if (GuiInitiateMiniEditor(isEditing, app))
        {
            display.mDisplay.print(mValueFormatterForEdit(mCapture, mValue.GetValue()));
        }
        display.ClearState();
        display.mDisplay.setCursor(mBounds.x, mBounds.y);
        display.SetClipRect(mBounds);
        display.mDisplay.print(mValueFormatter(mCapture, mValue.GetValue()));
    }
    virtual bool IGuiControl_EditBegin(DisplayApp &app) override
    {
        mEditingOriginalVal = mValue.GetValue();
        return true;
    }
    virtual void IGuiControl_EditEnd(DisplayApp &app, bool wasCancelled) override
    {
        if (wasCancelled)
        {
            mValue.SetValue(mEditingOriginalVal);
        }
    }
    virtual void IGuiControl_Update(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) override
    {
        if (isEditing)
        {
            int oldVal = mValue.GetValue();
            int newVal = mRange.AdjustValue(oldVal,
                                            app.mEnc.GetIntDelta(),
                                            app.mInput->mModifierCourse.CurrentValue(),
                                            app.mInput->mModifierFine.CurrentValue());
            mValue.SetValue(newVal);
        }
    }
};

// ---------------------------------------------------------------------------------------
struct GuiKnobControl : IGuiControl
{
    NumericEditRangeSpec<float> mRange;
    cc::function<String(void *, const float &)>::ptr_t mValueFormatterForEdit;
    Property<float> mValue;
    Property<bool> mIsSelectable;
    void *mCapture;
    // editing label placement (top/bottom)

    float mEditingOriginalVal = 0;

    GuiKnobControl(int page,
                   RectI bounds,
                   const NumericEditRangeSpec<float> &range,
                   cc::function<String(void *, const float &)>::ptr_t valueFormatterForEdit,
                   const Property<float> &value,
                   const Property<bool> &isSelectable,
                   void *capture)
        : IGuiControl(page, bounds), mRange(range), mValueFormatterForEdit(valueFormatterForEdit), mValue(value),
          mIsSelectable(isSelectable), mCapture(capture)
    {
    }
    virtual void IGuiControl_Render(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) override
    {
        if (GuiInitiateMiniEditor(isEditing, app))
        {
            display.mDisplay.print(mValueFormatterForEdit(mCapture, mValue.GetValue()));
        }

        display.ClearState();
        display.DrawBitmap(mBounds.UpperLeft(), KnobDetails::gKnobOutlineSpec);
        float val = mValue.GetValue();
        float a0 = mRange.remap(val, KnobDetails::MinAngle, KnobDetails::MaxAngle);
        float a1 = mRange.remap(0.0f, KnobDetails::MinAngle, KnobDetails::MaxAngle);
        display.fillPie(KnobDetails::Origin.Add(mBounds.UpperLeft()), KnobDetails::Radius, a0, a1 - a0);
    }
    virtual bool IGuiControl_EditBegin(DisplayApp &app) override
    {
        mEditingOriginalVal = mValue.GetValue();
        return true;
    }
    virtual void IGuiControl_EditEnd(DisplayApp &app, bool wasCancelled) override
    {
        if (wasCancelled)
        {
            mValue.SetValue(mEditingOriginalVal);
        }
    }
    virtual void IGuiControl_Update(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) override
    {
        if (isEditing)
        {
            float oldVal = mValue.GetValue();
            float newVal = mRange.AdjustValue(oldVal,
                                              app.mEnc.GetIntDelta(),
                                              app.mInput->mModifierCourse.CurrentValue(),
                                              app.mInput->mModifierFine.CurrentValue());
            mValue.SetValue(newVal);
        }
    }
};

// ---------------------------------------------------------------------------------------
// - supports -inf db
// - the zero point is always at the top, when the range can go positive.
//   - when the range max is 0, then 0 is full right
struct GuiKnobGainControl : IGuiControl
{
    NumericEditRangeSpecWithBottom mRange;
    cc::function<String(void *, const float &)>::ptr_t mValueFormatterForEdit;
    Property<float> mValue;
    Property<bool> mIsSelectable;
    void *mCapture;
    // editing label placement (top/bottom)

    float mEditingOriginalVal = 0;

    GuiKnobGainControl(int page,
                       RectI bounds,
                       const NumericEditRangeSpecWithBottom &range,
                       cc::function<String(void *, const float &)>::ptr_t valueFormatterForEdit,
                       const Property<float> &value,
                       const Property<bool> &isSelectable,
                       void *capture)
        : IGuiControl(page, bounds), mRange(range), mValueFormatterForEdit(valueFormatterForEdit), mValue(value),
          mIsSelectable(isSelectable), mCapture(capture)
    {
    }
    virtual void IGuiControl_Render(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) override
    {
        if (GuiInitiateMiniEditor(isEditing, app))
        {
            display.mDisplay.print(mValueFormatterForEdit(mCapture, mValue.GetValue()));
        }

        display.ClearState();

        float val = mValue.GetValue();
        float aCenter = KnobDetails::CenterAngle; // Remap01ToRange(0.5f, KnobDetails::MinAngle, KnobDetails::MaxAngle);
        if (mRange.IsBottom(val))
        {
            // -inf db,
            display.DrawBitmap(mBounds.UpperLeft(), KnobDetails::gKnobOutlineMutedSpec);
        }
        else
        {
            display.DrawBitmap(mBounds.UpperLeft(), KnobDetails::gKnobOutlineSpec);
            if (mRange.mRangeMax >= 0) // user can GAIN in addition to attenuation; put zero point at the top.
            {
                // negative & positive poles are different scales
                float a0;
                if (val < 0)
                {
                    float negPos = RemapTo01(val, mRange.mRangeMin, 0.0f);
                    a0 = Remap01ToRange(negPos, KnobDetails::MinAngle, aCenter);
                }
                else
                {
                    float posPos = RemapTo01(val, 0.0f, mRange.mRangeMax);
                    a0 = Remap01ToRange(posPos, aCenter, KnobDetails::MaxAngle);
                }
                display.fillPie(KnobDetails::Origin.Add(mBounds.UpperLeft()), KnobDetails::Radius, a0, aCenter - a0);
            }
            else // ONLY attenuation allowed. -inf is full left, max is full right
            {
                float aCenter = Remap01ToRange(0.5f, KnobDetails::MinAngle, KnobDetails::MaxAngle);
                float a0;
                float negPos = RemapTo01(val, mRange.mRangeMin, 0.0f);
                a0 = Remap01ToRange(negPos, KnobDetails::MinAngle, aCenter);
                display.fillPie(KnobDetails::Origin.Add(mBounds.UpperLeft()), KnobDetails::Radius, a0, aCenter - a0);
            }
        }
    }
    virtual bool IGuiControl_EditBegin(DisplayApp &app) override
    {
        mEditingOriginalVal = mValue.GetValue();
        return true;
    }
    virtual void IGuiControl_EditEnd(DisplayApp &app, bool wasCancelled) override
    {
        if (wasCancelled)
        {
            mValue.SetValue(mEditingOriginalVal);
        }
    }
    virtual void IGuiControl_Update(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) override
    {
        if (isEditing)
        {
            float oldVal = mValue.GetValue();
            float newVal = mRange.AdjustValue(oldVal,
                                              app.mEnc.GetIntDelta(),
                                              app.mInput->mModifierCourse.CurrentValue(),
                                              app.mInput->mModifierFine.CurrentValue());
            mValue.SetValue(newVal);
        }
    }
};

// ---------------------------------------------------------------------------------------
struct GuiStereoSpreadControl : IGuiControl
{
    NumericEditRangeSpec<float> mRange;
    cc::function<String(void *, const float &)>::ptr_t mValueFormatterForEdit;
    Property<float> mValue;
    Property<bool> mIsSelectable;
    void *mCapture;

    float mEditingOriginalVal = 0;

    GuiStereoSpreadControl(int page,
                           RectI bounds,
                           const NumericEditRangeSpec<float> &range,
                           cc::function<String(void *, const float &)>::ptr_t valueFormatterForEdit,
                           const Property<float> &value,
                           const Property<bool> &isSelectable,
                           void *capture)
        : IGuiControl(page, bounds), mRange(range), mValueFormatterForEdit(valueFormatterForEdit), mValue(value),
          mIsSelectable(isSelectable), mCapture(capture)
    {
    }
    virtual void IGuiControl_Render(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) override
    {
        if (GuiInitiateMiniEditor(isEditing, app))
        {
            display.mDisplay.print(mValueFormatterForEdit(mCapture, mValue.GetValue()));
        }

        display.ClearState();
        float val = mValue.GetValue();
        auto &spec = GetStereoSpreadBitmapSpec(val);
        display.DrawBitmap(mBounds.UpperLeft(), spec);
    }
    virtual bool IGuiControl_EditBegin(DisplayApp &app) override
    {
        mEditingOriginalVal = mValue.GetValue();
        return true;
    }
    virtual void IGuiControl_EditEnd(DisplayApp &app, bool wasCancelled) override
    {
        if (wasCancelled)
        {
            mValue.SetValue(mEditingOriginalVal);
        }
    }
    virtual void IGuiControl_Update(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) override
    {
        if (isEditing)
        {
            float oldVal = mValue.GetValue();
            float newVal = mRange.AdjustValue(oldVal,
                                              app.mEnc.GetIntDelta(),
                                              app.mInput->mModifierCourse.CurrentValue(),
                                              app.mInput->mModifierFine.CurrentValue());
            mValue.SetValue(newVal);
        }
    }
};

// ---------------------------------------------------------------------------------------
template <typename T>
struct GuiEnumEditor
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

    bool StartEditing(DisplayApp &app)
    {
        mListControl.Init(&mEnumInfo, &app.mInput->mMenuScrollA, mListSelectedItem); //
        mOldVal = mBinding.GetValue();
        return true;
    }

    void StopEditing(DisplayApp &app, bool wasCancelled)
    {
        if (wasCancelled)
        {
            mBinding.SetValue(mOldVal);
        }
    }

    void Update(DisplayApp &app, bool isEditing)
    {
        if (!isEditing)
            return;
        mListControl.Update();
    }

    void Render(DisplayApp &app, bool isEditing)
    {
        if (!isEditing)
            return;
        app.mDisplay.SetupModal();
        int nitems = app.mDisplay.ClippedAreaHeight() / app.mDisplay.mDisplay.GetLineHeight();
        mListControl.Render(&app.mDisplay, 12, 12, nitems + 1);
    }
};

// ---------------------------------------------------------------------------------------
template <typename T>
struct GuiEnumControl : IGuiControl
{
    GuiEnumEditor<T> mEditor;
    typename cc::function<void(void *, const T &, bool, bool, DisplayApp &)>::ptr_t mRenderFn = nullptr;

    Property<T> mBinding;
    Property<bool> mIsSelectable;
    void *mCapture;

    GuiEnumControl(
        int page,
        RectI bounds,
        const EnumInfo<T> &enumInfo,
        const Property<T> &binding,
        typename cc::function<void(void *, const T &, bool isSelected, bool isEditing, DisplayApp &app)>::ptr_t
            renderFn,
        const Property<bool> &isSelectable,
        void *capture)
        : IGuiControl(page, bounds), mEditor(enumInfo, binding), mRenderFn(renderFn), mBinding(binding),
          mIsSelectable(isSelectable), mCapture(capture)
    {
    }
    virtual void IGuiControl_Render(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) override
    {
        display.ClearState();
        display.mDisplay.setCursor(mBounds.x, mBounds.y);
        mRenderFn(mCapture, mBinding.GetValue(), isSelected, isEditing, app);

        mEditor.Render(app, isEditing);
    }
    virtual bool IGuiControl_EditBegin(DisplayApp &app) override
    {
        return mEditor.StartEditing(app);
    }
    virtual void IGuiControl_EditEnd(DisplayApp &app, bool wasCancelled) override
    {
        mEditor.StopEditing(app, wasCancelled);
    }
    virtual void IGuiControl_Update(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) override
    {
        mEditor.Update(app, isEditing);
    }
};

// ---------------------------------------------------------------------------------------
struct GuiBoolControl : IGuiControl
{
    typename cc::function<void(void *, bool val, bool isSelected, bool isEditing, DisplayApp &)>::ptr_t mRenderFn =
        nullptr;

    Property<bool> mBinding;
    Property<bool> mIsSelectable;
    void *mCapture;

    GuiBoolControl(
        int page,
        RectI bounds,
        const Property<bool> &binding,
        typename cc::function<void(void *, bool val, bool isSelected, bool isEditing, DisplayApp &app)>::ptr_t renderFn,
        const Property<bool> &isSelectable,
        void *capture)
        : IGuiControl(page, bounds), mRenderFn(renderFn), mBinding(binding), mIsSelectable(isSelectable),
          mCapture(capture)
    {
    }
    virtual void IGuiControl_Render(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) override
    {
        display.ClearState();
        display.mDisplay.setCursor(mBounds.x, mBounds.y);
        mRenderFn(mCapture, mBinding.GetValue(), isSelected, isEditing, app);
    }
    virtual bool IGuiControl_EditBegin(DisplayApp &app) override
    {
        mBinding.SetValue(!mBinding.GetValue());
        return false;
    }
    virtual void IGuiControl_EditEnd(DisplayApp &app, bool wasCancelled) override
    {
    }
    virtual void IGuiControl_Update(bool isSelected, bool isEditing, DisplayApp &app, CCDisplay &display) override
    {
    }
};

} // namespace clarinoid
