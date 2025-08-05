#pragma once

#include "../Numeric.hpp"
#include <array>
#include <cmath>
#include <cstddef>   // offsetof
#include <iterator>  // iterator tags
#include <limits>
#include <ostream>
#include <type_traits>

namespace clarinoid
{

template <typename T>
struct Vec4
{
  static_assert(is_scalar_like<T>::value, "Vec4<T>: T must be scalar-like");
  using value_type = T;

  // ----------------- data -----------------
  T x{}, y{}, z{}, w{};

  // ----------------- ctors -----------------
  constexpr Vec4() = default;
  constexpr Vec4(T x_, T y_, T z_, T w_)
      : x(x_)
      , y(y_)
      , z(z_)
      , w(w_)
  {
  }
  explicit constexpr Vec4(T v)
      : x(v)
      , y(v)
      , z(v)
      , w(v)
  {
  }  // broadcast
  constexpr Vec4(const std::array<T, 4>& a)
      : x(a[0])
      , y(a[1])
      , z(a[2])
      , w(a[3])
  {
  }

  template <class U, class = std::enable_if_t<std::is_convertible<U, T>::value>>
  explicit constexpr Vec4(const Vec4<U>& o)
      : x(static_cast<T>(o.x))
      , y(static_cast<T>(o.y))
      , z(static_cast<T>(o.z))
      , w(static_cast<T>(o.w))
  {
  }

  // ----------------- element access -----------------
  constexpr T& operator[](std::size_t i) noexcept
  {
    return i == 0 ? x : (i == 1 ? y : (i == 2 ? z : w));
  }
  constexpr const T& operator[](std::size_t i) const noexcept
  {
    return i == 0 ? x : (i == 1 ? y : (i == 2 ? z : w));
  }

  // Optional contiguous view (guarded): fails to compile if not tightly packed
  //constexpr T* data() noexcept
  //{
  //  static_assert(std::is_standard_layout<Vec4>::value, "Vec4 must be standard-layout");
  //  static_assert(offsetof(Vec4, y) == sizeof(T) && offsetof(Vec4, z) == 2 * sizeof(T) &&
  //                    offsetof(Vec4, w) == 3 * sizeof(T),
  //                "Vec4 must be tightly packed to use data()");
  //  return &x;
  //}
  //constexpr const T* data() const noexcept
  //{
  //  static_assert(std::is_standard_layout<Vec4>::value, "Vec4 must be standard-layout");
  //  static_assert(offsetof(Vec4, y) == sizeof(T) && offsetof(Vec4, z) == 2 * sizeof(T) &&
  //                    offsetof(Vec4, w) == 3 * sizeof(T),
  //                "Vec4 must be tightly packed to use data()");
  //  return &x;
  //}

  // Safe iterators that don't assume contiguity
  //template <bool Const>
  //struct elem_iter
  //{
  //  using vec_t = std::conditional_t<Const, const Vec4, Vec4>;
  //  vec_t* v{};
  //  int i{};
  //  using iterator_category = std::forward_iterator_tag;
  //  using value_type = T;
  //  using difference_type = std::ptrdiff_t;
  //  using reference = std::conditional_t<Const, const T&, T&>;
  //  using pointer = std::conditional_t<Const, const T*, T*>;
  //  reference operator*() const
  //  {
  //    return i == 0 ? v->x : (i == 1 ? v->y : (i == 2 ? v->z : v->w));
  //  }
  //  elem_iter& operator++()
  //  {
  //    ++i;
  //    return *this;
  //  }
  //  bool operator==(const elem_iter& o) const
  //  {
  //    return v == o.v && i == o.i;
  //  }
  //  bool operator!=(const elem_iter& o) const
  //  {
  //    return !(*this == o);
  //  }
  //};
  //using iterator = elem_iter<false>;
  //using const_iterator = elem_iter<true>;
  //iterator begin() noexcept
  //{
  //  return {this, 0};
  //}
  //iterator end() noexcept
  //{
  //  return {this, 4};
  //}
  //const_iterator begin() const noexcept
  //{
  //  return {this, 0};
  //}
  //const_iterator end() const noexcept
  //{
  //  return {this, 4};
  //}
  //const_iterator cbegin() const noexcept
  //{
  //  return {this, 0};
  //}
  //const_iterator cend() const noexcept
  //{
  //  return {this, 4};
  //}

