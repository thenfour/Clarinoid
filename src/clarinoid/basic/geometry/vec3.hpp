#pragma once

#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include "../Numeric.hpp"

namespace clarinoid {

template<typename T>
struct Vec3
{
  static_assert(is_scalar_like<T>::value, "Vec3<T>: T must be scalar-like");
  using value_type = T;

  // ----------------- data -----------------
  T x{}, y{}, z{};

  // ----------------- ctors -----------------
  constexpr Vec3() = default;
  constexpr Vec3(T x_, T y_, T z_)
    : x(x_)
    , y(y_)
    , z(z_)
  {
  }
  explicit constexpr Vec3(T v)
    : x(v)
    , y(v)
    , z(v)
  {
  } // broadcast
  constexpr Vec3(const std::array<T, 3>& a)
    : x(a[0])
    , y(a[1])
    , z(a[2])
  {
  } // from array

  template<class U, class = std::enable_if_t<std::is_convertible<U, T>::value>>
  explicit constexpr Vec3(const Vec3<U>& o)
    : x(static_cast<T>(o.x))
    , y(static_cast<T>(o.y))
    , z(static_cast<T>(o.z))
  {
  }

  // ----------------- element access/iterators -----------------
  constexpr T& operator[](std::size_t i) noexcept { return i == 0 ? x : (i == 1 ? y : z); }
  constexpr const T& operator[](std::size_t i) const noexcept { return i == 0 ? x : (i == 1 ? y : z); }
  constexpr T* data() noexcept { return &x; }
  constexpr const T* data() const noexcept { return &x; }
  constexpr T* begin() noexcept { return &x; }
  constexpr const T* begin() const noexcept { return &x; }
  constexpr T* end() noexcept { return (&z) + 1; }
  constexpr const T* end() const noexcept { return (&z) + 1; }

  // ----------------- unary -----------------
  [[nodiscard]] constexpr Vec3 operator+() const noexcept { return *this; }
  [[nodiscard]] constexpr Vec3 operator-() const noexcept { return { -x, -y, -z }; }

  // ----------------- arithmetic -----------------
  [[nodiscard]] constexpr Vec3 operator+(const Vec3& v) const noexcept { return { x + v.x, y + v.y, z + v.z }; }
  [[nodiscard]] constexpr Vec3 operator-(const Vec3& v) const noexcept { return { x - v.x, y - v.y, z - v.z }; }
  [[nodiscard]] constexpr Vec3 operator*(T s) const noexcept { return { x * s, y * s, z * s }; }
  [[nodiscard]] constexpr Vec3 operator/(T s) const noexcept { return { x / s, y / s, z / s }; }

  [[nodiscard]] constexpr Vec3& operator+=(const Vec3& v) noexcept
  {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
  }
  [[nodiscard]] constexpr Vec3& operator-=(const Vec3& v) noexcept
  {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
  }
  [[nodiscard]] constexpr Vec3& operator*=(T s) noexcept
  {
    x *= s;
    y *= s;
    z *= s;
    return *this;
  }
  [[nodiscard]] constexpr Vec3& operator/=(T s) noexcept
  {
    x /= s;
    y /= s;
    z /= s;
    return *this;
  }

  friend constexpr Vec3 operator*(T s, const Vec3& v) noexcept { return v * s; }

  // ----------------- comparisons -----------------
  [[nodiscard]] constexpr bool operator==(const Vec3& v) const noexcept { return x == v.x && y == v.y && z == v.z; }
  [[nodiscard]] constexpr bool operator!=(const Vec3& v) const noexcept { return !(*this == v); }

  // Epsilon compare for floats (component-wise)
  template<class U = T, class = std::enable_if_t<std::is_floating_point<U>::value>>
  [[nodiscard]] bool almost_equal(const Vec3& v, U eps = U(6) * std::numeric_limits<U>::epsilon()) const noexcept
  {
    using std::abs;
    return abs(x - v.x) <= eps && abs(y - v.y) <= eps && abs(z - v.z) <= eps;
  }

  // ----------------- geometry -----------------
  [[nodiscard]] constexpr T dot(const Vec3& v) const noexcept { return x * v.x + y * v.y + z * v.z; }
  [[nodiscard]] constexpr Vec3 cross(const Vec3& v) const noexcept
  {
    return { y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x };
  }

  [[nodiscard]] constexpr T length2() const noexcept { return x * x + y * y + z * z; }
  [[nodiscard]] T length() const noexcept
  {
    using std::sqrt;
    return static_cast<T>(sqrt(static_cast<long double>(length2())));
  }

  [[nodiscard]] Vec3 normalized() const noexcept
  {
    const T len = length();
    return (len > T{ 0 }) ? (*this / len) : *this;
  }

  [[nodiscard]] constexpr bool is_zero(T eps = T{ 0 }) const noexcept
  {
    return (eps == T{ 0 }) ? (x == T{ 0 } && y == T{ 0 } && z == T{ 0 })
                           : (x >= -eps && x <= eps && y >= -eps && y <= eps && z >= -eps && z <= eps);
  }

