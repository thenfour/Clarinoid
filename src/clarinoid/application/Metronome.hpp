
#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>

struct Metronome
{
    CCThrottlerT<500> mMetronomeTimer; // the 500 is arbitrary
    int GetBeatInt() const {
        // gSynthGraphControl.mMetronomeTimer.GetBeatFloat(60000.0f / gAppSettings.mBPM)
        return mMetronomeTimer.GetBeatInt(60000.0f / gAppSettings.mBPM);
    }
    float GetBeatFloat() const {
        return mMetronomeTimer.GetBeatFloat(60000.0f / gAppSettings.mBPM);
    }
    float GetBeatFrac() const {
        return mMetronomeTimer.GetBeatFrac(60000.0f / gAppSettings.mBPM);
    }
};


Metronome gMetronome;

