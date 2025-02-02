
#pragma once

#include <Adafruit_GFX.h>
#include "DisplayBase.hpp"

namespace clarinoid {

// FROM Adafruit_GFX.cpp:

// Many (but maybe not all) non-AVR board installs define macros
// for compatibility with existing PROGMEM-reading AVR code.
// Do our own checks and defines here for good measure...

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char*)(addr))
#endif
#ifndef pgm_read_word
#define pgm_read_word(addr) (*(const unsigned short*)(addr))
#endif
#ifndef pgm_read_dword
#define pgm_read_dword(addr) (*(const unsigned long*)(addr))
#endif

// Pointers are a peculiar case...typically 16-bit on AVR boards,
// 32 bits elsewhere.  Try to accommodate both...

#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
#define pgm_read_pointer(addr) ((void*)pgm_read_dword(addr))
#else
#define pgm_read_pointer(addr) ((void*)pgm_read_word(addr))
#endif

inline GFXglyph*
pgm_read_glyph_ptr(const GFXfont* gfxFont, uint8_t c)
{
#ifdef __AVR__
  return &(((GFXglyph*)pgm_read_pointer(&gfxFont->glyph))[c]);
#else
  // expression in __AVR__ section may generate "dereferencing type-punned
  // pointer will break strict-aliasing rules" warning In fact, on other
  // platforms (such as STM32) there is no need to do this pointer magic as
  // program memory may be read in a usual way So expression may be simplified
  return gfxFont->glyph + c;
#endif //__AVR__
}

inline uint8_t*
pgm_read_bitmap_ptr(const GFXfont* gfxFont)
{
#ifdef __AVR__
  return (uint8_t*)pgm_read_pointer(&gfxFont->bitmap);
#else
  // expression in __AVR__ section generates "dereferencing type-punned pointer
  // will break strict-aliasing rules" warning In fact, on other platforms (such
  // as STM32) there is no need to do this pointer magic as program memory may
  // be read in a usual way So expression may be simplified
  return gfxFont->bitmap;
#endif //__AVR__
}

// i need to subclass in order to support some things:
// text left margin (so println() new line doesn't set x=0)
// clipping bounds
// disabled text
// dithering support

// and converting the clumsy Adafruit_GFX to a clarinoid friendly IBasicDisplay

// PREVIOUSLY CCAdafruitSSD1306
struct SSD1306LowLevel : Adafruit_SSD1306
{
  SSD1306LowLevel(uint8_t w,
                  uint8_t h,
                  SPIClass* spi,
                  int8_t dc_pin,
                  int8_t rst_pin,
                  int8_t cs_pin,
                  uint32_t bitrate = 8000000UL)

    : // 128, 64, &SPI, 9/*DC*/, 8/*RST*/, 10/*CS*/, 44 * 1000000UL)
    Adafruit_SSD1306(w, h, spi, dc_pin, rst_pin, cs_pin, bitrate)
    , mClipRect(0, 0, w, h)
    , mScreenRect(0, 0, w, h)
  {
  }

  // #define OLED_MOSI   9
  // #define OLED_CLK   10
  // #define OLED_DC    11
  // #define OLED_CS    12
  // #define OLED_RESET 13
  SSD1306LowLevel(uint8_t w, uint8_t h, int8_t mosi_pin, int8_t sclk_pin, int8_t dc_pin, int8_t rst_pin, int8_t cs_pin)
    : Adafruit_SSD1306(w, h, mosi_pin, sclk_pin, dc_pin, rst_pin, cs_pin)
    , mClipRect(0, 0, w, h)
    , mScreenRect(0, 0, w, h)
  {
  }

  bool mSolidText = true;
  int mTextLeftMargin = 0;
  IDitherMatrix& mDitherMatrix = gBayer8x8Matrix;

  RectI mClipRect;
  RectI mScreenRect;

  // virtual void IClarinoidCrashReportOutput_Init() override { mDisplay.begin(SSD1306_SWITCHCAPVCC); }

