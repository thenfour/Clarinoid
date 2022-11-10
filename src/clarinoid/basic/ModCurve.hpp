
#pragma once

namespace clarinoid
{
namespace ModCurve
{

static double ClampD(double x, double low, double hi)
{
    if (x <= low)
        return low;
    if (x >= hi)
        return hi;
    return x;
}

// valid for 0<k<1 and 0<x<1
static inline double modCurve_x01_k01(double x, double k)
{
    double ret = 1 - ::pow(x, k);
    return ::pow(ret, 1.0 / k);
}
// extends range to support -1<x<0 and -1<k<0
// outputs -1 to 1
static inline double modCurve_xN11_kN11(double x, double k)
{
    static constexpr double CornerMargin = 0.77;
    k *= CornerMargin;
    k = ClampD(k, -CornerMargin, CornerMargin);
    if (k >= 0)
    {
        if (x > 0)
        {
            return 1.0 - modCurve_x01_k01(x, 1.0 - k);
        }
        return modCurve_x01_k01(-x, 1.0 - k) - 1;
    }
    if (x > 0)
    {
        return modCurve_x01_k01(1.0 - x, 1.0 + k);
    }
    return -modCurve_x01_k01(x + 1, 1.0 + k);
}

// X = sample values
// Y = k range
template <int LutSizeXLog2, int LutSizeYLog2, int ValueScaleLog2>
struct ModulationCurveLUT
{
    using q15_t = int16_t;
    using q31_t = int32_t;
    using q63_t = int64_t;
    // optimized version of arm_linear_interp_q15, by assuming x is in range, and 16.16 format instead of 12.20.
    // that avoids extra shifting to cram the 16-bit sample value into the 12-bit integral portion
    q15_t arm_linear_interp_q15(const q15_t *pYData, q31_t x)
    {
        q63_t y;       /* output */
        q15_t y0, y1;  /* Nearest output values */
        q31_t fract;   /* fractional part */
        int32_t index; /* Index to read nearest output values */

        /* Input is in 16.16 format */
        /* 16 bits for the table index */
        /* Index value calculation */
        index = (x >> 16);

        /* 16 bits for the fractional part */
        /* fract is in 16.16 format */
        fract = (x & 0xFFFF);

        /* Read two nearest output values from the index */
        y0 = pYData[index];
        y1 = pYData[index + 1];

        /* Calculation of y0 * (1-fract) and y is in 13.35 format */
        y = ((q63_t)y0 * (0xFFFF - fract));

        /* Calculation of (y0 * (1-fract) + y1 * fract) and y is in 13.35 format */
        y += ((q63_t)y1 * (fract));

        /* convert y to 1.15 format */
        return (q15_t)(y >> 16);
    }

    static constexpr int LutSizeX = 1 << LutSizeXLog2;
    static constexpr int LutSizeY = 1 << LutSizeYLog2;
    static constexpr int LinearYIndex = LutSizeY / 2;
    static constexpr int LutSizeTotal = 1 << (LutSizeXLog2 + LutSizeYLog2);
    static constexpr int ValueScale = 1 << ValueScaleLog2;

    // number of bits of headroom that the sample value can support as 32-bit.
    // so since we are dealing with 16-bit signed ints, they can be shifted up to 15 bits
    // without clipping in a 32-bit value.
    static constexpr int IntegralPartHeadroomBits = 14;

    // think of the transfer as:
    // out = (sampleval * lutsize) / valuescale
    // or,
    // out = (sampleVal / (valuescale / lutsize)
    // since (valuescale / lutsize) is a constant, we can be more efficient. a divide is necessary anyway,
    // might as well put as much into the divide as possible.
    // - it's valuescale*2, because the samplevalue is shifted into positive range, then must be divided by 2. this does
    // that scaling.
    // - it's lutsizeX-1, in order for 0 to return 0. LUTs are even # of bins, like [-32768, -16384, +16384, 32767].
    //   so if you want 0 to produce 0 out of that, you need to treat 32767 as an "end" value. so the size to divide by
    //   is 3 (bins - 1).
    // - +.49999 so it rounds as we convert to a uint32_t.
    static constexpr uint32_t TransferDenominator = (uint32_t)(double(ValueScale * 2) / (LutSizeX - 1) + .499999999);

    const int16_t *mpLut = nullptr;

    void setLUTData(int16_t *plut) {
        mpLut = plut;
    }

