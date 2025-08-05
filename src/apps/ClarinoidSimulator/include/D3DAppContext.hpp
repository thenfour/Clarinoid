#pragma once

#include <clarinoid/sim/ComPtr.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////
// the D3D11 plumbing that imgui needs.
struct D3DAppContext
{
  ComPtr<ID3D11Device> mpDevice;
  ComPtr<ID3D11DeviceContext> mpDeviceContext;
  ComPtr<IDXGISwapChain> mpSwapChain;
  ComPtr<ID3D11RenderTargetView> mpRenderTargetView;
  HWND mhWnd;

  D3DAppContext(HWND hWnd)
    : mhWnd(hWnd)
  {
    RECT rcClient;
    GetClientRect(hWnd, &rcClient);

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = rcClient.right;
    sd.BufferDesc.Height = rcClient.bottom;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[1] = { D3D_FEATURE_LEVEL_11_0 };
    if (D3D11CreateDeviceAndSwapChain(NULL,
                                      D3D_DRIVER_TYPE_HARDWARE,
                                      NULL,
                                      createDeviceFlags,
                                      featureLevelArray,
                                      1,
                                      D3D11_SDK_VERSION,
                                      &sd,
                                      mpSwapChain.GetAddressOf(),
                                      mpDevice.GetAddressOf(),
                                      &featureLevel,
                                      mpDeviceContext.GetAddressOf()) != S_OK) {
      throw std::runtime_error("Failed to create D3D11 device and swap chain");
    }

    Resize(rcClient.right, rcClient.bottom);
  }

  void Resize(int width, int height)
  {
    if (!mpSwapChain) {
      throw std::runtime_error("Swap chain is not initialized");
    }
    mpRenderTargetView.Reset();
    if (S_OK != mpSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0)) {
      throw std::runtime_error("Failed to resize swap chain buffers");
    }
    ComPtr<ID3D11Texture2D> pBackBuffer;
    if (S_OK != mpSwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()))) {
      throw std::runtime_error("Failed to get back buffer from swap chain after resize");
    }
    if (S_OK != mpDevice->CreateRenderTargetView(pBackBuffer.Get(), NULL, mpRenderTargetView.GetAddressOf())) {
      throw std::runtime_error("Failed to create render target view after resize");
    }
  }
};