  // virtual void IClarinoidCrashReportOutput_Blink() override
  // {
  //   fillScreen(SSD1306_WHITE);
  //   display();
  //   delay(150);
  //   fillScreen(SSD1306_BLACK);
  //   display();
  //   delay(150);
  // }
  // virtual void IClarinoidCrashReportOutput_Print(const char* s) override
  // {
  //   setTextColor(SSD1306_WHITE);
  //   setCursor(0, 0);
  //   mDisplay.clearDisplay();
  //   print(s);
  //   display();
  // }

  // bool mSolidText = true;
  // int mTextLeftMargin = 0;
  // RectI16 mClipRect;
  // IDitherMatrix& mDitherMatrix = gBayer8x8Matrix;

  // int mFrameCount = 0; // set by caller; display() is not a virtual fn

  //   virtual void SetClipRect(int left, int top, int right, int bottom) override
  //   {
  //     mClipLeft = left;
  //     mClipRight = right;
  //     mClipTop = top;
  //     mClipBottom = bottom;
  //   }

  //   int mClipLeft = 0;
  //   int mClipRight = 0;
  //   int mClipTop = 0;
  //   int mClipBottom = 0;

  // for checker-style bool checking
  static bool PixelParity(int16_t x, int16_t y) { return (x & 1) != (y & 1); }

  bool IsWithinClipRect(const PointI& pt) const { return mClipRect.Contains(pt); }

  //   bool IsWithinClipRect(int16_t x, int16_t y) const
  //   {
  //     if (x < mClipLeft)
  //       return false;
  //     if (x >= mClipRight)
  //       return false;
  //     if (y < mClipTop)
  //       return false;
  //     if (y >= mClipBottom)
  //       return false;
  //     return true;
  //   }

  // impl of IMonochromeDisplay

  // basics & utilities
  // virtual SizeI ScreenSize() const { return { mScreenRect }; }
  // virtual void Dim(bool d) { mDisplay.dim(d); }

  // clipping
  void SetClipRect(const RectI& rc) { mClipRect = rc; }
  RectI GetClipRect() const { return mClipRect; }
  void ResetClipRect() { mClipRect = mScreenRect; }
  // void ClipToMargin(int m) { mClipRect = mScreenRect.Inset(m); }

  // text
  virtual uint16_t GetLineHeight() const
  {
    if (gfxFont) {
      return textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
    }
    return textsize_y * 8;
  }
  virtual void SetTextSolid(bool b) { mSolidText = b; }
  virtual bool GetTextSolid() { return mSolidText; }
  // virtual void SetTextColor(uint16_t c) { mDisplay.setTextColor(c); }
  virtual int GetTextLeftMargin() { return mTextLeftMargin; }
  virtual void SetTextLeftMargin(int m) { mTextLeftMargin = m; }

  SizeI GetTextSize() const { return { textsize_x, textsize_y }; }

  // virtual void SetTextWrap(bool w) { mDisplay.setTextWrap(w); }
  // virtual PointI GetCursor() const { return mDisplay.getCursor(); }
  // virtual void SetCursor(const PointI& pt) { mDisplay.setCursor(pt.x, pt.y); }
  // virtual void Print(const String& s) { mDisplay.print(s); }
  // virtual void PrintLine(const String& s) { mDisplay.println(s); }
  // virtual void PrintInvertedText(const String& str, bool isInverted = true) override
  // {
  //   if (isInverted) {
  //     int16_t x, y;
  //     uint16_t w, h;
  //     mDisplay.getTextBounds(str, mDisplay.getCursorX(), mDisplay.getCursorY(), &x, &y, &w, &h);
  //     mDisplay.fillRect(x, y, w, h, SSD1306_WHITE);
  //     mDisplay.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  //   } else {
  //     mDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
  //   }

  //   mDisplay.print(str);
  // }

  // virtual void PrintInvertedLine(const String& str, bool isInverted = true) override
  // {
  //   PrintInvertedText(str, isInverted);
  //   mDisplay.println();
  // }

