

#pragma once

#include <clarinoid/basic/assert.hpp>

struct Edges
{
    using Flags = uint8_t;
    static constexpr Flags Left = 1, Top = 2, Right = 4, Bottom = 8,

                           All = 0xff;
};

enum struct AntStyle
{
    Continuous,
    Chasing,
};

#ifdef CLARINOID_MODULE_TEST

/// fit into the SSD1306_ naming scheme
#define SSD1306_BLACK 0   ///< Draw 'off' pixels
#define SSD1306_WHITE 1   ///< Draw 'on' pixels
#define SSD1306_INVERSE 2 ///< Invert pixels

#define SSD1306_MEMORYMODE 0x20          ///< See datasheet
#define SSD1306_COLUMNADDR 0x21          ///< See datasheet
#define SSD1306_PAGEADDR 0x22            ///< See datasheet
#define SSD1306_SETCONTRAST 0x81         ///< See datasheet
#define SSD1306_CHARGEPUMP 0x8D          ///< See datasheet
#define SSD1306_SEGREMAP 0xA0            ///< See datasheet
#define SSD1306_DISPLAYALLON_RESUME 0xA4 ///< See datasheet
#define SSD1306_DISPLAYALLON 0xA5        ///< Not currently used
#define SSD1306_NORMALDISPLAY 0xA6       ///< See datasheet
#define SSD1306_INVERTDISPLAY 0xA7       ///< See datasheet
#define SSD1306_SETMULTIPLEX 0xA8        ///< See datasheet
#define SSD1306_DISPLAYOFF 0xAE          ///< See datasheet
#define SSD1306_DISPLAYON 0xAF           ///< See datasheet
#define SSD1306_COMSCANINC 0xC0          ///< Not currently used
#define SSD1306_COMSCANDEC 0xC8          ///< See datasheet
#define SSD1306_SETDISPLAYOFFSET 0xD3    ///< See datasheet
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5  ///< See datasheet
#define SSD1306_SETPRECHARGE 0xD9        ///< See datasheet
#define SSD1306_SETCOMPINS 0xDA          ///< See datasheet
#define SSD1306_SETVCOMDETECT 0xDB       ///< See datasheet

#define SSD1306_SETLOWCOLUMN 0x00  ///< Not currently used
#define SSD1306_SETHIGHCOLUMN 0x10 ///< Not currently used
#define SSD1306_SETSTARTLINE 0x40  ///< See datasheet

#define SSD1306_EXTERNALVCC 0x01  ///< External display voltage source
#define SSD1306_SWITCHCAPVCC 0x02 ///< Gen. display voltage from 3.3V

#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26              ///< Init rt scroll
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27               ///< Init left scroll
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29 ///< Init diag scroll
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A  ///< Init diag scroll
#define SSD1306_DEACTIVATE_SCROLL 0x2E                    ///< Stop scroll
#define SSD1306_ACTIVATE_SCROLL 0x2F                      ///< Start scroll
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3             ///< Set scroll range

/// Font data stored PER GLYPH
typedef struct
{
    uint16_t bitmapOffset; ///< Pointer into GFXfont->bitmap
    uint8_t width;         ///< Bitmap dimensions in pixels
    uint8_t height;        ///< Bitmap dimensions in pixels
    uint8_t xAdvance;      ///< Distance to advance cursor (x axis)
    int8_t xOffset;        ///< X dist from cursor pos to UL corner
    int8_t yOffset;        ///< Y dist from cursor pos to UL corner
} GFXglyph;

/// Data stored for FONT AS A WHOLE
typedef struct
{
    uint8_t *bitmap;  ///< Glyph bitmaps, concatenated
    GFXglyph *glyph;  ///< Glyph array
    uint16_t first;   ///< ASCII extents (first char)
    uint16_t last;    ///< ASCII extents (last char)
    uint8_t yAdvance; ///< Newline distance (y axis)
} GFXfont;

struct SPIClass;

struct CCAdafruitSSD1306 : Print
{

        CCAdafruitSSD1306(uint8_t w,
                      uint8_t h,
                      SPIClass *spi,
                      int8_t dc_pin,
                      int8_t rst_pin,
                      int8_t cs_pin,
                      uint32_t bitrate = 8000000UL)
    {
    }


