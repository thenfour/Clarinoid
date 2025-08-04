
#pragma once

#include <algorithm>
#include <cmath>

#include "../function.hpp"

namespace clarinoid
{

struct PieData
{
  PointF p0;  // represents a0
  PointF p1;  // represents a1
};

// adapted from
// https://stackoverflow.com/questions/58222657/generate-a-pieslice-in-c-without-using-the-pieslice-of-graphics-h
// template<typename T>
inline PieData fillPie(float x0,
                       float y0,
                       float r,
                       float a0,
                       float a1,
                       typename cc::function<void(int, int, void*)>::ptr_t drawPixel,
                       void* capture)  // a0 < a1
{
  float x, y,      // circle centered point
      xx, yy, rr,  // x^2,y^2,r^2
      ux, uy,      // u
      vx, vy,      // v
      sx, sy;      // pixel position
  rr = r * r;
  ux = (r)*fast::cos(a0);
  uy = (r)*fast::sin(a0);
  vx = (r)*fast::cos(a1);
  vy = (r)*fast::sin(a1);
  PieData ret;
  ret.p0 = PointF::Construct(ux, uy);
  ret.p1 = PointF::Construct(vx, vy);
  // handle big/small pies
  x = a1 - a0;
  if (x < 0)
    x = -x;
  // render small pies
  int pixelsDrawn = 0;
  if (x < gPI<float>) /* 180 deg */
  {
    for (y = -r, yy = y * y, sy = y0 + y; y <= +r; y++, yy = y * y, sy++)
    {
      for (x = -r, xx = x * x, sx = x0 + x; x <= +r; x++, xx = x * x, sx++)
      {
        if (xx + yy <= rr)                    // inside circle
          if (((x * uy) - (y * ux) <= 0)      // x,y is above a0 in clockwise direction
              && ((x * vy) - (y * vx) >= 0))  // x,y is below a1 in counter clockwise direction
          {
            drawPixel((int)::floorf(sx), (int)::floorf(sy), capture);
            ++pixelsDrawn;
          }
      }
    }
    // drawLine(x0, y0, x0 + ux, y0 + uy, drawPixel);
  }
  else
  {
    for (y = -r, yy = y * y, sy = y0 + y; y <= +r; y++, yy = y * y, sy++)
    {
      for (x = -r, xx = x * x, sx = x0 + x; x <= +r; x++, xx = x * x, sx++)
      {
        if (xx + yy <= rr)
        {                                     // inside circle
          if (((x * uy) - (y * ux) <= 0)      // x,y is above a0 in clockwise direction
              || ((x * vy) - (y * vx) >= 0))  // x,y is below a1 in counter clockwise direction
          {
            drawPixel((int)::floorf(sx), (int)::floorf(sy), capture);
            ++pixelsDrawn;
          }
        }
      }
    }
  }
  return ret;
}

}  // namespace clarinoid
