
#pragma once

#include <algorithm>
#include <cmath>

#include "../function.hpp"

namespace clarinoid
{

// wikipedia https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
template <typename T>
inline void drawLine(int x0, int y0, int x1, int y1, T&& drawPixel)
{
  int dx = abs(x1 - x0);
  int sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0);
  int sy = y0 < y1 ? 1 : -1;
  int err = dx + dy; /* error value e_xy */
  while (true)
  { /* loop */
    drawPixel(x0, y0, true);
    if (x0 == x1 && y0 == y1)
      break;
    int e2 = 2 * err;
    if (e2 >= dy)
    { /* e_xy+e_x > 0 */
      err += dy;
      x0 += sx;
    }
    if (e2 <= dx)
    { /* e_xy+e_y < 0 */
      err += dx;
      y0 += sy;
    }
  }
}



}  // namespace clarinoid
