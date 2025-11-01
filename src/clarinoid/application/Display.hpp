
// This is basically a clarinoid-specific "controller" for a display.

#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include "clarinoid/components/AdafruitSSD1366Wrapper.hpp"
#include <clarinoid/application/ControlMapper.hpp>

#include "DisplayDefs.hpp"

#include "clarinoid/application/Font/matchup8.hpp"
#include "clarinoid/application/Font/chicago4px7b.hpp"
#include "clarinoid/application/Font/TomThumb.hpp"
#include "clarinoid/application/Font/eighties8.hpp"

namespace clarinoid
{

static constexpr int TOAST_DURATION_MILLIS = 1600;

static DitherMatrix<2> gBayer2x2Matrix{{0, 2, 3, 1}};

static DitherMatrix<4> gBayer4x4Matrix{{0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 15, 7, 13, 5}};

static DitherMatrix<8> gBayer8x8Matrix{{0,  48, 12, 60, 3,  51, 15, 63, 32, 16, 44, 28, 35, 19, 47, 31,
                                        8,  56, 4,  52, 11, 59, 7,  55, 40, 24, 36, 20, 43, 27, 39, 23,
                                        2,  50, 14, 62, 1,  49, 13, 61, 34, 18, 46, 30, 33, 17, 45, 29,
                                        10, 58, 6,  54, 9,  57, 5,  53, 42, 26, 38, 22, 41, 25, 37, 21}};

//////////////////////////////////////////////////////////////////////
struct _CCDisplay : IDisplay
{
    //   private:
    //     static CCAdafruitSSD1306
    //         *gDisplay; // this is only to allow the crash handler to output to the screen. not for app use in
    //         general.

  public:
    CCAdafruitSSD1306 &mDisplay;

    AppSettings *mAppSettings = nullptr;
    InputDelegator *mInput = nullptr;
    IHudProvider *mHudProvider = nullptr;

    bool mIsSetup = false; // used for crash handling to try and setup this if we can
    bool mFirstAppSelected = false;

    array_view<IDisplayApp *> mApps;
    int mCurrentAppIndex = 0;

    using PreToastRenderFunc = cc::function<void(void *)>::ptr_t; // void fn(void* capture)
    PreToastRenderFunc mPreToastRenderFunc[MAX_PRE_TOAST_RENDER_FUNCS];
    void *mPreToastRenderFuncCaptures[MAX_PRE_TOAST_RENDER_FUNCS];
    size_t mPreToastRenderFuncCount = 0;

    void RegisterPreToastRenderFunc(void *capture, PreToastRenderFunc func)
    {
        if (mPreToastRenderFuncCount < MAX_PRE_TOAST_RENDER_FUNCS)
        {
            mPreToastRenderFunc[mPreToastRenderFuncCount] = func;
            mPreToastRenderFuncCaptures[mPreToastRenderFuncCount] = capture;
            mPreToastRenderFuncCount++;
        }
    }

    // hardware SPI
    _CCDisplay(CCAdafruitSSD1306 &display) : mDisplay(display)
    {
    }

    void Init(AppSettings *appSettings, InputDelegator *input, IHudProvider *hud, const array_view<IDisplayApp *> &apps)
    {
        mAppSettings = appSettings;
        mInput = input;
        mApps = apps;
        mHudProvider = hud;

        // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
        mDisplay.begin(SSD1306_SWITCHCAPVCC);
        mIsSetup = true;

        // fix fonts up a bit
        for (auto &glyph : MatchupPro8pt7bGlyphs)
        {
            glyph.yOffset += 6;
        }
        for (auto &glyph : pixChicago4pt7bGlyphs)
        {
            glyph.yOffset += 9;
        }
        for (auto &glyph : TomThumbGlyphs)
        {
            glyph.yOffset += 6;
        }
        for (auto &glyph : Eighties8pt7bGlyphs)
        {
            glyph.yOffset += 6;
        }

        mDisplay.dim(mAppSettings->mDisplayDim);

        // welcome msg.
        ClearState();
        mDisplay.clearDisplay();
        mDisplay.println(gClarinoidVersion);
        mDisplay.display();

        for (size_t i = 0; i < mApps.mSize; ++i)
        {
            mApps.mData[i]->DisplayAppInit();
        }
    }

