#pragma once

#include <array>
#include <cmath>

#include "MeshDemoAppBase.hpp"
#include "clarinoid2MusicalStateTask.hpp"

namespace clarinoid
{

template <size_t TMajorSegments, size_t TMinorSegments>
struct TorusDemoParams
{
    // Perspective multiplier applied during projection; larger values render a bigger torus on screen.
    static constexpr float kProjectionScale = 90.0f;
    // Minimum allowed Z value when projecting (model units) to avoid divide-by-zero.
    static constexpr float kNearPlaneEpsilon = 0.12f;

    // Face shading parameters: ambient brightness offset, diffuse scale, specular lift, and normal epsilon.
    static constexpr float kFaceBaseBrightness = 40.0f;
    static constexpr float kFaceDiffuseScale = 190.0f;
    static constexpr float kFaceSpecularBias = 24.0f;
    static constexpr float kFaceNormalEpsilon = 1e-4f;

    // Torus tessellation: number of segments around major (ring) and minor (tube) axes.
    static constexpr size_t kMajorSegments = TMajorSegments; // 13;
    static constexpr size_t kMinorSegments = TMinorSegments; // 9;
    // Torus radii in model units: distance from center to tube center, and tube radius.
    static constexpr float kMajorRadius = 1.f;
    static constexpr float kMinorRadius = 0.5f;
    static constexpr size_t kTorusVertexCount = TorusDemoParams::kMajorSegments * TorusDemoParams::kMinorSegments;
    static constexpr size_t kTorusFaceCount = TorusDemoParams::kMajorSegments * TorusDemoParams::kMinorSegments;

    using ThisT = TorusDemoParams<TMajorSegments, TMinorSegments>;
    using SimulationBase = MeshSimulationBase<ThisT, kTorusVertexCount, kTorusFaceCount>;
};

// using TorusDemoBase = MeshSimulationBase<TorusDemoParams, kTorusVertexCount, kTorusFaceCount>;

template <size_t TMajorSegments, size_t TMinorSegments>
struct TorusSimulation : TorusDemoParams<TMajorSegments, TMinorSegments>::SimulationBase
{
    using Params = TorusDemoParams<TMajorSegments, TMinorSegments>;
    using Base = typename Params::SimulationBase;
    using FaceDesc = typename Base::FaceDesc;
    using FaceRenderInfo = typename Base::FaceRenderInfo;

    static constexpr size_t kVertexCount = Base::kVertexCount;
    static constexpr size_t kFaceCount = Base::kFaceCount;

    TorusSimulation(IDisplay &display, MusicalStateTask &musicalStateTask) : Base(display, musicalStateTask)
    {
        GenerateMesh();
    }

  protected:
    array_view<Vec3f, kVertexCount> mVertices;
    array_view<FaceDesc, kFaceCount> mFaces;

    void GenerateMesh()
    {
        mVertices = gGeomArena.instantiate_array<Vec3f, kVertexCount>();
        mFaces = gGeomArena.instantiate_array<FaceDesc, kFaceCount>();

        // Generate vertices
        for (size_t major = 0; major < Params::kMajorSegments; ++major)
        {
            const float majorTheta = kTwoPI_f * static_cast<float>(major) / static_cast<float>(Params::kMajorSegments);
            const float cosMajor = fast::cos(majorTheta);
            const float sinMajor = fast::sin(majorTheta);
            for (size_t minor = 0; minor < Params::kMinorSegments; ++minor)
            {
                const float minorTheta =
                    kTwoPI_f * static_cast<float>(minor) / static_cast<float>(Params::kMinorSegments);
                const float cosMinor = fast::cos(minorTheta);
                const float sinMinor = fast::sin(minorTheta);
                const float radial = Params::kMajorRadius + (Params::kMinorRadius * cosMinor);

                mVertices[(major * Params::kMinorSegments) + minor] = Vec3f{
                    radial * cosMajor,
                    radial * sinMajor,
                    Params::kMinorRadius * sinMinor,
                };
            }
        }

        // Generate faces
        for (size_t major = 0; major < Params::kMajorSegments; ++major)
        {
            const size_t nextMajor = (major + 1) % Params::kMajorSegments;
            for (size_t minor = 0; minor < Params::kMinorSegments; ++minor)
            {
                const size_t nextMinor = (minor + 1) % Params::kMinorSegments;
                const size_t i00 = (major * Params::kMinorSegments) + minor;
                const size_t i01 = (major * Params::kMinorSegments) + nextMinor;
                const size_t i11 = (nextMajor * Params::kMinorSegments) + nextMinor;
                const size_t i10 = (nextMajor * Params::kMinorSegments) + minor;

                mFaces[(major * Params::kMinorSegments) + minor] = FaceDesc{
                    static_cast<uint8_t>(i00),
                    static_cast<uint8_t>(i01),
                    static_cast<uint8_t>(i11),
                    static_cast<uint8_t>(i10),
                };
            }
        }
    }

