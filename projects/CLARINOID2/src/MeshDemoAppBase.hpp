#pragma once

#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/basic/Vec.hpp>
#include <clarinoid/basic/Rendering3D.hpp>
#include <clarinoid/basic/Random.hpp>
#include <clarinoid/application/Display.hpp>
#include <clarinoid/menu/MenuAppBase.hpp>

#include "clarinoid2MusicalStateTask.hpp"

namespace clarinoid
{

extern uint32_t gSynthVoiceNoteOnCount;

struct CommonDemoParams
{
    static constexpr float kCameraDistance = 3.1f;

    // Passive angular velocity about X/Y/Z (radians per frame) for idle motion.
    static constexpr float kAmbientAngularVelocityX = 0.00002f;
    static constexpr float kAmbientAngularVelocityY = 0.00003f;
    static constexpr float kAmbientAngularVelocityZ = 0.00004f;

    static constexpr float kBreathAngularVelocityFactor = 0.01f;

    // Exponential decay applied to angular velocity each frame (unitless 0-1).
    static constexpr float kAngularDamping = 0.985f;
    // Base and randomised portion of note-triggered impulses (radians per frame).
    static constexpr float kImpulseMagnitudeMin = 0.1f;
    static constexpr float kImpulseMagnitudeRange = 0.0f;
    // Maximum angular velocity per axis (radians per frame).
    static constexpr float kMaxAngularVelocity = 0.4f;

    // Directional light vector components (unitless, normalised internally).
    static constexpr float kLightDirX = -0.32f;
    static constexpr float kLightDirY = 0.74f;
    static constexpr float kLightDirZ = -0.58f;
};

template <typename ParamsT, size_t TVertexCount, size_t TFaceCount>
struct MeshSimulationBase
{
    using Params = ParamsT;
    static constexpr size_t kVertexCount = TVertexCount;
    static constexpr size_t kFaceCount = TFaceCount;

    IDisplay &mDisplay;
    MusicalStateTask &mMusicalStateTask;
    RandomNumberGenerator mRng;

    struct FaceRenderInfo
    {
        std::array<Vec2f, 4> projected{};
        float depth = 0.0f;
        uint8_t brightness = 0;
    };

    std::array<Vec3f, kVertexCount> mTransformedVertices{};
    std::array<FaceRenderInfo, kFaceCount> mFaceBuffer{};

    Vec3f mRotationAngles{};
    Vec3f mAngularVelocity{};
    Vec3f mLightDirectionNormal{};
    Vec2f mScreenOffsetPixels{0.0f, 0.0f};

    bool mMeshInitialized = false;
    int mLastNoteOnSerial = 0;

    struct FaceDesc
    {
        uint8_t i0;
        uint8_t i1;
        uint8_t i2;
        uint8_t i3;
    };

    MeshSimulationBase(IDisplay &display, MusicalStateTask &musicalStateTask)
        : mDisplay(display), mMusicalStateTask(musicalStateTask)
    {
    }

    void SetScreenOffsetPixels(float offsetX, float offsetY)
    {
        mScreenOffsetPixels.x = offsetX;
        mScreenOffsetPixels.y = offsetY;
    }

    Vec2f GetScreenOffsetPixels() const
    {
        return mScreenOffsetPixels;
    }

    void StepMeshSimulation()
    {
        EnsureMeshInitialized();

        HandleNoteImpulses();

        const float breath01 = mMusicalStateTask.mMusicalState.mCurrentBreath01.GetValue();

        mAngularVelocity.x += breath01 * mBreathAxis.x * CommonDemoParams::kBreathAngularVelocityFactor;
        mAngularVelocity.y += breath01 * mBreathAxis.y * CommonDemoParams::kBreathAngularVelocityFactor;
        mAngularVelocity.z += breath01 * mBreathAxis.z * CommonDemoParams::kBreathAngularVelocityFactor;

        mAngularVelocity.x += CommonDemoParams::kAmbientAngularVelocityX;
        mAngularVelocity.y += CommonDemoParams::kAmbientAngularVelocityY;
        mAngularVelocity.z += CommonDemoParams::kAmbientAngularVelocityZ;

        mAngularVelocity *= CommonDemoParams::kAngularDamping;
        ClampAngularVelocity();

        mRotationAngles += mAngularVelocity;
        WrapAngles();
    }