    virtual AppSettings *GetAppSettings() override
    {
        return mAppSettings;
    }
    virtual InputDelegator *GetInput() override
    {
        return mInput;
    }

    virtual void SelectApp(int n) override
    {
        n = RotateIntoRange(n, mApps.mSize);

        if (n == mCurrentAppIndex)
            return;

        mApps.mData[mCurrentAppIndex]->DisplayAppOnUnselected();
        mCurrentAppIndex = n;
        mApps.mData[mCurrentAppIndex]->DisplayAppOnSelected();
    }

    virtual void ScrollApps(int delta) override
    {
        SelectApp(mCurrentAppIndex + delta);
    }

    int mframe = 0;

    // splitting the entire "display actions" into sub tasks:
    // UpdateAndRenderTask which runs state updating and  renders to the DMA
    // DisplayTask which "uploads" to the device. This is a natural separation of things to give the task runner some
    // method of yielding.
    virtual void UpdateAndRenderTask() override
    {
        CCASSERT(this->mIsSetup);

        mToggleReader.Update(&mInput->mDisplayFontToggle);
        if (mToggleReader.IsNewlyPressed())
        {
            mCurrentFontIndex = RotateIntoRange(mCurrentFontIndex + 1, SizeofStaticArray(mGUIFonts));
        }

        IDisplayApp *pMenuApp = nullptr;

        if (mCurrentAppIndex < (int)mApps.mSize)
        {
            pMenuApp = mApps.mData[mCurrentAppIndex];

            if (!mFirstAppSelected)
            {
                mFirstAppSelected = true;
                pMenuApp->DisplayAppOnSelected();
            }

            pMenuApp->DisplayAppUpdate();
        }

        ClearState();
        mDisplay.clearDisplay();

        if (pMenuApp)
        {
            pMenuApp->DisplayAppRender();
        }

        // render pre-toast overlay handlers
        ClearState();
        for (size_t i = 0; i < mPreToastRenderFuncCount; i++)
        {
            mPreToastRenderFunc[i](mPreToastRenderFuncCaptures[i]);
        }

        ClearState();
        if (mIsShowingToast)
        {
            if (mToastTimer.ElapsedTime().ElapsedMillisI() >= TOAST_DURATION_MILLIS)
            {
                mIsShowingToast = false;
            }
            else
            {
                // render toast.
                SetupModal();
                mDisplay.print(mToastMsg);
                if (mToastRenderExtra)
                {
                    mToastRenderExtra(*this, mToastCapture);
                }
            }
        }

        ClearState();
        mHudProvider->IHudProvider_RenderHud(mDisplay.width(), mDisplay.height());

        auto s = mHudProvider->IHudProvider_GetHudTransientIndicator(this->mInput);
        // if (s.length() > 0)
        if (!pMenuApp || pMenuApp->AllowOverlayIndicators())
        {
            ClearState();
            // int16_t x, y;
            // uint16_t w, h;
            // mDisplay.getTextBounds(s, 0, 0, &x, &y, &w, &h);
            // x = mDisplay.width() - w; // x is where the text will appear
            // mDisplay.fillRect(x - 1 /*start rect 1 px left*/, y, w + 2, h + 2, SSD1306_WHITE);
            //  mDisplay.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
            //  mDisplay.setCursor(x, 1); // y + 2

            String chips[10];
            size_t chipCount = 0;

            if (s.length() > 0)
            {
                int16_t x = 0;
                DrawChip(s, x, 0);
            }

            if (this->mInput->mFine.CurrentValue())
            {
                chips[chipCount++] = "F";
            }
            if (this->mInput->mCourse.CurrentValue())
            {
                chips[chipCount++] = "C";
            }

            // place cursor above hud, draw chips.
            int16_t xPos = 0;
            int16_t yPos = mDisplay.height() - mHudProvider->IHudProvider_GetHudHeight() - 10;

            for (size_t i = 0; i < chipCount; i++)
            {
                DrawChip(chips[i], xPos, yPos);
            }

            // mDisplay.print(s);
        }
    }

