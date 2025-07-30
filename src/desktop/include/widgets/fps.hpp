#pragma once
#include <imgui.h>
#include <chrono>
#include <cmath>
#include <algorithm>

class GuiFps
{
public:
  using clock = std::chrono::steady_clock;

  explicit GuiFps(double half_life_sec = 0.5)
    : mHalfLife(half_life_sec > 0.0 ? half_life_sec : 0.5)
  {
  }

  void beginFrame()
  {
    auto now = clock::now();

    if (mHasPrevStart) {
      mLastActualDtSec = std::chrono::duration<double>(now - mPrevStart).count();
      const double a = alphaFromDt(mLastActualDtSec, mHalfLife);
      if (!mActualInited) {
        mEmaActualDt = mLastActualDtSec;
        mActualInited = true;
      } else {
        mEmaActualDt += a * (mLastActualDtSec - mEmaActualDt);
      }
    } else {
      mLastActualDtSec = 0.0;
      mActualInited = false;
    }

    mFrameStart = now;
    mPrevStart = now;
    mHasPrevStart = true;
  }

  void endFrame()
  {
    auto t1 = clock::now();
    const double render_dt = std::chrono::duration<double>(t1 - mFrameStart).count();

    // Use the same per-sample alpha based on elapsed wall time between frames.
    const double dt_for_alpha = (mLastActualDtSec > 0.0) ? mLastActualDtSec : render_dt;
    const double a = alphaFromDt(dt_for_alpha, mHalfLife);

    if (!mRenderInited) {
      mEmaRenderDt = render_dt;
      mRenderInited = true;
    } else {
      mEmaRenderDt += a * (render_dt - mEmaRenderDt);
    }
  }

  void renderImGui(const char* id = "FPS") const
  {
    (void)id; // kept for consistency/IDs if you wrap in a group/window
    const double actual_ms = mActualInited ? (mEmaActualDt * 1000.0) : 0.0;
    const double render_ms = mRenderInited ? (mEmaRenderDt * 1000.0) : 0.0;
    const double actual_fps = toFps(mEmaActualDt);
    const double theoretical_fps = toFps(mEmaRenderDt);

    ImGui::TextDisabled("FPS : %.0f (%.2f ms)",
                        actual_fps,
                        //actual_ms,
                        //theoretical_fps,
                        render_ms);
  }

  double actualFps() const { return toFps(mEmaActualDt); }
  double theoreticalFps() const { return toFps(mEmaRenderDt); }
  double actualMs() const { return mEmaActualDt * 1000.0; }
  double renderMs() const { return mEmaRenderDt * 1000.0; }

  struct Scope
  {
    GuiFps& m;
    explicit Scope(GuiFps& g)
      : m(g)
    {
      m.beginFrame();
    }
    ~Scope() { m.endFrame(); }
  };

private:
  // EMA helper: per-sample alpha computed from elapsed time and half-life
  static double alphaFromDt(double dt, double halfLife)
  {
    if (halfLife <= 0.0)
      return 1.0;
    // a = 1 - exp(-ln(2) * dt / halfLife)
    const double k = std::log(2.0) / halfLife;
    double a = 1.0 - std::exp(-k * std::max(0.0, dt));
    if (a < 0.0)
      a = 0.0;
    if (a > 1.0)
      a = 1.0;
    return a;
  }

  static double toFps(double dt_sec) { return (dt_sec > 1e-9) ? (1.0 / dt_sec) : 0.0; }

  // Smoothing config
  double mHalfLife;

  // Timing state
  clock::time_point mPrevStart{};
  clock::time_point mFrameStart{};
  bool mHasPrevStart = false;
  double mLastActualDtSec = 0.0;

  // EMA state
  double mEmaActualDt = 0.0;
  double mEmaRenderDt = 0.0;
  bool mActualInited = false;
  bool mRenderInited = false;
};
