#pragma once

#include <array>
#include <cmath>

#include "MeshDemoAppBase.hpp"
#include "clarinoid2MusicalStateTask.hpp"

namespace clarinoid
{

struct TorusDemoParams
{
    // Width of the torus viewport in pixels (centered on the display).
    static constexpr int kViewportWidth = 100;
    // Height of the torus viewport in pixels (centered on the display).
    static constexpr int kViewportHeight = 54;
    // Positive Z offset keeping the torus in front of the camera (model units).
    static constexpr float kCameraDistance = 5.f;
    // Perspective multiplier applied during projection; larger values render a bigger torus on screen.
    static constexpr float kProjectionScale = 112.0f;
    // Minimum allowed Z value when projecting (model units) to avoid divide-by-zero.
    static constexpr float kNearPlaneEpsilon = 0.12f;

    // Passive angular velocity about X/Y/Z (radians per frame) for idle motion.
    static constexpr float kAmbientAngularVelocityX = 0.00042f;
    static constexpr float kAmbientAngularVelocityY = 0.00078f;
    static constexpr float kAmbientAngularVelocityZ = 0.00025f;
    // Exponential decay applied to angular velocity each frame (unitless 0-1).
    static constexpr float kAngularDamping = 0.965f;
    // Base and randomised portion of note-triggered impulses (radians per frame).
    static constexpr float kImpulseMagnitudeMin = 0.13f;
    static constexpr float kImpulseMagnitudeRange = 0.05f;
    // Maximum angular velocity per axis (radians per frame).
    static constexpr float kMaxAngularVelocity = 0.42f;

    // Face shading parameters: ambient brightness offset, diffuse scale, specular lift, and normal epsilon.
    static constexpr float kFaceBaseBrightness = 40.0f;
    static constexpr float kFaceDiffuseScale = 190.0f;
    static constexpr float kFaceSpecularBias = 24.0f;
    static constexpr float kFaceNormalEpsilon = 1e-4f;

    // Directional light vector components (unitless, normalised internally).
    static constexpr float kLightDirX = -0.25f;
    static constexpr float kLightDirY = 0.62f;
    static constexpr float kLightDirZ = -0.75f;

    // Torus tessellation: number of segments around major (ring) and minor (tube) axes.
    static constexpr size_t kMajorSegments = 13;
    static constexpr size_t kMinorSegments = 9;
    // Torus radii in model units: distance from center to tube center, and tube radius.
    static constexpr float kMajorRadius = 1.15f;
    static constexpr float kMinorRadius = 0.4f;
};

static constexpr size_t kTorusVertexCount = TorusDemoParams::kMajorSegments * TorusDemoParams::kMinorSegments;
static constexpr size_t kTorusFaceCount = TorusDemoParams::kMajorSegments * TorusDemoParams::kMinorSegments;

using TorusDemoBase = MeshDemoAppBase<TorusDemoParams, kTorusVertexCount, kTorusFaceCount>;

struct TorusDemoApp : TorusDemoBase
{
    using Base = TorusDemoBase;
    using FaceDesc = typename Base::FaceDesc;
    using FaceRenderInfo = typename Base::FaceRenderInfo;

    static constexpr size_t kVertexCount = Base::kVertexCount;
    static constexpr size_t kFaceCount = Base::kFaceCount;

    TorusDemoApp(IDisplay &display, MusicalStateTask &musicalStateTask) : Base(display, musicalStateTask, 0x8BADC0DEu)
    {
    }

    virtual const char *DisplayAppGetName() override
    {
        return "demo torus";
    }

    virtual void RenderFrontPage() override
    {
        this->mDisplay.setCursor(0, 0);
        this->mDisplay.println("demo: torus");
        this->mDisplay.println("press ok");
        this->mDisplay.println("back to exit");
    }

  protected:
    virtual const std::array<Vec3f, kVertexCount> &GetBaseVertices() const override
    {
        static const std::array<Vec3f, kVertexCount> kVertices = []() {
            std::array<Vec3f, kVertexCount> verts{};
            size_t idx = 0;
            constexpr float kTwoPi = 6.28318530717958647692f;
            for (size_t major = 0; major < TorusDemoParams::kMajorSegments; ++major)
            {
                const float majorTheta =
                    kTwoPi * static_cast<float>(major) / static_cast<float>(TorusDemoParams::kMajorSegments);
                const float cosMajor = std::cos(majorTheta);
                const float sinMajor = std::sin(majorTheta);
                for (size_t minor = 0; minor < TorusDemoParams::kMinorSegments; ++minor)
                {
                    const float minorTheta =
                        kTwoPi * static_cast<float>(minor) / static_cast<float>(TorusDemoParams::kMinorSegments);
                    const float cosMinor = std::cos(minorTheta);
                    const float sinMinor = std::sin(minorTheta);
                    const float radial = TorusDemoParams::kMajorRadius + (TorusDemoParams::kMinorRadius * cosMinor);

                    verts[idx++] = Vec3f{
                        radial * cosMajor,
                        radial * sinMajor,
                        TorusDemoParams::kMinorRadius * sinMinor,
                    };
                }
            }
            return verts;
        }();

        return kVertices;
    }

    virtual const std::array<FaceDesc, kFaceCount> &GetFaces() const override
    {
        static const std::array<FaceDesc, kFaceCount> kFaces = []() {
            std::array<FaceDesc, kFaceCount> faces{};
            size_t idx = 0;
            for (size_t major = 0; major < TorusDemoParams::kMajorSegments; ++major)
            {
                const size_t nextMajor = (major + 1) % TorusDemoParams::kMajorSegments;
                for (size_t minor = 0; minor < TorusDemoParams::kMinorSegments; ++minor)
                {
                    const size_t nextMinor = (minor + 1) % TorusDemoParams::kMinorSegments;
                    const size_t i00 = (major * TorusDemoParams::kMinorSegments) + minor;
                    const size_t i01 = (major * TorusDemoParams::kMinorSegments) + nextMinor;
                    const size_t i11 = (nextMajor * TorusDemoParams::kMinorSegments) + nextMinor;
                    const size_t i10 = (nextMajor * TorusDemoParams::kMinorSegments) + minor;

                    faces[idx++] = FaceDesc{
                        static_cast<uint8_t>(i00),
                        static_cast<uint8_t>(i01),
                        static_cast<uint8_t>(i11),
                        static_cast<uint8_t>(i10),
                    };
                }
            }
            return faces;
        }();

        return kFaces;
    }

    virtual void OnFacePrepared(size_t faceIndex, FaceRenderInfo &info) override
    {
        constexpr float kTwoPi = 6.28318530717958647692f;
        const float breath = mMusicalStateTask.mMusicalState.mCurrentBreath01.GetValue();
        const float ringPhase = kTwoPi * static_cast<float>(faceIndex % TorusDemoParams::kMinorSegments) /
                                static_cast<float>(TorusDemoParams::kMinorSegments);
        const float accent = 1.0f + (0.35f * breath * std::sin(ringPhase));
        const float adjusted = Clamp(static_cast<float>(info.brightness) * accent, 0.0f, 255.0f);
        info.brightness = static_cast<uint8_t>(adjusted);
    }
};

} // namespace clarinoid
