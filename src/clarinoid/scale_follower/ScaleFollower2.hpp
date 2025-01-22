
#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/basic/Music.hpp>
#include <clarinoid/settings/AppSettings.hpp>
#include <clarinoid/harmonizer/harmonizer.hpp>

namespace clarinoid
{

class PitchClassEnvelope {
public:
    PitchClassEnvelope() : mEnvelope(0.0f) {}

    // Call this each frame to move the envelope toward the target or toward 0
    void Update(float target, float alphaAttack, float alphaRelease)
    {
        if (target > mEnvelope) {
            // Attack
            mEnvelope += (target - mEnvelope) * alphaAttack;
        } else {
            // Release (down to 0 if not active, or partial if partial approach)
            mEnvelope += (0.0f - mEnvelope) * alphaRelease;
        }
    }

    float GetLevel() const { return mEnvelope; }

private:
    float mEnvelope;
};

class PitchClassEnvelopes {
public:
    PitchClassEnvelopes() {
        mEnvelopes.fill(PitchClassEnvelope());
    }

    // For each frame, do:
    // 1. Calculate "desired level" for each pitch class.
    // 2. Call envelope.Update() for each pitch class.
    void Update(const MusicalVoice* voices,
                size_t voiceCount,
                float alphaAttack,
                float alphaRelease)
    {
        // 1) Gather desired levels
        std::array<float, 12> desired;
        desired.fill(0.0f);

        for (size_t i = 0; i < voiceCount; ++i) {
            if (voices[i].IsPlaying()) {
                int pc = (voices[i].mMidiNote % 12 + 12) % 12;
                // accumulate breath or loudness
                desired[pc] += 0.25f + 0.75 * voices[i].mBreath01.GetFloatVal(); // squeeze upwards; basically giving a minimum score for playing at all.
            }
        }

        // clamp to [0..1]
        for (int pc = 0; pc < 12; ++pc) {
            desired[pc] = std::min(1.0f, desired[pc]);
        }

        // 2) Update each pitch envelope
        for (int pc = 0; pc < 12; ++pc) {
            mEnvelopes[pc].Update(desired[pc], alphaAttack, alphaRelease);
        }
    }

    float GetLevel(int pc) const {
        return mEnvelopes[pc].GetLevel();
    }

private:
    std::array<PitchClassEnvelope, 12> mEnvelopes;
};

struct ScaleDetector {

    // Main call ~ every 3ms
    Scale Update(const MusicalVoice* voices, size_t voiceCount)
    {
        // 1. Update pitch envelopes

        mEnvelopes.Update(voices, voiceCount, mAlphaAttack, mAlphaRelease);

        // 2. Find best scale
        return FindBestScale();
    }

    PitchClassEnvelopes mEnvelopes;

private:
    float mAttackTimeSec = 0.1f;
    float mReleaseTimeSec = 3.0f;
    float mFrameTimeSec = 0.003f;

    float mAlphaAttack  = 1.0f - std::exp(-mFrameTimeSec / mAttackTimeSec);
    float mAlphaRelease = 1.0f - std::exp(-mFrameTimeSec / mReleaseTimeSec);

    ScaleFlavorIndex mScaleFlavors[3] = {
        ScaleFlavorIndex::Major,
        //ScaleFlavorIndex::MajorPentatonic,
        ScaleFlavorIndex::MelodicMinor,
        //ScaleFlavorIndex::Blues,
        ScaleFlavorIndex::HalfWholeDiminished,
    };

    Scale FindBestScale()
    {
        float bestScore = -1e9f;

        // We'll store the best scale as (root, scaleDefinition)
        Scale bestScale = { Note::C, ScaleFlavorIndex::Major }; // or however you want to represent it

        // Evaluate each root from 0..11
        for (int root = 0; root < 12; ++root) {
            for (auto& flavor : mScaleFlavors) {
                Scale scale = { (Note)root, flavor };
                float score = MeasureFitness(mEnvelopes, scale);
                if (score > bestScore) {
                    bestScore = score;
                    bestScale = scale;
                }
            }
        }

        return bestScale;
    }

    float MeasureFitness(const PitchClassEnvelopes& pce, const Scale& scale) const
    {
        float score = 0.0f;

        for (int pc = 0; pc < 12; ++pc) {
            float fit = scale.GetFitnessForNote((Note)pc);
            float presence = pce.GetLevel(pc);
            score += presence * fit;
        }
        return score;
    }
};

} // namespace clarinoid
