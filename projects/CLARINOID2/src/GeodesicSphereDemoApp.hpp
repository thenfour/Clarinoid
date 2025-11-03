#pragma once

#include <array>
#include <algorithm>
#include <cmath>

#include "MeshDemoAppBase.hpp"
#include "clarinoid2BigPerfDisplayApp.hpp"

namespace clarinoid
{

struct GeodesicSphereDemoParams
{
    static constexpr int kViewportWidth = 100;
    static constexpr int kViewportHeight = 54;

    // Positive Z offset keeping the sphere in front of the camera (model units).
    static constexpr float kCameraDistance = 3.1f;
    // Perspective multiplier applied during projection; larger values render a bigger sphere on screen.
    static constexpr float kProjectionScale = 104.0f;
    // Minimum allowed Z value when projecting (model units) to avoid divide-by-zero.
    static constexpr float kNearPlaneEpsilon = 0.12f;

    // Passive angular velocity about X/Y/Z (radians per frame) for idle motion.
    static constexpr float kAmbientAngularVelocityX = 0.00009f;
    static constexpr float kAmbientAngularVelocityY = 0.00008f;
    static constexpr float kAmbientAngularVelocityZ = 0.00007f;
    // Exponential decay applied to angular velocity each frame (unitless 0-1).
    static constexpr float kAngularDamping = 0.99f;
    // Base and randomised portion of note-triggered impulses (radians per frame).
    static constexpr float kImpulseMagnitudeMin = 0.03f;
    static constexpr float kImpulseMagnitudeRange = 0.0f;
    // Maximum angular velocity per axis (radians per frame).
    static constexpr float kMaxAngularVelocity = 0.38f;

    // Face shading parameters: ambient brightness offset, diffuse scale, specular lift, and normal epsilon.
    static constexpr float kFaceBaseBrightness = 6.0f;
    static constexpr float kFaceDiffuseScale = 100.0f;
    static constexpr float kFaceSpecularBias = 200.0f;
    static constexpr float kFaceNormalEpsilon = 1e-4f;

    // Directional light vector components (unitless, normalised internally).
    static constexpr float kLightDirX = -0.32f;
    static constexpr float kLightDirY = 0.74f;
    static constexpr float kLightDirZ = -0.58f;

    // Number of recursive icosahedron subdivisions (0-2 keeps vertex count within uint8_t indices).
    static constexpr size_t kSubdivisionDepth = 1;

    static constexpr size_t kFrequency = size_t{1} << kSubdivisionDepth;
    static constexpr size_t kVertexCount = (10u * kFrequency * kFrequency) + 2u;
    static constexpr size_t kFaceCount = 20u * kFrequency * kFrequency;
    static constexpr size_t kMaxEdgeCount = 30u * kFrequency * kFrequency;

    static_assert(
        kVertexCount <= 255u,
        "MeshDemoAppBase::FaceDesc uses uint8_t indices; increase width to uint16_t before using higher subdivision.");
};

namespace detail
{
struct GeodesicSphereMeshData
{
    std::array<Vec3f, GeodesicSphereDemoParams::kVertexCount> vertices{};
    std::array<std::array<uint32_t, 3>, GeodesicSphereDemoParams::kFaceCount> faces{};
};

inline GeodesicSphereMeshData GenerateGeodesicSphereMesh()
{
    GeodesicSphereMeshData mesh{};
    auto &verts = mesh.vertices;
    auto &faces = mesh.faces;

    std::array<std::array<uint32_t, 3>, GeodesicSphereDemoParams::kFaceCount> workingFaces{};

    size_t vertCount = 0;
    size_t faceCount = 0;

    const float phi = (1.0f + std::sqrt(5.0f)) * 0.5f;
    const float invLen = 1.0f / std::sqrt(1.0f + (phi * phi));

    auto addVertex = [&](float x, float y, float z) {
        CCASSERT(vertCount < GeodesicSphereDemoParams::kVertexCount);
        Vec3f v{x * invLen, y * invLen, z * invLen};
        verts[vertCount++] = Normalize(v, GeodesicSphereDemoParams::kFaceNormalEpsilon);
        return static_cast<uint32_t>(vertCount - 1u);
    };

    auto addFace = [&](uint32_t a, uint32_t b, uint32_t c) {
        CCASSERT(faceCount < GeodesicSphereDemoParams::kFaceCount);
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
        std::array<uint64_t, GeodesicSphereDemoParams::kMaxEdgeCount> keys{};
        std::array<uint32_t, GeodesicSphereDemoParams::kMaxEdgeCount> values{};
        size_t count = 0;

        uint32_t FindOrCreate(uint32_t a,
                              uint32_t b,
                              std::array<Vec3f, GeodesicSphereDemoParams::kVertexCount> &vertsRef,
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

            CCASSERT(count < GeodesicSphereDemoParams::kMaxEdgeCount);
            CCASSERT(vertCountRef < GeodesicSphereDemoParams::kVertexCount);
            Vec3f midpoint = vertsRef[lo] + vertsRef[hi];
            midpoint *= 0.5f;
            midpoint = Normalize(midpoint, GeodesicSphereDemoParams::kFaceNormalEpsilon);

            const uint32_t index = static_cast<uint32_t>(vertCountRef);
            vertsRef[vertCountRef++] = midpoint;

            keys[count] = key;
            values[count] = index;
            ++count;
            return index;
        }
    };

    std::array<std::array<uint32_t, 3>, GeodesicSphereDemoParams::kFaceCount> nextFaces{};

    for (size_t iteration = 0; iteration < GeodesicSphereDemoParams::kSubdivisionDepth; ++iteration)
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

    CCASSERT(vertCount == GeodesicSphereDemoParams::kVertexCount);
    CCASSERT(faceCount == GeodesicSphereDemoParams::kFaceCount);

    for (size_t i = 0; i < faceCount; ++i)
    {
        faces[i] = workingFaces[i];
    }

    return mesh;
}

inline const GeodesicSphereMeshData &GetGeodesicSphereMeshData()
{
    static const GeodesicSphereMeshData data = GenerateGeodesicSphereMesh();
    return data;
}
} // namespace detail

using GeodesicSphereDemoBase = MeshSimulationBase<GeodesicSphereDemoParams,
                                                  GeodesicSphereDemoParams::kVertexCount,
                                                  GeodesicSphereDemoParams::kFaceCount>;

struct GeodesicSphereSimulation : GeodesicSphereDemoBase
{
    using Base = GeodesicSphereDemoBase;
    using FaceDesc = typename Base::FaceDesc;
    using FaceRenderInfo = typename Base::FaceRenderInfo;