    void DrawChip(const String &text, int16_t &xPos, int16_t yPos)
    {
        int16_t x, y;
        uint16_t w, h;
        mDisplay.getTextBounds(text, 0, 0, &x, &y, &w, &h);
        // draw rounded rect filled with white, draw text in black over it.
        mDisplay.fillRect(xPos - 1 /*start rect 1 px left*/, yPos, w + 2, h + 2, SSD1306_WHITE);
        mDisplay.setCursor(xPos, yPos + 1);
        mDisplay.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        mDisplay.print(text);
        xPos += w + 6;
    }

    virtual int16_t GetHudHeight() const override
    {
        return mHudProvider->IHudProvider_GetHudHeight();
    }

    virtual int16_t GetClientHeight() const override
    {
        return mDisplay.height() - GetHudHeight();
    }
    virtual RectI GetClientRect() const override
    {
        return RectI::Construct(0, 0, mDisplay.width(), GetClientHeight());
    }

    // calculates in general, not for a specific location on screen.
    virtual RectI GetTextBounds(const String &str) override
    {
        int16_t x, y;
        uint16_t h, w;
        mDisplay.getTextBounds(str, 0, 0, &x, &y, &w, &h);
        return RectI::Construct(x, y, w, h);
    }

    virtual void PrintInvertedText(const String &str, bool isInverted = true) override
    {
        PrintInvertedText(str, RectI::Construct(0, 0, 0, 0), isInverted);
    }

    virtual void PrintInvertedText(const String &str, const RectI &padding, bool isInverted = true) override
    {
        if (isInverted)
        {
            int16_t x = mDisplay.getCursorX() - padding.x;
            int16_t y = mDisplay.getCursorY() - padding.y;
            int16_t w = 0;
            int16_t h = 0;
            {
                int16_t tx, ty;
                uint16_t tw, th;
                mDisplay.getTextBounds(str, mDisplay.getCursorX(), mDisplay.getCursorY(), &tx, &ty, &tw, &th);
                w = tw + padding.width + padding.x;
                h = th + padding.height + padding.y;
            }
            mDisplay.fillRect(x, y, w, h, SSD1306_WHITE);
            mDisplay.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        }
        else
        {
            mDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
        }

        mDisplay.print(str);
    }

    virtual void PrintInvertedLine(const String &str, bool isInverted = true) override
    {
        PrintInvertedText(str, isInverted);
        mDisplay.println();
    }

    virtual void DrawSelectionRect(const RectI &z) override
    {
        mDisplay.DrawMarchingAntsRectOutline(
            1, 3, 1, z.x, z.y, z.width, z.height, (micros() / (1000 * 120)), AntStyle::Chasing, Edges::All);
    }

    virtual void DisplayTask() override
    {
        mDisplay.mFrameCount++;
        mDisplay.display();
    }

    Stopwatch mToastTimer;
    bool mIsShowingToast = false;
    String mToastMsg;
    void *mToastCapture = nullptr;
    cc::function<void(struct IDisplay &, void *)>::ptr_t mToastRenderExtra = nullptr;

    size_t mCurrentFontIndex = 0;

    // NB: CHANGING ANYTHING IN HERE, ALSO CHANGE
    // SelectTinyFont().
    GFXfont const *mGUIFonts[5] = {
        &MatchupPro8pt7b,
        nullptr,
        &Eighties8pt7b,
        &pixChicago4pt7b,
        &TomThumb,
    };

    virtual void SelectTinyFont() override
    {
        mDisplay.setFont(mGUIFonts[4]);
    }

    virtual void SelectEightiesFont() override
    {
        mDisplay.setFont(mGUIFonts[2]);
    }

    virtual void SelectNormalFont() override
    {
        mDisplay.setFont(mGUIFonts[mCurrentFontIndex]);
    }

