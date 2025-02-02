#pragma once

namespace clarinoid {

struct BitmapSpec
{
  const uint8_t* pBmp = nullptr;
  int bitmapDataSizeBytes = 0;
  uint8_t widthBytes = 0;
  uint8_t widthPixels = 0;
  uint8_t heightPixels = 0;

  template<size_t BitmapDataSizeBytes>
  static BitmapSpec Construct(const uint8_t (&bmpBytes)[BitmapDataSizeBytes], uint8_t widthBytes, uint8_t widthPixels)
  {
    BitmapSpec ret;
    ret.pBmp = bmpBytes;
    ret.widthBytes = widthBytes;
    ret.widthPixels = widthPixels;
    ret.bitmapDataSizeBytes = BitmapDataSizeBytes;
    ret.heightPixels = BitmapDataSizeBytes / ret.widthBytes;
    return ret;
  }
};

} // namespace clarinoid
