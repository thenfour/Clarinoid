#pragma once

#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include "../FixedPoint.hpp"

namespace clarinoid {

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
struct Point
{
  static_assert(is_scalar_like<T>::value, "Point<T>: T must be scalar-like");

  T x;
  T y;

  Point()
    : x(static_cast<T>(0))
    , y(static_cast<T>(0))
  {
  }
  Point(T x_, T y_)
    : x(x_)
    , y(y_)
  {
  }

  template<typename TOther>
  Point<TOther> Cast() const
  {
    return { static_cast<TOther>(x), static_cast<TOther>(y) };
  }

  static Point Construct(T x_, T y_) { return { x_, y_ }; }

  T LengthSquared() const { return x * x + y * y; }
  // float Length() const { return sqrt(LengthSquared()); }

  // vector ops...? vector3?

  Point WithX(T x_) const { return { x_, y }; }
  Point WithY(T y_) const { return { x, y_ }; }
  Point WithXOffset(T dx) const { return { x + dx, y }; }
  Point WithYOffset(T dy) const { return { x, y + dy }; }
  Point WithOffset(T dx, T dy) const { return { x + dx, y + dy }; }
  Point WithOffset(const Point<T>& pt) const { return { x + pt.x, y + pt.y }; }
};
//
//using PointI = Point<int>;
//using PointI16 = Point<int16_t>;
//using PointF = Point<float>;

} // namespace clarinoid
