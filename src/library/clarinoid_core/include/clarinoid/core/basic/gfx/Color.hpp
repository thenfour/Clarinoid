#pragma once

namespace clarinoid {
struct ColorF
{
  float r;
  float g;
  float b;
};

// effectively Q8 format. todo: used fixed<>
struct ColorByte
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  // luminance
  // hsl -> vec3 / etc.
  // rgb -> vec3 / etc.

  // hsla -> vec4 / etc.

  // WithR() / WithAlpha()
  // WithSaturation()

  // etc...
};

} // namespace clarinoid
