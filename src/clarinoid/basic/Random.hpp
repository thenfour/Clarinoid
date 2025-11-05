#pragma once
#include <cstdint>
#include <clarinoid/basic/Uptime.hpp>
// #include <Entropy.h>

namespace clarinoid
{
// StaticInit gRandomInit([]() { Entropy.Initialize(); });

struct RandomNumberGenerator
{
    static constexpr float inv24 = 1.0f / 16777216.0f; // 1 / 2^24
    uint32_t mRngState;

    // explicit RandomNumberGenerator(uint32_t seed = 0) : mRngState(seed ? seed : Entropy.random())
    explicit RandomNumberGenerator(uint32_t seed = 0) : mRngState(seed ? seed : UptimeMicros())
    {
    }

    // LCG step (Numerical Recipes constants)
    inline uint32_t next_u32()
    {
        mRngState = mRngState * 1664525u + 1013904223u;
        return mRngState;
    }

    // // Xorshift step would be theoretically better but not critical and slightly more ops.
    // inline uint32_t next_u32()
    // {
    //     uint32_t x = mRngState ? mRngState : 0x9E3779B9u;
    //     x ^= x << 13;
    //     x ^= x >> 17;
    //     x ^= x << 5;
    //     mRngState = x;
    //     return x;
    // }

    // [0,255]
    uint8_t NextRandomByte()
    {
        return static_cast<uint8_t>(next_u32() >> 24);
    }

    // [0,1) using 24 bits of precision (2^-24 steps)
    float NextFloat01()
    {
        // Take the top 24 bits (better quality than low bits on LCGs)
        const uint32_t r24 = next_u32() >> 8; // 24 bits
        return r24 * inv24;                   // 1 / 2^24
    }

    // [-1,1)
    float NextFloatN11()
    {
        return (NextFloat01() * 2.0f) - 1.0f;
    }
};

} // namespace clarinoid
