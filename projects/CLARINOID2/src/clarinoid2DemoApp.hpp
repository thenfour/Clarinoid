#pragma once

#include <array>
#include <algorithm>
#include <cstdint>
#include <cmath>

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/menu/MenuSettings.hpp>
#include <clarinoid/menu/Plotter.hpp>

namespace clarinoid
{

struct MusicalStateTask;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct DemoApp : DisplayApp
{
    MusicalStateTask &mMusicalStateTask;

    // Fire texture / classic doom fire cellular model. Heat is injected along the
    // bottom edge, rises by averaging neighbouring cells, then cools and flickers with noise.
    // Heat values are stored as uint8_t (Q0.8 style 0–255, where 255 represents max heat).
    // The result is gamma-shaped back to 0–255 brightness for dithering output.
    struct FireParams
    {
        static constexpr int kRenderWidth = 100;        // Width of the simulated grid in pixels
        static constexpr int kRenderHeight = 54;        // Height of the grid in pixels
        static constexpr uint8_t kBaseCooling = 7;      // Heat units (0–255) subtracted from bottom row per frame
        static constexpr uint8_t kPropagationDecay = 2; // Heat units subtracted after neighbour average (0–255 space)
        static constexpr uint8_t kNoiseMask = 0x03;     // Bitmask applied to RNG (0–7) for stochastic flicker
        static constexpr uint8_t kNeighborAverageShift = 2; // Fixed-point right shift (2 -> divide by 4) when averaging
        static constexpr uint8_t kIgnitionProbability =
            10;                                      // Probability threshold (0–255) for spawning a new bottom spark
        static constexpr uint8_t kIgnitionMin = 160; // Minimum seed heat (0–255) when ignition fires
        static constexpr uint8_t kIgnitionRangeMask = 0x3F; // Additional random seed heat (mask => 0–63 additive)
        static constexpr uint8_t kEmbersFloor = 1;          // Lower bound heat (0–255) keeping embers glowing
        static constexpr uint8_t kOutputContrast = 255;     // Gain applied to squared heat (0–255 -> 0–255 brightness)
        static constexpr uint8_t kOutputBase = 0; // Brightness bias (0–255) added after contrast for ambient glow

        static constexpr int kPixelCount = kRenderWidth * kRenderHeight;
    };

    std::array<uint8_t, FireParams::kPixelCount> mFireBuffer{};  // Active heat field (Q0.8 0–255 intensity)
    std::array<uint8_t, FireParams::kPixelCount> mFireScratch{}; // Scratch buffer (same units as mFireBuffer)
    bool mFireInitialized = false;                               // Lazy init gate so we zero buffers once
    uint32_t mRngState = 0xA5A5F00Du;                            // 32-bit LCG state feeding all faux-random behaviour

    DemoApp(IDisplay &d, MusicalStateTask &musicalStateTask) : DisplayApp(d), mMusicalStateTask(musicalStateTask)
    {
    }

    virtual const char *DisplayAppGetName() override
    {
        return "demo viz";
    }

    virtual void UpdateApp() override
    {
        if (mBack.IsNewlyPressed())
        {
            GoToFrontPage();
        }
    }

    virtual void DisplayAppUpdate() override
    {
        DisplayApp::DisplayAppUpdate();
    }

    virtual void RenderApp() override
    {
        StepFireSimulation();
        RenderFireFrame();
    }

    virtual void RenderFrontPage() override
    {
        mDisplay.setCursor(0, 0);
        mDisplay.println("demo: fire");
        mDisplay.println("press ok");
        mDisplay.println("back to exit");
    }

  private:
    // Lazily clear buffers so we do not pay for memset each frame.
    void EnsureFireInitialized()
    {
        if (mFireInitialized)
        {
            return;
        }
        mFireBuffer.fill(0);
        mFireScratch.fill(0);
        mFireInitialized = true;
    }

    // Simple LCG returning the high byte for jitter / noise usage.
    uint8_t NextRandomByte()
    {
        mRngState = (mRngState * 1664525u) + 1013904223u;
        return static_cast<uint8_t>(mRngState >> 24);
    }

