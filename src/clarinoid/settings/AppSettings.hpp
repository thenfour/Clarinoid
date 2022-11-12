
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
struct ReverbSettings //: SerializableDictionary
{
    BoolParam mEnabled{true};
    FloatParam mGain{DecibelsToLinear(-3.0f)};
    FloatParam mDamping{0.6f};
    FloatParam mSize{0.6f};

    ReverbSettings() //: SerializableDictionary("Reverb", mSerializableChildObjects)
    {
    }

    // SerializableObject *mSerializableChildObjects[4] = {
    //     &mEnabled,
    //     &mGain,
    //     &mDamping,
    //     &mSize,
    // };

    bool SerializableObject_ToJSON(JsonVariant rhs) const
    {
        bool r = true;
        r = r && mEnabled.SerializableObject_ToJSON(rhs.createNestedObject("on"));
        r = r && mGain.SerializableObject_ToJSON(rhs.createNestedObject("gain"));
        r = r && mDamping.SerializableObject_ToJSON(rhs.createNestedObject("damp"));
        r = r && mSize.SerializableObject_ToJSON(rhs.createNestedObject("size"));
        return r;
    }

    Result SerializableObject_Deserialize(JsonVariant obj)
    {
        if (!obj.is<JsonObject>())
        {
            return Result::Failure("must be object");
        }

        Result ret = Result::Success();
        ret.AndRequires(mEnabled.SerializableObject_Deserialize(obj["on"]), "on");
        ret.AndRequires(mGain.SerializableObject_Deserialize(obj["gain"]), "gain");
        ret.AndRequires(mDamping.SerializableObject_Deserialize(obj["damp"]), "damp");
        ret.AndRequires(mSize.SerializableObject_Deserialize(obj["size"]), "size");
        return ret;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////
struct DelaySettings // : SerializableDictionary
{
    BoolParam mEnabled{true};
    FloatParam mGain{DecibelsToLinear(-3.0f)};
    TimeWithBasisParam mTime{TimeBasis::Milliseconds, 250};
    FloatParam mStereoSeparationDelayMS{40};
    FloatParam mFeedbackGain{0.3f};
    FilterSettings mFilter; // = {"Filter"};
    // ClarinoidFilterType mDelayFilterType = ClarinoidFilterType::BP_Moog2;
    // SerializableFloat mDelayCutoffFrequency = 1000;
    // SerializableFloat mDelaySaturation = 0.2f;
    // SerializableFloat mDelayQ = 0.1f;

    // DelaySettings() : SerializableDictionary("Delay", mSerializableChildObjects)
    // {
    // }

    // SerializableObject *mSerializableChildObjects[6] = {
    //     &mEnabled,
    //     &mGain,
    //     &mTime,
    //     &mStereoSeparationDelayMS,
    //     &mFeedbackGain,
    //     &mFilter,
    // };

    bool SerializableObject_ToJSON(JsonVariant rhs) const
    {
        bool r = true;
        r = r && mEnabled.SerializableObject_ToJSON(rhs.createNestedObject("on"));
        r = r && mGain.SerializableObject_ToJSON(rhs.createNestedObject("gain"));
        r = r && mTime.SerializableObject_ToJSON(rhs.createNestedObject("time"));
        r = r && mStereoSeparationDelayMS.SerializableObject_ToJSON(rhs.createNestedObject("sep"));
        r = r && mFeedbackGain.SerializableObject_ToJSON(rhs.createNestedObject("fb"));
        r = r && mFilter.SerializableObject_ToJSON(rhs.createNestedObject("filt"));
        return r;
    }

    Result SerializableObject_Deserialize(JsonVariant obj)
    {
        if (!obj.is<JsonObject>())
        {
            return Result::Failure("must be object");
        }

        Result ret = Result::Success();
        ret.AndRequires(mEnabled.SerializableObject_Deserialize(obj["on"]), "on");
        ret.AndRequires(mGain.SerializableObject_Deserialize(obj["gain"]), "gain");
        ret.AndRequires(mTime.SerializableObject_Deserialize(obj["time"]), "time");
        ret.AndRequires(mStereoSeparationDelayMS.SerializableObject_Deserialize(obj["sep"]), "sep");
        ret.AndRequires(mFeedbackGain.SerializableObject_Deserialize(obj["fb"]), "fb");
        ret.AndRequires(mFilter.SerializableObject_Deserialize(obj["filt"]), "filt");
        return ret;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////
struct PerformancePatch
{
  private:
    PerformancePatch &operator=(const PerformancePatch &rhs) = default;

  public:
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

    bool SerializableObject_ToJSON(JsonVariant rhs) const
    {
        bool r = true;
        r = r && mName.SerializableObject_ToJSON(rhs.createNestedObject("name"));
        r = r && mBPM.SerializableObject_ToJSON(rhs.createNestedObject("bpm"));
        r = r && mTranspose.SerializableObject_ToJSON(rhs.createNestedObject("trans"));
        r = r && mGlobalScaleRef.SerializableObject_ToJSON(rhs.createNestedObject("ScaleRef"));
        r = r && mGlobalScale.SerializableObject_ToJSON(rhs.createNestedObject("Scale"));

        r = r && mSynthPatchA.SerializableObject_ToJSON(rhs.createNestedObject("Apatch"));
        r = r && mSynthAEnabled.SerializableObject_ToJSON(rhs.createNestedObject("Aon"));
        r = r && mSynthAGain.SerializableObject_ToJSON(rhs.createNestedObject("Again"));

        r = r && mSynthPatchB.SerializableObject_ToJSON(rhs.createNestedObject("Bpatch"));
        r = r && mSynthBEnabled.SerializableObject_ToJSON(rhs.createNestedObject("Bon"));
        r = r && mSynthBGain.SerializableObject_ToJSON(rhs.createNestedObject("Bgain"));

        r = r && mHarmPreset.SerializableObject_ToJSON(rhs.createNestedObject("Hpatch"));
        r = r && mHarmEnabled.SerializableObject_ToJSON(rhs.createNestedObject("Hon"));
        r = r && mHarmGain.SerializableObject_ToJSON(rhs.createNestedObject("Hgain"));

        r = r && mSynthStereoSpread.SerializableObject_ToJSON(rhs.createNestedObject("spread"));

        r = r && mMasterGain.SerializableObject_ToJSON(rhs.createNestedObject("mstGain"));
        r = r && mMasterFXGain.SerializableObject_ToJSON(rhs.createNestedObject("fxgain"));
        r = r && mMasterFXEnable.SerializableObject_ToJSON(rhs.createNestedObject("fxon"));
        r = r && mReverb.SerializableObject_ToJSON(rhs.createNestedObject("verb"));
        r = r && mDelay.SerializableObject_ToJSON(rhs.createNestedObject("delay"));
        return r;
    }

    Result SerializableObject_Deserialize(JsonVariant obj)
    {
        if (!obj.is<JsonObject>())
        {
            return Result::Failure("must be object");
        }

        Result ret = Result::Success();

        ret.AndRequires(mName.SerializableObject_Deserialize(obj["name"]), "name");
        ret.AndRequires(mBPM.SerializableObject_Deserialize(obj["bpm"]), "bpm");
        ret.AndRequires(mTranspose.SerializableObject_Deserialize(obj["trans"]), "trans");
        ret.AndRequires(mGlobalScaleRef.SerializableObject_Deserialize(obj["ScaleRef"]), "ScaleRef");
        ret.AndRequires(mGlobalScale.SerializableObject_Deserialize(obj["Scale"]), "Scale");
        ret.AndRequires(mSynthPatchA.SerializableObject_Deserialize(obj["Apatch"]), "Apatch");
        ret.AndRequires(mSynthAEnabled.SerializableObject_Deserialize(obj["Aon"]), "Aon");
        ret.AndRequires(mSynthAGain.SerializableObject_Deserialize(obj["Again"]), "Again");
        ret.AndRequires(mSynthPatchB.SerializableObject_Deserialize(obj["Bpatch"]), "Bpatch");
        ret.AndRequires(mSynthBEnabled.SerializableObject_Deserialize(obj["Bon"]), "Bon");
        ret.AndRequires(mSynthBGain.SerializableObject_Deserialize(obj["Bgain"]), "Bgain");
        ret.AndRequires(mHarmPreset.SerializableObject_Deserialize(obj["Hpatch"]), "Hpatch");
        ret.AndRequires(mHarmEnabled.SerializableObject_Deserialize(obj["Hon"]), "Hon");
        ret.AndRequires(mHarmGain.SerializableObject_Deserialize(obj["Hgain"]), "Hgain");
        ret.AndRequires(mSynthStereoSpread.SerializableObject_Deserialize(obj["spread"]), "spread");
        ret.AndRequires(mMasterGain.SerializableObject_Deserialize(obj["mstGain"]), "mstGain");
        ret.AndRequires(mMasterFXGain.SerializableObject_Deserialize(obj["fxgain"]), "fxgain");
        ret.AndRequires(mMasterFXEnable.SerializableObject_Deserialize(obj["fxon"]), "fxon");
        ret.AndRequires(mReverb.SerializableObject_Deserialize(obj["verb"]), "verb");
        ret.AndRequires(mDelay.SerializableObject_Deserialize(obj["delay"]), "delay");

        return ret;
    }

    String ToString() const
    {
        return String("") + mMyIndex + ":" + mName.GetValue();
    }
};

static constexpr auto aosenuthaoesuth = sizeof(PerformancePatch);

////////////////////////////////////////////////////////////////////////////////////////////////
struct MetronomeSettings // : SerializableDictionary
{
    // MetronomeSettings() : SerializableDictionary("Metronome", mSerializableChildObjects)
    // {
    // }

    BoolParam mSoundOn{false};
    BoolParam mLEDOn{false};
    FloatParam mGain{0.34f};
    IntParam<uint8_t> mMidiNote{80};
    IntParam<int> mDecayMS{15};

    // SerializableInt<int> mMetronomeBrightness = { "", 255};
    // float mMetronomeLEDDecay = 0.1f;

    // SerializableObject *mSerializableChildObjects[5] = {
    //     &mSoundOn,
    //     &mLEDOn,
    //     &mGain,
    //     &mMidiNote,
    //     &mDecayMS,
    // };
    bool SerializableObject_ToJSON(JsonVariant rhs) const
    {
        bool ret = true;
        ret = ret && mSoundOn.SerializableObject_ToJSON(rhs.createNestedObject("snd"));
        ret = ret && mLEDOn.SerializableObject_ToJSON(rhs.createNestedObject("led"));
        ret = ret && mGain.SerializableObject_ToJSON(rhs.createNestedObject("gain"));
        ret = ret && mMidiNote.SerializableObject_ToJSON(rhs.createNestedObject("note"));
        ret = ret && mDecayMS.SerializableObject_ToJSON(rhs.createNestedObject("decay"));
        return ret;
    }

    Result SerializableObject_Deserialize(JsonVariant obj)
    {
        if (!obj.is<JsonObject>())
        {
            return Result::Failure("metronome must be object");
        }

        Result ret = Result::Success();
        ret.AndRequires(mSoundOn.SerializableObject_Deserialize(obj["snd"]), "snd");
        ret.AndRequires(mLEDOn.SerializableObject_Deserialize(obj["led"]), "led");
        ret.AndRequires(mGain.SerializableObject_Deserialize(obj["gain"]), "gain");
        ret.AndRequires(mMidiNote.SerializableObject_Deserialize(obj["note"]), "note");
        ret.AndRequires(mDecayMS.SerializableObject_Deserialize(obj["decay"]), "decay");
        return ret;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////
struct AppSettings // : SerializableDictionary
{
    // one day these will be configurable and therefore part of the exported settings JSON.
    // until then, leave it out.
    ControlMapping mControlMappings[MAX_CONTROL_MAPPINGS];

    // UnipolarMapping mPitchUpMapping;
    // UnipolarMapping mPitchDownMapping;
    // int mNoteChangeSmoothingFrames = 3;
    // // the idea here is that the bigger the interval, the bigger the delay required to lock in the note
    // float mNoteChangeSmoothingIntervalFrameFactor = 0.30f;

    BoolParam mSustainPedalPolarity { true};

    BoolParam mDisplayDim { false};

    MetronomeSettings mMetronome;

    HarmSettings mHarmSettings;
    LooperSettings mLooperSettings;
    SynthSettings mSynthSettings;

    std::array<PerformancePatch, PERFORMANCE_PATCH_COUNT> mPerformancePatches{
        initialize_array_with_indices<PerformancePatch, PERFORMANCE_PATCH_COUNT>()};
    // ArraySerializer<PerformancePatch, PERFORMANCE_PATCH_COUNT> mPerformancePatchSerializer{"PerfPatches",
    //  mPerformancePatches};

    IntParam<uint16_t> mCurrentPerformancePatch{ 0};

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

    // AppSettings() : SerializableDictionary("AppSettings", mSerializableChildObjects)
    // {
    // }

    bool SerializableObject_ToJSON(JsonVariant doc) const
    {
        // SerializableObject_ToJSON(doc);
        //  String ret;
        bool ret = true;

        ret = ret && mCurrentPerformancePatch.SerializableObject_ToJSON(doc.createNestedObject("perfPatch"));
        ret = ret && mMetronome.SerializableObject_ToJSON(doc.createNestedObject("metronome"));
        ret = ret && SerializeArrayToJSON(doc.createNestedArray("perfPatches"), mPerformancePatches);
        ret = ret && mHarmSettings.SerializableObject_ToJSON(doc.createNestedObject("harm"));
        ret = ret && mSynthSettings.SerializableObject_ToJSON(doc.createNestedObject("synth"));
        // synth settings

        // return serializeJson(doc, Serial); // minified
        // size_t ret = serializeMsgPack(doc, *pOut); // minified
        // serializeJsonPretty(doc, ret); // pretty
        // return "";
        return ret;
    }

    Result SerializableObject_Deserialize(JsonVariant obj)
    {
        if (!obj.is<JsonObject>())
        {
            return Result::Failure("AppSettings is not a json object");
        }

        Result ret = Result::Success();
        ret.AndRequires(mCurrentPerformancePatch.SerializableObject_Deserialize(obj["perfPatch"]), "perfPatch");
        ret.AndRequires(mMetronome.SerializableObject_Deserialize(obj["metronome"]), "metronome");
        ret.AndRequires(DeserializeArray(obj["perfPatches"], mPerformancePatches), "perfPatches");
        ret.AndRequires(mHarmSettings.SerializableObject_Deserialize(obj["harm"]), "harm");
        ret.AndRequires(mSynthSettings.SerializableObject_Deserialize(obj["synth"]), "synth");

        return ret;
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
