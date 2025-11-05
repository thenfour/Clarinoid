#pragma once

#include <array>
#include <algorithm>
#include <cstdint>
#include <cmath>

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/basic/Vec.hpp>
#include <clarinoid/basic/Rendering3D.hpp>
#include <clarinoid/menu/MenuSettings.hpp>
#include <clarinoid/menu/Plotter.hpp>
#include "clarinoid2MusicalStateTask.hpp"

namespace clarinoid
{

extern uint32_t gSynthVoiceNoteOnCount;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct DemoApp : DisplayApp
{
    MusicalStateTask &mMusicalStateTask;

    // Fire texture / classic doom fire cellular model. Heat is injected along the
    // bottom edge, rises by averaging neighbouring cells, then cools and flickers with noise.
    // Heat values are stored as uint8_t (Q0.8 style 0–255, where 255 represents max heat).
    // Column-local randomness and a slow wind oscillator keep simultaneous flares visually distinct.
    // The result is gamma-shaped back to 0–255 brightness for dithering output.
    struct FireParams
    {
        static constexpr int kRenderWidth = 100;        // Width of the simulated grid in pixels
        static constexpr int kRenderHeight = 54;        // Height of the grid in pixels
        static constexpr uint8_t kBaseCooling = 3;      // Heat units (0–255) subtracted from bottom row per frame
        static constexpr uint8_t kPropagationDecay = 2; // Heat units subtracted after neighbour average (0–255 space)
        static constexpr uint8_t kNoiseMask = 0x03;     // Bitmask applied to RNG (0–7) for stochastic flicker
        static constexpr uint8_t kNeighborAverageShift = 2; // Fixed-point right shift (2 -> divide by 4) when averaging

        static constexpr uint8_t kEmbersFloor = 6; // Lower bound heat (0–255) keeping embers glowing

        static constexpr uint8_t kAmbientIgnitionProbabilityBase = 4; // Baseline ambient probability (0–255)
        static constexpr uint8_t kAmbientIgnitionMin =
            128; // Heat units (0–255) for ambient sparks; keeps them visually subtle
        static constexpr uint8_t kAmbientIgnitionRangeMask =
            0x07; // Extra random ambient heat (mask => 0–7 additive in 0–255 space)

        static constexpr int kNoteColumnMapMin = 36; // MIDI note mapped to left edge of the grid
        static constexpr int kNoteColumnMapMax = 90; // MIDI note mapped to right edge of the grid
        static constexpr uint8_t kNoteFlareHalfWidth =
            2; // Columns (Q0) added on each side of flare centre for note-on bursts
        static constexpr uint8_t kNoteFlareHeat = 255; // Heat units (0–255) deposited at flare centre on note-on
        static constexpr uint8_t kNoteFlareHeatFalloff =
            20; // Heat fall-off per column away from flare centre (0–255 units)
        static constexpr uint8_t kNoteFlareVerticalFalloff =
            110; // Heat units reduced when copying flare one row above (keeps plume noticeable)
        static constexpr uint8_t kNoteFlareJitterRadius =
            2; // Maximum integer column jitter applied to note flares (0 => centred)

        static constexpr uint8_t kColumnCoolingVariance =
            3; // Per-column signed cooling adjustment range (Q0 units, applied around kBaseCooling)
        static constexpr uint8_t kColumnIgnitionVariance =
            10; // Per-column signed ignition probability adjust (0–255 space)
        static constexpr uint8_t kColumnSparkHeatVariance =
            40; // Additional ambient & flare heat added per column (0–255 space)
        static constexpr uint8_t kColumnWindVariance =
            2; // Per-column wind offset in columns (signed, applied to drift)
        static constexpr uint8_t kColumnNoiseVarianceMask =
            0x03; // Extra per-column flicker subtracted during propagation (mask of RNG)

        static constexpr float kWindGlobalAmplitude =
            1.2f;                                         // Column drift amplitude in pixels induced by global wind LFO
        static constexpr float kWindPhaseStep = 0.02f;    // Radians advanced per frame for global wind sine
        static constexpr float kWindRowPhaseStep = 0.18f; // Additional phase offset per row to create shear
        static constexpr uint8_t kWindMaxOffset =
            3; // Clamp for total wind offset in columns (global + column + jitter)
        static constexpr uint8_t kWindNoiseMask =
            0x01; // Random extra offset magnitude (0 or up to mask) mixed into wind per sample

        static constexpr uint8_t kPropagationFlickerMask =
            0x03; // Additional random loss per propagating cell to keep flames dancing

        static constexpr uint32_t kOutputContrast = 1024; // Gain applied to squared heat (0–255 -> 0–255 brightness)
        static constexpr uint8_t kOutputBase = 0; // Brightness bias (0–255) added after contrast for ambient glow

        static constexpr int kPixelCount = kRenderWidth * kRenderHeight;
    };

    std::array<uint8_t, FireParams::kPixelCount> mFireBuffer{};  // Active heat field (Q0.8 0–255 intensity)
    std::array<uint8_t, FireParams::kPixelCount> mFireScratch{}; // Scratch buffer (same units as mFireBuffer)
    bool mFireInitialized = false;                               // Lazy init gate so we zero buffers once
    uint32_t mRngState = 0xA5A5F00Du;                            // 32-bit LCG state feeding all faux-random behaviour
    int mLastNoteOnSerial = 0;                                   // Tracks MIDI note-on counter to detect new events
    std::array<int8_t, FireParams::kRenderWidth> mColumnCoolingBias{};     // Signed delta from kBaseCooling
    std::array<int8_t, FireParams::kRenderWidth> mColumnIgnitionBias{};    // Signed tweak to ambient probability
    std::array<uint8_t, FireParams::kRenderWidth> mColumnSparkHeatBonus{}; // Extra heat for sparks/flares
    std::array<int8_t, FireParams::kRenderWidth> mColumnWindBias{};        // Column-specific drift contribution
    std::array<uint8_t, FireParams::kRenderWidth> mColumnNoiseBias{};      // Additional flicker loss per column
    float mWindPhase = 0.0f;                                               // Global wind oscillator phase

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
        SeedColumnTraits();
        mWindPhase = 0.0f;
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
        UpdateWindPhase();

        // Bottom row update: cool existing heat (0–255), optionally ignite new sparks.
        for (int col = 0; col < width; ++col)
        {
            const int idx = bottomRowOffset + col;
            uint8_t cooled = src[idx];
            const int cooling = ClampInclusive<int>(FireParams::kBaseCooling + mColumnCoolingBias[col], 0, 255);
            cooled = (cooled > cooling) ? static_cast<uint8_t>(cooled - cooling) : 0;
            cooled = std::max<uint8_t>(cooled, FireParams::kEmbersFloor);

            const int ambientProbability =
                ClampInclusive<int>(FireParams::kAmbientIgnitionProbabilityBase + mColumnIgnitionBias[col], 0, 255);
            if (NextRandomByte() < ambientProbability)
            {
                int sparkHeat = FireParams::kAmbientIgnitionMin + mColumnSparkHeatBonus[col] +
                                (NextRandomByte() & FireParams::kAmbientIgnitionRangeMask);
                sparkHeat = ClampInclusive<int>(sparkHeat, 0, 255);
                const uint8_t spark = static_cast<uint8_t>(sparkHeat);
                cooled = std::max<uint8_t>(cooled, spark);
            }

            src[idx] = cooled;
        }

        HandleNoteTriggeredFlares(src);

        // Propagate heat upwards by averaging neighbours beneath each cell (integer divide keeps Q0.8 scaling).
        for (int row = 0; row < height - 1; ++row)
        {
            const int belowRow = row + 1;
            const int below2Row = std::min(row + 2, height - 1);
            const float rowPhase = mWindPhase + static_cast<float>(row) * FireParams::kWindRowPhaseStep;
            int globalWind = static_cast<int>(
                std::round(std::sin(static_cast<double>(rowPhase)) * FireParams::kWindGlobalAmplitude));
            globalWind = ClampInclusive<int>(globalWind,
                                             -static_cast<int>(FireParams::kWindMaxOffset),
                                             static_cast<int>(FireParams::kWindMaxOffset));
            for (int col = 0; col < width; ++col)
            {
                const int idx = row * width + col;
                int windOffset = globalWind + mColumnWindBias[col];
                if constexpr (FireParams::kWindNoiseMask != 0)
                {
                    int jitter = NextRandomByte() & FireParams::kWindNoiseMask;
                    if (jitter && (NextRandomByte() & 1))
                    {
                        jitter = -jitter;
                    }
                    windOffset += jitter;
                }
                windOffset = ClampInclusive<int>(windOffset,
                                                 -static_cast<int>(FireParams::kWindMaxOffset),
                                                 static_cast<int>(FireParams::kWindMaxOffset));

                uint32_t sum = SampleHeat(src, belowRow, col + windOffset);
                sum += SampleHeat(src, belowRow, col - 1 + windOffset);
                sum += SampleHeat(src, belowRow, col + 1 + windOffset);
                sum += SampleHeat(src, below2Row, col + windOffset);

                uint32_t averaged = sum >> FireParams::kNeighborAverageShift;
                int32_t val = static_cast<int32_t>(averaged);
                val -= FireParams::kPropagationDecay;
                val -= static_cast<int32_t>(NextRandomByte() & FireParams::kNoiseMask);
                val -= static_cast<int32_t>(mColumnNoiseBias[col]);
                if constexpr (FireParams::kPropagationFlickerMask != 0)
                {
                    val -= static_cast<int32_t>(NextRandomByte() & FireParams::kPropagationFlickerMask);
                }
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
        const int widthDelta = clientRect.width - FireParams::kRenderWidth;
        const int heightDelta = clientRect.height - FireParams::kRenderHeight;
        const int originX = clientRect.x + ((widthDelta > 0) ? (widthDelta / 2) : 0);
        const int originY = clientRect.y + ((heightDelta > 0) ? (heightDelta / 2) : 0);
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

    // Promote recent MIDI note-on events into high-energy flares so musical gestures show up immediately.
    void HandleNoteTriggeredFlares(std::array<uint8_t, FireParams::kPixelCount> &buffer)
    {
        const int currentSerial = gSynthVoiceNoteOnCount; // mMusicalStateTask.mMusicalState.mMidiOut.noteOns;
        if (currentSerial < mLastNoteOnSerial)
        {
            mLastNoteOnSerial = currentSerial;
            return;
        }
        const int delta = currentSerial - mLastNoteOnSerial;
        if (delta <= 0)
        {
            return;
        }

        mLastNoteOnSerial = currentSerial;
        const int midiNote = mMusicalStateTask.mMusicalState.mLastPlayedNote;
        for (int i = 0; i < delta; ++i)
        {
            IgniteNoteFlare(buffer, midiNote);
        }
    }

    void IgniteNoteFlare(std::array<uint8_t, FireParams::kPixelCount> &buffer, int midiNote)
    {
        constexpr int width = FireParams::kRenderWidth;
        constexpr int height = FireParams::kRenderHeight;
        const int bottomRowOffset = (height - 1) * width;

        const int baseColumn = ResolveFlareColumn(midiNote);
        const int centreColumn = ClampInclusive<int>(baseColumn, 0, width - 1);

        for (int offset = -FireParams::kNoteFlareHalfWidth; offset <= FireParams::kNoteFlareHalfWidth; ++offset)
        {
            const int column = centreColumn + offset;
            if (column < 0 || column >= width)
            {
                continue;
            }

            const int distance = std::abs(offset);
            int heat = FireParams::kNoteFlareHeat - distance * FireParams::kNoteFlareHeatFalloff;
            heat += mColumnSparkHeatBonus[column];
            heat = ClampInclusive<int>(heat, 0, 255);
            if (heat <= 0)
            {
                continue;
            }

            const int idxBottom = bottomRowOffset + column;
            buffer[idxBottom] = std::max<uint8_t>(buffer[idxBottom], static_cast<uint8_t>(heat));

            const int idxAbove = idxBottom - width;
            if (idxAbove >= 0)
            {
                int bleedHeat = heat - FireParams::kNoteFlareVerticalFalloff;
                bleedHeat += mColumnSparkHeatBonus[column] / 2;
                bleedHeat = ClampInclusive<int>(bleedHeat, FireParams::kEmbersFloor, 255);
                buffer[idxAbove] = std::max<uint8_t>(buffer[idxAbove], static_cast<uint8_t>(bleedHeat));
            }
        }
    }

    int ResolveFlareColumn(int midiNote)
    {
        constexpr int width = FireParams::kRenderWidth;
        if (midiNote <= 0)
        {
            return NextRandomByte() % width;
        }

        const float columnF = Clamp(RemapToRange(static_cast<float>(midiNote),
                                                 static_cast<float>(FireParams::kNoteColumnMapMin),
                                                 static_cast<float>(FireParams::kNoteColumnMapMax),
                                                 0.0f,
                                                 static_cast<float>(width - 1)),
                                    0.0f,
                                    static_cast<float>(width - 1));
        int approx = static_cast<int>(std::round(columnF));
        approx = ClampInclusive<int>(approx, 0, width - 1);

        int jitter = 0;
        if constexpr (FireParams::kNoteFlareJitterRadius > 0)
        {
            const int span = FireParams::kNoteFlareJitterRadius;
            jitter = static_cast<int>(NextRandomByte() % (span * 2 + 1)) - span;
        }

        const int sway = mColumnWindBias[approx];
        return ClampInclusive<int>(approx + sway + jitter, 0, width - 1);
    }

    void SeedColumnTraits()
    {
        constexpr int width = FireParams::kRenderWidth;
        for (int col = 0; col < width; ++col)
        {
            if constexpr (FireParams::kColumnCoolingVariance > 0)
            {
                const int span = static_cast<int>(FireParams::kColumnCoolingVariance);
                const int draw = static_cast<int>(NextRandomByte()) % (span * 2 + 1);
                mColumnCoolingBias[col] = static_cast<int8_t>(draw - span);
            }
            else
            {
                mColumnCoolingBias[col] = 0;
            }

            if constexpr (FireParams::kColumnIgnitionVariance > 0)
            {
                const int span = static_cast<int>(FireParams::kColumnIgnitionVariance);
                const int draw = static_cast<int>(NextRandomByte()) % (span * 2 + 1);
                mColumnIgnitionBias[col] = static_cast<int8_t>(draw - span);
            }
            else
            {
                mColumnIgnitionBias[col] = 0;
            }

            if constexpr (FireParams::kColumnSparkHeatVariance > 0)
            {
                const int range = static_cast<int>(FireParams::kColumnSparkHeatVariance) + 1;
                mColumnSparkHeatBonus[col] = static_cast<uint8_t>(static_cast<int>(NextRandomByte()) % range);
            }
            else
            {
                mColumnSparkHeatBonus[col] = 0;
            }

            if constexpr (FireParams::kColumnWindVariance > 0)
            {
                const int span = static_cast<int>(FireParams::kColumnWindVariance);
                const int draw = static_cast<int>(NextRandomByte()) % (span * 2 + 1);
                mColumnWindBias[col] = static_cast<int8_t>(draw - span);
            }
            else
            {
                mColumnWindBias[col] = 0;
            }

            if constexpr (FireParams::kColumnNoiseVarianceMask != 0)
            {
                mColumnNoiseBias[col] = static_cast<uint8_t>(NextRandomByte() & FireParams::kColumnNoiseVarianceMask);
            }
            else
            {
                mColumnNoiseBias[col] = 0;
            }
        }
    }

    void UpdateWindPhase()
    {
        mWindPhase += FireParams::kWindPhaseStep;
        while (mWindPhase > kTwoPI_f)
        {
            mWindPhase -= kTwoPI_f;
        }
    }

    static uint8_t SampleHeat(const std::array<uint8_t, FireParams::kPixelCount> &field, int row, int col)
    {
        row = ClampInclusive<int>(row, 0, FireParams::kRenderHeight - 1);
        col = ClampInclusive<int>(col, 0, FireParams::kRenderWidth - 1);
        return field[row * FireParams::kRenderWidth + col];
    }
};

} // namespace clarinoid
