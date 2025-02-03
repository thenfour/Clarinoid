#pragma once

namespace clarinoid {

#pragma once

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
  Rect WithWidth(T w) const
  {
    Rect r(*this);
    r.size.width = w;
    return r;
  }
  Rect WithHeight(T h) const
  {
    Rect r(*this);
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

  Rect WithBipolarVerticalFill(float fractionN11) const
  {
    if (fractionN11 >= 0) {
      return BottomHalf().TopFraction(fractionN11);
    }
    return TopHalf().BottomFraction(-fractionN11);
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
};

using RectI = Rect<int>;
using RectI16 = Rect<int16_t>;
using RectF = Rect<float>;

} // namespace clarinoid
