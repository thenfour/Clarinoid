
#ifndef CCDISPLAY_H
#define CCDISPLAY_H

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Shared_CCUtil.h"

static constexpr int RESOLUTION_X = 128;
static constexpr int RESOLUTION_Y = 32;
static constexpr int MAX_MENU_APPS = 20;

//////////////////////////////////////////////////////////////////////
class MenuAppBase
{
public:
  virtual void OnSelected() {};
  virtual void OnUnselected() {}
  virtual void Update() = 0; // called each frame
  virtual void Render() = 0; // called occasionally
};


// CC: not sure why every example uses software SPI. is there a hardware SPI that would work together with the audio shield?
// If using software SPI (the default case):
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13

framerateCalculator gFramerate;



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

#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                    \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }
#endif


// i need to subclass in order to support some things:
// text left margin (so println() new line doesn't set x=0)
// clipping bounds
// disabled text
class CCAdafruitSSD1306 : public Adafruit_SSD1306
{
public:

  CCAdafruitSSD1306() :
    Adafruit_SSD1306(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS)
  {}

  bool mSolidText = true;
  int mTextLeftMargin = 0;
  int mClipLeft = 0;
  int mClipRight = RESOLUTION_X;
  int mClipTop = 0;
  int mClipBottom = RESOLUTION_Y;

//  // Draw a character
//  /**************************************************************************/
//  /*!
//     @brief   Draw a single character
//      @param    x   Bottom left corner x coordinate
//      @param    y   Bottom left corner y coordinate
//      @param    c   The 8-bit font-indexed character (likely ascii)
//      @param    color 16-bit 5-6-5 Color to draw chraracter with
//      @param    bg 16-bit 5-6-5 Color to fill background with (if same as color,
//     no background)
//      @param    size_x  Font magnification level in X-axis, 1 is 'original' size
//      @param    size_y  Font magnification level in Y-axis, 1 is 'original' size
//  */
//  /**************************************************************************/
//  void Adafruit_GFX::drawChar(int16_t x, int16_t y, unsigned char c,
//                              uint16_t color, uint16_t bg, uint8_t size_x,
//                              uint8_t size_y) {
//  
//    if (!gfxFont) { // 'Classic' built-in font
//  
//      if ((x >= _width) ||              // Clip right
//          (y >= _height) ||             // Clip bottom
//          ((x + 6 * size_x - 1) < 0) || // Clip left
//          ((y + 8 * size_y - 1) < 0))   // Clip top
//        return;
//  
//      if (!_cp437 && (c >= 176))
//        c++; // Handle 'classic' charset behavior
//  
//      startWrite();
//      for (int8_t i = 0; i < 5; i++) { // Char bitmap = 5 columns
//        uint8_t line = pgm_read_byte(&font[c * 5 + i]);
//        for (int8_t j = 0; j < 8; j++, line >>= 1) {
//          if (line & 1) {
//            if (size_x == 1 && size_y == 1)
//              writePixel(x + i, y + j, color);
//            else
//              writeFillRect(x + i * size_x, y + j * size_y, size_x, size_y,
//                            color);
//          } else if (bg != color) {
//            if (size_x == 1 && size_y == 1)
//              writePixel(x + i, y + j, bg);
//            else
//              writeFillRect(x + i * size_x, y + j * size_y, size_x, size_y, bg);
//          }
//        }
//      }
//      if (bg != color) { // If opaque, draw vertical line for last column
//        if (size_x == 1 && size_y == 1)
//          writeFastVLine(x + 5, y, 8, bg);
//        else
//          writeFillRect(x + 5 * size_x, y, size_x, 8 * size_y, bg);
//      }
//      endWrite();
//  
//    } else { // Custom font
//  
//      // Character is assumed previously filtered by write() to eliminate
//      // newlines, returns, non-printable characters, etc.  Calling
//      // drawChar() directly with 'bad' characters of font may cause mayhem!
//  
//      c -= (uint8_t)pgm_read_byte(&gfxFont->first);
//      GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c);
//      uint8_t *bitmap = pgm_read_bitmap_ptr(gfxFont);
//  
//      uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
//      uint8_t w = pgm_read_byte(&glyph->width), h = pgm_read_byte(&glyph->height);
//      int8_t xo = pgm_read_byte(&glyph->xOffset),
//             yo = pgm_read_byte(&glyph->yOffset);
//      uint8_t xx, yy, bits = 0, bit = 0;
//      int16_t xo16 = 0, yo16 = 0;
//  
//      if (size_x > 1 || size_y > 1) {
//        xo16 = xo;
//        yo16 = yo;
//      }
//  
//      // Todo: Add character clipping here
//  
//      // NOTE: THERE IS NO 'BACKGROUND' COLOR OPTION ON CUSTOM FONTS.
//      // THIS IS ON PURPOSE AND BY DESIGN.  The background color feature
//      // has typically been used with the 'classic' font to overwrite old
//      // screen contents with new data.  This ONLY works because the
//      // characters are a uniform size; it's not a sensible thing to do with
//      // proportionally-spaced fonts with glyphs of varying sizes (and that
//      // may overlap).  To replace previously-drawn text when using a custom
//      // font, use the getTextBounds() function to determine the smallest
//      // rectangle encompassing a string, erase the area with fillRect(),
//      // then draw new text.  This WILL infortunately 'blink' the text, but
//      // is unavoidable.  Drawing 'background' pixels will NOT fix this,
//      // only creates a new set of problems.  Have an idea to work around
//      // this (a canvas object type for MCUs that can afford the RAM and
//      // displays supporting setAddrWindow() and pushColors()), but haven't
//      // implemented this yet.
//  
//      startWrite();
//      for (yy = 0; yy < h; yy++) {
//        for (xx = 0; xx < w; xx++) {
//          if (!(bit++ & 7)) {
//            bits = pgm_read_byte(&bitmap[bo++]);
//          }
//          if (bits & 0x80) {
//            if (size_x == 1 && size_y == 1) {
//              writePixel(x + xo + xx, y + yo + yy, color);
//            } else {
//              writeFillRect(x + (xo16 + xx) * size_x, y + (yo16 + yy) * size_y,
//                            size_x, size_y, color);
//            }
//          }
//          bits <<= 1;
//        }
//      }
//      endWrite();
//  
//    } // End classic vs custom font
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

//////////////////////////////////////////////////////////////////////
class CCDisplay : IUpdateObject
{
  CCThrottlerT<20> mThrottle;
  CCEWIApp& mApp;

