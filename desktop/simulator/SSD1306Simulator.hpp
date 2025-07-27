#pragma once

#include "D3DAppContext.hpp"

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
