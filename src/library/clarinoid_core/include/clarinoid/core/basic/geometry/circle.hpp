#pragma once

// #include <algorithm>
// #include <cmath>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>

#include "../Numeric.hpp"

namespace clarinoid
{

template <typename T>
struct Circle
{
  static_assert(is_scalar_like<T>::value, "Circle<T>: T must be scalar-like");

  Vec2<T> mCenter{};
  T mRadius{};  // invariant: r >= 0 for signed T; always true for unsigned T

  // construction
  constexpr Circle() = default;
  constexpr Circle(const Vec2<T>& c, T r)
      : mCenter(c)
      , mRadius(r)
  {
    if constexpr (std::is_signed_v<T>)
    { /* assert(r >= T{0}); */
    }
  }

  // basic queries
  [[nodiscard]] constexpr Vec2<T> center() const noexcept
  {
    return mCenter;
  }
  [[nodiscard]] constexpr T radius() const noexcept
  {
    return mRadius;
  }
  [[nodiscard]] constexpr T diameter() const noexcept
  {
    return mRadius * T{2};
  }
  [[nodiscard]] constexpr bool empty() const noexcept
  {
    if constexpr (std::is_signed_v<T>)
      return mRadius <= T{0};
    else
      return mRadius == T{0};
  }

  // metrics
  [[nodiscard]] T area() const noexcept
  {
    //using std::numbers::pi_v;
    using ::clarinoid::pi_v;
    return pi_v<T> * mRadius * mRadius;
  }
  [[nodiscard]] T circumference() const noexcept
  {
    //using std::numbers::pi_v;
    using ::clarinoid::pi_v;
    return T{2} * pi_v<T> * mRadius;
  }

  // containment
  [[nodiscard]] bool contains(const Vec2<T>& p) const noexcept
  {
    return distance2(p, mCenter) <= mRadius * mRadius;
  }
  [[nodiscard]] bool contains(const Circle& other) const noexcept
  {
    const T d = distance(mCenter, other.mCenter);
    return d + other.mRadius <= mRadius;
  }

  // intersection tests (inclusive of tangency by default)
  [[nodiscard]] bool intersects(const Circle& other) const noexcept
  {
    const T d2 = distance2(mCenter, other.mCenter);
    const T sum = mRadius + other.mRadius;
    if (d2 > sum * sum)
      return false;  // too far
    const T diff = (mRadius > other.mRadius) ? (mRadius - other.mRadius) : (other.mRadius - mRadius);
    if (d2 < diff * diff)
      return false;  // one completely inside without touching
    return true;     // overlapping or tangent
  }
  [[nodiscard]] bool overlaps_area(const Circle& other) const noexcept
  {
    const T d = distance(mCenter, other.mCenter);
    return (d < mRadius + other.mRadius) && (d > std::abs(mRadius - other.mRadius));
  }
  [[nodiscard]] bool tangent_to(const Circle& other) const noexcept
  {
    const T d = distance(mCenter, other.mCenter);
    return d == mRadius + other.mRadius || d == std::abs(mRadius - other.mRadius);
  }

  // distances to points
  [[nodiscard]] T signed_distance(const Vec2<T>& p) const noexcept
  {
    return distance(p, mCenter) - mRadius;  // inside: negative; on: 0; outside: positive
  }
  [[nodiscard]] T distance_to(const Vec2<T>& p) const noexcept
  {
    const T s = signed_distance(p);
    return (s > T{0}) ? s : T{0};
  }
  [[nodiscard]] Vec2<T> closest_point(const Vec2<T>& p) const noexcept
  {
    const Vec2<T> v = p - mCenter;
    const T len = v.length();
    if (len <= mRadius || len == T{0})
      return p;  // already inside/on, or degenerate
    return mCenter + v * (mRadius / len);
  }

  // bounding box (AABB): min, max
  [[nodiscard]] constexpr std::pair<Vec2<T>, Vec2<T>> aabb() const noexcept
  {
    const Vec2<T> r{mRadius, mRadius};
    return {mCenter - r, mCenter + r};
  }

  // transforms
  [[nodiscard]] constexpr Circle translated(const Vec2<T>& delta) const noexcept
  {
    return {mCenter + delta, mRadius};
  }
  [[nodiscard]] constexpr Circle with_center(const Vec2<T>& c) const noexcept
  {
    return {c, mRadius};
  }
  [[nodiscard]] constexpr Circle with_radius(T r) const noexcept
  {
    return {mCenter, (r > T{0}) ? r : T{0}};
  }

  // uniform scale about origin or pivot (radius scales by |s|)
  [[nodiscard]] constexpr Circle scaled(T s) const noexcept
  {
    const T ar = (s >= T{0}) ? (mRadius * s) : (mRadius * -s);
    return {mCenter * s, ar};
  }

