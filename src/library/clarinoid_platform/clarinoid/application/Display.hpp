
// This is basically a clarinoid-specific "controller" for a display.

#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include <clarinoid/components/SSD1306.hpp>
#include <clarinoid/application/ControlMapper.hpp>

#include "DisplayDefs.hpp"

#include "clarinoid/application/Font/matchup8.hpp"
#include "clarinoid/application/Font/chicago4px7b.hpp"
#include "clarinoid/application/Font/TomThumb.hpp"
#include "clarinoid/application/Font/eighties8.hpp"

namespace clarinoid {

static constexpr int TOAST_DURATION_MILLIS = 1600;

//////////////////////////////////////////////////////////////////////
struct ClarinoidDisplay : IAppDisplay
{
  //   private:
  //     static CCAdafruitSSD1306
  //         *gDisplay; // this is only to allow the crash handler to output to the screen. not for app use in general.

public:
  SSD1306& mDisplay;

  AppSettings* mAppSettings = nullptr;
  InputDelegator* mInput = nullptr;
  IHudProvider* mHudProvider = nullptr;

  bool mIsSetup = false; // used for crash handling to try and setup this if we can
  bool mFirstAppSelected = false;

  array_view<IDisplayApp*> mApps;
  int mCurrentAppIndex = 0;

  ClarinoidDisplay(SSD1306& display)
    : mDisplay(display)
  {
  }

  void Init(AppSettings* appSettings, InputDelegator* input, IHudProvider* hud, const array_view<IDisplayApp*>& apps)
  {
    mAppSettings = appSettings;
    mInput = input;
    mApps = apps;
    mHudProvider = hud;

    // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
    mDisplay.mDisplay.begin(SSD1306_SWITCHCAPVCC);
    mIsSetup = true;

    // fix fonts up a bit
    for (auto& glyph : MatchupPro8pt7bGlyphs) {
      glyph.yOffset += 6;
    }
    for (auto& glyph : pixChicago4pt7bGlyphs) {
      glyph.yOffset += 9;
    }
    for (auto& glyph : TomThumbGlyphs) {
      glyph.yOffset += 6;
    }
    for (auto& glyph : Eighties8pt7bGlyphs) {
      glyph.yOffset += 6;
    }

    mDisplay.mDisplay.dim(mAppSettings->mDisplayDim);

    // welcome msg.
    ClearState();
    mDisplay.mDisplay.clearDisplay();
    mDisplay.mDisplay.println(gClarinoidVersion);
    mDisplay.mDisplay.display();

    for (size_t i = 0; i < mApps.mSize; ++i) {
      mApps.mData[i]->DisplayAppInit();
    }
  }

  virtual AppSettings* GetAppSettings() override { return mAppSettings; }
  virtual InputDelegator* GetInput() override { return mInput; }

  virtual void SelectApp(int n) override
  {
    n = RotateIntoRange(n, mApps.mSize);

    if (n == mCurrentAppIndex)
      return;

    mApps.mData[mCurrentAppIndex]->DisplayAppOnUnselected();
    mCurrentAppIndex = n;
    mApps.mData[mCurrentAppIndex]->DisplayAppOnSelected();
  }

  virtual void ScrollApps(int delta) override { SelectApp(mCurrentAppIndex + delta); }

  int mframe = 0;

