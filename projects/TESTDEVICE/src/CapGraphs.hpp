

#include <clarinoid/basic/BaseDefs.hpp>
#include <clarinoid/application/DisplayDefs.hpp>

namespace clarinoid {

// class RangeBar
// {
//   public:
//     enum Orientation
//     {
//         LeftToRight,
//         RightToLeft,
//         TopToBottom,
//         BottomToTop
//     };

//     /**
//      * @brief Construct a new RangeBar object
//      *
//      * @param rc              The rectangle on screen where the bar will be drawn.
//      * @param inputMin        Minimum of the input range.
//      * @param inputMax        Maximum of the input range.
//      * @param orientation     One of the four orientations.
//      * @param barOriginValue  The "anchor" value from which the bar filling starts.
//      */
//     RangeBar(const RectI &rc, float inputMin, float inputMax, Orientation orientation, float barOriginValue)
//         : mRect(rc), mMin(inputMin), mMax(inputMax), mOrientation(orientation), mBarOriginValue(barOriginValue),
//           mVal(barOriginValue) // Start the bar at the origin value
//     {
//         // Ensure valid range (optional: you can assert if mMin >= mMax).
//         if (mMax < mMin)
//         {
//             float temp = mMax;
//             mMax = mMin;
//             mMin = temp;
//         }
//         clampValue(mVal);
//         clampValue(mBarOriginValue);
//     }

//     /**
//      * @brief Sets the current value, clamped to [mMin, mMax].
//      */
//     void SetValue(float v)
//     {
//         clampValue(v);
//         mVal = v;
//     }

//     /**
//      * @brief Returns the current bar value.
//      */
//     float GetValue() const
//     {
//         return mVal;
//     }

//     /**
//      * @brief Render the bar to the screen.
//      *        For simplicity, this example:
//      *         - Fills the entire mRect in gray as a background
//      *         - Fills the "active range" from barOriginValue -> mVal in green
//      */
//     void Render(IMonochromeDisplay *display)
//     {
//         if (!display)
//             return;

//         // --- 1) Draw the background (optional) ---
//         display->FillRectWithBrightness(mRect, 0);

//         // --- 2) Compute the fill segment ---
//         // Convert mVal and mBarOriginValue to [0..1] normalized range
//         float valRatio = normalize(mVal);
//         float originRatio = normalize(mBarOriginValue);

//         // If you want the bar always to go from the smaller ratio to the larger ratio:
//         // (In many designs, the bar can be reversed if the user sets barOriginValue > mVal.)
//         // But if you *want* to keep direction consistent, you can skip sorting them.
//         float startRatio = originRatio;
//         float endRatio = valRatio;

//         // We'll draw the filled portion from "startRatio" to "endRatio".
//         // They might be swapped if barOriginValue > mVal => let's handle that:
//         if (endRatio < startRatio)
//             std::swap(startRatio, endRatio);

//         // --- 3) Figure out pixel coordinates based on orientation ---
//         // We'll fill in fillRect{x, y, w, h} accordingly.
//         int fillX = mRect.x;
//         int fillY = mRect.y;
//         int fillW = mRect.width;
//         int fillH = mRect.height;

//         switch (mOrientation)
//         {
//         case LeftToRight: {
//             // The bar grows from left (mRect.x) to right (mRect.x + mRect.w)
//             // startRatio -> endRatio is the portion of the width
//             // y is always the same
//             int x1 = mRect.x + int(startRatio * mRect.width);
//             int x2 = mRect.x + int(endRatio * mRect.width);
//             fillX = std::min(x1, x2);
//             fillW = std::abs(x2 - x1);
//             fillY = mRect.y;
//             fillH = mRect.height;
//         }
//         break;

//         case RightToLeft: {
//             // The bar grows from right to left,
//             // so ratio=0 means the right edge, ratio=1 means the left edge
//             // we can invert the ratio by (1.0 - ratio), or do direct logic:
//             int x1 = mRect.x + int((1.0f - startRatio) * mRect.width);
//             int x2 = mRect.x + int((1.0f - endRatio) * mRect.width);
//             fillX = std::min(x1, x2);
//             fillW = std::abs(x2 - x1);
//             fillY = mRect.y;
//             fillH = mRect.height;
//         }
//         break;

//         case TopToBottom: {
//             // The bar grows from top to bottom
//             int y1 = mRect.y + int(startRatio * mRect.height);
//             int y2 = mRect.y + int(endRatio * mRect.height);
//             fillY = std::min(y1, y2);
//             fillH = std::abs(y2 - y1);
//             fillX = mRect.x;
//             fillW = mRect.width;
//         }
//         break;

//         case BottomToTop: {
//             // The bar grows from bottom to top
//             int y1 = mRect.y + int((1.0f - startRatio) * mRect.height);
//             int y2 = mRect.y + int((1.0f - endRatio) * mRect.height);
//             fillY = std::min(y1, y2);
//             fillH = std::abs(y2 - y1);
//             fillX = mRect.x;
//             fillW = mRect.width;
//         }
//         break;
//         }

//         // --- 4) Draw the filled portion ---
//         // display->SetColor(0, 255, 0); // green, for example
//         display->FillRectWithBrightness(RectI::Construct(fillX, fillY, fillW, fillH), 128);
//     }

//   private:
//     RectI mRect;              ///< Screen space rectangle
//     float mMin;               ///< Minimum input value
//     float mMax;               ///< Maximum input value
//     Orientation mOrientation; ///< Bar orientation
//     float mBarOriginValue;    ///< “Start” of the filled portion
//     float mVal;               ///< Current "end" of the filled portion

//     /**
//      * @brief Clamps the input value into [mMin, mMax].
//      */
//     void clampValue(float &v) const
//     {
//         if (v < mMin)
//             v = mMin;
//         if (v > mMax)
//             v = mMax;
//     }

//     /**
//      * @brief Converts a raw value in [mMin, mMax] to [0, 1].
//      */
//     float normalize(float v) const
//     {
//         if (mMax == mMin)
//             return 0.0f; // Avoid divide-by-zero
//         return (v - mMin) / (mMax - mMin);
//     }
// };

} // namespace clarinoid