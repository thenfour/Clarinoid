// display classes & interfaces are a bit of a mess.
// - Adafruit_SSD1306: the underlying low-level lib
// - SSD1306_LowLevel: subclasses it to support some additional features. should ONLY implement what's necessary,
// because the interface is messy. I should consider just forking Adafruit_SSD1306 but for now this works.
// - IMonochromeDisplay: cleaner interface we should use exclusively in clarinoid SSD1306: wraps
// - SSD1306_LowLevel and implements IMonochromeDisplay; this is the clean display device. IAppDisplay: app-level
// display interface
// - ClarinoidDisplay: wraps SSD1306 and implements IAppDisplay; this is the app-level display.

// still to clean up:
// - various color methods (ants, "solid text" which is actually not only for text, dithered, solid...)
//   - use a fixed class for Qp8 color.
// - unit cleanup (point or x/y)
// - arguably, separate IAppDisplay from IMonochromeDisplay

#pragma once

#include "SSD1306LowLevel.hpp"

namespace clarinoid {

struct SSD1306 : public clarinoid::IClarinoidCrashReportOutput, public IMonochromeDisplay
{
  SSD1306LowLevel mDisplay;
  SSD1306(uint8_t w,
          uint8_t h,
          SPIClass* spi,
          int8_t dc_pin,
          int8_t rst_pin,
          int8_t cs_pin,
          uint32_t bitrate = 8000000UL)

    : // 128, 64, &SPI, 9/*DC*/, 8/*RST*/, 10/*CS*/, 44 * 1000000UL)
    mDisplay(w, h, spi, dc_pin, rst_pin, cs_pin, bitrate)
  {
  }

  // #define OLED_MOSI   9
  // #define OLED_CLK   10
  // #define OLED_DC    11
  // #define OLED_CS    12
  // #define OLED_RESET 13
  SSD1306(uint8_t w, uint8_t h, int8_t mosi_pin, int8_t sclk_pin, int8_t dc_pin, int8_t rst_pin, int8_t cs_pin)
    : mDisplay(w, h, mosi_pin, sclk_pin, dc_pin, rst_pin, cs_pin)
  {
  }

  bool begin(uint8_t switchvcc = SSD1306_SWITCHCAPVCC, uint8_t i2caddr = 0, bool reset = true, bool periphBegin = true)
  {
    return mDisplay.begin(switchvcc, i2caddr, reset, periphBegin);
  }

  virtual void IClarinoidCrashReportOutput_Init() override { mDisplay.begin(SSD1306_SWITCHCAPVCC); }

  virtual void IClarinoidCrashReportOutput_Blink() override
  {
    mDisplay.fillScreen(SSD1306_WHITE);
    mDisplay.display();
    delay(150);
    mDisplay.fillScreen(SSD1306_BLACK);
    mDisplay.display();
    delay(150);
  }

  virtual void IClarinoidCrashReportOutput_Print(const char* s) override
  {
    mDisplay.setTextColor(SSD1306_WHITE);
    mDisplay.setCursor(0, 0);
    mDisplay.clearDisplay();
    mDisplay.print(s);
    mDisplay.display();
  }

  // impl of IMonochromeDisplay

  // basics & utilities
  virtual SizeI ScreenSize() const override { return mDisplay.mScreenRect.GetSize(); }
  virtual RectI ScreenRect() const override { return mDisplay.mScreenRect; }
  virtual void Dim(bool d) override { mDisplay.dim(d); }
  virtual void PresentToDevice() override { mDisplay.display(); }

  // clipping
  virtual void SetClipRect(const RectI& rc) override { mDisplay.SetClipRect(rc); }
  virtual RectI GetClipRect() const override { return mDisplay.GetClipRect(); }
  virtual void ResetClipRect() override { mDisplay.ResetClipRect(); }
  virtual void SetClipRectToMargin(int m) override { mDisplay.SetClipRect(mDisplay.mScreenRect.Inset(m)); }

  // text
  virtual SizeI GetTextSize() const override { return mDisplay.GetTextSize(); }
  virtual void SetTextSize(const SizeI& s) override { mDisplay.setTextSize(s.width, s.height); }

