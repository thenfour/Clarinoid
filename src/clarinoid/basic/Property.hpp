
#pragma once

namespace clarinoid
{

template <typename T>
struct Property
{
    T mOwnValue;
    T *mRefBinding = nullptr;
    typename cc::function<T(void *)>::ptr_t mGetter = nullptr;
    typename cc::function<T &(void *)>::ptr_t mRefGetter = nullptr;
    typename cc::function<void(void *, const T &)>::ptr_t mSetter = nullptr;
    void *mpCapture = nullptr;

    // copy
    Property(const Property<T> &rhs) = default;
    Property(Property<T> &&rhs) = default;
    Property<T> &operator=(const Property<T> &rhs) = default;
    Property<T> &operator=(Property<T> &&) = delete;

    Property(T &binding) : mRefBinding(&binding)
    {
    }

    Property(typename cc::function<T(void *)>::ptr_t getter) : mGetter(getter)
    {
    }

    Property(typename cc::function<T &(void *)>::ptr_t getter, void *cap) : mRefGetter(getter), mpCapture(cap)
    {
    }

    Property(typename cc::function<T(void *)>::ptr_t getter, void *capture) : mGetter(getter), mpCapture(capture)
    {
    }

    Property(typename cc::function<T(void *)>::ptr_t getter,
             typename cc::function<void(void *, const T &)>::ptr_t setter,
             void *capture)
        : mGetter(getter), mSetter(setter), mpCapture(capture)
    {
    }

    Property() : mRefBinding(&mOwnValue)
    {
    }

    T GetValue() const
    {
        if (mGetter)
        {
            return mGetter(mpCapture);
        }
        if (mRefGetter)
        {
            return mRefGetter(mpCapture);
        }
        CCASSERT(!!mRefBinding);
        return *mRefBinding;
    }
    void SetValue(const T &val)
    {
        T oldVal = GetValue();
        if (oldVal == val)
            return;
        bool didSet = false;
        if (mRefBinding)
        {
            *mRefBinding = val;
            didSet = true;
        }
        if (mRefGetter)
        {
            mRefGetter(mpCapture) = val;
            didSet = true;
        }
        if (mGetter && mSetter)
        {
            mSetter(mpCapture, val);
            didSet = true;
        }
        CCASSERT(didSet);
    }
};

template <typename Tprop, typename Tval>
Property<Tprop> MakePropertyByCasting(Tval *x)
{
    static auto getter = [](void *capture) {
        Tval *px = (Tval *)capture;
        return static_cast<Tprop>(*px);
    };

    static auto setter = [](void *capture, const Tprop &val) {
        Tval *px = (Tval *)capture;
        *px = static_cast<Tval>(val);
    };

    return Property<Tprop>(getter, setter, x);
}

} // namespace clarinoid
