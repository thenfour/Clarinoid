
#pragma once

#include "math.hpp"

namespace clarinoid
{
// for adjusting numeric values with the settings menu editor...
template <typename T>
struct NumericEditRangeSpec
{
    static constexpr int DefaultCourseSteps = 12;
    static constexpr int DefaultNormalSteps = 24;
    static constexpr int DefaultFineSteps = 500;

    T mRangeMin;
    T mRangeMax;

    T mCourseStep;
    T mNormalStep;
    T mFineStep;

    explicit NumericEditRangeSpec(T rangeMin, T rangeMax) : mRangeMin(rangeMin), mRangeMax(rangeMax)
    {
        mCourseStep = (T)(float(rangeMax - rangeMin) / DefaultCourseSteps);
        mNormalStep = (T)(float(rangeMax - rangeMin) / DefaultNormalSteps);
        mFineStep = (T)(float(rangeMax - rangeMin) / DefaultFineSteps);
        if (mCourseStep == 0)
        {
            if (std::is_integral<T>::value)
                mCourseStep = 1;
            else
                mCourseStep = (T)1 / 100;
        }
        if (mNormalStep == 0)
        {
            if (std::is_integral<T>::value)
                mNormalStep = 1;
            else
                mNormalStep = (T)1 / 100;
        }
        if (mFineStep == 0)
        {
            if (std::is_integral<T>::value)
                mFineStep = 1;
            else
                mFineStep = (T)1 / 100;
        }
    }

    explicit NumericEditRangeSpec(T rangeMin, T rangeMax, T courseStep, T normalStep, T fineStep)
        : mRangeMin(rangeMin), mRangeMax(rangeMax), mCourseStep(courseStep), mNormalStep(normalStep),
          mFineStep(fineStep)
    {
    }

    template <typename Trhs>
    NumericEditRangeSpec<Trhs> Cast() const
    {
        return NumericEditRangeSpec<Trhs>(static_cast<Trhs>(mRangeMin),
                                          static_cast<Trhs>(mRangeMax),
                                          static_cast<Trhs>(mCourseStep),
                                          static_cast<Trhs>(mNormalStep),
                                          static_cast<Trhs>(mFineStep));
    }

    virtual std::pair<bool, T> AdjustValue(T oldVal,
                                           int encoderIntDelta,
                                           bool isCoursePressed,
                                           bool isFinePressed) const
    {
        T step = mNormalStep;
        if (isCoursePressed && !isFinePressed)
            step = mCourseStep;
        if (isFinePressed && !isCoursePressed)
            step = mFineStep;

        T ret = oldVal + (step * encoderIntDelta);

        ret = (T)Clamp((float)ret, (float)mRangeMin, (float)mRangeMax);

        return std::make_pair(!NumberEquals(oldVal, ret), ret);
    }

    virtual float remap(T val, float newMin, float newMax) const
    {
        float v01 = RemapTo01((float)val, (float)mRangeMin, (float)mRangeMax);
        return Remap01ToRange(v01, newMin, newMax);
    }
};

// this is a weird funciton but it's convenient for decibel settings.
// it allows that below the minimum, there's an extra value of -inf db.
struct NumericEditRangeSpecWithBottom : NumericEditRangeSpec<float>
{
    static constexpr float BOTTOM_VALUE = MIN_DECIBEL_GAIN;

    explicit NumericEditRangeSpecWithBottom(float rangeMin, float rangeMax) : NumericEditRangeSpec(rangeMin, rangeMax)
    {
        CCASSERT(rangeMin > BOTTOM_VALUE);
    }

    explicit NumericEditRangeSpecWithBottom(float rangeMin,
                                            float rangeMax,
                                            float courseStep,
                                            float normalStep,
                                            float fineStep)
        : NumericEditRangeSpec(rangeMin, rangeMax, courseStep, normalStep, fineStep)
    {
        CCASSERT(rangeMin > BOTTOM_VALUE);
    }

    using T = float;

    bool IsBottom(T val) const
    {
        return val <= BOTTOM_VALUE;
    }

