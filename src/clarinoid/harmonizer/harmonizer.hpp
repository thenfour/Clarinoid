
#pragma once

#include "AnalogValue.hpp"
#include "MusicalVoice.hpp"
#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>

namespace clarinoid
{

struct Harmonizer
{
    AppSettings *mAppSettings;

    explicit Harmonizer(AppSettings *appSettings) : mAppSettings(appSettings)
    {
    }

    // state & processing for harmonizer.

    struct VoiceState
    {
        uint8_t mCurrentMidiNote = 0; // 0 means no note.
        // String mResult;
    };

    enum class VoiceFilterOptions : uint8_t
    {
        AllExceptDeducedVoices,
        OnlyDeducedVoices,
    };

    size_t mSequencePos = 0;
    Stopwatch mRotationTriggerTimer;
    VoiceState mVoiceStates[HARM_VOICES];

    // accepts input note + voice settings, returns harmonized note
    static uint8_t GetHarmonizedNote(uint8_t inputNote,
                                     Scale &scale,
                                     Scale &deducedScale,
                                     size_t sequencePos,
                                     HarmPreset &preset,
                                     HarmVoiceSettings &voiceSetting,
                                     VoiceState &voiceState)
    {
        auto sequenceIndex = sequencePos % voiceSetting.mSequenceLength;
        uint8_t scaleRoot = 0;
        auto ctx = scale.GetNoteInScaleContext(inputNote, scaleRoot, EnharmonicDirection::Sharp);
        if (ctx.mEnharmonic == 0)
        {
            // diatonic.
            ctx.mScaleDegree += voiceSetting.mSequence[sequenceIndex];
            auto ret = scale.GetMidiNoteFromContext(ctx, scaleRoot); // get the harmonized note in the same scale
            // voiceState.mResult = String("diatonic ") + ret;
            voiceState.mCurrentMidiNote = ret;
            return ret;
        }

        // deal with non-diatonic cases
        switch (voiceSetting.mNonDiatonicBehavior)
        {
        default:
        case NonDiatonicBehavior::Drop:
            // voiceState.mResult = "harm mute";
            return 0; // indicate mute
        case NonDiatonicBehavior::UseScaleFollower: {
            auto ctx = deducedScale.GetNoteInScaleContext(inputNote, scaleRoot, EnharmonicDirection::Sharp);
            // just ignore if it's diatonic here. it would be weird for the deduced scale not to contain the live note.
            // if it's non-diatonic, then it will chromatically adjust anyway.
            ctx.mScaleDegree += voiceSetting.mSequence[sequenceIndex];
            auto ret = scale.GetMidiNoteFromContext(ctx, scaleRoot); // get the harmonized note in the same scale
            // voiceState.mResult = String("scalefoll ") + ret;
            voiceState.mCurrentMidiNote = ret;
            return ret;
        }
        break;
        case NonDiatonicBehavior::NearestDiatonic: {
            // the input note is non-diatonic; adjust it to the nearest diatonic note and harmonize from that.
            // we already measured in context of sharps; measure as flat.
            uint8_t scaleRootFlat = 0;
            auto ctxFlat = scale.GetNoteInScaleContext(inputNote, scaleRootFlat, EnharmonicDirection::Flat);
            if (std::abs(ctxFlat.mEnharmonic) < std::abs(ctx.mEnharmonic))
            {
                ctx = ctxFlat;
            }
            // sharp is nearer (or equal)
            ctx.mEnharmonic = 0; // erase the chromatic adjustment; we're making it diatonic now.
            ctx.mScaleDegree += voiceSetting.mSequence[sequenceIndex];
            auto ret = scale.GetMidiNoteFromContext(ctx, scaleRoot);
            // voiceState.mResult = String("nearest ") + ret;
            voiceState.mCurrentMidiNote = ret;
            return ret;
        }
        break;
        case NonDiatonicBehavior::ChromaticFromAbove: {
            // just express the chromatic adjustment in terms of flats instead of sharps.
            auto ctx = scale.GetNoteInScaleContext(inputNote, scaleRoot, EnharmonicDirection::Flat);
            ctx.mScaleDegree += voiceSetting.mSequence[sequenceIndex];
            auto ret = scale.GetMidiNoteFromContext(ctx, scaleRoot);
            // voiceState.mResult = String("chromatic above ") + ret;
            voiceState.mCurrentMidiNote = ret;
            return ret;
        }
        break;
        case NonDiatonicBehavior::ChromaticFromBelow: {
            ctx.mScaleDegree += voiceSetting.mSequence[sequenceIndex];
            auto ret = scale.GetMidiNoteFromContext(ctx, scaleRoot);
            // voiceState.mResult = String("chromatic below ") + ret;
            voiceState.mCurrentMidiNote = ret;
            return ret;
        }
        break;
        }
    }

