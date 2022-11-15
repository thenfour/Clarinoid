#pragma once

namespace clarinoid
{

template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
struct IntParam
{
    T mValue;

    T GetValue() const
    {
        return mValue;
    }
    void SetValue(T v)
    {
        mValue = v;
    }

    explicit IntParam(T defaultValue) : mValue(defaultValue)
    {
    }
    // IntParam(const IntParam<T> &rhs) = default;
    // IntParam<T> &operator=(const IntParam<T> &rhs) = default;

    // Result SerializableObject_ToJSON(JsonVariant rhs) const
    // {
    //     return {rhs.set(mValue)};
    // }

    // Result SerializableObject_Deserialize(JsonVariant obj)
    // {
    //     if (!obj.is<T>())
    //         return Result::Failure("expected integer");
    //     mValue = obj.as<T>();
    //     return Result::Success();
    // }
};

template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
Result Serialize(JsonVariantWriter &myval, IntParam<T> &param)
{
    return myval.WriteNumberValue(param.mValue);
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
Result Deserialize(JsonVariantReader &myval, IntParam<T> &param)
{
    if (myval.mType != JsonDataType::Number)
    {
        return Result::Failure("expected number");
    }
    param.mValue = myval.mNumericValue.Get<T>();
    return Result::Success();
}

static constexpr size_t arcih = sizeof(IntParam<int>);

// serializing floats is fun. on one hand, using the decimal representation is terrible,
// on the other hand i want it to be human-readable.
struct FloatParam
{
    using T = float;
    T mValue;

    T GetValue() const
    {
        return mValue;
    }
    void SetValue(T v)
    {
        mValue = v;
    }

    explicit FloatParam(float defaultValue) : mValue(defaultValue)
    {
    }
    FloatParam(const FloatParam &rhs) = default;
    FloatParam &operator=(const FloatParam &rhs) = default;

    // Result SerializableObject_ToJSON(JsonVariant rhs) const
    // {
    //     return {rhs.set(mValue)};
    // }

    // Result SerializableObject_Deserialize(JsonVariant obj)
    // {
    //     Result ret = Result::Success();
    //     if (!ret.Requires(obj.is<T>(), "expected float"))
    //         return ret;
    //     mValue = obj.as<T>();
    //     return ret;
    // }
};

Result Serialize(JsonVariantWriter &myval, FloatParam &param)
{
    return myval.WriteNumberValue(param.mValue);
}

Result Deserialize(JsonVariantReader &myval, FloatParam &param)
{
    if (myval.mType != JsonDataType::Number)
    {
        return Result::Failure("expected number");
    }
    param.mValue = myval.mNumericValue.Get<float>();
    return Result::Success();
}

// serializing floats is fun. on one hand, using the decimal representation is terrible,
// on the other hand i want it to be human-readable.
struct BoolParam //: ParameterBase
{
    using T = bool;
    T mValue = false; // making public, in case you need a reference. (see GuiPerformanceApp.hpp)

    T GetValue() const
    {
        return mValue;
    }
    void SetValue(T v)
    {
        mValue = v;
    }

    BoolParam() = default;

    explicit BoolParam(bool initialValue) : mValue(initialValue)
    {
    }

    // Result SerializableObject_ToJSON(JsonVariant rhs) const
    // {
    //     return {rhs.set(mValue ? 1 : 0)};
    // }

    // Result SerializableObject_Deserialize(JsonVariant obj)
    // {
    //     if (!obj.is<int>())
    //         return Result::Failure("expected 1 or 0");
    //     int t = obj.as<int>();
    //     mValue = (t != 0);
    //     return Result::Success();
    // }
};

Result Serialize(JsonVariantWriter &myval, BoolParam &param)
{
    return myval.WriteNumberValue(param.mValue ? 1 : 0);
}

Result Deserialize(JsonVariantReader &myval, BoolParam &param)
{
    if (myval.mType == JsonDataType::Boolean)
    {
        param.mValue = myval.mBooleanValue;
        return Result::Success();
    }
    if (myval.mType == JsonDataType::Number)
    {
        param.mValue = (myval.mNumericValue.Get<int>() == 1);
    }
    return Result::Failure("expected boolean");
}

// serializing floats is fun. on one hand, using the decimal representation is terrible,
// on the other hand i want it to be human-readable.
struct StringParam // : ParameterBase
{
    using T = String;

    T mValue;

    T GetValue() const
    {
        return mValue;
    }
    void SetValue(T v)
    {
        mValue = v;
    }

    explicit StringParam(const T &initialValue) : mValue(initialValue)
    {
    }

    StringParam() = default;
    StringParam(const StringParam &rhs) = default;
    StringParam &operator=(const StringParam &rhs) = default;

    // Result SerializableObject_ToJSON(JsonVariant rhs) const
    // {
    //     return {rhs.set(mValue)};
    // }

    // Result SerializableObject_Deserialize(JsonVariant obj)
    // {
    //     if (!obj.is<String>())
    //         return Result::Failure("expected string");
    //     mValue = obj.as<String>();
    //     return Result::Success();
    // }
};

Result Serialize(JsonVariantWriter &myval, StringParam &param)
{
    return myval.WriteStringValue(param.mValue);
}

Result Deserialize(JsonVariantReader &myval, StringParam &param)
{
    if (myval.mType != JsonDataType::String)
    {
        return Result::Failure("expected string");
    }
    param.mValue = myval.mStringValue;
    return Result::Success();
}

// use enum values by NAME, so they can be human-readable.
template <typename TEnum>
struct EnumParam // : ParameterBase
{
    EnumInfo<TEnum> *mpEnumInfo;
    TEnum mValue;

    TEnum GetValue() const
    {
        return mValue;
    }
    void SetValue(TEnum v)
    {
        mValue = v;
    }

    explicit EnumParam(EnumInfo<TEnum> &info, TEnum initialValue)
        : mpEnumInfo(&info), //
          mValue(initialValue)
    {
    }
    EnumParam(const EnumParam<TEnum> &rhs) = default;
    EnumParam<TEnum> &operator=(const EnumParam<TEnum> &rhs) = default;

    // Result SerializableObject_ToJSON(JsonVariant rhs) const
    // {
    //     const char *s = mpEnumInfo->GetValueShortName(mValue);
    //     Serial.println(String("serializing enum value: ") + (int)mValue + " -> " + PointerToString(s) + " <" +
    //     mpEnumInfo->mTypeName +  "> = " + s); return {rhs.set(s)};
    // }

    // Result SerializableObject_Deserialize(JsonVariant obj)
    // {
    //     if (!obj.is<String>())
    //         return Result::Failure("expected string(enum)");
    //     String shortName = obj.as<String>();
    //     return mpEnumInfo->ValueForShortName(shortName, mValue);
    // }
};

template <typename TEnum>
Result Serialize(JsonVariantWriter &myval, EnumParam<TEnum> &param)
{
    const char *s = param.mpEnumInfo->GetValueShortName(param.mValue);
    return myval.WriteStringValue(s);
}

template <typename TEnum>
Result Deserialize(JsonVariantReader &myval, EnumParam<TEnum> &param)
{
    if (myval.mType != JsonDataType::String)
        return Result::Failure("expected string");
    return param.mpEnumInfo->ValueForShortName(myval.mStringValue, param.mValue);
}

// use enum values by NAME, so they can be human-readable.
struct ScaleParam //: ParameterBase
{
    Scale mValue;

    explicit ScaleParam(const Scale &initialValue) : mValue(initialValue)
    {
    }

    // Result SerializableObject_ToJSON(JsonVariant rhs) const
    // {
    //     return {rhs.set(mValue.ToSerializableString())};
    // }

    // Result SerializableObject_Deserialize(JsonVariant obj)
    // {
    //     if (!obj.is<String>())
    //     {
    //         return Result::Failure("ScaleParam must be string");
    //     }
    //     String t = obj.as<String>();
    //     return mValue.DeserializeFromString(t);
    // }
};

Result Serialize(JsonVariantWriter &myval, ScaleParam &param)
{
    return myval.WriteStringValue(param.mValue.ToSerializableString());
}

Result Deserialize(JsonVariantReader &myval, ScaleParam &param)
{
    if (myval.mType != JsonDataType::String)
        return Result::Failure("expected string");
    return param.mValue.DeserializeFromString(myval.mStringValue);
}

// "volume" parameters use a custom scale, so they feel more usable.
// linear (0% to 100%) feels too extreme; massive uses a square curve, and a -inf db to +1 db range.
struct VolumeParamValue
{
  private:
    static constexpr float gMaxVolumeLinearGain = 1.122f; // +1 db.
    float mParamValue = 0.0f;

    inline static float ParamToLinear(float x)
    {
        if (x <= 0)
            return 0;
        return x * x * gMaxVolumeLinearGain;
    }
    inline static float LinearToParam(float x)
    { // expensive
        return sqrtf(x / gMaxVolumeLinearGain);
    }

    inline static float ParamToDecibels(float x)
    {
        return LinearToDecibels(ParamToLinear(x));
    }
    inline static float DecibelsToParam(float db)
    { // pretty expensive
        return LinearToParam(DecibelsToLinear(db));
    }

  public:
    float ToLinearGain() const
    {
        return ParamToLinear(mParamValue);
    }
    float ToDecibels() const // expensive (ish)
    {
        return ParamToDecibels(mParamValue);
    }

    float IsSilent() const
    {
        return IsSilentGain(ToLinearGain());
    }
    float GetParamValue() const
    {
        return mParamValue;
    }
    void SetValue(float f)
    {
        mParamValue = f;
    }
    void SetLinearValue(float f)
    { // expensive
        mParamValue = LinearToParam(f);
    }
    void SetDecibels(float db)
    { // expensive
        mParamValue = DecibelsToParam(db);
    }

    // useful for adding a modulation signal to a volume.
    VolumeParamValue AddParam(float rhs) const
    {
        return FromParamValue(mParamValue + rhs);
    }

    static VolumeParamValue FromLinear(float f)
    {
        VolumeParamValue ret;
        ret.SetLinearValue(f);
        return ret;
    }
    static VolumeParamValue FromParamValue(float f)
    {
        VolumeParamValue ret;
        ret.mParamValue = f;
        return ret;
    }
    static VolumeParamValue FromDecibels(float f)
    {
        VolumeParamValue ret;
        ret.SetDecibels(f);
        return ret;
    }
};

// Result SerializableObject_ToJSON(JsonVariant obj) const
// {
//     return {obj.set(mParamValue)};
// }

// Result SerializableObject_Deserialize(JsonVariant obj)
// {
//     if (!obj.is<float>())
//     {
//         return Result::Failure("expected float");
//     }
//     mParamValue = obj.as<float>();
//     return Result::Success();
// }

Result Serialize(JsonVariantWriter &myval, VolumeParamValue &param)
{
    return myval.WriteNumberValue(param.GetParamValue());
}

Result Deserialize(JsonVariantReader &myval, VolumeParamValue &param)
{
    if (myval.mType != JsonDataType::Number)
        return Result::Failure("expected number");
    param.SetValue(myval.mNumericValue.Get<float>());
    return Result::Success();
}

// value 0.3 = unity, and each 0.1 param value = 1 octave transposition, when KT = 1.
// when KT = 0, 0.5 = 1khz, and each 0.1 param value = +/- octave.
struct FrequencyParamValue // : SerializableDictionary
{
    FloatParam mValue;
    FloatParam mKTValue;

    FrequencyParamValue(float initialValue, float initialKTValue)
        : mValue(initialValue), //
          mKTValue(initialKTValue)
    {
    }

    float GetParamValue() const
    {
        return mValue.GetValue();
    }
    float GetKTParamValue() const
    {
        return mKTValue.GetValue();
    }

    void SetParamValue(float v)
    {
        mValue.SetValue(v);
    }
    void SetKTParamValue(float kt)
    {
        mKTValue.SetValue(kt);
    }

    float GetFrequency(float noteHz, float paramModulation) const
    {
        float param = mValue.GetValue() + paramModulation; // apply current modulation value.
        // at 0.5, we use 1khz.
        // for each 0.1 param value, it's +/- one octave

        float centerFreq = 1000; // the cutoff frequency at 0.5 param value.

        // with no KT,
        // so if param is 0.8, we want to multiply by 8 (2^3)
        // if param is 0.3, multiply by 1/4 (2^(1/4))

        // with full KT,
        // at 0.3, we use playFrequency.
        // for each 0.1 param value, it's +/- one octave.
        float ktFreq = noteHz * 4; // to copy massive, 1:1 is at paramvalue 0.3. 0.5 is 2 octaves above playing freq.
        centerFreq = Lerp(centerFreq, ktFreq, mKTValue.GetValue());

        param -= 0.5f;  // signed distance from 0.5 -.2 (0.3 = -.2, 0.8 = .3)
        param *= 10.0f; // (.3 = -2, .8 = 3)
        float fact = fast::pow(2, param);
        return Clamp(centerFreq * fact, 0.0f, 22050.0f);
    }

    // param modulation is normal krate param mod
    // noteModulation includes osc.mPitchFine + osc.mPitchSemis + detune;
    float GetMidiNote(float playingMidiNote, float paramModulation) const
    {
        constexpr float oneKhzMidiNote =
            83.213094853f;                   // 1000hz, in midi notes. this replicates behavior of filter modulation.
        float ktNote = playingMidiNote + 24; // center represents playing note + 2 octaves.

        float centerNote = Lerp(oneKhzMidiNote, ktNote, mKTValue.GetValue());

        float param = mValue.GetValue() + paramModulation;

        param = (param - 0.5f) * 10; // rescale from 0-1 to -5 to +5 (octaves)
        float paramSemis =
            centerNote + param * 12; // each 1 param = 1 octave. because we're in semis land, it's just a mul.
        return paramSemis;
    }
};

// Result SerializableObject_ToJSON(JsonVariant rhs) const
// {
//     Result ret = Result::Success();
//     ret.AndRequires(mValue.SerializableObject_ToJSON(rhs.createNestedObject("val")), "val");
//     ret.AndRequires(mKTValue.SerializableObject_ToJSON(rhs.createNestedObject("kt")), "kt");
//     return ret;
// }

// Result SerializableObject_Deserialize(JsonVariant obj)
// {
//     if (!obj.is<JsonObject>())
//     {
//         return Result::Failure("must be object");
//     }

//     Result ret = Result::Success();
//     ret.AndRequires(mValue.SerializableObject_Deserialize(obj["val"]), "val");
//     ret.AndRequires(mKTValue.SerializableObject_Deserialize(obj["kt"]), "kt");
//     return ret;
// }

Result Serialize(JsonVariantWriter &myval, FrequencyParamValue &param)
{
    float paramVal = param.GetParamValue();
    float ktVal = param.GetKTParamValue();
    SerializationObjectMap<2> map{{
        CreateSerializationMapping(paramVal, "val"),
        CreateSerializationMapping(ktVal, "kt"),
    }};
    return Serialize(myval, map);
}

// todo: there needs to be a way to utilize object key mapping for this.
Result Deserialize(JsonVariantReader &myval, FrequencyParamValue &param)
{
    float paramVal = 0;
    float ktVal = 0;
    SerializationObjectMap<2> map{{
        CreateSerializationMapping(paramVal, "val"),
        CreateSerializationMapping(ktVal, "kt"),
    }};
    auto ret = Deserialize(myval, map);
    if (ret.IsSuccess())
    {
        param.SetParamValue(paramVal);
        param.SetKTParamValue(ktVal);
    }
    return ret;
}

struct EnvTimeParamValue
{
  private:
    float mValue;

  public:
    explicit EnvTimeParamValue(float initialValue) : mValue(initialValue)
    {
    }

    float GetValue() const
    {
        return mValue;
    }

    void SetValue(float v)
    {
        mValue = v;
    }

    static constexpr float gCenterValue = 375; // the MS at 0.5 param value.
    static constexpr int gRangeLog2 = 10;      // must be even for below calculations to work
    static constexpr float gMinRawVal =
        (1.0f / (1 << (gRangeLog2 / 2))) *
        gCenterValue; // minimum value as described by log calc (before subtracting min).
    static constexpr float gMaxRawVal = (1 << (gRangeLog2 / 2)) * gCenterValue;
    static constexpr float gMinRealVal = 0;
    static constexpr float gMaxRealVal = gMaxRawVal - gMinRawVal;

    float GetMilliseconds(float paramModulation) const
    {
        float param = mValue + paramModulation; // apply current modulation value.
        param = Clamp(param, 0, 1);
        param -= 0.5f;       // -.5 to .5
        param *= gRangeLog2; // -5 to +5 (2^-5 = .0312; 2^5 = 32), with 375ms center val means [12ms, 12sec]
        float fact = fast::pow(2, param);
        param = gCenterValue * fact;
        param -= gMinRawVal; // pow(2,x) doesn't ever reach 0 value. subtracting the min allows 0 to exist.
        return Clamp(param, gMinRealVal, gMaxRealVal);
    }

    // Result SerializableObject_ToJSON(JsonVariant rhs) const
    // {
    //     return {rhs.set(mValue)};
    // }

    // Result SerializableObject_Deserialize(JsonVariant obj)
    // {
    //     if (!obj.is<float>())
    //         return Result::Failure("expected float");
    //     mValue = obj.as<float>();
    //     return Result::Success();
    // }
};

Result Serialize(JsonVariantWriter &myval, EnvTimeParamValue &param)
{
    return myval.WriteNumberValue(param.GetValue());
}

Result Deserialize(JsonVariantReader &myval, EnvTimeParamValue &param)
{
    if (myval.mType != JsonDataType::Number)
        return Result::Failure("expected number");
    param.SetValue(myval.mNumericValue.Get<float>());
    return Result::Success();
}

struct CurveLUTParamValue
{
  private:
    // real LUT indices are int16_t, so that would be more accurate, but because they're all
    // modulated by floats, use float instead.
    float mValueN11;

  public:
    explicit CurveLUTParamValue(float initialValueN11) : mValueN11(initialValueN11)
    {
    }

    float GetParamValue() const
    {
        return mValueN11;
    }

    void SetParamValue(float v)
    {
        mValueN11 = v;
    }

    const q15_t *BeginLookup() const
    {
        return gModCurveLUT.BeginLookupF(Clamp(mValueN11, 0, 1));
    }
    const q15_t *BeginLookup(float modValue) const
    {
        float p = mValueN11 + modValue;
        p = Clamp(p, 0, 1);
        return gModCurveLUT.BeginLookupF(p);
    }

    // Result SerializableObject_ToJSON(JsonVariant rhs) const
    // {
    //     return {rhs.set(mValueN11)};
    // }

    // Result SerializableObject_Deserialize(JsonVariant obj)
    // {
    //     if (!obj.is<float>())
    //         return Result::Failure("expected float");
    //     mValueN11 = obj.as<float>();
    //     return Result::Success();
    // }
};

Result Serialize(JsonVariantWriter &myval, CurveLUTParamValue &param)
{
    return myval.WriteNumberValue(param.GetParamValue());
}

Result Deserialize(JsonVariantReader &myval, CurveLUTParamValue &param)
{
    if (myval.mType != JsonDataType::Number)
        return Result::Failure("expected number");
    param.SetParamValue(myval.mNumericValue.Get<float>());
    return Result::Success();
}

enum class TimeBasis : uint8_t
{
    Milliseconds,
    Hertz,
    Half,
    Quarter,
    Eighth,
    Sixteenth,
    ThirtySecond,
    DottedHalf,
    DottedQuarter,
    DottedEighth,
    DottedSixteenth,
    TripletHalf,
    TripletQuarter,
    TripletEighth,
    TripletSixteenth,
};

EnumItemInfo<TimeBasis> gTimeBasisItems[15] = {
    {TimeBasis::Milliseconds, "Milliseconds", "ms"},
    {TimeBasis::Hertz, "Hertz", "hz"},
    //
    {TimeBasis::Half, "Half", "2"},
    {TimeBasis::Quarter, "Quarter", "4"},
    {TimeBasis::Eighth, "Eighth", "8th"},
    {TimeBasis::Sixteenth, "Sixteenth", "16th"},
    {TimeBasis::ThirtySecond, "ThirtySecond", "32nd"},
    //
    {TimeBasis::DottedHalf, "DottedHalf", "2d"},
    {TimeBasis::DottedQuarter, "DottedQuarter", "4d"},
    {TimeBasis::DottedEighth, "DottedEighth", "8d"},
    {TimeBasis::DottedSixteenth, "DottedSixteenth", "16d"},
    //
    {TimeBasis::TripletHalf, "TripletHalf", "2-3"},
    {TimeBasis::TripletQuarter, "TripletQuarter", "4-3"},
    {TimeBasis::TripletEighth, "TripletEighth", "8-3"},
    {TimeBasis::TripletSixteenth, "TripletSixteenth", "16-3"},
};

EnumInfo<TimeBasis> gTimeBasisInfo("TimeBasis", gTimeBasisItems);

// bool SerializeHarmVoiceNoteSequence(const std::array<int8_t, HARM_SEQUENCE_LEN> &sequence,
//                                     const uint8_t &sequenceLength,
//                                     JsonVariant rhs)
// {
//     rhs.to<JsonArray>();
//     bool ret = true;
//     for (size_t i = 0; i < sequenceLength; ++i)
//     {
//         ret = ret && rhs.add(sequence[i]);
//         if (!ret)
//         {
//             break;
//         }
//     }
//     return ret;
// }

// Result DeserializeHarmVoiceNoteSequence(JsonVariant obj,
//                                         std::array<int8_t, HARM_SEQUENCE_LEN> &mSequence,
//                                         uint8_t &mSequenceLength)
// {
//     if (!obj.is<JsonArray>())
//     {
//         return Result::Failure("expected array");
//     }
//     JsonArray arr = obj.as<JsonArray>();
//     Result ret = Result::Success();
//     // sanity
//     if (arr.size() > HARM_SEQUENCE_LEN)
//     {
//         ret.AddWarning(String("seq truncated:") + arr.size() + ">" + HARM_SEQUENCE_LEN);
//     }
//     mSequenceLength = std::min(HARM_SEQUENCE_LEN, arr.size());
//     for (size_t i = 0; i < mSequenceLength; ++i)
//     {
//         if (!arr[i].is<int>())
//         {
//             return Result::Failure("expected int");
//         }
//         mSequence[i] = (uint8_t)arr[i].as<int>();
//     }
//     return ret;
// // }

// bool SerializeMidiNoteRange(const MidiNote &minValue, const MidiNote &maxValue, JsonVariant rhs)
// {
//     rhs.to<JsonArray>();
//     bool ret = true;
//     ret = ret && rhs.add(minValue.ToString());
//     ret = ret && rhs.add(maxValue.ToString());
//     return ret;
// }
// Result DeserializeMidiNoteRange(JsonVariant parent, MidiNote &minValue, MidiNote &maxValue)
// {
//     std::array<StringParam, 2> elements;
//     Result ret = DeserializeArray(parent, elements);
//     if (ret.IsFailure())
//         return ret.AddWarning("noterange:array");
//     ret.AndRequires(MidiNote::FromString(elements[0].GetValue(), minValue), "noterange:min");
//     ret.AndRequires(MidiNote::FromString(elements[1].GetValue(), maxValue), "noterange:max");
//     return ret;
// }

// struct MidiNoteRangeSerializer // : SerializableObject
// {
//     MidiNote &mMinValue;
//     MidiNote &mMaxValue;

//     MidiNoteRangeSerializer(MidiNote &minValue, MidiNote &maxValue)
//         : // SerializableObject(fieldName), //
//           mMinValue(minValue), mMaxValue(maxValue)
//     {
//     }
// };

// this is necessary to allow "rich" beat/frequency settings to beat-sync LFO for example
struct TimeWithBasisParam
{
    EnumParam<TimeBasis> mBasis{gTimeBasisInfo, TimeBasis::Milliseconds};
    FloatParam mParamValue{100};

    // Result SerializableObject_ToJSON(JsonVariant rhs) const
    // {
    //     Result ret = Result::Success();
    //     ret.AndRequires(mBasis.SerializableObject_ToJSON(rhs.createNestedObject("basis")), "basis");
    //     ret.AndRequires(mParamValue.SerializableObject_ToJSON(rhs.createNestedObject("val")), "val");
    //     return ret;
    // }

    // Result SerializableObject_Deserialize(JsonVariant obj)
    // {
    //     if (!obj.is<JsonObject>())
    //     {
    //         return Result::Failure("must be object");
    //     }

    //     Result ret = Result::Success();
    //     ret.AndRequires(mBasis.SerializableObject_Deserialize(obj["basis"]), "basis");
    //     ret.AndRequires(mParamValue.SerializableObject_Deserialize(obj["val"]), "val");
    //     return ret;
    // }

    TimeWithBasisParam(TimeBasis basis, float initialValue)
    {
        mBasis.SetValue(basis);
        mParamValue.SetValue(initialValue);
    }

    static TimeWithBasisParam FromMilliseconds(float timeMS)
    {
        TimeWithBasisParam ret{TimeBasis::Milliseconds, timeMS};
        return ret;
    }

    void SetFrequency(float hz)
    {
        mBasis.SetValue(TimeBasis::Hertz);
        mParamValue.SetValue(hz);
    }
    void SetMilliseconds(float timeMS)
    {
        mBasis.SetValue(TimeBasis::Milliseconds);
        mParamValue.SetValue(timeMS);
    }
    float ToHertz(float bpmIfNeeded) const
    {
        switch (mBasis.GetValue())
        {
        default:
        case TimeBasis::Milliseconds:
            return CycleMSToHertz(mParamValue.GetValue());
        case TimeBasis::Hertz:
            return mParamValue.GetValue();
        case TimeBasis::Half:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 2, 1)); // 2
        case TimeBasis::Quarter:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 1, 1)); // 1
        case TimeBasis::Eighth:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 1, 2)); // 0.5
        case TimeBasis::Sixteenth:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 1, 4));
        case TimeBasis::ThirtySecond:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 1, 8));
            //
        case TimeBasis::DottedHalf:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 3, 1)); // 3
        case TimeBasis::DottedQuarter:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 3, 2)); // 1.5
        case TimeBasis::DottedEighth:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 3, 4)); // 0.75
        case TimeBasis::DottedSixteenth:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 3, 8));
            //
        case TimeBasis::TripletHalf:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 4, 3)); // 1.33
        case TimeBasis::TripletQuarter:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 4, 6)); //
        case TimeBasis::TripletEighth:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 4, 12));
        case TimeBasis::TripletSixteenth:
            return CycleMSToHertz(NoteLengthToMS(bpmIfNeeded, 4, 24));
        }
    }
    float ToMS(float bpmIfNeeded) const
    {
        switch (mBasis.GetValue())
        {
        default:
        case TimeBasis::Milliseconds:
            return mParamValue.GetValue();
        case TimeBasis::Hertz:
            return HertzToCycleMS(mParamValue.GetValue());
        case TimeBasis::Half:
            return NoteLengthToMS(bpmIfNeeded, 2, 1); // 2
        case TimeBasis::Quarter:
            return NoteLengthToMS(bpmIfNeeded, 1, 1); // 1
        case TimeBasis::Eighth:
            return NoteLengthToMS(bpmIfNeeded, 1, 2); // 0.5
        case TimeBasis::Sixteenth:
            return NoteLengthToMS(bpmIfNeeded, 1, 4);
        case TimeBasis::ThirtySecond:
            return NoteLengthToMS(bpmIfNeeded, 1, 8);
            //
        case TimeBasis::DottedHalf:
            return NoteLengthToMS(bpmIfNeeded, 3, 1); // 3
        case TimeBasis::DottedQuarter:
            return NoteLengthToMS(bpmIfNeeded, 3, 2); // 1.5
        case TimeBasis::DottedEighth:
            return NoteLengthToMS(bpmIfNeeded, 3, 4); // 0.75
        case TimeBasis::DottedSixteenth:
            return NoteLengthToMS(bpmIfNeeded, 3, 8);
            //
        case TimeBasis::TripletHalf:
            return NoteLengthToMS(bpmIfNeeded, 4, 3); // 1.33
        case TimeBasis::TripletQuarter:
            return NoteLengthToMS(bpmIfNeeded, 4, 6); //
        case TimeBasis::TripletEighth:
            return NoteLengthToMS(bpmIfNeeded, 4, 12);
        case TimeBasis::TripletSixteenth:
            return NoteLengthToMS(bpmIfNeeded, 4, 24);
        }
    }
};

Result Serialize(JsonVariantWriter &myval, TimeWithBasisParam &param)
{
    SerializationObjectMap<2> map{{
        CreateSerializationMapping(param.mBasis, "u"),
        CreateSerializationMapping(param.mParamValue, "val"),
    }};
    return Serialize(myval, map);
}

// todo: there needs to be a way to utilize object key mapping for this.
Result Deserialize(JsonVariantReader &myval, TimeWithBasisParam &param)
{
    SerializationObjectMap<2> map{{
        CreateSerializationMapping(param.mBasis, "u"),
        CreateSerializationMapping(param.mParamValue, "val"),
    }};
    return Deserialize(myval, map);
}

} // namespace clarinoid
