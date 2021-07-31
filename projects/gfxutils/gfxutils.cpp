// gfxutils.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "gfxutils.h"
#include <cmath>
#include <algorithm>

template<typename T>
constexpr T gPI = T(3.1415926535897932385);

template <typename T, size_t N>
constexpr size_t SizeofStaticArray(const T(&x)[N])
{
  return N;
}

static float Clamp(float x, float low, float hi)
{
  if (x <= low)
    return low;
  if (x >= hi)
    return hi;
  return x;
}


template<typename T>
void drawLine(int x0, int y0, int x1, int y1, T&& drawPixel)
{
  int dx = abs(x1 - x0);
  int sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0);
  int sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;  /* error value e_xy */
  while (true) {   /* loop */
    drawPixel(x0, y0, true);
    if (x0 == x1 && y0 == y1) break;
    int e2 = 2 * err;
    if (e2 >= dy) { /* e_xy+e_x > 0 */
      err += dy;
      x0 += sx;
    }
    if (e2 <= dx) {/* e_xy+e_y < 0 */
      err += dx;
      y0 += sy;
    }
  }
}

// adapted from
// https://stackoverflow.com/questions/58222657/generate-a-pieslice-in-c-without-using-the-pieslice-of-graphics-h
template <typename T>
void fillPie(float x0, float y0, float r, float a0, float a1, T &&drawPixel) // a0 < a1
{
  float x, y,     // circle centered point
    xx, yy, rr, // x^2,y^2,r^2
    ux, uy,     // u
    vx, vy,     // v
    sx, sy;     // pixel position
  rr = r * r;
  ux = (r)*cosf(a0);
  uy = (r)*sinf(a0);
  vx = (r)*cosf(a1);
  vy = (r)*sinf(a1);
  // handle big/small pies
  x = a1 - a0;
  if (x < 0)
    x = -x;
  // render small pies
  int pixelsDrawn = 0;
  if (x < gPI<float>) /* 180 deg */
  {
    for (y = -r, yy = y * y, sy = y0 + y; y <= +r; y++, yy = y * y, sy++)
    {
      for (x = -r, xx = x * x, sx = x0 + x; x <= +r; x++, xx = x * x, sx++)
      {
        if (xx + yy <= rr)                     // inside circle
          if (((x * uy) - (y * ux) <= 0)     // x,y is above a0 in clockwise direction
            && ((x * vy) - (y * vx) >= 0)) // x,y is below a1 in counter clockwise direction
          {
            drawPixel((int)::floorf(sx), (int)::floorf(sy), false);
            ++pixelsDrawn;
          }
      }
    }
  }
  else
  {
    for (y = -r, yy = y * y, sy = y0 + y; y <= +r; y++, yy = y * y, sy++)
    {
      for (x = -r, xx = x * x, sx = x0 + x; x <= +r; x++, xx = x * x, sx++)
      {
        if (xx + yy <= rr)
        {                                      // inside circle
          if (((x * uy) - (y * ux) <= 0)     // x,y is above a0 in clockwise direction
            || ((x * vy) - (y * vx) >= 0)) // x,y is below a1 in counter clockwise direction
          {
            drawPixel((int)::floorf(sx), (int)::floorf(sy), false);
            ++pixelsDrawn;
          }
        }
      }
    }
  }
    drawLine(x0, y0, x0 + ux, y0 + uy, drawPixel);
}





namespace KnobDetails {
  static const uint8_t gKnobOutlineBMP[] = {
      0b00000111, 0b11000000, //
      0b00011000, 0b00110000, //
      0b00100000, 0b00001000, //
      0b01000000, 0b00000100, //
      0b01000000, 0b00000100, //
      0b10000000, 0b00000010, //
      0b10000000, 0b00000010, //
      0b10000001, 0b00000010, //
      0b10000000, 0b00000010, //
      0b10000000, 0b00000010, //
      0b01000000, 0b00000100, //
      0b01010000, 0b00010100, //
      0b00100000, 0b00001000, //
  };
  static const uint8_t BmpWidthBytes = 2;
  static const uint8_t Height = SizeofStaticArray(gKnobOutlineBMP) / BmpWidthBytes;
  static float CenterX = 7.75f;
  static float CenterY = 7.5f;
  static float MinAngle = 2.35f;
  static float MaxAngle = 7.15f;
  static float Radius = 7.1f;
}


