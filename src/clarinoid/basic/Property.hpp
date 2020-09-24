
#pragma once

#pragma pack(push,4)

template<typename T>
struct Property
{
  // there are a few variations of property, and to reduce memory usage we union some properties.
  //                 CHANGE+OWNVALUE          REF simple            REF+getters+setters
  // Ownvalue             x
  // Ref                                           x
  // getter                                                                 x
  // setter                                                                 x
  // onchange             x
  // capture              x                                                 x
  //
  // this shows which items can be "unionized".

  T mOwnValue;
  T* mRefBinding = nullptr;
  typename cc::function<T(void*)>::ptr_t mGetter = nullptr;
  union {
    typename cc::function<void(void*, const T&)>::ptr_t mSetter;
    typename cc::function<void(void*, const T& oldVal, const T& newVal)>::ptr_t mOnChange;
  };
  void* mpCapture = nullptr;

  // copy
  Property(const Property<T>& rhs)= default;
  Property(Property<T>&& rhs) = default;
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
    if (mGetter && mSetter) {
      mSetter(mpCapture, val);
    }
    if (!mGetter && (oldVal != val) && mOnChange) {
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

#pragma pack(pop)

static constexpr size_t aoeu3 = sizeof(Property<int>);

