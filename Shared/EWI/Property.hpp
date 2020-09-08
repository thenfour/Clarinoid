
#pragma once


template<typename T>
struct Property
{
  T mOwnValue;
  T* mRefBinding = &mOwnValue;
  std::function<T()> mGetter;
  std::function<void(const T&)> mSetter;
  std::function<void(const T& oldVal, const T& newVal)> mOnChange;

  // copy
  Property(const Property<T>& rhs) :
    mOwnValue(rhs.mOwnValue),
    mRefBinding(rhs.mRefBinding == &rhs.mOwnValue ? &mOwnValue : rhs.mRefBinding),
    mGetter(rhs.mGetter),
    mSetter(rhs.mSetter),
    mOnChange(rhs.mOnChange)
  {
  }

  Property(Property<T>&& rhs) :
    mOwnValue(rhs.mOwnValue),
    mRefBinding(rhs.mRefBinding == &rhs.mOwnValue ? &mOwnValue : rhs.mRefBinding),
    mGetter(std::move(rhs.mGetter)),
    mSetter(std::move(rhs.mSetter)),
    mOnChange(std::move(rhs.mOnChange))
  {
  }

  Property<T>& operator =(const Property<T>&) = delete;
  Property<T>& operator =(Property<T>&&) = delete;
  
  Property(std::function<T()> getter, std::function<void(const T&)> setter) :
    mGetter(getter),
    mSetter(setter)
  {}
  Property(T& binding) :
    mRefBinding(&binding)
  {
//    char x[100];
//    sprintf(x, "ref binding to %p", (&binding));
//    Serial.println(x);
  }
  Property(std::function<void(const T& oldVal, const T& newVal)> onChange) :
    mOnChange(onChange)
  {
  }
  Property() :
    mRefBinding(&mOwnValue)
  {
  }
  T GetValue() const
  {
    if (!!mGetter) {
      return mGetter();
    }
    CCASSERT(!!mRefBinding);
    return *mRefBinding;
  }
  void SetValue(const T& val)
  {
    T oldVal = GetValue();
    if (mRefBinding) {
      *mRefBinding = val;
    }
    if (mSetter) {
      mSetter(val);
    }
    if (oldVal != val && mOnChange) {
      mOnChange(oldVal, val);
    }
  }
};

template<typename Tprop, typename Tval>
Property<Tprop> MakePropertyByCasting(Tval& x) {
  return Property<Tprop>(
    [&]() { return static_cast<Tprop>(x); },
    [&](const Tprop& val) { x = static_cast<Tval>(val);
  });
}

