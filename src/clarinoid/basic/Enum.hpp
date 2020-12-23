
#pragma once

namespace clarinoid
{

  struct IList
  {
    virtual int List_GetItemCount() const = 0;
    virtual String List_GetItemCaption(int i) const = 0;
  };

  struct GenericEnumItemInfo
  {
    GenericEnumItemInfo(int intval, const char *name) : mName(name),
                                                        mIntValue(intval)
    {
    }
    const char *mName;
    int mIntValue;
  };

  template <typename T>
  struct EnumItemInfo : GenericEnumItemInfo
  {
    EnumItemInfo(T val, const char *name) : GenericEnumItemInfo((int)val, name),
                                            mValue(val)
    {
      //
    }
    T mValue;
  };

  struct GenericEnumInfo : IList
  {
    const size_t mItemCount;
    const char *mTypeName;

    GenericEnumInfo(size_t count, const char *name) : mItemCount(count),
                                                      mTypeName(name)
    {
    }

    virtual const GenericEnumItemInfo *GenericGetItem(size_t n) const = 0;

    int List_GetItemCount() const { return mItemCount; }

    String List_GetItemCaption(int n) const
    {
      CCASSERT(n >= 0 && n < (int)mItemCount);
      return String(GenericGetItem(n)->mName);
    }
  };

  // THIS REQUIRES THAT YOUR VALUES ARE SEQUENTIAL AND THE SAME AS LIST INDICES. Runtime checks performed.
  template <typename T>
  struct EnumInfo : /*GenericEnumItemInfo,*/ GenericEnumInfo
  {
    const EnumItemInfo<T> *mItems;

    template <size_t N>
    EnumInfo(const char *typeName, const EnumItemInfo<T> (&enumItems)[N]) : //GenericEnumItemInfo(N, typeName),
                                                                            GenericEnumInfo(N, typeName),
                                                                            mItems(enumItems)
    {
      for (size_t i = 0; i < N; ++i)
      {
        if (static_cast<size_t>(mItems[i].mValue) != i)
        {
          CCDIE(String("enum<") + typeName + "> bad item value");
        }
      }
    }

    const EnumItemInfo<T> *GetItem(size_t n) const
    {
      CCASSERT(n > 0 && n < this->mItemCount);
      return &mItems[n];
    }

    const char *GetValueString(const T &val) const
    {
      return mItems[static_cast<size_t>(val)].mName;
    }

    T AddRotate(const T &val, int n) const
    {
      int y = static_cast<int>(val) + n;
      return static_cast<T>(RotateIntoRange(y, (int)this->mItemCount));
    }

    int ToInt(const T &val) const { return static_cast<int>(val); }
    T ToValue(const int &val) const { return static_cast<T>(val); }

    virtual const GenericEnumItemInfo *GenericGetItem(size_t n) const override
    {
      return GetItem(n);
    }
  };

} // namespace clarinoid
