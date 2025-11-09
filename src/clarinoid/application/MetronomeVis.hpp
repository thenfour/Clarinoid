#pragma once

#include <algorithm>
#include <array>
#include <cmath>

// #include <clarinoid/basic/Basic.hpp>
// #include <clarinoid/basic/Envelope.hpp>
// #include <clarinoid/settings/AppSettings.hpp>
// #include "DisplayDefs.hpp"

// namespace clarinoid
// {
// struct MetronomeVisConfig
// {
//     PointF mCenter = {100, 27};
//     RectI mFlashRect = RectI::Construct(0, 0, 64, 64);
// };

// struct MetronomeVis
// {
//     MetronomeVis()
//     {
//         ConfigureEnvelopes();
//     }

//     void Render(IMetronome &metronome, IDisplay &display, AppSettings &appSettings, const MetronomeVisConfig &config)
//     {
//         if (SettingsChangedTrigger)
//         {
//             SettingsChangedTrigger = false;
//             Reset();
//         }

//         // initial calculations & setting fetch
//         constexpr float kRadianOffset = -kPI_f / 2.0f; // rotate everything so 0 is at top.
//         const auto &perf = appSettings.GetCurrentPerformancePatch();

//         int beatsPerBar = perf.mBeatsPerBar;
//         int subdivisionsPerBeat = perf.mBeatSubdivisions;
//         const int totalSubdivisions = beatsPerBar * subdivisionsPerBeat;
//         float totalBeatFloat = metronome.GetBeatFloat();
//         uint32_t beatSerial = (uint32_t)floorf(totalBeatFloat);
//         float subdivisionFloat = totalBeatFloat * (float)subdivisionsPerBeat;
//         int subdivisionSerial = (int)floorf(subdivisionFloat);
//         const PointF centerF = config.mCenter;

//         // setup & state change triggers
//         EnsureEnvelopeCount(beatsPerBar, subdivisionsPerBeat);

//         if (!mBeatSerialInitialized || beatSerial != mLastBeatSerial)
//         {
//             mBeatSerialInitialized = true;
//             mLastBeatSerial = beatSerial;
//             mBeatFlash.Trigger();

//             // int beatIndex = (int)(beatSerial % (uint32_t)beatsPerBar);
//         }

//         if (!mSubdivisionSerialInitialized || subdivisionSerial != mLastSubdivisionSerial)
//         {
//             mSubdivisionSerialInitialized = true;
//             mLastSubdivisionSerial = subdivisionSerial;

//             int markerIndex = RotateIntoRange(subdivisionSerial, totalSubdivisions);
//             mSubdivisionEnvelopes[markerIndex].Trigger();
//         }

//         // render main radar circle
//         display.FillCircleWithStrokeF(centerF, kRadius, 1, 0, 255, IDisplay::StrokeMode::Inside);

//         // big flash on beat
//         int mainFlashBrightnessQp8 = mBeatFlash.Sample01() * kBigFlashMaxBrightnessQp8;
//         display.FillRectWithBrightness(config.mFlashRect, mainFlashBrightnessQp8);

//         const float beatsPerMeasure = (float)beatsPerBar;
//         float measureProgress01 = Frac(totalBeatFloat / beatsPerMeasure);
//         float radarPosRadians = kTwoPI_f * measureProgress01;

//         // draw sweep (brightness from 0 to 255 at the main line)
//         {
//             float sweepStart = radarPosRadians + kRadianOffset - kSweepWidthRadians;
//             display.FillPieSliceGradient(centerF, kRadius, sweepStart, kSweepWidthRadians, 0, kSweepMaxBrightnessQp8);
//         }

//         // draw main animated line
//         {
//             auto lineEnd = PolarToCartesian(centerF, kRadius, radarPosRadians + kRadianOffset);
//             display.DrawLine(centerF.Round(), lineEnd.Round());
//         }

//         for (int marker = 0; marker < totalSubdivisions; ++marker)
//         {
//             float markerFrac = (float)marker / totalSubdivisions;
//             float markerAngle = kTwoPI_f * markerFrac;
//             PointF markerPos = PolarToCartesian(centerF, kRadius, markerAngle + kRadianOffset);

//             int brightness = mSubdivisionEnvelopes[marker].Sample01() * kSubdivFlashMaxBrightnessQp8;

//             // draw line for subdiv
//             display.DrawLineWithBrightness(centerF.Round(), markerPos.Round(), brightness);
//         }

//         // draw beat lines
//         for (int beatIndex = 0; beatIndex < beatsPerBar; ++beatIndex)
//         {
//             float beatFrac = (float)beatIndex / beatsPerBar;
//             float beatAngle = kTwoPI_f * beatFrac;

//             auto lineEnd = PolarToCartesian(centerF, kRadius, beatAngle + kRadianOffset);
//             display.DrawLine(centerF.Round(), lineEnd.Round());
//         }

