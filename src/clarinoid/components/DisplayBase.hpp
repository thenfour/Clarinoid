

#pragma once

#include <cstdint>
#include <clarinoid/basic/Basic.hpp>

namespace clarinoid {

struct Edges
{
  using Flags = uint8_t;
  static constexpr Flags Left = 1;
  static constexpr Flags Top = 2;
  static constexpr Flags Right = 4;
  static constexpr Flags Bottom = 8;
  static constexpr Flags All = 0xff;
};

enum struct AntStyle
{
  Continuous,
  Chasing,
};

// drawing modes:
// - solid color (SSD1306_BLACK / SSD1306_WHITE)
// - inverted
// - "marching ants" - animated
// - dithered
// - "selection"
// - "solid text" when false, hatch pattern 50%.

//////////////////////////////////////////////////////////////////////
struct IMonochromeDisplay
{
  // basics & utilities
  virtual SizeI ScreenSize() const = 0;
  virtual RectI ScreenRect() const = 0;
  virtual void Dim(bool d) = 0;
  virtual void PresentToDevice() = 0;

  // clipping
  virtual void SetClipRect(const RectI& rc) = 0;
  virtual RectI GetClipRect() const = 0;
  virtual void ResetClipRect() = 0;
  virtual void SetClipRectToMargin(int m) = 0;

  // text
  virtual SizeI GetTextSize() const = 0;
  virtual void SetTextSize(const SizeI& s) = 0;
  virtual uint16_t GetLineHeight() const = 0;
  virtual void SetTextSolid(bool b) = 0;
  virtual bool GetTextSolid() = 0;
  virtual void SetTextColor(uint16_t c) = 0;
  virtual void SetTextColor(uint16_t c, uint16_t bg) = 0;
  virtual int GetTextLeftMargin() = 0;
  virtual void SetTextLeftMargin(int) = 0;
  virtual void SetTextWrap(bool w) = 0;
  virtual RectI GetTextBounds(const String& str) = 0;
  virtual PointI GetCursor() const = 0;
  virtual void SetCursor(const PointI& pt) = 0;
  virtual void Print(const String& s) = 0;
  virtual void PrintLine(const String& s) = 0;
  virtual void PrintInvertedText(const String& str, bool isInverted = true) = 0;
  virtual void PrintInvertedLine(
    const String& str,
    bool isInverted = true) = 0; // calculates in general, not for a specific location on screen.

  // axis-aligned lines
  virtual void DrawDottedHLine(int16_t left, int16_t width, int16_t y, uint16_t color) = 0;
  virtual void DrawDottedRect(int16_t left, int16_t top, int16_t width, int16_t height, uint16_t color) = 0;
  virtual void DrawHLineDithered(const PointI& pt, int length, int brightnessQp8) = 0;
  virtual void DrawVLine(int16_t x, int16_t y, int16_t h, uint16_t color) = 0;
  virtual void DrawHLine(int16_t x, int16_t y, int16_t w, uint16_t color) = 0;

  // axis-aligned Rects
  virtual void FillScreen(uint16_t color) = 0;
  virtual void FillRect(const RectI& rc, uint16_t color) = 0;
  virtual void DrawMarchingAntsFilledRect(int AntSize, // DrawMarchingAntsFilledRect
                                          int AntMask,
                                          int ySign,
                                          int xSign,
                                          int xstart,
                                          int ystart,
                                          int w,
                                          int h,
                                          int variation) = 0;
  virtual void DrawMarchingAntsRectOutline(int LineWidth,
                                           int AntSize,
                                           int AntMask,
                                           int x,
                                           int y,
                                           int w,
                                           int h,
                                           int variation,
                                           AntStyle style,
                                           Edges::Flags edges) = 0;
  // virtual void DrawSelectionRect(const RectI& z) = 0;
  virtual void FillRectWithBrightness(const RectI& rc, int brightnessQp8) = 0;
  virtual void FillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color) = 0;

  // lines (not axis-aligned)
  virtual void DrawLineWithBrightness(const PointI& pt0, const PointI& pt1, int brightness) = 0;
  virtual void DrawLine(const PointI& pt0, const PointI& pt1) = 0;
  virtual void DrawInfiniteLineClipped(const PointI& pt0, const PointI& pt1, const RectI& clipRect, int brightness) = 0;

  // Bitmaps
  virtual void DrawBitmap(PointI pos, const BitmapSpec& bmp) = 0;

  // circles
  virtual void FillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) = 0;
  virtual void FillCircleWithBrightness(const PointI& c, int r, int brightnessQp8) = 0;

  // "pie"
  virtual void FillPie(const PointF& origin, float radius, float angleStart, float angleSweep, bool filled = true) = 0;

  // Pixel
  virtual void SetPixel(const PointI& pt, uint16_t color) = 0;
  virtual void SetPixelShaded(const PointI&, int coverageQp8) = 0; // 0..255
};

}