    static constexpr size_t kVertexCount = Base::kVertexCount;
    static constexpr size_t kFaceCount = Base::kFaceCount;

    GeodesicSphereSimulation(IDisplay &display, MusicalStateTask &musicalStateTask)
        : Base(display, musicalStateTask, 0xC0FEF00Du)
    {
    }

  protected:
    virtual const std::array<Vec3f, kVertexCount> &GetBaseVertices() const override
    {
        return detail::GetGeodesicSphereMeshData().vertices;
    }

    virtual const std::array<FaceDesc, kFaceCount> &GetFaces() const override
    {
        static const std::array<FaceDesc, kFaceCount> kFaces = []() {
            std::array<FaceDesc, kFaceCount> faceBuffer{};
            const auto &mesh = detail::GetGeodesicSphereMeshData();
            for (size_t i = 0; i < kFaceCount; ++i)
            {
                const auto &tri = mesh.faces[i];
                faceBuffer[i] = FaceDesc{static_cast<uint8_t>(tri[0]),
                                         static_cast<uint8_t>(tri[1]),
                                         static_cast<uint8_t>(tri[2]),
                                         static_cast<uint8_t>(tri[2])};
            }
            return faceBuffer;
        }();

        return kFaces;
    }

    virtual void OnFacePrepared(size_t faceIndex, FaceRenderInfo &info) override
    {
        // const float breath = mMusicalStateTask.mMusicalState.mCurrentBreath01.GetValue();
        // const float note = mMusicalStateTask.mMusicalState.mCurrentPitchN11.GetValue();
        // const float modulation = 1.0f + (0.25f * breath) + (0.15f * note);
        // const float adjusted = Clamp(static_cast<float>(info.brightness) * modulation, 0.0f, 255.0f);
        // info.brightness = static_cast<uint8_t>(adjusted);

        // (void)faceIndex;
    }
};

struct GeodesicSphereDemoApp : DisplayApp
{
    GeodesicSphereSimulation mGeodesicSphereSim;
    MusicalStateTask &mMusicalStateTask;
    ISysInfoProvider &mSysInfoProvider;

    GeodesicSphereDemoApp(IDisplay &display,
                          MusicalStateTask &musicalStateTask,
                          ISysInfoProvider &sysInfoProvider,
                          uint32_t rngSeed = 0x51F00DF5u)
        : DisplayApp(display), mGeodesicSphereSim(display, musicalStateTask), mMusicalStateTask(musicalStateTask),
          mSysInfoProvider(sysInfoProvider)
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

    virtual const char *DisplayAppGetName() override
    {
        return "mesh demo";
    }

    virtual void RenderApp() override
    {
    }

    bool AllowOverlayIndicators() const override
    {
        return false;
    }

