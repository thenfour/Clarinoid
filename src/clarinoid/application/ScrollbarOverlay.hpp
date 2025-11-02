#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/basic/Stopwatch.hpp>
#include <clarinoid/application/DisplayDefs.hpp>
#include <clarinoid/components/AdafruitSSD1366Wrapper.hpp>

namespace clarinoid
{

#ifndef SSD1306_WHITE
#define SSD1306_WHITE 1
#endif
#ifndef SSD1306_BLACK
#define SSD1306_BLACK 0
#endif

enum class ScrollbarOrientation : uint8_t
{
    Vertical,
    Horizontal,
};

struct ScrollbarOverlay
{
    static constexpr int kAnimDurationMs = 200;
    static constexpr int kVisibleDurationMs = 500;
    static constexpr int kMinimumHandlePixels = 8;

    ScrollbarOrientation mOrientation;
    bool mIsActive = false;
    Stopwatch mTimer;
    float mStartFraction = 0.0f;
    float mTargetFraction = 0.0f;

    explicit ScrollbarOverlay(ScrollbarOrientation orientation = ScrollbarOrientation::Vertical)
        : mOrientation(orientation)
    {
    }

    void Reset()
    {
        mIsActive = false;
        mStartFraction = 0.0f;
        mTargetFraction = 0.0f;
    }

    void SetOrientation(ScrollbarOrientation orientation)
    {
        mOrientation = orientation;
    }

    void TriggerMovement(int fromIndex, int toIndex, size_t totalItems, int step)
    {
        if (totalItems <= 1)
        {
            Reset();
            return;
        }

        if (step == 0)
        {
            return;
        }

        float fromFraction = IndexToFraction(fromIndex, totalItems);
        float toFraction = IndexToFraction(toIndex, totalItems);

        bool wrappedForward = (step > 0) && (toIndex < fromIndex);
        bool wrappedBackward = (step < 0) && (toIndex > fromIndex);

        if (wrappedForward || wrappedBackward)
        {
            mStartFraction = toFraction;
            mTargetFraction = toFraction;
            mIsActive = true;
            mTimer.Restart();
            return;
        }

        if (mIsActive)
        {
            int elapsed = mTimer.ElapsedTime().ElapsedMillisI();
            float currentFraction = AnimatedFractionInternal(elapsed);
            mStartFraction = currentFraction;
        }
        else
        {
            mStartFraction = fromFraction;
        }

        mTargetFraction = toFraction;
        mIsActive = true;
        mTimer.Restart();
    }

    void Render(IDisplay &display, const RectI &clientRect, size_t visibleItemCount, size_t totalItemCount)
    {
        if (!mIsActive)
        {
            return;
        }
        if (visibleItemCount == 0)
        {
            return;
        }
        if (totalItemCount == 0)
        {
            mIsActive = false;
            return;
        }
        if (totalItemCount <= visibleItemCount)
        {
            // Everything is already visible; hide the overlay.
            mIsActive = false;
            return;
        }

        int elapsed = mTimer.ElapsedTime().ElapsedMillisI();
        if (elapsed >= kVisibleDurationMs)
        {
            mIsActive = false;
            return;
        }

        float fraction = Clamp01(AnimatedFractionInternal(elapsed));

        switch (mOrientation)
        {
        case ScrollbarOrientation::Vertical:
            RenderVertical(display, clientRect, visibleItemCount, totalItemCount, fraction);
            break;
        case ScrollbarOrientation::Horizontal:
            RenderHorizontal(display, clientRect, visibleItemCount, totalItemCount, fraction);
            break;
        }
    }

  private:
    static float IndexToFraction(int index, size_t totalItems)
    {
        if (totalItems <= 1)
        {
            return 0.0f;
        }
        float denom = (float)(totalItems - 1);
        float fraction = (float)index / denom;
        return Clamp01(fraction);
    }

    float AnimatedFractionInternal(int elapsedMs) const
    {
        if (!mIsActive)
        {
            return mTargetFraction;
        }
        if (kAnimDurationMs <= 0)
        {
            return mTargetFraction;
        }
        int clamped = ClampInclusive(elapsedMs, 0, kAnimDurationMs);
        float t = (float)clamped / (float)kAnimDurationMs;
        return mStartFraction + (mTargetFraction - mStartFraction) * t;
    }

    static void RenderVertical(IDisplay &display,
                               const RectI &clientRect,
                               size_t visibleItemCount,
                               size_t totalItemCount,
                               float fraction)
    {
        int trackHeight = clientRect.height;
        if (trackHeight <= 0)
        {
            return;
        }
        const int trackWidth = 3;
        int trackX = clientRect.right() - trackWidth;
        if (trackX < clientRect.x)
        {
            trackX = clientRect.x;
        }

        int handleHeight = (int)((float)trackHeight * ((float)visibleItemCount / (float)totalItemCount));
        handleHeight = ClampInclusive(handleHeight, kMinimumHandlePixels, trackHeight);

        int maxTravel = trackHeight - handleHeight;
        int handleOffset = (int)((float)maxTravel * fraction + 0.5f);
        handleOffset = ClampInclusive(handleOffset, 0, maxTravel);

        int handleY = clientRect.y + handleOffset;

        display.fillRect(trackX, clientRect.y, trackWidth, trackHeight, SSD1306_BLACK);
        display.drawFastVLine(trackX + trackWidth / 2, clientRect.y, trackHeight, SSD1306_WHITE);
        display.fillRect(trackX, handleY, trackWidth, handleHeight, SSD1306_WHITE);
        if (trackWidth > 2 && handleHeight > 2)
        {
            display.fillRect(trackX + 1, handleY + 1, trackWidth - 2, handleHeight - 2, SSD1306_BLACK);
        }
    }

    static void RenderHorizontal(IDisplay &display,
                                 const RectI &clientRect,
                                 size_t visibleItemCount,
                                 size_t totalItemCount,
                                 float fraction)
    {
        int trackWidth = clientRect.width;
        if (trackWidth <= 0)
        {
            return;
        }
        const int trackHeight = 3;
        int trackY = clientRect.bottom() - trackHeight;
        if (trackY < clientRect.y)
        {
            trackY = clientRect.y;
        }

        int handleWidth = (int)((float)trackWidth * ((float)visibleItemCount / (float)totalItemCount));
        handleWidth = ClampInclusive(handleWidth, kMinimumHandlePixels, trackWidth);

        int maxTravel = trackWidth - handleWidth;
        int handleOffset = (int)((float)maxTravel * fraction + 0.5f);
        handleOffset = ClampInclusive(handleOffset, 0, maxTravel);

        int handleX = clientRect.x + handleOffset;

        display.fillRect(clientRect.x, trackY, trackWidth, trackHeight, SSD1306_BLACK);
        display.drawFastHLine(clientRect.x, trackY + trackHeight / 2, trackWidth, SSD1306_WHITE);
        display.fillRect(handleX, trackY, handleWidth, trackHeight, SSD1306_WHITE);
        if (trackHeight > 2 && handleWidth > 2)
        {
            display.fillRect(handleX + 1, trackY + 1, handleWidth - 2, trackHeight - 2, SSD1306_BLACK);
        }
    }
};

} // namespace clarinoid
