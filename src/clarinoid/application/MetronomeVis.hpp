#pragma once

#include <algorithm>
#include <array>
#include <cmath>

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/basic/Envelope.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include "DisplayDefs.hpp"

namespace clarinoid
{
struct MetronomeVisConfig
{
    PointF mCenter = {100, 27};
};

struct MetronomeVis
{
    MetronomeVis()
    {
        ConfigureEnvelopes();
    }

    void Render(IMetronome &metronome, IDisplay &display, AppSettings &appSettings, const MetronomeVisConfig &config)
    {
        if (SettingsChangedTrigger)
        {
            SettingsChangedTrigger = false;
            Reset();
        }

        // initial calculations & setting fetch
        constexpr float kRadianOffset = -kPI_f / 2.0f; // rotate everything so 0 is at top.
        const auto &perf = appSettings.GetCurrentPerformancePatch();

        int beatsPerBar = perf.mBeatsPerBar;
        int subdivisionsPerBeat = perf.mBeatSubdivisions;
        const int totalSubdivisions = beatsPerBar * subdivisionsPerBeat;
        float totalBeatFloat = metronome.GetBeatFloat();
        uint32_t beatSerial = (uint32_t)floorf(totalBeatFloat);
        float subdivisionFloat = totalBeatFloat * (float)subdivisionsPerBeat;
        int subdivisionSerial = (int)floorf(subdivisionFloat);
        const PointF centerF = config.mCenter;

        // setup & state change triggers
        EnsureEnvelopeCount(beatsPerBar, subdivisionsPerBeat);

        if (!mBeatSerialInitialized || beatSerial != mLastBeatSerial)
        {
            mBeatSerialInitialized = true;
            mLastBeatSerial = beatSerial;
            mBeatFlash.Trigger();

            // int beatIndex = (int)(beatSerial % (uint32_t)beatsPerBar);
        }

        if (!mSubdivisionSerialInitialized || subdivisionSerial != mLastSubdivisionSerial)
        {
            mSubdivisionSerialInitialized = true;
            mLastSubdivisionSerial = subdivisionSerial;

            int markerIndex = RotateIntoRange(subdivisionSerial, totalSubdivisions);
            mSubdivisionEnvelopes[markerIndex].Trigger();
        }

        // render main circle radar
        int mainFlashBrightnessQp8 = mBeatFlash.Sample();
        display.FillRectWithBrightness(display.GetClientRect(), mainFlashBrightnessQp8);

        display.FillCircleWithStrokeF(centerF, kRadius, 1, 0, 255, IDisplay::CircleStrokeMode::Inside);

        const float beatsPerMeasure = (float)beatsPerBar;
        float measureProgress01 = Frac(totalBeatFloat / beatsPerMeasure);

        float radarPosRadians = kTwoPI_f * measureProgress01;

        // draw sweep (brightness from 0 to 255 at the main line)
        {
            float sweepStart = radarPosRadians + kRadianOffset - kSweepWidthRadians;
            display.FillPieSliceGradient(centerF, kRadius, sweepStart, kSweepWidthRadians, 0, 255);
        }

        // float sweepHalfWidth = gPI<float> / 2.0f; // / 15.0f;
        // float sweepStart = sweepCenter + sweepHalfWidth;
        // float sweepSizeRadians = -2.0f * sweepHalfWidth;
        // display.FillPieSliceGradient(
        //     centerF, kRadius, sweepStart, sweepSizeRadians, sweepStartBrightness, sweepEndBrightness);

        // draw main line
        {
            auto lineEnd = PolarToCartesian(centerF, kRadius, radarPosRadians + kRadianOffset);
            display.DrawLine(centerF.Round(), lineEnd.Round());
        }

        // draw beat lines
        for (int beatIndex = 0; beatIndex < beatsPerBar; ++beatIndex)
        {
            float beatFrac = (float)beatIndex / beatsPerBar;
            float beatAngle = kTwoPI_f * beatFrac;

            auto lineEnd = PolarToCartesian(centerF, kRadius, beatAngle + kRadianOffset);
            display.DrawLine(centerF.Round(), lineEnd.Round());
        }

        const float bubbleRadiusF = std::max(1.0f, kSubdivisionRadius);
        for (int marker = 0; marker < totalSubdivisions; ++marker)
        {
            float markerFrac = (float)marker / totalSubdivisions;
            float markerAngle = kTwoPI_f * markerFrac;
            PointF markerPos = PolarToCartesian(centerF, kRadius, markerAngle + kRadianOffset);

            int brightness = mSubdivisionEnvelopes[marker].Sample();

            // draw line for subdiv
            display.DrawLineWithBrightness(centerF.Round(), markerPos.Round(), brightness);

            display.FillCircleWithStrokeF(
                PointF{markerPos.Round()}, bubbleRadiusF, 1, brightness, 255, IDisplay::CircleStrokeMode::Inside);
        }
    }

