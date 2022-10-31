#pragma once

namespace clarinoid
{

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
struct FrequencyParamValue
{
  private:
    float mValue;
    float mKTValue;

  public:
    FrequencyParamValue(float initialValue, float initialKTValue) : mValue(initialValue), mKTValue(initialKTValue)
    {
    }

    float GetParamValue() const
    {
        return mValue;
    }
    float GetKTParamValue() const
    {
        return mKTValue;
    }

    void SetParamValue(float v)
    {
        mValue = v;
    }
    void SetKTParamValue(float kt)
    {
        mKTValue = kt;
    }

    float GetFrequency(float noteHz, float paramModulation) const
    {
        float param = mValue + paramModulation; // apply current modulation value.
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
        centerFreq = Lerp(centerFreq, ktFreq, mKTValue);

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

        float centerNote = Lerp(oneKhzMidiNote, ktNote, mKTValue);

        float param = mValue + paramModulation;

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

} // namespace clarinoid
