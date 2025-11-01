
#pragma once

#include <algorithm>
#include <clarinoid/basic/Math.hpp>
#include <clarinoid/settings/SynthSettings.hpp>

namespace clarinoid
{
static constexpr size_t MAX_LINE_OSCILLATOR_SEGMENTS = 8;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct PhaseStep
{
    float phaseBegin01; // slave phase at sample start (no offset), ∈ [0,1)
    float dt;           // per-sample phase advance in phase units, ∈ [0,1)
    bool hasReset;      // master wrapped inside this sample?
    float resetAlpha01; // when in this sample the reset occurs, ∈ [0,1) (valid if hasReset)
};

// Simple accumulator: no offset, no wrap events
struct PhaseAccumulator
{
    float mPhase01 = 0.0;
    float mDelta = 0.0;

    void setPhase01(float p)
    {
        mPhase01 = math::wrap01(p);
    }
    void setFrequencyHz(float hz)
    {
        mDelta = std::max(hz * kSampleRateRecipF, 0.0f);
    }
    float getPhase01() const
    {
        return mPhase01;
    }
    float getDelta() const
    {
        return mDelta;
    }

    void SynchronizeWith(const PhaseAccumulator &src)
    {
        mPhase01 = src.mPhase01;
        mDelta = src.mDelta;
    }

    // advance without resets (used for slave when no hard sync)
    PhaseStep advanceOneSampleNoReset()
    {
        const float begin = mPhase01;
        const float end = math::wrap01(begin + mDelta);
        mPhase01 = end;
        return {begin, mDelta, false, 0.0}; //, end};
    }

    // advance with an externally provided reset alpha (0..1); updates internal phase
    PhaseStep advanceOneSampleWithReset(float alpha01)
    {
        const float begin = mPhase01;
        // pre segment: alpha01 * dt (ignored for state end)
        // post segment after reset: (1 - alpha01) * dt
        const float end = math::wrap01((1.0 - alpha01) * mDelta);
        mPhase01 = end;
        return {begin, mDelta, true, alpha01}; //, end};
    }

    // for high rate frequencies, may wrap more than once per sample.
    size_t advanceOneSampleReturningWrapsCrossed()
    {
        const float begin = mPhase01;
        const float end = begin + mDelta;
        size_t nWraps = (size_t)end; // how many times we crossed 1.0
        mPhase01 = math::wrap01(end);
        return nWraps;
    }
};

// Hard-sync pair: master only detects reset; slave just uses alpha (no offset here)
struct HardSyncPhase
{
    PhaseAccumulator master;
    PhaseAccumulator slave;
    bool enabled = false;

    void setPhase01(float p)
    {
        master.setPhase01(p);
        slave.setPhase01(p);
    }

    void setParams(float mainHz, bool hardSyncEnable, float syncHz)
    {
        enabled = hardSyncEnable;
        master.setFrequencyHz(mainHz);
        slave.setFrequencyHz(hardSyncEnable ? syncHz : mainHz);
    }

    void SynchronizeWith(const HardSyncPhase &src)
    {
        master.SynchronizeWith(src.master);
        slave.SynchronizeWith(src.slave);
    }

