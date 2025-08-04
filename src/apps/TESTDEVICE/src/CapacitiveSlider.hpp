#pragma once

// #include <Adafruit_MPR121.h>
#include <clarinoid/components/MPR121.hpp>

// todo: intermediate class to fetch data from mpr121 in a time-sliced way.
// todo: maybe need to have a touch threshold, to detect when the default value should be used

namespace clarinoid {

template<size_t NUM_ELECTRODES>
struct CapacitiveSlider
{
  void Init(MPR121::MPR121Device& mpr121)
  {
    m_mpr121 = &mpr121;
    // m_defaultValue = defaultValue;

    m_currentSliderValue = 0;
  }

  void Update()
  {
    for (uint8_t i = 0; i < NUM_ELECTRODES; ++i) {
      uint8_t e = i;
      m_baseline[e] = m_mpr121->GetBaselineData(e);
      m_filtered[e] = m_mpr121->GetFilteredData(e);
    }

    // float newPosition = computePositionFromElectrodes();
    // m_currentSliderValue = newPosition;

    // compute a centroid
    float totalStrength = 0.0f;
    float weightedSum = 0.0f;

    for (uint8_t i = 0; i < NUM_ELECTRODES; ++i) {
      float strength = computeTouchStrength(i);
      totalStrength += strength;
      weightedSum += strength * float(i);
    }

    if (totalStrength < 0.001f) {
      // No meaningful touch. Return the default position.
      m_currentSliderValue = 0.0f;
      mIsTouched = false;
    } else {
      // Weighted average -> range is [0, NUM_ELECTRODES-1]
      float avgElectrode = weightedSum / totalStrength;
      // Map to 0 - 1 range.
      // If you want to treat edges more precisely or have spacing, adapt here.
      float pos = avgElectrode / float(NUM_ELECTRODES - 1);
      m_currentSliderValue = Clamp01(pos);
      mIsTouched = true;
    }
  }

  float GetValue01() const { return m_currentSliderValue; }

  // private:

  MPR121::MPR121Device* m_mpr121 = nullptr;

  float m_currentSliderValue; ///< Smoothed slider value
  bool mIsTouched = false;

  uint16_t m_baseline[NUM_ELECTRODES] = { 0 };
  uint16_t m_filtered[NUM_ELECTRODES] = { 0 };

  float computeTouchStrength(uint8_t i) const
  {
    int m = (int)m_baseline[i] - (int)m_filtered[i];
    // using the threshold is useful to reduce noise when no touch is present; otherwise random electrodes will hog the
    // weighted avg.
    if (m < m_mpr121->mTouchThreshold)
      m = 0;
    return m;
  }
};

// accepts a monopolar 0-1 slider, and outputs a bipolar -1 to 1 slider, where the "untouched" state is 0.
template<size_t NUM_ELECTRODES>
struct BipolarCapacitiveSlider
{
  CapacitiveSlider<NUM_ELECTRODES> mSlider;

  void Init(MPR121::MPR121Device& mpr121) { mSlider.Init(mpr121); }

  void Update() { mSlider.Update(); }

  float GetValueN11() const
  {
    if (!mSlider.mIsTouched)
      return 0;
    float v = mSlider.GetValue01();
    return v * 2.0f - 1.0f;
  }

  bool IsTouched() const { return mSlider.mIsTouched; }
};

} // namespace clarinoid