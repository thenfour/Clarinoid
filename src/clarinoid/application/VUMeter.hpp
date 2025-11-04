#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/synth/Analysis.hpp>
#include <clarinoid/application/DisplayDefs.hpp>

namespace clarinoid
{

struct VUMeterConfig
{
    RectI rcDisplay = {0, 0, 24, 24};
};

void RenderVUMeterHoriz(IDisplay &display, const AudioAnalysisState &state, const VUMeterConfig &config)
{
    // hard-coded 1px border,
    display.DrawRectOutline1px(config.rcDisplay, SSD1306_WHITE);

    // left edge = 0 linear (-inf dB)
    // right edge = max db
    static constexpr float maxDb = +1.0f;
    const float maxLinear = DecibelsToLinear(maxDb);

    // inner drawable region (exclude the border we just drew)
    RectI inner = RectI::Construct(config.rcDisplay.x + 1,
                                   config.rcDisplay.y + 1,
                                   std::max(0, config.rcDisplay.width - 2),
                                   std::max(0, config.rcDisplay.height - 2));
    if (inner.width <= 0 || inner.height <= 0)
    {
        return;
    }

    // clear inside before drawing levels
    display.fillRect(inner.x, inner.y, inner.width, inner.height, SSD1306_BLACK);

    auto clampLinear = [&](float value) { return Clamp(value, 0.0f, maxLinear); };

    // nominal values are quite low, so apply a curve to make the meter more responsive at low levels.
    auto applyMeterCurve = [](float normalized) {
        normalized = Clamp(normalized, 0.0f, 1.0f);
        return std::sqrt(normalized);
    };

    auto linearToWidth = [&](float value) -> int {
        float normalized = 0.0f;
        if (maxLinear > 0.0f)
        {
            normalized = clampLinear(value) / maxLinear;
        }
        normalized = applyMeterCurve(normalized);
        return static_cast<int>(std::roundf(normalized * inner.width));
    };

    auto linearToX = [&](float value) -> int {
        if (inner.width <= 0)
        {
            return inner.x;
        }
        float normalized = 0.0f;
        if (maxLinear > 0.0f)
        {
            normalized = clampLinear(value) / maxLinear;
        }
        normalized = applyMeterCurve(normalized);
        float x = static_cast<float>(inner.x) + normalized * static_cast<float>(inner.width - 1);
        return static_cast<int>(std::roundf(x));
    };

    // peak value is shaded using FillRectWithBrightness
    static constexpr uint8_t peakBrightnessQp8 = 128;
    // because internal routing is 16-bit audio, >0db is not possible.
    // so use the clipping indicator instead.
    // if we're clipping, act as if peak level is maximum.
    float peakLinear01 = state.peakLinear;
    if (state.isClipping)
    {
        peakLinear01 = maxLinear;
    }
    int peakWidth = linearToWidth(peakLinear01);
    if (peakWidth > 0)
    {
        RectI peakRect = RectI::Construct(inner.x, inner.y, std::min(peakWidth, inner.width), inner.height);
        display.FillRectWithBrightness(peakRect, peakBrightnessQp8);
    }

    // RMS value is shaded as white (100% coverage)
    int rmsWidth = linearToWidth(state.rmsLinear);
    if (rmsWidth > 0)
    {
        display.fillRect(inner.x, inner.y, std::min(rmsWidth, inner.width), inner.height, SSD1306_WHITE);
    }

    // the 0dB point is shown with a vertical line.
    int zeroDbX = linearToX(1.0f);
    display.drawFastVLine(zeroDbX, inner.y, inner.height, SSD1306_WHITE);

    // the peak held value is shown as a vertical line.
    int heldPeakX = linearToX(state.heldPeakLinear);
    display.drawFastVLine(heldPeakX, inner.y, inner.height, SSD1306_WHITE);
}

} // namespace clarinoid