    static uint8_t EnsureHarmonizedNoteBounds(uint8_t note, uint8_t liveNote, HarmVoiceSettings &voiceSetting)
    {
        // out-of-bounds behavior
        switch (voiceSetting.mNoteOOBBehavior)
        {
        default:
        case NoteOOBBehavior::Mute: {
            if (note < voiceSetting.mMinOutpNote || note > voiceSetting.mMaxOutpNote)
            {
                return 0;
            }
        }
        break;
        case NoteOOBBehavior::RotateIntoRange: {
            int32_t outp = note;
            if (!wrapByStepIntoRange<12>(note, voiceSetting.mMinOutpNote, voiceSetting.mMaxOutpNote, outp))
            {
                return 0;
            }
            return outp;
        }
        break;
        case NoteOOBBehavior::RotateBelowLive: {
            // effectively, it's the same as RotateIntoRange, just constraining the range to [min, live-1]
            int32_t outp = note;
            if (!wrapByStepIntoRange<12>(note, voiceSetting.mMinOutpNote, liveNote - 1, outp))
            {
                return 0;
            }
            return outp;
        }
        break;
        case NoteOOBBehavior::RotateAboveLive: {
            // effectively, it's the same as RotateIntoRange, just constraining the range to [live+1, max]
            int32_t outp = note;
            if (!wrapByStepIntoRange<12>(note, liveNote + 1, voiceSetting.mMaxOutpNote, outp))
            {
                return 0;
            }
            return outp;
        }
        break;
        }
        return note;
    }