    virtual void RenderFrontPage() override
    {
        mGeodesicSphereSim.StepMeshSimulation();
        mGeodesicSphereSim.SetScreenOffsetPixels(9, -9);
        mGeodesicSphereSim.RenderMeshFrame({72, 0, 56, 56});

        auto &appSettings = *mMusicalStateTask.mAppSettings;
        auto &perf = appSettings.GetCurrentPerformancePatch();

        static constexpr int kFingeredNoteRowY = 12;
        static constexpr int kTextAreaWidth = 72;
        static constexpr int kPlayingNotesRowY = 33;
        auto kPadding = RectI::Construct(1, 1, 1, 1);

        if (perf.mSynthAEnabled && (perf.mSynthPresetA != -1))
        {
            mDisplay.setCursor(1, 1);
            mDisplay.PrintInvertedText("A", kPadding);
        }

        if (perf.mSynthBEnabled && (perf.mSynthPresetB != -1))
        {
            mDisplay.setCursor(12, 1);
            mDisplay.PrintInvertedText("B", kPadding);
        }

        if (perf.mHarmEnabled)
        {
            mDisplay.setCursor(24, 1);
            mDisplay.PrintInvertedText("H", kPadding);
            // String s = String("H") + perf.mHarmPreset + ":" + perf.mGlobalScale.ToShortString();
            mDisplay.setCursor(36, 1);
            mDisplay.PrintInvertedText(perf.mGlobalScale.ToShortString().substring(0, 6),
                                       kPadding); // ppSettings.GetHarmPatchName(perf.mHarmPreset));
        }

        if (perf.mTranspose != 0)
        {
            mDisplay.setCursor(1, kFingeredNoteRowY);
            mDisplay.PrintInvertedText(String(perf.mTranspose > 0 ? "+" : "") + perf.mTranspose, kPadding);
        }

        if (perf.mSynthATranspose != 0)
        {
            mDisplay.setCursor(kTextAreaWidth - 14, kFingeredNoteRowY);
            mDisplay.PrintInvertedText(String(perf.mSynthATranspose > 0 ? "+" : "") + perf.mSynthATranspose, kPadding);
        }
        if (perf.mSynthBTranspose != 0)
        {
            mDisplay.setCursor(kTextAreaWidth - 14, kFingeredNoteRowY + 9);
            mDisplay.PrintInvertedText(String(perf.mSynthBTranspose > 0 ? "+" : "") + perf.mSynthBTranspose, kPadding);
        }

        mDisplay.setTextColor(SSD1306_WHITE); // normal text

        const auto fingeredNote = mMusicalStateTask.mMusicalState.mFingeredNote;
        auto fingeredNoteName = fingeredNote.ToStringWithOctave();
        auto fingeredNoteBounds = mDisplay.GetTextBounds(fingeredNoteName);
        // center in text area
        mDisplay.setCursor((kTextAreaWidth - fingeredNoteBounds.width * 2) / 2, kFingeredNoteRowY);
        mDisplay.SetFontScale(2, 2);
        mDisplay.print(fingeredNoteName);
        mDisplay.SetFontScale(1, 1);

        // now playing notes.
        {
            auto voices = mSysInfoProvider.ISysInfoProvider_GetVoiceState();

            fixed_vector<SynthVoiceState, MAX_SYNTH_VOICES> voiceState;
            for (auto &v : voices)
            {
                if (v.mIsPlaying)
                {
                    voiceState.push_back(v);
                }
            }

            std::sort(voiceState.begin(),
                      voiceState.end(), //
                      [](const SynthVoiceState &a, const SynthVoiceState &b) {
                          return a.mNote.GetMidiValue() < b.mNote.GetMidiValue();
                      });

            fixed_vector<MidiNote, MAX_SYNTH_VOICES> chordNotes;
            MidiNote lastNote{0};
            for (auto &v : voiceState)
            {
                if (v.mNote.GetMidiValue() == lastNote.GetMidiValue())
                {
                    continue;
                }
                chordNotes.push_back(v.mNote);
                lastNote = v.mNote;
            }
            auto spelledChord = spellChord(chordNotes).notes;

            for (int i = 0; i < (int)spelledChord.size(); ++i)
            {
                auto &tone = spelledChord[i];
                const int x = kTextAreaWidth * (i) / (spelledChord.size());
                mDisplay.setCursor(x, kPlayingNotesRowY);
                mDisplay.print(tone.ToString(false)); // becasue we eliminate duplicates, and they're always sorted from
                                                      // low to high, not necessary to include octave.
            }
        }

        BigPerfDisplayApp::RenderPitchbendBar(
            mDisplay, mMusicalStateTask.mMusicalState.mCurrentPitchN11.GetValue(), 51, kTextAreaWidth);
    }
};

} // namespace clarinoid
