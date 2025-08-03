#include <windows.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include <string_view>
#include <cctype>
#include <array>
#include <d3d11.h>
#include <tchar.h>
#include <imgui.h>
#include <cassert>
#include <stdexcept>

#include <clarinoid/basic/Basic.hpp>

#include "HtmlColorUtils.hpp"
#include "ImguiUtils.hpp"
#include "ComPtr.hpp"
#include "SSD1306Simulator.hpp"
#include "ImGuiWindowApp.hpp"
#include "ClarinoidSimulatorApp.hpp"

#pragma comment(lib, "d3d11.lib")

struct SimulatorApp : public ImGuiWindowApp
{
  ClarinoidSimulatorApp mSimulatorApp;

  SimulatorApp() : mSimulatorApp(this->mAppWindow.mD3DContext.get()) {
  }
  virtual void RenderFrame() override { 
	  mSimulatorApp.Render();
  }
};

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
  SimulatorApp app;
  app.Main();
  return 0;
}
