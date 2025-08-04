#pragma once

#include "../Numeric.hpp"

//#include <experimental/optional>
#include <limits>
#include <type_traits>
#include <utility>


namespace clarinoid
{

template <typename T>
struct Vec2
{
  static_assert(is_scalar_like<T>::value, "Vec2<T>: T must be scalar-like");

  T x{}, y{};

  using value_type = T;

  constexpr Vec2() = default;
  constexpr Vec2(T x_, T y_)
      : x(x_)
      , y(y_)
  {
  }
  explicit constexpr Vec2(T v)
      : x(v)
      , y(v)
  {
  }

  constexpr Vec2(const std::array<T, 2>& a)
      : x(a[0])
      , y(a[1])
  {
  }

  template <class U, class = std::enable_if_t<std::is_convertible<U, T>::value>>
  explicit constexpr Vec2(const Vec2<U>& o)
      : x(static_cast<T>(o.x))
      , y(static_cast<T>(o.y))
  {
  }

  // ----------------- element access -----------------
  constexpr T& operator[](std::size_t i) noexcept
  {
    return (i == 0) ? x : y;
  }
  constexpr const T& operator[](std::size_t i) const noexcept
  {
    return (i == 0) ? x : y;
  }
  constexpr T* data() noexcept
  {
    return &x;
  }
  constexpr const T* data() const noexcept
  {
    return &x;
  }
  constexpr T* begin() noexcept
  {
    return &x;
  }
  constexpr const T* begin() const noexcept
  {
    return &x;
  }
  constexpr T* end() noexcept
  {
    return (&y) + 1;
  }
  constexpr const T* end() const noexcept
  {
    return (&y) + 1;
  }

  // ----------------- unary -----------------
  [[nodiscard]] constexpr Vec2 operator+() const noexcept
  {
    return *this;
  }
  [[nodiscard]] constexpr Vec2 operator-() const noexcept
  {
    return {-x, -y};
  }

  // ----------------- basic arithmetic -----------------
  [[nodiscard]] constexpr Vec2 operator+(const Vec2& v) const noexcept
  {
    return {x + v.x, y + v.y};
  }
  [[nodiscard]] constexpr Vec2 operator-(const Vec2& v) const noexcept
  {
    return {x - v.x, y - v.y};
  }
  [[nodiscard]] constexpr Vec2 operator*(T s) const noexcept
  {
    return {x * s, y * s};
  }
  [[nodiscard]] constexpr Vec2 operator/(T s) const noexcept
  {
    return {x / s, y / s};
  }

  [[nodiscard]] constexpr Vec2& operator+=(const Vec2& v) noexcept
  {
    x += v.x;
    y += v.y;
    return *this;
  }
  [[nodiscard]] constexpr Vec2& operator-=(const Vec2& v) noexcept
  {
    x -= v.x;
    y -= v.y;
    return *this;
  }
  [[nodiscard]] constexpr Vec2& operator*=(T s) noexcept
  {
    x *= s;
    y *= s;
    return *this;
  }
  [[nodiscard]] constexpr Vec2& operator/=(T s) noexcept
  {
    x /= s;
    y /= s;
    return *this;
  }

  // scalar on the left
  friend constexpr Vec2 operator*(T s, const Vec2& v) noexcept
  {
    return v * s;
  }

  // ----------------- comparisons -----------------
  [[nodiscard]] constexpr bool operator==(const Vec2& v) const noexcept
  {
    return x == v.x && y == v.y;
  }
  [[nodiscard]] constexpr bool operator!=(const Vec2& v) const noexcept
  {
    return !(*this == v);
  }

  // For floats: epsilon-based compare (component-wise)
  template <class U = T, class = std::enable_if_t<std::is_floating_point<U>::value>>
  [[nodiscard]] bool almost_equal(const Vec2& v, U eps = U(4) * std::numeric_limits<U>::epsilon()) const noexcept
  {
    using std::abs;
    return abs(x - v.x) <= eps && abs(y - v.y) <= eps;
  }

  // ----------------- geometry -----------------
  [[nodiscard]] constexpr T dot(const Vec2& v) const noexcept
  {
    return x * v.x + y * v.y;
  }
  [[nodiscard]] constexpr T cross(const Vec2& v) const noexcept
  {
    return x * v.y - y * v.x;
  }  // 2D scalar cross

  [[nodiscard]] constexpr T length2() const noexcept
  {
    return x * x + y * y;
  }

  [[nodiscard]] T length() const noexcept
  {
    using std::hypot;
    return static_cast<T>(hypot(x, y));  // if T is float/double this stays as-is; ints will truncate by design
  }

  [[nodiscard]] Vec2 normalized() const noexcept
  {
    const T len = length();
    return (len > T{0}) ? (*this / len) : *this;
  }

  [[nodiscard]] constexpr bool is_zero(T eps = T{0}) const noexcept
  {
    return (eps == T{0}) ? (x == T{0} && y == T{0}) : (x >= -eps && x <= eps && y >= -eps && y <= eps);
  }

  [[nodiscard]] T distance(const Vec2& v) const noexcept
  {
    using std::hypot;
    return static_cast<T>(hypot(x - v.x, y - v.y));
  }
  [[nodiscard]] constexpr T distance2(const Vec2& v) const noexcept
  {
    const T dx = x - v.x, dy = y - v.y;
    return dx * dx + dy * dy;
  }