  // ----------------- unary -----------------
  [[nodiscard]] constexpr Vec4 operator+() const noexcept
  {
    return *this;
  }
  [[nodiscard]] constexpr Vec4 operator-() const noexcept
  {
    return {-x, -y, -z, -w};
  }

  // ----------------- arithmetic -----------------
  [[nodiscard]] constexpr Vec4 operator+(const Vec4& v) const noexcept
  {
    return {x + v.x, y + v.y, z + v.z, w + v.w};
  }
  [[nodiscard]] constexpr Vec4 operator-(const Vec4& v) const noexcept
  {
    return {x - v.x, y - v.y, z - v.z, w - v.w};
  }
  [[nodiscard]] constexpr Vec4 operator*(T s) const noexcept
  {
    return {x * s, y * s, z * s, w * s};
  }
  [[nodiscard]] constexpr Vec4 operator/(T s) const noexcept
  {
    return {x / s, y / s, z / s, w / s};
  }

  [[nodiscard]] constexpr Vec4& operator+=(const Vec4& v) noexcept
  {
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
    return *this;
  }
  [[nodiscard]] constexpr Vec4& operator-=(const Vec4& v) noexcept
  {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;
    return *this;
  }
  [[nodiscard]] constexpr Vec4& operator*=(T s) noexcept
  {
    x *= s;
    y *= s;
    z *= s;
    w *= s;
    return *this;
  }
  [[nodiscard]] constexpr Vec4& operator/=(T s) noexcept
  {
    x /= s;
    y /= s;
    z /= s;
    w /= s;
    return *this;
  }
  friend constexpr Vec4 operator*(T s, const Vec4& v) noexcept
  {
    return v * s;
  }

  // ----------------- comparisons -----------------
  [[nodiscard]] constexpr bool operator==(const Vec4& v) const noexcept
  {
    return x == v.x && y == v.y && z == v.z && w == v.w;
  }
  [[nodiscard]] constexpr bool operator!=(const Vec4& v) const noexcept
  {
    return !(*this == v);
  }

  template <class U = T, class = std::enable_if_t<std::is_floating_point<U>::value>>
  [[nodiscard]] bool almost_equal(const Vec4& v, U eps = U(8) * std::numeric_limits<U>::epsilon()) const noexcept
  {
    using std::abs;
    return abs(x - v.x) <= eps && abs(y - v.y) <= eps && abs(z - v.z) <= eps && abs(w - v.w) <= eps;
  }

  // ----------------- geometry (N-D friendly) -----------------
  [[nodiscard]] constexpr T dot(const Vec4& v) const noexcept
  {
    return x * v.x + y * v.y + z * v.z + w * v.w;
  }

  [[nodiscard]] constexpr T length2() const noexcept
  {
    return x * x + y * y + z * z + w * w;
  }
  //[[nodiscard]] T length() const noexcept
  //{
  //  using std::sqrt;
  //  using F = long double;
  //  return static_cast<T>(sqrt(static_cast<F>(length2())));
  //}

  //[[nodiscard]] Vec4 normalized() const noexcept
  //{
  //  const T len = length();
  //  return (len > T{0}) ? (*this / len) : *this;
  //}

  [[nodiscard]] constexpr bool is_zero(T eps = T{0}) const noexcept
  {
    return (eps == T{0})
               ? (x == T{0} && y == T{0} && z == T{0} && w == T{0})
               : (x >= -eps && x <= eps && y >= -eps && y <= eps && z >= -eps && z <= eps && w >= -eps && w <= eps);
  }

  //[[nodiscard]] T distance(const Vec4& v) const noexcept
  //{
  //  using std::sqrt;
  //  const T dx = x - v.x, dy = y - v.y, dz = z - v.z, dw = w - v.w;
  //  return static_cast<T>(sqrt(static_cast<long double>(dx * dx + dy * dy + dz * dz + dw * dw)));
  //}
  [[nodiscard]] constexpr T distance2(const Vec4& v) const noexcept
  {
    const T dx = x - v.x, dy = y - v.y, dz = z - v.z, dw = w - v.w;
    return dx * dx + dy * dy + dz * dz + dw * dw;
  }

