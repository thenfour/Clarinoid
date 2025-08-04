
#pragma once

#include <imgui.h>
#include <vector>
#include <algorithm>
#include <cfloat>
#include "SeriesCollection.hpp"
#include "TimeSeriesStore.hpp"
#include <ImguiUtils.hpp>

class PlotterView
{
public:
  explicit PlotterView(SeriesCollection* reg, TimeSeriesStore* store)
    : mReg(reg)
    , mStore(store)
  {
  }

  void RenderImGui(const char* id = "Plotter")
  {
    ImGui::PushID(id);

    // --- Toolbar ---
    if (ImGui::BeginChild("##toolbar", ImVec2(0, ImGui::GetFrameHeightWithSpacing() + 200), ImGuiChildFlags_Borders, ImGuiWindowFlags_MenuBar)) {

        ImGui::BeginMenuBar();

      //ImGui::SameLine();
      ImGui::Checkbox("Auto-fit Y", &mAutoFitY);

      ImGui::SameLine();
      ImGui::BeginDisabled(mAutoFitY);
      ImGui::SetNextItemWidth(80);
      ImGui::InputFloat("Y min", &mYMin, 0, 0, "%.3f");
      ImGui::SameLine();
      ImGui::SetNextItemWidth(80);
      ImGui::InputFloat("Y max", &mYMax, 0, 0, "%.3f");
      ImGui::EndDisabled();

       ImGui::SameLine();
       ImGui::SetNextItemWidth(80);
       ImGui::SliderInt("VisibleSamples", &mVisibleSamples, 40, 400, "%d");

      ImGui::SameLine();
      //ImGui::Checkbox("Dots", &mShowDots);
      ImGui::SetNextItemWidth(80);
      ImGui::SliderFloat("Dot alpha", &mDotOpacity, 0, 1.f, "%.2f");
      ImGui::SameLine();
      ImGui::SetNextItemWidth(80);
      ImGui::SliderFloat("Dot size", &mDotRadius, 1.0f, 10.0f, "%.1f");

      ImGui::SameLine();
      //ImGui::Checkbox("Lines", &mShowLines);
      ImGui::SetNextItemWidth(80);
      ImGui::SliderFloat("Line alpha", &mLineOpacity, 0, 1.f, "%.2f");
      ImGui::SameLine();
      ImGui::SetNextItemWidth(80);
      ImGui::SliderFloat("Line width", &mLineThickness, 0.5f, 4.0f, "%.1f");

      ImGui::EndMenuBar();
      //// Legend (visibility + colors from SeriesCollection) -- todo: move to some global area together with pause / clear / etc.
      //auto rs = mReg->snapshot(false);
      //bool first = true;
      //for (auto& s : rs.items) {
      //  ImGui::PushID(s.id);
      //  if (!first)
      //    ImGui::SameLine(0, 16);
      //  first = false;
      //  ImVec4 col = ImGui::ColorConvertU32ToFloat4(s.color);
      //  ImGui::ColorButton("##col", col, ImGuiColorEditFlags_NoTooltip, ImVec2(14, 14));
      //  ImGui::SameLine();
      //  bool v = s.visible;
      //  if (ImGui::Checkbox(s.name.c_str(), &v))
      //    mReg->setVisible(s.id, v);
      //  ImGui::PopID();
      //}
    }
    //ImGui::EndChild();
    //ImGui::Separator();

    // --- Plot area ---
    //ImGui::BeginChild("##plot",
    //                  ImVec2(0, 0),
    //                  true,
    //                  ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImVec2 avail = ImGui::GetContentRegionAvail();
    const float pad = 6.0f;
    ImVec2 plotMin = { p0.x + pad, p0.y + pad };
    ImVec2 plotMax = { p0.x + avail.x - pad, p0.y + avail.y - pad };
    float plotW = std::max(1.0f, plotMax.x - plotMin.x);
    float plotH = std::max(1.0f, plotMax.y - plotMin.y);

    // Background/grid from scheme
    auto& scheme = mReg->scheme();
    dl->AddRectFilled(plotMin, plotMax, scheme.bg);
    dl->AddRect(plotMin, plotMax, ImGui::GetColorU32(ImGuiCol_Border));

    // Build visible id list
    auto regSnap = mReg->snapshot(false);
    std::vector<SeriesCollection::Id> ids;
    ids.reserve(regSnap.items.size());
    std::vector<ImU32> colors;
    colors.reserve(regSnap.items.size());
    std::vector<std::string> names;
    names.reserve(regSnap.items.size());
    for (auto& s : regSnap.items)
      if (s.visible) {
        ids.push_back(s.id);
        colors.push_back(s.color);
        names.push_back(s.name);
      }

    // Data snapshot (live or at pause head)
    TimeSeriesStore::Snapshot snap;
    if (!ids.empty()) {
      snap = mStore->snapshotLast(ids, mVisibleSamples);
    }
    const size_t N = snap.count;

    // Y range
    float yMin = -1.f, yMax = +1.f;
    if (mAutoFitY) {
      float lo = FLT_MAX, hi = -FLT_MAX;
      for (auto& s : snap.series)
        for (size_t i = 0; i < N; ++i) {
          float v = s.data[i];
          if (std::isnan(v))
            continue;
          lo = std::min(lo, v);
          hi = std::max(hi, v);
        }
      if (lo == FLT_MAX) {
        yMin = -1;
        yMax = +1;
      } else if (lo == hi) {
        yMin = lo - 1;
        yMax = hi + 1;
      } else {
        yMin = lo;
        yMax = hi;
      }

      // track the auto calculated values
      mYMin = yMin;
      mYMax = yMax;

    } else {
      yMin = std::min(mYMin, mYMax);
      yMax = std::max(mYMin, mYMax);
      if (yMin == yMax) {
        yMin -= 1;
        yMax += 1;
      }
    }

    // Grid + labels
    drawGrid_(dl, plotMin, plotMax, yMin, yMax, scheme.grid);

    if (N >= 2 && !snap.series.empty()) {

      // Geometry container for one series
      struct PolyGeom
      {
        std::vector<ImVec2> pts; // all finite points (screen space)
        struct Span
        {
          int start, count;
        };
        std::vector<Span> spans; // contiguous runs within pts
        void clear()
        {
          pts.clear();
          spans.clear();
        }
      };

      // Builds geometry for a single series: splits on NaN, maps to X/Y
      auto buildGeom = [&](const float* data, size_t N, float x0, float w, auto&& toY, PolyGeom& out) {
        out.clear();
        out.pts.reserve(N); // upper bound; actual may be less due to NaNs
        out.spans.reserve(8);

        const float invN1 = (N > 1) ? (1.0f / float(N - 1)) : 0.0f;
        bool in_span = false;
        int span_start = 0;

        for (size_t i = 0; i < N; ++i) {
          float v = data[i];
          if (!std::isfinite(v)) {
            if (in_span) {
              // close span
              out.spans.push_back({ span_start, int(out.pts.size() - span_start) });
              in_span = false;
            }
            continue;
          }
          float x = x0 + float(i) * invN1 * w;
          float y = toY(v);

          if (!in_span) {
            span_start = (int)out.pts.size();
            in_span = true;
          }
          out.pts.emplace_back(x, y);
        }
        if (in_span) {
          out.spans.push_back({ span_start, int(out.pts.size() - span_start) });
        }
      };

      // Issues draw calls (lines then dots) for a built geometry
      auto drawGeom = [&](ImDrawList* dl, const PolyGeom& g, ImU32 col) {
        // Lines first (so dots overlay)
        if (mLineOpacity > 0.0001f) {
          auto lineCol = apply_opacity_mul(col, mLineOpacity);
          for (const auto& s : g.spans) {
            if (s.count >= 2) {
              dl->AddPolyline(&g.pts[s.start], s.count, lineCol, 0 /*closed*/, mLineThickness);
            }
          }
        }
        // Dots on top
        if (mDotOpacity >0.0001f) {
            auto dotCol = apply_opacity_mul(col, mDotOpacity);
          for (const auto& p : g.pts) {
            dl->AddCircleFilled(p, mDotRadius, dotCol);
          }
        }
      };

      // basically, if there are more samples than pixels, a different rendering must be employed
      const bool dense = (int)N > (int)plotW * 2;

      auto toY = [&](float v) {
        float t = (v - yMin) / (yMax - yMin);
        t = 1.0f - t;
        return plotMin.y + t * plotH;
      };

      if (!dense) {
        // Clip all dots/lines to the plot rect (optional but nice)
        ImGui::PushClipRect(plotMin, plotMax, true);

        PolyGeom geom; // scratch reused per series (reduces allocations)
        for (size_t si = 0; si < snap.series.size(); ++si) {
          const auto& s = snap.series[si];
          ImU32 col = colors[si];

          buildGeom(s.data, N, plotMin.x, plotW, toY, geom);
          if (!geom.pts.empty())
            drawGeom(dl, geom, col);
        }
        ImGui::PopClipRect();
      } else {
        // High-density: per-pixel min/max bars (unchanged)
        const int px = (int)plotW;
        if (px > 0) {
          for (size_t si = 0; si < snap.series.size(); ++si) {
            const auto& s = snap.series[si];
            ImU32 col = colors[si];

            for (int x = 0; x < px; ++x) {
              size_t s0 = (size_t)((uint64_t)x * N / px);
              size_t s1 = (size_t)((uint64_t)(x + 1) * N / px);
              if (s1 <= s0)
                s1 = s0 + 1;
              if (s1 > N)
                s1 = N;

              float vmin = FLT_MAX, vmax = -FLT_MAX;
              bool any = false;
              for (size_t k = s0; k < s1; ++k) {
                float v = s.data[k];
                if (!std::isfinite(v))
                  continue;
                any = true;
                vmin = std::min(vmin, v);
                vmax = std::max(vmax, v);
              }
              if (!any)
                continue;

              float y0 = toY(vmin), y1 = toY(vmax);
              if (fabsf(y1 - y0) < 0.5f)
                y1 = y0 + 1.0f;
              float fx = plotMin.x + (float)x + 0.5f;
              dl->AddLine(ImVec2(fx, y0), ImVec2(fx, y1), col);
            }
          }
        }
      }

      // Hover tooltip
      if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(plotMin, plotMax, true)) {
        ImVec2 mp = ImGui::GetMousePos();
        float t = (plotW > 0 ? (mp.x - plotMin.x) / plotW : 0.f);
        t = t < 0 ? 0 : (t > 1 ? 1 : t);
        size_t idx = (size_t)llroundf(t * float(N - 1));
        float xIdx = plotMin.x + (N > 1 ? float(idx) / float(N - 1) * plotW : 0.f);
        dl->AddLine(ImVec2(xIdx, plotMin.y), ImVec2(xIdx, plotMax.y), IM_COL32(255, 255, 255, 64));

        ImGui::BeginTooltip();
        ImGui::TextDisabled("Sample %zu / %zu", idx, N - 1);
        for (size_t si = 0; si < snap.series.size(); ++si) {
          float v = snap.series[si].data[idx];
          if (std::isnan(v))
            continue;
          ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(colors[si]));
          ImGui::Text("%s: %.6g", names[si].c_str(), (double)v);
          ImGui::PopStyleColor();
        }
        ImGui::EndTooltip();
      }
    }

    ImGui::EndChild();
    ImGui::PopID();
  }

  // Optional policy: clearing clears the shared store or just the view
  void setClearClearsStore(bool on) { mClearClearsStore = on; }

