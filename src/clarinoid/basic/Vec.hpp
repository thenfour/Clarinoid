#pragma once

#include <cmath>
#include <cstddef>

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

struct Mat3f
{
    float m00 = 1.0f;
    float m01 = 0.0f;
    float m02 = 0.0f;
    float m10 = 0.0f;
    float m11 = 1.0f;
    float m12 = 0.0f;
    float m20 = 0.0f;
    float m21 = 0.0f;
    float m22 = 1.0f;

    constexpr Mat3f() = default;
    constexpr Mat3f(float im00,
                    float im01,
                    float im02,
                    float im10,
                    float im11,
                    float im12,
                    float im20,
                    float im21,
                    float im22)
        : m00(im00), m01(im01), m02(im02), m10(im10), m11(im11), m12(im12), m20(im20), m21(im21), m22(im22)
    {
    }

    constexpr Mat3f(const Vec3f &row0, const Vec3f &row1, const Vec3f &row2)
        : Mat3f(row0.x, row0.y, row0.z, row1.x, row1.y, row1.z, row2.x, row2.y, row2.z)
    {
    }

    static constexpr Mat3f Identity()
    {
        return Mat3f{};
    }

    constexpr Vec3f Row(size_t index) const
    {
        switch (index)
        {
        case 0:
            return Vec3f{m00, m01, m02};
        case 1:
            return Vec3f{m10, m11, m12};
        default:
            return Vec3f{m20, m21, m22};
        }
    }

    constexpr Vec3f Column(size_t index) const
    {
        switch (index)
        {
        case 0:
            return Vec3f{m00, m10, m20};
        case 1:
            return Vec3f{m01, m11, m21};
        default:
            return Vec3f{m02, m12, m22};
        }
    }

    constexpr Mat3f Transposed() const
    {
        return Mat3f{m00, m10, m20, m01, m11, m21, m02, m12, m22};
    }

    Vec3f operator*(const Vec3f &v) const
    {
        return Vec3f{
            (m00 * v.x) + (m01 * v.y) + (m02 * v.z),
            (m10 * v.x) + (m11 * v.y) + (m12 * v.z),
            (m20 * v.x) + (m21 * v.y) + (m22 * v.z),
        };
    }

    Mat3f operator*(const Mat3f &rhs) const
    {
        return Mat3f{
            (m00 * rhs.m00) + (m01 * rhs.m10) + (m02 * rhs.m20),
            (m00 * rhs.m01) + (m01 * rhs.m11) + (m02 * rhs.m21),
            (m00 * rhs.m02) + (m01 * rhs.m12) + (m02 * rhs.m22),
            (m10 * rhs.m00) + (m11 * rhs.m10) + (m12 * rhs.m20),
            (m10 * rhs.m01) + (m11 * rhs.m11) + (m12 * rhs.m21),
            (m10 * rhs.m02) + (m11 * rhs.m12) + (m12 * rhs.m22),
            (m20 * rhs.m00) + (m21 * rhs.m10) + (m22 * rhs.m20),
            (m20 * rhs.m01) + (m21 * rhs.m11) + (m22 * rhs.m21),
            (m20 * rhs.m02) + (m21 * rhs.m12) + (m22 * rhs.m22),
        };
    }

    Mat3f &operator*=(const Mat3f &rhs)
    {
        *this = *this * rhs;
        return *this;
    }

    static Mat3f RotationXYZ(float x, float y, float z)
    {
        const float sx = std::sin(x);
        const float cx = std::cos(x);
        const float sy = std::sin(y);
        const float cy = std::cos(y);
        const float sz = std::sin(z);
        const float cz = std::cos(z);
        return RotationXYZFromTrig(sx, cx, sy, cy, sz, cz);
    }

    static Mat3f RotationXYZ(const Vec3f &angles)
    {
        return RotationXYZ(angles.x, angles.y, angles.z);
    }

    static Mat3f RotationXYZFromTrig(float sx, float cx, float sy, float cy, float sz, float cz)
    {
        return Mat3f{
            (cz * cy),
            (cz * sy * sx) - (sz * cx),
            (cz * sy * cx) + (sz * sx),
            (sz * cy),
            (sz * sy * sx) + (cz * cx),
            (sz * sy * cx) - (cz * sx),
            -sy,
            (cy * sx),
            (cy * cx),
        };
    }
};

} // namespace clarinoid
