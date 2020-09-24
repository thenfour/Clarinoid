
#pragma once


template<typename T>
struct Property
{
  T mOwnValue;
  T* mRefBinding = &mOwnValue;
  typename cc::function<T(void*)>::ptr_t mGetter = nullptr;
  typename cc::function<void(void*, const T&)>::ptr_t mSetter = nullptr;
  typename cc::function<void(void*, const T& oldVal, const T& newVal)>::ptr_t mOnChange = nullptr;
  void* mpCapture = nullptr;

  // copy
  Property(const Property<T>& rhs) :
    mOwnValue(rhs.mOwnValue),
    mRefBinding(rhs.mRefBinding == &rhs.mOwnValue ? &mOwnValue : rhs.mRefBinding),
    mGetter(rhs.mGetter),
    mSetter(rhs.mSetter),
    mOnChange(rhs.mOnChange),
    mpCapture(rhs.mpCapture)
  {
  }

  Property(Property<T>&& rhs) :
    mOwnValue(rhs.mOwnValue),
    mRefBinding(rhs.mRefBinding == &rhs.mOwnValue ? &mOwnValue : rhs.mRefBinding),
    mGetter(std::move(rhs.mGetter)),
    mSetter(std::move(rhs.mSetter)),
    mOnChange(std::move(rhs.mOnChange)),
    mpCapture(rhs.mpCapture)
  {
  }

  Property<T>& operator =(const Property<T>&) = delete;
  Property<T>& operator =(Property<T>&&) = delete;
  
  Property(typename cc::function<T(void*)>::ptr_t getter, typename cc::function<void(void*, const T&)>::ptr_t setter, void* capture) :
    mGetter(getter),
    mSetter(setter),
    mpCapture(capture)
  {}

  Property(T& binding) :
    mRefBinding(&binding)
  {
  }

  Property(typename cc::function<void(void* capture, const T& oldVal, const T& newVal)>::ptr_t onChange, void* capture) :
    mOnChange(onChange),
    mpCapture(capture)
  {
  }
  Property() :
    mRefBinding(&mOwnValue)
  {
  }
  T GetValue() const
  {
    if (mGetter) {
      return mGetter(mpCapture);
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
      mSetter(mpCapture, val);
    }
    if (oldVal != val && mOnChange) {
      mOnChange(mpCapture, oldVal, val);
    }
  }
};

template<typename Tprop, typename Tval>
Property<Tprop> MakePropertyByCasting(Tval* x) {
  static auto getter = [](void* capture)
  {
    Tval* px = (Tval*)capture;
    return static_cast<Tprop>(*px); 
  };

  static auto setter = [](void* capture, const Tprop& val)
  {
    Tval* px = (Tval*)capture;
    *px = static_cast<Tval>(val);
  };
 
  return Property<Tprop>(getter, setter, x);
}

