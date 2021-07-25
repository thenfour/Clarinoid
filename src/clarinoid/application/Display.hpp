
// This is basically a clarinoid-specific "controller" for a display.

#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include "clarinoid/components/AdafruitSSD1366Wrapper.hpp"
#include <clarinoid/application/ControlMapper.hpp>

#include "clarinoid/application/Font/matchup8.hpp"
#include "clarinoid/application/Font/chicago4px7b.hpp"
#include "clarinoid/application/Font/TomThumb.hpp"
#include "clarinoid/application/Font/eighties8.hpp"

namespace clarinoid
{

static constexpr int TOAST_DURATION_MILLIS = 1700;

//////////////////////////////////////////////////////////////////////
struct IHudProvider
{
    virtual int16_t IHudProvider_GetHudHeight() = 0;
    virtual void IHudProvider_RenderHud(int16_t displayWidth, int16_t displayHeight) = 0;
};

//////////////////////////////////////////////////////////////////////
struct IDisplayApp
{
    virtual void DisplayAppInit()
    {
    }
    virtual void DisplayAppOnSelected()
    {
    }
    virtual void DisplayAppOnUnselected()
    {
    }
    virtual void DisplayAppUpdate() = 0; // called to update internal state.
    virtual void DisplayAppRender() = 0; // called to render to display.
    virtual const char *DisplayAppGetName() = 0;
};

//////////////////////////////////////////////////////////////////////
struct CCDisplay
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
    CCDisplay(uint8_t w,
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
        // Serial.println(String("Display ctor, control mapper = ") + ((uintptr_t)(&mControlMapper)));
    }

    // bit-bang
    CCDisplay(uint8_t w, uint8_t h, int8_t mosi_pin, int8_t sclk_pin, int8_t dc_pin, int8_t rst_pin, int8_t cs_pin)
        : mDisplay(w, h, mosi_pin, sclk_pin, dc_pin, rst_pin, cs_pin)
    {
        // Serial.println(String("Display ctor, control mapper = ") + ((uintptr_t)(&mControlMapper)));
        // Init();
    }

    void Init(AppSettings *appSettings, InputDelegator *input, IHudProvider *hud, const array_view<IDisplayApp *> &apps)
    {
        mAppSettings = appSettings;
        mInput = input;
        mApps = apps;
        mHudProvider = hud;

        // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
        // Serial.println(String("Display init"));
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

        pfnCrashHandler = []() {
            Serial.println(gCrashMessage);
            Serial.println(String("Display ptr = ") + (uintptr_t)gDisplay);
            // why doesn't this work???
            // gDisplay->clearDisplay();
            // gDisplay->mSolidText = true;
            // gDisplay->mTextLeftMargin = 0;
            // gDisplay->ResetClip();
            // gDisplay->setTextSize(1);
            // gDisplay->setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
            // gDisplay->setCursor(0,0);
            // gDisplay->println(gCrashMessage);
            // gDisplay->display();
            DefaultCrashHandler();
        };

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

    void SelectApp(int n)
    {
        n = RotateIntoRange(n, mApps.mSize);

        if (n == mCurrentAppIndex)
            return;

        mApps.mData[mCurrentAppIndex]->DisplayAppOnUnselected();
        mCurrentAppIndex = n;
        mApps.mData[mCurrentAppIndex]->DisplayAppOnSelected();
    }

    void ScrollApps(int delta)
    {
        SelectApp(mCurrentAppIndex + delta);
    }

    int mframe = 0;

    // splitting the entire "display actions" into sub tasks:
    // UpdateAndRenderTask which runs state updating and  renders to the DMA
    // DisplayTask which "uploads" to the device. This is a natural separation of things to give the task runner some
    // method of yielding.
    void UpdateAndRenderTask()
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

    int16_t GetHudHeight() const
    {
        return mHudProvider->IHudProvider_GetHudHeight();
    }

    int16_t GetClientHeight() const
    {
        return mDisplay.height() - GetHudHeight();
    }

    void DrawInvertedText(const String &str, bool isInverted = true)
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

    void DrawInvertedLine(const String &str, bool isInverted = true)
    {
        DrawInvertedText(str, isInverted);
        mDisplay.println();
    }

    void DrawSelectionRect(const RectI &rc)
    {
        mDisplay.DrawDottedRectOutlineWithGlobalParity(rc.x, rc.y, rc.width, rc.height, !!((micros() / (1000 * 150)) & 1));
    }


    void DisplayTask()
    {
        NoInterrupts _ni;
        mDisplay.display();
    }

    Stopwatch mToastTimer;
    bool mIsShowingToast = false;
    String mToastMsg;
    size_t mCurrentFontIndex = 0;

    GFXfont const *mGUIFonts[4] = {
        &MatchupPro8pt7b,
        //&Eighties8pt7b,
        nullptr,
        &pixChicago4pt7b,
        &TomThumb,
    };

    SwitchControlReader mToggleReader;

    void ShowToast(const String &msg)
    {
        mIsShowingToast = true;
        mToastMsg = msg;
        mToastTimer.Restart();
    }

    int ClippedAreaHeight() const
    {
        return mDisplay.mClipBottom - mDisplay.mClipTop;
    }

    void ResetClip()
    {
        mDisplay.SetClipRect(0, 0, mDisplay.width(), GetClientHeight());
    }

    void SetClipRect(const RectI &rc)
    {
        mDisplay.SetClipRect(rc.x, rc.y, rc.right(), rc.bottom());
    }

    void ClipToMargin(int m)
    {
        mDisplay.SetClipRect(m, m, mDisplay.width() - m, GetClientHeight() - m);
    }

    void ClearState()
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

    // draws & prepares the screen for a modal message. after this just print text whatever.
    inline void SetupModal(int pad = 1, int rectStart = 2, int textStart = 4)
    {
        ClearState();
        mDisplay.fillRect(pad, pad, mDisplay.width() - pad, GetClientHeight() - pad, SSD1306_BLACK);
        mDisplay.drawRect(
            rectStart, rectStart, mDisplay.width() - rectStart, GetClientHeight() - rectStart, SSD1306_WHITE);
        mDisplay.mTextLeftMargin = textStart;
        ClipToMargin(textStart);
        mDisplay.setCursor(textStart, textStart);
    }
};

CCAdafruitSSD1306 *CCDisplay::gDisplay = nullptr;

} // namespace clarinoid
