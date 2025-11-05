#pragma once

#include <array>
#include <algorithm>
#include <cmath>

#include "MeshDemoAppBase.hpp"
#include "clarinoid2BigPerfDisplayApp.hpp"

namespace clarinoid
{

template <size_t TSubdivisionDepth>
struct GeodesicSphereDemoParams
{
    // Perspective multiplier applied during projection; larger values render a bigger sphere on screen.
    static constexpr float kProjectionScale = 128.0f;
    // Minimum allowed Z value when projecting (model units) to avoid divide-by-zero.
    static constexpr float kNearPlaneEpsilon = 0.12f;

    // Face shading parameters: ambient brightness offset, diffuse scale, specular lift, and normal epsilon.
    static constexpr float kFaceBaseBrightness = 8.0f;
    static constexpr float kFaceDiffuseScale = 192.0f;
    static constexpr float kFaceSpecularBias = 128.0f;
    static constexpr float kFaceNormalEpsilon = 1e-4f;

    // Number of recursive icosahedron subdivisions (0-2 keeps vertex count within uint8_t indices).
    static constexpr size_t kSubdivisionDepth = TSubdivisionDepth;

    static constexpr size_t kFrequency = size_t{1} << kSubdivisionDepth;
    static constexpr size_t kVertexCount = (10u * kFrequency * kFrequency) + 2u;
    static constexpr size_t kFaceCount = 20u * kFrequency * kFrequency;
    static constexpr size_t kMaxEdgeCount = 30u * kFrequency * kFrequency;

    using ThisT = GeodesicSphereDemoParams<TSubdivisionDepth>;
    using SimulationBase = MeshSimulationBase<ThisT, kVertexCount, kFaceCount>;

    static_assert(
        kVertexCount <= 255u,
        "MeshDemoAppBase::FaceDesc uses uint8_t indices; increase width to uint16_t before using higher subdivision.");
};

namespace detail
{
template <size_t TSubdivisionDepth>
struct GeodesicSphereMeshData
{
    static constexpr size_t kVertexCount = GeodesicSphereDemoParams<TSubdivisionDepth>::kVertexCount;
    static constexpr size_t kFaceCount = GeodesicSphereDemoParams<TSubdivisionDepth>::kFaceCount;
    using Base = MeshSimulationBase<GeodesicSphereDemoParams<TSubdivisionDepth>, kVertexCount, kFaceCount>;
    using FaceDesc = typename Base::FaceDesc;

