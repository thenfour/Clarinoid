
#pragma once

namespace clarinoid
{

struct IList
{
    virtual int List_GetItemCount() const = 0;
    virtual String List_GetItemDisplayName(int i) const = 0;
};

struct GenericEnumItemInfo
{
    GenericEnumItemInfo(int intval, const char *displayName, const char *shortName)
        :                            //
          mDisplayName(displayName), //
          mShortName(shortName), mIntValue(intval)
    {
    }
    const char *mDisplayName;
    const char *mShortName;
    int mIntValue;
};

template <typename T>
struct EnumItemInfo : GenericEnumItemInfo
{
    EnumItemInfo(T val, const char *displayName, const char *shortName) : GenericEnumItemInfo((int)val, displayName, shortName), mValue(val)
    {
    }
    EnumItemInfo(T val, const char *displayName) : GenericEnumItemInfo((int)val, displayName, displayName), mValue(val)
    {
    }
    T mValue;
};

struct GenericEnumInfo : IList
{
    const size_t mItemCount;
    const char *mTypeName;

    GenericEnumInfo(size_t count, const char *typeName) : mItemCount(count), mTypeName(typeName)
    {
    }

    virtual const GenericEnumItemInfo &GenericGetItem(size_t n) const = 0;

    int List_GetItemCount() const
    {
        return (int)mItemCount;
    }

    String List_GetItemDisplayName(int n) const
    {
        CCASSERT(n >= 0 && n < (int)mItemCount);
        return String(GenericGetItem(n).mDisplayName);
    }
};

// THIS REQUIRES THAT YOUR VALUES ARE SEQUENTIAL AND THE SAME AS LIST INDICES. Runtime checks performed.
template <typename T>
struct EnumInfo : /*GenericEnumItemInfo,*/ GenericEnumInfo
{
    const EnumItemInfo<T> *mItems;
    const size_t mCount;

    size_t count() const
    {
        return mCount;
    }

    template <size_t N>
    EnumInfo(const char *typeName, const EnumItemInfo<T> (&enumItems)[N])
        : GenericEnumInfo(N, typeName), mItems(enumItems), mCount(N)
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (static_cast<size_t>(mItems[i].mValue) != i)
            {
                CCASSERT2(!"bad item value", typeName);
            }
        }
    }

    const EnumItemInfo<T> &GetItem(T tn) const
    {
        size_t n = (size_t)tn;
        // this is bloaty.
        //if (n >= this->mItemCount)
        //{
        //    CCDIE(String("EnumItemInfo::GetItem where n(") + n + ") >= itemCount(" + this->mItemCount + ")");
        //    CCASSERT(n > 0 && n < this->mItemCount);
        //}
        return mItems[n];
    }

    const char *GetValueDisplayName(const T &val) const
    {
        return GetItem(val).mDisplayName;
    }

    const char *GetValueShortName(const T &val) const
    {
        return GetItem(val).mShortName;
    }

    Result ValueForShortName(const String& shortName, T &mValue) const
    {
        for (size_t i = 0; i < mItemCount; ++ i)
        {
            auto item = mItems[i];
            if (shortName == item.mShortName) {
                mValue = item.mValue;
                return Result::Success();
            }
        }
        return Result::Failure(String("val:") + shortName + " not found in " + mTypeName);
    }

    T AddRotate(const T &val, int n) const
    {
        int y = static_cast<int>(val) + n;
        return static_cast<T>(RotateIntoRange(y, (int)this->mItemCount));
    }

    int ToInt(const T &val) const
    {
        return static_cast<int>(val);
    }
    T ToValue(const int &val) const
    {
        return static_cast<T>(val);
    }

    virtual const GenericEnumItemInfo &GenericGetItem(size_t n) const override
    {
        return GetItem((T)n);
    }
};

} // namespace clarinoid
