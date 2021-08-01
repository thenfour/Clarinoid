
#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

namespace KnobDetails
{
static const uint8_t PROGMEM gKnobOutlineBMP[] = {
    0b00000111, 0b11000000, //
    0b00011000, 0b00110000, //
    0b00100000, 0b00001000, //
    0b01000000, 0b00000100, //
    0b01000000, 0b00000100, //
    0b10000000, 0b00000010, //
    0b10000000, 0b00000010, //
    0b10000001, 0b00000010, //
    0b10000000, 0b00000010, //
    0b10000000, 0b00000010, //
    0b01000000, 0b00000100, //
    0b01000000, 0b00000100, //
    0b00100000, 0b00001000, //
};

static const BitmapSpec gKnobOutlineSpec = BitmapSpec::Construct(gKnobOutlineBMP, 2, 15);

// same thing but showing a static "-inf" indicator
static const uint8_t PROGMEM gKnobOutlineMutedBMP[] = {
    0b00000111, 0b11000000, //
    0b00011000, 0b00110000, //
    0b00100000, 0b00001000, //
    0b01000000, 0b00000100, //
    0b01000000, 0b00000100, //
    0b10000011, 0b01100010, //
    0b10000101, 0b10010010, //
    0b10110100, 0b11010010, //
    0b10000011, 0b01100010, //
    0b10000000, 0b00000010, //
    0b01000000, 0b00000100, //
    0b01000000, 0b00000100, //
    0b00100000, 0b00001000, //
};

static const BitmapSpec gKnobOutlineMutedSpec = BitmapSpec::Construct(gKnobOutlineMutedBMP, 2, 15);

static const float CenterX = 7.75f;
static const float CenterY = 7.5f;
static const float MinAngle = 2.35f;
static const float CenterAngle = 4.712388975f; // 1.5 pi
static const float MaxAngle = 7.15f;
static const float Radius = 7.1f;

static PointF Origin = PointF::Construct(CenterX, CenterY);

} // namespace KnobDetails

static const uint8_t PROGMEM gLowPassBMP[] = {
    0b00000000, 0b00000000, //
    0b00000000, 0b00000000, //
    0b00000000, 0b00000000, //
    0b00000000, 0b00000000, //
    0b01110000, 0b00000000, //
    0b01111000, 0b00000000, //
    0b01111100, 0b00000000, //
    0b01111110, 0b00000000, //
    0b01111111, 0b00000000, //
    0b01111111, 0b10000000, //
    0b01111111, 0b11000000, //
    0b01111111, 0b11100000, //
};
static const BitmapSpec gLowPassBitmapSpec = BitmapSpec::Construct(gLowPassBMP, 2, 15);

static const uint8_t PROGMEM gHighPassBMP[] = {
    0b00000000, 0b00000000, //
    0b00000000, 0b00000000, //
    0b00000000, 0b00000000, //
    0b00000000, 0b00000000, //
    0b00000000, 0b00001110, //
    0b00000000, 0b00011110, //
    0b00000000, 0b00111110, //
    0b00000000, 0b01111110, //
    0b00000000, 0b11111110, //
    0b00000001, 0b11111110, //
    0b00000011, 0b11111110, //
    0b00000111, 0b11111110, //
};
static const BitmapSpec gHighPassBitmapSpec = BitmapSpec::Construct(gHighPassBMP, 2, 15);

static const uint8_t PROGMEM gBandPassBMP[] = {
    0b00000000, 0b00000000, //
    0b00000000, 0b00000000, //
    0b00000000, 0b00000000, //
    0b00000000, 0b00000000, //
    0b00000111, 0b00000000, //
    0b00001111, 0b10000000, //
    0b00001111, 0b10000000, //
    0b00011111, 0b11000000, //
    0b00011111, 0b11000000, //
    0b00111111, 0b11100000, //
    0b00111111, 0b11100000, //
    0b01111111, 0b11110000, //
};
static const BitmapSpec gBandPassBitmapSpec = BitmapSpec::Construct(gBandPassBMP, 2, 15);

static const uint8_t PROGMEM gNextPageBMP[] = {
    0b10000000,
    0b10000000,
    0b10000000,
    0b11000000,
    0b11000000,
    0b11000000,
    0b11100000,
    0b11100000,
    0b11100000,
    0b11000000,
    0b11000000,
    0b11000000,
    0b10000000,
    0b10000000,
    0b10000000,
};
static const BitmapSpec gNextPageBitmapSpec = BitmapSpec::Construct(gNextPageBMP, 1, 3);

static const uint8_t PROGMEM gPrevPageBMP[] = {
    0b00100000,
    0b00100000,
    0b00100000,
    0b01100000,
    0b01100000,
    0b01100000,
    0b11100000,
    0b11100000,
    0b11100000,
    0b01100000,
    0b01100000,
    0b01100000,
    0b00100000,
    0b00100000,
    0b00100000,
};
static const BitmapSpec gPrevPageBitmapSpec = BitmapSpec::Construct(gPrevPageBMP, 1, 3);



static const uint8_t PROGMEM gMuteOnBMP[] = {
    B_____x__, B________, //
    B____x___, Bx___x___, //
    B__xx_x__, B_x_x____, //
    B__x_x___, B__x_____, //
    B__xx_x__, B_x_x____, //
    B____x___, Bx___x___, //
    B_____x__, B________, //
};

static const BitmapSpec gMuteOnBitmapSpec = BitmapSpec::Construct(gMuteOnBMP, 2, 16);



static const uint8_t PROGMEM gMuteOffBMP[] = {
    B_____x__, B___x____, //
    B____xx__, B_x__x___, //
    B__xxxx_x, B__x_x___, //
    B__xxxx__, Bx_x_x___, //
    B__xxxx_x, B__x_x___, //
    B____xx__, B_x__x___, //
    B_____x__, B___x____, //
};

static const BitmapSpec gMuteOffBitmapSpec = BitmapSpec::Construct(gMuteOffBMP, 2, 16);





} // namespace clarinoid
