#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
namespace clarinoid {

// TODO: test logic for int vs float. best to support fixed point.
// more geometry types:
//   circle
// more complete hit testing than just "contains"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
struct Point
{
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

using PointI = Point<int>;
using PointI16 = Point<int16_t>;
using PointF = Point<float>;


// ── minimal 2D vector (no dependencies) ───────────────────────────────
template<typename T>
struct Vec2
{
  T x{}, y{};
  constexpr Vec2() = default;
  constexpr Vec2(T x_, T y_)
    : x(x_)
    , y(y_)
  {
  }

  [[nodiscard]] constexpr Vec2 operator+(const Vec2& v) const noexcept { return { x + v.x, y + v.y }; }
  [[nodiscard]] constexpr Vec2 operator-(const Vec2& v) const noexcept { return { x - v.x, y - v.y }; }
  [[nodiscard]] constexpr Vec2 operator*(T s) const noexcept { return { x * s, y * s }; }
  [[nodiscard]] constexpr Vec2 operator/(T s) const noexcept { return { x / s, y / s }; }

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

  [[nodiscard]] constexpr T dot(const Vec2& v) const noexcept { return x * v.x + y * v.y; }
  [[nodiscard]] constexpr T cross(const Vec2& v) const noexcept { return x * v.y - y * v.x; } // 2D scalar cross
  [[nodiscard]] constexpr T length2() const noexcept { return x * x + y * y; }
  [[nodiscard]] T length() const noexcept
  {
    using std::hypot;
    return hypot(x, y);
  }

  [[nodiscard]] Vec2 normalized() const noexcept
  {
    const T len = length();
    return (len > T{ 0 }) ? (*this / len) : *this;
  }
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 1D segment with origin, size. used for things like an edge of a rectangle.
template<typename T>
struct Span
{
  static_assert(std::is_arithmetic_v<T>, "Span<T> requires an arithmetic type.");

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
    return (hi > lo) ? Span{ lo, hi - lo } : Span{ lo, T{ 0 } };
  }

  // If you ever need to interop with a *closed* [lo, hi] integer range:
  // (only makes sense for integral T)
  [[nodiscard]] static constexpr Span from_closed(T lo, T hi) noexcept
  {
    static_assert(std::is_integral_v<T>, "from_closed is for integral types only.");
    return (hi >= lo) ? Span{ lo, (hi - lo) + T{ 1 } } : Span{ lo, T{ 0 } };
  }

  // ── basic queries ──────────────────────────────────────────────────
  [[nodiscard]] constexpr T start() const noexcept { return mOrigin; }         // inclusive
  [[nodiscard]] constexpr T end() const noexcept { return mOrigin + mLength; } // exclusive
  [[nodiscard]] constexpr T length() const noexcept { return mLength; }
  [[nodiscard]] constexpr bool empty() const noexcept
  {
    if constexpr (std::is_signed_v<T>)
      return mLength <= T{ 0 };
    else
      return mLength == T{ 0 };
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
      return std::nullopt; // empty intersection
    return Span{ lo, hi - lo };
  }

  // Minimal span that covers both (convex hull).
  [[nodiscard]] constexpr Span united(const Span& s) const noexcept
  {
    const T lo = std::min(start(), s.start());
    const T hi = std::max(end(), s.end());
    return Span{ lo, hi - lo };
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
    if constexpr (std::is_signed_v<T>) {
      maxStart = bounds.end() - length();
    } else {
      // avoid unsigned underflow when length() > bounds.end()
      maxStart = (bounds.end() >= length()) ? (bounds.end() - length()) : bounds.start();
    }
    const T newStart = std::clamp(start(), bounds.start(), maxStart);
    return Span{ newStart, length() };
  }

  // ── translation / scaling ──────────────────────────────────────────
  [[nodiscard]] constexpr Span operator+(T delta) const noexcept { return { start() + delta, length() }; }
  [[nodiscard]] constexpr Span operator-(T delta) const noexcept { return { start() - delta, length() }; }

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

  template<typename U = T>
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
    return (newLen > T{ 0 }) ? Span{ newStart, newLen } : Span{ start(), T{ 0 } };
  }