  virtual uint16_t GetLineHeight() const override { return mDisplay.GetLineHeight(); }
  virtual void SetTextSolid(bool b) override { mDisplay.SetTextSolid(b); }
  virtual bool GetTextSolid() override { return mDisplay.GetTextSolid(); }
  virtual void SetTextColor(uint16_t c) override { mDisplay.setTextColor(c); }
  virtual void SetTextColor(uint16_t c, uint16_t bg) override { mDisplay.setTextColor(c, bg); }
  virtual int GetTextLeftMargin() override { return mDisplay.GetTextLeftMargin(); }
  virtual void SetTextLeftMargin(int m) override { mDisplay.SetTextLeftMargin(m); }
  virtual void SetTextWrap(bool w) override { mDisplay.setTextWrap(w); }
  virtual RectI GetTextBounds(const String& str) override
  {
    // return mDisplay.GetTextBounds(str);
    // RectI GetTextBounds(const String& str)
    //{
    int16_t x, y;
    uint16_t h, w;
    mDisplay.getTextBounds(str, 0, 0, &x, &y, &w, &h);
    return { x, y, w, h };
    //}
  }
  virtual PointI GetCursor() const override { return { mDisplay.getCursorX(), mDisplay.getCursorY() }; }
  virtual void SetCursor(const PointI& pt) override { mDisplay.setCursor(pt.x, pt.y); }
  virtual void Print(const String& s) override { mDisplay.print(s); }
  virtual void PrintLine(const String& s) override { mDisplay.println(s); }
  virtual void PrintInvertedText(const String& str, bool isInverted = true) override
  {
    if (isInverted) {
      int16_t x, y;
      uint16_t w, h;
      mDisplay.getTextBounds(str, mDisplay.getCursorX(), mDisplay.getCursorY(), &x, &y, &w, &h);
      mDisplay.fillRect(x, y, w, h, SSD1306_WHITE);
      mDisplay.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    } else {
      mDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
    }

    mDisplay.print(str);
  }

  virtual void PrintInvertedLine(const String& str, bool isInverted = true) override
  {
    PrintInvertedText(str, isInverted);
    mDisplay.println();
  }

  // axis-aligned lines
  virtual void DrawDottedHLine(int16_t left, int16_t width, int16_t y, uint16_t color) override
  {
    const int skip = 2;
    for (int16_t x = left; x < left + width; x += skip) {
      mDisplay.drawPixel(x, y, color);
    }
  }

  virtual void DrawDottedRect(int16_t left, int16_t top, int16_t width, int16_t height, uint16_t color) override
  {
    for (int16_t y = top; y < top + height; ++y) {
      for (int16_t x = left; x < left + width; x += 2) {
        if (mDisplay.PixelParity(x, y)) {
          continue;
        }
        mDisplay.drawPixel(x, y, color);
      }
    }
  }

  virtual void DrawHLineDithered(const PointI& pt, int length, int brightnessQp8) override
  {
    if (length <= 0)
      return;
    for (int i = 0; i < length; i++) {
      int currentX = pt.x + i;
      PointI currentPt{ currentX, pt.y };
      SetPixelShaded(currentPt, brightnessQp8);
    }
  }

  virtual void DrawVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override
  {
    mDisplay.drawFastVLine(x, y, h, color);
  }

  virtual void DrawHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override
  {
    mDisplay.drawFastHLine(x, y, w, color);
  }

  // axis-aligned Rects
  virtual void FillScreen(uint16_t color) override { mDisplay.fillScreen(color); }
  virtual void FillRect(const RectI& rc, uint16_t color) override
  {
    mDisplay.fillRect(rc.Left(), rc.Top(), rc.Width(), rc.Height(), color);
  }
  virtual void DrawMarchingAntsFilledRect(int AntSize,
                                          int AntMask,
                                          int ySign,
                                          int xSign,
                                          int xstart,
                                          int ystart,
                                          int w,
                                          int h,
                                          int variation) override
  {
    for (int y = ystart; y < ystart + h; ++y) {
      for (int x = xstart; x < xstart + w; ++x) {
        int p = abs(xSign * x + ySign * y - variation) % AntSize; // p is the position in the pattern.
        bool parity = !!(AntMask & (1 << p));
        mDisplay.drawPixel(x, y, parity ? SSD1306_WHITE : SSD1306_BLACK);
      }
    }
  }