private:
  // simple grid + labels
  static void drawGrid_(ImDrawList* dl, ImVec2 minp, ImVec2 maxp, float yMin, float yMax, ImU32 gridCol)
  {
    const int rows = 4;
    for (int i = 0; i <= rows; ++i) {
      float t = (float)i / (float)rows;
      float y = minp.y + (1.0f - t) * (maxp.y - minp.y);
      dl->AddLine(ImVec2(minp.x, y), ImVec2(maxp.x, y), gridCol);
    }
    char buf[64];
    ImVec2 off(4.0f, -ImGui::GetTextLineHeight() * 0.5f);
    for (int i = 0; i <= rows; ++i) {
      float t = (float)i / (float)rows;
      float v = yMin + t * (yMax - yMin);
      snprintf(buf, sizeof(buf), "%.3g", v);
      float y = minp.y + (1.0f - t) * (maxp.y - minp.y);
      dl->AddText(ImVec2(minp.x + off.x, y + off.y), ImGui::GetColorU32(ImGuiCol_TextDisabled), buf);
    }
  }

  // state
  SeriesCollection* mReg = nullptr;
  TimeSeriesStore* mStore = nullptr;
  int mVisibleSamples = 90;

  // view options
  bool mAutoFitY = true;
  float mDotOpacity = 1.f;
  float mLineOpacity = 0.8f;
  //bool mShowDots = false;
  //bool mShowLines = true;
  bool mClearClearsStore = false;
  float mDotRadius = 3.0f;
  float mLineThickness = 1.7f;
  float mYMin = -1.0f, mYMax = +1.0f;
};
