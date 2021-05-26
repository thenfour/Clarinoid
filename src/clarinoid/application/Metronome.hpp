
#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>

namespace clarinoid
{

  struct Metronome
  {
    AppSettings *mAppSettings;

    explicit Metronome(AppSettings *appSettings) : mAppSettings(appSettings)
    {
    }

    PeriodicTimer mTimer;

    void SyncPeriod()
    {
      mTimer.SetPeriod(TimeSpan::FromBPM(mAppSettings->mBPM));
    }

    int GetBeatInt()
    {
      SyncPeriod();
      return mTimer.GetBeatInt();
    }
    float GetBeatFloat()
    {
      SyncPeriod();
      return mTimer.GetBeatFloat();
    }
    float GetBeatFrac()
    {
      SyncPeriod();
      return mTimer.GetBeatFrac();
    }
  };

} // namespace clarinoid