  // Angle in radians: [-pi, pi]
  [[nodiscard]] T angle() const noexcept
  {
    using std::atan2;
    return static_cast<T>(atan2(y, x));
  }

  // Signed smallest angle from this to v (radians, [-pi, pi])
  [[nodiscard]] T angle_to(const Vec2& v) const noexcept
  {
    using std::atan2;
    return static_cast<T>(atan2(cross(v), dot(v)));
  }

  // Rotate by angle (radians)
  [[nodiscard]] Vec2 rotated(T radians) const noexcept
  {
    using std::cos;
    using std::sin;
    const T c = static_cast<T>(cos(radians));
    const T s = static_cast<T>(sin(radians));
    return {x * c - y * s, x * s + y * c};
  }
  Vec2& rotate(T radians) noexcept
  {
    return (*this = this->rotated(radians));
  }

  // 90-degree left/right perpendiculars
  [[nodiscard]] constexpr Vec2 perp_ccw() const noexcept
  {
    return {-y, x};
  }  // +90�
  [[nodiscard]] constexpr Vec2 perp_cw() const noexcept
  {
    return {y, -x};
  }  // -90�

  // Projection of this onto v, and rejection (component orthogonal to v)
  [[nodiscard]] Vec2 project_onto(const Vec2& v) const noexcept
  {
    const T denom = v.length2();
    return (denom != T{0}) ? v * (dot(v) / denom) : Vec2{};
  }
  [[nodiscard]] Vec2 reject_from(const Vec2& v) const noexcept
  {
    return *this - project_onto(v);
  }

  // Reflection of this vector about a (non-necessarily unit) normal n
  [[nodiscard]] Vec2 reflect(const Vec2& n) const noexcept
  {
    const T denom = n.length2();
    return (denom != T{0}) ? (*this - n * (T{2} * dot(n) / denom)) : *this;
  }

  // Clamp length to at most max_len (and optionally at least min_len)
  [[nodiscard]] Vec2 clamped_length(T max_len) const noexcept
  {
    const T len = length();
    return (len > max_len && len > T{0}) ? (*this * (max_len / len)) : *this;
  }
  [[nodiscard]] Vec2 clamped_length(T min_len, T max_len) const noexcept
  {
    const T len = length();
    if (len == T{0})
      return *this;
    if (len < min_len)
      return *this * (min_len / len);
    if (len > max_len)
      return *this * (max_len / len);
    return *this;
  }

  template <class U = T, class = std::enable_if_t<std::is_floating_point<U>::value>>
  [[nodiscard]] Vec2 floor() const noexcept
  {
    using std::floor;
    return {static_cast<T>(floor(x)), static_cast<T>(floor(y))};
  }
  template <class U = T, class = std::enable_if_t<std::is_floating_point<U>::value>>
  [[nodiscard]] Vec2 ceil() const noexcept
  {
    using std::ceil;
    return {static_cast<T>(ceil(x)), static_cast<T>(ceil(y))};
  }
  template <class U = T, class = std::enable_if_t<std::is_floating_point<U>::value>>
  [[nodiscard]] Vec2 round() const noexcept
  {
    using std::round;
    return {static_cast<T>(round(x)), static_cast<T>(round(y))};
  }
  [[nodiscard]] constexpr Vec2 abs() const noexcept
  {
    using std::abs;
    return {static_cast<T>(abs(x)), static_cast<T>(abs(y))};
  }

  [[nodiscard]] static constexpr Vec2 min(const Vec2& a, const Vec2& b) noexcept
  {
    return {(a.x < b.x ? a.x : b.x), (a.y < b.y ? a.y : b.y)};
  }
  [[nodiscard]] static constexpr Vec2 max(const Vec2& a, const Vec2& b) noexcept
  {
    return {(a.x > b.x ? a.x : b.x), (a.y > b.y ? a.y : b.y)};
  }

  [[nodiscard]] static constexpr Vec2 clamp(const Vec2& v, const Vec2& lo, const Vec2& hi) noexcept
  {
    return Vec2::min(Vec2::max(v, lo), hi);
  }

  // ----------------- polar helpers -----------------
  [[nodiscard]] static Vec2 from_polar(T radius, T radians) noexcept
  {
    using std::cos;
    using std::sin;
    return {static_cast<T>(radius * cos(radians)), static_cast<T>(radius * sin(radians))};
  }
  [[nodiscard]] static Vec2 from_angle(T radians) noexcept
  {
    return from_polar(T{1}, radians);
  }

  // Rotate around a pivot point
  [[nodiscard]] Vec2 rotated_around(const Vec2& pivot, T radians) const noexcept
  {
    return ((*this - pivot).rotated(radians) + pivot);
  }

  // ----------------- conversions & utilities -----------------
  [[nodiscard]] constexpr std::array<T, 2> to_array() const noexcept
  {
    return {x, y};
  }

  void swap(Vec2& other) noexcept
  {
    auto tmp = *this;
    *this = other;
    other = tmp;
  }

  // ----------------- constants -----------------
  [[nodiscard]] static constexpr Vec2 zero() noexcept
  {
    return {T{0}, T{0}};
  }
  [[nodiscard]] static constexpr Vec2 one() noexcept
  {
    return {T{1}, T{1}};
  }
};

}  // namespace clarinoid