  //// Angle between vectors in [0, pi]
  //[[nodiscard]] T angle_between(const Vec4& v) const noexcept
  //{
  //  using std::acos;
  //  using F = long double;
  //  const F la = static_cast<F>(length());
  //  const F lb = static_cast<F>(v.length());
  //  if (la == F{0} || lb == F{0})
  //    return T{0};
  //  F c = static_cast<F>(dot(v)) / (la * lb);
  //  if (c > F{1})
  //    c = F{1};
  //  else if (c < F{-1})
  //    c = F{-1};
  //  return static_cast<T>(acos(c));
  //}

  //// Projection/rejection relative to v (v may be non-unit)
  //[[nodiscard]] Vec4 project_onto(const Vec4& v) const noexcept
  //{
  //  const T denom = v.length2();
  //  return (denom != T{0}) ? v * (dot(v) / denom) : Vec4{};
  //}
  //[[nodiscard]] Vec4 reject_from(const Vec4& v) const noexcept
  //{
  //  return *this - project_onto(v);
  //}

  //// Project onto hyperplane with normal n
  //[[nodiscard]] Vec4 project_onto_hyperplane(const Vec4& n) const noexcept
  //{
  //  const T denom = n.length2();
  //  return (denom != T{0}) ? (*this - n * (dot(n) / denom)) : *this;
  //}

  //// Reflect about (possibly non-unit) normal n
  //[[nodiscard]] Vec4 reflect(const Vec4& n) const noexcept
  //{
  //  const T denom = n.length2();
  //  return (denom != T{0}) ? (*this - n * (T{2} * dot(n) / denom)) : *this;
  //}

  //// Refract through a hyperplane with normal n; eta = eta_i/eta_t
  //[[nodiscard]] Vec4 refract(const Vec4& n, T eta) const noexcept
  //{
  //  const Vec4 I = this->normalized();
  //  const Vec4 N = n.normalized();
  //  const T cosi = -(I.dot(N));
  //  const T k = T{1} - eta * eta * (T{1} - cosi * cosi);
  //  if (k < T{0})
  //    return Vec4{};  // total internal reflection -> zero vector
  //  return eta * I + (eta * cosi - static_cast<T>(std::sqrt(static_cast<long double>(k)))) * N;
  //}

  //// Clamp / set length
  //[[nodiscard]] Vec4 clamped_length(T max_len) const noexcept
  //{
  //  const T len = length();
  //  return (len > max_len && len > T{0}) ? (*this * (max_len / len)) : *this;
  //}
  //[[nodiscard]] Vec4 clamped_length(T min_len, T max_len) const noexcept
  //{
  //  const T len = length();
  //  if (len == T{0})
  //    return *this;
  //  if (len < min_len)
  //    return *this * (min_len / len);
  //  if (len > max_len)
  //    return *this * (max_len / len);
  //  return *this;
  //}
  //[[nodiscard]] Vec4 with_length(T new_len) const noexcept
  //{
  //  const T len = length();
  //  return (len > T{0}) ? (*this * (new_len / len)) : *this;
  //}

  //// Move towards target by at most max_delta
  //[[nodiscard]] Vec4 move_towards(const Vec4& target, T max_delta) const noexcept
  //{
  //  const T d = distance(target);
  //  if (d <= max_delta || d == T{0})
  //    return target;
  //  return *this + (target - *this) * (max_delta / d);
  //}

  // ----------------- component-wise helpers -----------------
  [[nodiscard]] constexpr Vec4 hadamard_mul(const Vec4& v) const noexcept
  {
    return {x * v.x, y * v.y, z * v.z, w * v.w};
  }
  [[nodiscard]] constexpr Vec4 hadamard_div(const Vec4& v) const noexcept
  {
    return {x / v.x, y / v.y, z / v.z, w / v.w};
  }

