#pragma once

#include <array>
#include <cmath>

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
    static constexpr float kProjectionScale = 120.0f;
    // Minimum Z distance during projection to avoid division blow-ups (model units).
    static constexpr float kNearPlaneEpsilon = 0.15f;

    // Face shading parameters: ambient offset, diffuse multiplier, specular boost, and normal epsilon.
    static constexpr float kFaceBaseBrightness = 48.0f;
    static constexpr float kFaceDiffuseScale = 180.0f;
    static constexpr float kFaceSpecularBias = 40.0f;
    static constexpr float kFaceNormalEpsilon = 1e-4f;
};

using CubeDemoBase = MeshSimulationBase<CubeDemoParams, 8, 6>;

struct CubeSimulation : CubeDemoBase
{
    using Base = CubeDemoBase;
    using FaceDesc = typename Base::FaceDesc;

    static constexpr size_t kVertexCount = Base::kVertexCount;
    static constexpr size_t kFaceCount = Base::kFaceCount;

    CubeSimulation(IDisplay &display, MusicalStateTask &musicalStateTask) : Base(display, musicalStateTask)
    {
        GenerateMesh();
    }

  protected:
    array_view<Vec3f, kVertexCount> mVertices;
    array_view<FaceDesc, kFaceCount> mFaceDescs;

    void GenerateMesh()
    {
        // mVertices = gGeomArena.instantiate_array<Vec3f, kVertexCount>();
        // mFaceDescs = gGeomArena.instantiate_array<FaceDesc, kFaceCount>();

        mVertices = gGeomArena.instantiate_array<Vec3f, kVertexCount>(
            Vec3f{-CubeDemoParams::kHalfEdge, -CubeDemoParams::kHalfEdge, -CubeDemoParams::kHalfEdge},
            Vec3f{CubeDemoParams::kHalfEdge, -CubeDemoParams::kHalfEdge, -CubeDemoParams::kHalfEdge},
            Vec3f{CubeDemoParams::kHalfEdge, CubeDemoParams::kHalfEdge, -CubeDemoParams::kHalfEdge},
            Vec3f{-CubeDemoParams::kHalfEdge, CubeDemoParams::kHalfEdge, -CubeDemoParams::kHalfEdge},
            Vec3f{-CubeDemoParams::kHalfEdge, -CubeDemoParams::kHalfEdge, CubeDemoParams::kHalfEdge},
            Vec3f{CubeDemoParams::kHalfEdge, -CubeDemoParams::kHalfEdge, CubeDemoParams::kHalfEdge},
            Vec3f{CubeDemoParams::kHalfEdge, CubeDemoParams::kHalfEdge, CubeDemoParams::kHalfEdge},
            Vec3f{-CubeDemoParams::kHalfEdge, CubeDemoParams::kHalfEdge, CubeDemoParams::kHalfEdge});

        mFaceDescs = gGeomArena.instantiate_array<FaceDesc, kFaceCount>(FaceDesc{0, 3, 2, 1},
                                                                        FaceDesc{4, 5, 6, 7},
                                                                        FaceDesc{0, 4, 7, 3},
                                                                        FaceDesc{1, 2, 6, 5},
                                                                        FaceDesc{3, 7, 6, 2},
                                                                        FaceDesc{0, 1, 5, 4});
    }

    virtual const array_view<Vec3f, kVertexCount> &GetBaseVertices() const override
    {
        return mVertices;
    }

    virtual const array_view<FaceDesc, kFaceCount> &GetFaces() const override
    {
        return mFaceDescs;
    }
};

struct TetrahedronDemoParams
{
    static constexpr float kRadius = 0.8f;
    static constexpr float kProjectionScale = 135.0f;
    static constexpr float kNearPlaneEpsilon = 0.12f;
    static constexpr float kFaceBaseBrightness = 56.0f;
    static constexpr float kFaceDiffuseScale = 200.0f;
    static constexpr float kFaceSpecularBias = 48.0f;
    static constexpr float kFaceNormalEpsilon = 1e-4f;
};

using TetrahedronDemoBase = MeshSimulationBase<TetrahedronDemoParams, 4, 4>;

struct TetrahedronSimulation : TetrahedronDemoBase
{
    using Base = TetrahedronDemoBase;
    using FaceDesc = typename Base::FaceDesc;
    static constexpr size_t kVertexCount = Base::kVertexCount;
    static constexpr size_t kFaceCount = Base::kFaceCount;

