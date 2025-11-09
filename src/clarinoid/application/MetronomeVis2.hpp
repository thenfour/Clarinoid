#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/basic/ModCurve.hpp>
#include <clarinoid/basic/Envelope.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include "DisplayDefs.hpp"

namespace clarinoid
{

struct MetronomeVis2
{
    struct Division
    {
        FlashEnvelope mFlash;

        bool mFirstInMeasure = false;
        bool mIsMajor = false;
        uint8_t mBrightnessQp8 = 0;
    };

    void Render(IMetronome &metronome, IDisplay &display, AppSettings &appSettings, PointF centerF, RectI flashRect)
    {
        constexpr float kRadianOffset = -kPI_f / 2.0f; // rotate everything so 0 is at top.
        const auto &perf = appSettings.GetCurrentPerformancePatch();
        int beatsPerBar = perf.mBeatsPerBar;
        int subdivisionsPerBeat = perf.mBeatSubdivisions;
        const int totalSubdivisions = beatsPerBar * subdivisionsPerBeat;

        if (SettingsChangedTrigger)
        {
            SettingsChangedTrigger = false;

            Reset(perf);
        }

        // initial calculations & setting fetch
        float totalBeatFloat = metronome.GetBeatFloat();
        uint32_t beatSerial = (uint32_t)floorf(totalBeatFloat);
        float subdivisionFloat = totalBeatFloat * (float)subdivisionsPerBeat;
        int subdivisionSerial = (int)floorf(subdivisionFloat);

        // setup & state change triggers
        if (beatSerial != mLastBeatSerial)
        {
            mLastBeatSerial = beatSerial;
            mBigFlash.Trigger();
        }

        if (subdivisionSerial != mLastSubdivisionSerial)
        {
            mLastSubdivisionSerial = subdivisionSerial;
            int markerIndex = RotateIntoRange(subdivisionSerial, totalSubdivisions);
            mDivisions[markerIndex].mFlash.Trigger();
        }

        float measureProgress01 = Frac(totalBeatFloat / beatsPerBar);
        float radarPosRadians = kTwoPI_f * measureProgress01;

        // draw sweep (brightness from 0 to 255 at the main line)
        {
            float sweepStart = radarPosRadians + kRadianOffset - kSweepSizeRadians;

            // use inner radius of major beat because we assume it's the tightest.
            display.FillDonutSliceGradient(centerF,
                                           kBigFlashRadius,
                                           kMajorInnerRadius,
                                           sweepStart,
                                           kSweepSizeRadians,
                                           0,
                                           kSweepBrightnessQp8,
                                           mpModCurveSweep);
        }

        // draw main animated line
        {
            auto lineStart = PolarToCartesian(centerF, kBigFlashRadius, radarPosRadians + kRadianOffset);
            auto lineEnd = PolarToCartesian(centerF, kMajorInnerRadius, radarPosRadians + kRadianOffset);
            display.DrawLine(lineStart.Round(), lineEnd.Round());
        }

        // draw divisions.
        for (int divisionIndex = 0; divisionIndex < totalSubdivisions; ++divisionIndex)
        {
            Division &division = mDivisions[divisionIndex];

            float mBeginAngleRadians = mAnglePerDivision * (float)divisionIndex;
            ModCurveRow mpModCurve = division.mIsMajor ? mpModCurveMajorFlash : mpModCurveMinorFlash;

            bool isThis = subdivisionSerial == divisionIndex;
            int brightnessQp8 = isThis ? 255 : 0;

            // int brightnessQp8 = 0;
            // {
            //     float envVal = division.mFlash.Sample01();
            //     float brightness01 = gModCurveLUT.Transfer32(envVal, mpModCurve);
            //     brightnessQp8 = (int)(brightness01 * division.mBrightnessQp8 + 0.5f);
            // }

            // brightness is 0 when we are not currently in the division;
            // 255 when we are in the beat.

            float innerRadius = division.mIsMajor ? kMajorInnerRadius : kMinorInnerRadius;
            display.FillDonutSliceGradient(centerF,
                                           innerRadius,
                                           kOuterRadius,
                                           mBeginAngleRadians + kRadianOffset,
                                           mAnglePerDivision,
                                           brightnessQp8,
                                           brightnessQp8,
                                           nullptr);
        }

        // big flash on beat
        {
            float s = mBigFlash.Sample01();
            float flash01 = gModCurveLUT.Transfer32(s, mpModCurveBigFlash);
            int mainFlashBrightnessQp8 = flash01 * kBigFlashBrightnessQp8;
            display.FillRectWithBrightness(flashRect, mainFlashBrightnessQp8);
            // draw center circle (major beat flash)
            display.FillCircleWithBrightnessF(centerF, kBigFlashRadius, mainFlashBrightnessQp8);
        }

        // draw division bounds
    }

    static bool SettingsChangedTrigger;

  private:
    static constexpr int kMaxSubdivisionSlots = kMaxBeatsPerBar * kMaxSubdivisionsPerBeat;
    // static constexpr int kBaseSubdivisionBrightness = 0;

    FlashEnvelope mBigFlash;
    fixed_vector<Division, kMaxSubdivisionSlots> mDivisions;
    uint32_t mLastBeatSerial = 0;
    int mLastSubdivisionSerial = 0;