  [[nodiscard]] constexpr Vec4 abs() const noexcept
  {
    using std::abs;
    return {static_cast<T>(abs(x)), static_cast<T>(abs(y)), static_cast<T>(abs(z)), static_cast<T>(abs(w))};
  }
  template <class U = T, class = std::enable_if_t<std::is_floating_point<U>::value>>
  [[nodiscard]] Vec4 floor() const noexcept
  {
    using std::floor;
    return {static_cast<T>(floor(x)), static_cast<T>(floor(y)), static_cast<T>(floor(z)), static_cast<T>(floor(w))};
  }
  template <class U = T, class = std::enable_if_t<std::is_floating_point<U>::value>>
  [[nodiscard]] Vec4 ceil() const noexcept
  {
    using std::ceil;
    return {static_cast<T>(ceil(x)), static_cast<T>(ceil(y)), static_cast<T>(ceil(z)), static_cast<T>(ceil(w))};
  }
  template <class U = T, class = std::enable_if_t<std::is_floating_point<U>::value>>
  [[nodiscard]] Vec4 round() const noexcept
  {
    using std::round;
    return {static_cast<T>(round(x)), static_cast<T>(round(y)), static_cast<T>(round(z)), static_cast<T>(round(w))};
  }

  [[nodiscard]] static constexpr Vec4 min(const Vec4& a, const Vec4& b) noexcept
  {
    return {(a.x < b.x ? a.x : b.x), (a.y < b.y ? a.y : b.y), (a.z < b.z ? a.z : b.z), (a.w < b.w ? a.w : b.w)};
  }
  [[nodiscard]] static constexpr Vec4 max(const Vec4& a, const Vec4& b) noexcept
  {
    return {(a.x > b.x ? a.x : b.x), (a.y > b.y ? a.y : b.y), (a.z > b.z ? a.z : b.z), (a.w > b.w ? a.w : b.w)};
  }
  [[nodiscard]] static constexpr Vec4 clamp(const Vec4& v, const Vec4& lo, const Vec4& hi) noexcept
  {
    return Vec4::min(Vec4::max(v, lo), hi);
  }

  // Snap each component to nearest multiple of "cell"
  [[nodiscard]] Vec4 snap(T cell) const noexcept
  {
    if (cell == T{0})
      return *this;
    if constexpr (std::is_floating_point<T>::value)
    {
      using std::round;
      return {static_cast<T>(round(x / cell) * cell),
              static_cast<T>(round(y / cell) * cell),
              static_cast<T>(round(z / cell) * cell),
              static_cast<T>(round(w / cell) * cell)};
    }
    else
    {
      return {(x / cell) * cell, (y / cell) * cell, (z / cell) * cell, (w / cell) * cell};
    }
  }

  // Reciprocal (watch for zeros)
  [[nodiscard]] Vec4 rcp() const noexcept
  {
    return {T{1} / x, T{1} / y, T{1} / z, T{1} / w};
  }

  // Major/minor axis by absolute value (index 0..3)
  [[nodiscard]] int major_axis() const noexcept
  {
    const Vec4 a = abs();
    int m = 0;
    T v = a.x;
    if (a.y > v)
    {
      v = a.y;
      m = 1;
    }
    if (a.z > v)
    {
      v = a.z;
      m = 2;
    }
    if (a.w > v)
    {
      v = a.w;
      m = 3;
    }
    return m;
  }
  [[nodiscard]] int minor_axis() const noexcept
  {
    const Vec4 a = abs();
    int m = 0;
    T v = a.x;
    if (a.y < v)
    {
      v = a.y;
      m = 1;
    }
    if (a.z < v)
    {
      v = a.z;
      m = 2;
    }
    if (a.w < v)
    {
      v = a.w;
      m = 3;
    }
    return m;
  }

  //// Any unit vector orthogonal to this (or +X if zero). In 4D, the orthogonal subspace is 3D;
  //// this returns one representative via a simple Gram-Schmidt step.
  //[[nodiscard]] Vec4 any_orthogonal() const noexcept
  //{
  //  if (this->is_zero())
  //    return Vec4::unit_x();
  //  // pick an axis least aligned with *this*
  //  const int m = minor_axis();
  //  Vec4 axis = (m == 0)   ? Vec4{T{1}, T{0}, T{0}, T{0}}
  //              : (m == 1) ? Vec4{T{0}, T{1}, T{0}, T{0}}
  //              : (m == 2) ? Vec4{T{0}, T{0}, T{1}, T{0}}
  //                         : Vec4{T{0}, T{0}, T{0}, T{1}};
  //  // Gram-Schmidt: u = axis - proj_axis_on_this
  //  const Vec4 n = this->normalized();
  //  const Vec4 u = axis - n * (axis.dot(n));
  //  return u.normalized();
  //}

