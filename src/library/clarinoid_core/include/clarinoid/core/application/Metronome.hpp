
#pragma once

#include <clarinoid/core/basic/Basic.hpp>
#include <clarinoid/core/settings/AppSettings.hpp>

namespace clarinoid
{

// from 7jam
// https://github.com/thenfour/digifujam/blob/988608a35682dda7a5b8266bb9ccd48769d59382/clientsrc/quantizer.js#L20
struct Metronome : IMetronome
{
    AppSettings &mAppSettings;
    TimeSpan mRootTime;
    float mBPM; // important that we store this here, during bpm changes to know previous / new.

    Metronome(AppSettings &appSettings) : mAppSettings(appSettings)
    {
        mBPM = mAppSettings.GetCurrentPerformancePatch().mBPM;
        mRootTime = Uptime();
    }

    void OnBPMChanged() override
    {
        // make it smoothly modulated; "now" should finish out the current beat.
        // so, make the new root time (now - current beat fraction * new bpm)
        // this is required in order to make BPM changes and not cause total chaos with regards to sequencer timing.
        float b = GetBeatFloat();                                                        // old bpm
        auto t = TimeSpan::FromBeats(b, mAppSettings.GetCurrentPerformancePatch().mBPM); // new bpm
        mRootTime = Uptime() - t;
        mBPM = mAppSettings.GetCurrentPerformancePatch().mBPM;
    }

    uint32_t GetBeatInt() const override
    {
        return (uint32_t)floorf(GetBeatFloat());
    }

    float GetBeatFrac() const override
    {
        return Frac(GetBeatFloat());
    }

    float GetBeatFloat() const override
    {
        auto absTime = (Uptime() - mRootTime);
        return absTime.ElapsedBeats(mBPM);
    }
};

} // namespace clarinoid