    // Advance the heat field one tick: cool bottom, seed sparks, then propagate upwards.
    void StepFireSimulation()
    {
        EnsureFireInitialized();

        auto &src = mFireBuffer;
        auto &dst = mFireScratch;
        constexpr int width = FireParams::kRenderWidth;
        constexpr int height = FireParams::kRenderHeight;
        const int bottomRowOffset = (height - 1) * width;

        // Bottom row update: cool existing heat (0–255), optionally ignite new sparks.
        for (int col = 0; col < width; ++col)
        {
            const int idx = bottomRowOffset + col;
            uint8_t cooled = src[idx];
            cooled = (cooled > FireParams::kBaseCooling) ? static_cast<uint8_t>(cooled - FireParams::kBaseCooling) : 0;
            cooled = std::max<uint8_t>(cooled, FireParams::kEmbersFloor);

            if (NextRandomByte() < FireParams::kIgnitionProbability)
            {
                const uint8_t spark = static_cast<uint8_t>(FireParams::kIgnitionMin +
                                                           (NextRandomByte() & FireParams::kIgnitionRangeMask));
                cooled = std::max<uint8_t>(cooled, spark);
            }

            src[idx] = cooled;
        }

        // Propagate heat upwards by averaging neighbours beneath each cell (integer divide keeps Q0.8 scaling).
        for (int row = 0; row < height - 1; ++row)
        {
            for (int col = 0; col < width; ++col)
            {
                const int idx = row * width + col;
                const int below = idx + width;
                const int belowLeft = below + ((col > 0) ? -1 : 0);
                const int belowRight = below + ((col + 1 < width) ? 1 : 0);
                const int below2 = (row + 2 < height) ? below + width : below;

                uint32_t sum = src[below];
                sum += src[belowLeft];
                sum += src[belowRight];
                sum += src[below2];

                uint32_t averaged = sum >> FireParams::kNeighborAverageShift;
                int32_t val = static_cast<int32_t>(averaged);
                val -= FireParams::kPropagationDecay;
                val -= static_cast<int32_t>(NextRandomByte() & FireParams::kNoiseMask);
                val = std::max<int32_t>(val, 0);

                dst[idx] = static_cast<uint8_t>(val);
            }
        }

        // Preserve bottom row so fresh seeds feed the next frame (same Q0.8 units).
        for (int col = 0; col < width; ++col)
        {
            const int idx = bottomRowOffset + col;
            dst[idx] = src[idx];
        }

        mFireBuffer.swap(mFireScratch);
    }

    // Convert the heat field (0–255 heat) to 0–255 brightness using intensity^2 for pseudo-gamma.
    void RenderFireFrame()
    {
        const auto clientRect = mDisplay.GetClientRect();
        const int originX = clientRect.x + std::max(0, (clientRect.width - FireParams::kRenderWidth) / 2);
        const int originY = clientRect.y + std::max(0, (clientRect.height - FireParams::kRenderHeight) / 2);
        const RectI renderRect =
            RectI::Construct(originX, originY, FireParams::kRenderWidth, FireParams::kRenderHeight);

        mDisplay.SetClipRect(renderRect);

        for (int row = 0; row < FireParams::kRenderHeight; ++row)
        {
            for (int col = 0; col < FireParams::kRenderWidth; ++col)
            {
                const int idx = row * FireParams::kRenderWidth + col;
                const uint8_t intensity = mFireBuffer[idx];                   // Heat Q0.8 (0–255)
                int32_t brightness = (intensity * intensity) >> 8;            // Heat^2 scaled back to 0–255
                brightness = (brightness * FireParams::kOutputContrast) >> 8; // Contrast in 0–255 space
                brightness =
                    ClampInclusive<int32_t>(brightness + FireParams::kOutputBase, 0, 255); // Final biased brightness

                const PointI pt = PointI::Construct(renderRect.x + col, renderRect.y + row);
                mDisplay.SetPixelShaded(pt, brightness);
            }
        }

        mDisplay.ResetClip();
    }
};

} // namespace clarinoid