  [[nodiscard]] T distance(const Vec3& v) const noexcept
  {
    using std::sqrt;
    const T dx = x - v.x, dy = y - v.y, dz = z - v.z;
    return static_cast<T>(sqrt(static_cast<long double>(dx * dx + dy * dy + dz * dz)));
  }
  [[nodiscard]] constexpr T distance2(const Vec3& v) const noexcept
  {
    const T dx = x - v.x, dy = y - v.y, dz = z - v.z;
    return dx * dx + dy * dy + dz * dz;
  }

  // Angle between vectors (radians) in [0, pi]
  [[nodiscard]] T angle_between(const Vec3& v) const noexcept
  {
    using std::acos;
    using F = long double;
    const F la = static_cast<F>(length());
    const F lb = static_cast<F>(v.length());
    if (la == F{ 0 } || lb == F{ 0 })
      return T{ 0 };
    F c = static_cast<F>(dot(v)) / (la * lb);
    if (c > F{ 1 })
      c = F{ 1 };
    else if (c < F{ -1 })
      c = F{ -1 };
    return static_cast<T>(acos(c));
  }

  // Oriented smallest angle this->v around given axis (assumes axis != 0)
  [[nodiscard]] T signed_angle_to(const Vec3& v, const Vec3& axis) const noexcept
  {
    using std::atan2;
    const Vec3 a = this->cross(v);
    const T s = axis.dot(a);
    const T c = this->dot(v);
    return static_cast<T>(atan2(static_cast<long double>(s), static_cast<long double>(c)));
  }

  // Projection/rejection relative to v (v may be non-unit)
  [[nodiscard]] Vec3 project_onto(const Vec3& v) const noexcept
  {
    const T denom = v.length2();
    return (denom != T{ 0 }) ? v * (dot(v) / denom) : Vec3{};
  }
  [[nodiscard]] Vec3 reject_from(const Vec3& v) const noexcept { return *this - project_onto(v); }

  // Project onto plane with normal n
  [[nodiscard]] Vec3 project_onto_plane(const Vec3& n) const noexcept
  {
    const T denom = n.length2();
    return (denom != T{ 0 }) ? (*this - n * (dot(n) / denom)) : *this;
  }

  // Reflect about a (possibly non-unit) normal n
  [[nodiscard]] Vec3 reflect(const Vec3& n) const noexcept
  {
    const T denom = n.length2();
    return (denom != T{ 0 }) ? (*this - n * (T{ 2 } * dot(n) / denom)) : *this;
  }

  // Refract through surface with normal n. IOR ratio eta = eta_i / eta_t.
  // Assumes *this is the incident direction and n points "out" of the surface.
  [[nodiscard]] Vec3 refract(const Vec3& n, T eta) const noexcept
  {
    // Based on GLSL refract
    const Vec3 I = this->normalized();
    const Vec3 N = n.normalized();
    const T cosi = -(I.dot(N));
    const T k = T{ 1 } - eta * eta * (T{ 1 } - cosi * cosi);
    if (k < T{ 0 })
      return Vec3{}; // total internal reflection -> zero vector
    return eta * I + (eta * cosi - static_cast<T>(std::sqrt(static_cast<long double>(k)))) * N;
  }

  // Clamp length
  [[nodiscard]] Vec3 clamped_length(T max_len) const noexcept
  {
    const T len = length();
    return (len > max_len && len > T{ 0 }) ? (*this * (max_len / len)) : *this;
  }
  [[nodiscard]] Vec3 clamped_length(T min_len, T max_len) const noexcept
  {
    const T len = length();
    if (len == T{ 0 })
      return *this;
    if (len < min_len)
      return *this * (min_len / len);
    if (len > max_len)
      return *this * (max_len / len);
    return *this;
  }
  [[nodiscard]] Vec3 with_length(T new_len) const noexcept
  {
    const T len = length();
    return (len > T{ 0 }) ? (*this * (new_len / len)) : *this;
  }

  // Move towards target by at most max_delta
  [[nodiscard]] Vec3 move_towards(const Vec3& target, T max_delta) const noexcept
  {
    const T d = distance(target);
    if (d <= max_delta || d == T{ 0 })
      return target;
    return *this + (target - *this) * (max_delta / d);
  }

  // ----------------- rotations -----------------
  // Axis-angle rotation using Rodrigues' formula (axis can be non-unit)
  [[nodiscard]] Vec3 rotated_about_axis(const Vec3& axis, T radians) const noexcept
  {
    using std::sin;
    using std::cos;
    const Vec3 u = axis.normalized();
    const T c = static_cast<T>(cos(radians));
    const T s = static_cast<T>(sin(radians));
    return (*this) * c + u.cross(*this) * s + u * (u.dot(*this) * (T{ 1 } - c));
  }
  Vec3& rotate_about_axis(const Vec3& axis, T radians) noexcept { return (*this = rotated_about_axis(axis, radians)); }

