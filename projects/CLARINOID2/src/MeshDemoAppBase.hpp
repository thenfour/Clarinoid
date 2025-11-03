#pragma once

#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/basic/Vec.hpp>
#include <clarinoid/basic/Rendering3D.hpp>
#include <clarinoid/application/Display.hpp>

namespace clarinoid
{

extern uint32_t gSynthVoiceNoteOnCount;

struct MusicalStateTask;

template <typename ParamsT, size_t TVertexCount, size_t TFaceCount>
struct MeshDemoAppBase : DisplayApp
{
    using Params = ParamsT;
    static constexpr size_t kVertexCount = TVertexCount;
    static constexpr size_t kFaceCount = TFaceCount;

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

    MeshDemoAppBase(IDisplay &display, MusicalStateTask &musicalStateTask, uint32_t rngSeed = 0x51F00DF5u)
        : DisplayApp(display), mMusicalStateTask(musicalStateTask), mRngState(rngSeed)
    {
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
        StepMeshSimulation();
        RenderMeshFrame();
    }

  protected:
    virtual const std::array<Vec3f, kVertexCount> &GetBaseVertices() const = 0;
    virtual const std::array<FaceDesc, kFaceCount> &GetFaces() const = 0;

    virtual uint8_t ResolveFaceBrightness(size_t /*faceIndex*/, float diffuse) const
    {
        const float brightnessF = Params::kFaceBaseBrightness + (diffuse * Params::kFaceDiffuseScale) +
                                  (std::pow(diffuse, 4.0f) * Params::kFaceSpecularBias);
        return static_cast<uint8_t>(Clamp(brightnessF, 0.0f, 255.0f));
    }

    virtual Vec3f GenerateImpulseAxis()
    {
        Vec3f axis{NextRandomSignedFloat(), NextRandomSignedFloat(), NextRandomSignedFloat()};
        const float lenSq = LengthSq(axis);
        if (lenSq <= Params::kFaceNormalEpsilon * Params::kFaceNormalEpsilon)
        {
            axis = Vec3f{0.0f, 0.0f, 1.0f};
        }
        else
        {
            axis = Normalize(axis, Params::kFaceNormalEpsilon);
        }
        return axis;
    }

    virtual float GenerateImpulseMagnitude()
    {
        return Params::kImpulseMagnitudeMin +
               (static_cast<float>(NextRandomByte()) / 255.0f) * Params::kImpulseMagnitudeRange;
    }

    virtual void OnPostTransformVertices()
    {
    }

    virtual void OnFacePrepared(size_t /*faceIndex*/, FaceRenderInfo & /*info*/)
    {
    }

    MusicalStateTask &mMusicalStateTask;

    std::array<Vec3f, kVertexCount> mTransformedVertices{};
    std::array<FaceRenderInfo, kFaceCount> mFaceBuffer{};

    Vec3f mRotationAngles{};
    Vec3f mAngularVelocity{};
    Vec3f mLightDirectionNormal{};

    bool mMeshInitialized = false;
    uint32_t mRngState;
    int mLastNoteOnSerial = 0;