  // Asymmetric padding (left,right); negative values shrink.
  [[nodiscard]] constexpr Span padded(T left, T right) const noexcept
  {
    const T newStart = start() - left;
    const T newEnd = end() + right;
    return Span::from_endpoints(newStart, newEnd);
  }

  // Point along the span: t in [0,1] -> [start,end)
  template<typename U>
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
  template<typename U = T>
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
      return (x >= end()) ? (empty() ? start() : end() - T{ 1 }) : x;
    else
      return (x >= end()) ? std::nextafter(end(), start()) : x;
  }

  // Remap a point x from `src` span into `this` span (affine map)
  template<typename U = T>
  [[nodiscard]] constexpr std::common_type_t<T, U> remap_from(U x, const Span& src) const noexcept
  {
    using R = std::common_type_t<T, U>;
    const R t = (static_cast<R>(x) - static_cast<R>(src.start())) / static_cast<R>(src.length());
    return static_cast<R>(start()) + t * static_cast<R>(length());
  }

  // Keep end fixed, move start.
  [[nodiscard]] constexpr Span with_start(T s) const noexcept { return Span::from_endpoints(s, end()); }

  // Keep start fixed, move end.
  [[nodiscard]] constexpr Span with_end(T e) const noexcept { return Span::from_endpoints(start(), e); }

  // Keep start fixed, set length (clamps negative to empty)
  [[nodiscard]] constexpr Span with_length(T L) const noexcept
  {
    return (L > T{ 0 }) ? Span{ start(), L } : Span{ start(), T{ 0 } };
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
      return T{ 0 };
    return (s.start() > end()) ? (s.start() - end()) : (start() - s.end());
  }

  // Intersection length (0 if none)
  [[nodiscard]] constexpr T overlap_length(const Span& s) const noexcept
  {
    const T lo = std::max(start(), s.start());
    const T hi = std::min(end(), s.end());
    return (hi > lo) ? (hi - lo) : T{ 0 };
  }

  // Split at x (assumes x in [start,end]); keeps half-open pieces.
  [[nodiscard]] constexpr std::pair<Span, Span> split_at(T x) const noexcept
  {
    const T a = std::clamp(x, start(), end());
    return { Span::from_endpoints(start(), a), Span::from_endpoints(a, end()) };
  }

  // Subdivide into n equal (last may be shorter for integers)
  [[nodiscard]] std::vector<Span> subdivide(std::size_t n) const
  {
    std::vector<Span> out;
    if (n == 0 || empty())
      return out;
    out.reserve(n);
    if constexpr (std::is_floating_point_v<T>) {
      for (std::size_t i = 0; i < n; ++i) {
        const T a = start() + (length() * static_cast<T>(i)) / static_cast<T>(n);
        const T b = start() + (length() * static_cast<T>(i + 1)) / static_cast<T>(n);
        out.emplace_back(Span::from_endpoints(a, b));
      }
    } else {
      // integral: best-effort even chunks, distribute remainder to early buckets
      const T base = length() / static_cast<T>(n);
      T rem = length() % static_cast<T>(n);
      T cur = start();
      for (std::size_t i = 0; i < n; ++i) {
        const T sz = base + (rem ? T{ 1 } : T{ 0 });
        if (rem)
          --rem;
        out.emplace_back(Span{ cur, sz });
        cur += sz;
      }
    }
    return out;
  }

  // Expand to include a point or another span (convex hull).
  [[nodiscard]] constexpr Span including(T x) const noexcept
  {
    if (empty())
      return Span{ x, T{ 0 } }; // degenerate; could also choose [x,x)
    const T lo = std::min(start(), x);
    // include point in half-open => ensure x < newEnd; if x >= end(), grow end to x+ε
    const T hi =
      std::max(end(), (x < end()) ? end() : (x + (std::is_integral_v<T> ? T{ 1 } : std::numeric_limits<T>::epsilon())));
    return Span::from_endpoints(lo, hi);
  }

  [[nodiscard]] constexpr Span including(const Span& s) const noexcept { return united(s); }

