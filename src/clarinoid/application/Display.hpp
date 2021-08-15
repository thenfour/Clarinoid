
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

static constexpr int TOAST_DURATION_MILLIS = 1700;

//////////////////////////////////////////////////////////////////////
struct _CCDisplay : IDisplay
{
  private:
    static CCAdafruitSSD1306
        *gDisplay; // this is only to allow the crash handler to output to the screen. not for app use in general.

  public:
    CCAdafruitSSD1306 mDisplay;

    AppSettings *mAppSettings = nullptr;
    InputDelegator *mInput = nullptr;
    IHudProvider *mHudProvider = nullptr;

    bool mIsSetup = false; // used for crash handling to try and setup this if we can
    bool mFirstAppSelected = false;

    array_view<IDisplayApp *> mApps;
    int mCurrentAppIndex = 0;

    // hardware SPI
    _CCDisplay(uint8_t w,
               uint8_t h,
               SPIClass *spi,
               int8_t dc_pin,
               int8_t rst_pin,
               int8_t cs_pin,
               uint32_t bitrate = 8000000UL)
        : mDisplay(w,
                   h,
                   spi,
                   dc_pin,
                   rst_pin,
                   cs_pin,
                   bitrate) // 128, 64, &SPI, 9/*DC*/, 8/*RST*/, 10/*CS*/, 44 * 1000000UL)
    {
    }

    // bit-bang
    _CCDisplay(uint8_t w, uint8_t h, int8_t mosi_pin, int8_t sclk_pin, int8_t dc_pin, int8_t rst_pin, int8_t cs_pin)
        : mDisplay(w, h, mosi_pin, sclk_pin, dc_pin, rst_pin, cs_pin)
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

        gDisplay = &mDisplay;

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

        // pfnCrashHandler = []() {
        //     Serial.println(gCrashMessage);
        //     Serial.println(String("Display ptr = ") + (uintptr_t)gDisplay);
        //     // why doesn't this work???
        //     // gDisplay->clearDisplay();
        //     // gDisplay->mSolidText = true;
        //     // gDisplay->mTextLeftMargin = 0;
        //     // gDisplay->ResetClip();
        //     // gDisplay->setTextSize(1);
        //     // gDisplay->setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
        //     // gDisplay->setCursor(0,0);
        //     // gDisplay->println(gCrashMessage);
        //     // gDisplay->display();
        //     DefaultCrashHandler();
        // };

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
            }
        }

        ClearState();
        mHudProvider->IHudProvider_RenderHud(mDisplay.width(), mDisplay.height());

        String s = "";

        if (this->mInput->mModifierFine.CurrentValue())
        {
            s += "F ";
        }
        if (this->mInput->mModifierCourse.CurrentValue())
        {
            s += "C ";
        }
        if (this->mInput->mModifierShift.CurrentValue())
        {
            s += "Sh ";
        }

        if (s.length() > 0)
        {
            ClearState();
            int16_t x, y;
            uint16_t w, h;
            mDisplay.getTextBounds(s, 0, 0, &x, &y, &w, &h);
            x = mDisplay.width() - w; // x is where the text will appear
            mDisplay.fillRect(x - 1 /*start rect 1 px left*/, y, w + 2, h + 2, SSD1306_WHITE);
            mDisplay.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
            mDisplay.setCursor(x, 1); // y + 2
            mDisplay.print(s);
        }
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
        if (isInverted)
        {
            int16_t x, y;
            uint16_t w, h;
            mDisplay.getTextBounds(str, mDisplay.getCursorX(), mDisplay.getCursorY(), &x, &y, &w, &h);
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

    virtual void ShowToast(const String &msg) override
    {
        mIsShowingToast = true;
        mToastMsg = msg;
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
};

CCAdafruitSSD1306 *_CCDisplay::gDisplay = nullptr; // for debug!

} // namespace clarinoid