    virtual std::pair<bool, T> AdjustValue(T oldVal,
                                           int encoderIntDelta,
                                           bool isCoursePressed,
                                           bool isFinePressed) const override
    {
        T step = mNormalStep;
        if (isCoursePressed && !isFinePressed)
            step = mCourseStep;
        if (isFinePressed && !isCoursePressed)
            step = mFineStep;

        T ret = oldVal;

        if (encoderIntDelta > 0 && FloatLessThanOrEquals(oldVal, BOTTOM_VALUE))
        {
            // handle the first encoder detent jumping from 0 to the range min.
            ret = mRangeMin;
            encoderIntDelta--;
        }

        ret += (step * encoderIntDelta);

        // if you're within finestep/2 of a value rounded to 0.25, then round it.
        float quantizedValue = floorf((ret + 0.125f) * 4) / 4;
        if (fabsf(quantizedValue - ret) < (mFineStep / 2.0f))
        {
            ret = quantizedValue;
        }

        // transition from >min to =min, if you're within a finestep away.
        if (ret > mRangeMin && ret <= (mRangeMin + (mFineStep / 2)))
        {
            ret = mRangeMin;
        }

        if (ret < mRangeMin)
        {
            ret = BOTTOM_VALUE - 1; // minus one to allow for imprecision during round trips
        }
        else if (ret > mRangeMax)
        {
            ret = mRangeMax;
        }

        return std::make_pair(!NumberEquals(ret, oldVal), ret);
    }
};

namespace StandardRangeSpecs
{
static const NumericEditRangeSpec<float> gFloat_0_1 =
    NumericEditRangeSpec<float>{0.0f, 1.0f, 0.1f /*course*/, 0.025f /*normal*/, 0.001f /*fine*/};
static const NumericEditRangeSpec<float> gFloat_0_1_Fine =
    NumericEditRangeSpec<float>{0.0f, 1.0f, 0.1f /*course*/, 0.025f /*normal*/, 0.001f /*fine*/};
static const NumericEditRangeSpec<float> gFloat_N1_1 = NumericEditRangeSpec<float>{-1.0f, 1.0f, 0.1f /*course*/, 0.025f /*normal*/, 0.001f /*fine*/};

static const NumericEditRangeSpec<float> gBreathMin =
    NumericEditRangeSpec<float>{0.0f, 0.3f, 0.01f /*course*/, 0.005f /*normal*/, 0.001f /*fine*/};

// used by detune
static const NumericEditRangeSpec<float> gFloat_0_2 = NumericEditRangeSpec<float>{0.0f, 2.0f};

static const NumericEditRangeSpecWithBottom gMasterGainDb =
    NumericEditRangeSpecWithBottom{-30.0f, 12.0f, 3.0f /*course*/, 1.0f /*normal*/, 0.25f /*fine*/};

static const NumericEditRangeSpecWithBottom gGeneralGain =
    NumericEditRangeSpecWithBottom{-30.0f, 12.0f, 3.0f /*course*/, 1.0f /*normal*/, 0.25f /*fine*/};
// static const NumericEditRangeSpecWithBottom gSendGain =
//     NumericEditRangeSpecWithBottom{-30.0f, 0.0f, 3.0f /*course*/, 1.0f /*normal*/, 0.25f /*fine*/};

static const NumericEditRangeSpec<int> gPortamentoRange = NumericEditRangeSpec<int>{0, 1000};

// osc pitch and global transpose
static const NumericEditRangeSpec<int> gTransposeRange = NumericEditRangeSpec<int>{-48, 48, 6, 1, 1};
static const NumericEditRangeSpec<int> gCurveIndexRange = NumericEditRangeSpec<int>{0, gModCurveLUT.LutSizeY - 1};

static const NumericEditRangeSpec<float> gDelayStereoSpread = NumericEditRangeSpec<float>{0.0f, 100.0f};
static const NumericEditRangeSpec<float> gFilterFreqRange = NumericEditRangeSpec<float>{0.0f, 20000.0f};

static const NumericEditRangeSpec<float> gFreqMulRange = NumericEditRangeSpec<float>{0, 24, 1.0f, 0.05f, 0.002f};
static const NumericEditRangeSpec<float> gFreqOffsetRange = NumericEditRangeSpec<float>{-5000, 5000, 500, 10, 0.2f};

static const NumericEditRangeSpec<float> gOverallFMStrengthRange = NumericEditRangeSpec<float>{0.0f, 1.0f, 0.1f, 0.01f, 0.001f};

static const NumericEditRangeSpec<float> gLFOFrequency = NumericEditRangeSpec<float>{0.01f, 15.0f};

static const NumericEditRangeSpec<float> gEnvSustainLevel = gFloat_0_1;

static const NumericEditRangeSpec<float> gBPMRange = NumericEditRangeSpec<float>{20, 300, 10, 2, 1};
static const NumericEditRangeSpec<int> gMetronomeNoteRange = NumericEditRangeSpec<int>{20, 120, 10, 1, 1};
static const NumericEditRangeSpec<int> gMetronomeDecayRange = NumericEditRangeSpec<int>{1, 200, 10, 1, 1};
} // namespace StandardRangeSpecs

} // namespace clarinoid
