#pragma once

#include <limits>

#include "Memory.hpp"
#include "Property.hpp"

namespace clarinoid
{

template <typename Tval, typename TEnum>
static bool HasFlag(Tval val, TEnum e)
{
    auto ival = (typename std::underlying_type<TEnum>::type)val;
    auto ie = (typename std::underlying_type<TEnum>::type)e;
    return (ival & ie) == ie;
}

// use as a global var to run init code
struct StaticInit
{
    template <typename T>
    StaticInit(T &&x)
    {
        x();
    }
};

template <typename T>
struct array_view
{
    size_t mSize = 0;
    T *mData = nullptr;

    array_view() = default;
    size_t size() const
    {
        return mSize;
    }

    template <size_t N>
    array_view(T (&a)[N]) : mSize(N), mData(a)
    {
    }

    using iterator = T *;

    iterator begin()
    {
        return mData;
    }
    iterator end()
    {
        return mData + mSize;
    }

    T &operator[](size_t i)
    {
        CCASSERT(i < mSize);
        return mData[i];
    }
};

template <typename T, size_t N>
array_view<T> make_array_view(T (&a)[N])
{
    return array_view<T>(a);
}

// https://stackoverflow.com/questions/26351587/how-to-create-stdarray-with-initialization-list-without-providing-size-directl
template <typename V, typename... T>
constexpr auto array_of(T &&...t) -> std::array<V, sizeof...(T)>
{
    return {{std::forward<T>(t)...}};
}

// Tfn is callable bool(const Item&)
template <typename Tit, typename Tfn>
bool Any(const Tit &begin, const Tit &end, Tfn pred)
{
    for (Tit it = begin; it != end; ++it)
    {
        if (pred(*it))
            return true;
    }
    return false;
}

// Tfn is callable bool(const Item&)
template <typename Tcontainer, typename Tfn>
bool Any(const Tcontainer &container, Tfn pred)
{
    for (auto &x : container)
    {
        if (pred(x))
            return true;
    }
    return false;
}

template <typename T, size_t N>
bool ArrayContains(const T (&arr)[N], const T& val)
{
    for (auto &x : arr)
    {
        if (x == val)
            return true;
    }
    return false;
}



struct NoInterrupts
{
    static int gNoInterruptRefs;
    NoInterrupts()
    {
#ifndef CLARINOID_PLATFORM_X86
        if (0 == gNoInterruptRefs)
        {
            // __disable_irq(); // not sure which one to use honestly...
            NVIC_DISABLE_IRQ(IRQ_SOFTWARE);
        }
#endif
        gNoInterruptRefs++;
    }
    ~NoInterrupts()
    {
        gNoInterruptRefs--;
#ifndef CLARINOID_PLATFORM_X86
        if (0 == gNoInterruptRefs)
        {
            // __enable_irq(); // not sure which one to use honestly...
            NVIC_ENABLE_IRQ(IRQ_SOFTWARE);
        }
#endif
    }
};

int NoInterrupts::gNoInterruptRefs = 0;

// // allows throttled plotting to Serial.
// class PlotHelper : UpdateObjectT<ProfileObjectType::PlotHelper>
// {
//   CCThrottlerT<5> mThrot;
//   String mFields;
// public:
//   virtual void loop() {
//     if (mThrot.IsReady()) {
//       if (mFields.length() > 0) {
//         if (Serial) {
//           Serial.println(mFields);
//         }
//       }
//     }
//     mFields = "";
//   }

//   template<typename T>
//   void AppendField(const T& s) {
//     if (mFields.length() > 0) {
//       mFields.append("\t");
//     }
//     mFields.append(s);
//   }
// };

// PlotHelper gPlot;
// template<typename T>
// inline void CCPlot(const T& val) {
//   gPlot.AppendField(val);
// }

// // helps interpreting touch keys like buttons. like a computer keyboard, starts repeating after an initial delay.
// // untouching the key will reset the delays
// template<int TrepeatInitialDelayMS, int TrepeatPeriodMS>
// class BoolKeyWithRepeat
// {
//   bool mPrevState = false;
//   CCThrottlerT<TrepeatInitialDelayMS> mInitialDelayTimer;
//   CCThrottlerT<TrepeatPeriodMS> mRepeatTimer;
//   bool mInitialDelayPassed = false;
//   bool mIsTriggered = false; // one-frame
// public:
//   void Update(bool pressed) { // call once a frame. each call to this will reset IsTriggered

//     if (!mPrevState && !pressed) {
//       // typical idle state; nothing to do.
//       mIsTriggered = false;
//       return;
//     }

//     if (mPrevState && pressed) {
//       // key repeat?
//       if (!mInitialDelayPassed) {
//         if (mInitialDelayTimer.IsReady()) {
//           mInitialDelayPassed = true;
//           mRepeatTimer.Reset();
//           // retrig.
//           mIsTriggered = true;
//           return;
//         }
//         // during initial delay; nothing to do.
//         mIsTriggered = false;
//         return;
//       }
//       // mInitialDelayPassed is satisfied. now we can check the normal key repeat timer.
//       if (mRepeatTimer.IsReady()) {
//         // trig!
//         mIsTriggered = true;
//         return;
//       }
//       // between key repeats
//       mIsTriggered = false;
//       return;
//     }

//     if (!mPrevState && pressed) {
//       // newly pressed. reset initial repeat delay
//       mInitialDelayTimer.Reset();
//       mInitialDelayPassed = false;
//       mPrevState = true;
//       mIsTriggered = true;
//       return;
//     }

//     if (mPrevState && !pressed) {
//       // newly released.
//       mIsTriggered = false;
//       mPrevState = false;
//       return;
//     }
//   }

//   // valid during 1 frame.
//   bool IsTriggered() const {
//     return mIsTriggered;
//   }
// };

} // namespace clarinoid