    array_view<Vec3f, kVertexCount> mVertices;
    array_view<FaceDesc, kFaceCount> mFaces;

    TetrahedronSimulation(IDisplay &display, MusicalStateTask &musicalStateTask) : Base(display, musicalStateTask)
    {
        GenerateMesh();
    }

  protected:
    void GenerateMesh()
    {
        mVertices = gGeomArena.instantiate_array<Vec3f, kVertexCount>();
        mFaces = gGeomArena.instantiate_array<FaceDesc, kFaceCount>();

        const float scale = TetrahedronDemoParams::kRadius / std::sqrt(3.0f);
        const Vec3f baseVerts[4] = {
            Vec3f{1.0f, 1.0f, 1.0f},
            Vec3f{-1.0f, -1.0f, 1.0f},
            Vec3f{-1.0f, 1.0f, -1.0f},
            Vec3f{1.0f, -1.0f, -1.0f},
        };

        for (size_t i = 0; i < kVertexCount; ++i)
        {
            mVertices[i] = baseVerts[i] * scale;
        }

        mFaces[0] = FaceDesc{0, 1, 2, 2};
        mFaces[1] = FaceDesc{0, 3, 1, 1};
        mFaces[2] = FaceDesc{0, 2, 3, 3};
        mFaces[3] = FaceDesc{1, 3, 2, 2};
    }

    virtual const array_view<Vec3f, kVertexCount> &GetBaseVertices() const override
    {
        return mVertices;
    }

    virtual const array_view<FaceDesc, kFaceCount> &GetFaces() const override
    {
        return mFaces;
    }
};

struct OctahedronDemoParams
{
    static constexpr float kRadius = 0.75f;
    static constexpr float kProjectionScale = 132.0f;
    static constexpr float kNearPlaneEpsilon = 0.12f;
    static constexpr float kFaceBaseBrightness = 52.0f;
    static constexpr float kFaceDiffuseScale = 208.0f;
    static constexpr float kFaceSpecularBias = 44.0f;
    static constexpr float kFaceNormalEpsilon = 1e-4f;
};

using OctahedronDemoBase = MeshSimulationBase<OctahedronDemoParams, 6, 8>;

struct OctahedronSimulation : OctahedronDemoBase
{
    using Base = OctahedronDemoBase;
    using FaceDesc = typename Base::FaceDesc;
    static constexpr size_t kVertexCount = Base::kVertexCount;
    static constexpr size_t kFaceCount = Base::kFaceCount;

    array_view<Vec3f, kVertexCount> mVertices;
    array_view<FaceDesc, kFaceCount> mFaces;

    OctahedronSimulation(IDisplay &display, MusicalStateTask &musicalStateTask) : Base(display, musicalStateTask)
    {
        GenerateMesh();
    }

  protected:
    void GenerateMesh()
    {
        mVertices = gGeomArena.instantiate_array<Vec3f, kVertexCount>();
        mFaces = gGeomArena.instantiate_array<FaceDesc, kFaceCount>();

        const float r = OctahedronDemoParams::kRadius;
        mVertices[0] = Vec3f{0.0f, 0.0f, r};
        mVertices[1] = Vec3f{r, 0.0f, 0.0f};
        mVertices[2] = Vec3f{0.0f, r, 0.0f};
        mVertices[3] = Vec3f{-r, 0.0f, 0.0f};
        mVertices[4] = Vec3f{0.0f, -r, 0.0f};
        mVertices[5] = Vec3f{0.0f, 0.0f, -r};

        mFaces[0] = FaceDesc{0, 1, 2, 2};
        mFaces[1] = FaceDesc{0, 2, 3, 3};
        mFaces[2] = FaceDesc{0, 3, 4, 4};
        mFaces[3] = FaceDesc{0, 4, 1, 1};
        mFaces[4] = FaceDesc{5, 2, 1, 1};
        mFaces[5] = FaceDesc{5, 3, 2, 2};
        mFaces[6] = FaceDesc{5, 4, 3, 3};
        mFaces[7] = FaceDesc{5, 1, 4, 4};
    }

    virtual const array_view<Vec3f, kVertexCount> &GetBaseVertices() const override
    {
        return mVertices;
    }

    virtual const array_view<FaceDesc, kFaceCount> &GetFaces() const override
    {
        return mFaces;
    }
};

