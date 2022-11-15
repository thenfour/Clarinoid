
#pragma once

#include <clarinoid/basic/Basic.hpp>

#include "HarmonizerSettings.hpp"
#include "SynthSettings.hpp"
#include "LoopstationSettings.hpp"
#include "ControlMapping.hpp"

namespace clarinoid
{

enum class GlobalScaleRefType : uint8_t
{
    Chosen,
    Deduced,
};

EnumItemInfo<GlobalScaleRefType> gGlobalScaleRefTypeItems[2] = {
    {GlobalScaleRefType::Chosen, "Chosen"},
    {GlobalScaleRefType::Deduced, "Deduced"},
};

EnumInfo<GlobalScaleRefType> gGlobalScaleRefTypeInfo("GlobalScaleRefType", gGlobalScaleRefTypeItems);

////////////////////////////////////////////////////////////////////////////////////////////////
struct ReverbSettings : ISerializationObjectMap<4>
{
    BoolParam mEnabled{true};
    FloatParam mGain{DecibelsToLinear(-3.0f)};
    FloatParam mDamping{0.6f};
    FloatParam mSize{0.6f};

    virtual SerializationObjectMapArray GetSerializationObjectMap() override
    {
        return {{
            CreateSerializationMapping(mEnabled, "Enabled"),
            CreateSerializationMapping(mGain, "Gain"),
            CreateSerializationMapping(mDamping, "Damping"),
            CreateSerializationMapping(mSize, "Size"),
        }};
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////
struct DelaySettings : ISerializationObjectMap<6>
{
    virtual SerializationObjectMapArray GetSerializationObjectMap() override
    {
        return {{
            CreateSerializationMapping(mEnabled, "Enabled"),
            CreateSerializationMapping(mGain, "Gain"),
            CreateSerializationMapping(mTime, "Time"),
            CreateSerializationMapping(mStereoSeparationDelayMS, "StereoSeparationDelayMS"),
            CreateSerializationMapping(mFeedbackGain, "FeedbackGain"),
            CreateSerializationMapping(mFilter, "Filter"),
        }};
    }

    BoolParam mEnabled{true};
    FloatParam mGain{DecibelsToLinear(-3.0f)};
    TimeWithBasisParam mTime{TimeBasis::Milliseconds, 250};
    FloatParam mStereoSeparationDelayMS{40};
    FloatParam mFeedbackGain{0.3f};
    FilterSettings mFilter; // = {"Filter"};
};

////////////////////////////////////////////////////////////////////////////////////////////////
struct PerformancePatch : ISerializationObjectMap<20>
{
  private:
    PerformancePatch &operator=(const PerformancePatch &rhs) = default;

  public:
    virtual SerializationObjectMapArray GetSerializationObjectMap() override
    {
        return {{
            CreateSerializationMapping(mName, "Name"),
            CreateSerializationMapping(mBPM, "BPM"),
            CreateSerializationMapping(mTranspose, "Transpose"),
            CreateSerializationMapping(mGlobalScaleRef, "GlobalScaleRef"),
            CreateSerializationMapping(mGlobalScale, "GlobalScale"),
            CreateSerializationMapping(mSynthPatchA, "SynthPatchA"),
            CreateSerializationMapping(mSynthAEnabled, "SynthAEnabled"),
            CreateSerializationMapping(mSynthAGain, "SynthAGain"),
            CreateSerializationMapping(mSynthPatchB, "SynthPatchB"),
            CreateSerializationMapping(mSynthBEnabled, "SynthBEnabled"),
            CreateSerializationMapping(mSynthBGain, "SynthBGain"),
            CreateSerializationMapping(mHarmPreset, "HarmPreset"),
            CreateSerializationMapping(mHarmEnabled, "HarmEnabled"),
            CreateSerializationMapping(mHarmGain, "HarmGain"),
            CreateSerializationMapping(mSynthStereoSpread, "SynthStereoSpread"),
            CreateSerializationMapping(mMasterGain, "MasterGain"),
            CreateSerializationMapping(mMasterFXGain, "MasterFXGain"),
            CreateSerializationMapping(mMasterFXEnable, "MasterFXEnable"),
            CreateSerializationMapping(mReverb, "Reverb"),
            CreateSerializationMapping(mDelay, "Delay"),
        }};
    }

    StringParam mName{"--"};

    FloatParam mBPM{104.0f};

    IntParam<int8_t> mTranspose{DEFAULT_TRANSPOSE};

    EnumParam<GlobalScaleRefType> mGlobalScaleRef{gGlobalScaleRefTypeInfo, GlobalScaleRefType::Deduced};
    ScaleParam mGlobalScale{Scale{Note::E, ScaleFlavorIndex::MajorPentatonic}}; // you can set this in menus
    ScaleParam mDeducedScale{
        Scale{Note::C, ScaleFlavorIndex::MajorPentatonic}}; // this is automatically populated always

    IntParam<int16_t> mSynthPatchA{0};
    BoolParam mSynthAEnabled{true};
    FloatParam mSynthAGain{1.0f};

    IntParam<int16_t> mSynthPatchB{-1}; // -1 = mute, no patch.
    BoolParam mSynthBEnabled{true};
    FloatParam mSynthBGain{1.0f};

    IntParam<int16_t> mHarmPreset{0};
    BoolParam mHarmEnabled{false};
    FloatParam mHarmGain{1.0f};

    FloatParam mSynthStereoSpread{0.35f}; // -1 to 1

    FloatParam mMasterGain{1.0f};
    FloatParam mMasterFXGain{1.0f};
    BoolParam mMasterFXEnable{true};
    ReverbSettings mReverb;
    DelaySettings mDelay;

    size_t mMyIndex;

    explicit PerformancePatch(size_t mIndex) : mMyIndex(mIndex)
    {
    }
    PerformancePatch(const PerformancePatch &rhs) = default;

    void CopyFrom(const PerformancePatch &rhs)
    {
        size_t mSavedIndex = mMyIndex;
        *this = rhs;
        mMyIndex = mSavedIndex;
    }


    String ToString() const
    {
        return String("") + mMyIndex + ":" + mName.GetValue();
    }
};

static constexpr auto aosenuthaoesuth = sizeof(PerformancePatch);

////////////////////////////////////////////////////////////////////////////////////////////////
struct MetronomeSettings : ISerializationObjectMap<5>
{
    virtual SerializationObjectMapArray GetSerializationObjectMap() override
    {
        return {{
            CreateSerializationMapping(mSoundOn, "SoundOn"),
            CreateSerializationMapping(mLEDOn, "LEDOn"),
            CreateSerializationMapping(mGain, "Gain"),
            CreateSerializationMapping(mMidiNote, "MidiNote"),
            CreateSerializationMapping(mDecayMS, "DecayMS"),
        }};
    }

    BoolParam mSoundOn{false};
    BoolParam mLEDOn{false};
    FloatParam mGain{0.34f};
    IntParam<uint8_t> mMidiNote{80};
    IntParam<int> mDecayMS{15};
};

////////////////////////////////////////////////////////////////////////////////////////////////
struct AppSettings : ISerializationObjectMap<3>
{
    virtual SerializationObjectMapArray GetSerializationObjectMap() override
    {
        return {{
            CreateSerializationMapping(mCurrentPerformancePatch, "currentPerformancePatch"),
            CreateSerializationMapping(mMetronome, "metronome"),
            CreateSerializationMapping(mPerformancePatches, "performancePatches"),
            // CreateSerializationMapping(mHarmSettings, "harmSettings"),
            // CreateSerializationMapping(mSynthSettings, "synthSettings"),
        }};
    }

    // one day these will be configurable and therefore part of the exported settings JSON.
    // until then, leave it out.
    ControlMapping mControlMappings[MAX_CONTROL_MAPPINGS];

    // UnipolarMapping mPitchUpMapping;
    // UnipolarMapping mPitchDownMapping;
    // int mNoteChangeSmoothingFrames = 3;
    // // the idea here is that the bigger the interval, the bigger the delay required to lock in the note
    // float mNoteChangeSmoothingIntervalFrameFactor = 0.30f;

    BoolParam mSustainPedalPolarity{true};

    BoolParam mDisplayDim{false};

    MetronomeSettings mMetronome;

    HarmSettings mHarmSettings;
    LooperSettings mLooperSettings;
    SynthSettings mSynthSettings;

    std::array<PerformancePatch, PERFORMANCE_PATCH_COUNT> mPerformancePatches{
        initialize_array_with_indices<PerformancePatch, PERFORMANCE_PATCH_COUNT>()};
    // ArraySerializer<PerformancePatch, PERFORMANCE_PATCH_COUNT> mPerformancePatchSerializer{"PerfPatches",
    //  mPerformancePatches};

    IntParam<uint16_t> mCurrentPerformancePatch{0};

    PerformancePatch &GetCurrentPerformancePatch()
    {
        return mPerformancePatches[RotateIntoRange(mCurrentPerformancePatch.GetValue(), mPerformancePatches.size())];
    }

    bool IsValidSynthPatchId(int16_t id)
    {
        if (id < 0 || (size_t)id >= SYNTH_PRESET_COUNT)
            return false;
        return true;
    }

    String GetSynthPatchName(int16_t id)
    {
        if (id < 0 || (size_t)id >= SYNTH_PRESET_COUNT)
            return String("<none>");
        return mSynthSettings.mPatches[id].ToString();
    }

    String GetHarmPatchName(int16_t id)
    {
        if (id < 0 || (size_t)id >= HARM_PRESET_COUNT)
            return String("<none>");
        return mHarmSettings.mPatches[id].ToString(id);
    }

    String GetPerfPatchName(int16_t id)
    {
        if (id < 0 || (size_t)id >= PERFORMANCE_PATCH_COUNT)
            return String("<none>");
        return mPerformancePatches[id].ToString();
    }

    HarmPatch &FindHarmPreset(int16_t id)
    {
        id = RotateIntoRange(id, HARM_PRESET_COUNT);
        return mHarmSettings.mPatches[id];
    }

    SynthPatch &FindSynthPreset(int16_t id)
    {
        id = RotateIntoRange(id, SYNTH_PRESET_COUNT);
        return mSynthSettings.mPatches[id];
    }

    SynthPatch &GetSynthPresetForHarmonizerLayer(const HarmPatch &harm, size_t voiceId)
    {
        CCASSERT(voiceId < HARM_VOICES);
        auto &perf = GetCurrentPerformancePatch();
        auto &voice = harm.mVoiceSettings[voiceId];
        switch (voice.mSynthPatchRef.GetValue())
        {
        case HarmSynthPresetRefType::GlobalA:
            return FindSynthPreset(perf.mSynthPatchA.GetValue());
        case HarmSynthPresetRefType::GlobalB:
            return FindSynthPreset(perf.mSynthPatchB.GetValue());
        case HarmSynthPresetRefType::Preset1:
            return FindSynthPreset(harm.mSynthPatch1.GetValue());
        case HarmSynthPresetRefType::Preset2:
            return FindSynthPreset(harm.mSynthPatch2.GetValue());
        case HarmSynthPresetRefType::Preset3:
            return FindSynthPreset(harm.mSynthPatch3.GetValue());
        case HarmSynthPresetRefType::Preset4:
            return FindSynthPreset(harm.mSynthPatch4.GetValue());
        case HarmSynthPresetRefType::Voice:
            return FindSynthPreset(voice.mSynthPatch.GetValue());
        }
        CCASSERT(!"unknown HarmSynthPresetRefType");
        return FindSynthPreset(perf.mSynthPatchA.GetValue());
    }
};

static constexpr auto appsettingssize = sizeof(AppSettings);
static constexpr auto appsettihtdngssize = sizeof(LFOSpec);
static constexpr auto appsettihtdnccgssize = sizeof(FloatParam);
static constexpr auto appsettingssize67 = sizeof(AppSettings::mHarmSettings);
static constexpr auto appsettingssize55 = sizeof(AppSettings::mPerformancePatches);
static constexpr auto appse55 = sizeof(PerformancePatch);
static constexpr auto app8se55 = sizeof(EnumParam<GlobalScaleRefType>);

static constexpr auto app8s4e55 = sizeof(BoolParam);
static constexpr auto app8s5e55 = sizeof(DelaySettings);
static constexpr auto app8s45e55 = sizeof(FloatParam);
static constexpr auto app8se6255 = sizeof(IntParam<int16_t>);
static constexpr auto app8se755 = sizeof(IntParam<int8_t>);
static constexpr auto app8se8855 = sizeof(ReverbSettings);
static constexpr auto app8oeuise55 = sizeof(ScaleParam);
static constexpr auto app8soeuie55 = sizeof(StringParam);

static constexpr auto appsettingssize44 = sizeof(AppSettings::mSynthSettings);
static constexpr auto rththth = sizeof(AppSettings::mControlMappings);

} // namespace clarinoid