    virtual const array_view<Vec3f, kVertexCount> &GetBaseVertices() const override
    {
        // static const std::array<Vec3f, kVertexCount> kVertices = []() {
        //     std::array<Vec3f, kVertexCount> verts{};
        //     size_t idx = 0;
        //     for (size_t major = 0; major < TorusDemoParams::kMajorSegments; ++major)
        //     {
        //         const float majorTheta =
        //             kTwoPI_f * static_cast<float>(major) / static_cast<float>(TorusDemoParams::kMajorSegments);
        //         const float cosMajor = std::cos(majorTheta);
        //         const float sinMajor = std::sin(majorTheta);
        //         for (size_t minor = 0; minor < TorusDemoParams::kMinorSegments; ++minor)
        //         {
        //             const float minorTheta =
        //                 kTwoPI_f * static_cast<float>(minor) / static_cast<float>(TorusDemoParams::kMinorSegments);
        //             const float cosMinor = std::cos(minorTheta);
        //             const float sinMinor = std::sin(minorTheta);
        //             const float radial = TorusDemoParams::kMajorRadius + (TorusDemoParams::kMinorRadius * cosMinor);

        //             verts[idx++] = Vec3f{
        //                 radial * cosMajor,
        //                 radial * sinMajor,
        //                 TorusDemoParams::kMinorRadius * sinMinor,
        //             };
        //         }
        //     }
        //     return verts;
        // }();

        // return kVertices;
        return mVertices;
    }

    virtual const array_view<FaceDesc, kFaceCount> &GetFaces() const override
    {
        return mFaces;
        // static const std::array<FaceDesc, kFaceCount> kFaces = []() {
        //     std::array<FaceDesc, kFaceCount> faces{};
        //     size_t idx = 0;
        //     for (size_t major = 0; major < TorusDemoParams::kMajorSegments; ++major)
        //     {
        //         const size_t nextMajor = (major + 1) % TorusDemoParams::kMajorSegments;
        //         for (size_t minor = 0; minor < TorusDemoParams::kMinorSegments; ++minor)
        //         {
        //             const size_t nextMinor = (minor + 1) % TorusDemoParams::kMinorSegments;
        //             const size_t i00 = (major * TorusDemoParams::kMinorSegments) + minor;
        //             const size_t i01 = (major * TorusDemoParams::kMinorSegments) + nextMinor;
        //             const size_t i11 = (nextMajor * TorusDemoParams::kMinorSegments) + nextMinor;
        //             const size_t i10 = (nextMajor * TorusDemoParams::kMinorSegments) + minor;

        //             faces[idx++] = FaceDesc{
        //                 static_cast<uint8_t>(i00),
        //                 static_cast<uint8_t>(i01),
        //                 static_cast<uint8_t>(i11),
        //                 static_cast<uint8_t>(i10),
        //             };
        //         }
        //     }
        //     return faces;
        // }();

        // return kFaces;
    }

    virtual void OnFacePrepared(size_t faceIndex, FaceRenderInfo &info) override
    {
        // const float breath = mMusicalStateTask.mMusicalState.mCurrentBreath01.GetValue();
        // const float ringPhase = kTwoPI_f * static_cast<float>(faceIndex % TorusDemoParams::kMinorSegments) /
        //                         static_cast<float>(TorusDemoParams::kMinorSegments);
        // const float accent = 1.0f + (0.35f * breath * fast::sin(ringPhase));
        // const float adjusted = Clamp(static_cast<float>(info.brightness) * accent, 0.0f, 255.0f);
        // info.brightness = static_cast<uint8_t>(adjusted);
    }
};

// struct TorusDemoApp : DisplayApp
// {
//     TorusSimulation mTorusSim;

//     TorusDemoApp(IDisplay &display, MusicalStateTask &musicalStateTask, uint32_t rngSeed = 0x51F00DF5u)
//         : DisplayApp(display), mTorusSim(display, musicalStateTask)
//     {
//     }

//     virtual void UpdateApp() override
//     {
//         if (mBack.IsNewlyPressed())
//         {
//             GoToFrontPage();
//         }
//     }

//     virtual void DisplayAppUpdate() override
//     {
//         DisplayApp::DisplayAppUpdate();
//     }

//     virtual const char *DisplayAppGetName() override
//     {
//         return "mesh demo";
//     }

//     virtual void RenderApp() override
//     {
//     }

//     virtual void RenderFrontPage() override
//     {
//         mTorusSim.StepMeshSimulation();
//         mTorusSim.RenderMeshFrame({14, 0, 100, 54});
//     }
// };

} // namespace clarinoid
