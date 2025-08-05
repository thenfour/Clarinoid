#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <cmath>
#include <d3d11.h>
#include <imgui.h>
#include <stdexcept>
#include <string_view>
#include <tchar.h>
#include <vector>
#include <windows.h>


#include <clarinoid/core/basic/Basic.hpp>

#include <clarinoid/sim/ClarinoidSimulatorApp.hpp>
#include "./ComPtr.hpp"
#include "./HtmlColorUtils.hpp"
#include "./ImGuiWindowApp.hpp"
#include "./ImguiUtils.hpp"
#include "./SSD1306Simulator.hpp"


#pragma comment(lib, "d3d11.lib")

struct SimulatorApp : public ImGuiWindowApp
{
  ClarinoidSimulatorApp mSimulatorApp;

  SimulatorApp()
      : mSimulatorApp(this->mAppWindow.mD3DContext.get())
  {
  }
  virtual void RenderFrame() override
  {
    mSimulatorApp.Render();
  }
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
  SimulatorApp app;
  app.Main();
  return 0;
}
