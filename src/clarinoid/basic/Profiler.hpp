#pragma once

#include <functional>
#include "Enum.hpp"
#include "Stopwatch.hpp"

//////////////////////////////////////////////////////////////////////
enum class ProfileObjectType
{
  Total,
  LED,
  Pot,
  Encoder,
  MIDI,
  Synth,
  EWIApp,
  Display,
  PlotHelper,
  Switch,
  TxRx,
  TouchKeyCalibration,
  TouchKey,
  BreathSensor,
  END
};

static constexpr size_t gProfileObjectTypeCount = (size_t)(ProfileObjectType::END);

EnumItemInfo<ProfileObjectType> gProfileObjectTypeItems[gProfileObjectTypeCount] = {
  { ProfileObjectType::Total, "Total" },
  { ProfileObjectType::LED, "LED" },
  { ProfileObjectType::Pot, "Pot" },
  { ProfileObjectType::Encoder, "Encoder" },
  { ProfileObjectType::MIDI, "MIDI" },
  { ProfileObjectType::Synth, "Synth" },
  { ProfileObjectType::EWIApp, "EWIApp" },
  { ProfileObjectType::Display, "Display" },
  { ProfileObjectType::PlotHelper, "PlotHelper" },
  { ProfileObjectType::Switch, "Switch" },
  { ProfileObjectType::TxRx, "TxRx" },
  { ProfileObjectType::TouchKeyCalibration, "TouchKeyCalibration" },
  { ProfileObjectType::TouchKey, "TouchKey" },
  { ProfileObjectType::BreathSensor, "BreathSensor" },
};

EnumInfo<ProfileObjectType> gProfileObjectTypeInfo (gProfileObjectTypeItems);


struct ProfileTiming
{
  ProfileObjectType mType;
  uint32_t mUpdateMillis = 0;
  uint32_t mRenderMillis = 0;
  uint32_t mLoopMillis = 0;
};


struct Profiler
{
  ProfileTiming mTimings[gProfileObjectTypeCount];

  Profiler() {
    for (size_t i = 0; i < gProfileObjectTypeCount; ++ i) {
      mTimings[i].mType = (ProfileObjectType)i;  
    }
  }
};

Profiler gProfiler;

struct ProfileTimer
{
  Stopwatch mStopwatch;
  ProfileObjectType mType;
  cc::function<uint32_t*(ProfileTiming&)>::ptr_t mSelector;
  ProfileTimer(ProfileObjectType type, cc::function<uint32_t*(ProfileTiming&)>::ptr_t selector) :
    mType(type),
    mSelector(selector)
  { }
  ~ProfileTimer()
  {
    uint32_t elapsed = (uint32_t)mStopwatch.ElapsedMicros();
    *(mSelector(gProfiler.mTimings[(size_t)mType])) += elapsed;
    *(mSelector(gProfiler.mTimings[(size_t)ProfileObjectType::Total])) += elapsed;
  }
};