void SetPixel2(HDC dc, int x, int y, COLORREF c)
{
  SetPixel(dc, x * 2, y * 2, c);
  SetPixel(dc, x * 2 + 1, y * 2, c);
  SetPixel(dc, x * 2, y * 2 + 1, c);
  SetPixel(dc, x * 2 + 1, y * 2 + 1, c);
}

void DrawKnobOutline(HDC dc, int ox, int oy)
{
  for (int y = 0; y < KnobDetails::Height; ++y)
  {
    for (int x = 0; x < KnobDetails::BmpWidthBytes; ++x)
    {
      for (int b = 0; b < 8; ++b) {
        if (KnobDetails::gKnobOutlineBMP[y*KnobDetails::BmpWidthBytes + x] & (1 << (7-b))) {
          SetPixel2(dc, ox + x*8 + b, oy + y, RGB(128, 0, 0));
        }
      }
    }
  }
}



// Global Variables:
HINSTANCE hInst;                                // current instance

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GFXUTILS));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GFXUTILS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GFXUTILS);
    wcex.lpszClassName  = L"aosentuhaoesnuth";
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(L"aosentuhaoesnuth", L"aosentuhaoesnuth", WS_OVERLAPPEDWINDOW,
      0, 0, 300, 300, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   SetTimer(hWnd, 0, 100, NULL);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

float g1 = 0.50f;
float g2 = 0.70f;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_TIMER:
      InvalidateRect(hWnd, NULL, TRUE);
      return 0;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_KEYDOWN:
    {
      if (GetKeyState(VK_SHIFT) & 0x8000)
      {
        //switch (wParam /*vk*/) {
        //case VK_UP:
        //  //KnobDetails::Radius-= .5f;
        //  break;
        //case VK_DOWN:
        //  //KnobDetails::Radius+=.5f;
        //  break;
        //case VK_LEFT:
        //  //KnobDetails::CenterX -= .5f;
        //  g1 -= 0.1f;
        //  break;
        //case VK_RIGHT:
        //  //KnobDetails::CenterX += .5f;
        //  g2 += 0.1f;
        //  break;
        //}
      }
      else  if (GetKeyState(VK_CONTROL) & 0x8000)
      {
        //switch (wParam /*vk*/) {
        //case VK_UP:
        //  KnobDetails::MinAngle -= .1f;
        //  break;
        //case VK_DOWN:
        //  KnobDetails::MinAngle += .1f;
        //  break;
        //case VK_LEFT:
        //  KnobDetails::MaxAngle -= .1f;
        //  break;
        //case VK_RIGHT:
        //  KnobDetails::MaxAngle += .1f;
        //  break;
        //}
      }
      else {
        switch (wParam /*vk*/) {
        case VK_UP:
          //KnobDetails::CenterY -= .5f;
          g2 += 0.1f;
          break;
        case VK_DOWN:
          //KnobDetails::CenterY += .5f;
          g2 -= 0.1f;
          break;
        case VK_LEFT:
          //KnobDetails::CenterX -= .5f;
          g1 -= 0.1f;
          break;
        case VK_RIGHT:
//          KnobDetails::CenterX += .5f;
          g1 += 0.1f;
          break;
        }

      }
      wchar_t s[1000];

      g1 = Clamp(g1, 0, 1);
      g2 = Clamp(g2, 0, 1);

      swprintf_s(s, L"(%g, %g), radius %g  ang %g -> %g  g1=%g g2=%g \r\n", KnobDetails::CenterX, KnobDetails::CenterY, KnobDetails::Radius, KnobDetails::MinAngle, KnobDetails::MaxAngle, g1, g2);
      OutputDebugStringW(s);
      return 0;

    }
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            DrawKnobOutline(hdc, 70, 70);

            float f1 = GetTickCount();
            f1 /= 1000.0f;
            float f2 = GetTickCount();
            f2 /= 10000.0f;

            float a0 = ((KnobDetails::MaxAngle - KnobDetails::MinAngle) * g1) + KnobDetails::MinAngle;
            float a1 = ((KnobDetails::MaxAngle - KnobDetails::MinAngle) * g2) + KnobDetails::MinAngle;

            //fillPie(70 + KnobDetails::CenterX, 70 + KnobDetails::CenterY, KnobDetails::Radius, KnobDetails::MinAngle, KnobDetails::MaxAngle, [&](float x, float y, bool p) {
            fillPie(70 + KnobDetails::CenterX, 70 + KnobDetails::CenterY, KnobDetails::Radius, a0, a1, [&](float x, float y, bool p) {
                SetPixel2(ps.hdc, x, y, RGB(p ? 255 : 0, p ? 1 : 0, 0));
            });

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