    void ApplyNoteImpulse()
    {
        Vec3f axis = GenerateImpulseAxis();
        const float impulseMagnitude = GenerateImpulseMagnitude();
        mAngularVelocity += axis * impulseMagnitude;
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

  private:
    void EnsureMeshInitialized()
    {
        if (mMeshInitialized)
        {
            return;
        }

        const Vec3f rawLight{Params::kLightDirX, Params::kLightDirY, Params::kLightDirZ};
        const float lightLenSq = LengthSq(rawLight);
        if (lightLenSq <= Params::kFaceNormalEpsilon * Params::kFaceNormalEpsilon)
        {
            mLightDirectionNormal = Vec3f{0.0f, 0.0f, 1.0f};
        }
        else
        {
            mLightDirectionNormal = Normalize(rawLight, Params::kFaceNormalEpsilon);
        }

        mRotationAngles = Vec3f{};
        mAngularVelocity = Vec3f{};
        mLastNoteOnSerial = gSynthVoiceNoteOnCount;
        mMeshInitialized = true;
    }

    void StepMeshSimulation()
    {
        EnsureMeshInitialized();

        HandleNoteImpulses();

        mAngularVelocity.x += Params::kAmbientAngularVelocityX;
        mAngularVelocity.y += Params::kAmbientAngularVelocityY;
        mAngularVelocity.z += Params::kAmbientAngularVelocityZ;

        mAngularVelocity *= Params::kAngularDamping;
        ClampAngularVelocity();

        mRotationAngles += mAngularVelocity;
        WrapAngles();
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

    void ClampAngularVelocity()
    {
        const float maxMag = Params::kMaxAngularVelocity;
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

    void RenderMeshFrame()
    {
        const RectI clientRect = mDisplay.GetClientRect();
        const int originX = clientRect.x + (clientRect.width - Params::kViewportWidth) / 2;
        const int originY = clientRect.y + (clientRect.height - Params::kViewportHeight) / 2;
        const RectI renderRect = RectI::Construct(originX, originY, Params::kViewportWidth, Params::kViewportHeight);
        mDisplay.SetClipRect(renderRect);

        const float centreX = static_cast<float>(renderRect.x) + (renderRect.width * 0.5f);
        const float centreY = static_cast<float>(renderRect.y) + (renderRect.height * 0.5f);

        const auto &baseVertices = GetBaseVertices();

        const float sx = static_cast<float>(std::sin(mRotationAngles.x));
        const float cx = static_cast<float>(std::cos(mRotationAngles.x));
        const float sy = static_cast<float>(std::sin(mRotationAngles.y));
        const float cy = static_cast<float>(std::cos(mRotationAngles.y));
        const float sz = static_cast<float>(std::sin(mRotationAngles.z));
        const float cz = static_cast<float>(std::cos(mRotationAngles.z));

        const float m00 = cz * cy;
        const float m01 = (cz * sy * sx) - (sz * cx);
        const float m02 = (cz * sy * cx) + (sz * sx);
        const float m10 = sz * cy;
        const float m11 = (sz * sy * sx) + (cz * cx);
        const float m12 = (sz * sy * cx) - (cz * sx);
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

            rotated.z += Params::kCameraDistance;
            rotated.z = std::max(rotated.z, Params::kNearPlaneEpsilon);
            mTransformedVertices[i] = rotated;
        }

        OnPostTransformVertices();

        size_t visibleFaceCount = 0;
        const auto &faces = GetFaces();
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
            if (normalLenSq <= Params::kFaceNormalEpsilon * Params::kFaceNormalEpsilon)
            {
                continue;
            }

            const Vec3f faceCentre = Vec3f{
                (v0.x + v1.x + v2.x + v3.x) * 0.25f,
                (v0.y + v1.y + v2.y + v3.y) * 0.25f,
                (v0.z + v1.z + v2.z + v3.z) * 0.25f,
            };

            const Vec3f toCamera{-faceCentre.x, -faceCentre.y, -faceCentre.z};
            const float viewDot = Dot(normal, toCamera);
            if (viewDot <= 0.0f)
            {
                continue;
            }

            normal = Normalize(normal, Params::kFaceNormalEpsilon);

            const float diffuse = Clamp(Dot(normal, mLightDirectionNormal), 0.0f, 1.0f);
            FaceRenderInfo &info = mFaceBuffer[visibleFaceCount];
            info.brightness = ResolveFaceBrightness(faceIndex, diffuse);
            info.depth = faceCentre.z;
            info.projected[0] = ProjectVertex(v0, centreX, centreY);
            info.projected[1] = ProjectVertex(v1, centreX, centreY);
            info.projected[2] = ProjectVertex(v2, centreX, centreY);
            info.projected[3] = ProjectVertex(v3, centreX, centreY);

            OnFacePrepared(faceIndex, info);

            ++visibleFaceCount;
        }

        std::sort(mFaceBuffer.begin(),
                  mFaceBuffer.begin() + visibleFaceCount,
                  [](const FaceRenderInfo &a, const FaceRenderInfo &b) { return a.depth > b.depth; });

        for (size_t i = 0; i < visibleFaceCount; ++i)
        {
            const FaceRenderInfo &info = mFaceBuffer[i];
            FillFace(info, renderRect);
        }

        mDisplay.ResetClip();
    }

    Vec2f ProjectVertex(const Vec3f &vertex, float centreX, float centreY) const
    {
        const float invZ = Params::kProjectionScale / vertex.z;
        const float px = centreX + (vertex.x * invZ);
        const float py = centreY - (vertex.y * invZ);
        return Vec2f{px, py};
    }

    void FillFace(const FaceRenderInfo &info, const RectI &renderRect)
    {
        const auto &p = info.projected;
        auto drawPixel = [this, brightness = info.brightness](int x, int y) {
            mDisplay.SetPixelShaded(PointI::Construct(x, y), brightness);
        };
        render3d::RasterizeTriangle(p[0], p[1], p[2], renderRect, drawPixel, Params::kFaceNormalEpsilon);
        render3d::RasterizeTriangle(p[2], p[3], p[0], renderRect, drawPixel, Params::kFaceNormalEpsilon);
    }
};

} // namespace clarinoid