  // // axis-aligned lines
  // virtual void DrawDottedHLine(int16_t left, int16_t width, int16_t y, uint16_t color) override
  // {
  //   const int skip = 2;
  //   for (int16_t x = left; x < left + width; x += skip) {
  //     drawPixel(x, y, color);
  //   }
  // }

  // virtual void DrawDottedRect(int16_t left, int16_t top, int16_t width, int16_t height, uint16_t color) override
  // {
  //   for (int16_t y = top; y < top + height; ++y) {
  //     for (int16_t x = left; x < left + width; x += 2) {
  //       if (PixelParity(x, y)) {
  //         continue;
  //       }
  //       drawPixel(x, y, color);
  //     }
  //   }
  // }

  // virtual void DrawHLineDithered(const PointI& pt, int length, int brightnessQp8) override
  // {
  //   if (length <= 0)
  //     return;
  //   for (int i = 0; i < length; i++) {
  //     int currentX = pt.x + i;
  //     PointI currentPt{ currentX, pt.y };
  //     SetPixelShaded(currentPt, brightnessQp8);
  //   }
  // }

  // virtual void DrawVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override
  // {
  //   this->drawFastVLine(x, y, h, color);
  // }

  // virtual void DrawHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override
  // {
  //   this->drawFastHLine(x, y, w, color);
  // }

  // // axis-aligned Rects
  // virtual void FillScreen(uint16_t color) override { mDisplay.fillScreen(color); }
  // virtual void FillRect(const RectI& rc, uint16_t color) override
  // {
  //   mDisplay.fillRect(rc.x, rc.y, rc.width, rc.height, color);
  // }
  // virtual void DrawMarchingAntsFilledRect(int AntSize,
  //                                         int AntMask,
  //                                         int ySign,
  //                                         int xSign,
  //                                         int xstart,
  //                                         int ystart,
  //                                         int w,
  //                                         int h,
  //                                         int variation) override
  // {
  //   for (int y = ystart; y < ystart + h; ++y) {
  //     for (int x = xstart; x < xstart + w; ++x) {
  //       int p = abs(xSign * x + ySign * y - variation) % AntSize; // p is the position in the pattern.
  //       bool parity = !!(AntMask & (1 << p));
  //       drawPixel(x, y, parity ? SSD1306_WHITE : SSD1306_BLACK);
  //     }
  //   }
  // }

  // virtual void DrawMarchingAntsRectOutline(int LineWidth,
  //                                          int AntSize,
  //                                          int AntMask,
  //                                          int x,
  //                                          int y,
  //                                          int w,
  //                                          int h,
  //                                          int variation,
  //                                          AntStyle style,
  //                                          Edges::Flags edges) override
  // {
  //   int n1 = style == AntStyle::Chasing ? -1 : 1;
  //   // draw rect INSIDE the given coords
  //   if (edges & Edges::Top)
  //     DrawMarchingAntsFilledRect(AntSize, AntMask, 1, 1, x, y, w, LineWidth, variation); // top rect
  //   if (edges & Edges::Bottom)
  //     DrawMarchingAntsFilledRect(AntSize, AntMask, 1, n1, x, y + h - LineWidth, w, LineWidth, variation); // bottom
  //     rect
  //   if (edges & Edges::Left)
  //     DrawMarchingAntsFilledRect(
  //       AntSize, AntMask, n1, n1, x, y + LineWidth, LineWidth, h - LineWidth - LineWidth, variation); // left
  //   if (edges & Edges::Right)
  //     DrawMarchingAntsFilledRect(AntSize,
  //                                AntMask,
  //                                1,
  //                                1,
  //                                x + w - LineWidth,
  //                                y + LineWidth,
  //                                LineWidth,
  //                                h - LineWidth - LineWidth,
  //                                variation); // right
  // }

