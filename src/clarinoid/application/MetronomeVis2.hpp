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
        bool mFirstInMeasure = false;
        bool mIsMajor = false;
    };

    void Render(IMetronome &metronome, IDisplay &display, AppSettings &appSettings)
    {
        const PointF kCenterF = PointF::Construct(100, 27);
        const RectI kFlashRectI = RectI::Construct(0, 0, 64, 64);

        constexpr float kRadianOffset = -kPI_f / 2.0f; // rotate everything so 0 is at top.
        const auto &perf = appSettings.GetCurrentPerformancePatch();
        int beatsPerBar = perf.mBeatsPerBar;
        int subdivisionsPerBeat = perf.mBeatSubdivisions;
        const int totalSubdivisions = beatsPerBar * subdivisionsPerBeat;

        // if settings changed that affect viz, reset.
        if (SettingsChangedTrigger || Metronome::SettingsChangedForViz ||
            mLastPerfPatchIndex != appSettings.mCurrentPerformancePatch || mLastBeatsPerBar != beatsPerBar ||
            mLastSubdivisionsPerBeat != subdivisionsPerBeat || mLastBPM != perf.mBPM)
        {
            mLastPerfPatchIndex = appSettings.mCurrentPerformancePatch;
            mLastBeatsPerBar = beatsPerBar;
            mLastSubdivisionsPerBeat = subdivisionsPerBeat;
            mLastBPM = perf.mBPM;
            SettingsChangedTrigger = false;
            Metronome::SettingsChangedForViz = false;
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

        int markerIndex = RotateIntoRange(subdivisionSerial, totalSubdivisions);

        // if (subdivisionSerial != mLastSubdivisionSerial)
        // {
        //     mLastSubdivisionSerial = subdivisionSerial;
        //     mDivisions[markerIndex].mFlash.Trigger();
        // }

        float measureProgress01 = Frac(totalBeatFloat / beatsPerBar);
        float radarPosRadians = kTwoPI_f * measureProgress01;

        // big flash on beat (best drawn behind other elements)
        {
            float s = mBigFlash.Sample01();
            float flash01 = gModCurveLUT.Transfer32(s, mpModCurveBigFlash);
            // int mainFlashBrightnessQp8 = flash01 * kBigFlashBrightnessQp8;
            if (flash01 > 0.5f)
            {
                display.fillRect(kFlashRectI.x, kFlashRectI.y, kFlashRectI.width, kFlashRectI.height, SSD1306_WHITE);
                display.fillCircle(kCenterF.x, kCenterF.y, (int)kBigFlashRadius, SSD1306_WHITE);
                // display.FillRectWithBrightness(flashRect, mainFlashBrightnessQp8);
                // display.FillCircleWithBrightnessF(centerF, kBigFlashRadius, mainFlashBrightnessQp8);
            }
        }

        // draw sweep (brightness from 0 to 255 at the main line)
        {
            // float sweepStart = radarPosRadians + kRadianOffset - kSweepSizeRadians;

            // display.FillDonutSliceGradient(
            //     centerF, 0, kMinorInnerRadius, sweepStart, kSweepSizeRadians, 0, kSweepBrightnessQp8, nullptr);
        }

        // draw main animated line
        {
            // auto lineStart = PolarToCartesian(centerF, kBigFlashRadius, radarPosRadians + kRadianOffset);
            auto lineEnd = PolarToCartesian(kCenterF, kMajorInnerRadius, radarPosRadians + kRadianOffset);
            display.DrawLine(kCenterF.Round(), lineEnd.Round());
        }

        // draw divisions.
        for (int divisionIndex = 0; divisionIndex < totalSubdivisions; ++divisionIndex)
        {
            Division &division = mDivisions[divisionIndex];

            float mBeginAngleRadians = mAnglePerDivision * (float)divisionIndex;

            bool isThis = markerIndex == divisionIndex;
            int brightnessQp8 = isThis ? 255 : 0;
            if (brightnessQp8 == 0)
                continue;

            float innerRadius = division.mIsMajor ? kMajorInnerRadius : kMinorInnerRadius;
            display.FillDonutSliceGradient(kCenterF,
                                           innerRadius,
                                           kOuterRadius,
                                           mBeginAngleRadians + kRadianOffset,
                                           mAnglePerDivision,
                                           brightnessQp8,
                                           brightnessQp8,
                                           nullptr);
        }

        // draw beat division lines. at the beginning (radially) of each beat, draw a line from center to major inner
        // radius.
        {
            for (int beatIndex = 0; beatIndex < beatsPerBar; ++beatIndex)
            {
                float beatFrac = (float)beatIndex / beatsPerBar;
                float beatAngle = kTwoPI_f * beatFrac;

                auto lineEnd = PolarToCartesian(kCenterF, kMajorInnerRadius, beatAngle + kRadianOffset);
                display.DrawLine(kCenterF.Round(), lineEnd.Round());
            }
        }

        // draw division bounds
        display.DrawCircleStroke1px(kCenterF, kOuterRadius, IDisplay::StrokeMode::Inside);

        // for each division,
        for (int divisionIndex = 0; divisionIndex < totalSubdivisions; ++divisionIndex)
        {
            Division &division = mDivisions[divisionIndex];
            // draw line at start angle from its inner radius to outer radius
            float mBeginAngleRadians = mAnglePerDivision * (float)divisionIndex;
            float lineAngleRadians = mBeginAngleRadians + kRadianOffset;
            float innerRadius = division.mIsMajor ? kMajorInnerRadius : kMinorInnerRadius;
            auto lineStart = PolarToCartesian(kCenterF, innerRadius, lineAngleRadians);
            auto lineEnd = PolarToCartesian(kCenterF, kOuterRadius, lineAngleRadians);
            display.DrawLine(lineStart.Round(), lineEnd.Round());

            // end angle line.
            float mEndAngleRadians = mBeginAngleRadians + mAnglePerDivision;
            float lineEndAngleRadians = mEndAngleRadians + kRadianOffset;
            auto lineEndStart = PolarToCartesian(kCenterF, innerRadius, lineEndAngleRadians);
            auto lineEndEnd = PolarToCartesian(kCenterF, kOuterRadius, lineEndAngleRadians);
            display.DrawLine(lineEndStart.Round(), lineEndEnd.Round());

            // arc at inner radius
            display.DrawArcStroke1px(kCenterF,
                                     innerRadius,
                                     mBeginAngleRadians + kRadianOffset,
                                     mAnglePerDivision,
                                     IDisplay::StrokeMode::Inside);
        }
    }

    static bool SettingsChangedTrigger;

  private:
    static constexpr int kMaxSubdivisionSlots = kMaxBeatsPerBar * kMaxSubdivisionsPerBeat;

    FlashEnvelope mBigFlash;
    fixed_vector<Division, kMaxSubdivisionSlots> mDivisions;
    uint32_t mLastBeatSerial = 0;
    int mLastSubdivisionSerial = 0;

    ModCurveRow mpModCurveBigFlash = nullptr;
    // ModCurveRow mpModCurveMajorFlash = nullptr;
    // ModCurveRow mpModCurveMinorFlash = nullptr;
    // ModCurveRow mpModCurveSweep = nullptr;
    float mAnglePerDivision = 0;

    // detect when we need to reset.
    int mLastPerfPatchIndex = -1;
    int mLastBeatsPerBar = -1;
    int mLastSubdivisionsPerBeat = -1;
    float mLastBPM = -1.0f;

    // call when configuration or settings change
    void Reset(const PerformancePatch &perf)
    {
        mpModCurveBigFlash = gModCurveLUT.BeginLookupF(kBigFlashCurveN11);
        // mpModCurveMajorFlash = gModCurveLUT.BeginLookupF(kMajorCurveN11);
        // mpModCurveMinorFlash = gModCurveLUT.BeginLookupF(kMinorCurveN11);
        // mpModCurveSweep = gModCurveLUT.BeginLookupF(kSweepCurveN11);

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

            // if (division.mIsMajor)
            // {
            //     division.mFlash.Configure(kMajorHoldMs, kMajorDecayMs);
            // }
            // else
            // {
            //     division.mFlash.Configure(kMinorHoldMs, kMinorDecayMs);
            // }

            // division.mFlash.Reset();
        }
    }

  public:
    // static /*constexpr*/ float kOuterRadius;
    // static /*constexpr*/ int kBigFlashBrightnessQp8;
    // static /*constexpr*/ int kBigFlashHoldMs;
    // static /*constexpr*/ int kBigFlashDecayMs;
    // static /*constexpr*/ float kBigFlashCurveN11;
    // static /*constexpr*/ float kBigFlashRadius;
    // static /*constexpr*/ float kMinorInnerRadius;
    // static /*constexpr*/ float kMajorInnerRadius;
    // static /*constexpr*/ float kSweepSizeRadians;
    // static /*constexpr*/ int kSweepBrightnessQp8;

    static constexpr float kOuterRadius = 27.0f;
    static constexpr int kBigFlashBrightnessQp8 = 255;
    static constexpr int kBigFlashHoldMs = 30;
    static constexpr int kBigFlashDecayMs = 160;
    static constexpr float kBigFlashCurveN11 = -0.5f;
    static constexpr float kBigFlashRadius = 12.0f;
    static constexpr float kMinorInnerRadius = 18.0f;
    static constexpr float kMajorInnerRadius = 12.0f;
    static constexpr float kSweepSizeRadians = 0.6f;
    static constexpr int kSweepBrightnessQp8 = 96;
};

bool MetronomeVis2::SettingsChangedTrigger = true;

} // namespace clarinoid
