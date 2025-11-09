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
    // int mPeakValue = 0;
    bool mActive = false;

    void Configure(int holdMs, int decayMs)
    {
        mHoldMs = std::max(holdMs, 0);
        mDecayMs = std::max(decayMs, 1);
        // mPeakValue = peakValue;
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

    // return 0-1 amplitude
    float Sample01()
    {
        if (!mActive)
        {
            return 0;
        }

        int elapsedMs = (int)mTimer.ElapsedTime().ElapsedMillisI();
        if (elapsedMs <= mHoldMs)
        {
            return 1.0f;
        }

        int msPastHold = elapsedMs - mHoldMs;
        if (msPastHold >= mDecayMs)
        {
            mActive = false;
            return 0;
        }

        float remaining = (float)(mDecayMs - msPastHold);
        float amp = remaining / (float)mDecayMs;
        return amp;

        // int remaining = mDecayMs - msPastHold;
        // int brightness = (mPeakValue * remaining) / mDecayMs;
        // brightness = ClampInclusive(brightness, 0, mPeakValue);
        // if (brightness == 0)
        // {
        //     mActive = false;
        // }
        // return brightness;
    }
};

} // namespace clarinoid