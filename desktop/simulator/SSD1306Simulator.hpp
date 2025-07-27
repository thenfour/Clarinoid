#pragma once

#include "D3DAppContext.hpp"
#include <clarinoid/basic/Geometry.hpp>

namespace oled128x64 {

constexpr int W = 128;
constexpr int H = 64;

inline void
FillLissajous(std::array<bool, W * H>& buf,
              float timeSeconds,
              int a = 3,               // x frequency
              int b = 2,               // y frequency
              float phase0 = 0.0f,     // initial phase delta between x/y
              float phaseSpeed = 1.0f, // radians/sec added to delta
              int thickness = 0,       // 0 = single‑pixel, 1.. = radius in px
              int samples = 2000,      // curve sampling
              int margin = 2,          // border margin in px
    float samplePhaseOffset = 0
) 
{
  std::fill(buf.begin(), buf.end(), false);

  auto setPix = [&](int x, int y) {
    if (unsigned(x) < unsigned(W) && unsigned(y) < unsigned(H))
      buf[y * W + x] = true;
  };

  auto drawDisk = [&](int cx, int cy, int r) {
    if (r <= 0) {
      setPix(cx, cy);
      return;
    }
    const int r2 = r * r;
    for (int dy = -r; dy <= r; ++dy) {
      int yy = cy + dy;
      if (yy < 0 || yy >= H)
        continue;
      int dxmax = int(std::floor(std::sqrt(float(r2 - dy * dy))));
      int x0 = cx - dxmax, x1 = cx + dxmax;
      if (x0 < 0)
        x0 = 0;
      if (x1 >= W)
        x1 = W - 1;
      for (int x = x0; x <= x1; ++x)
        buf[yy * W + x] = true;
    }
  };

  auto drawLine = [&](int x0, int y0, int x1, int y1) {
    int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    for (;;) {
      drawDisk(x0, y0, thickness);
      if (x0 == x1 && y0 == y1)
        break;
      int e2 = err << 1;
      if (e2 >= dy) {
        err += dy;
        x0 += sx;
      }
      if (e2 <= dx) {
        err += dx;
        y0 += sy;
      }
    }
  };

  // Fit curve to screen with a small margin
  const float cx = (W - 1) * 0.5f;
  const float cy = (H - 1) * 0.5f;
  const float ax = (W - 1 - 2 * margin) * 0.5f;
  const float ay = (H - 1 - 2 * margin) * 0.5f;

  const float delta = phase0 + phaseSpeed * timeSeconds; // animate by shifting phase
  constexpr float TWO_PI = 6.28318530717958647692f;

  int prevx = INT32_MIN, prevy = INT32_MIN;
  for (int i = 0; i < samples; ++i) {
    float u = (i / float(samples - 1)) * TWO_PI;
    float X = std::sinf(a * u + delta + samplePhaseOffset);
    float Y = std::sinf(b * u + samplePhaseOffset);

    int x = int(std::lround(cx + ax * X));
    int y = int(std::lround(cy - ay * Y)); // minus because screen y grows downward

    if (i == 0)
      drawDisk(x, y, thickness);
    else
      drawLine(prevx, prevy, x, y);

    prevx = x;
    prevy = y;
  }
}

} // namespace oled128x64

// renderer of memory-backed bitmap
class MonoBitmapTexture
{
  D3DAppContext* mpd3dAppContext;

  int width_;
  int height_;
  std::vector<uint32_t> rgba_; // scratch CPU buffer

  ComPtr<ID3D11Texture2D> tex_;
  ComPtr<ID3D11ShaderResourceView> srv_;
  NearestNeighborRenderer pointSampler_;

public:

    int GetWidth() const { return width_; }
  int GetHeight() const { return height_; }

  MonoBitmapTexture(D3DAppContext* pd3dAppContext, int width = 128, int height = 64)
    : mpd3dAppContext(pd3dAppContext)
    , pointSampler_(pd3dAppContext)
    , width_(width)
    , height_(height)
  {
    createResources(); // throws on failure (optional: return HRESULT instead)
    rgba_.resize(static_cast<size_t>(width_) * height_);
  }

  // Non-copyable, movable if you like
  MonoBitmapTexture(const MonoBitmapTexture&) = delete;
  MonoBitmapTexture& operator=(const MonoBitmapTexture&) = delete;

