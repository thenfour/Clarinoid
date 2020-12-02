
#pragma once

namespace clarinoid
{

struct IList {
  virtual int List_GetItemCount() const = 0;
  virtual String List_GetItemCaption(int i) const = 0;
};


template<typename T>
struct EnumItemInfo
{
  T mValue;
  const char *mName;
};

// THIS REQUIRES THAT YOUR VALUES ARE SEQUENTIAL AND THE SAME AS LIST INDICES. Runtime checks performed.
template<typename T>
struct EnumInfo : IList {
  const size_t mItemCount;
  const EnumItemInfo<T>* mItems;
  const char *mTypeName;
  
  template<size_t N>
  EnumInfo(const char *typeName, const EnumItemInfo<T>(&enumItems)[N]) :
    mItemCount(N),
    mItems(enumItems),
    mTypeName(typeName)
  {
    for (size_t i = 0; i < N; ++ i) {
      if (static_cast<size_t>(mItems[i].mValue) != i) {
        CCDIE(String("enum<") + mTypeName + "> bad item value");
      }
    }
  }

  int List_GetItemCount() const { return mItemCount; }
  
  String List_GetItemCaption(int n) const
  {
    CCASSERT(n >= 0 && n < (int)mItemCount);
    return String(mItems[n].mName);
  }
  
  const EnumItemInfo<T>* GetItem(size_t n) {
    CCASSERT(n >0 && n <mItemCount);
    return &mItems[n];
  }

  const char *GetValueString(const T& val) const {
    return mItems[static_cast<size_t>(val)].mName;
  }

  T AddRotate(const T& val, int n) const {
    int y = static_cast<int>(val) + n;
    return static_cast<T>(RotateIntoRange(y, (int)mItemCount));
  }

  int ToInt(const T& val) const { return static_cast<int>(val); }
  T ToValue(const int& val) const { return  static_cast<T>(val); }
};

} // namespace clarinoid