  // ── comparison ─────────────────────────────────────────────────────
  //[[nodiscard]] constexpr bool operator==(const Span&) const = default;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
struct Size
{
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

  static Size Construct(T w_, T h_) { return { w_, h_ }; }

  bool IsEmpty() const { return (width <= 0) || (height <= 0); }
  T Area() const { return width * height; }
  float AspectRatio() const { return (height == 0) ? 0.f : float(width) / float(height); }
  Size<T> Scale(T factor) const { return Size{ static_cast<T>(width * factor), static_cast<T>(height * factor) }; }

  Size<T> WithWidth(T w) const { return { w, height }; }
  Size<T> WithHeight(T h) const { return { width, h }; }

  static Size Square(T s) { return { s, s }; }

  bool IsSquare() const { return width == height; }
};

using SizeI = Size<int>;
using SizeI16 = Size<int16_t>;
using SizeF = Size<float>;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
struct Rect
{
  Point<T> position;
  Size<T> size;

  // rule of 5
  // Rect(const Rect& other) = default;
  // Rect(Rect&& other) = default;
  // Rect& operator=(const Rect& other) = default;
  // Rect& operator=(Rect&& other) = default;
  // ~Rect() = default;

  Rect()
    : position(Point<T>::Construct(static_cast<T>(0), static_cast<T>(0)))
    , size(Size<T>::Construct(static_cast<T>(0), static_cast<T>(0)))
  {
  }

  Rect(T x, T y, T w, T h)
    : position(Point<T>::Construct(x, y))
    , size(Size<T>::Construct(w, h))
  {
  }

  Rect(const Point<T>& pos, const Size<T>& sz)
    : position(pos)
    , size(sz)
  {
  }

  static Rect Construct(const Point<T>& pos, const Size<T>& sz) { return { pos, sz }; }

  static Rect Construct(T x, T y, T w, T h) { return { x, y, w, h }; }

  T Left() const { return position.x; }
  T Top() const { return position.y; }
  T Width() const { return size.width; }
  T Height() const { return size.height; }

  // "end" of the rect; the first value that is not part of the rect.
  T Right() const { return position.x + size.width; }
  T Bottom() const { return position.y + size.height; }

  // Set the top edge (i.e. move the top while keeping the bottom fixed)
  void SetTop(T t)
  {
    T bottom = Bottom();
    position.y = t;
    size.height = bottom - t;
  }

  // Set the left edge (move left while keeping the right fixed)
  void SetLeft(T l)
  {
    T right = Right();
    position.x = l;
    size.width = right - l;
  }

  // Set the right edge (move right; top-left remains fixed)
  void SetRight(T r) { size.width = r - position.x; }

  // Set the bottom edge (move bottom; top remains fixed)
  void SetBottom(T b) { size.height = b - position.y; }

  //
  // Set corner functions.
  // In each case the opposite corner is kept fixed.
  //

  // Set the top-left corner (keeping the bottom-right constant)
  void SetTopLeft(const Point<T>& pt)
  {
    T right = Right();
    T bottom = Bottom();
    position = pt;
    size.width = right - pt.x;
    size.height = bottom - pt.y;
  }

  // Set the top-right corner (keeping the bottom-left constant)
  void SetTopRight(const Point<T>& pt)
  {
    T left = Left();
    T bottom = Bottom();
    position.y = pt.y;
    size.height = bottom - pt.y;
    size.width = pt.x - left;
  }

  // Set the bottom-left corner (keeping the top-right constant)
  void SetBottomLeft(const Point<T>& pt)
  {
    T top = Top();
    T right = Right();
    position.x = pt.x;
    size.width = right - pt.x;
    size.height = pt.y - top;
  }

  // Set the bottom-right corner (keeping the top-left constant)
  void SetBottomRight(const Point<T>& pt)
  {
    size.width = pt.x - position.x;
    size.height = pt.y - position.y;
  }

  //
  // Offset edge functions.
  // When you “offset” one edge, you move that edge only.
  // For example, offsetting the left edge moves it while leaving the right edge fixed.
  //

  void OffsetLeft(T dx)
  {
    // Moving the left edge by dx means both shifting it and reducing (or increasing)
    // the width so that the right edge stays put.
    position.x += dx;
    size.width -= dx;
  }