    ModCurveRow mpModCurveBigFlash = nullptr;
    ModCurveRow mpModCurveMajorFlash = nullptr;
    ModCurveRow mpModCurveMinorFlash = nullptr;
    ModCurveRow mpModCurveSweep = nullptr;
    float mAnglePerDivision = 0;

    // call when configuration or settings change
    void Reset(const PerformancePatch &perf)
    {
        mpModCurveBigFlash = gModCurveLUT.BeginLookupF(kBigFlashCurveN11);
        mpModCurveMajorFlash = gModCurveLUT.BeginLookupF(kMajorCurveN11);
        mpModCurveMinorFlash = gModCurveLUT.BeginLookupF(kMinorCurveN11);
        mpModCurveSweep = gModCurveLUT.BeginLookupF(kSweepCurveN11);

        mLastBeatSerial = 0;
        mLastSubdivisionSerial = 0;
        mBigFlash.Reset();
        mBigFlash.Configure(kBigFlashHoldMs, kBigFlashDecayMs);

        // ensure correct envelope count
        int beatsPerBar = perf.mBeatsPerBar;
        int subdivisionsPerBeat = perf.mBeatSubdivisions;
        const int totalSubdivisions = beatsPerBar * subdivisionsPerBeat;

        mDivisions.resize(totalSubdivisions);
        mAnglePerDivision = kTwoPI_f / (float)totalSubdivisions;

        for (int i = 0; i < totalSubdivisions; ++i)
        {
            Division &division = mDivisions[i];
            division.mIsMajor = false;
            division.mFirstInMeasure = false; // subdivision 0 of beat 0

            int divisionIndex = &division - mDivisions.data();
            if (divisionIndex % subdivisionsPerBeat == 0)
            {
                // major beat
                division.mIsMajor = true;
                if ((divisionIndex / subdivisionsPerBeat) % beatsPerBar == 0)
                {
                    division.mFirstInMeasure = true;
                }
            }

            if (division.mIsMajor)
            {
                division.mFlash.Configure(kMajorHoldMs, kMajorDecayMs);
            }
            else
            {
                division.mFlash.Configure(kMinorHoldMs, kMinorDecayMs);
            }

            division.mFlash.Reset();

            // division.mAngularSizeRadians = anglePerDivision;
            // division.mBeginAngleRadians = anglePerDivision * (float)i;

            // division.mpModCurve = division.mIsMajor ? mpModCurveMajorFlash : mpModCurveMinorFlash;
            division.mBrightnessQp8 = division.mIsMajor ? kMajorBrightnessQp8 : kMinorBrightnessQp8;
        }
    }

  public:
    static /*constexpr*/ float kOuterRadius;

    static /*constexpr*/ int kBigFlashBrightnessQp8;
    static /*constexpr*/ int kBigFlashHoldMs;
    static /*constexpr*/ int kBigFlashDecayMs;
    static /*constexpr*/ float kBigFlashCurveN11;

    static /*constexpr*/ float kBigFlashRadius;

    static /*constexpr*/ float kMinorInnerRadius;
    static /*constexpr*/ int kMinorBrightnessQp8;
    static /*constexpr*/ int kMinorHoldMs;
    static /*constexpr*/ int kMinorDecayMs;
    static /*constexpr*/ float kMinorCurveN11;

    static /*constexpr*/ float kMajorInnerRadius;
    static /*constexpr*/ int kMajorBrightnessQp8;
    static /*constexpr*/ int kMajorHoldMs;
    static /*constexpr*/ int kMajorDecayMs;
    static /*constexpr*/ float kMajorCurveN11;

    static /*constexpr*/ float kSweepSizeRadians;
    static /*constexpr*/ int kSweepBrightnessQp8;
    static /*constexpr*/ float kSweepCurveN11;
};

bool MetronomeVis2::SettingsChangedTrigger = true;

float MetronomeVis2::kOuterRadius = 28.0f;

int MetronomeVis2::kBigFlashBrightnessQp8 = 255;
int MetronomeVis2::kBigFlashHoldMs = 20;
int MetronomeVis2::kBigFlashDecayMs = 100;
float MetronomeVis2::kBigFlashCurveN11 = 0;
float MetronomeVis2::kBigFlashRadius = 7.0f;

float MetronomeVis2::kMinorInnerRadius = 28.0f - 4.f;
int MetronomeVis2::kMinorBrightnessQp8 = 255;
int MetronomeVis2::kMinorHoldMs = 20;
int MetronomeVis2::kMinorDecayMs = 100;
float MetronomeVis2::kMinorCurveN11 = 0;

float MetronomeVis2::kMajorInnerRadius = 28.0f - 10.f;
int MetronomeVis2::kMajorBrightnessQp8 = 255;
int MetronomeVis2::kMajorHoldMs = 20;
int MetronomeVis2::kMajorDecayMs = 100;
float MetronomeVis2::kMajorCurveN11 = 0;

float MetronomeVis2::kSweepSizeRadians = 0.6f;
int MetronomeVis2::kSweepBrightnessQp8 = 96;
float MetronomeVis2::kSweepCurveN11 = 0;

} // namespace clarinoid
