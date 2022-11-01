
namespace clarinoid
{

//////////////////////////////////////////////////////////////////////
struct IHudProvider
{
    virtual int16_t IHudProvider_GetHudHeight() = 0;
    virtual void IHudProvider_RenderHud(int16_t displayWidth, int16_t displayHeight) = 0;
};

//////////////////////////////////////////////////////////////////////
struct IDisplayApp
{
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
struct IDisplay
{
    virtual void Init(AppSettings *appSettings,
                      InputDelegator *input,
                      IHudProvider *hud,
                      const array_view<IDisplayApp *> &apps) = 0;

    //virtual AppSettings *GetAppSettings() = 0;
    //virtual InputDelegator *GetInput() = 0;

    virtual uint16_t GetLineHeight() const = 0;
    virtual RectI SetupModal() = 0;
    virtual void SelectApp(int n) = 0;
    virtual void ScrollApps(int delta) = 0;
    virtual void UpdateAndRenderTask() = 0;

    virtual int16_t GetHudHeight() const = 0;
    virtual int16_t GetClientHeight() const = 0;
    virtual RectI GetClientRect() const = 0;

    // CCAdafruitSSD1306
    virtual void SetClipRect(int left, int top, int right, int bottom) = 0;
    virtual void SetTextSolid(bool b) = 0;
    virtual bool GetTextSolid() = 0;
    virtual int GetTextLeftMargin() = 0;
    virtual void SetTextLeftMargin(int) = 0;
    virtual void DrawDottedHLine(int16_t left, int16_t width, int16_t y, uint16_t color) = 0;
    virtual void DrawDottedRect(int16_t left, int16_t top, int16_t width, int16_t height, uint16_t color) = 0;
    virtual void DrawMarchingAntsFilledRect(int AntSize,
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

    // calculates in general, not for a specific location on screen.
    virtual RectI GetTextBounds(const String &str) = 0;
    virtual void PrintInvertedText(const String &str, bool isInverted = true) = 0;
    virtual void PrintInvertedLine(const String &str, bool isInverted = true) = 0;
    virtual void DrawSelectionRect(const RectI &z) = 0;
    virtual void DisplayTask() = 0;
    virtual void SelectTinyFont() = 0;
    virtual void SelectEightiesFont() = 0;
    virtual void SelectNormalFont() = 0;
    virtual void ShowToast(const String &msg) = 0;
    virtual int ClippedAreaHeight() const = 0;
    virtual void ResetClip() = 0;
    virtual void SetClipRect(const RectI &rc) = 0;
    virtual void ClipToMargin(int m) = 0;
    virtual void ClearState() = 0;
    virtual void DrawBitmap(PointI pos, const BitmapSpec &bmp) = 0;
    virtual void fillPie(const PointF &origin,
                         float radius,
                         float angleStart,
                         float angleSweep,
                         bool filled = true) = 0;

    // from Adafruit_GFX
    virtual int16_t width() const = 0;
    virtual int16_t height() const = 0;
    virtual int16_t getCursorX() const = 0;
    virtual int16_t getCursorY() const = 0;
    virtual void setTextWrap(bool w) = 0;
    virtual void setTextColor(uint16_t c) = 0;
    virtual void setCursor(int16_t x, int16_t y) = 0;
    virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) = 0;
    virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) = 0;
    virtual void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color) = 0;
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    virtual void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) = 0;
    virtual void print(const String &s) = 0;
    virtual void println(const String &s) = 0;
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
    virtual void dim(bool d) = 0;
    virtual void fillScreen(uint16_t color) = 0;
};

} // namespace clarinoid