    void RenderMeshFrame(const RectI &renderRect)
    {
        // const RectI clientRect = mDisplay.GetClientRect();
        // const int originX = Params::kViewportX;
        // const int originY = Params::kViewportY; // clientRect.y + (clientRect.height - Params::kViewportHeight) / 2;
        // const RectI renderRect = RectI::Construct(originX, originY, Params::kViewportWidth, Params::kViewportHeight);
        mDisplay.SetClipRect(renderRect);

        const float centreX = static_cast<float>(renderRect.x) + (renderRect.width * 0.5f) + mScreenOffsetPixels.x;
        const float centreY = static_cast<float>(renderRect.y) + (renderRect.height * 0.5f) + mScreenOffsetPixels.y;

        const auto &baseVertices = GetBaseVertices();

        const float sx = static_cast<float>(fast::sin(mRotationAngles.x));
        const float cx = static_cast<float>(fast::cos(mRotationAngles.x));
        const float sy = static_cast<float>(fast::sin(mRotationAngles.y));
        const float cy = static_cast<float>(fast::cos(mRotationAngles.y));
        const float sz = static_cast<float>(fast::sin(mRotationAngles.z));
        const float cz = static_cast<float>(fast::cos(mRotationAngles.z));

        const Mat3f rotation = Mat3f::RotationXYZFromTrig(sx, cx, sy, cy, sz, cz);

        for (size_t i = 0; i < kVertexCount; ++i)
        {
            const Vec3f &v = baseVertices[i];
            Vec3f rotated = rotation * v;

            rotated.z += CommonDemoParams::kCameraDistance;
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

  protected:
    virtual const std::array<Vec3f, kVertexCount> &GetBaseVertices() const = 0;
    virtual const std::array<FaceDesc, kFaceCount> &GetFaces() const = 0;

    // breath-controlled angular velocity factors
    Vec3f mBreathAxis{GenerateImpulseAxis()};

    virtual uint8_t ResolveFaceBrightness(size_t /*faceIndex*/, float diffuse) const
    {
        const float brightnessF = Params::kFaceBaseBrightness + (diffuse * Params::kFaceDiffuseScale) +
                                  (fast::pow(diffuse, 4.0f) * Params::kFaceSpecularBias);
        return static_cast<uint8_t>(Clamp(brightnessF, 0.0f, 255.0f));
    }

    virtual Vec3f GenerateImpulseAxis()
    {
        Vec3f axis{mRng.NextFloatN11(), mRng.NextFloatN11(), mRng.NextFloatN11()};
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
        return CommonDemoParams::kImpulseMagnitudeMin + mRng.NextFloat01() * CommonDemoParams::kImpulseMagnitudeRange;
    }

    virtual void OnPostTransformVertices()
    {
    }

    virtual void OnFacePrepared(size_t /*faceIndex*/, FaceRenderInfo & /*info*/)
    {
    }

    void ApplyNoteImpulse()
    {
        mBreathAxis = GenerateImpulseAxis();

        const float impulseMagnitude = GenerateImpulseMagnitude();
        mAngularVelocity += mBreathAxis * impulseMagnitude; // give it a big push
    }

  private:
    void EnsureMeshInitialized()
    {
        if (mMeshInitialized)
        {
            return;
        }

        const Vec3f rawLight{CommonDemoParams::kLightDirX, CommonDemoParams::kLightDirY, CommonDemoParams::kLightDirZ};
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
        // for (int i = 0; i < delta; ++i) // it's not useful to stack impulses; just looks messy.
        {
            ApplyNoteImpulse();
        }
    }

    void ClampAngularVelocity()
    {
        const float maxMag = CommonDemoParams::kMaxAngularVelocity;
        mAngularVelocity.x = Clamp(mAngularVelocity.x, -maxMag, maxMag);
        mAngularVelocity.y = Clamp(mAngularVelocity.y, -maxMag, maxMag);
        mAngularVelocity.z = Clamp(mAngularVelocity.z, -maxMag, maxMag);
    }

    void WrapAngles()
    {
        mRotationAngles.x = WrapAngle(mRotationAngles.x);
        mRotationAngles.y = WrapAngle(mRotationAngles.y);
        mRotationAngles.z = WrapAngle(mRotationAngles.z);
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
