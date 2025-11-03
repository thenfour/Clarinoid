#pragma once

#include <cmath>

namespace clarinoid
{

struct Vec2f
{
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vec2f() = default;
    constexpr Vec2f(float ix, float iy) : x(ix), y(iy)
    {
    }

    constexpr Vec2f operator+(const Vec2f &rhs) const
    {
        return Vec2f{x + rhs.x, y + rhs.y};
    }
    constexpr Vec2f operator-(const Vec2f &rhs) const
    {
        return Vec2f{x - rhs.x, y - rhs.y};
    }
    constexpr Vec2f operator*(float s) const
    {
        return Vec2f{x * s, y * s};
    }
    constexpr Vec2f operator-() const
    {
        return Vec2f{-x, -y};
    }

    Vec2f &operator+=(const Vec2f &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    Vec2f &operator-=(const Vec2f &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    Vec2f &operator*=(float s)
    {
        x *= s;
        y *= s;
        return *this;
    }
};

inline Vec2f operator*(float s, const Vec2f &v)
{
    return v * s;
}

inline float Dot(const Vec2f &a, const Vec2f &b)
{
    return (a.x * b.x) + (a.y * b.y);
}

inline float LengthSq(const Vec2f &v)
{
    return Dot(v, v);
}

inline float Length(const Vec2f &v)
{
    return std::sqrt(LengthSq(v));
}

inline Vec2f Normalize(const Vec2f &v, float epsilon = 1e-6f)
{
    const float lenSq = LengthSq(v);
    if (lenSq <= epsilon * epsilon)
    {
        return Vec2f{};
    }

    const float invLen = 1.0f / std::sqrt(lenSq);
    return v * invLen;
}

struct Vec3f
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    constexpr Vec3f() = default;
    constexpr Vec3f(float ix, float iy, float iz) : x(ix), y(iy), z(iz)
    {
    }

    constexpr Vec3f operator+(const Vec3f &rhs) const
    {
        return Vec3f{x + rhs.x, y + rhs.y, z + rhs.z};
    }
    constexpr Vec3f operator-(const Vec3f &rhs) const
    {
        return Vec3f{x - rhs.x, y - rhs.y, z - rhs.z};
    }
    constexpr Vec3f operator*(float s) const
    {
        return Vec3f{x * s, y * s, z * s};
    }
    constexpr Vec3f operator-() const
    {
        return Vec3f{-x, -y, -z};
    }

    Vec3f &operator+=(const Vec3f &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    Vec3f &operator-=(const Vec3f &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    Vec3f &operator*=(float s)
    {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }
};

inline Vec3f operator*(float s, const Vec3f &v)
{
    return v * s;
}

inline float Dot(const Vec3f &a, const Vec3f &b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

inline Vec3f Cross(const Vec3f &a, const Vec3f &b)
{
    return Vec3f{(a.y * b.z) - (a.z * b.y), (a.z * b.x) - (a.x * b.z), (a.x * b.y) - (a.y * b.x)};
}

inline float LengthSq(const Vec3f &v)
{
    return Dot(v, v);
}

inline float Length(const Vec3f &v)
{
    return std::sqrt(LengthSq(v));
}

inline Vec3f Normalize(const Vec3f &v, float epsilon = 1e-6f)
{
    const float lenSq = LengthSq(v);
    if (lenSq <= epsilon * epsilon)
    {
        return Vec3f{};
    }

    const float invLen = 1.0f / std::sqrt(lenSq);
    return v * invLen;
}

} // namespace clarinoid