struct DodecahedronDemoParams
{
    static constexpr float kRadius = 0.9f;
    static constexpr float kProjectionScale = 120.0f;
    static constexpr float kNearPlaneEpsilon = 0.12f;
    static constexpr float kFaceBaseBrightness = 48.0f;
    static constexpr float kFaceDiffuseScale = 190.0f;
    static constexpr float kFaceSpecularBias = 50.0f;
    static constexpr float kFaceNormalEpsilon = 1e-4f;
};

using DodecahedronDemoBase = MeshSimulationBase<DodecahedronDemoParams, 20, 36>;

struct DodecahedronSimulation : DodecahedronDemoBase
{
    using Base = DodecahedronDemoBase;
    using FaceDesc = typename Base::FaceDesc;
    static constexpr size_t kVertexCount = Base::kVertexCount;
    static constexpr size_t kFaceCount = Base::kFaceCount;

    array_view<Vec3f, kVertexCount> mVertices;
    array_view<FaceDesc, kFaceCount> mFaces;

    DodecahedronSimulation(IDisplay &display, MusicalStateTask &musicalStateTask) : Base(display, musicalStateTask)
    {
        GenerateMesh();
    }

  protected:
    void GenerateMesh()
    {
        mVertices = gGeomArena.instantiate_array<Vec3f, kVertexCount>();
        mFaces = gGeomArena.instantiate_array<FaceDesc, kFaceCount>();

        const float phi = (1.0f + std::sqrt(5.0f)) * 0.5f;
        const float invPhi = 1.0f / phi;

        const Vec3f baseVerts[kVertexCount] = {
            Vec3f{-1.0f, -1.0f, -1.0f}, Vec3f{-1.0f, -1.0f, 1.0f}, Vec3f{-1.0f, 1.0f, -1.0f}, Vec3f{-1.0f, 1.0f, 1.0f},
            Vec3f{1.0f, -1.0f, -1.0f},  Vec3f{1.0f, -1.0f, 1.0f},  Vec3f{1.0f, 1.0f, -1.0f},  Vec3f{1.0f, 1.0f, 1.0f},
            Vec3f{0.0f, -invPhi, -phi}, Vec3f{0.0f, -invPhi, phi}, Vec3f{0.0f, invPhi, -phi}, Vec3f{0.0f, invPhi, phi},
            Vec3f{-invPhi, -phi, 0.0f}, Vec3f{-invPhi, phi, 0.0f}, Vec3f{invPhi, -phi, 0.0f}, Vec3f{invPhi, phi, 0.0f},
            Vec3f{-phi, 0.0f, -invPhi}, Vec3f{phi, 0.0f, -invPhi}, Vec3f{-phi, 0.0f, invPhi}, Vec3f{phi, 0.0f, invPhi},
        };

        for (size_t i = 0; i < kVertexCount; ++i)
        {
            Vec3f v = Normalize(baseVerts[i], DodecahedronDemoParams::kFaceNormalEpsilon);
            mVertices[i] = v * DodecahedronDemoParams::kRadius;
        }

        static constexpr uint8_t faces[12][5] = {
            {0, 16, 2, 10, 8},
            {0, 8, 4, 14, 12},
            {0, 12, 1, 9, 16},
            {1, 13, 5, 15, 9},
            {1, 12, 14, 6, 13},
            {2, 16, 9, 15, 11},
            {2, 11, 3, 17, 10},
            {3, 11, 15, 5, 7},
            {3, 7, 19, 18, 17},
            {4, 8, 10, 17, 18},
            {4, 18, 19, 6, 14},
            {5, 13, 6, 19, 7},
        };

        size_t faceIndex = 0;
        for (const auto &face : faces)
        {
            mFaces[faceIndex++] = FaceDesc{face[0], face[1], face[2], face[2]};
            mFaces[faceIndex++] = FaceDesc{face[0], face[2], face[3], face[3]};
            mFaces[faceIndex++] = FaceDesc{face[0], face[3], face[4], face[4]};
        }

        // Ensure correct outward winding...
        // the geom still renders messed up though.
        for (size_t i = 0; i < kFaceCount; ++i)
        {
            auto tri = mFaces[i];
            const Vec3f &a = mVertices[tri.i0], &b = mVertices[tri.i1], &c = mVertices[tri.i2];
            Vec3f n = Normalize(Cross(b - a, c - a));
            Vec3f centroid = (a + b + c) * (1.0f / 3.0f);
            if (Dot(n, centroid) < 0.0f)
            {
                std::swap(mFaces[i].i1, mFaces[i].i2); // flip winding outward
                mFaces[i].i3 = mFaces[i].i2;
            }
        }
    }

