#pragma once

#include "D3DAppContext.hpp"

struct ClarinoidSimulatorApp
{
  MonoBitmapTexture mSSD1306Texture;

  ClarinoidSimulatorApp(D3DAppContext* pd3dAppContext)
    : mSSD1306Texture(pd3dAppContext)
  {
    //
  }

  ImColor mFg = "#6aa"_imu32;
  ImColor mBg = "#081010"_imu32;
  bool mShowDemoWindow = false;

  void Render()
  {

      static bool use_work_area = true;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;

    // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
    // Based on your use case you may want one or the other.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
    ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

    if (ImGui::Begin("Clarinoid simulator", nullptr, flags)) {

    if (ImGui::BeginMenuBar()) {

        if (ImGui::BeginMenu("Menu")) {
        ImGui::MenuItem("Show demo window", nullptr, &mShowDemoWindow);
          ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
      }


      static int pattern1 = 2;
      ImGui::SliderInt("pattern1", &pattern1, 1, 100);

      static int pattern2 = 3;
      ImGui::SliderInt("pattern2", &pattern2, 1, 100);

      static int samples = 10;
      ImGui::SliderInt("samples", &samples, 3, 100);

      std::array<bool, 128 * 64> ssd1306_mono_buffer;

      oled128x64::FillLissajous(
        ssd1306_mono_buffer, ImGui::GetTime() * 0.3f, pattern1, pattern2, 0.0f, 1.0f, 0, samples, 1,
          ImGui::GetTime() * 2.0f
          );

      mSSD1306Texture.Update(ssd1306_mono_buffer, mFg, mBg);
      mSSD1306Texture.Draw(4);
      ImGui::SameLine();
      mSSD1306Texture.Draw(2);
      ImGui::SameLine();
      mSSD1306Texture.Draw(1);

      // float fgrgb[3] = {
      //   mFg.Value.x,
      //   mFg.Value.y,
      //   mFg.Value.z,
      // };
      // ImGui::ColorEdit3("foreground", fgrgb);
      // mFg = ImColor{ fgrgb[0], fgrgb[1], fgrgb[2] };

      // float bgrgb[3] = {
      //   mBg.Value.x,
      //   mBg.Value.y,
      //   mBg.Value.z,
      // };
      // ImGui::ColorEdit3("background", bgrgb);
      // mBg = ImColor{ bgrgb[0], bgrgb[1], bgrgb[2] };

      //// todo: create LED components, encoders, etc...
      //// audio simulator can show oscilloscope, spectrum, etc..
    }
    ImGui::End();

    //static bool demoOpen = false;
    if (mShowDemoWindow) {
      ImGui::ShowDemoWindow();
    }
  }
};