  void OffsetTop(T dy)
  {
    position.y += dy;
    size.height -= dy;
  }

  void OffsetRight(T dx)
  {
    // Right edge moves relative to the fixed left edge.
    size.width += dx;
  }

  void OffsetBottom(T dy) { size.height += dy; }

  //
  // "With" functions: they return a modified copy of the rectangle.
  //

  Rect WithTop(T t) const
  {
    Rect r(*this);
    r.SetTop(t);
    return r;
  }
  Rect WithLeft(T l) const
  {
    Rect r(*this);
    r.SetLeft(l);
    return r;
  }
  Rect WithRight(T rVal) const
  {
    Rect r(*this);
    r.SetRight(rVal);
    return r;
  }
  Rect WithBottom(T b) const
  {
    Rect r(*this);
    r.SetBottom(b);
    return r;
  }

  Rect WithSize(const Size<T>& sz) const
  {
    Rect r(*this);
    r.size = sz;
    return r;
  }
  Rect WithWidthLeftJustified(T w) const
  {
    Rect r(*this);
    r.size.width = w;
    return r;
  }
  Rect WithHeightTopJustified(T h) const
  {
    Rect r(*this);
    r.size.height = h;
    return r;
  }

  Rect WithWidthRightJustified(T w) const
  {
    Rect r(*this);
    r.position.x = Right() - w;
    r.size.width = w;
    return r;
  }

  Rect WithHeightBottomJustified(T h) const
  {
    Rect r(*this);
    r.position.y = Bottom() - h;
    r.size.height = h;
    return r;
  }

  Rect WithTopLeft(const Point<T>& pt) const
  {
    Rect r(*this);
    r.SetTopLeft(pt);
    return r;
  }
  Rect WithTopRight(const Point<T>& pt) const
  {
    Rect r(*this);
    r.SetTopRight(pt);
    return r;
  }
  Rect WithBottomLeft(const Point<T>& pt) const
  {
    Rect r(*this);
    r.SetBottomLeft(pt);
    return r;
  }
  Rect WithBottomRight(const Point<T>& pt) const
  {
    Rect r(*this);
    r.SetBottomRight(pt);
    return r;
  }

  Rect WithOffsetLeft(T dx) const
  {
    Rect r(*this);
    r.OffsetLeft(dx);
    return r;
  }
  Rect WithOffsetTop(T dy) const
  {
    Rect r(*this);
    r.OffsetTop(dy);
    return r;
  }
  Rect WithOffsetRight(T dx) const
  {
    Rect r(*this);
    r.OffsetRight(dx);
    return r;
  }
  Rect WithOffsetBottom(T dy) const
  {
    Rect r(*this);
    r.OffsetBottom(dy);
    return r;
  }

  Rect CenteredHorizontallyIn(const Rect& container) const
  {
    auto centerX = container.Left() + container.Width() / 2;
    auto newLeft = centerX - Width() / 2;
    return Construct(newLeft, Top(), Width(), Height());
  }

  // Returns a new rect that fills from the center upward towards the top, or downward towards the bottom, depending on
  // the fraction value specified in the range of -1 to 1.
  Rect WithBipolarVerticalFill(float fractionN11) const
  {
    if (fractionN11 >= 0) {
      return BottomHalf().TopFraction(fractionN11);
    }
    return TopHalf().BottomFraction(-fractionN11);
  }

  Rect WithBipolarHorizontalFill(float fractionN11) const
  {
    if (fractionN11 >= 0) {
      return RightHalf().LeftFraction(fractionN11);
    }
    return LeftHalf().RightFraction(-fractionN11);
  }

  Rect Inflate(T amount) const
  {
    // Because we're inflating outwards from top-left, the new left/top
    // will be (x - amount, y - amount), and the new size grows
    // by 2 * amount horizontally and vertically.
    return Construct(position.x - amount,
                     position.y - amount,
                     size.width + amount * static_cast<T>(2),
                     size.height + amount * static_cast<T>(2));
  }