  // splitting the entire "display actions" into sub tasks:
  // UpdateAndRenderTask which runs state updating and  renders to the DMA
  // DisplayTask which "uploads" to the device. This is a natural separation of things to give the task runner some
  // method of yielding.
  virtual void UpdateAndRenderTask() override
  {
    CCASSERT(this->mIsSetup);

    mToggleReader.Update(&mInput->mDisplayFontToggle);
    if (mToggleReader.IsNewlyPressed()) {
      mCurrentFontIndex = RotateIntoRange(mCurrentFontIndex + 1, SizeofStaticArray(mGUIFonts));
    }

    IDisplayApp* pMenuApp = nullptr;

    if (mCurrentAppIndex < (int)mApps.mSize) {
      pMenuApp = mApps.mData[mCurrentAppIndex];

      if (!mFirstAppSelected) {
        mFirstAppSelected = true;
        pMenuApp->DisplayAppOnSelected();
      }

      pMenuApp->DisplayAppUpdate();
    }

    ClearState();
    mDisplay.mDisplay.clearDisplay();

    if (pMenuApp) {
      pMenuApp->DisplayAppRender();
    }

    if (mIsShowingToast) {
      if (mToastTimer.ElapsedTime().ElapsedMillisI() >= TOAST_DURATION_MILLIS) {
        mIsShowingToast = false;
      } else {
        // render toast.
        SetupModal();
        mDisplay.mDisplay.print(mToastMsg);
      }
    }

    ClearState();
    mHudProvider->IHudProvider_RenderHud(mDisplay.mDisplay.width(), mDisplay.mDisplay.height());

    String s = mHudProvider->IHudProvider_GetHudTransientIndicator(this->mInput->mModifierFine.CurrentValue(),
                                                                   this->mInput->mModifierCourse.CurrentValue(),
                                                                   this->mInput->mModifierShift.CurrentValue(),
                                                                   this->mInput->mModifierTranspose.CurrentValue(),
                                                                   this->mInput->mModifierTempo.CurrentValue(),
                                                                   this->mInput->mModifierKey.CurrentValue(),
                                                                   this->mInput->mModifierHarm.CurrentValue());
    if (s.length() > 0) {
      ClearState();
      int16_t x, y;
      uint16_t w, h;
      mDisplay.mDisplay.getTextBounds(s, 0, 0, &x, &y, &w, &h);
      x = mDisplay.mDisplay.width() - w; // x is where the text will appear
      mDisplay.FillRect({ x - 1 /*start rect 1 px left*/, y, w + 2, h + 2 }, SSD1306_WHITE);
      mDisplay.SetTextColor(SSD1306_BLACK, SSD1306_WHITE);
      mDisplay.SetCursor({ x, 1 }); // y + 2
      mDisplay.Print(s);
    }
  }

  virtual int16_t GetHudHeight() const override { return mHudProvider->IHudProvider_GetHudHeight(); }

  virtual int16_t GetClientHeight() const override { return mDisplay.ScreenSize().height - GetHudHeight(); }
  virtual RectI GetClientRect() const override
  {
    return RectI::Construct({ 0, 0 }, { mDisplay.ScreenSize().height, GetClientHeight() });
  }

  //   // calculates in general, not for a specific location on screen.
  //   virtual RectI GetTextBounds(const String& str) override
  //   {
  //     int16_t x, y;
  //     uint16_t h, w;
  //     mDisplay.getTextBounds(str, 0, 0, &x, &y, &w, &h);
  //     return RectI::Construct(x, y, w, h);
  //   }

  //   virtual void PrintInvertedText(const String& str, bool isInverted = true) override
  //   {
  //     if (isInverted) {
  //       int16_t x, y;
  //       uint16_t w, h;
  //       mDisplay.getTextBounds(str, mDisplay.getCursorX(), mDisplay.getCursorY(), &x, &y, &w, &h);
  //       mDisplay.fillRect(x, y, w, h, SSD1306_WHITE);
  //       mDisplay.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  //     } else {
  //       mDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
  //     }

  //     mDisplay.print(str);
  //   }

  //   virtual void PrintInvertedLine(const String& str, bool isInverted = true) override
  //   {
  //     PrintInvertedText(str, isInverted);
  //     mDisplay.println();
  //   }

  virtual void DrawSelectionRect(const RectI& z) override
  {
    mDisplay.DrawMarchingAntsRectOutline(
      1, 3, 1, z.Left(), z.Top(), z.Width(), z.Height(), (micros() / (1000 * 120)), AntStyle::Chasing, Edges::All);
  }

  virtual void DisplayTask() override
  {
    // mDisplay.mFrameCount++;
    mDisplay.mDisplay.display();
  }

  Stopwatch mToastTimer;
  bool mIsShowingToast = false;
  String mToastMsg;
  size_t mCurrentFontIndex = 0;

  // NB: CHANGING ANYTHING IN HERE, ALSO CHANGE
  // SelectTinyFont().
  GFXfont const* mGUIFonts[5] = {
    &MatchupPro8pt7b, nullptr, &Eighties8pt7b, &pixChicago4pt7b, &TomThumb,
  };

  virtual void SelectTinyFont() override { mDisplay.mDisplay.setFont(mGUIFonts[4]); }

  virtual void SelectEightiesFont() override { mDisplay.mDisplay.setFont(mGUIFonts[2]); }