    SwitchControlReader mToggleReader;

    virtual void ShowToast(const String &msg,
                           void *capture = nullptr,
                           cc::function<void(IDisplay &, void *)>::ptr_t renderExtra = nullptr) override
    {
        mIsShowingToast = true;
        mToastMsg = msg;
        mToastCapture = capture;
        mToastRenderExtra = renderExtra;
        mToastTimer.Restart();
    }

    virtual int ClippedAreaHeight() const override
    {
        return mDisplay.mClipBottom - mDisplay.mClipTop;
    }

    virtual void ResetClip() override
    {
        mDisplay.SetClipRect(0, 0, mDisplay.width(), GetClientHeight());
    }

    virtual void SetClipRect(const RectI &rc) override
    {
        mDisplay.SetClipRect(rc.x, rc.y, rc.right(), rc.bottom());
    }

    virtual void ClipToMargin(int m) override
    {
        mDisplay.SetClipRect(m, m, mDisplay.width() - m, GetClientHeight() - m);
    }

    virtual void ClearState() override
    {
        mDisplay.setFont(mGUIFonts[mCurrentFontIndex]);
        mDisplay.mSolidText = true;
        mDisplay.mTextLeftMargin = 0;
        mDisplay.setTextWrap(true);
        ResetClip();
        mDisplay.setTextSize(1);
        mDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
        mDisplay.setCursor(0, 0);
    }

    virtual void DrawBitmap(PointI pos, const BitmapSpec &bmp) override
    {
        mDisplay.drawBitmap(pos.x, pos.y, bmp.pBmp, bmp.widthPixels, bmp.heightPixels, SSD1306_WHITE);
    }

    virtual void fillPie(const PointF &origin,
                         float radius,
                         float angleStart,
                         float angleSweep,
                         bool filled = true) override
    {
        float a0, a1;
        if (angleSweep >= 0)
        {
            a0 = angleStart;
            a1 = a0 + angleSweep;
        }
        else
        {
            a0 = angleStart + angleSweep;
            a1 = angleStart;
        }
        ::clarinoid::PieData pd;
        if (filled)
        {
            pd = ::clarinoid::fillPie(origin.x, origin.y, radius, a0, a1, [&](int x, int y, bool line) {
                if (!line && ((x + y) & 1))
                    return;
                mDisplay.drawPixel(x, y, SSD1306_WHITE);
            });
        }
        else
        {
            pd = ::clarinoid::fillPie(origin.x, origin.y, radius, a0, a1, [&](int x, int y, bool line) {
                if (line || (((x * 2 + y) % 4) == 1))
                {
                    mDisplay.drawPixel(x, y, SSD1306_WHITE);
                }
            });
        }
        if (angleSweep >= 0)
        {
            drawLine(origin.x, origin.y, origin.x + pd.p0.x, origin.y + pd.p0.y, [&](int x, int y, bool) {
                mDisplay.drawPixel(x, y, SSD1306_WHITE);
            });
        }
        else
        {
            drawLine(origin.x, origin.y, origin.x + pd.p1.x, origin.y + pd.p1.y, [&](int x, int y, bool) {
                mDisplay.drawPixel(x, y, SSD1306_WHITE);
            });
        }
    }

    // draws & prepares the screen for a modal message. after this just print text whatever.
    // returns the client area of the modal
    virtual RectI SetupModal(/*int pad = 1, int rectStart = 2, int textStart = 4*/) override
    {
        const int pad = 1;
        const int rectStart = 2;
        const int textStart = 4;
        ClearState();
        mDisplay.fillRect(pad, pad, mDisplay.width() - pad, GetClientHeight() - pad, SSD1306_BLACK);
        mDisplay.drawRect(
            rectStart, rectStart, mDisplay.width() - rectStart, GetClientHeight() - rectStart, SSD1306_WHITE);
        mDisplay.mTextLeftMargin = textStart;
        ClipToMargin(textStart);
        mDisplay.setCursor(textStart, textStart);
        auto ret = RectI::Construct(textStart, textStart, mDisplay.width() - textStart, GetClientHeight() - textStart);
        return ret;
    }

