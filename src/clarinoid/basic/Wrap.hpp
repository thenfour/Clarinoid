
#pragma once

#include <algorithm>
#include <cmath>

namespace clarinoid
{

// Non-negative remainder for 32-bit ints. Requires n > 0.
constexpr int32_t euclidean_mod32(int32_t a, int32_t n)
{
    int32_t r = a % n; // trunc toward zero
    return (r < 0) ? (r + n) : r;
}

// Runtime step & range.
// Returns false only if inputs invalid or no representative exists in [lo, hi].
inline bool wrapByStepIntoRange(int32_t value, int32_t lo, int32_t hi, int32_t step, int32_t &out)
{
    if (!(lo <= hi) || !(step > 0))
        return false;

    if (value < lo)
    {
        // First representative >= lo (lowest valid in-range)
        int32_t v = lo + euclidean_mod32(value - lo, step);
        if (v <= hi)
        {
            out = v;
            return true;
        }
        return false; // progression jumps over the interval
    }

    if (value > hi)
    {
        // Last representative <= hi (highest valid in-range)
        int32_t v = hi - euclidean_mod32(hi - value, step);
        if (v >= lo)
        {
            out = v;
            return true;
        }
        return false; // progression jumps over the interval
    }

    out = value; // already inside
    return true;
}

// Compile-time STEP.
template <int STEP>
inline bool wrapByStepIntoRange(int32_t value, int32_t lo, int32_t hi, int32_t &out)
{
    static_assert(STEP > 0, "STEP must be > 0");
    if (!(lo <= hi))
        return false;

    if (value < lo)
    {
        int32_t v = lo + euclidean_mod32(value - lo, STEP);
        if (v <= hi)
        {
            out = v;
            return true;
        }
        return false;
    }

    if (value > hi)
    {
        int32_t v = hi - euclidean_mod32(hi - value, STEP);
        if (v >= lo)
        {
            out = v;
            return true;
        }
        return false;
    }

    out = value;
    return true;
}

// Fully fixed range (max constant folding).
template <int LO, int HI, int STEP = 12>
constexpr bool wrapByStepIntoFixedRange(int32_t value, int32_t &out)
{
    static_assert(LO <= HI, "LO must be <= HI");
    static_assert(STEP > 0, "STEP must be > 0");

    if (value < LO)
    {
        int32_t v = LO + euclidean_mod32(value - LO, STEP);
        if (v <= HI)
        {
            out = v;
            return true;
        }
        return false;
    }

    if (value > HI)
    {
        int32_t v = HI - euclidean_mod32(HI - value, STEP);
        if (v >= LO)
        {
            out = v;
            return true;
        }
        return false;
    }

    out = value;
    return true;
}

inline float WrapAngle(float angle)
{
    static constexpr float twoPi = 6.28318530718f;
    if (angle > twoPi || angle < -twoPi)
    {
        angle = std::fmod(angle, twoPi);
    }
    return angle;
}

} // namespace clarinoid
