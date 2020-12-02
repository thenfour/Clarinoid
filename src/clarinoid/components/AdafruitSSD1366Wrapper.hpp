

#pragma once


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

inline GFXglyph *pgm_read_glyph_ptr(const GFXfont *gfxFont, uint8_t c) {
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

inline uint8_t *pgm_read_bitmap_ptr(const GFXfont *gfxFont) {
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
struct CCAdafruitSSD1306 : public Adafruit_SSD1306
{
  CCAdafruitSSD1306(uint8_t w, uint8_t h, SPIClass *spi, int8_t dc_pin, int8_t rst_pin, int8_t cs_pin, uint32_t bitrate = 8000000UL) :
    Adafruit_SSD1306(w, h, spi, dc_pin, rst_pin, cs_pin, bitrate),// 128, 64, &SPI, 9/*DC*/, 8/*RST*/, 10/*CS*/, 44 * 1000000UL)
    mClipRight(w),
    mClipBottom(h)
  {
    ResetClip();
  }

// #define OLED_MOSI   9
// #define OLED_CLK   10
// #define OLED_DC    11
// #define OLED_CS    12
// #define OLED_RESET 13
  CCAdafruitSSD1306(uint8_t w, uint8_t h, int8_t mosi_pin, int8_t sclk_pin,
                   int8_t dc_pin, int8_t rst_pin, int8_t cs_pin) :
    Adafruit_SSD1306(w, h, mosi_pin, sclk_pin, dc_pin, rst_pin, cs_pin)
  {
    ResetClip();
  }

  bool mSolidText = true;
  int mTextLeftMargin = 0;
  int mClipLeft = 0;
  int mClipRight = 0;
  int mClipTop = 0;
  int mClipBottom = 0;

  void ResetClip()
  {
    mClipLeft = 0;
    mClipRight = width();
    mClipTop = 0;
    mClipBottom = height();
  }

  void ClipToMargin(int m)
  {
    mClipLeft = m;
    mClipRight = width() - m;
    mClipTop = m;
    mClipBottom = height() - m;
  }

  // for checker-style bool checking
  bool PixelParity(int16_t x, int16_t y) const {
    return (x & 1) != (y & 1);
  }

  bool IsWithinClipRect(int16_t x, int16_t y) const {
    if (x < mClipLeft) return false;
    if (x >= mClipRight) return false;
    if (y < mClipTop) return false;
    if (y >= mClipBottom) return false;
    return true;
  }

  // overriding Adafruit_GFX::writePixel. base forwards to drawPixel().
  virtual void writePixel(int16_t x, int16_t y, uint16_t color) {
    if (!IsWithinClipRect(x, y))
      return;
    if (!mSolidText) {
      if (!PixelParity(x, y)) {
        // there are probably much more legible ways of graying text. this sorta destroys background color info; we should probably instead set a bg/fg color before the text write op
        return;
      }
    }
    drawPixel(x, y, color);
  }

  void DrawDottedRect(int16_t left, int16_t top, int16_t width, int16_t height, uint16_t color) {
    for (int16_t y = top; y < top + height; ++ y) {
      for (int16_t x = left; x < left + width; x += 2) {
        if (PixelParity(x, y)) {
          continue;
        }
        drawPixel(x, y, color);
      }
    }
  }

  // the text rendering routine calls these, so if you want clipped/checkered text you need to implement these too.
  //  virtual void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  //    // TODO: clip
  //    // TODO: checker
  //    fillRect(x, y, w, h, color);
  //  }
  //
  //  void writeFastVLine(int16_t x, int16_t y, int16_t h,
  //                                    uint16_t color) {
  //    if (!mSolidText) {
  //      writeFillRect(x, y, 1, h, color);
  //      return;
  //    }
  //    // TODO: clip
  //    drawFastVLine(x, y, h, color);
  //  }

  /**************************************************************************/
  /*!
      @brief  Print one byte/character of data, used to support print()
      @param  c  The 8-bit ascii character to write
  */
  /**************************************************************************/
  size_t /*Adafruit_GFX::*/write(uint8_t c) {
    if (!gfxFont) { // 'Classic' built-in font
  
      if (c == '\n') {              // Newline?
        cursor_x = mTextLeftMargin;               // Reset x to left margin
        cursor_y += textsize_y * 8; // advance y one line
      } else if (c != '\r') {       // Ignore carriage returns
        if (wrap && ((cursor_x + textsize_x * 6) > _width)) { // Off right?
          cursor_x = mTextLeftMargin; // reset x to left margin
          cursor_y += textsize_y * 8; // advance y one line
        }
        drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x,
                 textsize_y);
        cursor_x += textsize_x * 6; // Advance x one char
      }
  
    } else { // Custom font
  
      if (c == '\n') {
        cursor_x = mTextLeftMargin;
        cursor_y +=
            (int16_t)textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
      } else if (c != '\r') {
        uint8_t first = pgm_read_byte(&gfxFont->first);
        if ((c >= first) && (c <= (uint8_t)pgm_read_byte(&gfxFont->last))) {
          GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c - first);
          uint8_t w = pgm_read_byte(&glyph->width),
                  h = pgm_read_byte(&glyph->height);
          if ((w > 0) && (h > 0)) { // Is there an associated bitmap?
            int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset); // sic
            if (wrap && ((cursor_x + textsize_x * (xo + w)) > _width)) {
              cursor_x = mTextLeftMargin;
              cursor_y += (int16_t)textsize_y *
                          (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
            }
            drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x,
                     textsize_y);
          }
          cursor_x +=
              (uint8_t)pgm_read_byte(&glyph->xAdvance) * (int16_t)textsize_x;
        }
      }
    }
    return 1;
  }
};
