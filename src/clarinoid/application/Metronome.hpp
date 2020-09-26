
#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>

struct Metronome
{
    CCThrottlerT<500> mMetronomeTimer; // the 500 is arbitrary
    int GetBeatInt() const {
        // gSynthGraphControl.mMetronomeTimer.GetBeatFloat(60000.0f / gAppSettings.mBPM)
        return mMetronomeTimer.GetBeatInt((int)(60000.0f / gAppSettings.mBPM));
    }
    float GetBeatFloat() const {
        return mMetronomeTimer.GetBeatFloat((int)(60000.0f / gAppSettings.mBPM));
    }
    float GetBeatFrac() const {
        return mMetronomeTimer.GetBeatFrac((int)(60000.0f / gAppSettings.mBPM));
    }
};


Metronome gMetronome;