  Rect Inset(T amount) const
  {
    return Construct(position.x + amount,
                     position.y + amount,
                     size.width - amount * static_cast<T>(2),
                     size.height - amount * static_cast<T>(2));
  }

  // Return corners as Points
  Point<T> TopLeft() const { return { position }; }
  Point<T> TopRight() const { return Point<T>::Construct(Right(), Top()); }
  Point<T> BottomLeft() const { return Point<T>::Construct(Left(), Bottom()); }
  Point<T> BottomRight() const { return Point<T>::Construct(Right(), Bottom()); }

  Rect<T> LeftHalf() const { return Cell(2, 1, 0, 0); }
  Rect<T> RightHalf() const { return Cell(2, 1, 1, 0); }
  Rect<T> TopHalf() const { return Cell(1, 2, 0, 0); }
  Rect<T> BottomHalf() const { return Cell(1, 2, 0, 1); }

  Rect<T> TopFraction(float fraction01) const
  {
    return Construct(position, Size<T>::Construct(size.width, size.height * fraction01));
  }
  Rect<T> BottomFraction(float fraction01) const
  {
    return Construct(Point<T>::Construct(position.x, position.y + size.height * (1 - fraction01)),
                     Size<T>::Construct(size.width, size.height * fraction01));
  }
  Rect<T> LeftFraction(float fraction01) const
  {
    return Construct(position, Size<T>::Construct(size.width * fraction01, size.height));
  }
  Rect<T> RightFraction(float fraction01) const
  {
    return Construct(Point<T>::Construct(position.x + size.width * (1 - fraction01), position.y),
                     Size<T>::Construct(size.width * fraction01, size.height));
  }

  // breaks this rect into equally sized cells, and returns the rect for the requested cell index.
  // cells are indexed from left to right, top to bottom. out of bounds cell indices are supported.
  // using this operation is pretty powerful; can do things like mirroring around edges, taking quadrants etc.
  Rect<T> Cell(size_t columnCount, size_t rowCount, size_t cellXIndex, size_t cellYIndex) const
  {
    T cellWidth = size.width / columnCount;
    T cellHeight = size.height / rowCount;
    return Construct(position.x + cellXIndex * cellWidth, position.y + cellYIndex * cellHeight, cellWidth, cellHeight);
  }

  Rect<T> CellWithSize(size_t columnSizePixels, size_t rowSizePixels, size_t cellXIndex, size_t cellYIndex) const
  {
    return Construct(position.x + cellXIndex * columnSizePixels,
                     position.y + cellYIndex * rowSizePixels,
                     columnSizePixels,
                     rowSizePixels);
  }

  Rect<T> UpperLeftRect(T width, T height) const { return Construct(position.x, position.y, width, height); }
  Rect<T> UpperRightRect(T width, T height) const
  {
    return Construct(position.x + size.width - width, position.y, width, height);
  }
  Rect<T> LowerLeftRect(T width, T height) const
  {
    return Construct(position.x, position.y + size.height - height, width, height);
  }
  Rect<T> LowerRightRect(T width, T height) const
  {
    return Construct(position.x + size.width - width, position.y + size.height - height, width, height);
  }

  Rect<T> HorizontalSlice(T yOffset, T height) const
  {
    return Construct(position.x, position.y + yOffset, size.width, height);
  }

  Rect<T> VerticalSlice(T xOffset, T width) const
  {
    return Construct(position.x + xOffset, position.y, width, size.height);
  }

  Point<T> Center() const
  {
    return Point<T>::Construct(position.x + size.width / static_cast<T>(2),
                               position.y + size.height / static_cast<T>(2));
  }

  bool YInRect(T testY) const { return (testY >= Top()) && (testY < Bottom()); }
  bool XInRect(T testX) const { return (testX >= Left()) && (testX < Right()); }

  bool Contains(const Point<T>& pt) const
  {
    return pt.x >= Left() && pt.x < Right() && pt.y >= Top() && pt.y < Bottom();
  }

  Size<T> GetSize() const { return size; }

  // returns a new rect that contains both this rect and the other rect.
  Rect<T> Union(const Rect<T>& other) const
  {
    T x1 = std::min(Left(), other.Left());
    T y1 = std::min(Top(), other.Top());
    T x2 = std::max(Right(), other.Right());
    T y2 = std::max(Bottom(), other.Bottom());
    return Construct(x1, y1, x2 - x1, y2 - y1);
  }

