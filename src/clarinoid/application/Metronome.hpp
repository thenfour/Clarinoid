
#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>

namespace clarinoid
{

struct Metronome
{
  AppSettings* mAppSettings;

  explicit Metronome(AppSettings* appSettings) :
    mAppSettings(appSettings)
  {
  }

  PeriodicTimer mTimer;

    int GetBeatInt() {
        mTimer.SetPeriod(MillisToMicros((uint32_t)(60000.0f / mAppSettings->mBPM)));
        return mTimer.GetBeatInt();
    }
    float GetBeatFloat() {
        mTimer.SetPeriod(MillisToMicros((uint32_t)(60000.0f / mAppSettings->mBPM)));
       return mTimer.GetBeatFloat();
    }
    float GetBeatFrac() {
        mTimer.SetPeriod(MillisToMicros((uint32_t)(60000.0f / mAppSettings->mBPM)));
       return mTimer.GetBeatFrac();
    }
};


} // namespace clarinoid