    bool begin(uint8_t switchvcc = SSD1306_SWITCHCAPVCC,
               uint8_t i2caddr = 0,
               bool reset = true,
               bool periphBegin = true)
    {
        return true;
    }

    void dim(bool dim)
    {
    }

    void clearDisplay(void)
    {
    }

    void display(void)
    {
    }
    int16_t width(void) const
    {
        return 0;
    };

    int16_t height(void) const
    {
        return 0;
    }

    uint16_t GetLineHeight() const
    {
        return 0;
    }

    void drawPixel(int16_t x, int16_t y, uint16_t color)
    {
    }
    virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
    {
    }
    virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
    {
    }
    void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
    {
    }
    void getTextBounds(const __FlashStringHelper *s,
                       int16_t x,
                       int16_t y,
                       int16_t *x1,
                       int16_t *y1,
                       uint16_t *w,
                       uint16_t *h)
    {
    }
    void getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
    {
    }
    void setTextSize(uint8_t s)
    {
    }
    void setTextSize(uint8_t sx, uint8_t sy)
    {
    }
    void setFont(const GFXfont *f = NULL)
    {
    }
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
    {
    }
    void setTextColor(uint16_t c)
    {
    }
    void setTextColor(uint16_t c, uint16_t c2)
    {
    }
    void setCursor(int16_t x, int16_t y)
    {
    }
    int16_t getCursorX(void) const
    {
        return 0;
    }
    int16_t getCursorY(void) const
    {
        return 0;
    };

    void DrawMarchingAntsFilledRect(int AntSize,
                                    int AntMask,
                                    int ySign,
                                    int xSign,
                                    int xstart,
                                    int ystart,
                                    int w,
                                    int h,
                                    int variation)
    {
    }

    void DrawMarchingAntsRectOutline(int LineWidth,
                                     int AntSize,
                                     int AntMask,
                                     int x,
                                     int y,
                                     int w,
                                     int h,
                                     int variation,
                                     AntStyle style,
                                     Edges::Flags edges)
    {
    }

    void DrawDottedRect(int16_t left, int16_t top, int16_t width, int16_t height, uint16_t color)
    {
    }

    void DrawDottedHLine(int16_t left, int16_t width, int16_t y, uint16_t color)
    {
    }

    void DrawDottedHLineWithGlobalParity(int xstart, int w, int y, bool variation)
    {
    }

    void DrawDottedVLineWithGlobalParity(int x, int ystart, int h, bool variation)
    {
    }

    void DrawDottedRectOutlineWithGlobalParity(int x, int y, int w, int h, bool variation)
    {
    }
    bool mSolidText = true;
    int mTextLeftMargin = 0;
    int mFrameCount = 0; // set by caller; display() is not a virtual fn

    void SetClipRect(int left, int top, int right, int bottom)
    {
        mClipLeft = left;
        mClipRight = right;
        mClipTop = top;
        mClipBottom = bottom;
    }

    int mClipLeft = 0;
    int mClipRight = 0;
    int mClipTop = 0;
    int mClipBottom = 0;

    void setTextWrap(bool w)
    {
    }

    void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color)
    {
    }

    void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color, uint16_t bg)
    {
    }

    void drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
    {
    }

    void drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg)
    {
    }
    virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
    {
    }
    void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color)
    {
    }
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
    {
    }
    virtual void fillScreen(uint16_t color)
    {
    }

    virtual size_t /*Adafruit_GFX::*/ write(uint8_t c) override
    {
        return 1;
    }
};

#else

// FROM Adafruit_GFX.cpp:

// Many (but maybe not all) non-AVR board installs define macros
// for compatibility with existing PROGMEM-reading AVR code.
// Do our own checks and defines here for good measure...

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif
#ifndef pgm_read_word
#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif
#ifndef pgm_read_dword
#define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#endif

// Pointers are a peculiar case...typically 16-bit on AVR boards,
// 32 bits elsewhere.  Try to accommodate both...

#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
#define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
#define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif

inline GFXglyph *pgm_read_glyph_ptr(const GFXfont *gfxFont, uint8_t c)
{
#ifdef __AVR__
    return &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
#else
    // expression in __AVR__ section may generate "dereferencing type-punned
    // pointer will break strict-aliasing rules" warning In fact, on other
    // platforms (such as STM32) there is no need to do this pointer magic as
    // program memory may be read in a usual way So expression may be simplified
    return gfxFont->glyph + c;
#endif //__AVR__
}

