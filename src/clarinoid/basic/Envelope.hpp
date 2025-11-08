#include "Stopwatch.hpp"
#include "Math.hpp"

namespace clarinoid
{

// Lightweight one-shot envelope (hold + decay) to drive flash intensities.
struct FlashEnvelope
{
    StopwatchLight mTimer;
    int mHoldMs = 0;
    int mDecayMs = 1;
    int mPeakValue = 0;
    bool mActive = false;

    void Configure(int holdMs, int decayMs, int peakValue)
    {
        mHoldMs = std::max(holdMs, 0);
        mDecayMs = std::max(decayMs, 1);
        mPeakValue = peakValue;
        mActive = false;
    }

    void Trigger()
    {
        mTimer.Restart();
        mActive = true;
    }

    void Reset()
    {
        mActive = false;
    }

    int Sample()
    {
        if (!mActive)
        {
            return 0;
        }

        int elapsedMs = (int)mTimer.ElapsedTime().ElapsedMillisI();
        if (elapsedMs <= mHoldMs)
        {
            return mPeakValue;
        }

        int msPastHold = elapsedMs - mHoldMs;
        if (msPastHold >= mDecayMs)
        {
            mActive = false;
            return 0;
        }

        int remaining = mDecayMs - msPastHold;
        int brightness = (mPeakValue * remaining) / mDecayMs;
        brightness = ClampInclusive(brightness, 0, mPeakValue);
        if (brightness == 0)
        {
            mActive = false;
        }
        return brightness;
    }
};

} // namespace clarinoid