    virtual const array_view<Vec3f, kVertexCount> &GetBaseVertices() const override
    {
        return mVertices;
    }

    virtual const array_view<FaceDesc, kFaceCount> &GetFaces() const override
    {
        return mFaces;
    }
};

struct TessellatedCubeDemoParams
{
    static constexpr float kHalfEdge = 0.55f;
    static constexpr float kCentreOffset = 0.18f;
    static constexpr float kProjectionScale = 120.0f;
    static constexpr float kNearPlaneEpsilon = 0.12f;
    static constexpr float kFaceBaseBrightness = 48.0f;
    static constexpr float kFaceDiffuseScale = 185.0f;
    static constexpr float kFaceSpecularBias = 42.0f;
    static constexpr float kFaceNormalEpsilon = 1e-4f;
};

using TessellatedCubeDemoBase = MeshSimulationBase<TessellatedCubeDemoParams, 14, 24>;

struct TessellatedCubeSimulation : TessellatedCubeDemoBase
{
    using Base = TessellatedCubeDemoBase;
    using FaceDesc = typename Base::FaceDesc;
    static constexpr size_t kVertexCount = Base::kVertexCount;
    static constexpr size_t kFaceCount = Base::kFaceCount;

    array_view<Vec3f, kVertexCount> mVertices;
    array_view<FaceDesc, kFaceCount> mFaces;

    TessellatedCubeSimulation(IDisplay &display, MusicalStateTask &musicalStateTask) : Base(display, musicalStateTask)
    {
        GenerateMesh();
    }

  protected:
    void GenerateMesh()
    {
        mVertices = gGeomArena.instantiate_array<Vec3f, kVertexCount>();
        mFaces = gGeomArena.instantiate_array<FaceDesc, kFaceCount>();

        const float h = TessellatedCubeDemoParams::kHalfEdge;
        mVertices[0] = Vec3f{-h, -h, -h};
        mVertices[1] = Vec3f{h, -h, -h};
        mVertices[2] = Vec3f{h, h, -h};
        mVertices[3] = Vec3f{-h, h, -h};
        mVertices[4] = Vec3f{-h, -h, h};
        mVertices[5] = Vec3f{h, -h, h};
        mVertices[6] = Vec3f{h, h, h};
        mVertices[7] = Vec3f{-h, h, h};

        const float offset = TessellatedCubeDemoParams::kCentreOffset;
        mVertices[8] = Vec3f{0.0f, 0.0f, -(h + offset)};  // -Z face
        mVertices[9] = Vec3f{0.0f, 0.0f, h + offset};     // +Z face
        mVertices[10] = Vec3f{-(h + offset), 0.0f, 0.0f}; // -X face
        mVertices[11] = Vec3f{h + offset, 0.0f, 0.0f};    // +X face
        mVertices[12] = Vec3f{0.0f, h + offset, 0.0f};    // +Y face
        mVertices[13] = Vec3f{0.0f, -(h + offset), 0.0f}; // -Y face

        static constexpr uint8_t faceCorners[6][4] = {
            {0, 3, 2, 1}, // -Z
            {4, 5, 6, 7}, // +Z
            {0, 4, 7, 3}, // -X
            {1, 2, 6, 5}, // +X
            {3, 7, 6, 2}, // +Y
            {0, 1, 5, 4}, // -Y
        };

        static constexpr uint8_t faceCenters[6] = {8, 9, 10, 11, 12, 13};

        size_t faceIndex = 0;
        for (size_t i = 0; i < 6; ++i)
        {
            const uint8_t centre = faceCenters[i];
            for (size_t corner = 0; corner < 4; ++corner)
            {
                const uint8_t a = faceCorners[i][corner];
                const uint8_t b = faceCorners[i][(corner + 1) % 4];
                mFaces[faceIndex++] = FaceDesc{a, b, centre, centre};
            }
        }
    }

    virtual const array_view<Vec3f, kVertexCount> &GetBaseVertices() const override
    {
        return mVertices;
    }

    virtual const array_view<FaceDesc, kFaceCount> &GetFaces() const override
    {
        return mFaces;
    }
};

} // namespace clarinoid