inline uint8_t *pgm_read_bitmap_ptr(const GFXfont *gfxFont)
{
#ifdef __AVR__
    return (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);
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
struct CCAdafruitSSD1306 :                          //
                           public Adafruit_SSD1306, //
                           clarinoid::IClarinoidCrashReportOutput
{
    CCAdafruitSSD1306(uint8_t w,
                      uint8_t h,
                      SPIClass *spi,
                      int8_t dc_pin,
                      int8_t rst_pin,
                      int8_t cs_pin,
                      uint32_t bitrate = 8000000UL)
        : Adafruit_SSD1306(w,
                           h,
                           spi,
                           dc_pin,
                           rst_pin,
                           cs_pin,
                           bitrate), // 128, 64, &SPI, 9/*DC*/, 8/*RST*/, 10/*CS*/, 44 * 1000000UL)
          mClipRight(w), mClipBottom(h)
    {
    }

    // #define OLED_MOSI   9
    // #define OLED_CLK   10
    // #define OLED_DC    11
    // #define OLED_CS    12
    // #define OLED_RESET 13
    // CCAdafruitSSD1306(uint8_t w,
    //                  uint8_t h,
    //                  int8_t mosi_pin,
    //                  int8_t sclk_pin,
    //                  int8_t dc_pin,
    //                  int8_t rst_pin,
    //                  int8_t cs_pin)
    //    : Adafruit_SSD1306(w, h, mosi_pin, sclk_pin, dc_pin, rst_pin, cs_pin), mClipRight(w), mClipBottom(h)
    //{
    //}

    virtual void IClarinoidCrashReportOutput_Init() override
    {
        begin(SSD1306_SWITCHCAPVCC);
    }

    virtual void IClarinoidCrashReportOutput_Blink() override
    {
        fillScreen(SSD1306_WHITE);
        display();
        delay(150);
        fillScreen(SSD1306_BLACK);
        display();
        delay(150);
    }
    virtual void IClarinoidCrashReportOutput_Print(const char *s) override
    {
        setTextColor(SSD1306_WHITE);
        setCursor(0, 0);
        clearDisplay();
        print(s);
        display();
    }

    bool mSolidText = true;
    int mTextLeftMargin = 0;
    int mFrameCount = 0; // set by caller; display() is not a virtual fn

    void SetClipRect(int left, int top, int right, int bottom)
    {
        mClipLeft = left;
        mClipRight = right;
        mClipTop = top;
        mClipBottom = bottom;
    }

    int mClipLeft = 0;
    int mClipRight = 0;
    int mClipTop = 0;
    int mClipBottom = 0;

    // for checker-style bool checking
    bool PixelParity(int16_t x, int16_t y) const
    {
        return (x & 1) != (y & 1);
    }

    bool IsWithinClipRect(int16_t x, int16_t y) const
    {
        if (x < mClipLeft)
            return false;
        if (x >= mClipRight)
            return false;
        if (y < mClipTop)
            return false;
        if (y >= mClipBottom)
            return false;
        return true;
    }

    // overriding Adafruit_GFX::writePixel. base forwards to drawPixel().
    virtual void writePixel(int16_t x, int16_t y, uint16_t color)
    {
        if (!IsWithinClipRect(x, y))
            return;
        if (!mSolidText)
        {
            if (!PixelParity(x, y))
            {
                // there are probably much more legible ways of graying text. this sorta destroys background color info;
                // we should probably instead set a bg/fg color before the text write op
                return;
            }
        }
        drawPixel(x, y, color);
    }

    void DrawDottedRect(int16_t left, int16_t top, int16_t width, int16_t height, uint16_t color)
    {
        for (int16_t y = top; y < top + height; ++y)
        {
            for (int16_t x = left; x < left + width; x += 2)
            {
                if (PixelParity(x, y))
                {
                    continue;
                }
                drawPixel(x, y, color);
            }
        }
    }

    void DrawDottedHLine(int16_t left, int16_t width, int16_t y, uint16_t color)
    {
        const int skip = 2;
        for (int16_t x = left; x < left + width; x += skip)
        {
            drawPixel(x, y, color);
        }
    }

    void DrawDottedHLineWithGlobalParity(int xstart, int w, int y, bool variation)
    {
        for (int16_t x = xstart; x < xstart + w; ++x)
        {
            if (variation ? ((x & 1) == (y & 1)) : ((x & 1) != (y & 1)))
                drawPixel(x, y, SSD1306_WHITE);
        }
    }

    void DrawDottedVLineWithGlobalParity(int x, int ystart, int h, bool variation)
    {
        for (int16_t y = ystart; y < ystart + h; ++y)
        {
            if (variation ? ((x & 1) == (y & 1)) : ((x & 1) != (y & 1)))
                drawPixel(x, y, SSD1306_WHITE);
        }
    }

    void DrawDottedRectOutlineWithGlobalParity(int x, int y, int w, int h, bool variation)
    {
        DrawDottedHLineWithGlobalParity(x, w, y, variation);
        DrawDottedHLineWithGlobalParity(x, w, y + h - 1, variation);
        DrawDottedVLineWithGlobalParity(x, y + 1, h - 2, variation);
        DrawDottedVLineWithGlobalParity(x + w - 1, y + 1, h - 2, variation);
    }

    // template <int AntSize, int AntMask, int ySign, int xSign>
    void DrawMarchingAntsFilledRect(int AntSize,
                                    int AntMask,
                                    int ySign,
                                    int xSign,
                                    int xstart,
                                    int ystart,
                                    int w,
                                    int h,
                                    int variation)
    {
        for (int y = ystart; y < ystart + h; ++y)
        {
            for (int x = xstart; x < xstart + w; ++x)
            {
                int p = abs(xSign * x + ySign * y - variation) % AntSize; // p is the position in the pattern.
                bool parity = !!(AntMask & (1 << p));
                drawPixel(x, y, parity ? SSD1306_WHITE : SSD1306_BLACK);
            }
        }
    }

    // template <int LineWidth, int AntSize, int AntMask>
    void DrawMarchingAntsRectOutline(int LineWidth,
                                     int AntSize,
                                     int AntMask,
                                     int x,
                                     int y,
                                     int w,
                                     int h,
                                     int variation,
                                     AntStyle style,
                                     Edges::Flags edges)
    {
        int n1 = style == AntStyle::Chasing ? -1 : 1;
        // draw rect INSIDE the given coords
        if (edges & Edges::Top)
            DrawMarchingAntsFilledRect(AntSize, AntMask, 1, 1, x, y, w, LineWidth, variation); // top rect
        if (edges & Edges::Bottom)
            DrawMarchingAntsFilledRect(
                AntSize, AntMask, 1, n1, x, y + h - LineWidth, w, LineWidth, variation); // bottom rect
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

    uint16_t GetLineHeight() const
    {
        if (gfxFont)
        {
            return textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
        }
        return textsize_y * 8;
    }

    /**************************************************************************/
    /*!
        @brief  Print one byte/character of data, used to support print()
        @param  c  The 8-bit ascii character to write
    */
    /**************************************************************************/
    size_t /*Adafruit_GFX::*/ write(uint8_t c)
    {
        if (!gfxFont)
        { // 'Classic' built-in font

            if (c == '\n')
            {                               // Newline?
                cursor_x = mTextLeftMargin; // Reset x to left margin
                cursor_y += textsize_y * 8; // advance y one line
            }
            else if (c != '\r')
            { // Ignore carriage returns
                if (wrap && ((cursor_x + textsize_x * 6) > _width))
                {                               // Off right?
                    cursor_x = mTextLeftMargin; // reset x to left margin
                    cursor_y += textsize_y * 8; // advance y one line
                }
                drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x, textsize_y);
                cursor_x += textsize_x * 6; // Advance x one char
            }
        }
        else
        { // Custom font

            if (c == '\n')
            {
                cursor_x = mTextLeftMargin;
                cursor_y += (int16_t)textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
            }
            else if (c != '\r')
            {
                uint8_t first = pgm_read_byte(&gfxFont->first);
                if ((c >= first) && (c <= (uint8_t)pgm_read_byte(&gfxFont->last)))
                {
                    GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c - first);
                    uint8_t w = pgm_read_byte(&glyph->width), h = pgm_read_byte(&glyph->height);
                    if ((w > 0) && (h > 0))
                    {                                                        // Is there an associated bitmap?
                        int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset); // sic
                        if (wrap && ((cursor_x + textsize_x * (xo + w)) > _width))
                        {
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

#endif // CLARINOID_MODULE_TEST