  virtual void SelectNormalFont() override { mDisplay.mDisplay.setFont(mGUIFonts[mCurrentFontIndex]); }

  SwitchControlReader mToggleReader;

  virtual void ShowToast(const String& msg) override
  {
    mIsShowingToast = true;
    mToastMsg = msg;
    mToastTimer.Restart();
  }

  // virtual int ClippedAreaHeight() const override { return mDisplay.mClipBottom - mDisplay.mClipTop; }

  // virtual void ResetClip() override { mDisplay.SetClipRect(0, 0, mDisplay.width(), GetClientHeight()); }

  // virtual void SetClipRect(const RectI& rc) override { mDisplay.SetClipRect(rc.x, rc.y, rc.right(), rc.bottom()); }

  // virtual void ClipToMargin(int m) override { mDisplay.SetClipRect(m, m, mDisplay.width() - m, GetClientHeight() -
  // m); }

  virtual void ClearState() override
  {
    mDisplay.mDisplay.setFont(mGUIFonts[mCurrentFontIndex]);
    mDisplay.SetTextSolid(true);
    mDisplay.SetTextLeftMargin(0);
    mDisplay.SetTextWrap(true);
    mDisplay.ResetClipRect();
    mDisplay.mDisplay.setTextSize(1);
    mDisplay.SetTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
    mDisplay.SetCursor({ 0, 0 });
  }

  //   virtual void DrawBitmap(PointI pos, const BitmapSpec& bmp) override
  //   {
  //     mDisplay.drawBitmap(pos.x, pos.y, bmp.pBmp, bmp.widthPixels, bmp.heightPixels, SSD1306_WHITE);
  //   }

  //   virtual void fillPie(const PointF& origin,
  //                        float radius,
  //                        float angleStart,
  //                        float angleSweep,
  //                        bool filled = true) override
  //   {
  //     float a0, a1;
  //     if (angleSweep >= 0) {
  //       a0 = angleStart;
  //       a1 = a0 + angleSweep;
  //     } else {
  //       a0 = angleStart + angleSweep;
  //       a1 = angleStart;
  //     }
  //     ::clarinoid::PieData pd;
  //     if (filled) {
  //       pd = ::clarinoid::fillPie(origin.x, origin.y, radius, a0, a1, [&](int x, int y, bool line) {
  //         if (!line && ((x + y) & 1))
  //           return;
  //         mDisplay.drawPixel(x, y, SSD1306_WHITE);
  //       });
  //     } else {
  //       pd = ::clarinoid::fillPie(origin.x, origin.y, radius, a0, a1, [&](int x, int y, bool line) {
  //         if (line || (((x * 2 + y) % 4) == 1)) {
  //           mDisplay.drawPixel(x, y, SSD1306_WHITE);
  //         }
  //       });
  //     }
  //     if (angleSweep >= 0) {
  //       drawLine(origin.x, origin.y, origin.x + pd.p0.x, origin.y + pd.p0.y, [&](int x, int y, bool) {
  //         mDisplay.drawPixel(x, y, SSD1306_WHITE);
  //       });
  //     } else {
  //       drawLine(origin.x, origin.y, origin.x + pd.p1.x, origin.y + pd.p1.y, [&](int x, int y, bool) {
  //         mDisplay.drawPixel(x, y, SSD1306_WHITE);
  //       });
  //     }
  //   }

  // draws & prepares the screen for a modal message. after this just print text whatever.
  // returns the client area of the modal
  virtual RectI SetupModal(/*int pad = 1, int rectStart = 2, int textStart = 4*/) override
  {
    const int pad = 1;
    const int rectStart = 2;
    const int textStart = 4;
    ClearState();
    mDisplay.mDisplay.fillRect(pad, pad, mDisplay.ScreenSize().width - pad, GetClientHeight() - pad, SSD1306_BLACK);
    mDisplay.mDisplay.drawRect(
      rectStart, rectStart, mDisplay.ScreenSize().width - rectStart, GetClientHeight() - rectStart, SSD1306_WHITE);
    mDisplay.SetTextLeftMargin(textStart);
    mDisplay.SetClipRectToMargin(textStart);
    mDisplay.SetCursor({ textStart, textStart });
    auto ret = RectI::Construct({ textStart, textStart },
                                { mDisplay.ScreenSize().width - textStart, GetClientHeight() - textStart });
    return ret;
  }