    static /*constexpr*/ float kRadius;
    static /*constexpr*/ float kSubdivisionRadius;

    static /*constexpr*/ int kMainFlashHoldMs;  // = 33;
    static /*constexpr*/ int kMainFlashDecayMs; // = 220;

    static /*constexpr*/ int kSubdivisionFlashHoldMs;  // = 33;
    static /*constexpr*/ int kSubdivisionFlashDecayMs; // = 220;

    static /*constexpr*/ float kSweepWidthRadians;

    static bool SettingsChangedTrigger;

  private:
    static constexpr int kMaxSubdivisionSlots = kMaxBeatsPerBar * kMaxSubdivisionsPerBeat;
    static constexpr int kBaseSubdivisionBrightness = 0;

    FlashEnvelope mBeatFlash;
    std::array<FlashEnvelope, kMaxSubdivisionSlots> mSubdivisionEnvelopes;
    bool mBeatSerialInitialized = false;
    bool mSubdivisionSerialInitialized = false;
    uint32_t mLastBeatSerial = 0;
    int mLastSubdivisionSerial = 0;
    int mCachedBeatCount = 0;
    int mCachedSubdivisionPerBeat = 0;

    // call when configuration or settings change
    void Reset()
    {
        //
        mBeatSerialInitialized = false;
        mSubdivisionSerialInitialized = false;
        mLastBeatSerial = 0;
        mLastSubdivisionSerial = 0;
        mBeatFlash.Reset();
        for (auto &env : mSubdivisionEnvelopes)
        {
            env.Reset();
        }
        ConfigureEnvelopes();
    }

    void ConfigureEnvelopes()
    {
        mBeatFlash.Configure(kMainFlashHoldMs, kMainFlashDecayMs, 255);
        for (auto &env : mSubdivisionEnvelopes)
        {
            env.Configure(kMainFlashHoldMs, kMainFlashDecayMs, 255);
        }
    }

    void EnsureEnvelopeCount(int beatCount, int subdivisionsPerBeat)
    {
        if (beatCount == mCachedBeatCount && subdivisionsPerBeat == mCachedSubdivisionPerBeat)
        {
            return;
        }

        int total = beatCount * subdivisionsPerBeat;
        for (int i = total; i < kMaxSubdivisionSlots; ++i)
        {
            mSubdivisionEnvelopes[i].Reset();
        }

        mCachedBeatCount = beatCount;
        mCachedSubdivisionPerBeat = subdivisionsPerBeat;
        mBeatSerialInitialized = false;
        mSubdivisionSerialInitialized = false;
    }
};

float MetronomeVis::kRadius = 23.0f;
float MetronomeVis::kSubdivisionRadius = 5.f;
int MetronomeVis::kMainFlashHoldMs = 40;
int MetronomeVis::kMainFlashDecayMs = 120;
int MetronomeVis::kSubdivisionFlashHoldMs = 40;
int MetronomeVis::kSubdivisionFlashDecayMs = 120;
bool MetronomeVis::SettingsChangedTrigger = false;
float MetronomeVis::kSweepWidthRadians = 0.5f;

} // namespace clarinoid