  // returns a new rect representing the overlapping region of this rect and the other rect.
  // would be good for clipping an incoming rect to avoid drawing outside of bounded area.
  Rect Intersection(const Rect& other) const
  {
    T nx = std::max(Left(), other.Left());
    T ny = std::max(Top(), other.Top());
    T nr = std::min(Right(), other.Right());
    T nb = std::min(Bottom(), other.Bottom());
    if (nr > nx && nb > ny) {
      // (nx, ny) is top-left, (nr - nx, nb - ny) is width, height
      return Rect::Construct(nx, ny, nr - nx, nb - ny);
    }
    return {}; // or however you define an empty rect
  }
  bool Intersects(const Rect& other) const
  {
    return (Right() > other.Left()) && (Left() < other.Right()) && (Bottom() > other.Top()) && (Top() < other.Bottom());
  }
  Rect<T> Offset(T dx, T dy) const { return Construct(position.x + dx, position.y + dy, size.width, size.height); }

  Point<T> Clamp(const Point<T>& pt) const
  {
    T nx = (pt.x < Left()) ? Left() : (pt.x > Right() ? Right() : pt.x);
    T ny = (pt.y < Top()) ? Top() : (pt.y > Bottom() ? Bottom() : pt.y);
    return Point<T>::Construct(nx, ny);
  }

  [[nodiscard]] Span<T> TopSpan() const
  {
    return Span<T>::from_endpoints(Left(), Right()).with_length(size.width);
  }
  [[nodiscard]] Span<T> BottomSpan() const
  {
    return Span<T>::from_endpoints(Left(), Right()).with_length(size.width).translated(Bottom() - Top());
  }
  [[nodiscard]] Span<T> LeftSpan() const
  {
    return Span<T>::from_endpoints(Top(), Bottom()).with_length(size.height);
  }
  [[nodiscard]] Span<T> RightSpan() const
  {
    return Span<T>::from_endpoints(Top(), Bottom()).with_length(size.height).translated(Right() - Left());
  }
};

using RectI = Rect<int>;
using RectI16 = Rect<int16_t>;
using RectF = Rect<float>;


// ── Circle<T> ─────────────────────────────────────────────────────────
template<typename T>
struct Circle
{
  static_assert(std::is_arithmetic_v<T>, "Circle<T> requires an arithmetic T.");

  Vec2<T> mCenter{};
  T mRadius{}; // invariant: r >= 0 for signed T; always true for unsigned T

  // construction
  constexpr Circle() = default;
  constexpr Circle(const Vec2<T>& c, T r)
    : mCenter(c)
    , mRadius(r)
  {
    if constexpr (std::is_signed_v<T>) { /* assert(r >= T{0}); */
    }
  }

  // basic queries
  [[nodiscard]] constexpr Vec2<T> center() const noexcept { return mCenter; }
  [[nodiscard]] constexpr T radius() const noexcept { return mRadius; }
  [[nodiscard]] constexpr T diameter() const noexcept { return mRadius * T{ 2 }; }
  [[nodiscard]] constexpr bool empty() const noexcept
  {
    if constexpr (std::is_signed_v<T>)
      return mRadius <= T{ 0 };
    else
      return mRadius == T{ 0 };
  }

  // metrics
  [[nodiscard]] T area() const noexcept
  {
    using std::numbers::pi_v;
    return pi_v<T> * mRadius * mRadius;
  }
  [[nodiscard]] T circumference() const noexcept
  {
    using std::numbers::pi_v;
    return T{ 2 } * pi_v<T> * mRadius;
  }