    // returns slave step; includes hasReset/resetAlpha01 if master wrapped
    PhaseStep advanceOneSample()
    {
        // detect master wrap (at most one, under dt<1)
        const float mBegin = master.getPhase01();
        const float mDt = master.getDelta();
        const float mEnd = mBegin + mDt;

        bool hasReset = false;
        float alpha = 0.0;
        if (enabled && mEnd >= 1.0) // master wraps this sample
        {
            // when-in-sample = time to reach phase 1.0 divided by dt
            alpha = (1.0 - mBegin) / mDt; // ∈ (0,1]
            if (alpha >= 1.0)
                alpha = 0.0; // push to next sample per your policy
            hasReset = (alpha > 0.0);
        }

        // advance master state (always)
        master.setPhase01(math::wrap01(mEnd));

        // advance slave accordingly (no offset)
        if (!hasReset)
            return slave.advanceOneSampleNoReset();
        return slave.advanceOneSampleWithReset(alpha);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// an output sample of the oscillator.
// struct CoreSample
// {
//     float amplitude = 0.0f; // the final sample value (with bandlimiting applied if applicable)

//     float naive = 0.0f;      // the naive sample value (without bandlimiting)
//     float correction = 0.0f; // the bandlimiting correction to add to the naive value
//     PhaseStep phaseAdvance;  // phase kinematics for this sample
//     // std::string log;
// };

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// all oscillators derive from this (e.g. shape-based, sine wave, or noise...)
struct OscillatorBase
{
  public:
    HardSyncPhase mPhaseAcc;
    float mWaveshapeA = 0.0f;
    float mWaveshapeB = 0.0f;
    float mMainFrequencyHz = 0;
    float mSyncFrequencyHz = 0;
    bool mHardSyncEnabled = false;
    // OscillatorWaveform mWaveformType;

    //   protected:
    //     OscillatorCore(OscillatorWaveform waveformType) : mWaveformType(waveformType)
    //     {
    //     }

  public:
    virtual void SetKRateParams(float shapeA, float shapeB, float mainFreqHz, bool enableHardSync, float syncFreqHz)
    {
        mWaveshapeA = shapeA;
        mWaveshapeB = shapeB;
        mMainFrequencyHz = mainFreqHz;
        mSyncFrequencyHz = syncFreqHz;
        mHardSyncEnabled = enableHardSync;
        mPhaseAcc.setParams(mainFreqHz, enableHardSync, syncFreqHz);
        HandleParamsChanged();
    };

    // used by LFOs to just hard-set the phase. LFO phase, when "note restart" is disabled, is global, so
    // all individual voice LFOs should be in sync and act as if they're the same.
    // Everything after the 1st call will effectively be a NOP, so no special bandlimiting or processing necessary.
    virtual void ForcefullySynchronizePhase(const OscillatorBase &src)
    {
        mPhaseAcc.SynchronizeWith(src.mPhaseAcc);
    };

    virtual void RestartDueToNoteOn()
    {
        // set phase to 0 (because the oscillator's "phase offset" is performed via audioRatePhaseOffset in
        // renderSampleAndAdvance.
        mPhaseAcc.setPhase01(0);
    };

    // allows cores to react to param changes
    virtual void HandleParamsChanged()
    {
    }

    // Render the current sample, and advance phase by 1 sample.
    // step : describes the phase kinematics for this sample.
    // The reason we output the "current" sample and not the sample at the end of the step,
    // is so we render the first sample of the waveform at the initial phase. Otherwise we would always skip the 1st
    // sample of the waveform.
    // - audioRatePhaseOffset: optional phase offset applied at audio rate (e.g. for PM)
    //   I think audio-rate PM may add too much complexity to emit proper band-limiting so we can just assume:
    //   - proper band-limiting if no PM.
    //   - a-rate PM just doesn't get band-limited.
    //
    // Because phase offset is constant over the duration of a single sample,
    // in-sample event times remain the same relative to the window—only their phase locations shift.
    // so we can just shift phaseBegin01 and phaseEnd01 by the offset when you evaluate the shape, keeping the logic
    // simple.
    virtual float renderSampleAndAdvance(float audioRatePhaseOffset) = 0;
};

//
// -------------- straight-line-based waveform shape representation
struct WVSegment
{
    float beginPhase01 = 0;
    float endPhaseIncluding1 = 0; // exclusive (the idea is end is always > begin to make calculating length simple)
    // y value -1 to +1 nominal
    float beginAmp = 0;
    // slope is dy/dx where x is phase in [0,1) (saw wave slope = 2 because it goes from -1 to +1 in one cycle = 2/1 =
    // 2)
    float slope = 0;

    float LengthInPhase01() const
    {
        return endPhaseIncluding1 - beginPhase01;
    }

    // evaluate the amplitude and slope at the given absolute cycle phase (not relative to segment)
    inline std::pair<float, float> EvalAmpSlopeAtPhase(float sampleInPhase01) const
    {
        // do not wrap; allow evaluating outside segment
        const float deltaPhase = sampleInPhase01 - beginPhase01;
        const float amp = beginAmp + slope * (float)deltaPhase;
        return {amp, slope};
    }
};
struct WVShape
{
    fixed_vector<WVSegment, MAX_LINE_OSCILLATOR_SEGMENTS> mSegments;

    // find segment at phase:
    WVSegment FindSegment(float sampleInPhase01) const
    {
        sampleInPhase01 = math::wrap01(sampleInPhase01);
        // assumes segments are sorted by phase ascending
        // assumes >= 1 segment.
        for (size_t i = 0; i < mSegments.size(); ++i)
        {
            const auto &seg = mSegments[i];
            if (sampleInPhase01 < seg.endPhaseIncluding1)
            {
                return seg;
            }
        }
        // unreachable
        return {};
    }

    inline std::pair<float, float> EvalAmpSlopeAt(float sampleInPhase01) const
    {
        auto seg = FindSegment(sampleInPhase01);
        return seg.EvalAmpSlopeAtPhase(sampleInPhase01);
    }
};

// -------------- segment walker (wrapless inside each segment; wraps across 1→0 naturally)
struct SegmentWalker
{
    const WVShape &shape;
    float phase0; // absolute phase at subwindow start (no wrap)
    float dt;     // full-sample dt
    float winLen; // subwindow length as fraction of the sample ∈ [0,1]
    float amp0, slope0;

    static SegmentWalker Begin(const WVShape &sh, float phaseBegin, float dtSample)
    {
        auto [a, s] = sh.EvalAmpSlopeAt(phaseBegin);
        return {sh, phaseBegin, dtSample, 1.0, a, s};
    }
    void ResetSubwindow(float newPhase, float newLen)
    {
        phase0 = newPhase;
        winLen = newLen;
        auto [a, s] = shape.EvalAmpSlopeAt(newPhase);
        amp0 = a;
        slope0 = s;
    }

    template <class F> // F(alpha, dAmp, dSlope)
    void VisitEdges(F &&onEdge)
    {
        float consumed = 0.0;
        float curPhase = phase0;
        float curAmp = amp0;
        float curSlope = slope0;

        while (consumed < winLen)
        {
            const WVSegment &seg = shape.FindSegment(curPhase);
            const float edgePhi = (seg.endPhaseIncluding1 >= 1.0) ? 1.0 : seg.endPhaseIncluding1;

            const float dPhaseToEdge = edgePhi - curPhase;
            const float alpha = dPhaseToEdge / dt; // portion of the *full* sample; may exceed remaining

            if (alpha <= (winLen - consumed) && alpha >= 0.0)
            {
                const float preAlpha = alpha;
                const float preAmp = curAmp + float(preAlpha * dt) * curSlope;
                const float preSlope = curSlope;

                const float postPhase = (edgePhi >= 1.0) ? 0.0 : edgePhi;
                const auto [postAmp, postSlope] = shape.EvalAmpSlopeAt(postPhase);

                onEdge(consumed + preAlpha, float(postAmp) - float(preAmp), float(postSlope) - float(preSlope));

                // step across the edge
                consumed += preAlpha;
                curPhase = postPhase;
                curAmp = postAmp;
                curSlope = postSlope;
            }
            else
                break;
        }
    }
};

// alpha ∈ [0,1): time of the edge within the current sample.
// u = 1 - alpha = remaining fraction of the sample after the edge.
// dAmp = postAmp - preAmp, dSlope = postSlope - preSlope (slope is dy/dphase).
namespace SplitKernels
{
// * 0.5 for canonical polyBLEP normalization (ΔA * .5), to return correction for 1 unit.
// since delta Y is max 2 (-1 to +1), poly_blep has to halve it for the function to work.
static inline void add_blep(float alpha, float dAmp, float &now, float &next)
{
    const float u = 1.0 - alpha;
    const float halfDAmp = 0.5 * dAmp;
    now += halfDAmp * (u * u);
    next += halfDAmp * (-(alpha * alpha));
}

static inline void add_blamp(float alpha, float dSlope, float dt, float &now, float &next)
{
    static constexpr float OneThird = 1.0 / 3.0;
    const float u = 1.0 - alpha;
    const float outputScale = dSlope * dt * 0.5;
    now += outputScale * u * u * u * OneThird;
    const float um1 = u - 1.0;
    next += outputScale * -OneThird * um1 * um1 * um1;
}
}; // namespace SplitKernels

struct CorrectionSpill
{
    float now = 0.0;
    float next = 0.0;
    inline void open_sample()
    {
        now = next;
        next = 0.0;
    }
    inline void add_edge(float alpha, float dAmp, float dSlope, float dt)
    {
        if (dAmp != 0.0)
            SplitKernels::add_blep(alpha, dAmp, now, next);
        if (dSlope != 0.0)
            SplitKernels::add_blamp(alpha, dSlope, dt, now, next);
    }
};

// struct IShapeGenerator
// {
//     virtual WVShape GetShape(float shapeA, float shapeB) const = 0;
// };

// oscillator core for saw / tri / square / etc...
struct StraightLineBasedOscillator : public OscillatorBase
{
    // std::unique_ptr<IShapeGenerator> mShapeGen;
    OscWaveformShape mShapeType;
    WVShape mShape;
    CorrectionSpill mSpill;

    StraightLineBasedOscillator(OscWaveformShape shape) : mShapeType(shape)
    {
    }

    static inline WVShape MakeSawShape()
    {
        return WVShape{{
            // WVSegment{.beginPhase01 = 0.0, .endPhaseIncluding1 = 1, .beginAmp = -1.0f, .slope = +2.0f},
            WVSegment{
                0.0f,  // beginPhase01
                1.0f,  // endPhaseIncluding1
                -1.0f, // beginAmp
                +2.0f  // slope
            },
        }};
    }
    // static inline WVShape MakeTriangleShape()
    // {
    //     // return WVShape{.mSegments = {
    //     //                    WVSegment{.beginPhase01 = 0.0, .endPhaseIncluding1 = 0.5, .beginAmp = -1.0f, .slope =
    //     +4.0f},
    //     //                    WVSegment{.beginPhase01 = 0.5, .endPhaseIncluding1 = 1.0, .beginAmp = +1.0f, .slope =
    //     -4.0f},
    //     //                }};
    // }
    // static inline WVShape MakePulseShape(float dutyCycle01)
    // {
    //     // dutyCycle01 = std::clamp(dutyCycle01, 0.001, 0.999);
    //     // return WVShape{
    //     //     .mSegments = {
    //     //         WVSegment{.beginPhase01 = 0.0, .endPhaseIncluding1 = dutyCycle01, .beginAmp = -1.0f, .slope = 0},
    //     //         WVSegment{.beginPhase01 = dutyCycle01, .endPhaseIncluding1 = 1.0, .beginAmp = 1.0f, .slope = 0},
    //     //     }};
    // }

    void HandleParamsChanged() override
    {
        // switch on mShapeType; output shapes here.
        // tri shape =
        // saw shape =
        // pulse shape =
        // mShape = mShapeGen->GetShape(mWaveshapeA, mWaveshapeB);
        mShape = MakeSawShape();
    };

    float renderSampleAndAdvance(float audioRatePhaseOffset) override
    {
        const PhaseStep step = mPhaseAcc.advanceOneSample(); // no offset here
        const float dt = step.dt;
        const float phase = step.phaseBegin01;

        mSpill.open_sample();

        // naive (evaluation) uses offset
        const float evalPhase = math::wrap01(phase + audioRatePhaseOffset);
        const auto [ampNaive, slopeNaive] = mShape.EvalAmpSlopeAt(evalPhase);
        float y = (float)ampNaive + mSpill.now;
        float corr = mSpill.now;

        // split the sample into pre/post reset windows (if any)
        float preLen = step.hasReset ? step.resetAlpha01 : 1.0;
        float postLen = step.hasReset ? (1.0 - step.resetAlpha01) : 0.0;

        // ---- pre-reset window: walk shape edges starting at evalPhase
        SegmentWalker w = SegmentWalker::Begin(mShape, evalPhase, dt);
        if (preLen > 0.0)
        {
            w.winLen = preLen;
            w.VisitEdges([&](float alpha, float dA, float dS) { mSpill.add_edge(alpha, dA, dS, dt); });
        }

        // ---- dynamic Reset (if any): synthesize edge with exact deltas at the event
        if (step.hasReset)
        {
            const float alpha = step.resetAlpha01;
            const float prePh = evalPhase + alpha * dt;              // just BEFORE reset (no wrap)
            const float postPh = math::wrap01(audioRatePhaseOffset); // AFTER reset (phase = 0 + offset)

            const auto [ampPre, slopePre] = mShape.EvalAmpSlopeAt(prePh);
            const auto [ampPost, slopePost] = mShape.EvalAmpSlopeAt(postPh);

            mSpill.add_edge(alpha, float(ampPost) - float(ampPre), float(slopePost) - float(slopePre), dt);

            // ---- post-reset window: continue walking from postPh
            if (postLen > 0.0)
            {
                w.ResetSubwindow(postPh, postLen);
                w.VisitEdges([&](float alphaLocal, float dA, float dS) {
                    const float alphaFull = alpha + alphaLocal;
                    mSpill.add_edge(alphaFull, dA, dS, dt);
                });
            }
        }

        y = (float)ampNaive + mSpill.now;
        corr = mSpill.now;

        return y;

        // return CoreSample{
        //     .amplitude = (float)y,
        //     .naive = (float)ampNaive,
        //     .correction = (float)corr,
        //     .phaseAdvance = {/* if you still want to return step info, adapt here */},
        // };
    }
};

} // namespace clarinoid