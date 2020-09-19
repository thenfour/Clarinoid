#pragma once


//////////////////////////////////////////////////////////////////////
struct ListControl
{
  const IList* mpList;
  Property<int> mSelectedItem;
  int mX;
  int mY;
  int mMaxItemsToRender;

  ListControl(const IList* list, Property<int> selectedItemBinding, int x, int y, int nVisibleItems) : 
    mpList(list),
    mSelectedItem(selectedItemBinding),
    mX(x),
    mY(y),
    mMaxItemsToRender(nVisibleItems)
  {
//    CCASSERT(!!selectedItemBinding.mGetter);
//    CCASSERT(!!selectedItemBinding.mSetter);
//    CCASSERT(!!mSelectedItem.mGetter);
//    CCASSERT(!!mSelectedItem.mSetter);
  }
  
  void Render()
  {
    auto count = mpList->List_GetItemCount();
    if (count == 0) return;
    gDisplay.mDisplay.setTextSize(1);
    //gDisplay.mDisplay.setCursor(0, 0);
    gDisplay.mDisplay.setTextWrap(false);
    int itemToRender = RotateIntoRange(mSelectedItem.GetValue() - 1, count);
    const int itemsToRender = min(mMaxItemsToRender, count);
    for (int i = 0; i < itemsToRender; ++ i) {
      if (itemToRender == mSelectedItem.GetValue()) {
        gDisplay.mDisplay.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
      } else {
        gDisplay.mDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
      }

      gDisplay.mDisplay.println(mpList->List_GetItemCaption(itemToRender));

      itemToRender = RotateIntoRange(itemToRender + 1, count);
    }
  }
  
  virtual void Update()
  {
    CCASSERT(mpList);
    auto c = mpList->List_GetItemCount();
    if (c == 0) return;
    //auto v = mSelectedItem.GetValue();
    mSelectedItem.SetValue(AddConstrained(mSelectedItem.GetValue(), gEnc.GetIntDelta(), 0, mpList->List_GetItemCount() - 1));
  }
};


