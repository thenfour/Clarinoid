
#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/settings/AppSettings.hpp>

namespace clarinoid
{

// with original clarinoid, harmonizing all voices was done in 1 function call.
// but calculating them by layer is necessary:
// - to help match the voice tracking performed by HarmonizerDeviceLayer
// - in the case where a layer is monophonic, and others are polyphonic, triller behavior requires calculation of
// individual layers.
struct HarmonizerLayer
{
    AppSettings *mpAppSettings = nullptr;
    size_t mLayerID = 0xff;

    size_t mSequencePos = 0;
    Stopwatch mRotationTriggerTimer;

    void Init(AppSettings &appSettings, size_t layerID)
    {
        mpAppSettings = &appSettings;
        mLayerID = layerID;
    }

    enum class HarmonizeFlags
    {
        None = 0,
        BecauseOfNoteOn = 1,
    };

    // modelvoice could be nullptr for note ons.
    MusicalVoice GetHarmonizedNote(
        const HarmPatch &harmPatch,
        const HeldNoteInfo &liveNote,
        const MusicalVoice &modelVoice, // required to know things like paramprovider, source, noteID for non-noteons
        HarmonizeFlags opt)
    {
        CCASSERT(!!mpAppSettings);
        auto &perf = mpAppSettings->GetCurrentPerformancePatch();
        auto &harmLayer = harmPatch.mVoiceSettings[mLayerID];

        // create a new note.
        MusicalVoice ret = modelVoice;
        ret.mIsActive = false; // guilty until proven innocent.
        ret.mHarmonizerSourceNoteID = liveNote.mLiveNoteSequenceID;
        ret.mSource.mHarmonizerVoiceIndex = mLayerID;
        ret.mNoteInfo = liveNote;

        bool noteOn = HasFlag(opt, HarmonizeFlags::BecauseOfNoteOn);
        bool log = false;

        if (noteOn && log)
        {
            Serial.println(String("harmonizing note on layer:") + mLayerID +
                           ", id:" + ret.mNoteInfo.mLiveNoteSequenceID);
        }

        if (harmLayer.mSequenceLength == 0)
        {
            if (noteOn && log)
            {
                Serial.println(String(" -> seq len 0"));
            }
            return ret;
        }
        if (ret.mNoteInfo.mVelocity01 <= gMinMidiVelocity01)
        {
            if (noteOn && log)
            {
                Serial.println(String(" -> velocity 0"));
            }
            return ret;
        }

        Scale scale;

        switch (harmLayer.mScaleRef.GetValue())
        {
        case HarmScaleRefType::Voice:
            scale = harmLayer.mLocalScale.mValue;
            break;
        case HarmScaleRefType::Preset:
            scale = harmPatch.mPatchScale.mValue;
            break;
        case HarmScaleRefType::Global: {
            bool globalDeduced = perf.mGlobalScaleRef.GetValue() == GlobalScaleRefType::Deduced;
            Scale globalScale = globalDeduced ? perf.mDeducedScale.mValue : perf.mGlobalScale.mValue;
            scale = globalScale;
            break;
        }
        }

        // todo: voice gain, panning, spreading
        // todo: use hv.mNonDiatonicBehavior
        auto newNote = scale.AdjustNoteByInterval(ret.mNoteInfo.mMidiNote.GetMidiValue(),
                                                  harmLayer.mSequence[mSequencePos % harmLayer.mSequenceLength],
                                                  EnharmonicDirection::Sharp);
        if (!newNote)
        {
            if (noteOn && log)
            {
                Serial.println(String(" -> note 0"));
            }
            return ret;
        }

        ret.mNoteInfo.mMidiNote = newNote;

        // corrective settings...
        switch (harmLayer.mNoteOOBBehavior.GetValue())
        {
        case NoteOOBBehavior::TransposeOctave:
            while (ret.mNoteInfo.mMidiNote < harmLayer.mMinOutpNote)
                ret.mNoteInfo.mMidiNote += 12;
            while (ret.mNoteInfo.mMidiNote > harmLayer.mMaxOutpNote)
                ret.mNoteInfo.mMidiNote -= 12;
            break;
        case NoteOOBBehavior::Mute:
            break;
        }

        if ((ret.mNoteInfo.mMidiNote < harmLayer.mMinOutpNote) ||
            (ret.mNoteInfo.mMidiNote > harmLayer.mMaxOutpNote))
        {
            if (noteOn && log)
            {
                Serial.println(String(" -> note value out of range: ") + ret.mNoteInfo.mMidiNote.GetMidiValue());
            }
            return ret;
        }

        ret.mpSynthPatch = &mpAppSettings->GetSynthPresetForHarmonizerLayer(harmPatch, mLayerID);
        ret.mIsActive = true;
        if (noteOn && log)
        {
            Serial.println(String(" -> SUCCESS: ") + ret.mNoteInfo.mMidiNote.GetNoteDesc().mName);
        }

        if (noteOn)
        {
            // create a new note.
            ret.mNoteInfo.mLiveNoteSequenceID = GetNextLiveNoteSequenceID();
            ret.mNoteInfo.mRandomTrigger01 = prng_f01();
            if (mRotationTriggerTimer.ElapsedTime().ElapsedMillisI() >= harmPatch.mMinRotationTimeMS.GetValue())
            {
                mRotationTriggerTimer.Restart();
                mSequencePos++;
            }
        }
        else
        {
            ret.mNoteInfo.mLiveNoteSequenceID = modelVoice.mNoteInfo.mLiveNoteSequenceID;
            ret.mNoteInfo.mRandomTrigger01 = modelVoice.mNoteInfo.mRandomTrigger01;
        }

        return ret;
    }
};

} // namespace clarinoid