    // called each frame to add harmonizer voices to the output, given the live
    // playing voice. liveVoice is considered a part of the output. It will be
    // muted or unmuted whether it should be part of playback returns the number
    // of voices added (including live voice, even if muted) layerID is needed in
    // order to create the voiceID
    size_t Harmonize(uint8_t loopLayerID,
                     MusicalVoice *liveVoice,
                     const MusicalVoiceTransitionEvents &transitionEvents,
                     MusicalVoice *outp,
                     MusicalVoice *end,
                     VoiceFilterOptions voiceFilter,
                     bool log)
    {
        HarmPreset &preset = mAppSettings->FindHarmPreset(liveVoice->mHarmPatch);

        size_t ret = 0;

        // advance sequence pointer?
        if (transitionEvents.mNeedsNoteOn)
        {
            if (mRotationTriggerTimer.ElapsedTime().ElapsedMillisI() >= preset.mMinRotationTimeMS)
            {
                mRotationTriggerTimer.Restart();
                mSequencePos++;
                // Serial.println(String("seq") + mSequencePos);
            }
        }

        auto &perf = mAppSettings->GetCurrentPerformancePatch();

        // LIVE note:
        // harmonizing should always output the live note; if it's not part of the
        // real harmonized output, then mark it as muted. it needs to be there so
        // the scale deducer can use it.
        liveVoice->mIsNoteCurrentlyMuted = !preset.mEmitLiveNote || !perf.mSynthAEnabled;
        liveVoice->mVoiceId = MakeMusicalVoiceID(loopLayerID, MAGIC_VOICE_ID_LIVE_A);
        liveVoice->mGain *= perf.mSynthAGain;
        if (voiceFilter == Harmonizer::VoiceFilterOptions::AllExceptDeducedVoices)
        {
            ++ret; // live voice is a non-deduced voice.
        }

        MusicalVoice *pout = outp;
        if (pout >= end)
        {
            return ret;
        }

        if (preset.mEmitLiveNote && (liveVoice->mSynthPatchB >= 0))
        {
            *pout = *liveVoice; // copy from live voice to get started.
            pout->mVoiceId = MakeMusicalVoiceID(loopLayerID, MAGIC_VOICE_ID_LIVE_B);
            pout->mGain *= perf.mSynthBGain;
            pout->mIsNoteCurrentlyMuted = !preset.mEmitLiveNote || !perf.mSynthBEnabled;
            pout->mSynthPatchA = pout->mSynthPatchB;
            pout->mSynthPatchB = liveVoice->mSynthPatchB =
                -1; // as we split this voice into 2, remove the reference to patch B.

            // stereo spread of A & B synth patches
            liveVoice->mPan -= perf.mSynthStereoSpread;
            pout->mPan += perf.mSynthStereoSpread;

            ++pout;
            ++ret;
            if (pout >= end)
            {
                return ret;
            }
        }

        bool globalDeduced = perf.mGlobalScaleRef == GlobalScaleRefType::Deduced;
        Scale globalScale = globalDeduced ? perf.mDeducedScale : perf.mGlobalScale;

        // reset states.
        VoiceState sentinel;
        for (size_t nVoice = 0; nVoice < SizeofStaticArray(preset.mVoiceSettings); ++nVoice)
        {
            auto &voiceState = log ? mVoiceStates[nVoice] : sentinel;
            // voiceState.mResult = "?";
            voiceState.mCurrentMidiNote = 0;
        }

        for (size_t nVoice = 0; nVoice < SizeofStaticArray(preset.mVoiceSettings); ++nVoice)
        {
            auto &hv = preset.mVoiceSettings[nVoice];
            auto &voiceState = log ? mVoiceStates[nVoice] : sentinel;

            // can we skip straight away?
            if (hv.mSequenceLength == 0)
            {
                // voiceState.mResult = "No seq";
                continue;
            }
            if (pout >= end)
            {
                // voiceState.mResult = "No output";
                return ret;
            }

            // is it a deduced voice? in other words, one that a scale follower
            // selects? we may need to filter it.
            bool deduced = false;
            Scale scale;

            switch (hv.mScaleRef)
            {
            case HarmScaleRefType::Voice:
                scale = hv.mLocalScale;
                break;
            case HarmScaleRefType::Preset:
                scale = preset.mPresetScale;
                break;
            case HarmScaleRefType::Global:
                deduced = globalDeduced;
                scale = globalScale;
                break;
            }

            bool wantDeduced = (voiceFilter == VoiceFilterOptions::OnlyDeducedVoices);
            if (wantDeduced != deduced)
            {
                // voiceState.mResult = deduced ? "is deduced" : "not deduced";
                continue;
            }

            *pout = *liveVoice; // copy from live voice to get started.
            pout->mIsNoteCurrentlyMuted = !perf.mHarmEnabled;
            pout->mVoiceId =
                MakeMusicalVoiceID(loopLayerID, HarmLayerToVoiceID((uint8_t)nVoice)); // +1 because live voice is id 0.
            pout->mGain *= perf.mHarmGain;

            pout->mPan += preset.mStereoSeparation * ((((int)nVoice & 1) * 2) - 1); // turns bit 0 to -1 or 1

            switch (hv.mPitchBendParticipation)
            {
            case PitchBendParticipation::Off:
                pout->mPitchBendN11.SetFloat(0);
                break;
            case PitchBendParticipation::Invert:
                pout->mPitchBendN11.SetFloat(-pout->mPitchBendN11.GetFloatVal());
                break;
            case PitchBendParticipation::Same:
            default:
                // already fine.
                break;
            }

            // old method:
            // auto newNote = scale.AdjustNoteByInterval(
            //     pout->mMidiNote, hv.mSequence[mSequencePos % hv.mSequenceLength], EnharmonicDirection::Sharp);
            // if (!newNote)
            // {
            //     voiceState.mResult = "oob mute";
            //     continue;
            // }

            // pout->mMidiNote = newNote;
            // voiceState.mCurrentMidiNote = pout->mMidiNote;
            // voiceState.mResult = String("note ") + pout->mMidiNote;

            // new method:
            auto newNote =
                GetHarmonizedNote(pout->mMidiNote, scale, perf.mDeducedScale, mSequencePos, preset, hv, voiceState);
            if (!newNote)
            {
                continue; // muted
            }
            newNote = EnsureHarmonizedNoteBounds(newNote, liveVoice->mMidiNote, hv);
            if (!newNote)
            {
                // voiceState.mResult = "oob mute";
                continue; // muted
            }
            pout->mMidiNote = newNote;
            voiceState.mCurrentMidiNote = newNote;
            // voiceState.mResult = String("note ") + newNote;

            if (pout->mVelocity == 0)
            {
                // voiceState.mResult = "vel 0";
                continue;
            }

            switch (hv.mSynthPresetRef)
            {
            case HarmSynthPresetRefType::GlobalA:
                pout->mSynthPatchA = perf.mSynthPresetA; //  ->mGlobalSynthPreset;
                break;
            case HarmSynthPresetRefType::GlobalB:
                pout->mSynthPatchA = perf.mSynthPresetB; //  ->mGlobalSynthPreset;
                break;
            case HarmSynthPresetRefType::Preset1:
                pout->mSynthPatchA = preset.mSynthPreset1;
                break;
            case HarmSynthPresetRefType::Preset2:
                pout->mSynthPatchA = preset.mSynthPreset2;
                break;
            case HarmSynthPresetRefType::Preset3:
                pout->mSynthPatchA = preset.mSynthPreset3;
                break;
            case HarmSynthPresetRefType::Preset4:
                pout->mSynthPatchA = preset.mSynthPreset4;
                break;
            case HarmSynthPresetRefType::Voice:
                pout->mSynthPatchA = hv.mVoiceSynthPreset;
                break;
            }

            ++pout;
            ++ret;
        } // for voice

        return ret;
    }
};

} // namespace clarinoid
