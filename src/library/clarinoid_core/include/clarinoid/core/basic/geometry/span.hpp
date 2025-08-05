#pragma once

// #include <algorithm>
// #include <cmath>
#include <limits>
#include <optional>
#include "../Numeric.hpp"
#include <type_traits>
#include <utility>


namespace clarinoid
{

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 1D segment with origin, size. used for things like an edge of a rectangle.
template <typename T>
struct Span
{
  static_assert(is_scalar_like<T>::value, "Span<T>: T must be scalar-like");

  // Invariant: mLength >= 0 (for signed T); for unsigned T this is always true.
  T mOrigin{};
  T mLength{};

  // ── construction ───────────────────────────────────────────────────
  constexpr Span() = default;
  // constexpr Span(T origin, T length)
  //   : mOrigin(origin)
  //   , mLength(length)
  //{
  //   // Keep debug-time guard for signed types.
  //   if constexpr (std::is_signed_v<T>) { /* assert(mLength >= T{0}); */
  //   }
  // }

  [[nodiscard]] constexpr T center() const noexcept
  {
    // avoids overflow, works for integral & floating
    return std::midpoint(start(), end());
  }

  // Factories from endpoints, for convenience and clarity.
  // For half-open [lo, hi): length = max(hi - lo, 0)
  [[nodiscard]] static constexpr Span from_endpoints(T lo, T hi) noexcept
  {
    return (hi > lo) ? Span{lo, hi - lo} : Span{lo, T{0}};
  }

  // If you ever need to interop with a *closed* [lo, hi] integer range:
  // (only makes sense for integral T)
  [[nodiscard]] static constexpr Span from_closed(T lo, T hi) noexcept
  {
    static_assert(std::is_integral_v<T>, "from_closed is for integral types only.");
    return (hi >= lo) ? Span{lo, (hi - lo) + T{1}} : Span{lo, T{0}};
  }

  // ── basic queries ──────────────────────────────────────────────────
  [[nodiscard]] constexpr T start() const noexcept
  {
    return mOrigin;
  }  // inclusive
  [[nodiscard]] constexpr T end() const noexcept
  {
    return mOrigin + mLength;
  }  // exclusive
  [[nodiscard]] constexpr T length() const noexcept
  {
    return mLength;
  }
  [[nodiscard]] constexpr bool empty() const noexcept
  {
    if constexpr (std::is_signed_v<T>)
      return mLength <= T{0};
    else
      return mLength == T{0};
  }

  // ── containment & intersection (half-open) ─────────────────────────
  [[nodiscard]] constexpr bool contains(T x) const noexcept
  {
    // [start, end)
    return x >= start() && x < end();
  }

  [[nodiscard]] constexpr bool contains(const Span& s) const noexcept
  {
    return s.start() >= start() && s.end() <= end();
  }

  [[nodiscard]] constexpr bool overlaps(const Span& s) const noexcept
  {
    // Half-open intervals overlap iff max(starts) < min(ends)
    return std::max(start(), s.start()) < std::min(end(), s.end());
  }

  [[nodiscard]] constexpr std::optional<Span> intersection(const Span& s) const noexcept
  {
    const T lo = std::max(start(), s.start());
    const T hi = std::min(end(), s.end());
    if (!(lo < hi))
      return std::nullopt;  // empty intersection
    return Span{lo, hi - lo};
  }

  // Minimal span that covers both (convex hull).
  [[nodiscard]] constexpr Span united(const Span& s) const noexcept
  {
    const T lo = std::min(start(), s.start());
    const T hi = std::max(end(), s.end());
    return Span{lo, hi - lo};
  }

  // Clamp this span to lie within 'bounds'. If it doesn't fit, it is squeezed.
  [[nodiscard]] constexpr Span clamped(const Span& bounds) const noexcept
  {
    if (empty())
      return *this;

    // If we don't fit, the best we can do is the whole bounds.
    if (length() >= bounds.length())
      return bounds;

    // Compute the max allowed start so [newStart, newStart + length) ⊆ bounds.
    T maxStart;
    if constexpr (std::is_signed_v<T>)
    {
      maxStart = bounds.end() - length();
    }
    else
    {
      // avoid unsigned underflow when length() > bounds.end()
      maxStart = (bounds.end() >= length()) ? (bounds.end() - length()) : bounds.start();
    }
    const T newStart = std::clamp(start(), bounds.start(), maxStart);
    return Span{newStart, length()};
  }

