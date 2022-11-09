

#pragma once
#include <array>

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
struct StaticArray
{
    T (&mArray)[N];
    static constexpr size_t Size = N;
    StaticArray(T (&x)[N]) : mArray(x)
    {
    }
};

template <typename T, size_t N>
constexpr size_t SizeofStaticArray(const T (&x)[N])
{
    return N;
}

template <typename T, size_t N>
void CopyPODArray(const T (&from)[N], T (&to)[N])
{
    memcpy(to, from, sizeof(T) * N);
}

template <typename T>
void CopyPODArray(const T *from, T *to, size_t N)
{
    memcpy(to, from, sizeof(T) * N);
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
    PointF Add(const PointF &rhs) const
    {
        return PointF::Construct(x + rhs.x, y + rhs.y);
    }
    PointF Add(const PointI &rhs) const
    {
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
    static RectI Construct(PointI upperLeft, int w, int h)
    {
        RectI ret;
        ret.x = upperLeft.x;
        ret.y = upperLeft.y;
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
    bool YInRect(int y) const
    {
        return (y >= this->y) && (y < this->right());
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

struct BitmapSpec
{
    const uint8_t *pBmp = nullptr;
    int bitmapDataSizeBytes = 0;
    uint8_t widthBytes = 0;
    uint8_t widthPixels = 0;
    uint8_t heightPixels = 0;

    template <size_t BitmapDataSizeBytes>
    static BitmapSpec Construct(const uint8_t (&bmpBytes)[BitmapDataSizeBytes], uint8_t widthBytes, uint8_t widthPixels)
    {
        BitmapSpec ret;
        ret.pBmp = bmpBytes;
        ret.widthBytes = widthBytes;
        ret.widthPixels = widthPixels;
        ret.bitmapDataSizeBytes = BitmapDataSizeBytes;
        ret.heightPixels = BitmapDataSizeBytes / ret.widthBytes;
        return ret;
    }
};

// #define STATIC_INITIALIZER_LIST_1 {0}
// #define STATIC_INITIALIZER_LIST_2 {0},{1}
// #define STATIC_INITIALIZER_LIST_3 {0},{1},{2}
// #define STATIC_INITIALIZER_LIST_4 {0},{1},{2},{3}
// #define STATIC_INITIALIZER_LIST_5 {0},{1},{2},{3},{4}
// #define STATIC_INITIALIZER_LIST_6 {0},{1},{2},{3},{4},{5}
// #define STATIC_INITIALIZER_LIST_7 {0},{1},{2},{3},{4},{5},{6}
// #define STATIC_INITIALIZER_LIST_8 {0},{1},{2},{3},{4},{5},{6},{7}
// #define STATIC_INITIALIZER_LIST_9 {0},{1},{2},{3},{4},{5},{6},{7},{8}
// #define STATIC_INITIALIZER_LIST_10 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9}
// #define STATIC_INITIALIZER_LIST_11 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10}
// #define STATIC_INITIALIZER_LIST_12 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11}
// #define STATIC_INITIALIZER_LIST_13 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12}
// #define STATIC_INITIALIZER_LIST_14 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13}
// #define STATIC_INITIALIZER_LIST_15 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14}
// #define STATIC_INITIALIZER_LIST_16 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15}
// #define STATIC_INITIALIZER_LIST_17 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15},{16}
// #define STATIC_INITIALIZER_LIST_18 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15},{16},{17}
// #define STATIC_INITIALIZER_LIST_19 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15},{16},{17},{18}
// #define STATIC_INITIALIZER_LIST_20 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15},{16},{17},{18},{19}
// #define STATIC_INITIALIZER_LIST_21 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15},{16},{17},{18},{19},{20}
// #define STATIC_INITIALIZER_LIST_22 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15},{16},{17},{18},{19},{20},{21}
// #define STATIC_INITIALIZER_LIST_23 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15},{16},{17},{18},{19},{20},{21},{22}
// #define STATIC_INITIALIZER_LIST_24 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15},{16},{17},{18},{19},{20},{21},{22},{23}
// #define STATIC_INITIALIZER_LIST_25 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15},{16},{17},{18},{19},{20},{21},{22},{23},{24}
// #define STATIC_INITIALIZER_LIST_26 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15},{16},{17},{18},{19},{20},{21},{22},{23},{24},{25}
// #define STATIC_INITIALIZER_LIST_27 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15},{16},{17},{18},{19},{20},{21},{22},{23},{24},{25},{26}
// #define STATIC_INITIALIZER_LIST_28 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15},{16},{17},{18},{19},{20},{21},{22},{23},{24},{25},{26},{27}
// #define STATIC_INITIALIZER_LIST_29 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15},{16},{17},{18},{19},{20},{21},{22},{23},{24},{25},{26},{27},{28}
// #define STATIC_INITIALIZER_LIST_30 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15},{16},{17},{18},{19},{20},{21},{22},{23},{24},{25},{26},{27},{28},{29}
// #define STATIC_INITIALIZER_LIST_31 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15},{16},{17},{18},{19},{20},{21},{22},{23},{24},{25},{26},{27},{28},{29},{30}
// #define STATIC_INITIALIZER_LIST_32 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15},{16},{17},{18},{19},{20},{21},{22},{23},{24},{25},{26},{27},{28},{29},{30},{31}
// #define STATIC_INITIALIZER_LIST_48 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15},{16},{17},{18},{19},{20},{21},{22},{23},{24},{25},{26},{27},{28},{29},{30},{31},{32},{33},{34},{35},{36},{37},{38},{39},{40},{41},{42},{43},{44},{45},{46},{47}
// #define STATIC_INITIALIZER_LIST_80 {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15},{16},{17},{18},{19},{20},{21},{22},{23},{24},{25},{26},{27},{28},{29},{30},{31},{32},{33},{34},{35},{36},{37},{38},{39},{40},{41},{42},{43},{44},{45},{46},{47},{48},{49},{50},{51},{52},{53},{54},{55},{56},{57},{58},{59},{60},{61},{62},{63},{64},{65},{66},{67},{68},{69},{70},{71},{72},{73},{74},{75},{76},{77},{78},{79}


// https://stackoverflow.com/a/74351185/402169
template<typename T, std::size_t N, std::size_t... Is>
constexpr std::array<T, N> initialize_array_with_indices_helper(std::index_sequence<Is...>) {
    return std::array<T, N>{Is...};
}

template<typename T, std::size_t N>
constexpr std::array<T,N> initialize_array_with_indices() {
    return initialize_array_with_indices_helper<T, N>(std::make_index_sequence<N>());
}

} // namespace clarinoid
