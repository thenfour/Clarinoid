
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

  //

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

  virtual void setup() {
    // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
    mDisplay.begin(SSD1306_SWITCHCAPVCC);
    mIsSetup = true;
    mDisplay.clearDisplay();
    mDisplay.display();
    mDisplay.dim(gAppSettings.mDisplayDim);
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



// draws & prepares the screen for a modal message. after this just print text whatever.
static inline void SetupModal(int pad = 1, int rectStart = 2, int textStart = 4) {
  gDisplay.mDisplay.setCursor(0,0);
  gDisplay.mDisplay.fillRect(pad, pad, gDisplay.mDisplay.width() - pad, gDisplay.mDisplay.height() - pad, SSD1306_BLACK);
  gDisplay.mDisplay.drawRect(rectStart, rectStart, gDisplay.mDisplay.width() - rectStart, gDisplay.mDisplay.height() - rectStart, SSD1306_WHITE);
  gDisplay.mDisplay.setTextSize(1);
  gDisplay.mDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // normal text
  gDisplay.mDisplay.mTextLeftMargin = textStart;
  gDisplay.mDisplay.mClipLeft = textStart;
  gDisplay.mDisplay.mClipRight = RESOLUTION_X - textStart;
  gDisplay.mDisplay.mClipTop = textStart;
  gDisplay.mDisplay.mClipBottom = RESOLUTION_Y - textStart;
  gDisplay.mDisplay.setCursor(textStart, textStart);
}


#endif