  int mMenuAppCount = 0;
  MenuAppBase* mMenuApps[MAX_MENU_APPS];
  int mCurrentMenuAppIndex = 0;

public:
  CCAdafruitSSD1306 mDisplay;
  bool mIsSetup = false;

  void AddMenuApp(MenuAppBase* p) {
    mMenuApps[mMenuAppCount] = p;
    mMenuAppCount ++;
  }

  // display gets access to the whole app
  CCDisplay(CCEWIApp& app) : 
    mApp(app)
  {
  }

  void SelectApp(int n) {
    n %= mMenuAppCount;
    while (n < 0)
      n += mMenuAppCount;

    if (n == mCurrentMenuAppIndex)
      return;

    mMenuApps[mCurrentMenuAppIndex]->OnUnselected();
    mCurrentMenuAppIndex = n;
    //Serial.println(String("selecting app ") + n );
    mMenuApps[mCurrentMenuAppIndex]->OnSelected();
  }

  void ScrollApps(int delta) {
    SelectApp(mCurrentMenuAppIndex + delta);
  }

  void DisplayToast(const String& s) {
    // todo
  }

  virtual void setup() {
    // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
    mDisplay.begin(SSD1306_SWITCHCAPVCC);
    mDisplay.clearDisplay();
    mDisplay.display();
    mIsSetup = true;
  }

  bool first = true;

  virtual void loop() {    
    gFramerate.onFrame();

    MenuAppBase* pMenuApp = mMenuApps[mCurrentMenuAppIndex];

    if (first) {
      first = false;
      pMenuApp->OnSelected();
    }

    pMenuApp->Update();
    
    if (!mThrottle.IsReady())
      return;

    mDisplay.clearDisplay();
    mDisplay.mSolidText = true;
    mDisplay.mTextLeftMargin = 0;
    mDisplay.mClipLeft = 0;
    mDisplay.mClipRight = RESOLUTION_X;
    mDisplay.mClipTop = 0;
    mDisplay.mClipBottom = RESOLUTION_Y;

    pMenuApp->Render();
    
    mDisplay.display();
  }
};

CCDisplay gDisplay(gApp);




//////////////////////////////////////////////////////////////////////
struct ListControl
{
  const IList* mpList;
  Property<int> mSelectedItem;
  int mX;
  int mY;
  int mMaxItemsToRender;

  ListControl(const IList* list, Property<int> selectedItemBinding, int x, int y, int nVisibleItems) : 
    mpList(list),
    mSelectedItem(selectedItemBinding),
    mX(x),
    mY(y),
    mMaxItemsToRender(nVisibleItems)
  {
    CCASSERT(!!selectedItemBinding.mGetter);
    CCASSERT(!!selectedItemBinding.mSetter);
    CCASSERT(!!mSelectedItem.mGetter);
    CCASSERT(!!mSelectedItem.mSetter);
  }
  
  void Render()
  {
    auto count = mpList->List_GetItemCount();
    if (count == 0) return;
    gDisplay.mDisplay.setTextSize(1);
    //gDisplay.mDisplay.setCursor(0, 0);
    gDisplay.mDisplay.setTextWrap(false);
    int itemToRender = RotateIntoRange(mSelectedItem.GetValue() - 1, count);
    const int itemsToRender = min(mMaxItemsToRender, count);
    for (int i = 0; i < itemsToRender; ++ i) {
      if (itemToRender == mSelectedItem.GetValue()) {
        gDisplay.mDisplay.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
      } else {
        gDisplay.mDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
      }

      gDisplay.mDisplay.println(mpList->List_GetItemCaption(itemToRender));

      itemToRender = RotateIntoRange(itemToRender + 1, count);
    }
  }
  
  virtual void Update()
  {
    CCASSERT(mpList);
    auto c = mpList->List_GetItemCount();
    if (c == 0) return;
    //auto v = mSelectedItem.GetValue();
    mSelectedItem.SetValue(AddConstrained(mSelectedItem.GetValue(), gEnc.GetIntDelta(), 0, mpList->List_GetItemCount() - 1));
  }
};



#endif
