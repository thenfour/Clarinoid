

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

// template <typename T, size_t N>
// struct StaticArray
// {
//     T (&mArray)[N];
//     static constexpr size_t Size = N;
//     StaticArray(T (&x)[N]) : mArray(x)
//     {
//     }
// };

template <typename T, size_t N>
constexpr size_t SizeofStaticArray(const T (&x)[N])
{
    return N;
}

// template <typename T, size_t N>
// void CopyPODArray(const T (&from)[N], T (&to)[N])
// {
//     memcpy(to, from, sizeof(T) * N);
// }

// template <typename T>
// void CopyPODArray(const T *from, T *to, size_t N)
// {
//     memcpy(to, from, sizeof(T) * N);
// }

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

// effectively just a tuple<bool, string>
struct Result
{
    bool mSuccess = false;
    String mMessage{};
    Result() = default;
    Result (const Result & rhs) = default;
    Result(bool success, const String &message) :mSuccess(success), mMessage(message)
    {
    }
    Result(bool success) : mSuccess(success)
    {
    }
    bool IsSuccess() const
    {
        return mSuccess;
    }
    bool IsFailure() const
    {
        return !mSuccess;
    }
    static Result Failure(const String &message)
    {
        return {false, message};
    }
    static Result Failure()
    {
        return {false};
    }
    static Result Success(const String &message)
    {
        return {true, message};
    }
    static Result Success()
    {
        return {true};
    }

    // use for chaining conditions together and accumulating error status.
    // when condition is false, we return false after setting the message.
    // enables this kind of usage:
    // Result ret = Result::Success();
    // if (!ret.AndRequires(trySomething(), "trySomething failed"))) return ret;
    // if (!ret.AndRequires(trySomethingElse(), "trySomethingElse failed"))) return ret;
    // if (!ret.AndRequires(tryAnotherThing(), "tryAnotherThing failed"))) return ret;
    // or,
    // Result ret = Result::Success();
    // ret.AndRequires(trySomething(), "trySomething failed"));
    // ret.AndRequires(trySomethingElse(), "trySomethingElse failed"));
    // ret.AndRequires(tryAnotherThing(), "tryAnotherThing failed"));
    // if (!ret) return ret; // now contains all the error messages concatenated.
    bool AndRequires(bool condition, const String& messageIfFailure)
    {
        if (!condition) {
            mMessage += messageIfFailure;
        }
        mSuccess = mSuccess && condition;
        return mSuccess;
    }

    // when condition is false, we return false after setting the message.
    // clears any existing state.
    // enables this kind of usage:
    // Result ret;
    // if (!ret.Requires(trySomething(), "trySomething failed"))) return ret;
    // if (!ret.Requires(trySomethingElse(), "trySomethingElse failed"))) return ret;
    // if (!ret.Requires(tryAnotherThing(), "tryAnotherThing failed"))) return ret;
    bool Requires(bool condition, const String& messageIfFailure)
    {
        if (!condition) {
            mMessage = messageIfFailure;
        }
        mSuccess = condition;
        return mSuccess;
    }

    // use when an inner call returns a Result, but you want to add a prefix msg in case of error.
    bool Requires(const Result& rhs, const String& messagePrefix)
    {
        mSuccess = rhs.mSuccess;
        if (!mSuccess) {
            mMessage = messagePrefix + rhs.mMessage;
        }
        return mSuccess;
    }
};

// https://stackoverflow.com/a/74351185/402169
template <typename T, std::size_t N, std::size_t... Is>
constexpr std::array<T, N> initialize_array_with_indices_helper(std::index_sequence<Is...>)
{
    return std::array<T, N>{Is...};
}

template <typename T, std::size_t N>
constexpr std::array<T, N> initialize_array_with_indices()
{
    return initialize_array_with_indices_helper<T, N>(std::make_index_sequence<N>());
}

} // namespace clarinoid
