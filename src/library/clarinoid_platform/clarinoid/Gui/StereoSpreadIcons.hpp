

#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

static const uint8_t PROGMEM gStereoSpreadBMP_P1[] = {
    B__x_____, B______x_, B________, //
    B_x__x___, B____x__x, B________, //
    Bxx_x__x_, B__x__x_x, Bx_______, //
    Bxx_x_x__, Bx__x_x_x, Bx_______, //
    Bxx_x__x_, B__x__x_x, Bx_______, //
    B_x__x___, B____x__x, B________, //
    B__x_____, B______x_, B________, //
};

static const BitmapSpec gStereoSpreadSpec_P1 = BitmapSpec::Construct(gStereoSpreadBMP_P1, 3, 17);

static const uint8_t PROGMEM gStereoSpreadBMP_P0_75[] = {
    B___x____,
    B_____x__, //
    B__x_____,
    B______x_, //
    B_x__x___,
    B____x__x, //
    B_x__x_x_,
    B__x_x__x, //
    B_x__x___,
    B____x__x, //
    B__x_____,
    B______x_, //
    B___x____,
    B_____x__, //
};

static const BitmapSpec gStereoSpreadSpec_P0_75 = BitmapSpec::Construct(gStereoSpreadBMP_P0_75, 2, 16);

static const uint8_t PROGMEM gStereoSpreadBMP_P0_50[] = {
    B___x____,
    B_____x__, //
    B__x__x__,
    B___x__x_, //
    B__x_x___,
    B____x_x_, //
    B__x_x_x_,
    B__x_x_x_, //
    B__x_x___,
    B____x_x_, //
    B__x__x__,
    B___x__x_, //
    B___x____,
    B_____x__, //
};

static const BitmapSpec gStereoSpreadSpec_P0_50 = BitmapSpec::Construct(gStereoSpreadBMP_P0_50, 2, 16);

static const uint8_t PROGMEM gStereoSpreadBMP_P0_25[] = {
    B____x___,
    B____x___, //
    B___x__x_,
    B__x__x__, //
    B___x_x__,
    B___x_x__, //
    B___x_x__,
    B___x_x__, //
    B___x_x__,
    B___x_x__, //
    B___x__x_,
    B__x__x__, //
    B____x___,
    B____x___, //
};

static const BitmapSpec gStereoSpreadSpec_P0_25 = BitmapSpec::Construct(gStereoSpreadBMP_P0_25, 2, 16);

static const uint8_t PROGMEM gStereoSpreadBMP_0[] = {
    B____x___,
    B____x___, //
    B___x____,
    B_____x__, //
    B___x____,
    B_____x__, //
    B___x____,
    B_____x__, //
    B___x____,
    B_____x__, //
    B___x____,
    B_____x__, //
    B____x___,
    B____x___, //
};

static const BitmapSpec gStereoSpreadSpec_0 = BitmapSpec::Construct(gStereoSpreadBMP_0, 2, 16);

static const uint8_t PROGMEM gStereoSpreadBMP_N0_25[] = {
    B___x____,
    B_____x__, //
    B____x___,
    B____x___, //
    B____x___,
    B____x___, //
    B____x_x_,
    B__x_x___, //
    B____x___,
    B____x___, //
    B____x___,
    B____x___, //
    B___x____,
    B_____x__, //
};

static const BitmapSpec gStereoSpreadSpec_N0_25 = BitmapSpec::Construct(gStereoSpreadBMP_N0_25, 2, 16);

static const uint8_t PROGMEM gStereoSpreadBMP_N0_50[] = {
    B____x___,
    B____x___, //
    B_____x__,
    B___x____, //
    B_____x_x,
    B_x_x____, //
    B_____x_x,
    B_x_x____, //
    B_____x_x,
    B_x_x____, //
    B_____x__,
    B___x____, //
    B____x___,
    B____x___, //
};

static const BitmapSpec gStereoSpreadSpec_N0_50 = BitmapSpec::Construct(gStereoSpreadBMP_N0_50, 2, 16);

static const uint8_t PROGMEM gStereoSpreadBMP_N0_75[] = {
    B_____x__,
    B___x____, //
    B______x_,
    B__x_____, //
    B______x_,
    Bx_x_____, //
    B______x_,
    Bx_x_____, //
    B______x_,
    Bx_x_____, //
    B______x_,
    B__x_____, //
    B_____x__,
    B___x____, //
};

static const BitmapSpec gStereoSpreadSpec_N0_75 = BitmapSpec::Construct(gStereoSpreadBMP_N0_75, 2, 16);

static const uint8_t PROGMEM gStereoSpreadBMP_N1[] = {
    B_______x,
    B_x______, //
    B________,
    Bx_______, //
    B________,
    Bx_______, //
    B________,
    Bx_______, //
    B________,
    Bx_______, //
    B________,
    Bx_______, //
    B_______x,
    B_x______, //
};

static const BitmapSpec gStereoSpreadSpec_N1 = BitmapSpec::Construct(gStereoSpreadBMP_N1, 2, 16);

static const BitmapSpec &GetStereoSpreadBitmapSpec(float spread)
{
    static const BitmapSpec *gIcons[] = {
        &gStereoSpreadSpec_N1,
        &gStereoSpreadSpec_N0_75,
        &gStereoSpreadSpec_N0_50,
        &gStereoSpreadSpec_N0_25,
        &gStereoSpreadSpec_0,
        &gStereoSpreadSpec_P0_25,
        &gStereoSpreadSpec_P0_50,
        &gStereoSpreadSpec_P0_75,
        &gStereoSpreadSpec_P1,
    };

    static constexpr float SegmentSize = 1.0f / SizeofStaticArray(gIcons);

    static const float gBoundaries[SizeofStaticArray(gIcons) - 1] = {
        (1.0f * SegmentSize) * 2 - 1,
        (2.0f * SegmentSize) * 2 - 1,
        (3.0f * SegmentSize) * 2 - 1,
        (4.0f * SegmentSize) * 2 - 1,
        (5.0f * SegmentSize) * 2 - 1,
        (6.0f * SegmentSize) * 2 - 1,
        (7.0f * SegmentSize) * 2 - 1,
        (8.0f * SegmentSize) * 2 - 1,
    };

    for (size_t i = 0; i < SizeofStaticArray(gBoundaries); ++i)
    {
        if (gBoundaries[i] > spread)
        {
            return *gIcons[i];
        }
    }
    return *gIcons[SizeofStaticArray(gIcons) - 1];
}

} // namespace clarinoid
