#pragma once

// #include <algorithm>
// #include <cmath>
#include "../Numeric.hpp"
//#include <experimental/optional>
#include <limits>
#include <type_traits>
#include <utility>


namespace clarinoid
{

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct Size
{
  static_assert(is_scalar_like<T>::value, "Size<T>: T must be scalar-like");

  T width;
  T height;

  Size()
      : width(static_cast<T>(0))
      , height(static_cast<T>(0))
  {
  }
  Size(T w_, T h_)
      : width(w_)
      , height(h_)
  {
  }

  static Size Construct(T w_, T h_)
  {
    return {w_, h_};
  }

  bool IsEmpty() const
  {
    return (width <= 0) || (height <= 0);
  }
  T Area() const
  {
    return width * height;
  }
  float AspectRatio() const
  {
    return (height == 0) ? 0.f : float(width) / float(height);
  }
  Size<T> Scale(T factor) const
  {
    return Size{static_cast<T>(width * factor), static_cast<T>(height * factor)};
  }

  Size<T> WithWidth(T w) const
  {
    return {w, height};
  }
  Size<T> WithHeight(T h) const
  {
    return {width, h};
  }

  static Size Square(T s)
  {
    return {s, s};
  }

  bool IsSquare() const
  {
    return width == height;
  }
};
//
//using SizeI = Size<int>;
//using SizeI16 = Size<int16_t>;
//using SizeF = Size<float>;

}  // namespace clarinoid
