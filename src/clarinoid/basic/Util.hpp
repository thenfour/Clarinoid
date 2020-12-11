#pragma once

#include <limits>

#include "Memory.hpp"
//#include "Taskman.hpp"
//#include "FPS.hpp"
//#include "Profiler.hpp"
#include "Property.hpp"

namespace clarinoid
{
  bool FloatEquals(float f1, float f2, float eps = 0.00001f)
  {
    return fabs(f1 - f2) < eps;
  }

    struct PointF
    {
      float x;
      float y;
    };
    struct PointI
    {
      int x;
      int y;
    };

    struct RectF
    {
      float x;
      float y;
      float width;
      float height;
    };
    struct RectI
    {
      int x;
      int y;
      int width;
      int height;
    };

    struct ColorF
    {
        float r;
        float g;
        float b;
    };

    template<typename T, T minOutput, T maxOutput>
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
  template<typename T>
  StaticInit(T&& x) { 
    x();
  }
};

uint16_t ClampUint32ToUint16(uint32_t a) {
  if (a > std::numeric_limits<uint16_t>::max())
    return std::numeric_limits<uint16_t>::max();
  return (uint16_t)a;
}


  template<typename T>
  struct array_view
  {
    size_t mSize = 0;
    T* mData = nullptr;

    array_view() = default;

    template<size_t N>
    array_view(T(&a)[N]) :
      mSize(N),
      mData(a)
    {
    }
  };

  template<typename T, size_t N>
  array_view<T> make_array_view(T(&a)[N])
  {
    return array_view<T>(a);
  }

// https://stackoverflow.com/questions/26351587/how-to-create-stdarray-with-initialization-list-without-providing-size-directl
template <typename V, typename... T>
constexpr auto array_of(T&&... t)
    -> std::array < V, sizeof...(T) >
{
    return {{ std::forward<T>(t)... }};
}


template<typename T, T divisor>
void DivRem(T val, T& wholeParts, T& remainder)
{
  wholeParts = val / divisor;
  remainder = val % divisor;
}


template<size_t divBits, typename Tval, typename Tremainder>
void DivRemBitwise(Tval val, size_t& wholeParts, Tremainder& remainder)
{
  static_assert(std::is_integral<Tval>::value, "must be integral");
  static_assert(std::is_integral<Tremainder>::value, "must be integral");
  auto mask = (1 << divBits) - 1;
  wholeParts = val / mask;
  //auto rem = val & mask;// wish this would work but it doesn't.
  auto rem = (val - (wholeParts * mask));
  CCASSERT((Tremainder)std::max((decltype(rem))0, rem) <= std::numeric_limits<Tremainder>::max());
  remainder = (Tremainder)rem;
}


template<int period>
int ModularDistance(int a, int b)
{
  a = RotateIntoRange(a, period);
  b = RotateIntoRange(b, period);
  if (a > b) {
    return std::min(a - b, b + period - a);
  }
  return std::min(b - a, a + period - b);
}

struct SawWave
{
  uint32_t mFrequencyMicros = 1000000; // just 1 hz frequency default
  void SetFrequency(float f)
  {
    mFrequencyMicros = (uint32_t)(1000000.0f / f);
  }
  float GetValue01(uint64_t tmicros) const
  {
    float r = (float)(tmicros % mFrequencyMicros);
    return r / mFrequencyMicros;
  }
};

struct PulseWave
{
  float mFrequency;
  uint32_t mFrequencyMicros;
  float mDutyCycle01;
  uint32_t mDutyCycleMicros;

  void SetFrequencyAndDutyCycle01(float freq, float dc01)
  {
    mFrequency = freq;
    mFrequencyMicros = (uint32_t)(1000000.0f / freq);
    mDutyCycle01 = dc01;
    mDutyCycleMicros = (uint32_t)(dc01 * mFrequencyMicros);
  }

  void SetFrequency(float f)
  {
    SetFrequencyAndDutyCycle01(f, mDutyCycle01);
  }

  void SetDutyCycle01(float dc01)
  {
    SetFrequencyAndDutyCycle01(mFrequency, dc01);
  }

  PulseWave()
  {
    SetFrequencyAndDutyCycle01(1.0f, 0.5f);
  }

  int GetValue01Int(uint64_t tmicros) const
  {

    uint64_t pos = (tmicros % mFrequencyMicros);
    return (pos < mDutyCycleMicros) ? 0 : 1;
  }
};

// for a triangle, it's just a modified sawtooth.
struct TriangleWave
{
  uint32_t mFrequencyMicros;
  void SetFrequency(float f)
  {
    mFrequencyMicros = (uint32_t)(1000000.0f / f);
  }
  TriangleWave() { SetFrequency(1.0f); }
  float GetValue01(uint64_t tmicros) const
  {
    float r = (float)(tmicros % mFrequencyMicros);
    r = r / mFrequencyMicros; // saw wave
    r -= .5f; // 1st half the wave is negative.
    r = ::fabsf(r); // it's a triangle but half height.
    return r * 2;
  }
};



struct NoInterrupts
{
  static int gNoInterruptRefs;
  NoInterrupts() {
#ifndef CLARINOID_PLATFORM_X86
    if (0 == gNoInterruptRefs) {
      // __disable_irq(); // not sure which one to use honestly...
      NVIC_DISABLE_IRQ(IRQ_SOFTWARE);
    }
#endif
    gNoInterruptRefs ++;
  }
  ~NoInterrupts()
  {
    gNoInterruptRefs --;
#ifndef CLARINOID_PLATFORM_X86
    if (0 == gNoInterruptRefs) {
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

