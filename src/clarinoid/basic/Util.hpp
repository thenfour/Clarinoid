#pragma once

#include <limits>

#include "Memory.hpp"
#include "Property.hpp"

namespace clarinoid
{
struct PointI
{
    int x;
    int y;
    static PointI Construct(int x_, int y_)
    {
        PointI ret;
        ret.x = x_;
        ret.y = y_;
        return ret;
    }
};

struct PointF
{
    float x;
    float y;
    static PointF Construct(float x_, float y_)
    {
        PointF ret;
        ret.x = x_;
        ret.y = y_;
        return ret;
    }
    PointF Add(const PointF& rhs) const {
        return PointF::Construct(x + rhs.x, y + rhs.y);
    }
    PointF Add(const PointI& rhs) const {
        return PointF::Construct(x + rhs.x, y + rhs.y);
    }
};

struct RectI
{
    static RectI Construct(int x, int y, int w, int h)
    {
        RectI ret;
        ret.x = x;
        ret.y = y;
        ret.width = w;
        ret.height = h;
        return ret;
    }
    int x;
    int y;
    int width;
    int height;
    int right() const
    {
        return x + width;
    }
    int bottom() const
    {
        return y + height;
    }
    RectI Inflate(int n) const
    {
        return Construct(x - n, y - n, width + n + n, height + n + n);
    }
    PointI UpperLeft() const
    {
        return PointI::Construct(x, y);
    }
    PointI UpperRight() const
    {
        return PointI::Construct(x + width, y);
    }
    PointI BottomLeft() const
    {
        return PointI::Construct(x, y + height);
    }
    PointI BottomRight() const
    {
        return PointI::Construct(x + width, y + height);
    }
};
struct RectF
{
    float x;
    float y;
    float width;
    float height;
};

struct ColorF
{
    float r;
    float g;
    float b;
};
struct ColorByte
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

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