  //// Orthonormal basis (t1,t2,t3,n) with n = normalized(*this)
  //[[nodiscard]] std::array<Vec4, 4> orthonormal_basis() const noexcept
  //{
  //  const Vec4 n = normalized();
  //  Vec4 t1 = n.any_orthogonal();  // first tangent
  //  // pick another axis and orthogonalize again
  //  Vec4 seed2 = (t1.major_axis() != 0) ? Vec4::unit_x() : Vec4::unit_y();
  //  Vec4 t2 = (seed2 - n * (seed2.dot(n)) - t1 * (seed2.dot(t1))).normalized();
  //  // third tangent
  //  Vec4 seed3 = Vec4::unit_z();
  //  Vec4 t3 = (seed3 - n * (seed3.dot(n)) - t1 * (seed3.dot(t1)) - t2 * (seed3.dot(t2))).normalized();
  //  return {t1, t2, t3, n};
  //}

  // ----------------- interpolation -----------------
  [[nodiscard]] static constexpr Vec4 lerp(const Vec4& a, const Vec4& b, T t) noexcept
  {
    return a + (b - a) * t;
  }
  [[nodiscard]] static Vec4 nlerp(const Vec4& a, const Vec4& b, T t) noexcept
  {
    const Vec4 v = lerp(a, b, t);
    const T l = v.length();
    return (l > T{0}) ? (v / l) : v;
  }
  // Spherical interpolation for directions in 4D; magnitudes are linearly interpolated
  [[nodiscard]] static Vec4 slerp(const Vec4& a, const Vec4& b, T t) noexcept
  {
    const T la = a.length(), lb = b.length();
    const T L = la + (lb - la) * t;
    const Vec4 ua = (la > T{0}) ? (a / la) : a;
    const Vec4 ub = (lb > T{0}) ? (b / lb) : b;

    const T cosT = ua.dot(ub);
    if (cosT > T{0.9995} || cosT < T{-0.9995})
    {
      return (nlerp(ua, ub, t) * L);
    }
    using std::acos;
    using std::sin;
    const T theta = static_cast<T>(acos(static_cast<long double>(cosT)));
    const T s0 = static_cast<T>(sin(static_cast<long double>((T{1} - t) * theta)));
    const T s1 = static_cast<T>(sin(static_cast<long double>(t * theta)));
    const T s = static_cast<T>(sin(static_cast<long double>(theta)));
    return ((ua * s0 + ub * s1) * (L / s));
  }

  // ----------------- numerics -----------------
  //[[nodiscard]] bool is_finite() const noexcept
  //{
  //  if constexpr (std::is_floating_point<T>::value)
  //  {
  //    using std::isfinite;
  //    return isfinite(x) && isfinite(y) && isfinite(z) && isfinite(w);
  //  }
  //  else
  //  {
  //    return true;
  //  }
  //}

  // ----------------- conversions & helpers -----------------
  [[nodiscard]] constexpr std::array<T, 4> to_array() const noexcept
  {
    return {x, y, z, w};
  }
  // Perspective divide (x,y,z)/w -> returns array; caller can map to Vec3 if desired
  [[nodiscard]] std::array<T, 3> perspective_divide() const noexcept
  {
    if (w == T{0})
      return {x, y, z};
    return {x / w, y / w, z / w};
  }

  // ----------------- constants -----------------
  [[nodiscard]] static constexpr Vec4 zero() noexcept
  {
    return {T{0}, T{0}, T{0}, T{0}};
  }
  [[nodiscard]] static constexpr Vec4 one() noexcept
  {
    return {T{1}, T{1}, T{1}, T{1}};
  }
  [[nodiscard]] static constexpr Vec4 unit_x() noexcept
  {
    return {T{1}, T{0}, T{0}, T{0}};
  }
  [[nodiscard]] static constexpr Vec4 unit_y() noexcept
  {
    return {T{0}, T{1}, T{0}, T{0}};
  }
  [[nodiscard]] static constexpr Vec4 unit_z() noexcept
  {
    return {T{0}, T{0}, T{1}, T{0}};
  }
  [[nodiscard]] static constexpr Vec4 unit_w() noexcept
  {
    return {T{0}, T{0}, T{0}, T{1}};
  }
};

}  // namespace clarinoid