  // virtual void FillRectWithBrightness(const RectI& rc, int brightnessQp8) override
  // {
  //   brightnessQp8 = ClampInclusive(brightnessQp8, 0, 255);
  //   for (int row = 0; row < rc.height; row++) {
  //     int currentY = rc.y + row;
  //     for (int col = 0; col < rc.width; col++) {
  //       int currentX = rc.x + col;
  //       PointI currentPt{ currentX, currentY };
  //       SetPixelShaded(currentPt, brightnessQp8);
  //     }
  //   }
  // }

  // virtual void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color) override
  // {
  //   mDisplay.fillRoundRect(x0, y0, w, h, radius, color);
  // }

  // // lines (not axis-aligned)
  // virtual void DrawLineWithBrightness(const PointI& pt0, const PointI& pt1, int brightnessQp8) override
  // {
  //   brightnessQp8 = ClampInclusive(brightnessQp8, 0, 255);

  //   int dx = std::abs(pt1.x - pt0.x);
  //   int dy = std::abs(pt1.y - pt0.y);

  //   int sx = (pt0.x < pt1.x) ? 1 : -1;
  //   int sy = (pt0.y < pt1.y) ? 1 : -1;

  //   int err = dx - dy;

  //   int x = pt0.x;
  //   int y = pt0.y;

  //   // is this function freezing? alternative impl below...
  //   // while (true)
  //   // {
  //   //     SetPixelShaded(PointI { x, y }, brightnessQp8);

  //   //     if (x == pt1.x && y == pt1.y) break;

  //   //     int e2 = 2 * err;
  //   //     if (e2 > -dy) { err -= dy; x += sx; }
  //   //     if (e2 < dx) { err += dx; y += sy; }
  //   // }

  //   while (true) {
  //     SetPixelShaded(PointI{ x, y }, brightnessQp8);

  //     if (x == pt1.x && y == pt1.y)
  //       break;

  //     int e2 = 2 * err;
  //     if (e2 > -dy) {
  //       err -= dy;
  //       x += sx;
  //     } else if (e2 < dx) { // Use `else if` to ensure only one step occurs
  //       err += dx;
  //       y += sy;
  //     }
  //   }
  // }

  // virtual void DrawLine(const PointI& pt0, const PointI& pt1) override
  // {
  //   return mDisplay.drawLine(pt0.x, pt0.y, pt1.x, pt1.y, SSD1306_WHITE);
  // }

  // // Specialized function:
  // //   draws the infinite line through (x0,y0)-(x1,y1),
  // //   clipped to [clipLeft..clipRight] x [clipTop..clipBottom].
  // virtual void DrawInfiniteLineClipped(const PointI& pt0,
  //                                      const PointI& pt1,
  //                                      const RectI& clipRect,
  //                                      int brightness) override
  // {
  //   // 1) Handle trivial cases: vertical / horizontal
  //   if (pt0.x == pt1.x) {
  //     // Vertical line x=x0
  //     if (pt0.x < clipRect.left() || pt0.x > clipRect.right())
  //       return; // outside
  //     int yStart = clipRect.top();
  //     int yEnd = clipRect.bottom();
  //     // Just draw from (x0, yStart) to (x0, yEnd)
  //     DrawLineWithBrightness(PointI{ pt0.x, yStart }, PointI{ pt0.x, yEnd }, brightness);
  //     return;
  //   }

  //   if (pt0.y == pt1.y) {
  //     // Horizontal line y=y0
  //     if (pt0.y < clipRect.top() || pt0.y > clipRect.bottom())
  //       return; // outside
  //     int xStart = clipRect.left();
  //     int xEnd = clipRect.right();
  //     // Draw from (xStart, y0) to (xEnd, y0)
  //     DrawLineWithBrightness(PointI{ xStart, pt0.y }, PointI{ xEnd, pt0.y }, brightness);
  //     return;
  //   }

  //   // 2) General case
  //   float slope = (float)(pt1.y - pt0.y) / (float)(pt1.x - pt0.x);