  // containment
  [[nodiscard]] bool contains(const Vec2<T>& p) const noexcept { return distance2(p, mCenter) <= mRadius * mRadius; }
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
      return false; // too far
    const T diff = (mRadius > other.mRadius) ? (mRadius - other.mRadius) : (other.mRadius - mRadius);
    if (d2 < diff * diff)
      return false; // one completely inside without touching
    return true;    // overlapping or tangent
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
    return distance(p, mCenter) - mRadius; // inside: negative; on: 0; outside: positive
  }
  [[nodiscard]] T distance_to(const Vec2<T>& p) const noexcept
  {
    const T s = signed_distance(p);
    return (s > T{ 0 }) ? s : T{ 0 };
  }
  [[nodiscard]] Vec2<T> closest_point(const Vec2<T>& p) const noexcept
  {
    const Vec2<T> v = p - mCenter;
    const T len = v.length();
    if (len <= mRadius || len == T{ 0 })
      return p; // already inside/on, or degenerate
    return mCenter + v * (mRadius / len);
  }

  // bounding box (AABB): min, max
  [[nodiscard]] constexpr std::pair<Vec2<T>, Vec2<T>> aabb() const noexcept
  {
    const Vec2<T> r{ mRadius, mRadius };
    return { mCenter - r, mCenter + r };
  }

  // transforms
  [[nodiscard]] constexpr Circle translated(const Vec2<T>& delta) const noexcept
  {
    return { mCenter + delta, mRadius };
  }
  [[nodiscard]] constexpr Circle with_center(const Vec2<T>& c) const noexcept { return { c, mRadius }; }
  [[nodiscard]] constexpr Circle with_radius(T r) const noexcept { return { mCenter, (r > T{ 0 }) ? r : T{ 0 } }; }

  // uniform scale about origin or pivot (radius scales by |s|)
  [[nodiscard]] constexpr Circle scaled(T s) const noexcept
  {
    const T ar = (s >= T{ 0 }) ? (mRadius * s) : (mRadius * -s);
    return { mCenter * s, ar };
  }

  // parameterization & angles
  // point at angle 'theta' (radians)
  template<typename U>
  [[nodiscard]] Vec2<std::common_type_t<T, U>> point_at(U theta) const noexcept
  {
    using R = std::common_type_t<T, U>;
    R ct = std::cos(static_cast<R>(theta));
    R st = std::sin(static_cast<R>(theta));
    return { static_cast<R>(mCenter.x) + static_cast<R>(mRadius) * ct,
             static_cast<R>(mCenter.y) + static_cast<R>(mRadius) * st };
  }
  // angle of a point relative to center (radians)
  template<typename U>
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
    const T newR = (d + a.mRadius + b.mRadius) / T{ 2 };
    const T t = (newR - a.mRadius) / d; // fraction from a.center towards b.center
    const Vec2<T> newC = { a.mCenter.x + (b.mCenter.x - a.mCenter.x) * t,
                           a.mCenter.y + (b.mCenter.y - a.mCenter.y) * t };
    return { newC, newR };
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

    if (d > r0 + r1 || d < std::abs(r0 - r1) || (d == T{ 0 } && r0 == r1)) {
      return std::nullopt; // separate, contained w/out touching, or coincident infinite points
    }

    // a = distance from c0 to chord center along the center line
    const T a = (r0 * r0 - r1 * r1 + d * d) / (T{ 2 } * d);
    const T h2 = r0 * r0 - a * a;
    const T h = (h2 > T{ 0 }) ? std::sqrt(h2) : T{ 0 };

    const Vec2<T> dir = (d > T{ 0 }) ? (dC / d) : Vec2<T>{ T{ 1 }, T{ 0 } }; // fallback direction
    const Vec2<T> p0 = mCenter + dir * a;

    // perpendicular vector
    const Vec2<T> perp{ -dir.y, dir.x };
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

    if (left < T{ 0 })
      out.mCenter.x -= left;
    if (right < T{ 0 })
      out.mCenter.x += right;
    if (bottom < T{ 0 })
      out.mCenter.y -= bottom;
    if (top < T{ 0 })
      out.mCenter.y += top;

    if (shrink) {
      const T maxR =
        std::min(
            { out.mCenter.x - rmin.x, rmax.x - out.mCenter.x, out.mCenter.y - rmin.y, rmax.y - out.mCenter.y }
        );
      if (maxR < out.mRadius)
        out.mRadius = std::max(T{ 0 }, maxR);
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



} // namespace clarinoid