  virtual void DrawMarchingAntsRectOutline(int LineWidth,
                                           int AntSize,
                                           int AntMask,
                                           int x,
                                           int y,
                                           int w,
                                           int h,
                                           int variation,
                                           AntStyle style,
                                           Edges::Flags edges) override
  {
    int n1 = style == AntStyle::Chasing ? -1 : 1;
    // draw rect INSIDE the given coords
    if (edges & Edges::Top)
      DrawMarchingAntsFilledRect(AntSize, AntMask, 1, 1, x, y, w, LineWidth, variation); // top rect
    if (edges & Edges::Bottom)
      DrawMarchingAntsFilledRect(AntSize, AntMask, 1, n1, x, y + h - LineWidth, w, LineWidth, variation); // bottom rect
    if (edges & Edges::Left)
      DrawMarchingAntsFilledRect(
        AntSize, AntMask, n1, n1, x, y + LineWidth, LineWidth, h - LineWidth - LineWidth, variation); // left
    if (edges & Edges::Right)
      DrawMarchingAntsFilledRect(AntSize,
                                 AntMask,
                                 1,
                                 1,
                                 x + w - LineWidth,
                                 y + LineWidth,
                                 LineWidth,
                                 h - LineWidth - LineWidth,
                                 variation); // right
  }

  virtual void FillRectWithBrightness(const RectI& rc, int brightnessQp8) override
  {
    brightnessQp8 = ClampInclusive(brightnessQp8, 0, 255);
    for (int row = 0; row < rc.Height(); row++) {
      int currentY = rc.Top() + row;
      for (int col = 0; col < rc.Width(); col++) {
        int currentX = rc.Left() + col;
        PointI currentPt{ currentX, currentY };
        SetPixelShaded(currentPt, brightnessQp8);
      }
    }
  }

  virtual void FillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color) override
  {
    mDisplay.fillRoundRect(x0, y0, w, h, radius, color);
  }

  // lines (not axis-aligned)
  virtual void DrawLineWithBrightness(const PointI& pt0, const PointI& pt1, int brightnessQp8) override
  {
    brightnessQp8 = ClampInclusive(brightnessQp8, 0, 255);

    int dx = std::abs(pt1.x - pt0.x);
    int dy = std::abs(pt1.y - pt0.y);

    int sx = (pt0.x < pt1.x) ? 1 : -1;
    int sy = (pt0.y < pt1.y) ? 1 : -1;

    int err = dx - dy;

    int x = pt0.x;
    int y = pt0.y;

    // is this function freezing? alternative impl below...
    // while (true)
    // {
    //     SetPixelShaded(PointI { x, y }, brightnessQp8);

    //     if (x == pt1.x && y == pt1.y) break;

    //     int e2 = 2 * err;
    //     if (e2 > -dy) { err -= dy; x += sx; }
    //     if (e2 < dx) { err += dx; y += sy; }
    // }

    while (true) {
      SetPixelShaded(PointI{ x, y }, brightnessQp8);

      if (x == pt1.x && y == pt1.y)
        break;

      int e2 = 2 * err;
      if (e2 > -dy) {
        err -= dy;
        x += sx;
      } else if (e2 < dx) { // Use `else if` to ensure only one step occurs
        err += dx;
        y += sy;
      }
    }
  }

  virtual void DrawLine(const PointI& pt0, const PointI& pt1) override
  {
    return mDisplay.drawLine(pt0.x, pt0.y, pt1.x, pt1.y, SSD1306_WHITE);
  }

