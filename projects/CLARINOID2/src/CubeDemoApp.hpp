#pragma once

#include <array>

#include "MeshDemoAppBase.hpp"
#include <clarinoid/menu/MenuAppBase.hpp>

namespace clarinoid
{

struct MusicalStateTask;

struct CubeDemoParams
{
    // Width of the cube viewport in pixels (centered within client rect).
    static constexpr int kViewportWidth = 100;
    // Height of the cube viewport in pixels (centered within client rect).
    static constexpr int kViewportHeight = 54;
    // Half of the cube edge length in model units; full edge = 2 * kHalfEdge.
    static constexpr float kHalfEdge = 0.55f;

    // Positive Z offset pushing the cube away from the camera (model units).
    static constexpr float kCameraDistance = 3.2f;
    // Perspective multiplier applied during projection; larger renders a bigger cube on screen.
    static constexpr float kProjectionScale = 88.0f;
    // Minimum Z distance during projection to avoid division blow-ups (model units).
    static constexpr float kNearPlaneEpsilon = 0.15f;

    // Passive angular velocity around X/Y/Z (radians per frame).
    static constexpr float kAmbientAngularVelocityX = 0.00065f;
    static constexpr float kAmbientAngularVelocityY = 0.00045f;
    static constexpr float kAmbientAngularVelocityZ = 0.00030f;
    // Exponential decay applied to angular velocity each frame (unitless 0-1).
    static constexpr float kAngularDamping = 0.97f;
    // Minimum and randomised range of note-driven angular impulses (radians per frame).
    static constexpr float kImpulseMagnitudeMin = 0.2f;
    static constexpr float kImpulseMagnitudeRange = 0.1f;
    // Clamp for maximum angular speed per axis (radians per frame).
    static constexpr float kMaxAngularVelocity = 0.4f;

    // Face shading parameters: ambient offset, diffuse multiplier, specular boost, and normal epsilon.
    static constexpr float kFaceBaseBrightness = 48.0f;
    static constexpr float kFaceDiffuseScale = 180.0f;
    static constexpr float kFaceSpecularBias = 40.0f;
    static constexpr float kFaceNormalEpsilon = 1e-4f;

    // Directional light vector components (unitless, normalised internally).
    static constexpr float kLightDirX = 0.35f;
    static constexpr float kLightDirY = 0.45f;
    static constexpr float kLightDirZ = -0.82f;
};

using CubeDemoBase = MeshDemoAppBase<CubeDemoParams, 8, 6>;

struct CubeDemoApp : CubeDemoBase
{
    using Base = CubeDemoBase;
    using FaceDesc = typename Base::FaceDesc;

    static constexpr size_t kVertexCount = Base::kVertexCount;
    static constexpr size_t kFaceCount = Base::kFaceCount;

    CubeDemoApp(IDisplay &display, MusicalStateTask &musicalStateTask) : Base(display, musicalStateTask, 0x51F00DF5u)
    {
    }

    virtual const char *DisplayAppGetName() override
    {
        return "demo cube";
    }

    // virtual void RenderFrontPage() override
    // {
    //     this->mDisplay.setCursor(0, 0);
    //     this->mDisplay.println("demo: cube");
    //     this->mDisplay.println("press ok");
    //     this->mDisplay.println("back to exit");
    // }

  protected:
    virtual const std::array<Vec3f, kVertexCount> &GetBaseVertices() const override
    {
        static const std::array<Vec3f, kVertexCount> kBaseVertices = {
            Vec3f{-CubeDemoParams::kHalfEdge, -CubeDemoParams::kHalfEdge, -CubeDemoParams::kHalfEdge},
            Vec3f{CubeDemoParams::kHalfEdge, -CubeDemoParams::kHalfEdge, -CubeDemoParams::kHalfEdge},
            Vec3f{CubeDemoParams::kHalfEdge, CubeDemoParams::kHalfEdge, -CubeDemoParams::kHalfEdge},
            Vec3f{-CubeDemoParams::kHalfEdge, CubeDemoParams::kHalfEdge, -CubeDemoParams::kHalfEdge},
            Vec3f{-CubeDemoParams::kHalfEdge, -CubeDemoParams::kHalfEdge, CubeDemoParams::kHalfEdge},
            Vec3f{CubeDemoParams::kHalfEdge, -CubeDemoParams::kHalfEdge, CubeDemoParams::kHalfEdge},
            Vec3f{CubeDemoParams::kHalfEdge, CubeDemoParams::kHalfEdge, CubeDemoParams::kHalfEdge},
            Vec3f{-CubeDemoParams::kHalfEdge, CubeDemoParams::kHalfEdge, CubeDemoParams::kHalfEdge},
        };
        return kBaseVertices;
    }

    virtual const std::array<FaceDesc, kFaceCount> &GetFaces() const override
    {
        static const std::array<FaceDesc, kFaceCount> kFaces = {
            FaceDesc{0, 3, 2, 1},
            FaceDesc{4, 5, 6, 7},
            FaceDesc{0, 4, 7, 3},
            FaceDesc{1, 2, 6, 5},
            FaceDesc{3, 7, 6, 2},
            FaceDesc{0, 1, 5, 4},
        };
        return kFaces;
    }
};

} // namespace clarinoid
