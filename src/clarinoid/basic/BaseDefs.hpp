

#pragma once

namespace clarinoid
{

// clarinoid custom fonts will define these characters
#define CHARSTR_DB "\x7f"
#define CHARSTR_QEQ "\x80"
#define CHARSTR_INFINITY "\x81"
#define CHARSTR_SHARP "\x82"
#define CHARSTR_FLAT "\x83"
#define CHARSTR_NARROWPLUS "\x84"
#define CHARSTR_NARROWMINUS "\x85"
#define CHARSTR_NARROWPLUSMINUS "\x86"
#define CHARSTR_DIGITWIDTHSPACE "\x87"

template <typename T, size_t N>
constexpr size_t SizeofStaticArray(const T (&x)[N])
{
    return N;
}
struct PointI
{
    int x;
    int y;
    static PointI Construct(int x_, int y_)
    {
        PointI ret;
        ret.x = x_;
        ret.y = y_;
        return ret;
    }
};

struct PointF
{
    float x;
    float y;
    static PointF Construct(float x_, float y_)
    {
        PointF ret;
        ret.x = x_;
        ret.y = y_;
        return ret;
    }
    PointF Add(const PointF& rhs) const {
        return PointF::Construct(x + rhs.x, y + rhs.y);
    }
    PointF Add(const PointI& rhs) const {
        return PointF::Construct(x + rhs.x, y + rhs.y);
    }
};

struct RectI
{
    static RectI Construct(int x, int y, int w, int h)
    {
        RectI ret;
        ret.x = x;
        ret.y = y;
        ret.width = w;
        ret.height = h;
        return ret;
    }
    int x;
    int y;
    int width;
    int height;
    int right() const
    {
        return x + width;
    }
    int bottom() const
    {
        return y + height;
    }
    RectI Inflate(int n) const
    {
        return Construct(x - n, y - n, width + n + n, height + n + n);
    }
    PointI UpperLeft() const
    {
        return PointI::Construct(x, y);
    }
    PointI UpperRight() const
    {
        return PointI::Construct(x + width, y);
    }
    PointI BottomLeft() const
    {
        return PointI::Construct(x, y + height);
    }
    PointI BottomRight() const
    {
        return PointI::Construct(x + width, y + height);
    }
};
struct RectF
{
    float x;
    float y;
    float width;
    float height;
};

struct ColorF
{
    float r;
    float g;
    float b;
};
struct ColorByte
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};


} // namespace clarinoid


