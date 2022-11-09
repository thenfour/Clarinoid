#pragma once

namespace clarinoid
{

struct ParameterBase : SerializableObject
{
    ParameterBase(const char *fieldName) : SerializableObject(fieldName)
    {
    }
};

template <typename T>
struct IntParam : ParameterBase
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

    IntParam(const char *fieldName, T defaultValue)
        : ParameterBase(fieldName), //
          mValue(defaultValue)
    {
    }

    virtual bool SerializableObject_ToJSON(JsonVariant rhs) override
    {
        return rhs.set(mValue);
    }
    virtual bool SerializableObject_FromJSON(JsonVariant rhs) override
    {
        mValue = rhs.as<T>();
        return true;
    }
};

// serializing floats is fun. on one hand, using the decimal representation is terrible,
// on the other hand i want it to be human-readable.
struct FloatParam : ParameterBase
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

    FloatParam(const char *fieldName, float defaultValue)
        : ParameterBase(fieldName), //
          mValue(defaultValue)
    {
    }

    virtual bool SerializableObject_ToJSON(JsonVariant rhs) override
    {
        rhs.set(mValue);
        return true;
    }
    virtual bool SerializableObject_FromJSON(JsonVariant rhs) override
    {
        mValue = rhs.as<T>();
        return true;
    }
};

// serializing floats is fun. on one hand, using the decimal representation is terrible,
// on the other hand i want it to be human-readable.
struct BoolParam : ParameterBase
{
    using T = bool;
    T mValue; // making public, in case you need a reference. (see GuiPerformanceApp.hpp)

    T GetValue() const
    {
        return mValue;
    }
    void SetValue(T v)
    {
        mValue = v;
    }

    BoolParam(const char *fieldName, bool initialValue)
        : ParameterBase(fieldName), //
          mValue(initialValue)
    {
    }

    virtual bool SerializableObject_ToJSON(JsonVariant rhs) override
    {
        rhs.set(mValue ? 1 : 0);
        return true;
    }
    virtual bool SerializableObject_FromJSON(JsonVariant rhs) override
    {
        mValue = rhs.as<int>() == 1;
        return true;
    }
};

// serializing floats is fun. on one hand, using the decimal representation is terrible,
// on the other hand i want it to be human-readable.
struct StringParam : ParameterBase
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

    StringParam(const char *fieldName, const T &initialValue)
        : ParameterBase(fieldName), //
          mValue(initialValue)
    {
    }

    virtual bool SerializableObject_ToJSON(JsonVariant rhs) override
    {
        rhs.set(mValue);
        return true;
    }
    virtual bool SerializableObject_FromJSON(JsonVariant rhs) override
    {
        mValue = rhs.as<T>();
        return true;
    }
};

// use enum values by NAME, so they can be human-readable.
template <typename TEnum>
struct EnumParam : ParameterBase
{
    EnumInfo<TEnum> &mEnumInfo;
    TEnum mValue;

    TEnum GetValue() const
    {
        return mValue;
    }
    void SetValue(TEnum v)
    {
        mValue = v;
    }

    EnumParam(const char *fieldName, EnumInfo<TEnum> &info, TEnum initialValue)
        : ParameterBase(fieldName), //
          mEnumInfo(info),          //
          mValue(initialValue)
    {
    }

    virtual bool SerializableObject_ToJSON(JsonVariant rhs) override
    {
        rhs.set(mEnumInfo.GetValueString(mValue));
        return true;
    }
    virtual bool SerializableObject_FromJSON(JsonVariant rhs) override
    {
        // const char * valueName = rhs.as<const char *>();
        //  todo: convert value name to enum value.
        return true;
    }
};

// use enum values by NAME, so they can be human-readable.
struct ScaleParam : ParameterBase
{
    Scale mValue;

    ScaleParam(const char *fieldName, const Scale &initialValue)
        : ParameterBase(fieldName), //
          mValue(initialValue)
    {
    }

    virtual bool SerializableObject_ToJSON(JsonVariant rhs) override
    {
        return rhs.set(mValue.ToSerializableString());
    }
    virtual bool SerializableObject_FromJSON(JsonVariant rhs) override
    {
        return mValue.DeserializeFromString(rhs.as<String>());
    }
};

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

// value 0.3 = unity, and each 0.1 param value = 1 octave transposition, when KT = 1.
// when KT = 0, 0.5 = 1khz, and each 0.1 param value = +/- octave.
struct FrequencyParamValue : SerializableDictionary
{
    FloatParam mValue;   // = {"Value", 0.3f};
    FloatParam mKTValue; // = {"KT", 1.0f};

    SerializableObject *mSerializableChildObjects[2] = {
        &mValue,
        &mKTValue,
    };

