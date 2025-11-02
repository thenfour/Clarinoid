#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/basic/Stopwatch.hpp>
#include <clarinoid/application/DisplayDefs.hpp>

namespace clarinoid
{

enum class ScrollbarOrientation : uint8_t
{
    Vertical,
    Horizontal,
};

struct ScrollbarOverlay
{
    static constexpr int kAnimDurationMs = 300;
    static constexpr int kVisibleDurationMs = 600;
    static constexpr float kMinimumHandlePixels = 8.0f;
    static constexpr float kTrackMargin = 0; // 1.0f;
    static constexpr float kTrackThickness = 6.0f;
    // static constexpr float kTrackCornerRadius = 1.0f;
    // static constexpr float kHandleCornerRadius = 3.0f;
    // static constexpr int kTrackCoverageQp8 = 0;
    static constexpr int kHandleCoverageQp8 = 255;

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

    // stepDirection = -1, 0, +1 to indicate how we got to this new index.
    void TriggerMovement(int fromIndex, int toIndex, size_t totalItems, int stepDirection)
    {
        if (totalItems <= 1)
        {
            Reset();
            return;
        }

        if (stepDirection == 0)
        {
            return;
        }

        float fromFraction = IndexToFraction(fromIndex, totalItems);
        float toFraction = IndexToFraction(toIndex, totalItems);

        bool wrappedForward = (stepDirection > 0) && (toIndex < fromIndex);
        bool wrappedBackward = (stepDirection < 0) && (toIndex > fromIndex);

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
            mIsActive = true;
        }

        mTargetFraction = toFraction;
        mTimer.Restart();
    }

    void Render(IDisplay &display, const RectI &clientRect_, size_t visibleItemCount, size_t totalItemCount)
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

        // hackhack: client rect should be 1 px taller for perfect alignment.
        RectI clientRect = clientRect_;
        clientRect.height += 1;

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
        float trackHeight = (float)clientRect.height - 2.0f * kTrackMargin;
        if (trackHeight <= 0.0f)
        {
            return;
        }
        float trackWidth = kTrackThickness;
        float trackX = (float)(clientRect.right()) - trackWidth - kTrackMargin;
        float trackY = (float)clientRect.y + kTrackMargin;
        if (trackX < (float)clientRect.x)
        {
            trackX = (float)clientRect.x;
        }

        int backgroundX = (int)floorf(trackX - 1.0f);
        int backgroundY = clientRect.y;
        int backgroundWidth = (int)ceilf(trackWidth + 2.0f);
        if (backgroundWidth < 1)
        {
            backgroundWidth = 1;
        }
        display.fillRect(backgroundX, backgroundY, backgroundWidth, clientRect.height, 0);
        display.drawFastVLine(backgroundX, backgroundY, clientRect.height, 1);

        float handleHeight = trackHeight * ((float)visibleItemCount / (float)totalItemCount);
        handleHeight = Clamp(handleHeight, kMinimumHandlePixels, trackHeight);
        float maxTravel = trackHeight - handleHeight;
        if (maxTravel < 0.0f)
        {
            maxTravel = 0.0f;
        }
        float handleOffset = maxTravel * fraction;
        float handleY = trackY + handleOffset;

        RectF handleRect{trackX, handleY, trackWidth, handleHeight};
        display.FillRectSubpixel(handleRect, kHandleCoverageQp8);
    }

    static void RenderHorizontal(IDisplay &display,
                                 const RectI &clientRect,
                                 size_t visibleItemCount,
                                 size_t totalItemCount,
                                 float fraction)
    {
        float trackWidth = (float)clientRect.width - 2.0f * kTrackMargin;
        if (trackWidth <= 0.0f)
        {
            return;
        }
        float trackHeight = kTrackThickness;
        float trackX = (float)clientRect.x + kTrackMargin;
        float trackY = (float)(clientRect.bottom()) - trackHeight - kTrackMargin;
        if (trackY < (float)clientRect.y)
        {
            trackY = (float)clientRect.y;
        }

        int backgroundX = clientRect.x;
        int backgroundY = (int)floorf(trackY - 1.0f);
        int backgroundHeight = (int)ceilf(trackHeight + 2.0f);
        if (backgroundHeight < 1)
        {
            backgroundHeight = 1;
        }
        display.fillRect(backgroundX, backgroundY, clientRect.width, backgroundHeight, 0);
        display.drawFastHLine(backgroundX, backgroundY, clientRect.width, 1);

        float handleWidth = trackWidth * ((float)visibleItemCount / (float)totalItemCount);
        handleWidth = Clamp(handleWidth, kMinimumHandlePixels, trackWidth);
        float maxTravel = trackWidth - handleWidth;
        if (maxTravel < 0.0f)
        {
            maxTravel = 0.0f;
        }
        float handleOffset = maxTravel * fraction;
        float handleX = trackX + handleOffset;

        RectF handleRect{handleX, trackY, handleWidth, trackHeight};
        display.FillRectSubpixel(handleRect, kHandleCoverageQp8);
    }
};

} // namespace clarinoid