    std::array<Vec3f, kVertexCount> vertices{};
    std::array<std::array<uint32_t, 3>, kFaceCount> faces{};
    std::array<FaceDesc, kFaceCount> faceDescs{};
};

template <size_t TSubdivisionDepth>
inline GeodesicSphereMeshData<TSubdivisionDepth> GenerateGeodesicSphereMesh()
{
    using TRet = GeodesicSphereMeshData<TSubdivisionDepth>;
    using Params = GeodesicSphereDemoParams<TSubdivisionDepth>;
    TRet mesh{};
    auto &verts = mesh.vertices;
    auto &faces = mesh.faces;

    std::array<std::array<uint32_t, 3>, Params::kFaceCount> workingFaces{};

    size_t vertCount = 0;
    size_t faceCount = 0;

    const float phi = (1.0f + std::sqrt(5.0f)) * 0.5f;
    const float invLen = 1.0f / std::sqrt(1.0f + (phi * phi));

    auto addVertex = [&](float x, float y, float z) {
        CCASSERT(vertCount < Params::kVertexCount);
        Vec3f v{x * invLen, y * invLen, z * invLen};
        verts[vertCount++] = Normalize(v, Params::kFaceNormalEpsilon);
        return static_cast<uint32_t>(vertCount - 1u);
    };

    auto addFace = [&](uint32_t a, uint32_t b, uint32_t c) {
        CCASSERT(faceCount < Params::kFaceCount);
        workingFaces[faceCount++] = {a, b, c};
    };

    const uint32_t v0 = addVertex(-1.0f, phi, 0.0f);
    const uint32_t v1 = addVertex(1.0f, phi, 0.0f);
    const uint32_t v2 = addVertex(-1.0f, -phi, 0.0f);
    const uint32_t v3 = addVertex(1.0f, -phi, 0.0f);
    const uint32_t v4 = addVertex(0.0f, -1.0f, phi);
    const uint32_t v5 = addVertex(0.0f, 1.0f, phi);
    const uint32_t v6 = addVertex(0.0f, -1.0f, -phi);
    const uint32_t v7 = addVertex(0.0f, 1.0f, -phi);
    const uint32_t v8 = addVertex(phi, 0.0f, -1.0f);
    const uint32_t v9 = addVertex(phi, 0.0f, 1.0f);
    const uint32_t v10 = addVertex(-phi, 0.0f, -1.0f);
    const uint32_t v11 = addVertex(-phi, 0.0f, 1.0f);

    addFace(v0, v11, v5);
    addFace(v0, v5, v1);
    addFace(v0, v1, v7);
    addFace(v0, v7, v10);
    addFace(v0, v10, v11);
    addFace(v1, v5, v9);
    addFace(v5, v11, v4);
    addFace(v11, v10, v2);
    addFace(v10, v7, v6);
    addFace(v7, v1, v8);
    addFace(v3, v9, v4);
    addFace(v3, v4, v2);
    addFace(v3, v2, v6);
    addFace(v3, v6, v8);
    addFace(v3, v8, v9);
    addFace(v4, v9, v5);
    addFace(v2, v4, v11);
    addFace(v6, v2, v10);
    addFace(v8, v6, v7);
    addFace(v9, v8, v1);

    struct EdgeCache
    {
        std::array<uint64_t, Params::kMaxEdgeCount> keys{};
        std::array<uint32_t, Params::kMaxEdgeCount> values{};
        size_t count = 0;

        uint32_t FindOrCreate(uint32_t a,
                              uint32_t b,
                              std::array<Vec3f, Params::kVertexCount> &vertsRef,
                              size_t &vertCountRef)
        {
            const uint32_t lo = std::min(a, b);
            const uint32_t hi = std::max(a, b);
            const uint64_t key = (static_cast<uint64_t>(lo) << 32) | hi;
            for (size_t i = 0; i < count; ++i)
            {
                if (keys[i] == key)
                {
                    return values[i];
                }
            }

            CCASSERT(count < Params::kMaxEdgeCount);
            CCASSERT(vertCountRef < Params::kVertexCount);
            Vec3f midpoint = vertsRef[lo] + vertsRef[hi];
            midpoint *= 0.5f;
            midpoint = Normalize(midpoint, Params::kFaceNormalEpsilon);

            const uint32_t index = static_cast<uint32_t>(vertCountRef);
            vertsRef[vertCountRef++] = midpoint;

            keys[count] = key;
            values[count] = index;
            ++count;
            return index;
        }
    };

    std::array<std::array<uint32_t, 3>, Params::kFaceCount> nextFaces{};

    for (size_t iteration = 0; iteration < Params::kSubdivisionDepth; ++iteration)
    {
        EdgeCache cache;
        size_t nextFaceCount = 0;
        for (size_t faceIndex = 0; faceIndex < faceCount; ++faceIndex)
        {
            const auto &tri = workingFaces[faceIndex];
            const uint32_t a = tri[0];
            const uint32_t b = tri[1];
            const uint32_t c = tri[2];

            const uint32_t ab = cache.FindOrCreate(a, b, verts, vertCount);
            const uint32_t bc = cache.FindOrCreate(b, c, verts, vertCount);
            const uint32_t ca = cache.FindOrCreate(c, a, verts, vertCount);

            nextFaces[nextFaceCount++] = {a, ab, ca};
            nextFaces[nextFaceCount++] = {b, bc, ab};
            nextFaces[nextFaceCount++] = {c, ca, bc};
            nextFaces[nextFaceCount++] = {ab, bc, ca};
        }

        workingFaces = nextFaces;
        faceCount = nextFaceCount;
    }

    CCASSERT(vertCount == Params::kVertexCount);
    CCASSERT(faceCount == Params::kFaceCount);

    using FaceDesc = typename TRet::FaceDesc;

    for (size_t i = 0; i < faceCount; ++i)
    {
        faces[i] = workingFaces[i];

        mesh.faceDescs[i] = FaceDesc{static_cast<uint8_t>(faces[i][0]),
                                     static_cast<uint8_t>(faces[i][1]),
                                     static_cast<uint8_t>(faces[i][2]),
                                     static_cast<uint8_t>(faces[i][2])};
    }

    return mesh;
}

// inline const GeodesicSphereMeshData &GetGeodesicSphereMeshData()
// {
//     static const GeodesicSphereMeshData data = GenerateGeodesicSphereMesh();
//     return data;
// }
} // namespace detail

// using GeodesicSphereDemoBase = MeshSimulationBase<GeodesicSphereDemoParams,
//                                                   GeodesicSphereDemoParams::kVertexCount,
//                                                   GeodesicSphereDemoParams::kFaceCount>;

template <size_t TSubdivisionDepth>
struct GeodesicSphereSimulation : GeodesicSphereDemoParams<TSubdivisionDepth>::SimulationBase
{
    using Base = typename GeodesicSphereDemoParams<TSubdivisionDepth>::SimulationBase;
    using FaceDesc = typename Base::FaceDesc;
    using FaceRenderInfo = typename Base::FaceRenderInfo;

    static constexpr size_t kVertexCount = Base::kVertexCount;
    static constexpr size_t kFaceCount = Base::kFaceCount;

    detail::GeodesicSphereMeshData<TSubdivisionDepth> mMeshData;

    GeodesicSphereSimulation(IDisplay &display, MusicalStateTask &musicalStateTask) : Base(display, musicalStateTask)
    {
        mMeshData = detail::GenerateGeodesicSphereMesh<TSubdivisionDepth>();
    }

  protected:
    virtual const std::array<Vec3f, kVertexCount> &GetBaseVertices() const override
    {
        // return detail::GetGeodesicSphereMeshData<TSubdivisionDepth>().vertices;
        return mMeshData.vertices;
    }

    virtual const std::array<FaceDesc, kFaceCount> &GetFaces() const override
    {
        // static const std::array<FaceDesc, kFaceCount> kFaces = []() {
        //     std::array<FaceDesc, kFaceCount> faceBuffer{};
        //     const auto &mesh = detail::GetGeodesicSphereMeshData();
        //     for (size_t i = 0; i < kFaceCount; ++i)
        //     {
        //         const auto &tri = mesh.faces[i];
        //         faceBuffer[i] = FaceDesc{static_cast<uint8_t>(tri[0]),
        //                                  static_cast<uint8_t>(tri[1]),
        //                                  static_cast<uint8_t>(tri[2]),
        //                                  static_cast<uint8_t>(tri[2])};
        //     }
        //     return faceBuffer;
        // }();

        // return kFaces;
        return mMeshData.faceDescs;
    }

    virtual void OnFacePrepared(size_t faceIndex, FaceRenderInfo &info) override
    {
    }
};

} // namespace clarinoid