  //   virtual void FillRect2Pt(int16_t p1x, int16_t p1y, int16_t p2x, int16_t p2y, uint16_t color) override
  //   {
  //     // account for the fact that p1 and p2 may not be in the correct order.
  //     int16_t x1 = min(p1x, p2x);
  //     int16_t x2 = max(p1x, p2x);
  //     int16_t y1 = min(p1y, p2y);
  //     int16_t y2 = max(p1y, p2y);
  //     mDisplay.fillRect(x1, y1, x2 - x1, y2 - y1, color);
  //   }

  //   // required for IDisplay.
  //   virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override
  //   {
  //     mDisplay.fillRect(x, y, w, h, color);
  //   }
  //   virtual void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) override
  //   {
  //     mDisplay.fillCircle(x0, y0, r, color);
  //   }
  //   virtual int16_t width() const override { return mDisplay.width(); }
  //   virtual int16_t height() const override { return mDisplay.height(); }
  //   virtual int16_t getCursorX() const override { return mDisplay.getCursorX(); }
  //   virtual int16_t getCursorY() const override { return mDisplay.getCursorY(); }
  //   virtual void setTextWrap(bool w) override { mDisplay.setTextWrap(w); }
  //   virtual void setTextColor(uint16_t c) override { mDisplay.setTextColor(c); }
  //   virtual void setCursor(int16_t x, int16_t y) override { mDisplay.setCursor(x, y); }
  //   virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override
  //   {
  //     mDisplay.drawFastVLine(x, y, h, color);
  //   }
  //   virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override
  //   {
  //     mDisplay.drawFastHLine(x, y, w, color);
  //   }
  //   virtual void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color) override
  //   {
  //     mDisplay.fillRoundRect(x0, y0, w, h, radius, color);
  //   }
  //   virtual void print(const String& s) override { mDisplay.print(s); }
  //   virtual void println(const String& s) override { mDisplay.println(s); }

  //   virtual uint16_t GetLineHeight() const override { return mDisplay.GetLineHeight(); }
  //   virtual void SetClipRect(int left, int top, int right, int bottom) override
  //   {
  //     mDisplay.SetClipRect(left, top, right, bottom);
  //   }
  //   virtual void SetTextSolid(bool b) override { mDisplay.mSolidText = b; }
  //   virtual bool GetTextSolid() override { return mDisplay.mSolidText; }
  //   virtual int GetTextLeftMargin() override { return mDisplay.mTextLeftMargin; }
  //   virtual void SetTextLeftMargin(int m) override { mDisplay.mTextLeftMargin = m; }
  //   virtual void drawPixel(int16_t x, int16_t y, uint16_t color) override { mDisplay.drawPixel(x, y, color); }
  //   virtual void fillScreen(uint16_t color) override { mDisplay.fillScreen(color); }
  //   virtual void DrawDottedRect(int16_t left, int16_t top, int16_t width, int16_t height, uint16_t color) override
  //   {
  //     mDisplay.DrawDottedRect(left, top, width, height, color);
  //   }
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
  //     mDisplay.DrawMarchingAntsFilledRect(AntSize, AntMask, ySign, xSign, xstart, ystart, w, h, variation);
  //   }
  //   virtual void DrawDottedHLine(int16_t left, int16_t width, int16_t y, uint16_t color) override
  //   {
  //     mDisplay.DrawDottedHLine(left, width, y, color);
  //   }
  //   virtual void dim(bool d) override { mDisplay.dim(d); }
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
  //     mDisplay.DrawMarchingAntsRectOutline(LineWidth, AntSize, AntMask, x, y, w, h, variation, style, edges);
  //   }

  //   virtual void SetPixel(const PointI& pt, uint16_t color) override { mDisplay.drawPixel(pt.x, pt.y, color); }

  //   virtual bool IsInBounds(const PointI& pt) const override
  //   {
  //     // return pt.x >= 0 && pt.x < mDisplay.width() && pt.y >= 0 && pt.y < mDisplay.height();
  //     //  consider the clip rect.
  //     return pt.x >= mDisplay.mClipLeft && pt.x < mDisplay.mClipRight && pt.y >= mDisplay.mClipTop &&
  //            pt.y < mDisplay.mClipBottom;
  //   }

