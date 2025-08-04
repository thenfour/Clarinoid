#pragma once
#include <algorithm>

#include "./CommandLine.hpp"
#include "./D3DAppContext.hpp"
#include "./PolledValue.hpp"
#include "./SeriesCollection.hpp"
#include "./TelemetryIngestor.hpp"
#include "./TimeSeriesStore.hpp"
#include "./serial.hpp"
#include "./utils.hpp"
#include <StringLineDispatcher.hpp>
#include <widgets/GuiLog.hpp>
#include <widgets/Plotter.hpp>
#include <widgets/fps.hpp>


struct ClarinoidSimulatorApp
{
  MonoBitmapTexture mSSD1306Texture;
  GuiLog mLog;
  GuiFps mFps;
  SerialPort mSerial;
  Polled<std::vector<PortInfo>> mDeviceEnumeration;
  CommandLine mCommandLine;
  StringLineDispatcher mStringLineDispatcher;

  SeriesCollection mSeriesCollection;
  TimeSeriesStore mTimeSeriesStore;
  TelemetryIngestor mTelemetryIngestor{mSeriesCollection, mTimeSeriesStore};
  PlotterView mPlotter{&mSeriesCollection, &mTimeSeriesStore};

  ImColor mFg = "#6aa"_imu32;
  ImColor mBg = "#081010"_imu32;
  bool mShowDemoWindow = false;

  ClarinoidSimulatorApp(D3DAppContext* pd3dAppContext)
      : mSSD1306Texture(pd3dAppContext)
      , mDeviceEnumeration(std::chrono::milliseconds(1000),
                           [&]() -> std::vector<PortInfo>
                           {
                             return enumSerialPorts();
                           })
  {
    mStringLineDispatcher.mLineCallback = [&](const std::string& line)
    {
      mTelemetryIngestor.HandleLine(line);

      if (line.find("#") == 0)
      {
        mLog.append(line.substr(1));
      }
    };

    mSerial.setReceiveCallback(
        [&](const uint8_t* data, size_t n)
        {
          std::string msg{reinterpret_cast<const char*>(data), n};
          mStringLineDispatcher.HandleIncomingString(msg);
        });

    mCommandLine.setExecuteCallback(
        [&](const char* cmd)
        {
          if (cmd && *cmd)
          {
            mLog.append(std::string("> ") + cmd);
            if (mSerial.isOpen())
            {
              mSerial.writeString(cmd);
            }
            else
            {
              mLog.append("Serial port not open, command ignored.");
            }
          }
        });
  }

  void Render()
  {
    GuiFps::Scope _s(mFps);

    static bool use_work_area = true;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar |
                                    ImGuiWindowFlags_AlwaysVerticalScrollbar;

    // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
    // Based on your use case you may want one or the other.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
    ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

    if (ImGui::Begin("Clarinoid simulator", nullptr, flags))
    {
      if (ImGui::BeginMenuBar())
      {
        if (ImGui::BeginMenu("Menu"))
        {
          ImGui::MenuItem("Show demo window", nullptr, &mShowDemoWindow);
          ImGui::EndMenu();
        }

        // if (ImGui::BeginMenu("Devices")) {
        if (mDeviceEnumeration.snapshot().has_value())
        {
          const auto& list = *mDeviceEnumeration.snapshot().get();
          for (const auto& p : list)
          {
            if (ImGui::MenuItem(wstring_to_bytes(p.knownProduct + L"" + p.friendlyName).c_str()))
            {
              if (!mSerial.open(p))
              {
                // log.
                int a = 0;
              }
              mSerial.setDtr(true);
            }
          }
        }
        else
        {
          ImGui::Text("No serial devices found");
        }
        // ImGui::EndMenu();
        //}

        ImGui::Text(mSerial.isOpen() ? "Connected" : "Not connected");

        mFps.renderImGui();

        ImGui::EndMenuBar();
      }

      //{
      //  char buf[2000] = "parse test";
      //  ImGui::InputTextWithHint("##testinpu", "parse test", buf, 2000);
      //  TelemetryMessage msg;
      //  bool x = ParseTelemetryMessage(buf, msg);
      //  ImGui::Text(x ? "is telemetry" : "Not telemetry");
      //}

      static float phaseShiftSpeed = 0.0f;
      static float sampleShiftSpeed = 0.1f;
      static float thickness = 0.0f;
      static float speed = 0.1f;
      static int samples = 19;
      static int pattern2 = 3;
      static int pattern1 = 2;

      // ImGui::SliderInt("pattern1", &pattern1, 1, 100);
      // ImGui::SliderInt("pattern2", &pattern2, 1, 100);
      // ImGui::SliderInt("samples", &samples, 3, 100);
      // ImGui::SliderFloat("spin", &speed, 0, 1);
      //  ImGui::SliderFloat("phaseShiftSpeed", &phaseShiftSpeed, 0, 1);
      //  ImGui::SliderFloat("sampleShiftSpeed", &sampleShiftSpeed, 0, 1);
      // ImGui::SliderFloat("thickness", &thickness, 0, 10);

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
      int scaleBase = 4;  // the scale you used above

      // Convert mouse to source pixel coords (clamp)
      int px = int((mouse.x - imgMin.x) / float(scaleBase));
      int py = int((mouse.y - imgMin.y) / float(scaleBase));
      px = std::clamp(px, 0, mSSD1306Texture.GetWidth() - 1);
      py = std::clamp(py, 0, mSSD1306Texture.GetHeight() - 1);

      ImGui::SameLine();

      mSSD1306Texture.DrawMagnifierOutlineOverImage(px, py, 24, 24, 4, "#f80"_imu32, 1);
      mSSD1306Texture.DrawMagnifier(px,
                                    py,
                                    /*regionW*/ 16,
                                    /*regionH*/ 16,
                                    /*scale*/ 16,
                                    /*grid*/ "#122"_imu32,
                                    /*border*/ "#f80"_imu32,
                                    /*cross*/ "#355"_imu32,
                                    /*cellOutline*/ "#ff0"_imu32);

      ImGui::SameLine();
      ImGui::BeginGroup();
      mSSD1306Texture.Draw(2);
      //ImGui::SameLine();
      mSSD1306Texture.Draw(1);
      ImGui::EndGroup();

      // mAudioCycleScope.RenderImGui();
      // mOscilloscope.RenderImGui();
      mPlotter.RenderImGui();
      mLog.render();

      mCommandLine.Render();
    }
    ImGui::End();

    // static bool demoOpen = false;
    if (mShowDemoWindow)
    {
      ImGui::ShowDemoWindow();
    }
  }
};
