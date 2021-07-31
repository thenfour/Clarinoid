#pragma once

#include <limits>

#include "Memory.hpp"
#include "Property.hpp"

namespace clarinoid
{

template <typename T, T minOutput, T maxOutput>
static T Float01ToInt(float f)
{
    if (f <= 0.0f)
        return minOutput;
    if (f >= 1.0f)
        return maxOutput;
    T ret = (T)(f * (maxOutput - minOutput));
    return ret;
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

    template <size_t N>
    array_view(T (&a)[N]) : mSize(N), mData(a)
    {
    }
};

template <typename T, size_t N>
array_view<T> make_array_view(T (&a)[N])
{
    return array_view<T>(a);
}

// https://stackoverflow.com/questions/26351587/how-to-create-stdarray-with-initialization-list-without-providing-size-directl
template <typename V, typename... T>
constexpr auto array_of(T &&... t) -> std::array<V, sizeof...(T)>
{
    return {{std::forward<T>(t)...}};
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

static String IndexToChar(int i)
{
    char r[2] = {0};
    if (i < 10)
    { // 0-9
        r[0] = '0' + i;
    }
    else if (i < 37)
    { // 10-36
        r[0] = 'A' + (i - 10);
    }
    else
    {
        r[0] = '!';
    }
    return String(r);
}

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
