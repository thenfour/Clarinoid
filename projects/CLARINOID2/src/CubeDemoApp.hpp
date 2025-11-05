#pragma once

#include <array>

#include "MeshDemoAppBase.hpp"
#include <clarinoid/menu/MenuAppBase.hpp>

namespace clarinoid
{

struct MusicalStateTask;

struct CubeDemoParams
{
    // Half of the cube edge length in model units; full edge = 2 * kHalfEdge.
    static constexpr float kHalfEdge = 0.55f;

    // Perspective multiplier applied during projection; larger renders a bigger cube on screen.
    static constexpr float kProjectionScale = 88.0f;
    // Minimum Z distance during projection to avoid division blow-ups (model units).
    static constexpr float kNearPlaneEpsilon = 0.15f;

    // Face shading parameters: ambient offset, diffuse multiplier, specular boost, and normal epsilon.
    static constexpr float kFaceBaseBrightness = 48.0f;
    static constexpr float kFaceDiffuseScale = 180.0f;
    static constexpr float kFaceSpecularBias = 40.0f;
    static constexpr float kFaceNormalEpsilon = 1e-4f;
};

using CubeDemoBase = MeshSimulationBase<CubeDemoParams, 8, 6>;

struct CubeDemoApp : CubeDemoBase
{
    using Base = CubeDemoBase;
    using FaceDesc = typename Base::FaceDesc;

    static constexpr size_t kVertexCount = Base::kVertexCount;
    static constexpr size_t kFaceCount = Base::kFaceCount;

    CubeDemoApp(IDisplay &display, MusicalStateTask &musicalStateTask) : Base(display, musicalStateTask)
    {
    }

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