  //   // We'll track up to 4 intersection candidates:
  //   PointI candidates[4];
  //   size_t candidateCount = 0;

  //   // Intersection at x=clipLeft => y = y0 + slope*(clipLeft - x0)
  //   float yLeft = pt0.y + slope * (clipRect.left() - pt0.x);
  //   if (yLeft >= clipRect.top() && yLeft <= clipRect.bottom()) {
  //     candidates[candidateCount++] = PointI{ clipRect.left(), (int)yLeft };
  //   }

  //   float yRight = pt0.y + slope * (clipRect.right() - pt0.x);
  //   if (yRight >= clipRect.top() && yRight <= clipRect.bottom()) {
  //     candidates[candidateCount++] = PointI{ clipRect.right(), (int)yRight };
  //   }

  //   // Intersection at y=clipTop => x = x0 + (clipTop - y0)/slope
  //   float xTop = pt0.x + (clipRect.top() - pt0.y) / slope;
  //   if (xTop >= clipRect.left() && xTop <= clipRect.right()) {
  //     candidates[candidateCount++] = PointI{ (int)xTop, clipRect.top() };
  //   }

  //   // Intersection at y=clipBottom => x = x0 + (clipBottom - y0)/slope
  //   float xBottom = pt0.x + (clipRect.bottom() - pt0.y) / slope;
  //   if (xBottom >= clipRect.left() && xBottom <= clipRect.right()) {
  //     candidates[candidateCount++] = PointI{ (int)xBottom, clipRect.bottom() };
  //   }

  //   // Remove duplicates or near-duplicates if they happen (optional).
  //   // If there's no valid intersection => no draw
  //   if (candidateCount < 2)
  //     return;

  //   // We only need two extremes. Possibly we have more if the line hits exactly a corner, etc.
  //   // Let's pick the two that are farthest from each other:
  //   //  (One way: measure the bounding box in that set.)
  //   float minX = 1e6;
  //   float minY = 1e6;
  //   float maxX = -1e6;
  //   float maxY = -1e6;

  //   for (size_t i = 0; i < candidateCount; i++) {
  //     if (candidates[i].x < minX)
  //       minX = candidates[i].x;
  //     if (candidates[i].x > maxX)
  //       maxX = candidates[i].x;
  //     if (candidates[i].y < minY)
  //       minY = candidates[i].y;
  //     if (candidates[i].y > maxY)
  //       maxY = candidates[i].y;
  //   }

  //   // Because it's a straight line, the "two extremes" in your intersection list
  //   // will either share the same min or max in both X and Y, or you can pick any
  //   // pair that yields the maximum distance. For simplicity, let's just pick
  //   // the minX-based intersection and the maxX-based intersection if the slope
  //   // is not near-infinite. That covers the typical case.

  //   // But to be robust, let's do a small function that picks the two
  //   // intersection points in 'candidates' that are farthest apart:
  //   PointI pA, pB;
  //   FindFarthestPair(candidates, candidateCount, pA, pB);
  //   DrawLineWithBrightness(pA, pB, brightness);
  // }

  // // finds the two points in 'points' that are farthest apart. supports only up to 4 points due to O(n^2) complexity.
  // // assumes at least 1 point.
  // void FindFarthestPair(const PointI* points, size_t pointCount, PointI& bestA, PointI& bestB)
  // {
  //   bestA = points[0];
  //   bestB = points[0];
  //   float maxDistSq = 0;

  //   // Simple O(n^2) check (fine for up to 4 points):
  //   for (size_t i = 0; i < pointCount; i++) {
  //     for (size_t j = i + 1; j < pointCount; j++) {
  //       float dx = points[i].x - points[j].x;
  //       float dy = points[i].y - points[j].y;
  //       float distSq = dx * dx + dy * dy;
  //       if (distSq > maxDistSq) {
  //         maxDistSq = distSq;
  //         bestA = points[i];
  //         bestB = points[j];
  //       }
  //     }
  //   }
  // }