  // ── translation / scaling ──────────────────────────────────────────
  [[nodiscard]] constexpr Span operator+(T delta) const noexcept
  {
    return {start() + delta, length()};
  }
  [[nodiscard]] constexpr Span operator-(T delta) const noexcept
  {
    return {start() - delta, length()};
  }

  [[nodiscard]] constexpr Span& operator+=(T delta) noexcept
  {
    mOrigin += delta;
    return *this;
  }
  [[nodiscard]] constexpr Span& operator-=(T delta) noexcept
  {
    mOrigin -= delta;
    return *this;
  }

  // Scale about zero. For negative scales, normalize so length stays non-negative.
  [[nodiscard]] constexpr Span operator*(T scale) const noexcept
  {
    const T a = start() * scale;
    const T b = end() * scale;
    const T lo = std::min(a, b);
    const T hi = std::max(a, b);
    return Span::from_endpoints(lo, hi);
  }

  // Optional: scale about an arbitrary pivot p (keeps p fixed).
  [[nodiscard]] constexpr Span scaled_about(T pivot, T scale) const noexcept
  {
    const T a = (start() - pivot) * scale + pivot;
    const T b = (end() - pivot) * scale + pivot;
    const T lo = std::min(a, b);
    const T hi = std::max(a, b);
    return Span::from_endpoints(lo, hi);
  }

  template <typename U = T>
  [[nodiscard]] constexpr Span recentered(U c) const noexcept
  {
    static_assert(std::is_signed_v<T> || std::is_floating_point_v<T>, "recentered requires a signed/floating T");
    const auto delta = static_cast<U>(c) - static_cast<U>(center());
    return *this + static_cast<T>(delta);
  }

  // Symmetric grow/shrink by 'd' on each side (signed/floating T only)
  [[nodiscard]] constexpr Span grown(T d) const noexcept
  {
    static_assert(std::is_signed_v<T> || std::is_floating_point_v<T>, "grown requires signed/floating T");
    const T newStart = start() - d;
    const T newLen = length() + d + d;
    return (newLen > T{0}) ? Span{newStart, newLen} : Span{start(), T{0}};
  }

  // Asymmetric padding (left,right); negative values shrink.
  [[nodiscard]] constexpr Span padded(T left, T right) const noexcept
  {
    const T newStart = start() - left;
    const T newEnd = end() + right;
    return Span::from_endpoints(newStart, newEnd);
  }

  // Point along the span: t in [0,1] -> [start,end)
  template <typename U>
  [[nodiscard]] constexpr std::common_type_t<T, U> point_along(U t01) const noexcept
  {
    using R = std::common_type_t<T, U>;
    return static_cast<R>(start()) + static_cast<R>(length()) * static_cast<R>(t01);
  }

  // Fractional position of 'x' within the span (not clamped)
  //   O--------------->
  //   |   |   |   |   |
  //           x
  // returns 0.5
  template <typename U = T>
  [[nodiscard]] constexpr std::common_type_t<T, U> fraction(U x) const noexcept
  {
    using R = std::common_type_t<T, U>;
    return (static_cast<R>(x) - static_cast<R>(start())) / static_cast<R>(length());
  }

  // Clamp a point into the span (keeps half-open rules)
  [[nodiscard]] constexpr T clamp_point(T x) const noexcept
  {
    if (x < start())
      return start();
    // for half-open, the "last" valid integral point is end()-1
    if constexpr (std::is_integral_v<T>)
      return (x >= end()) ? (empty() ? start() : end() - T{1}) : x;
    else
      return (x >= end()) ? std::nextafter(end(), start()) : x;
  }

