
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

static const size_t MAX_SYNTH_VOICES = 6;

static const size_t HARM_PRESET_COUNT = 16;
static const size_t HARM_VOICES = 6;
static const size_t HARM_SEQUENCE_LEN = 8;

static const size_t LOOP_LAYERS = 6;
static constexpr size_t MAX_MUSICAL_VOICES = LOOP_LAYERS * (HARM_VOICES + 1 /* each harmonized preset can also output the playing (live) note as well, so make room.*/);

static const size_t PRESET_NAME_LEN = 16;

static const size_t SYNTH_PRESET_COUNT = 16;

} // namespace clarinoid

#include "HarmonizerSettings.hpp"
#include "SynthSettings.hpp"
#include "LoopstationSettings.hpp"

namespace clarinoid
{



enum class GlobalScaleRefType : uint8_t
{
  Chosen,
  Deduced,
};

EnumItemInfo<GlobalScaleRefType> gGlobalScaleRefTypeItems[2] = {
  { GlobalScaleRefType::Chosen, "Chosen" },
  { GlobalScaleRefType::Deduced, "Deduced" },
};

EnumInfo<GlobalScaleRefType> gGlobalScaleRefTypeInfo ("GlobalScaleRefType", gGlobalScaleRefTypeItems);



// the pitch bend strip has various zones.
//                 | IDLE  | UP MAX | UP VARIABLE     | ZERO       | DOWN VARIABLE                | DOWN MAX  |
//                 |-------|--------|-----------------|------------|------------------------------|-----------|
// analog value:  Latch   0.03      0.08              0.20         0.38                           0.88        1.00
// IDLE =               pitch is unchanged (latching behavior issue #30)
// UPMAX =              pitch is +1
// UP VARIABLE = transition from 0 -> 1
// ZERO =               pitch is 0
// DOWN VARIABLE = transition from 0 -> -1
// DOWN MAX     =       pitch is -1
// 
// * the UP MAX range is very important, to have some buffer between the hard cutoff of IDLE to UPMAX.
//   maybe even a small transition zone would be useful to a player as a warning.
struct PitchStripSettings
{
  float mHandsOffNoiseThresh = 0.035f;
  float mPitchUpMax = 0.08f; // or min? depends how you measure this.
  float mZeroMin = 0.20f;
  float mZeroMax = 0.38f;
  float mPitchDownMax = 0.88f;
};

struct BreathCalibrationSettings
{
    float mRangeMin = 0.11f;
    float mRangeMax = 0.57f;
    float mCurve = 1.0f;
    //float mNoteOnThreshold = 0.1f;// POST-scaling.

    BreathCalibrationSettings() = default;
    ~BreathCalibrationSettings() = default;
    bool operator ==(const BreathCalibrationSettings& rhs) const
    {
      if (mRangeMin != rhs.mRangeMin) return false;
      if (mRangeMax != rhs.mRangeMax) return false;
      if (mCurve != rhs.mCurve) return false;
      //if (mNoteOnThreshold != rhs.mNoteOnThreshold) return false;
      return true;
    }
    bool operator !=(const BreathCalibrationSettings& rhs) const
    {
      return !(rhs == *this);
    } 

    float TranfsormValue01(float f)
    {
      float realMin, realMax;
      if (mRangeMin < mRangeMax) {
        realMin = mRangeMin;
        realMax = mRangeMax;
      } else {
        realMin = mRangeMax;
        realMax = mRangeMin;
      }
        // scale
        f -= realMin;
        f /= (realMax - realMin);
        if (f < 0) f = 0;
        if (f > 1) f = 1;
        if (mCurve != 1.0f) {
            f = 1.0f - ::powf(1.0f - f, mCurve);
            if (f < 0) f = 0;
            if (f > 1) f = 1;
        }
        return f;
    }
};

struct AppSettings
{
  bool mDisplayDim = true;
  bool mOrangeLEDs = false;

  BreathCalibrationSettings mBreathCalibration;
  
  //float mTouchMaxFactor = 1.5f;

  PitchStripSettings mPitchStrip;

  float mReverbGain = 0.02f;

  bool mMetronomeOn = false;
  float mMetronomeGain = 0.8f;
  int mMetronomeNote = 80;
  int mMetronomeDecayMS= 15;

  float mBPM = 104.0f;

  int mTranspose = 0;

  GlobalScaleRefType mGlobalScaleRef = GlobalScaleRefType::Deduced;
  Scale mGlobalScale = Scale { Note::E, ScaleFlavorIndex::MajorPentatonic }; // you can set this in menus
  Scale mDeducedScale = Scale { Note::C, ScaleFlavorIndex::MajorPentatonic };; // this is automatically populated always
  
  HarmSettings mHarmSettings;
  LooperSettings mLooperSettings;
  SynthSettings mSynthSettings;

  // these are for the live playing voice. a harmonizer's voices can override the synth preset though.
  uint16_t mGlobalSynthPreset = 0;
  uint16_t mGlobalHarmPreset = 0;


  HarmPreset& FindHarmPreset(uint16_t id) {
    if (id < 0) return mHarmSettings.mDisabledPreset;
    if (id >= SYNTH_PRESET_COUNT) return mHarmSettings.mDisabledPreset;
    return mHarmSettings.mPresets[id];
  }

  SynthPreset& FindSynthPreset(uint16_t id)
  {
    if (id < 0) id = 0;
    if (id >= SYNTH_PRESET_COUNT) id = 0;
    return mSynthSettings.mPresets[id];
  }

};

//AppSettings gAppSettings;
//static constexpr auto appsettingssize = sizeof(AppSettings);


} // namespace clarinoid