  // // Bitmaps
  // virtual void DrawBitmap(PointI pos, const BitmapSpec& bmp) override
  // {
  //   mDisplay.drawBitmap(pos.x, pos.y, bmp.pBmp, bmp.widthPixels, bmp.heightPixels, SSD1306_WHITE);
  // }

  // // circles
  // virtual void FillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) override
  // {
  //   mDisplay.fillCircle(x0, y0, r, color);
  // }

  // virtual void FillCircleWithBrightness(const PointI& c, int r, int brightnessQp8) override
  // {
  //   if (r <= 0)
  //     return; // no valid radius

  //   // Midpoint circle algorithm setup
  //   int x = 0;
  //   int y = r;

  //   // f is our "decision" variable, starts at 1 - r
  //   int f = 1 - r;
  //   // ddF_x, ddF_y track derivative changes
  //   int ddF_x = 1;
  //   int ddF_y = -2 * r;

  //   // We'll fill from the center's horizontal line out
  //   // so first fill the horizontal line across the circle's diameter
  //   // y=0 => from (x0-r) to (x0+r)
  //   DrawHLineDithered({ c.x - r, c.y }, (2 * r + 1), brightnessQp8);

  //   // Also fill the symmetrical horizontal lines above & below
  //   // for each step of x,y as we move around the circle edges
  //   while (x < y) {
  //     // If f >= 0, move y inward
  //     if (f >= 0) {
  //       y--;
  //       ddF_y += 2;
  //       f += ddF_y;
  //     }
  //     // Always move x outward
  //     x++;
  //     ddF_x += 2;
  //     f += ddF_x;

  //     // Now we have a circle boundary at (x,y). We fill horizontal lines:
  //     // "Top"  side at y0 + y
  //     // "Bottom" side at y0 - y
  //     // each goes from (x0 - x) to (x0 + x)

  //     DrawHLineDithered({ c.x - x, c.y + y }, (2 * x + 1), brightnessQp8);
  //     if (y != 0) // if y=0, top & bottom would be same line
  //     {
  //       DrawHLineDithered({ c.x - x, c.y - y }, (2 * x + 1), brightnessQp8);
  //     }

  //     // For x != y, fill those "side" lines near (y,x) due to circle symmetry:
  //     //   left side at x0 - y .. x0 + y, top y= y0 + x
  //     //   left side at x0 - y .. x0 + y, bottom y= y0 - x
  //     if (x != y) {
  //       DrawHLineDithered({ c.x - y, c.y + x }, (2 * y + 1), brightnessQp8);
  //       if (x != 0) // if x=0, same line repeated
  //       {
  //         DrawHLineDithered({ c.x - y, c.y - x }, (2 * y + 1), brightnessQp8);
  //       }
  //     }
  //   }
  // }

  // // "pie"
  // virtual void FillPie(const PointF& origin,
  //                      float radius,
  //                      float angleStart,
  //                      float angleSweep,
  //                      bool filled = true) override
  // {
  //   float a0, a1;
  //   if (angleSweep >= 0) {
  //     a0 = angleStart;
  //     a1 = a0 + angleSweep;
  //   } else {
  //     a0 = angleStart + angleSweep;
  //     a1 = angleStart;
  //   }
  //   ::clarinoid::PieData pd;
  //   if (filled) {
  //     pd = ::clarinoid::fillPie(origin.x, origin.y, radius, a0, a1, [&](int x, int y, bool line) {
  //       if (!line && ((x + y) & 1))
  //         return;
  //       mDisplay.drawPixel(x, y, SSD1306_WHITE);
  //     });
  //   } else {
  //     pd = ::clarinoid::fillPie(origin.x, origin.y, radius, a0, a1, [&](int x, int y, bool line) {
  //       if (line || (((x * 2 + y) % 4) == 1)) {
  //         mDisplay.drawPixel(x, y, SSD1306_WHITE);
  //       }
  //     });
  //   }
  //   if (angleSweep >= 0) {
  //     drawLine(origin.x, origin.y, origin.x + pd.p0.x, origin.y + pd.p0.y, [&](int x, int y, bool) {
  //       mDisplay.drawPixel(x, y, SSD1306_WHITE);
  //     });
  //   } else {
  //     drawLine(origin.x, origin.y, origin.x + pd.p1.x, origin.y + pd.p1.y, [&](int x, int y, bool) {
  //       mDisplay.drawPixel(x, y, SSD1306_WHITE);
  //     });
  //   }
  // }

