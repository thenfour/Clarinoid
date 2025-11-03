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
        static constexpr float kTwoPi = 6.283185307f;
        while (mWindPhase > kTwoPi)
        {
            mWindPhase -= kTwoPi;
        }
    }

    static uint8_t SampleHeat(const std::array<uint8_t, FireParams::kPixelCount> &field, int row, int col)
    {
        row = ClampInclusive<int>(row, 0, FireParams::kRenderHeight - 1);
        col = ClampInclusive<int>(col, 0, FireParams::kRenderWidth - 1);
        return field[row * FireParams::kRenderWidth + col];
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct CubeDemoApp : DisplayApp
{
    MusicalStateTask &mMusicalStateTask;

    // Solid cube with simple painter's algorithm. Rotation integrates angular velocity with light shading per face.
    struct CubeParams
    {
        static constexpr int kViewportWidth = 100; // Pixels allocated for cube rendering (centred within client rect)
        static constexpr int kViewportHeight = 54; // Pixel height of the cube viewport
        static constexpr float kHalfEdge = 0.55f;  // Half edge length of cube in model space units

        static constexpr float kCameraDistance = 3.2f;    // Positive Z offset pushing cube in front of camera
        static constexpr float kProjectionScale = 88.0f;  // Perspective multiplier (larger => bigger cube)
        static constexpr float kNearPlaneEpsilon = 0.15f; // Clamp to avoid blowing up projection when facing camera

        static constexpr float kAmbientAngularVelocityX = 0.00065f; // Passive angular drift radians/frame (pitch)
        static constexpr float kAmbientAngularVelocityY = 0.00045f; // Passive angular drift radians/frame (yaw)
        static constexpr float kAmbientAngularVelocityZ = 0.00030f; // Passive angular drift radians/frame (roll)
        static constexpr float kAngularDamping =
            0.97f; // Exponential decay applied to angular velocity. smaller = more damping. 1.0 = no damping.
        static constexpr float kImpulseMagnitudeMin = 0.2f;   // Note impulse base magnitude (radians/frame)
        static constexpr float kImpulseMagnitudeRange = 0.1f; // Extra random impulse magnitude

        static constexpr float kMaxAngularVelocity = 0.4f; // Clamp per-axis angular velocity for stability

        static constexpr float kFaceBaseBrightness = 48.0f; // Ambient brightness applied to every face (0–255)
        static constexpr float kFaceDiffuseScale = 180.0f;  // Diffuse term applied after dot(light, normal)
        static constexpr float kFaceSpecularBias = 40.0f;   // Additional lift for nearly facing faces
        static constexpr float kFaceNormalEpsilon = 1e-4f;  // Guards degenerate faces when cube edge collapses

        static constexpr float kLightDirX = 0.35f; // Directional light vector (normalised internally)
        static constexpr float kLightDirY = 0.45f;
        static constexpr float kLightDirZ = -0.82f;
    };

    struct FaceDesc
    {
        uint8_t i0;
        uint8_t i1;
        uint8_t i2;
        uint8_t i3;
    };

    struct FaceRenderInfo
    {
        std::array<Vec2f, 4> projected{};
        float depth = 0.0f;
        uint8_t brightness = 0;
    };

    static constexpr size_t kVertexCount = 8;
    static constexpr size_t kFaceCount = 6;

    std::array<Vec3f, kVertexCount> mTransformedVertices{}; // Model -> world (with camera offset)
    std::array<FaceRenderInfo, kFaceCount> mFaceBuffer{};   // Painter ordering buffer each frame

    Vec3f mRotationAngles{};       // Current Euler angles (radians)
    Vec3f mAngularVelocity{};      // Radians per frame around X/Y/Z
    Vec3f mLightDirectionNormal{}; // Cached normalised light direction

    bool mCubeInitialized = false;
    uint32_t mRngState = 0x51F00DF5u; // Independent RNG seed for cube dynamics
    int mLastNoteOnSerial = 0;

    CubeDemoApp(IDisplay &d, MusicalStateTask &musicalStateTask) : DisplayApp(d), mMusicalStateTask(musicalStateTask)
    {
    }

    virtual const char *DisplayAppGetName() override
    {
        return "demo cube";
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
        StepCubeSimulation();
        RenderCubeFrame();
    }

    virtual void RenderFrontPage() override
    {
        mDisplay.setCursor(0, 0);
        mDisplay.println("demo: cube");
        mDisplay.println("press ok");
        mDisplay.println("back to exit");
    }

  private:
    void EnsureCubeInitialized()
    {
        if (mCubeInitialized)
        {
            return;
        }

        const Vec3f rawLight{CubeParams::kLightDirX, CubeParams::kLightDirY, CubeParams::kLightDirZ};
        const float lightLenSq = LengthSq(rawLight);
        if (lightLenSq <= CubeParams::kFaceNormalEpsilon * CubeParams::kFaceNormalEpsilon)
        {
            mLightDirectionNormal = Vec3f{0.0f, 0.0f, 1.0f};
        }
        else
        {
            mLightDirectionNormal = Normalize(rawLight, CubeParams::kFaceNormalEpsilon);
        }
        mRotationAngles = Vec3f{};
        mAngularVelocity = Vec3f{};
        mLastNoteOnSerial = gSynthVoiceNoteOnCount;
        mCubeInitialized = true;
    }

    uint8_t NextRandomByte()
    {
        mRngState = (mRngState * 1664525u) + 1013904223u;
        return static_cast<uint8_t>(mRngState >> 24);
    }

    float NextRandomSignedFloat()
    {
        const float unipolar = static_cast<float>(NextRandomByte()) / 255.0f;
        return (unipolar * 2.0f) - 1.0f;
    }

    void StepCubeSimulation()
    {
        EnsureCubeInitialized();

        HandleNoteImpulses();

        mAngularVelocity.x += CubeParams::kAmbientAngularVelocityX;
        mAngularVelocity.y += CubeParams::kAmbientAngularVelocityY;
        mAngularVelocity.z += CubeParams::kAmbientAngularVelocityZ;

        mAngularVelocity *= CubeParams::kAngularDamping;
        ClampAngularVelocity();

        mRotationAngles += mAngularVelocity;
        WrapAngles();
    }

    void ClampAngularVelocity()
    {
        const float maxMag = CubeParams::kMaxAngularVelocity;
        mAngularVelocity.x = Clamp(mAngularVelocity.x, -maxMag, maxMag);
        mAngularVelocity.y = Clamp(mAngularVelocity.y, -maxMag, maxMag);
        mAngularVelocity.z = Clamp(mAngularVelocity.z, -maxMag, maxMag);
    }

    void WrapAngles()
    {
        mRotationAngles.x = render3d::WrapAngle(mRotationAngles.x);
        mRotationAngles.y = render3d::WrapAngle(mRotationAngles.y);
        mRotationAngles.z = render3d::WrapAngle(mRotationAngles.z);
    }

    void HandleNoteImpulses()
    {
        const int currentSerial = gSynthVoiceNoteOnCount;
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
        for (int i = 0; i < delta; ++i)
        {
            ApplyNoteImpulse();
        }
    }

    void ApplyNoteImpulse()
    {
        Vec3f axis{NextRandomSignedFloat(), NextRandomSignedFloat(), NextRandomSignedFloat()};
        const float lenSq = LengthSq(axis);
        if (lenSq <= CubeParams::kFaceNormalEpsilon * CubeParams::kFaceNormalEpsilon)
        {
            axis = Vec3f{0.0f, 0.0f, 1.0f};
        }
        else
        {
            axis = Normalize(axis, CubeParams::kFaceNormalEpsilon);
        }

        const float impulseMagnitude =
            CubeParams::kImpulseMagnitudeMin +
            (static_cast<float>(NextRandomByte()) / 255.0f) * CubeParams::kImpulseMagnitudeRange;
        mAngularVelocity += axis * impulseMagnitude;
    }

    void RenderCubeFrame()
    {
        const RectI clientRect = mDisplay.GetClientRect();
        const int originX = clientRect.x + (clientRect.width - CubeParams::kViewportWidth) / 2;
        const int originY = clientRect.y + (clientRect.height - CubeParams::kViewportHeight) / 2;
        const RectI renderRect =
            RectI::Construct(originX, originY, CubeParams::kViewportWidth, CubeParams::kViewportHeight);
        mDisplay.SetClipRect(renderRect);

        const float centreX = static_cast<float>(renderRect.x) + (renderRect.width * 0.5f);
        const float centreY = static_cast<float>(renderRect.y) + (renderRect.height * 0.5f);

        static const Vec3f baseVertices[kVertexCount] = {
            Vec3f{-CubeParams::kHalfEdge, -CubeParams::kHalfEdge, -CubeParams::kHalfEdge},
            Vec3f{CubeParams::kHalfEdge, -CubeParams::kHalfEdge, -CubeParams::kHalfEdge},
            Vec3f{CubeParams::kHalfEdge, CubeParams::kHalfEdge, -CubeParams::kHalfEdge},
            Vec3f{-CubeParams::kHalfEdge, CubeParams::kHalfEdge, -CubeParams::kHalfEdge},
            Vec3f{-CubeParams::kHalfEdge, -CubeParams::kHalfEdge, CubeParams::kHalfEdge},
            Vec3f{CubeParams::kHalfEdge, -CubeParams::kHalfEdge, CubeParams::kHalfEdge},
            Vec3f{CubeParams::kHalfEdge, CubeParams::kHalfEdge, CubeParams::kHalfEdge},
            Vec3f{-CubeParams::kHalfEdge, CubeParams::kHalfEdge, CubeParams::kHalfEdge},
        };

        static const FaceDesc faces[kFaceCount] = {
            FaceDesc{0, 3, 2, 1}, // Back (-Z)
            FaceDesc{4, 5, 6, 7}, // Front (+Z)
            FaceDesc{0, 4, 7, 3}, // Left (-X)
            FaceDesc{1, 2, 6, 5}, // Right (+X)
            FaceDesc{3, 7, 6, 2}, // Top (+Y)
            FaceDesc{0, 1, 5, 4}, // Bottom (-Y)
        };

        const float sx = static_cast<float>(std::sin(mRotationAngles.x));
        const float cx = static_cast<float>(std::cos(mRotationAngles.x));
        const float sy = static_cast<float>(std::sin(mRotationAngles.y));
        const float cy = static_cast<float>(std::cos(mRotationAngles.y));
        const float sz = static_cast<float>(std::sin(mRotationAngles.z));
        const float cz = static_cast<float>(std::cos(mRotationAngles.z));

        // Combined rotation matrix R = Rz * Ry * Rx
        const float m00 = cz * cy;
        const float m01 = cz * sy * sx - sz * cx;
        const float m02 = cz * sy * cx + sz * sx;
        const float m10 = sz * cy;
        const float m11 = sz * sy * sx + cz * cx;
        const float m12 = sz * sy * cx - cz * sx;
        const float m20 = -sy;
        const float m21 = cy * sx;
        const float m22 = cy * cx;

        for (size_t i = 0; i < kVertexCount; ++i)
        {
            const Vec3f &v = baseVertices[i];
            Vec3f rotated{
                (m00 * v.x) + (m01 * v.y) + (m02 * v.z),
                (m10 * v.x) + (m11 * v.y) + (m12 * v.z),
                (m20 * v.x) + (m21 * v.y) + (m22 * v.z),
            };

            rotated.z += CubeParams::kCameraDistance;
            rotated.z = std::max(rotated.z, CubeParams::kNearPlaneEpsilon);
            mTransformedVertices[i] = rotated;
        }

        size_t visibleFaceCount = 0;
        for (size_t faceIndex = 0; faceIndex < kFaceCount; ++faceIndex)
        {
            const auto &face = faces[faceIndex];
            const Vec3f &v0 = mTransformedVertices[face.i0];
            const Vec3f &v1 = mTransformedVertices[face.i1];
            const Vec3f &v2 = mTransformedVertices[face.i2];
            const Vec3f &v3 = mTransformedVertices[face.i3];

            const Vec3f edgeA = v1 - v0;
            const Vec3f edgeB = v2 - v0;
            Vec3f normal = Cross(edgeA, edgeB);

            const float normalLenSq = LengthSq(normal);
            if (normalLenSq <= CubeParams::kFaceNormalEpsilon * CubeParams::kFaceNormalEpsilon)
            {
                continue;
            }

            const Vec3f faceCentre = Vec3f{(v0.x + v1.x + v2.x + v3.x) * 0.25f,
                                           (v0.y + v1.y + v2.y + v3.y) * 0.25f,
                                           (v0.z + v1.z + v2.z + v3.z) * 0.25f};

            const Vec3f toCamera{-faceCentre.x, -faceCentre.y, -faceCentre.z};
            const float viewDot = Dot(normal, toCamera);
            if (viewDot <= 0.0f)
            {
                continue; // Back-face cull
            }

            normal = Normalize(normal, CubeParams::kFaceNormalEpsilon);

            const float diffuse = Clamp(Dot(normal, mLightDirectionNormal), 0.0f, 1.0f);
            const float brightnessF = CubeParams::kFaceBaseBrightness + (diffuse * CubeParams::kFaceDiffuseScale) +
                                      (std::pow(diffuse, 4.0f) * CubeParams::kFaceSpecularBias);
            const uint8_t brightness = static_cast<uint8_t>(Clamp(brightnessF, 0.0f, 255.0f));

            FaceRenderInfo &info = mFaceBuffer[visibleFaceCount++];
            info.brightness = brightness;
            info.depth = faceCentre.z;
            info.projected[0] = ProjectVertex(v0, centreX, centreY);
            info.projected[1] = ProjectVertex(v1, centreX, centreY);
            info.projected[2] = ProjectVertex(v2, centreX, centreY);
            info.projected[3] = ProjectVertex(v3, centreX, centreY);
        }

        std::sort(mFaceBuffer.begin(),
                  mFaceBuffer.begin() + visibleFaceCount,
                  [](const FaceRenderInfo &a, const FaceRenderInfo &b) {
                      return a.depth > b.depth; // Painter from far to near
                  });

        for (size_t i = 0; i < visibleFaceCount; ++i)
        {
            const FaceRenderInfo &info = mFaceBuffer[i];
            FillFace(info, renderRect);
        }

        mDisplay.ResetClip();
    }

    Vec2f ProjectVertex(const Vec3f &vertex, float centreX, float centreY) const
    {
        const float invZ = CubeParams::kProjectionScale / vertex.z;
        const float px = centreX + vertex.x * invZ;
        const float py = centreY - vertex.y * invZ;
        return Vec2f{px, py};
    }

    void FillFace(const FaceRenderInfo &info, const RectI &renderRect)
    {
        const auto &p = info.projected;
        auto drawPixel = [this, brightness = info.brightness](int x, int y) {
            mDisplay.SetPixelShaded(PointI::Construct(x, y), brightness);
        };
        render3d::RasterizeTriangle(p[0], p[1], p[2], renderRect, drawPixel, CubeParams::kFaceNormalEpsilon);
        render3d::RasterizeTriangle(p[2], p[3], p[0], renderRect, drawPixel, CubeParams::kFaceNormalEpsilon);
    }
};

} // namespace clarinoid