  // virtual void SetPixelShaded(const PointI& pt, int coverageQp8) override
  // {
  //   if (!IsInBounds(pt)) {
  //     return;
  //   }

  //   coverageQp8 = ClampInclusive(coverageQp8, 0, 255);
  //   SetPixel(pt, matrix.getDitheredColor(coverageQp8, pt));
  // }

  /// Draws a horizontal line of 'length' pixels starting at (x, y),
  /// dithering each pixel according to 'brightness' and the DitherMatrix.
  //   virtual void DrawHLineDithered(const PointI& pt, int length, int brightnessQp8) override
  //   {
  //     if (length <= 0)
  //       return;
  //     for (int i = 0; i < length; i++) {
  //       int currentX = pt.x + i;
  //       PointI currentPt{ currentX, pt.y };
  //       SetPixelShaded(currentPt, brightnessQp8);
  //     }
  //   }

  //   virtual void FillRectWithBrightness(const RectI& rc, int brightnessQp8) override
  //   {
  //     brightnessQp8 = ClampInclusive(brightnessQp8, 0, 255);
  //     for (int row = 0; row < rc.height; row++) {
  //       int currentY = rc.y + row;
  //       for (int col = 0; col < rc.width; col++) {
  //         int currentX = rc.x + col;
  //         PointI currentPt{ currentX, currentY };
  //         SetPixelShaded(currentPt, brightnessQp8);
  //       }
  //     }
  //   }

  //   // finds the two points in 'points' that are farthest apart. supports only up to 4 points due to O(n^2)
  //   complexity.
  //   // assumes at least 1 point.
  //   virtual void FindFarthestPair(const PointI* points, size_t pointCount, PointI& bestA, PointI& bestB) override
  //   {
  //     bestA = points[0];
  //     bestB = points[0];
  //     float maxDistSq = 0;

  //     // Simple O(n^2) check (fine for up to 4 points):
  //     for (size_t i = 0; i < pointCount; i++) {
  //       for (size_t j = i + 1; j < pointCount; j++) {
  //         float dx = points[i].x - points[j].x;
  //         float dy = points[i].y - points[j].y;
  //         float distSq = dx * dx + dy * dy;
  //         if (distSq > maxDistSq) {
  //           maxDistSq = distSq;
  //           bestA = points[i];
  //           bestB = points[j];
  //         }
  //       }
  //     }
  //   }

  /// <summary>
  /// Draws a line from (x0, y0) to (x1, y1) with simulated brightness using
  /// the given DitherMatrix for Bayer dithering (purely in fixed-point).
  /// </summary>
  /// <param name="matrix">The dither matrix to use (e.g. 2x2, 4x4).</param>
  /// <param name="x0">Starting X coordinate.</param>
  /// <param name="y0">Starting Y coordinate.</param>
  /// <param name="x1">Ending X coordinate.</param>
  /// <param name="y1">Ending Y coordinate.</param>
  /// <param name="brightness">Brightness level (0-255).</param>
  //   virtual void DrawLineWithBrightness(const PointI& pt0, const PointI& pt1, int brightnessQp8) override
  //   {
  //     brightnessQp8 = ClampInclusive(brightnessQp8, 0, 255);

  //     int dx = std::abs(pt1.x - pt0.x);
  //     int dy = std::abs(pt1.y - pt0.y);

  //     int sx = (pt0.x < pt1.x) ? 1 : -1;
  //     int sy = (pt0.y < pt1.y) ? 1 : -1;

  //     int err = dx - dy;

  //     int x = pt0.x;
  //     int y = pt0.y;

  //     // is this function freezing? alternative impl below...
  //     // while (true)
  //     // {
  //     //     SetPixelShaded(PointI { x, y }, brightnessQp8);

  //     //     if (x == pt1.x && y == pt1.y) break;

  //     //     int e2 = 2 * err;
  //     //     if (e2 > -dy) { err -= dy; x += sx; }
  //     //     if (e2 < dx) { err += dx; y += sy; }
  //     // }

  //     while (true) {
  //       SetPixelShaded(PointI{ x, y }, brightnessQp8);

  //       if (x == pt1.x && y == pt1.y)
  //         break;

  //       int e2 = 2 * err;
  //       if (e2 > -dy) {
  //         err -= dy;
  //         x += sx;
  //       } else if (e2 < dx) { // Use `else if` to ensure only one step occurs
  //         err += dx;
  //         y += sy;
  //       }
  //     }
  //   }