    FrequencyParamValue(const char *fieldName, float initialValue, float initialKTValue)
        : SerializableDictionary(fieldName, mSerializableChildObjects), //
          mValue("Value", initialValue),                                //
          mKTValue("KT", initialKTValue)
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

struct EnvTimeParamValue
{
  private:
    float mValue;

  public:
    EnvTimeParamValue(float initialValue) : mValue(initialValue)
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
};

struct CurveLUTParamValue
{
  private:
    // real LUT indices are int16_t, so that would be more accurate, but because they're all
    // modulated by floats, use float instead.
    float mValueN11;

  public:
    CurveLUTParamValue(float initialValueN11) : mValueN11(initialValueN11)
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

    q15_t *BeginLookup() const
    {
        return gModCurveLUT.BeginLookupF(Clamp(mValueN11, 0, 1));
    }
    q15_t *BeginLookup(float modValue) const
    {
        float p = mValueN11 + modValue;
        p = Clamp(p, 0, 1);
        return gModCurveLUT.BeginLookupF(p);
    }
};

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
    {TimeBasis::Milliseconds, "Milliseconds"},
    {TimeBasis::Hertz, "Hertz"},
    //
    {TimeBasis::Half, "Half"},
    {TimeBasis::Quarter, "Quarter"},
    {TimeBasis::Eighth, "Eighth"},
    {TimeBasis::Sixteenth, "Sixteenth"},
    {TimeBasis::ThirtySecond, "ThirtySecond"},
    //
    {TimeBasis::DottedHalf, "DottedHalf"},
    {TimeBasis::DottedQuarter, "DottedQuarter"},
    {TimeBasis::DottedEighth, "DottedEighth"},
    {TimeBasis::DottedSixteenth, "DottedSixteenth"},
    //
    {TimeBasis::TripletHalf, "TripletHalf"},
    {TimeBasis::TripletQuarter, "TripletQuarter"},
    {TimeBasis::TripletEighth, "TripletEighth"},
    {TimeBasis::TripletSixteenth, "TripletSixteenth"},
};

EnumInfo<TimeBasis> gTimeBasisInfo("TimeBasis", gTimeBasisItems);

struct HarmVoiceNoteSequenceSerializer : SerializableObject
{
    std::array<int8_t, HARM_SEQUENCE_LEN> &mSequence;
    uint8_t &mSequenceLength;

    HarmVoiceNoteSequenceSerializer(const char *fieldName,
                                    std::array<int8_t, HARM_SEQUENCE_LEN> &sequence,
                                    uint8_t &sequenceLength)
        : SerializableObject(fieldName), //
          mSequence(sequence),           //
          mSequenceLength(sequenceLength)
    {
    }

    virtual bool SerializableObject_ToJSON(JsonVariant rhs) override
    {
        rhs.to<JsonArray>();
        for (size_t i = 0; i < mSequenceLength; ++ i) {
            rhs.add(mSequence[i]);
        }
        return true;
    }
    virtual bool SerializableObject_FromJSON(JsonVariant rhs) override
    {
        // todo
        return false;
    }
};

struct MidiNoteRangeSerializer : SerializableObject
{
    MidiNote &mMinValue;
    MidiNote &mMaxValue;

    MidiNoteRangeSerializer(const char *fieldName, MidiNote &minValue, MidiNote &maxValue)
        : SerializableObject(fieldName), //
          mMinValue(minValue), mMaxValue(maxValue)
    {
    }

    virtual bool SerializableObject_ToJSON(JsonVariant rhs) override
    {
        rhs.set(mMinValue.ToString() + "," + mMaxValue.ToString());
        return true;
    }
    virtual bool SerializableObject_FromJSON(JsonVariant rhs) override
    {
        // todo

        // String s = rhs.as<String>();
        // s.indexOf(',');
        // mValue = rhs.as<T>();
        return false;
    }
};

// this is necessary to allow "rich" beat/frequency settings to beat-sync LFO for example
struct TimeWithBasisParam : SerializableDictionary
{
    EnumParam<TimeBasis> mBasis = {"Basis", gTimeBasisInfo, TimeBasis::Milliseconds};
    FloatParam mParamValue = {"Value", 100};
    // float mHz = 6;
    // float mTimeBeats = 1;

    SerializableObject *mSerializableChildObjects[2] = {
        &mBasis,
        &mParamValue,
    };

    TimeWithBasisParam(const char *fieldName, TimeBasis basis, float initialValue)
        : SerializableDictionary(fieldName, mSerializableChildObjects)
    {
        mBasis.SetValue(basis);
        mParamValue.SetValue(initialValue);
    }

    static TimeWithBasisParam FromMilliseconds(const char *fieldName, float timeMS)
    {
        TimeWithBasisParam ret{fieldName, TimeBasis::Milliseconds, timeMS};
        // ret.SetMilliseconds(timeMS);
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

} // namespace clarinoid
