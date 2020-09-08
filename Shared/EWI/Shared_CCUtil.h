#pragma once

#include "Taskman.hpp"
#include "FPS.hpp"
#include "Basic.hpp"
#include "Profiler.hpp"
#include "Property.hpp"

template <size_t N>
class SimpleMovingAverage
{
  public:
    void Update(float sample)
    {
        if (num_samples_ < N)
        {
            samples_[num_samples_++] = sample;
            total_ += sample;
        }
        else
        {
            float& oldest = samples_[num_samples_++ % N];
            total_ += sample - oldest;
            oldest = sample;
        }
    }

    float GetValue() const {
      return total_ / min(num_samples_, N);
    }
    size_t GetSampleCount() const { return num_samples_; }

    void Clear() {
      total_ = 0;
      num_samples_ = 0;
    }

  private:
    float samples_[N];
    size_t num_samples_ = 0;
    float total_ = 0;
};


// allows throttled plotting to Serial.
class PlotHelper : UpdateObjectT<ProfileObjectType::PlotHelper>
{
  CCThrottlerT<5> mThrot;
  String mFields;
public:
  virtual void loop() {
    if (mThrot.IsReady()) {
      if (mFields.length() > 0) {
        if (Serial) {
          Serial.println(mFields);
        }
      }
    }
    mFields = "";
  }

  template<typename T>
  void AppendField(const T& s) {
    if (mFields.length() > 0) {
      mFields.append("\t");
    }
    mFields.append(s);
  }
};

PlotHelper gPlot;
template<typename T>
inline void CCPlot(const T& val) {
  gPlot.AppendField(val);
}



// helps interpreting touch keys like buttons. like a computer keyboard, starts repeating after an initial delay.
// untouching the key will reset the delays
template<int TrepeatInitialDelayMS, int TrepeatPeriodMS>
class BoolKeyWithRepeat
{
  bool mPrevState = false;
  CCThrottlerT<TrepeatInitialDelayMS> mInitialDelayTimer;
  CCThrottlerT<TrepeatPeriodMS> mRepeatTimer;
  bool mInitialDelayPassed = false;
  bool mIsTriggered = false; // one-frame
public:
  void Update(bool pressed) { // call once a frame. each call to this will reset IsTriggered
    
    if (!mPrevState && !pressed) {
      // typical idle state; nothing to do.
      mIsTriggered = false;
      return;
    }
    
    if (mPrevState && pressed) {
      // key repeat?
      if (!mInitialDelayPassed) {
        if (mInitialDelayTimer.IsReady()) {
          mInitialDelayPassed = true;
          mRepeatTimer.Reset();
          // retrig.
          mIsTriggered = true;
          return;
        }
        // during initial delay; nothing to do.
        mIsTriggered = false;
        return;
      }
      // mInitialDelayPassed is satisfied. now we can check the normal key repeat timer.
      if (mRepeatTimer.IsReady()) {
        // trig!
        mIsTriggered = true;
        return;
      }
      // between key repeats
      mIsTriggered = false;
      return;
    }
    
    if (!mPrevState && pressed) {
      // newly pressed. reset initial repeat delay
      mInitialDelayTimer.Reset();
      mInitialDelayPassed = false;
      mPrevState = true;
      mIsTriggered = true;
      return;      
    }
    
    if (mPrevState && !pressed) {
      // newly released.
      mIsTriggered = false;
      mPrevState = false;
      return;
    }
  }

  // valid during 1 frame.
  bool IsTriggered() const {
    return mIsTriggered;
  }
};