  // Specialized function:
  //   draws the infinite line through (x0,y0)-(x1,y1),
  //   clipped to [clipLeft..clipRight] x [clipTop..clipBottom].
  virtual void DrawInfiniteLineClipped(const PointI& pt0,
                                       const PointI& pt1,
                                       const RectI& clipRect,
                                       int brightness) override
  {
    // 1) Handle trivial cases: vertical / horizontal
    if (pt0.x == pt1.x) {
      // Vertical line x=x0
      if (pt0.x < clipRect.Left() || pt0.x > clipRect.Right())
        return; // outside
      int yStart = clipRect.Top();
      int yEnd = clipRect.Bottom();
      // Just draw from (x0, yStart) to (x0, yEnd)
      DrawLineWithBrightness(PointI{ pt0.x, yStart }, PointI{ pt0.x, yEnd }, brightness);
      return;
    }

    if (pt0.y == pt1.y) {
      // Horizontal line y=y0
      if (pt0.y < clipRect.Top() || pt0.y > clipRect.Bottom())
        return; // outside
      int xStart = clipRect.Left();
      int xEnd = clipRect.Right();
      // Draw from (xStart, y0) to (xEnd, y0)
      DrawLineWithBrightness(PointI{ xStart, pt0.y }, PointI{ xEnd, pt0.y }, brightness);
      return;
    }

    // 2) General case
    float slope = (float)(pt1.y - pt0.y) / (float)(pt1.x - pt0.x);

    // We'll track up to 4 intersection candidates:
    std::vector<PointI> candidates;
    // PointI candidates[4];
    // size_t candidateCount = 0;

    // Intersection at x=clipLeft => y = y0 + slope*(clipLeft - x0)
    float yLeft = pt0.y + slope * (clipRect.Left() - pt0.x);
    if (yLeft >= clipRect.Top() && yLeft <= clipRect.Bottom()) {
      // candidates[candidateCount++] = PointI{ clipRect.Left(), (int)yLeft };
      candidates.push_back(PointI{ clipRect.Left(), (int)yLeft });
    }

    float yRight = pt0.y + slope * (clipRect.Right() - pt0.x);
    if (yRight >= clipRect.Top() && yRight <= clipRect.Bottom()) {
      // candidates[candidateCount++] = PointI{ clipRect.Right(), (int)yRight };
      candidates.push_back(PointI{ clipRect.Right(), (int)yRight });
    }

    // Intersection at y=clipTop => x = x0 + (clipTop - y0)/slope
    float xTop = pt0.x + (clipRect.Top() - pt0.y) / slope;
    if (xTop >= clipRect.Left() && xTop <= clipRect.Right()) {
      // candidates[candidateCount++] = PointI{ (int)xTop, clipRect.Top() };
      candidates.push_back(PointI{ (int)xTop, clipRect.Top() });
    }

    // Intersection at y=clipBottom => x = x0 + (clipBottom - y0)/slope
    float xBottom = pt0.x + (clipRect.Bottom() - pt0.y) / slope;
    if (xBottom >= clipRect.Left() && xBottom <= clipRect.Right()) {
      // candidates[candidateCount++] = PointI{ (int)xBottom, clipRect.Bottom() };
      candidates.push_back(PointI{ (int)xBottom, clipRect.Bottom() });
    }

    // Remove duplicates or near-duplicates if they happen (optional).
    // If there's no valid intersection => no draw
    // if (candidateCount < 2)
    //      return;
    if (candidates.size() < 2)
      return;

    // We only need two extremes. Possibly we have more if the line hits exactly a corner, etc.
    // Let's pick the two that are farthest from each other:
    //  (One way: measure the bounding box in that set.)
    float minX = 1e6;
    float minY = 1e6;
    float maxX = -1e6;
    float maxY = -1e6;

    for (size_t i = 0; i < candidates.size(); i++) {
      if (candidates[i].x < minX)
        minX = candidates[i].x;
      if (candidates[i].x > maxX)
        maxX = candidates[i].x;
      if (candidates[i].y < minY)
        minY = candidates[i].y;
      if (candidates[i].y > maxY)
        maxY = candidates[i].y;
    }

    // Because it's a straight line, the "two extremes" in your intersection list
    // will either share the same min or max in both X and Y, or you can pick any
    // pair that yields the maximum distance. For simplicity, let's just pick
    // the minX-based intersection and the maxX-based intersection if the slope
    // is not near-infinite. That covers the typical case.

    // But to be robust, let's do a small function that picks the two
    // intersection points in 'candidates' that are farthest apart:
    PointI *pA, *pB;
    FindFarthestPair(candidates.data(), candidates.size(), pA, pB);
    DrawLineWithBrightness(*pA, *pB, brightness);
  }