  // Rotate "this" toward "to" by at most max_radians
  [[nodiscard]] Vec3 rotate_towards(const Vec3& to, T max_radians) const noexcept
  {
    const Vec3 a = this->normalized();
    const Vec3 b = to.normalized();
    const T ang = a.angle_between(b);
    if (ang <= max_radians)
      return with_length(length()).normalized() * to.length(); // basically "to" with this magnitude
    const Vec3 ax = a.cross(b);
    if (ax.is_zero())
      return *this; // same or opposite; nothing better to do
    return with_length(length()).rotated_about_axis(ax, max_radians);
  }

  // ----------------- component-wise helpers -----------------
  [[nodiscard]] constexpr Vec3 abs() const noexcept
  {
    using std::abs;
    return { static_cast<T>(abs(x)), static_cast<T>(abs(y)), static_cast<T>(abs(z)) };
  }
  template<class U = T, class = std::enable_if_t<std::is_floating_point<U>::value>>
  [[nodiscard]] Vec3 floor() const noexcept
  {
    using std::floor;
    return { static_cast<T>(floor(x)), static_cast<T>(floor(y)), static_cast<T>(floor(z)) };
  }
  template<class U = T, class = std::enable_if_t<std::is_floating_point<U>::value>>
  [[nodiscard]] Vec3 ceil() const noexcept
  {
    using std::ceil;
    return { static_cast<T>(ceil(x)), static_cast<T>(ceil(y)), static_cast<T>(ceil(z)) };
  }
  template<class U = T, class = std::enable_if_t<std::is_floating_point<U>::value>>
  [[nodiscard]] Vec3 round() const noexcept
  {
    using std::round;
    return { static_cast<T>(round(x)), static_cast<T>(round(y)), static_cast<T>(round(z)) };
  }

  [[nodiscard]] static constexpr Vec3 min(const Vec3& a, const Vec3& b) noexcept
  {
    return { (a.x < b.x ? a.x : b.x), (a.y < b.y ? a.y : b.y), (a.z < b.z ? a.z : b.z) };
  }
  [[nodiscard]] static constexpr Vec3 max(const Vec3& a, const Vec3& b) noexcept
  {
    return { (a.x > b.x ? a.x : b.x), (a.y > b.y ? a.y : b.y), (a.z > b.z ? a.z : b.z) };
  }
  [[nodiscard]] static constexpr Vec3 clamp(const Vec3& v, const Vec3& lo, const Vec3& hi) noexcept
  {
    return Vec3::min(Vec3::max(v, lo), hi);
  }


  // Major/minor axis (index of component with largest/smallest absolute value)
  [[nodiscard]] int major_axis() const noexcept
  {
    const Vec3 a = abs();
    return (a.x >= a.y && a.x >= a.z) ? 0 : (a.y >= a.z ? 1 : 2);
  }
  [[nodiscard]] int minor_axis() const noexcept
  {
    const Vec3 a = abs();
    return (a.x <= a.y && a.x <= a.z) ? 0 : (a.y <= a.z ? 1 : 2);
  }

  // Any unit vector orthogonal to this (or +X if zero)
  [[nodiscard]] Vec3 any_orthogonal() const noexcept
  {
    if (this->is_zero())
      return Vec3::unit_x();
    const int m = minor_axis();
    Vec3 axis = (m == 0)   ? Vec3{ T{ 1 }, T{ 0 }, T{ 0 } }
                : (m == 1) ? Vec3{ T{ 0 }, T{ 1 }, T{ 0 } }
                           : Vec3{ T{ 0 }, T{ 0 }, T{ 1 } };
    return this->cross(axis).normalized();
  }

  // Orthonormal basis: returns (tangent, bitangent, normal) with normal = normalized(*this)
  [[nodiscard]] std::tuple<Vec3, Vec3, Vec3> orthonormal_basis() const noexcept
  {
    const Vec3 n = normalized();
    const Vec3 t = n.any_orthogonal();
    const Vec3 b = n.cross(t);
    return std::make_tuple(t, b, n);
  }

  // Face-forward (GLSL-style): orient this normal to face away from "I", using reference normal "Nref"
  [[nodiscard]] Vec3 face_forward(const Vec3& I, const Vec3& Nref) const noexcept
  {
    return (Nref.dot(I) < T{ 0 }) ? *this : -*this;
  }

  // ----------------- conversions & utilities -----------------
  [[nodiscard]] constexpr std::array<T, 3> to_array() const noexcept { return { x, y, z }; }
  void swap(Vec3& other) noexcept
  {
    auto tmp = *this;
    *this = other;
    other = tmp;
  }

  // ----------------- constants -----------------
  [[nodiscard]] static constexpr Vec3 zero() noexcept { return { T{ 0 }, T{ 0 }, T{ 0 } }; }
  [[nodiscard]] static constexpr Vec3 one() noexcept { return { T{ 1 }, T{ 1 }, T{ 1 } }; }
  [[nodiscard]] static constexpr Vec3 unit_x() noexcept { return { T{ 1 }, T{ 0 }, T{ 0 } }; }
  [[nodiscard]] static constexpr Vec3 unit_y() noexcept { return { T{ 0 }, T{ 1 }, T{ 0 } }; }
  [[nodiscard]] static constexpr Vec3 unit_z() noexcept { return { T{ 0 }, T{ 0 }, T{ 1 } }; }
};

} // namespace clarinoid