  //   virtual void DrawLine(const PointI& pt0, const PointI& pt1) override
  //   {
  //     return mDisplay.drawLine(pt0.x, pt0.y, pt1.x, pt1.y, SSD1306_WHITE);
  //   }

  // draw circle filled with brightness
  /// <summary>
  /// Fills a circle of radius r centered at (x0,y0) with the given brightness,
  /// using a DitherMatrix for 1-bit dithering. No floating-point is used.
  /// </summary>
  //   virtual void FillCircleWithBrightness(const PointI& c, int r, int brightnessQp8) override
  //   {
  //     if (r <= 0)
  //       return; // no valid radius

  //     // Midpoint circle algorithm setup
  //     int x = 0;
  //     int y = r;

  //     // f is our "decision" variable, starts at 1 - r
  //     int f = 1 - r;
  //     // ddF_x, ddF_y track derivative changes
  //     int ddF_x = 1;
  //     int ddF_y = -2 * r;

  //     // We'll fill from the center's horizontal line out
  //     // so first fill the horizontal line across the circle's diameter
  //     // y=0 => from (x0-r) to (x0+r)
  //     DrawHLineDithered({ c.x - r, c.y }, (2 * r + 1), brightnessQp8);

  //     // Also fill the symmetrical horizontal lines above & below
  //     // for each step of x,y as we move around the circle edges
  //     while (x < y) {
  //       // If f >= 0, move y inward
  //       if (f >= 0) {
  //         y--;
  //         ddF_y += 2;
  //         f += ddF_y;
  //       }
  //       // Always move x outward
  //       x++;
  //       ddF_x += 2;
  //       f += ddF_x;

  //       // Now we have a circle boundary at (x,y). We fill horizontal lines:
  //       // "Top"  side at y0 + y
  //       // "Bottom" side at y0 - y
  //       // each goes from (x0 - x) to (x0 + x)

  //       DrawHLineDithered({ c.x - x, c.y + y }, (2 * x + 1), brightnessQp8);
  //       if (y != 0) // if y=0, top & bottom would be same line
  //       {
  //         DrawHLineDithered({ c.x - x, c.y - y }, (2 * x + 1), brightnessQp8);
  //       }

  //       // For x != y, fill those "side" lines near (y,x) due to circle symmetry:
  //       //   left side at x0 - y .. x0 + y, top y= y0 + x
  //       //   left side at x0 - y .. x0 + y, bottom y= y0 - x
  //       if (x != y) {
  //         DrawHLineDithered({ c.x - y, c.y + x }, (2 * y + 1), brightnessQp8);
  //         if (x != 0) // if x=0, same line repeated
  //         {
  //           DrawHLineDithered({ c.x - y, c.y - x }, (2 * y + 1), brightnessQp8);
  //         }
  //       }
  //     }
  //   }

  // impl of passthrough IMonochromeDisplay stuff...
  virtual SizeI ScreenSize() const override { return mDisplay.ScreenSize(); }
  virtual RectI ScreenRect() const override { return mDisplay.ScreenRect(); }
  virtual void Dim(bool d) override { mDisplay.Dim(d); }
  virtual void PresentToDevice() override { mDisplay.PresentToDevice(); }

  virtual void SetClipRect(const RectI& rc) override { mDisplay.SetClipRect(rc); }
  virtual RectI GetClipRect() const override { return mDisplay.GetClipRect(); }
  virtual void ResetClipRect() override { mDisplay.ResetClipRect(); }
  virtual void SetClipRectToMargin(int m) override { mDisplay.SetClipRectToMargin(m); }
  virtual uint16_t GetLineHeight() const override { return mDisplay.GetLineHeight(); }