    explicit ModulationCurveLUT(const int16_t *plut) : mpLut(plut)
    {
    }

#ifndef ARDUINO
    ModulationCurveLUT()
    {
        // because the 12.20 LUT index format, Lut sizes of > 12 would break the interpolation.
        static_assert(LutSizeXLog2 <= 12, "Lut too big");

        // initialize the LUT
        mpLut = new int16_t[LutSizeTotal];
        for (size_t iy = 0; iy < LutSizeY; ++iy)
        {
            for (size_t ix = 0; ix < LutSizeX; ++ix)
            {
                double k = double(iy) / (LutSizeY - 1) * 2 - 1; // [-1,1]
                double ix2 = ((double(ix) / (LutSizeX - 1)) * 2) - 1;
                auto curveOutp = modCurve_xN11_kN11(ix2, k);
                mpLut[ix + iy * LutSizeX] = saturate16(ClampD(curveOutp, -1.0f, 1.0f) * ValueScale);
            }
        }
    }
#endif

    // from double to 12.20 integer
    // static int32_t To12p20(double x)
    //{
    //  return x *(1 << 20);
    //}

    // static uint32_t To12p20(uint32_t numerator, uint32_t denominator)
    // {
    //     // the naive way is like this:
    //     // int32_t i = numerator / denominator;
    //     // int32_t f = numerator - (i * denominator);
    //     // int32_t ret = i << 20;
    //     // ret |= (f << 20) / denominator;
    //     // return ret;

    //     // the most *effecient* way is:
    //     // return (numerator << 20) / denominator;
    //     // however this will clip values because you only have 12 bits of precision for the integral part.

    //     // so instead, calculate a different fixed precision format, and shift it into 12.20.
    //     // it means the denominator must be big enough to reduce the need for integral part precision.
    //     return ((numerator << IntegralPartHeadroomBits) / denominator) << (20 - IntegralPartHeadroomBits);
    // }

    // if we use 16.16 format, we save ourselves a shift (~10% faster), in exchange for a small loss of precision
    static uint32_t To16p16(uint32_t numerator, uint32_t denominator)
    {
        return (numerator << 16) / denominator;
    }

    // version of To12p20, but specialized when denominator can be represented as a right shift
    // this actually works very well and is very efficient, but unfortunately it can't support the "perfect 0"
    // behavior of using an actual divide. But the perf loss is pretty minor, just 1 divide instead of shift.
    // template <int IntegralPartHeadroomBits, uint32_t denominatorShift>
    // static uint32_t To12p20Shift(uint32_t numerator)
    //{
    //  return ((numerator << IntegralPartHeadroomBits) >> denominatorShift) << (20 - IntegralPartHeadroomBits);
    //}

    inline const q15_t *BeginLookupF(float kN11)
    {
        int32_t lutY = (kN11 * .5 + .5) * LutSizeY;
        return BeginLookupI(lutY);
    }

    inline const q15_t *BeginLookupI(int32_t lutY)
    {
        if (lutY >= LutSizeY)
            lutY = LutSizeY - 1; // for kN11 == 1.
        if (lutY < 0)
            lutY = 0;
        if (lutY == (LutSizeY / 2))
            return nullptr; // special case for linear
        auto ret = mpLut + LutSizeX * lutY;
        return ret;
    }

    inline int16_t Transfer16(int16_t inpVal, const q15_t *pLutRow)
    {
        if (!pLutRow)
            return inpVal;
        int32_t lutX12p20 = To16p16(int32_t(inpVal) + ValueScale, TransferDenominator);
        return arm_linear_interp_q15(pLutRow, lutX12p20);
    }
    inline float Transfer32(float inpVal, const q15_t *pLutRow)
    {
        if (!pLutRow)
            return inpVal;
        int32_t lutX12p20 = To16p16(int32_t(fast::Sample32To16(inpVal)) + ValueScale, TransferDenominator);
        return fast::Sample16To32(arm_linear_interp_q15(pLutRow, lutX12p20));
    }
};

} // namespace ModCurve
} // namespace clarinoid

#include "ModCurveLUT.hpp"

namespace clarinoid
{
    //static int16_t *gModCurveLUTData = nullptr;
    static const int16_t *gModCurveLUTData = gModCurveLUTData_PROGMEM;
    static ModCurve::ModulationCurveLUT<7, 7, 15> gModCurveLUT(gModCurveLUTData);

    StaticInit __initModCurveLUT([](){
        //gModCurveLUTData = new int16_t[SizeofStaticArray(gModCurveLUTData_PROGMEM)];
        // for (size_t i = 0; i < SizeofStaticArray(gModCurveLUTData_PROGMEM); ++ i){
        //     gModCurveLUTData[i] = gModCurveLUTData_PROGMEM[i];
        // }
        // gModCurveLUT.setLUTData(gModCurveLUTData);
    });
}


