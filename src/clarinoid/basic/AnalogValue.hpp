
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

static const size_t ANALOG_RESOLUTION_BITS = 12;

template <size_t BitsResolution = ANALOG_RESOLUTION_BITS>
struct AnalogValue01
{
    using this_t = AnalogValue01<BitsResolution>;
    void SetFloat(float v)
    {
        mFloatVal = v;
        if (v >= 1.0f)
        {
            mIntVal = (1 << BitsResolution) - 1;
        }
        else if (v <= 0)
        {
            mIntVal = 0;
        }
        else
        {
            mIntVal = (uint16_t)(v * (1 << BitsResolution));
        }
    }
    void Deserialize12Bit(uint16_t v)
    {
        mIntVal = v;
        mFloatVal = (float)v / (1 << BitsResolution);
    }
    uint16_t Serialize12Bit() const
    {
        return mIntVal;
    }
    float GetFloatVal() const
    {
        return mFloatVal;
    }
    bool operator==(const this_t &rhs) const
    {
        return mIntVal == rhs.mIntVal;
    }
    bool operator!=(const this_t &rhs) const
    {
        return !(*this == rhs);
    }
    bool operator!=(float rhs) const
    {
        return mFloatVal != rhs;
    }
    this_t &operator=(float v)
    {
        SetFloat(v);
        return *this;
    }

  private:
    uint16_t mIntVal = 0;
    float mFloatVal = 0;
};

template <size_t BitsResolution = ANALOG_RESOLUTION_BITS>
struct AnalogValueN11
{
    using this_t = AnalogValueN11<BitsResolution>;
    using Int = uint16_t;
    static constexpr uint16_t IntStorageSize = (1 << BitsResolution);
    static constexpr uint16_t IntMaxVal = IntStorageSize - 1;

    void SetFloat(float v)
    {
        mFloatVal = v;

        v += 1.0f;           // now 0-2.
        v *= IntStorageSize; // because v could have been 2, it's now packed into bitsres+1 values
        v /= 2;              // now packed to bitsres values.
        if (v <= 0)
        {
            mIntVal = 0;
        }
        else if (v >= IntStorageSize)
        {
            mIntVal = IntMaxVal;
        }
        else
        {
            mIntVal = (Int)v;
        }
    }
    void Deserialize12Bit(uint16_t v)
    {
        mIntVal = v;
        mFloatVal = v;               // 0-1024
        mFloatVal *= 2;              // 0-2048
        mFloatVal /= IntStorageSize; // 0-2
        mFloatVal -= 1;              // -1 to 1
    }

    // trivial...

    uint16_t Serialize12Bit() const
    {
        return mIntVal;
    }
    float GetFloatVal() const
    {
        return mFloatVal;
    }
    bool operator==(const this_t &rhs) const
    {
        return mIntVal == rhs.mIntVal;
    }
    bool operator!=(const this_t &rhs) const
    {
        return !(*this == rhs);
    }
    bool operator!=(float rhs) const
    {
        return mFloatVal != rhs;
    }
    this_t &operator=(float v)
    {
        SetFloat(v);
        return *this;
    }

  private:
    uint16_t mIntVal = 0;
    float mFloatVal = 0;
};

} // namespace clarinoid
