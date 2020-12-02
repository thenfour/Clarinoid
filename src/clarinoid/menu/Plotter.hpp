#pragma once

#include <clarinoid/basic/Basic.hpp>
#include "MenuAppBase.hpp"
#include "MenuListControl.hpp"

namespace clarinoid
{


// // feed it data and it will plot.
// // supports also 1 "boolean" series which plots solid when true, nothing when off.
// template<int TseriesCount, int Tspeed> // Tspeed is # of plots per column
// struct Plotter
// {
//   static constexpr int DisplayWidth = RESOLUTION_X;
//   static constexpr int DisplayHeight = RESOLUTION_Y;
//   size_t mCursor = 0;
//   size_t mValid = 0;
//   static constexpr size_t sampleCount = DisplayWidth * Tspeed;
//   int32_t vals[TseriesCount][sampleCount];
//   bool boolVals[sampleCount];

//   Plotter() {
//     for (auto& b : boolVals) {
//       b = false;      
//     }
//   }
  
//   void clear() {
//     mValid = 0;
//     mCursor = 0;
//   }
  
//   void Plot4(uint32_t val1, uint32_t val2, uint32_t val3, uint32_t val4) {
//     CCASSERT(TseriesCount == 4);
//     vals[0][mCursor] = val1;
//     vals[1][mCursor] = val2;
//     vals[2][mCursor] = val3;
//     vals[3][mCursor] = val4;
//     mValid = max(mValid, mCursor);
//     mCursor = (mCursor + 1) % sampleCount;
//   }
  
//   void Plot3b(uint32_t val1, uint32_t val2, uint32_t val3, bool boolVal) {
//     CCASSERT(TseriesCount == 3);
//     vals[0][mCursor] = val1;
//     vals[1][mCursor] = val2;
//     vals[2][mCursor] = val3;
//     boolVals[mCursor] = boolVal;
//     mValid = max(mValid, mCursor);
//     mCursor = (mCursor + 1) % sampleCount;
//   }
  
//   void Plot1(uint32_t val1) {
//     CCASSERT(TseriesCount == 1);
//     vals[0][mCursor] = val1;
//     mValid = max(mValid, mCursor);
//     mCursor = (mCursor + 1) % sampleCount;
//   }
  
//   void Render() {
//     // determine min/max for scale.
//     if (mValid == 0) return;
//     int32_t min_ = vals[0][0];
//     int32_t max_ = min_;
//     for (size_t x = 0; x < mValid; ++ x) {
//       for (size_t s = 0; s < TseriesCount; ++ s) {
//         min_ = min(min_, vals[s][x]);
//         max_ = max(max_, vals[s][x]);
//       }
//     }
//     if (min_ == max_) max_ ++; // avoid div0

//     // draw back from cursor.
//     for (int n = 0; n < (int)mValid; ++ n) {
//       int x = ((sampleCount - n) / Tspeed) - 1;
//       int i = (int)mCursor - n - 1;
//       if (i < 0)
//         i += sampleCount;
//       for (size_t s = 0; s < TseriesCount; ++ s) {
//         uint32_t y = map(vals[s][i], min_, max_, DisplayHeight - 1, 0);
//         gDisplay.mDisplay.drawPixel(x, y, WHITE);
//       }

//       // plot bool val
//       if (boolVals[i]) {
//         gDisplay.mDisplay.DrawDottedRect(x, DisplayHeight - 4, 1, RESOLUTION_Y, WHITE);
//       }
//     }
    
//   }
// };

} // namespace clarinoid