  template<size_t N>
  void Update(const std::array<bool, N>& mono, const ImColor& fg, const ImColor& bg)
  {
    assert(N == static_cast<size_t>(width_) * height_ && "size mismatch");
    for (size_t i = 0; i < N; ++i) {

      rgba_[i] = mono[i] ? fg : bg;
    }

    // Upload to GPU
    D3D11_MAPPED_SUBRESOURCE mapped{};
    if (S_OK == mpd3dAppContext->mpDeviceContext->Map(tex_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)) {
      const uint8_t* src = reinterpret_cast<const uint8_t*>(rgba_.data());
      const UINT srcPitch = static_cast<UINT>(width_) * 4;
      uint8_t* dst = static_cast<uint8_t*>(mapped.pData);
      for (int y = 0; y < height_; ++y) {
        memcpy(dst + y * mapped.RowPitch, src + y * srcPitch, srcPitch);
      }
      mpd3dAppContext->mpDeviceContext->Unmap(tex_.Get(), 0);
    }
  }

  // Draw with ImGui. ImTextureID for the DX11 backend is ID3D11ShaderResourceView*.
  void Draw(int scale = 4)
  {
    pointSampler_.Begin();
    ImGui::Image(srv_.Get(), ImVec2(static_cast<float>(width_ * scale), static_cast<float>(height_ * scale)));
    pointSampler_.End();
  }

  // If you need to change size at runtime
  void Resize(int w, int h)
  {
    if (w == width_ && h == height_)
      return;
    width_ = w;
    height_ = h;
    rgba_.resize(static_cast<size_t>(width_) * height_);
    tex_.Reset();
    srv_.Reset();
    createResources();
  }