  // Pixel
  // virtual void SetPixel(const PointI& pt, uint16_t color){ writePixel(pt.x, pt.y, color); }

  // uses clipping, dithering
  virtual void writeDitheredPixel(const PointI& pt, int coverageQp8)
  {
    if (!IsWithinClipRect(pt))
      return;
    coverageQp8 = ClampInclusive(coverageQp8, 0, 255);
    drawPixel(pt.x, pt.y, mDitherMatrix.getDitheredColor(coverageQp8, pt));
  }

  // overriding Adafruit_GFX::writePixel. base forwards to drawPixel().
  void writePixel(int16_t x, int16_t y, uint16_t color)
  {
    if (!IsWithinClipRect({ x, y }))
      return;
    if (!mSolidText) {
      if (!PixelParity(x, y)) {
        // there are probably much more legible ways of graying text. this sorta destroys background color info;
        // we should probably instead set a bg/fg color before the text write op
        return;
      }
    }
    drawPixel(x, y, color);
  }

  //   virtual void DrawDottedRect(int16_t left, int16_t top, int16_t width, int16_t height, uint16_t color) override
  //   {
  //     for (int16_t y = top; y < top + height; ++y) {
  //       for (int16_t x = left; x < left + width; x += 2) {
  //         if (PixelParity(x, y)) {
  //           continue;
  //         }
  //         drawPixel(x, y, color);
  //       }
  //     }
  //   }

  //   virtual void DrawDottedHLine(int16_t left, int16_t width, int16_t y, uint16_t color) override
  //   {
  //     const int skip = 2;
  //     for (int16_t x = left; x < left + width; x += skip) {
  //       drawPixel(x, y, color);
  //     }
  //   }

  //   void DrawDottedHLineWithGlobalParity(int xstart, int w, int y, bool variation)
  //   {
  //     for (int16_t x = xstart; x < xstart + w; ++x) {
  //       if (variation ? ((x & 1) == (y & 1)) : ((x & 1) != (y & 1)))
  //         drawPixel(x, y, SSD1306_WHITE);
  //     }
  //   }

  //   void DrawDottedVLineWithGlobalParity(int x, int ystart, int h, bool variation)
  //   {
  //     for (int16_t y = ystart; y < ystart + h; ++y) {
  //       if (variation ? ((x & 1) == (y & 1)) : ((x & 1) != (y & 1)))
  //         drawPixel(x, y, SSD1306_WHITE);
  //     }
  //   }

  //   void DrawDottedRectOutlineWithGlobalParity(int x, int y, int w, int h, bool variation)
  //   {
  //     DrawDottedHLineWithGlobalParity(x, w, y, variation);
  //     DrawDottedHLineWithGlobalParity(x, w, y + h - 1, variation);
  //     DrawDottedVLineWithGlobalParity(x, y + 1, h - 2, variation);
  //     DrawDottedVLineWithGlobalParity(x + w - 1, y + 1, h - 2, variation);
  //   }

  // // template <int AntSize, int AntMask, int ySign, int xSign>
  // virtual void DrawMarchingAntsFilledRect(int AntSize,
  //                                         int AntMask,
  //                                         int ySign,
  //                                         int xSign,
  //                                         int xstart,
  //                                         int ystart,
  //                                         int w,
  //                                         int h,
  //                                         int variation) override
  // {
  //   for (int y = ystart; y < ystart + h; ++y) {
  //     for (int x = xstart; x < xstart + w; ++x) {
  //       int p = abs(xSign * x + ySign * y - variation) % AntSize; // p is the position in the pattern.
  //       bool parity = !!(AntMask & (1 << p));
  //       drawPixel(x, y, parity ? SSD1306_WHITE : SSD1306_BLACK);
  //     }
  //   }
  // }

