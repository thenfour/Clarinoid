

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
    }

    void Render(CCDisplay *mDisplay, int x, int y, int mVisibleItems)
    {
        auto count = mpList->List_GetItemCount();
        if (count == 0)
            return;
        mDisplay->mDisplay.setTextSize(1);
        mDisplay->mDisplay.setTextWrap(false);
        int itemToRender = RotateIntoRange(mSelectedItem.GetValue() - 1, count);
        const int itemsToRender = min(mVisibleItems, count);
        for (int i = 0; i < itemsToRender; ++i)
        {
            mDisplay->DrawInvertedLine(mpList->List_GetItemCaption(itemToRender), itemToRender == mSelectedItem.GetValue());
            itemToRender = RotateIntoRange(itemToRender + 1, count);
            if (itemToRender == (mpList->List_GetItemCount() - 1)) {
                auto cursorY = mDisplay->mDisplay.getCursorY();
                int separatorY = cursorY + mDisplay->mDisplay.GetLineHeight() - 1;
                mDisplay->mDisplay.drawFastHLine(0, separatorY, mDisplay->mDisplay.width(), SSD1306_INVERSE);
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

} // namespace clarinoid