  virtual SizeI GetTextSize() const override { return mDisplay.GetTextSize(); }
  virtual void SetTextSize(const SizeI& s) override { mDisplay.SetTextSize(s); }
  virtual void SetTextSolid(bool b) override { mDisplay.SetTextSolid(b); }
  virtual bool GetTextSolid() override { return mDisplay.GetTextSolid(); }
  virtual void SetTextColor(uint16_t c) override { mDisplay.SetTextColor(c); }
  virtual void SetTextColor(uint16_t c, uint16_t bg) override { mDisplay.SetTextColor(c, bg); }
  virtual int GetTextLeftMargin() override { return mDisplay.GetTextLeftMargin(); }
  virtual void SetTextLeftMargin(int m) override { mDisplay.SetTextLeftMargin(m); }
  virtual void SetTextWrap(bool w) override { mDisplay.SetTextWrap(w); }
  virtual RectI GetTextBounds(const String& str) override { return GetTextBounds(str); }
  virtual PointI GetCursor() const override { return mDisplay.GetCursor(); }
  virtual void SetCursor(const PointI& pt) override { mDisplay.SetCursor(pt); }
  virtual void Print(const String& s) override { mDisplay.Print(s); }
  virtual void PrintLine(const String& s) override { mDisplay.PrintLine(s); }
  virtual void PrintInvertedText(const String& str, bool isInverted = true) override
  {
    mDisplay.PrintInvertedText(str, isInverted);
  }
  virtual void PrintInvertedLine(const String& str, bool isInverted = true) override
  {
    mDisplay.PrintInvertedLine(str, isInverted);
  }
  virtual void DrawDottedHLine(int16_t left, int16_t width, int16_t y, uint16_t color) override
  {
    mDisplay.DrawDottedHLine(left, width, y, color);
  }
  virtual void DrawDottedRect(int16_t left, int16_t top, int16_t width, int16_t height, uint16_t color) override
  {
    mDisplay.DrawDottedRect(left, top, width, height, color);
  }
  virtual void DrawHLineDithered(const PointI& pt, int length, int brightnessQp8) override
  {
    mDisplay.DrawHLineDithered(pt, length, brightnessQp8);
  }
  virtual void DrawVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override
  {
    mDisplay.DrawVLine(x, y, h, color);
  }
  virtual void DrawHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override
  {
    mDisplay.DrawHLine(x, y, w, color);
  }
  virtual void FillScreen(uint16_t color) override { mDisplay.FillScreen(color); }
  virtual void FillRect(const RectI& rc, uint16_t color) override { mDisplay.FillRect(rc, color); }
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
    mDisplay.DrawMarchingAntsFilledRect(AntSize, AntMask, ySign, xSign, xstart, ystart, w, h, variation);
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
    mDisplay.DrawMarchingAntsRectOutline(LineWidth, AntSize, AntMask, x, y, w, h, variation, style, edges);
  }
  // virtual void DrawSelectionRect(const RectI& z) override { mDisplay.DrawSelectionRect(z); }
  virtual void FillRectWithBrightness(const RectI& rc, int brightnessQp8) override
  {
    mDisplay.FillRectWithBrightness(rc, brightnessQp8);
  }
  virtual void FillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color) override
  {
    mDisplay.FillRoundRect(x0, y0, w, h, radius, color);
  }
  virtual void DrawLineWithBrightness(const PointI& pt0, const PointI& pt1, int brightness) override
  {
    mDisplay.DrawLineWithBrightness(pt0, pt1, brightness);
  }
  virtual void DrawLine(const PointI& pt0, const PointI& pt1) override { mDisplay.DrawLine(pt0, pt1); }
  virtual void DrawInfiniteLineClipped(const PointI& pt0,
                                       const PointI& pt1,
                                       const RectI& clipRect,
                                       int brightness) override
  {
    mDisplay.DrawInfiniteLineClipped(pt0, pt1, clipRect, brightness);
  }
  virtual void DrawBitmap(PointI pos, const BitmapSpec& bmp) override { mDisplay.DrawBitmap(pos, bmp); }
  virtual void FillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) override
  {
    mDisplay.FillCircle(x0, y0, r, color);
  }
  virtual void FillCircleWithBrightness(const PointI& c, int r, int brightnessQp8) override
  {
    mDisplay.FillCircleWithBrightness(c, r, brightnessQp8);
  }
  virtual void FillPie(const PointF& origin,
                       float radius,
                       float angleStart,
                       float angleSweep,
                       bool filled = true) override
  {
    mDisplay.FillPie(origin, radius, angleStart, angleSweep, filled);
  }
  virtual void SetPixel(const PointI& pt, uint16_t color) override { mDisplay.SetPixel(pt, color); }
  virtual void SetPixelShaded(const PointI& pt, int coverageQp8) override { mDisplay.SetPixelShaded(pt, coverageQp8); }
};

} // namespace clarinoid