  // finds the two points in 'points' that are farthest apart. supports only up to 4 points due to O(n^2) complexity.
  // assumes at least 1 point.
  void FindFarthestPair(PointI* points, size_t pointCount, PointI*& bestA, PointI*& bestB)
  {
    bestA = &points[0];
    bestB = &points[0];
    float maxDistSq = 0;

    // Simple O(n^2) check (fine for up to 4 points):
    for (size_t i = 0; i < pointCount; i++) {
      for (size_t j = i + 1; j < pointCount; j++) {
        float dx = points[i].x - points[j].x;
        float dy = points[i].y - points[j].y;
        float distSq = dx * dx + dy * dy;
        if (distSq > maxDistSq) {
          maxDistSq = distSq;
          bestA = &points[i];
          bestB = &points[j];
        }
      }
    }
  }

  // Bitmaps
  virtual void DrawBitmap(PointI pos, const BitmapSpec& bmp) override
  {
    mDisplay.drawBitmap(pos.x, pos.y, bmp.pBmp, bmp.widthPixels, bmp.heightPixels, SSD1306_WHITE);
  }

  // circles
  virtual void FillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) override
  {
    mDisplay.fillCircle(x0, y0, r, color);
  }

  virtual void FillCircleWithBrightness(const PointI& c, int r, int brightnessQp8) override
  {
    if (r <= 0)
      return; // no valid radius

    // Midpoint circle algorithm setup
    int x = 0;
    int y = r;

    // f is our "decision" variable, starts at 1 - r
    int f = 1 - r;
    // ddF_x, ddF_y track derivative changes
    int ddF_x = 1;
    int ddF_y = -2 * r;

    // We'll fill from the center's horizontal line out
    // so first fill the horizontal line across the circle's diameter
    // y=0 => from (x0-r) to (x0+r)
    DrawHLineDithered({ c.x - r, c.y }, (2 * r + 1), brightnessQp8);

    // Also fill the symmetrical horizontal lines above & below
    // for each step of x,y as we move around the circle edges
    while (x < y) {
      // If f >= 0, move y inward
      if (f >= 0) {
        y--;
        ddF_y += 2;
        f += ddF_y;
      }
      // Always move x outward
      x++;
      ddF_x += 2;
      f += ddF_x;

      // Now we have a circle boundary at (x,y). We fill horizontal lines:
      // "Top"  side at y0 + y
      // "Bottom" side at y0 - y
      // each goes from (x0 - x) to (x0 + x)

      DrawHLineDithered({ c.x - x, c.y + y }, (2 * x + 1), brightnessQp8);
      if (y != 0) // if y=0, top & bottom would be same line
      {
        DrawHLineDithered({ c.x - x, c.y - y }, (2 * x + 1), brightnessQp8);
      }

      // For x != y, fill those "side" lines near (y,x) due to circle symmetry:
      //   left side at x0 - y .. x0 + y, top y= y0 + x
      //   left side at x0 - y .. x0 + y, bottom y= y0 - x
      if (x != y) {
        DrawHLineDithered({ c.x - y, c.y + x }, (2 * y + 1), brightnessQp8);
        if (x != 0) // if x=0, same line repeated
        {
          DrawHLineDithered({ c.x - y, c.y - x }, (2 * y + 1), brightnessQp8);
        }
      }
    }
  }

  // "pie"
  virtual void FillPie(const PointF& origin,
                       float radius,
                       float angleStart,
                       float angleSweep,
                       bool filled = true) override
  {
    float a0, a1;
    if (angleSweep >= 0) {
      a0 = angleStart;
      a1 = a0 + angleSweep;
    } else {
      a0 = angleStart + angleSweep;
      a1 = angleStart;
    }

    cc::function<void(int, int, void*)>::ptr_t drawPixelProc = filled
                                                                 ? //
                                                                 ([](int x, int y, void* cap) {
                                                                   if ((x + y) & 1)
                                                                     return;
                                                                   auto pThis = (SSD1306*)cap;
                                                                   pThis->mDisplay.drawPixel(x, y, SSD1306_WHITE);
                                                                 })
                                                                 : ([](int x, int y, void* cap) {
                                                                     if (((x * 2 + y) % 4) == 1) {
                                                                       auto pThis = (SSD1306*)cap;
                                                                       pThis->mDisplay.drawPixel(x, y, SSD1306_WHITE);
                                                                     }
                                                                   });

    PieData pd = fillPie(origin.x, origin.y, radius, a0, a1, drawPixelProc, this);

    // if (filled) {
    //   pd = ::clarinoid::fillPie(origin.x, origin.y, radius, a0, a1, );
    // } else {
    //   pd = ::clarinoid::fillPie(origin.x, origin.y, radius, a0, a1, [&](int x, int y, bool line) {
    //     if (line || (((x * 2 + y) % 4) == 1)) {
    //       mDisplay.drawPixel(x, y, SSD1306_WHITE);
    //     }
    //   });
    // }

    if (angleSweep >= 0) {
      drawLine(origin.x, origin.y, origin.x + pd.p0.x, origin.y + pd.p0.y, [&](int x, int y, bool) {
        mDisplay.drawPixel(x, y, SSD1306_WHITE);
      });
    } else {
      drawLine(origin.x, origin.y, origin.x + pd.p1.x, origin.y + pd.p1.y, [&](int x, int y, bool) {
        mDisplay.drawPixel(x, y, SSD1306_WHITE);
      });
    }
  }

  // Pixel
  virtual void SetPixel(const PointI& pt, uint16_t color) override { mDisplay.writePixel(pt.x, pt.y, color); }
  virtual void SetPixelShaded(const PointI& pt, int coverageQp8) override
  {
    mDisplay.writeDitheredPixel(pt, coverageQp8);
  }

  // template <int AntSize, int AntMask, int ySign, int xSign>
  //   virtual void DrawMarchingAntsFilledRect(int AntSize,
  //                                           int AntMask,
  //                                           int ySign,
  //                                           int xSign,
  //                                           int xstart,
  //                                           int ystart,
  //                                           int w,
  //                                           int h,
  //                                           int variation) override
  //   {
  //     for (int y = ystart; y < ystart + h; ++y) {
  //       for (int x = xstart; x < xstart + w; ++x) {
  //         int p = abs(xSign * x + ySign * y - variation) % AntSize; // p is the position in the pattern.
  //         bool parity = !!(AntMask & (1 << p));
  //         mDisplay.drawPixel(x, y, parity ? SSD1306_WHITE : SSD1306_BLACK);
  //       }
  //     }
  //   }

  // template <int LineWidth, int AntSize, int AntMask>
  //   virtual void DrawMarchingAntsRectOutline(int LineWidth,
  //                                            int AntSize,
  //                                            int AntMask,
  //                                            int x,
  //                                            int y,
  //                                            int w,
  //                                            int h,
  //                                            int variation,
  //                                            AntStyle style,
  //                                            Edges::Flags edges) override
  //   {
  //     int n1 = style == AntStyle::Chasing ? -1 : 1;
  //     // draw rect INSIDE the given coords
  //     if (edges & Edges::Top)
  //       DrawMarchingAntsFilledRect(AntSize, AntMask, 1, 1, x, y, w, LineWidth, variation); // top rect
  //     if (edges & Edges::Bottom)
  //       DrawMarchingAntsFilledRect(AntSize, AntMask, 1, n1, x, y + h - LineWidth, w, LineWidth, variation); // bottom
  //       rect
  //     if (edges & Edges::Left)
  //       DrawMarchingAntsFilledRect(
  //         AntSize, AntMask, n1, n1, x, y + LineWidth, LineWidth, h - LineWidth - LineWidth, variation); // left
  //     if (edges & Edges::Right)
  //       DrawMarchingAntsFilledRect(AntSize,
  //                                  AntMask,
  //                                  1,
  //                                  1,
  //                                  x + w - LineWidth,
  //                                  y + LineWidth,
  //                                  LineWidth,
  //                                  h - LineWidth - LineWidth,
  //                                  variation); // right
  //   }
};
} // namespace clarinoid
