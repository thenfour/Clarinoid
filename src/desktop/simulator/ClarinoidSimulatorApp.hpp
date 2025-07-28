#pragma once
#include <algorithm>
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

      static int samples = 19;
      ImGui::SliderInt("samples", &samples, 3, 100);

      static float speed = 0.1f;
      ImGui::SliderFloat("spin", &speed, 0, 1);

      static float phaseShiftSpeed = 0.0f;
      ImGui::SliderFloat("phaseShiftSpeed", &phaseShiftSpeed, 0, 1);

      static float sampleShiftSpeed = 0.1f;
      ImGui::SliderFloat("sampleShiftSpeed", &sampleShiftSpeed, 0, 1);

      static float thickness = 0.0f;
      ImGui::SliderFloat("thickness", &thickness, 0, 10);

      std::array<bool, 128 * 64> ssd1306_mono_buffer;

      oled128x64::FillLissajous(ssd1306_mono_buffer,
                                ImGui::GetTime() * speed,
                                pattern1,
                                pattern2,
                                ImGui::GetTime() * phaseShiftSpeed,
                                1.0f,
                                thickness,
                                samples,
                                1,
                                ImGui::GetTime() * sampleShiftSpeed);

      mSSD1306Texture.Update(ssd1306_mono_buffer, mFg, mBg);
      mSSD1306Texture.Draw(4);

      ImVec2 imgMin = ImGui::GetItemRectMin();
      ImVec2 mouse = ImGui::GetMousePos();
      int scaleBase = 4; // the scale you used above

      // Convert mouse to source pixel coords (clamp)
      int px = int((mouse.x - imgMin.x) / float(scaleBase));
      int py = int((mouse.y - imgMin.y) / float(scaleBase));
      px = std::clamp(px, 0, mSSD1306Texture.GetWidth() - 1);
      py = std::clamp(py, 0, mSSD1306Texture.GetHeight() - 1);

      ImGui::SameLine();

      mSSD1306Texture.DrawMagnifierOutlineOverImage(px, py, 24, 24, 4, "#f80"_imu32, 1);
      mSSD1306Texture.DrawMagnifier(px,
                                    py,
                                    /*regionW*/ 24,
                                    /*regionH*/ 24,
                                    /*scale*/ 16,
                                    /*grid*/ "#122"_imu32,
                                    /*border*/ "#f80"_imu32,
                                    /*cross*/ "#355"_imu32,
                                    /*cellOutline*/ "#ff0"_imu32
          );
      //ImGui::Text("(%d, %d)", px, py);

      mSSD1306Texture.Draw(2);
      ImGui::SameLine();
      mSSD1306Texture.Draw(1);

    }
    ImGui::End();

    // static bool demoOpen = false;
    if (mShowDemoWindow) {
      ImGui::ShowDemoWindow();
    }
  }
};