  // Remap a point x from `src` span into `this` span (affine map)
  template <typename U = T>
  [[nodiscard]] constexpr std::common_type_t<T, U> remap_from(U x, const Span& src) const noexcept
  {
    using R = std::common_type_t<T, U>;
    const R t = (static_cast<R>(x) - static_cast<R>(src.start())) / static_cast<R>(src.length());
    return static_cast<R>(start()) + t * static_cast<R>(length());
  }

  // Keep end fixed, move start.
  [[nodiscard]] constexpr Span with_start(T s) const noexcept
  {
    return Span::from_endpoints(s, end());
  }

  // Keep start fixed, move end.
  [[nodiscard]] constexpr Span with_end(T e) const noexcept
  {
    return Span::from_endpoints(start(), e);
  }

  // Keep start fixed, set length (clamps negative to empty)
  [[nodiscard]] constexpr Span with_length(T L) const noexcept
  {
    return (L > T{0}) ? Span{start(), L} : Span{start(), T{0}};
  }

  // true if they touch but do not overlap (half-open adjacency)
  [[nodiscard]] constexpr bool is_adjacent(const Span& s) const noexcept
  {
    return end() == s.start() || s.end() == start();
  }

  // Signed gap: 0 if overlapping/adjacent; negative if s overlaps into this.
  //            o-------->
  //   <---s....               returns 4
  //   s--->....               returns 4
  //                s------->  returns 0
  [[nodiscard]] constexpr T gap_to(const Span& s) const noexcept
  {
    if (overlaps(s) || adjacent(s))
      return T{0};
    return (s.start() > end()) ? (s.start() - end()) : (start() - s.end());
  }

  // Intersection length (0 if none)
  [[nodiscard]] constexpr T overlap_length(const Span& s) const noexcept
  {
    const T lo = std::max(start(), s.start());
    const T hi = std::min(end(), s.end());
    return (hi > lo) ? (hi - lo) : T{0};
  }

  // Split at x (assumes x in [start,end]); keeps half-open pieces.
  [[nodiscard]] constexpr std::pair<Span, Span> split_at(T x) const noexcept
  {
    const T a = std::clamp(x, start(), end());
    return {Span::from_endpoints(start(), a), Span::from_endpoints(a, end())};
  }

  //// Subdivide into n equal (last may be shorter for integers)
  //[[nodiscard]] std::vector<Span> subdivide(std::size_t n) const
  //{
  //  std::vector<Span> out;
  //  if (n == 0 || empty())
  //    return out;
  //  out.reserve(n);
  //  if constexpr (std::is_floating_point_v<T>) {
  //    for (std::size_t i = 0; i < n; ++i) {
  //      const T a = start() + (length() * static_cast<T>(i)) / static_cast<T>(n);
  //      const T b = start() + (length() * static_cast<T>(i + 1)) / static_cast<T>(n);
  //      out.emplace_back(Span::from_endpoints(a, b));
  //    }
  //  } else {
  //    // integral: best-effort even chunks, distribute remainder to early buckets
  //    const T base = length() / static_cast<T>(n);
  //    T rem = length() % static_cast<T>(n);
  //    T cur = start();
  //    for (std::size_t i = 0; i < n; ++i) {
  //      const T sz = base + (rem ? T{ 1 } : T{ 0 });
  //      if (rem)
  //        --rem;
  //      out.emplace_back(Span{ cur, sz });
  //      cur += sz;
  //    }
  //  }
  //  return out;
  //}

  // Expand to include a point or another span (convex hull).
  [[nodiscard]] constexpr Span including(T x) const noexcept
  {
    if (empty())
      return Span{x, T{0}};  // degenerate; could also choose [x,x)
    const T lo = std::min(start(), x);
    // include point in half-open => ensure x < newEnd; if x >= end(), grow end to x+ε
    const T hi = std::max(end(),
                          (x < end()) ? end()
                                      : (x + (std::is_integral_v<T> ? T{1} : std::numeric_limits<T>::epsilon())));
    return Span::from_endpoints(lo, hi);
  }

  [[nodiscard]] constexpr Span including(const Span& s) const noexcept
  {
    return united(s);
  }

  // ── comparison ─────────────────────────────────────────────────────
  //[[nodiscard]] constexpr bool operator==(const Span&) const = default;
};

}  // namespace clarinoid
