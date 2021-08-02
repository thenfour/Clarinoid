

#pragma once

namespace clarinoid
{

//////////////////////////////////////////////////////////////////////
struct ListControl
{
    const IList *mpList = nullptr;
    Property<int> mSelectedItem;
    EncoderReader mEnc;
    IEncoder *pEncoder = nullptr;

    void Init(const IList *list, IEncoder *penc, const Property<int> &selectedItemBinding)
    {
        mpList = list;
        mSelectedItem = selectedItemBinding;
        pEncoder = penc;
        mEnc.ClearState(); // important so the next Update() call doesn't incorrectly calculate a delta.
    }

    void Render(IDisplay *mDisplay, int x, int y, int mVisibleItems)
    {
        auto count = mpList->List_GetItemCount();
        if (count == 0)
            return;
        mDisplay->setTextWrap(false);
        int itemToRender = RotateIntoRange(mSelectedItem.GetValue() - 1, count);
        const int itemsToRender = min(mVisibleItems, count);
        for (int i = 0; i < itemsToRender; ++i)
        {
            mDisplay->PrintInvertedLine(mpList->List_GetItemCaption(itemToRender),
                                        itemToRender == mSelectedItem.GetValue());
            itemToRender = RotateIntoRange(itemToRender + 1, count);
            if (itemToRender == (mpList->List_GetItemCount() - 1))
            {
                auto cursorY = mDisplay->getCursorY();
                int separatorY = cursorY + mDisplay->GetLineHeight() - 1;
                mDisplay->drawFastHLine(0, separatorY, mDisplay->width(), SSD1306_INVERSE);
            }
        }
    }

    virtual void Update()
    {
        mEnc.Update(pEncoder);
        CCASSERT(mpList);
        auto c = mpList->List_GetItemCount();
        if (c == 0)
            return;
        mSelectedItem.SetValue(
            AddConstrained(mSelectedItem.GetValue(), mEnc.GetIntDelta(), 0, mpList->List_GetItemCount() - 1));
    }
};

//////////////////////////////////////////////////////////////////////
// similar to above but more things done on the fly rather than holding state.
struct ListControl2
{
    EncoderReader mEnc;

    void OnShow()
    {
        mEnc.ClearState(); // important so the next Update() call doesn't incorrectly calculate a delta.
    }

    template <typename Tindex>
    void Render(IDisplay *mDisplay,
                const RectI &rc,
                int itemCount,
                Property<Tindex> &selectedItem,
                typename cc::function<String(void *, Tindex)>::ptr_t captionGetter,
                void *capture)
    {
        if (itemCount == 0)
            return;

        int mVisibleItems = 1 + (rc.height / mDisplay->GetLineHeight());

        int itemToRender = RotateIntoRange(selectedItem.GetValue() - 1, itemCount);
        mDisplay->ClearState();
        const int itemsToRender = min(mVisibleItems, itemCount);
        mDisplay->SetClipRect(rc.x, rc.y, rc.right(), rc.bottom());
        mDisplay->setCursor(rc.x, rc.y);
        mDisplay->SetTextLeftMargin(rc.x);
        for (int i = 0; i < itemsToRender; ++i)
        {
            mDisplay->PrintInvertedLine(captionGetter(capture, itemToRender), itemToRender == selectedItem.GetValue());
            itemToRender = RotateIntoRange(itemToRender + 1, itemCount);
            if (itemToRender == (itemCount - 1))
            {
                auto cursorY = mDisplay->getCursorY();
                int separatorY = cursorY + mDisplay->GetLineHeight() - 1;
                if (rc.YInRect(separatorY))
                {
                    mDisplay->drawFastHLine(rc.x, separatorY, rc.width, SSD1306_INVERSE);
                }
            }
        }
    }

    template <typename Tindex>
    void Update(IEncoder *pEncoder, int itemCount, Property<Tindex> &selectedItem)
    {
        mEnc.Update(pEncoder);
        if (itemCount == 0)
            return;
        selectedItem.SetValue(AddConstrained((int)selectedItem.GetValue(), mEnc.GetIntDelta(), 0, itemCount - 1));
    }
};

} // namespace clarinoid