  // // template <int LineWidth, int AntSize, int AntMask>
  // virtual void DrawMarchingAntsRectOutline(int LineWidth,
  //                                          int AntSize,
  //                                          int AntMask,
  //                                          int x,
  //                                          int y,
  //                                          int w,
  //                                          int h,
  //                                          int variation,
  //                                          AntStyle style,
  //                                          Edges::Flags edges) override
  // {
  //   int n1 = style == AntStyle::Chasing ? -1 : 1;
  //   // draw rect INSIDE the given coords
  //   if (edges & Edges::Top)
  //     DrawMarchingAntsFilledRect(AntSize, AntMask, 1, 1, x, y, w, LineWidth, variation); // top rect
  //   if (edges & Edges::Bottom)
  //     DrawMarchingAntsFilledRect(AntSize, AntMask, 1, n1, x, y + h - LineWidth, w, LineWidth, variation); // bottom
  //     rect
  //   if (edges & Edges::Left)
  //     DrawMarchingAntsFilledRect(
  //       AntSize, AntMask, n1, n1, x, y + LineWidth, LineWidth, h - LineWidth - LineWidth, variation); // left
  //   if (edges & Edges::Right)
  //     DrawMarchingAntsFilledRect(AntSize,
  //                                AntMask,
  //                                1,
  //                                1,
  //                                x + w - LineWidth,
  //                                y + LineWidth,
  //                                LineWidth,
  //                                h - LineWidth - LineWidth,
  //                                variation); // right
  // }

  // virtual uint16_t GetLineHeight() const override
  // {
  //   if (gfxFont) {
  //     return textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
  //   }
  //   return textsize_y * 8;
  // }

  /**************************************************************************/
  /*!
      @brief  Print one byte/character of data, used to support print()
      @param  c  The 8-bit ascii character to write
  */
  /**************************************************************************/
  size_t /*Adafruit_GFX::*/ write(uint8_t c)
  {
    if (!gfxFont) { // 'Classic' built-in font

      if (c == '\n') {                                        // Newline?
        cursor_x = mTextLeftMargin;                           // Reset x to left margin
        cursor_y += textsize_y * 8;                           // advance y one line
      } else if (c != '\r') {                                 // Ignore carriage returns
        if (wrap && ((cursor_x + textsize_x * 6) > _width)) { // Off right?
          cursor_x = mTextLeftMargin;                         // reset x to left margin
          cursor_y += textsize_y * 8;                         // advance y one line
        }
        drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x, textsize_y);
        cursor_x += textsize_x * 6; // Advance x one char
      }
    } else { // Custom font

      if (c == '\n') {
        cursor_x = mTextLeftMargin;
        cursor_y += (int16_t)textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
      } else if (c != '\r') {
        uint8_t first = pgm_read_byte(&gfxFont->first);
        if ((c >= first) && (c <= (uint8_t)pgm_read_byte(&gfxFont->last))) {
          GFXglyph* glyph = pgm_read_glyph_ptr(gfxFont, c - first);
          uint8_t w = pgm_read_byte(&glyph->width), h = pgm_read_byte(&glyph->height);
          if ((w > 0) && (h > 0)) {                              // Is there an associated bitmap?
            int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset); // sic
            if (wrap && ((cursor_x + textsize_x * (xo + w)) > _width)) {
              cursor_x = mTextLeftMargin;
              cursor_y += (int16_t)textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
            }
            drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x, textsize_y);
          }
          cursor_x += (uint8_t)pgm_read_byte(&glyph->xAdvance) * (int16_t)textsize_x;
        }
      }
    }
    return 1;
  }
};
} // namespace clarinoid
