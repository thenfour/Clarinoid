#pragma once

#include <algorithm>
#include <cmath>
#include <clarinoid/basic/BaseDefs.hpp>
#include <clarinoid/basic/Vec.hpp>

namespace clarinoid::render3d
{

inline float WrapAngle(float angle)
{
    static constexpr float twoPi = 6.28318530718f;
    if (angle > twoPi || angle < -twoPi)
    {
        angle = std::fmod(angle, twoPi);
    }
    return angle;
}

inline float EdgeFunction(const Vec2f &a, const Vec2f &b, const Vec2f &c)
{
    return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

template <typename PixelFunc>
void RasterizeTriangle(const Vec2f &a,
                       const Vec2f &b,
                       const Vec2f &c,
                       const RectI &bounds,
                       PixelFunc &&setPixel,
                       float degenerateEpsilon = 1e-3f)
{
    const float area = EdgeFunction(a, b, c);
    if (std::fabs(area) <= degenerateEpsilon)
    {
        return;
    }
    const bool areaPositive = area > 0.0f;

    const float minXf = std::floor(std::min({a.x, b.x, c.x}));
    const float maxXf = std::ceil(std::max({a.x, b.x, c.x}));
    const float minYf = std::floor(std::min({a.y, b.y, c.y}));
    const float maxYf = std::ceil(std::max({a.y, b.y, c.y}));

    const int boundsLeft = bounds.x;
    const int boundsRight = bounds.x + bounds.width - 1;
    const int boundsTop = bounds.y;
    const int boundsBottom = bounds.y + bounds.height - 1;

    const int minX = std::max(boundsLeft, static_cast<int>(minXf));
    const int maxX = std::min(boundsRight, static_cast<int>(maxXf));
    const int minY = std::max(boundsTop, static_cast<int>(minYf));
    const int maxY = std::min(boundsBottom, static_cast<int>(maxYf));

    if (minX > maxX || minY > maxY)
    {
        return;
    }

    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            const Vec2f sample{static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f};
            const float w0 = EdgeFunction(b, c, sample);
            const float w1 = EdgeFunction(c, a, sample);
            const float w2 = EdgeFunction(a, b, sample);

            if (areaPositive)
            {
                if (w0 < 0.0f || w1 < 0.0f || w2 < 0.0f)
                {
                    continue;
                }
            }
            else
            {
                if (w0 > 0.0f || w1 > 0.0f || w2 > 0.0f)
                {
                    continue;
                }
            }

            setPixel(x, y);
        }
    }
}

} // namespace clarinoid::render3d