  ID3D11ShaderResourceView* GetSRV() const { return srv_.Get(); }


void DrawMagnifier(int cx,
                     int cy,
                     int regionW = 16,
                     int regionH = 16,
                     int scale = 12,
                     ImU32 gridColor = IM_COL32(80, 80, 80, 255),
                     ImU32 borderColor = IM_COL32(255, 255, 255, 255),
                     ImU32 crossColor = IM_COL32(255, 0, 0, 255),
                     ImU32 cellOutlineColor = IM_COL32(255, 255, 0, 200))
  {
    if (!srv_ || regionW <= 0 || regionH <= 0 || scale <= 0)
      return;

    // ----- same crop math as before -----
    int halfW = regionW / 2;
    int halfH = regionH / 2;
    int x0 = cx - halfW;
    int y0 = cy - halfH;
    int x1 = x0 + regionW - 1;
    int y1 = y0 + regionH - 1;
    if (x0 < 0) {
      x1 -= x0;
      x0 = 0;
    }
    if (y0 < 0) {
      y1 -= y0;
      y0 = 0;
    }
    if (x1 >= width_) {
      x0 -= (x1 - (width_ - 1));
      x1 = width_ - 1;
      if (x0 < 0)
        x0 = 0;
    }
    if (y1 >= height_) {
      y0 -= (y1 - (height_ - 1));
      y1 = height_ - 1;
      if (y0 < 0)
        y0 = 0;
    }
    const int w = x1 - x0 + 1;
    const int h = y1 - y0 + 1;

    const ImVec2 uv0 = ImVec2(float(x0) / float(width_), float(y0) / float(height_));
    const ImVec2 uv1 = ImVec2(float(x1 + 1) / float(width_), float(y1 + 1) / float(height_));
    const ImVec2 size = ImVec2(float(w * scale), float(h * scale));

    // ----- draw sub-image with nearest -----
    pointSampler_.Begin();
    ImGui::Image(srv_.Get(), size, uv0, uv1);
    pointSampler_.End();

    // ----- overlay: border, grid, crosshair -----
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 p0 = ImGui::GetItemRectMin();
    const ImVec2 p1 = ImGui::GetItemRectMax();

    // Outer border
    //dl->AddRect(p0, p1, borderColor, 0.0f, 0, 1.0f);

    // Grid between pixels
    for (int i = 1; i < w; ++i) {
      float x = p0.x + float(i * scale);
      dl->AddRectFilled(ImVec2(x, p0.y), ImVec2(x + 1.0f, p1.y), gridColor);
    }
    for (int j = 1; j < h; ++j) {
      float y = p0.y + float(j * scale);
      dl->AddRectFilled(ImVec2(p0.x, y), ImVec2(p1.x, y + 1.0f), gridColor);
    }

    // Compute the highlighted pixel (the requested (cx,cy), clamped to the region)
    const int hx = (cx < x0) ? x0 : (cx > x1 ? x1 : cx);
    const int hy = (cy < y0) ? y0 : (cy > y1 ? y1 : cy);
    const int lx = hx - x0; // local cell x index in [0,w)
    const int ly = hy - y0; // local cell y index in [0,h)

    const float cellX = p0.x + float(lx * scale);
    const float cellY = p0.y + float(ly * scale);
    const ImVec2 cellMin(cellX, cellY);
    const ImVec2 cellMax(cellX + scale, cellY + scale);

    // Optional box outline around the selected cell (thickness 1..2 looks good)
    dl->AddRect(cellMin, cellMax, cellOutlineColor, 0.0f, 0, 2.0f);

    // Crosshair through the center of the selected cell.
    // For crisp 1px lines, place them on integer coords (integer scale helps).
    const float xMid = std::floor(cellX + 0.5f * float(scale));
    const float yMid = std::floor(cellY + 0.5f * float(scale));

    // Vertical line
    dl->AddRectFilled(ImVec2(xMid, p0.y), ImVec2(xMid + 1.0f, p1.y), crossColor);
    // Horizontal line
    dl->AddRectFilled(ImVec2(p0.x, yMid), ImVec2(p1.x, yMid + 1.0f), crossColor);

    // Outer border
    dl->AddRect(p0, p1, borderColor, 0.0f, 0, 1.0f);
  }



// Draw an outline over the *last drawn* full image, showing the magnified region.
  // Call this immediately after Draw(baseScale), while that ImGui item is still "current".
  void DrawMagnifierOutlineOverImage(int cx,
                                     int cy,
                                     int regionW,
                                     int regionH,
                                     float baseScale,
                                     ImU32 color = IM_COL32(255, 255, 0, 255),
                                     float thickness = 2.0f)
  {
    // Compute region (same clamp as in DrawMagnifier)
    int halfW = regionW / 2;
    int halfH = regionH / 2;
    int x0 = cx - halfW;
    int y0 = cy - halfH;
    int x1 = x0 + regionW - 1;
    int y1 = y0 + regionH - 1;
    if (x0 < 0) {
      x1 -= x0;
      x0 = 0;
    }
    if (y0 < 0) {
      y1 -= y0;
      y0 = 0;
    }
    if (x1 >= width_) {
      x0 -= (x1 - (width_ - 1));
      x1 = width_ - 1;
      if (x0 < 0)
        x0 = 0;
    }
    if (y1 >= height_) {
      y0 -= (y1 - (height_ - 1));
      y1 = height_ - 1;
      if (y0 < 0)
        y0 = 0;
    }

    // Map source pixels -> screen space rect over the last ImGui::Image()
    const ImVec2 imgMin = ImGui::GetItemRectMin();
    const ImVec2 imgMax = ImGui::GetItemRectMax();
    // Guard: if the item isn’t an image you just drew, early out.
    if (imgMin.x >= imgMax.x || imgMin.y >= imgMax.y)
      return;

    const float sx = baseScale; // pixels on screen per source pixel
    const ImVec2 rmin(imgMin.x + sx * x0, imgMin.y + sx * y0);
    const ImVec2 rmax(imgMin.x + sx * (x1 + 1.0f), imgMin.y + sx * (y1 + 1.0f));

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Option A: simple stroked rect (may anti‑alias slightly)
    dl->AddRect(rmin, rmax, color, 0.0f, 0, thickness);

    // Option B (crisp 1‑px edges even with integer scales):
    // Uncomment and remove the AddRect above if you prefer exact 1‑px edges.
    /*
    const float t = 1.0f;
    dl->AddRectFilled(ImVec2(rmin.x, rmin.y), ImVec2(rmax.x, rmin.y + t), color); // top
    dl->AddRectFilled(ImVec2(rmin.x, rmax.y - t), ImVec2(rmax.x, rmax.y), color); // bottom
    dl->AddRectFilled(ImVec2(rmin.x, rmin.y), ImVec2(rmin.x + t, rmax.y), color); // left
    dl->AddRectFilled(ImVec2(rmax.x - t, rmin.y), ImVec2(rmax.x, rmax.y), color); // right
    */
  }

private:
  void createResources()
  {
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width_;
    desc.Height = height_;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ComPtr<ID3D11Texture2D> tex;
    HRESULT hr = mpd3dAppContext->mpDevice->CreateTexture2D(&desc, nullptr, tex.GetAddressOf());
    if (FAILED(hr))
      throw std::runtime_error("CreateTexture2D failed");

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    ComPtr<ID3D11ShaderResourceView> srv;
    hr = mpd3dAppContext->mpDevice->CreateShaderResourceView(tex.Get(), &srvDesc, srv.GetAddressOf());
    if (FAILED(hr))
      throw std::runtime_error("CreateShaderResourceView failed");

    tex_ = std::move(tex);
    srv_ = std::move(srv);
  }
};
