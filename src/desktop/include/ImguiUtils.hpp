#pragma once

#include <utils.hpp>
#include <imgui.h>

#include "D3DAppContext.hpp"


extern IMGUI_IMPL_API LRESULT
ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


ImU32
apply_opacity_mul(ImU32 col, float opacity01)
{
  ImVec4 c = ImGui::ColorConvertU32ToFloat4(col); // RGBA in [0..1]
  c.w *= clamp(opacity01, 0.0f, 1.0f);          // multiply alpha
  return ImGui::ColorConvertFloat4ToU32(c);
}


// wrap image calls in this to use a nearest neighbor point sampler
struct NearestNeighborRenderer
{
  NearestNeighborRenderer(D3DAppContext* pd3dAppContext)
    : mpd3dAppContext(pd3dAppContext)
  {
    D3D11_SAMPLER_DESC sd{};
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; // nearest-neighbor
    sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    HRESULT hr = mpd3dAppContext->mpDevice->CreateSamplerState(&sd, mpSamplerState.GetAddressOf());
    if (FAILED(hr))
      throw std::runtime_error("CreateSamplerState (POINT) failed");
  }

  void Begin()
  {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddCallback(&NearestNeighborRenderer::SetPointSamplerCB, this);
  }

  void End()
  {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr); // restore default (linear) state
  }

private:
  static void SetPointSamplerCB(const ImDrawList*, const ImDrawCmd* cmd)
  {
    auto* self = static_cast<NearestNeighborRenderer*>(cmd->UserCallbackData);
    ID3D11SamplerState* s = self->mpSamplerState.Get();
    self->mpd3dAppContext->mpDeviceContext->PSSetSamplers(0, 1, &s); // ImGui backend samples from PS slot 0
  }

  D3DAppContext* mpd3dAppContext;
  ComPtr<ID3D11SamplerState> mpSamplerState; // owned
};