//         for (int marker = 0; marker < totalSubdivisions; ++marker)
//         {
//             float markerFrac = (float)marker / totalSubdivisions;
//             float markerAngle = kTwoPI_f * markerFrac;
//             PointF markerPos = PolarToCartesian(centerF, kRadius, markerAngle + kRadianOffset);

//             int brightness = mSubdivisionEnvelopes[marker].Sample01() * kSubdivFlashMaxBrightnessQp8;

//             display.FillCircleWithStrokeF(
//                 PointF{markerPos.Round()}, kSubdivisionRadius, 1, brightness, 255, IDisplay::StrokeMode::Inside);
//         }
//     }

//     static /*constexpr*/ float kRadius;
//     static /*constexpr*/ float kBeatRadius;
//     static /*constexpr*/ float kSubdivisionRadius;

//     static /*constexpr*/ int kMainFlashHoldMs;  // = 33;
//     static /*constexpr*/ int kMainFlashDecayMs; // = 220;

//     static /*constexpr*/ int kSubdivisionFlashHoldMs;  // = 33;
//     static /*constexpr*/ int kSubdivisionFlashDecayMs; // = 220;

//     static /*constexpr*/ float kSweepWidthRadians;

//     static /*constexpr*/ int kSweepMaxBrightnessQp8;
//     static /*constexpr*/ int kSubdivFlashMaxBrightnessQp8; // for subdivisions
//     static /*constexpr*/ int kBeatFlashMaxBrightnessQp8;   // for major beats
//     static /*constexpr*/ int kBigFlashMaxBrightnessQp8;    // for flood fill beat flash

//     static bool SettingsChangedTrigger;

//   private:
//     static constexpr int kMaxSubdivisionSlots = kMaxBeatsPerBar * kMaxSubdivisionsPerBeat;
//     static constexpr int kBaseSubdivisionBrightness = 0;

//     FlashEnvelope mBeatFlash;
//     std::array<FlashEnvelope, kMaxSubdivisionSlots> mSubdivisionEnvelopes;
//     bool mBeatSerialInitialized = false;
//     bool mSubdivisionSerialInitialized = false;
//     uint32_t mLastBeatSerial = 0;
//     int mLastSubdivisionSerial = 0;
//     int mCachedBeatCount = 0;
//     int mCachedSubdivisionPerBeat = 0;

//     // call when configuration or settings change
//     void Reset()
//     {
//         //
//         mBeatSerialInitialized = false;
//         mSubdivisionSerialInitialized = false;
//         mLastBeatSerial = 0;
//         mLastSubdivisionSerial = 0;
//         mBeatFlash.Reset();
//         for (auto &env : mSubdivisionEnvelopes)
//         {
//             env.Reset();
//         }
//         ConfigureEnvelopes();
//     }

//     void ConfigureEnvelopes()
//     {
//         mBeatFlash.Configure(kMainFlashHoldMs, kMainFlashDecayMs);
//         for (auto &env : mSubdivisionEnvelopes)
//         {
//             env.Configure(kSubdivisionFlashHoldMs, kSubdivisionFlashDecayMs);
//         }
//     }

//     void EnsureEnvelopeCount(int beatCount, int subdivisionsPerBeat)
//     {
//         if (beatCount == mCachedBeatCount && subdivisionsPerBeat == mCachedSubdivisionPerBeat)
//         {
//             return;
//         }

//         int total = beatCount * subdivisionsPerBeat;
//         for (int i = total; i < kMaxSubdivisionSlots; ++i)
//         {
//             mSubdivisionEnvelopes[i].Reset();
//         }

//         mCachedBeatCount = beatCount;
//         mCachedSubdivisionPerBeat = subdivisionsPerBeat;
//         mBeatSerialInitialized = false;
//         mSubdivisionSerialInitialized = false;
//     }
// };

// float MetronomeVis::kRadius = 23.0f;
// float MetronomeVis::kSubdivisionRadius = 5.f;
// int MetronomeVis::kMainFlashHoldMs = 20;
// int MetronomeVis::kMainFlashDecayMs = 100;
// int MetronomeVis::kSubdivisionFlashHoldMs = 20;
// int MetronomeVis::kSubdivisionFlashDecayMs = 200;
// bool MetronomeVis::SettingsChangedTrigger = false;
// float MetronomeVis::kSweepWidthRadians = 0.6f;
// int MetronomeVis::kSweepMaxBrightnessQp8 = 96;
// int MetronomeVis::kSubdivFlashMaxBrightnessQp8 = 255;
// int MetronomeVis::kBeatFlashMaxBrightnessQp8 = 255;
// int MetronomeVis::kBigFlashMaxBrightnessQp8 = 255;

// } // namespace clarinoid