  // parameterization & angles
  // point at angle 'theta' (radians)
  template <typename U>
  [[nodiscard]] Vec2<std::common_type_t<T, U>> point_at(U theta) const noexcept
  {
    using R = std::common_type_t<T, U>;
    R ct = std::cos(static_cast<R>(theta));
    R st = std::sin(static_cast<R>(theta));
    return {static_cast<R>(mCenter.x) + static_cast<R>(mRadius) * ct,
            static_cast<R>(mCenter.y) + static_cast<R>(mRadius) * st};
  }
  // angle of a point relative to center (radians)
  template <typename U>
  [[nodiscard]] U angle_of(const Vec2<U>& p) const noexcept
  {
    using std::atan2;
    return atan2(p.y - static_cast<U>(mCenter.y), p.x - static_cast<U>(mCenter.x));
  }

  // circle-circle minimal covering circle (convex hull in 2D for two disks)
  [[nodiscard]] Circle united(const Circle& b) const noexcept
  {
    const Circle& a = *this;
    // If one contains the other, return the bigger one
    const T d = distance(a.mCenter, b.mCenter);
    if (a.mRadius >= b.mRadius + d)
      return a;
    if (b.mRadius >= a.mRadius + d)
      return b;

    // Otherwise, minimal enclosing circle of two circles:
    // center lies on the line segment between centers.
    const T newR = (d + a.mRadius + b.mRadius) / T{2};
    const T t = (newR - a.mRadius) / d;  // fraction from a.center towards b.center
    const Vec2<T> newC = {a.mCenter.x + (b.mCenter.x - a.mCenter.x) * t, a.mCenter.y + (b.mCenter.y - a.mCenter.y) * t};
    return {newC, newR};
  }

  // intersection points (if any). Returns:
  //  - empty -> no intersection
  //  - one point duplicated when tangent
  //  - two points otherwise
  [[nodiscard]] std::optional<std::pair<Vec2<T>, Vec2<T>>> intersection_points(const Circle& o) const noexcept
  {
    const Vec2<T> dC = o.mCenter - mCenter;
    const T d = dC.length();
    const T r0 = mRadius, r1 = o.mRadius;

    if (d > r0 + r1 || d < std::abs(r0 - r1) || (d == T{0} && r0 == r1))
    {
      return std::nullopt;  // separate, contained w/out touching, or coincident infinite points
    }

    // a = distance from c0 to chord center along the center line
    const T a = (r0 * r0 - r1 * r1 + d * d) / (T{2} * d);
    const T h2 = r0 * r0 - a * a;
    const T h = (h2 > T{0}) ? std::sqrt(h2) : T{0};

    const Vec2<T> dir = (d > T{0}) ? (dC / d) : Vec2<T>{T{1}, T{0}};  // fallback direction
    const Vec2<T> p0 = mCenter + dir * a;

    // perpendicular vector
    const Vec2<T> perp{-dir.y, dir.x};
    const Vec2<T> i1 = p0 + perp * h;
    const Vec2<T> i2 = p0 - perp * h;
    return std::make_pair(i1, i2);
  }

  // Fit / clamp inside an axis-aligned rectangle given by (min,max).
  // If shrink=true and it doesn't fit, shrink radius; else translate only.
  [[nodiscard]] Circle clamped_in_rect(const Vec2<T>& rmin, const Vec2<T>& rmax, bool shrink = true) const noexcept
  {
    Circle out = *this;
    // First translate center so the disk lies inside when possible
    const T left = (out.mCenter.x - out.mRadius) - rmin.x;
    const T right = rmax.x - (out.mCenter.x + out.mRadius);
    const T bottom = (out.mCenter.y - out.mRadius) - rmin.y;
    const T top = rmax.y - (out.mCenter.y + out.mRadius);

    if (left < T{0})
      out.mCenter.x -= left;
    if (right < T{0})
      out.mCenter.x += right;
    if (bottom < T{0})
      out.mCenter.y -= bottom;
    if (top < T{0})
      out.mCenter.y += top;

    if (shrink)
    {
      const T maxR = std::min(
          {out.mCenter.x - rmin.x, rmax.x - out.mCenter.x, out.mCenter.y - rmin.y, rmax.y - out.mCenter.y});
      if (maxR < out.mRadius)
        out.mRadius = std::max(T{0}, maxR);
    }
    return out;
  }

  // comparisons & hashing friendliness
  //[[nodiscard]] constexpr bool operator==(const Circle&) const = default;
  [[nodiscard]] constexpr bool operator<(const Circle& o) const noexcept
  {
    return (mCenter.x < o.mCenter.x) ||
           (mCenter.x == o.mCenter.x && (mCenter.y < o.mCenter.y || (mCenter.y == o.mCenter.y && mRadius < o.mRadius)));
  }
};

}  // namespace clarinoid
