#pragma once

#include <memory>
#include <chrono>
#include <functional>

#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// the little win32 boilerplate; override HandleMessage to handle messages
struct SimpleWindow
{
  HWND mhWnd;
  WNDCLASSEXA mWindowClassDesc = { 0 };

  SimpleWindow(const char* windowName, int width, int height)
  {
    mWindowClassDesc.cbSize = sizeof(mWindowClassDesc);
    mWindowClassDesc.style = CS_HREDRAW | CS_VREDRAW;
    mWindowClassDesc.lpfnWndProc = WndProc;
    mWindowClassDesc.hInstance = GetModuleHandle(NULL);
    mWindowClassDesc.lpszClassName = "crch1234";

    if (!::RegisterClassExA(&mWindowClassDesc)) {

      throw std::runtime_error("Failed to register window class");
    }
    mhWnd = ::CreateWindowExA(WS_EX_OVERLAPPEDWINDOW,
                              mWindowClassDesc.lpszClassName,
                              windowName,
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              width,
                              height,
                              NULL, // parent
                              NULL, // menu
                              mWindowClassDesc.hInstance,
                              this); // param
    if (!mhWnd) {
      throw std::runtime_error("Failed to create window");
    }
  }

  ~SimpleWindow()
  {
    if (mhWnd) {
      DestroyWindow(mhWnd);
      mhWnd = NULL;
    }
    UnregisterClassA(mWindowClassDesc.lpszClassName, mWindowClassDesc.hInstance);
  }

  virtual bool HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) { return false; }

private:
  static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    switch (msg) {
      case WM_CREATE:
        auto& createParams = *reinterpret_cast<CREATESTRUCT*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(createParams.lpCreateParams));
        break;
    }
    auto* self = reinterpret_cast<SimpleWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (self) {
      return self->HandleMessageInternal(hWnd, msg, wParam, lParam);
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
  };

  LRESULT HandleMessageInternal(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    if (this->HandleMessage(hWnd, msg, wParam, lParam)) {
      return 0; // handled
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
struct ImGuiD3D11Window : SimpleWindow
{
  std::unique_ptr<D3DAppContext> mD3DContext;
  bool mInSizeMove = false;
  static constexpr UINT kRenderTimer = 1;
  std::function<void()> onRenderFrame;

  ImGuiD3D11Window()
    : SimpleWindow("Clarinoid Test Bench", 1200, 900)
  {

    ShowWindow(mhWnd, SW_SHOWDEFAULT);
    UpdateWindow(mhWnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();

    mD3DContext = std::make_unique<D3DAppContext>(mhWnd);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.IniFilename = nullptr;
    ImGui::GetIO().IniFilename = nullptr;

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(mhWnd);
    ImGui_ImplDX11_Init(mD3DContext->mpDevice.Get(), mD3DContext->mpDeviceContext.Get());
  }

  inline ~ImGuiD3D11Window()
  {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
  }

  inline void BeginFrame()
  {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
  }

  inline void EndFrame(bool vsync = true)
  {
    ImGui::Render();
    const float clear_color[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
    mD3DContext->mpDeviceContext->OMSetRenderTargets(1, mD3DContext->mpRenderTargetView.GetAddressOf(), NULL);
    mD3DContext->mpDeviceContext->ClearRenderTargetView(mD3DContext->mpRenderTargetView.Get(), clear_color);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    mD3DContext->mpSwapChain->Present(vsync ? 1 : 0, 0); // Present with vsync
  }

  // one-shot frame (used by main loop and by WM_TIMER while sizing)
  inline void RenderOneFrame(bool vsync = true)
  {
    BeginFrame();
    if (onRenderFrame)
      onRenderFrame();
    EndFrame(vsync);
  }
  virtual inline bool HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
  {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
      return true;
    switch (msg) {

      case WM_ENTERSIZEMOVE:
        mInSizeMove = true;
        // ~60 FPS while dragging; use 0 vsync inside timer frames
        SetTimer(hWnd, kRenderTimer, 16, nullptr);
        return true;

      case WM_EXITSIZEMOVE:
        mInSizeMove = false;
        KillTimer(hWnd, kRenderTimer);
        InvalidateRect(hWnd, nullptr, FALSE); // force a repaint once done
        return true;

      case WM_TIMER:
        if (wParam == kRenderTimer && mInSizeMove) {
          // Render with no vsync to avoid stalls during drag
          RenderOneFrame(/*presentSync*/ false);
          return true;
        }
        break;

      // Optional: during SIZING ask for paints; don't resize swapchain here
      case WM_SIZING:
        InvalidateRect(hWnd, nullptr, FALSE);
        return true;
      case WM_SIZE:
        if (mD3DContext && wParam != SIZE_MINIMIZED) {
          mD3DContext->Resize((int)LOWORD(lParam), (int)HIWORD(lParam));
        }
        return true;
      case WM_DESTROY:
        PostQuitMessage(0);
        return true;
        // case WM_SYSCOMMAND:
        //    if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
        //        return 0;
        //    break;
    }
    return false;
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
struct ImGuiWindowApp
{
  ImGuiD3D11Window mAppWindow;
  double mTargetFps = 60;

  virtual void inline RenderFrame() = 0;

  inline void Main()
  {
    using clock = std::chrono::steady_clock;
    const auto frame_interval =
      std::chrono::duration_cast<clock::duration>(std::chrono::duration<double>(1.0 / mTargetFps));

    mAppWindow.onRenderFrame = [this]() { this->RenderFrame(); };

    auto next_frame = clock::now(); // schedule first frame at "now"

    bool done = false;
    while (!done) {
      MSG msg;
      while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT)
          done = true;
      }
      if (done)
        break;

      // Sleep until the next frame time, but wake early for input/messages.
      auto now = clock::now();
      if (now < next_frame) {
        DWORD timeout_ms = (DWORD)std::chrono::duration_cast<std::chrono::milliseconds>(next_frame - now).count();
        // Wake early if there is any kind of input/message.
        MsgWaitForMultipleObjectsEx(0, nullptr, timeout_ms, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
        continue; // loop to pump messages before deciding to render
      }

      // Time to render a frame
      if (!mAppWindow.mInSizeMove) {
        mAppWindow.RenderOneFrame(); // should call Present(0,0) if you want this cap independent of vsync
      }

      // Schedule the next frame, catching up if we fell behind
      next_frame += frame_interval;
      while (next_frame <= clock::now()) {
        next_frame += frame_interval; // avoid drift if a frame took long
      }
    }
  }
};
