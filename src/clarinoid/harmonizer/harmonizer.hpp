
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

    static bool MidiNoteIsDiatonic(uint8_t midiNote, Scale &scale)
    {
        uint8_t root = 0;
        auto ctx = scale.GetNoteInScaleContext(midiNote, root, EnharmonicDirection::Sharp);
        if (ctx.mEnharmonic == 0)
        {
            return true;
        }
        ctx = scale.GetNoteInScaleContext(midiNote, root, EnharmonicDirection::Flat);
        return ctx.mEnharmonic == 0;
    }

    static uint8_t FindNextDiatonic(uint8_t midiNote, Scale &scale, int direction)
    {
        CCASSERT(direction != 0);
        int current = midiNote;
        while (true)
        {
            current += direction;
            if (current < 0)
            {
                NoteInScaleFlavorContext tempCtx;
                uint8_t tempRoot = 0;
                uint8_t tempMidi = 0;
                ResolveNearestDiatonic(0, scale, tempCtx, tempRoot, tempMidi);
                return tempMidi;
            }
            if (current > 127)
            {
                NoteInScaleFlavorContext tempCtx;
                uint8_t tempRoot = 0;
                uint8_t tempMidi = 0;
                ResolveNearestDiatonic(127, scale, tempCtx, tempRoot, tempMidi);
                return tempMidi;
            }
            if (MidiNoteIsDiatonic((uint8_t)current, scale))
            {
                return (uint8_t)current;
            }
        }
    }

    static void ResolveNearestDiatonic(uint8_t referenceNote,
                                       Scale &scale,
                                       NoteInScaleFlavorContext &ctxOut,
                                       uint8_t &rootOut,
                                       uint8_t &midiOut)
    {
        uint8_t rootSharp = 0;
        auto ctxSharp = scale.GetNoteInScaleContext(referenceNote, rootSharp, EnharmonicDirection::Sharp);
        auto candSharp = ctxSharp;
        candSharp.mEnharmonic = 0;
        uint8_t midiSharp = scale.GetMidiNoteFromContext(candSharp, rootSharp);
        int distSharp = std::abs(int(midiSharp) - int(referenceNote));

        uint8_t rootFlat = 0;
        auto ctxFlat = scale.GetNoteInScaleContext(referenceNote, rootFlat, EnharmonicDirection::Flat);
        auto candFlat = ctxFlat;
        candFlat.mEnharmonic = 0;
        uint8_t midiFlat = scale.GetMidiNoteFromContext(candFlat, rootFlat);
        int distFlat = std::abs(int(midiFlat) - int(referenceNote));

        if (distFlat < distSharp)
        {
            ctxOut = candFlat;
            rootOut = rootFlat;
            midiOut = midiFlat;
        }
        else
        {
            ctxOut = candSharp;
            rootOut = rootSharp;
            midiOut = midiSharp;
        }
    }

    static bool ResolveDiatonicAnchor(uint8_t inputNote,
                                      Scale &scale,
                                      Scale &deducedScale,
                                      const HarmVoiceSettings &voiceSetting,
                                      NoteInScaleFlavorContext &ctxOut,
                                      uint8_t &rootOut,
                                      uint8_t &midiOut)
    {
        if (MidiNoteIsDiatonic(inputNote, scale))
        {
            ResolveNearestDiatonic(inputNote, scale, ctxOut, rootOut, midiOut);
            return true;
        }

        switch (voiceSetting.mNonDiatonicBehavior)
        {
        case NonDiatonicBehavior::Drop:
            return false;
        case NonDiatonicBehavior::ChromaticUp: {
            uint8_t next = FindNextDiatonic(inputNote, scale, +1);
            ResolveNearestDiatonic(next, scale, ctxOut, rootOut, midiOut);
            return true;
        }
        case NonDiatonicBehavior::ChromaticDown: {
            uint8_t prev = FindNextDiatonic(inputNote, scale, -1);
            ResolveNearestDiatonic(prev, scale, ctxOut, rootOut, midiOut);
            return true;
        }
        case NonDiatonicBehavior::UseScaleFollower: {
            uint8_t followerRoot = 0;
            auto followerCtx = deducedScale.GetNoteInScaleContext(inputNote, followerRoot, EnharmonicDirection::Sharp);
            uint8_t followerMidi = deducedScale.GetMidiNoteFromContext(followerCtx, followerRoot);
            ResolveNearestDiatonic(followerMidi, scale, ctxOut, rootOut, midiOut);
            return true;
        }
        default:
            break;
        }

        ResolveNearestDiatonic(inputNote, scale, ctxOut, rootOut, midiOut);
        return true;
    }

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
        NoteInScaleFlavorContext anchorCtx;
        uint8_t anchorRoot = 0;
        uint8_t anchorMidi = 0;
        if (!ResolveDiatonicAnchor(inputNote, scale, deducedScale, voiceSetting, anchorCtx, anchorRoot, anchorMidi))
        {
            voiceState.mCurrentMidiNote = 0;
            return 0;
        }

        int8_t intervalValue = voiceSetting.mSequence[sequenceIndex];
        uint8_t ret = 0;

        if (voiceSetting.mIntervalMode == HarmVoiceIntervalMode::ScaleDegrees)
        {
            auto ctx = anchorCtx;
            ctx.mScaleDegree += intervalValue;
            ret = scale.GetMidiNoteFromContext(ctx, anchorRoot);
        }
        else
        {
            int target = int(anchorMidi) + int(intervalValue);
            if (target < 0)
            {
                target = 0;
            }
            else if (target > 127)
            {
                target = 127;
            }
            ret = (uint8_t)target;

            switch (voiceSetting.mIntervalMode)
            {
            case HarmVoiceIntervalMode::Chromatic:
                break;
            case HarmVoiceIntervalMode::ChromaticOOSMute:
                if (!MidiNoteIsDiatonic(ret, scale))
                {
                    voiceState.mCurrentMidiNote = 0;
                    return 0;
                }
                break;
            case HarmVoiceIntervalMode::ChromaticDown:
                if (!MidiNoteIsDiatonic(ret, scale))
                {
                    ret = FindNextDiatonic(ret, scale, -1);
                }
                break;
            case HarmVoiceIntervalMode::ChromaticUp:
                if (!MidiNoteIsDiatonic(ret, scale))
                {
                    ret = FindNextDiatonic(ret, scale, +1);
                }
                break;
            default:
                break;
            }
        }

        voiceState.mCurrentMidiNote = ret;
        return ret;
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
        liveVoice->mTransposeSemis = perf.mSynthATranspose;
        liveVoice->mVoiceSource = VoiceSource::Live;
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
            liveVoice->mTransposeSemis += perf.mDetuneSemis;
            *pout = *liveVoice; // copy from live voice to get started.
            pout->mTransposeSemis = perf.mSynthBTranspose - perf.mDetuneSemis;
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
            pout->mVoiceSource = VoiceSource::Harmonizer;
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

            auto newNote =
                GetHarmonizedNote(pout->mMidiNote, scale, perf.mDeducedScale, mSequencePos, preset, hv, voiceState);
            if (!newNote)
            {
                continue; // muted
            }
            newNote += hv.mOctaveTranspose * 12;
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