    virtual void FillRect2Pt(int16_t p1x, int16_t p1y, int16_t p2x, int16_t p2y, uint16_t color) override
    {
        // account for the fact that p1 and p2 may not be in the correct order.
        int16_t x1 = min(p1x, p2x);
        int16_t x2 = max(p1x, p2x);
        int16_t y1 = min(p1y, p2y);
        int16_t y2 = max(p1y, p2y);
        mDisplay.fillRect(x1, y1, x2 - x1, y2 - y1, color);
    }

    // required for IDisplay.
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override
    {
        mDisplay.fillRect(x, y, w, h, color);
    }
    virtual void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) override
    {
        mDisplay.fillCircle(x0, y0, r, color);
    }
    virtual int16_t width() const override
    {
        return mDisplay.width();
    }
    virtual int16_t height() const override
    {
        return mDisplay.height();
    }
    virtual int16_t getCursorX() const override
    {
        return mDisplay.getCursorX();
    }
    virtual int16_t getCursorY() const override
    {
        return mDisplay.getCursorY();
    }
    virtual void SetFontScale(int sx, int sy) override
    {
        mDisplay.setTextSize(sx, sy);
    }
    virtual void setTextWrap(bool w) override
    {
        mDisplay.setTextWrap(w);
    }
    virtual void setTextColor(uint16_t c) override
    {
        mDisplay.setTextColor(c);
    }
    virtual void setCursor(int16_t x, int16_t y) override
    {
        mDisplay.setCursor(x, y);
    }
    virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override
    {
        mDisplay.drawFastVLine(x, y, h, color);
    }
    virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override
    {
        mDisplay.drawFastHLine(x, y, w, color);
    }
    virtual void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color) override
    {
        mDisplay.fillRoundRect(x0, y0, w, h, radius, color);
    }
    virtual void print(const String &s) override
    {
        mDisplay.print(s);
    }
    virtual void println(const String &s) override
    {
        mDisplay.println(s);
    }

    virtual uint16_t GetLineHeight() const override
    {
        return mDisplay.GetLineHeight();
    }
    virtual void SetClipRect(int left, int top, int right, int bottom) override
    {
        mDisplay.SetClipRect(left, top, right, bottom);
    }
    virtual void SetTextSolid(bool b) override
    {
        mDisplay.mSolidText = b;
    }
    virtual bool GetTextSolid() override
    {
        return mDisplay.mSolidText;
    }
    virtual int GetTextLeftMargin() override
    {
        return mDisplay.mTextLeftMargin;
    }
    virtual void SetTextLeftMargin(int m) override
    {
        mDisplay.mTextLeftMargin = m;
    }
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) override
    {
        mDisplay.drawPixel(x, y, color);
    }
    virtual void fillScreen(uint16_t color) override
    {
        mDisplay.fillScreen(color);
    }
    virtual void DrawDottedRect(int16_t left, int16_t top, int16_t width, int16_t height, uint16_t color) override
    {
        mDisplay.DrawDottedRect(left, top, width, height, color);
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
        mDisplay.DrawMarchingAntsFilledRect(AntSize, AntMask, ySign, xSign, xstart, ystart, w, h, variation);
    }
    virtual void DrawDottedHLine(int16_t left, int16_t width, int16_t y, uint16_t color) override
    {
        mDisplay.DrawDottedHLine(left, width, y, color);
    }
    virtual void dim(bool d) override
    {
        mDisplay.dim(d);
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

    IDitherMatrix &matrix = gBayer8x8Matrix;

    virtual void SetPixel(const PointI &pt, uint16_t color) override
    {
        mDisplay.drawPixel(pt.x, pt.y, color);
    }

    virtual bool IsInBounds(const PointI &pt) const override
    {
        // return pt.x >= 0 && pt.x < mDisplay.width() && pt.y >= 0 && pt.y < mDisplay.height();
        //  consider the clip rect.
        return pt.x >= mDisplay.mClipLeft && pt.x < mDisplay.mClipRight && pt.y >= mDisplay.mClipTop &&
               pt.y < mDisplay.mClipBottom;
    }

    virtual void SetPixelShaded(const PointI &pt, int coverageQp8) override
    {
        if (!IsInBounds(pt))
        {
            return;
        }

        coverageQp8 = ClampInclusive(coverageQp8, 0, 255);
        SetPixel(pt, matrix.getDitheredColor(coverageQp8, pt));
    }

    /// Draws a horizontal line of 'length' pixels starting at (x, y),
    /// dithering each pixel according to 'brightness' and the DitherMatrix.
    virtual void DrawHLineDithered(const PointI &pt, int length, int brightnessQp8) override
    {
        if (length <= 0)
            return;
        for (int i = 0; i < length; i++)
        {
            int currentX = pt.x + i;
            PointI currentPt{currentX, pt.y};
            SetPixelShaded(currentPt, brightnessQp8);
        }
    }

    virtual void FillRectWithBrightness(const RectI &rc, int brightnessQp8) override
    {
        brightnessQp8 = ClampInclusive(brightnessQp8, 0, 255);
        for (int row = 0; row < rc.height; row++)
        {
            int currentY = rc.y + row;
            for (int col = 0; col < rc.width; col++)
            {
                int currentX = rc.x + col;
                PointI currentPt{currentX, currentY};
                SetPixelShaded(currentPt, brightnessQp8);
            }
        }
    }

    // finds the two points in 'points' that are farthest apart. supports only up to 4 points due to O(n^2) complexity.
    // assumes at least 1 point.
    virtual void FindFarthestPair(const PointI *points, size_t pointCount, PointI &bestA, PointI &bestB) override
    {
        bestA = points[0];
        bestB = points[0];
        float maxDistSq = 0;

        // Simple O(n^2) check (fine for up to 4 points):
        for (size_t i = 0; i < pointCount; i++)
        {
            for (size_t j = i + 1; j < pointCount; j++)
            {
                float dx = points[i].x - points[j].x;
                float dy = points[i].y - points[j].y;
                float distSq = dx * dx + dy * dy;
                if (distSq > maxDistSq)
                {
                    maxDistSq = distSq;
                    bestA = points[i];
                    bestB = points[j];
                }
            }
        }
    }

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
    virtual void DrawLineWithBrightness(const PointI &pt0, const PointI &pt1, int brightnessQp8) override
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

        while (true)
        {
            SetPixelShaded(PointI{x, y}, brightnessQp8);

            if (x == pt1.x && y == pt1.y)
                break;

            int e2 = 2 * err;
            if (e2 > -dy)
            {
                err -= dy;
                x += sx;
            }
            else if (e2 < dx)
            { // Use `else if` to ensure only one step occurs
                err += dx;
                y += sy;
            }
        }
    }

    virtual void DrawLine(const PointI &pt0, const PointI &pt1) override
    {
        return mDisplay.drawLine(pt0.x, pt0.y, pt1.x, pt1.y, SSD1306_WHITE);
    }

    // Specialized function:
    //   draws the infinite line through (x0,y0)-(x1,y1),
    //   clipped to [clipLeft..clipRight] x [clipTop..clipBottom].
    virtual void DrawInfiniteLineClipped(const PointI &pt0,
                                         const PointI &pt1,
                                         const RectI &clipRect,
                                         int brightness) override
    {
        // 1) Handle trivial cases: vertical / horizontal
        if (pt0.x == pt1.x)
        {
            // Vertical line x=x0
            if (pt0.x < clipRect.left() || pt0.x > clipRect.right())
                return; // outside
            int yStart = clipRect.top();
            int yEnd = clipRect.bottom();
            // Just draw from (x0, yStart) to (x0, yEnd)
            DrawLineWithBrightness(PointI{pt0.x, yStart}, PointI{pt0.x, yEnd}, brightness);
            return;
        }

        if (pt0.y == pt1.y)
        {
            // Horizontal line y=y0
            if (pt0.y < clipRect.top() || pt0.y > clipRect.bottom())
                return; // outside
            int xStart = clipRect.left();
            int xEnd = clipRect.right();
            // Draw from (xStart, y0) to (xEnd, y0)
            DrawLineWithBrightness(PointI{xStart, pt0.y}, PointI{xEnd, pt0.y}, brightness);
            return;
        }

        // 2) General case
        float slope = (float)(pt1.y - pt0.y) / (float)(pt1.x - pt0.x);

        // We'll track up to 4 intersection candidates:
        PointI candidates[4];
        size_t candidateCount = 0;

        // Intersection at x=clipLeft => y = y0 + slope*(clipLeft - x0)
        float yLeft = pt0.y + slope * (clipRect.left() - pt0.x);
        if (yLeft >= clipRect.top() && yLeft <= clipRect.bottom())
        {
            candidates[candidateCount++] = PointI{clipRect.left(), (int)yLeft};
        }

        float yRight = pt0.y + slope * (clipRect.right() - pt0.x);
        if (yRight >= clipRect.top() && yRight <= clipRect.bottom())
        {
            candidates[candidateCount++] = PointI{clipRect.right(), (int)yRight};
        }

        // Intersection at y=clipTop => x = x0 + (clipTop - y0)/slope
        float xTop = pt0.x + (clipRect.top() - pt0.y) / slope;
        if (xTop >= clipRect.left() && xTop <= clipRect.right())
        {
            candidates[candidateCount++] = PointI{(int)xTop, clipRect.top()};
        }

        // Intersection at y=clipBottom => x = x0 + (clipBottom - y0)/slope
        float xBottom = pt0.x + (clipRect.bottom() - pt0.y) / slope;
        if (xBottom >= clipRect.left() && xBottom <= clipRect.right())
        {
            candidates[candidateCount++] = PointI{(int)xBottom, clipRect.bottom()};
        }

        // Remove duplicates or near-duplicates if they happen (optional).
        // If there's no valid intersection => no draw
        if (candidateCount < 2)
            return;

        // We only need two extremes. Possibly we have more if the line hits exactly a corner, etc.
        // Let's pick the two that are farthest from each other:
        //  (One way: measure the bounding box in that set.)
        float minX = 1e6;
        float minY = 1e6;
        float maxX = -1e6;
        float maxY = -1e6;

        for (size_t i = 0; i < candidateCount; i++)
        {
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
        PointI pA, pB;
        FindFarthestPair(candidates, candidateCount, pA, pB);
        DrawLineWithBrightness(pA, pB, brightness);
    }

    // draw circle filled with brightness
    /// <summary>
    /// Fills a circle of radius r centered at (x0,y0) with the given brightness,
    /// using a DitherMatrix for 1-bit dithering. No floating-point is used.
    /// </summary>
    virtual void FillCircleWithBrightness(const PointI &c, int r, int brightnessQp8) override
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
        DrawHLineDithered({c.x - r, c.y}, (2 * r + 1), brightnessQp8);

        // Also fill the symmetrical horizontal lines above & below
        // for each step of x,y as we move around the circle edges
        while (x < y)
        {
            // If f >= 0, move y inward
            if (f >= 0)
            {
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

            DrawHLineDithered({c.x - x, c.y + y}, (2 * x + 1), brightnessQp8);
            if (y != 0) // if y=0, top & bottom would be same line
            {
                DrawHLineDithered({c.x - x, c.y - y}, (2 * x + 1), brightnessQp8);
            }

            // For x != y, fill those "side" lines near (y,x) due to circle symmetry:
            //   left side at x0 - y .. x0 + y, top y= y0 + x
            //   left side at x0 - y .. x0 + y, bottom y= y0 - x
            if (x != y)
            {
                DrawHLineDithered({c.x - y, c.y + x}, (2 * y + 1), brightnessQp8);
                if (x != 0) // if x=0, same line repeated
                {
                    DrawHLineDithered({c.x - y, c.y - x}, (2 * y + 1), brightnessQp8);
                }
            }
        }
    }
};

} // namespace clarinoid
