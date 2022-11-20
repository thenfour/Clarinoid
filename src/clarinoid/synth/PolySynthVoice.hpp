
#pragma once

#ifdef CLARINOID_MODULE_TEST
#error not for x86 tests
#endif

#include <cfloat>

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/synth/EnvelopeNode.hpp>

#include "FilterNode.hpp"
#include "PannerNode.hpp"
#include "MultiMixer.hpp"
#include "ModulationMatrixNode.hpp"
#include "Patch.hpp"
#include "polyBlepOscillator.hpp"

namespace clarinoid
{

struct Voice : IModulationProvider
{
    AudioBandlimitedOsci mOsc;

    // Modulation sources
    VoiceModulationMatrixNode mModMatrix;
    EnvelopeNode mEnvelopes[ENVELOPE_COUNT];
    AudioSynthWaveform mLfos[LFO_COUNT];

    // have to track these because they're private members of AudioWaveform.
    short mLfoWaveshapes[LFO_COUNT] = {0xff, 0xff, 0xff}; // invalid so 1st run will always set the shape.

    virtual std::pair<AudioStream *, size_t> IModulationProvider_GetARateSourcePort(ARateModulationSource src) override
    {
        switch (src)
        {
        case ARateModulationSource::ENV1:
            return {&mEnvelopes[0], 0};
        case ARateModulationSource::ENV2:
            return {&mEnvelopes[1], 0};
        case ARateModulationSource::ENV3:
            return {&mEnvelopes[2], 0};
        case ARateModulationSource::LFO1:
            return {&mLfos[0], 0};
        case ARateModulationSource::LFO2:
            return {&mLfos[1], 0};
        case ARateModulationSource::LFO3:
            return {&mLfos[2], 0};
        // case ARateModulationSource::Osc1FB:
        //     return {&mOsc, 0};
        // case ARateModulationSource::Osc2FB:
        //     return {&mOsc, 1};
        // case ARateModulationSource::Osc3FB:
        //     return {&mOsc, 2};
        }
        CCASSERT(!"arate src port incorrect"); // probably enum set up incorrectly.
        return {nullptr, 0};
    }

    virtual std::pair<AudioStream *, size_t> IModulationProvider_GetARateDestinationPort(
        ARateModulationDestination dest) override
    {
        switch (dest)
        {
        case ARateModulationDestination::Osc1PulseWidth:
            return {&mOsc, (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::pwm1};
        case ARateModulationDestination::Osc2PulseWidth:
            return {&mOsc, (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::pwm2};
        case ARateModulationDestination::Osc3PulseWidth:
            return {&mOsc, (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::pwm3};
        // case ARateModulationDestination::Osc1Phase:
        //     return {&mOsc, (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::pm1};
        // case ARateModulationDestination::Osc2Phase:
        //     return {&mOsc, (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::pm2};
        // case ARateModulationDestination::Osc3Phase:
        //     return {&mOsc, (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::pm3};
        }
        CCASSERT(!"arate dest port incorrect"); // probably enum set up incorrectly.
        return {nullptr, 0};
    }

    // ...
    MultiMixerPannerNode<3> mOscMixerPanner;

    CCPatch mPatchOsc1ToMixer = {mOsc, 0, mOscMixerPanner, 0};
    CCPatch mPatchOsc2ToMixer = {mOsc, 1, mOscMixerPanner, 1};
    CCPatch mPatchOsc3ToMixer = {mOsc, 2, mOscMixerPanner, 2};

    ::clarinoid::StereoFilterNode mFilter;
    CCPatch mPatchMixToFilterL = {mOscMixerPanner, 0, mFilter, 0};
    CCPatch mPatchMixToFilterR = {mOscMixerPanner, 1, mFilter, 1};

    ::clarinoid::StereoGainerSplitterNode<3> mSplitter; // handles panning and send levels to delay/verb/dry
    CCPatch mPatchFilterToPannerSplitterL = {mFilter, 0, mSplitter, 0};
    CCPatch mPatchFilterToPannerSplitterR = {mFilter, 1, mSplitter, 1};

    CCPatch mPatchOutDryLeft;
    CCPatch mPatchOutDryRight;

    CCPatch mPatchOutDelayLeft;
    CCPatch mPatchOutDelayRight;

    CCPatch mPatchOutVerbLeft;
    CCPatch mPatchOutVerbRight;

    int mVoiceIndex;
    MusicalVoice mRunningVoice;       // latest running voice data.
    uint32_t mReleaseTimestampMS = 0; // 0 means the note is still active.
    AppSettings *mAppSettings;
    MusicalState *mpMusicalState;

    void EnsurePatchConnections(AppSettings *appSettings, MusicalState *pMS)
    {
        mAppSettings = appSettings;
        mpMusicalState = pMS;

        mPatchOsc1ToMixer.connect();
        mPatchOsc2ToMixer.connect();
        mPatchOsc3ToMixer.connect();
        mPatchMixToFilterL.connect();
        mPatchMixToFilterR.connect();
        mPatchFilterToPannerSplitterL.connect();
        mPatchFilterToPannerSplitterR.connect();
        mPatchOutDryLeft.connect();
        mPatchOutDryRight.connect();
        mPatchOutDelayLeft.connect();
        mPatchOutDelayRight.connect();
        mPatchOutVerbLeft.connect();
        mPatchOutVerbRight.connect();

        mModMatrix.Init(this);
    }

    virtual float IModulationProvider_GetKRateModulationSourceValueN11(KRateModulationSource src) override
    {
        switch (src)
        {
        case KRateModulationSource::Breath:
            return mRunningVoice.mpParamProvider->SynthParamProvider_GetBreath01();
        case KRateModulationSource::PitchStrip:
            return mRunningVoice.mpParamProvider->SynthParamProvider_GetPitchBendN11();
        case KRateModulationSource::Velocity:
            return mRunningVoice.mNoteInfo.mVelocity01;
        case KRateModulationSource::NoteValue:
            return float(mRunningVoice.mNoteInfo.mMidiNote.GetMidiValue()) / 127;
        case KRateModulationSource::RandomTrigger:
            return mRunningVoice.mNoteInfo.mRandomTrigger01;
        case KRateModulationSource::ModWheel:
            return float(mRunningVoice.mpParamProvider->SynthParamProvider_GetMidiCC(MidiCCValue::ModWheel)) / 127;
        case KRateModulationSource::Macro1:
            return mRunningVoice.mpParamProvider->SynthParamProvider_GetMacroValue01(0);
        case KRateModulationSource::Macro2:
            return mRunningVoice.mpParamProvider->SynthParamProvider_GetMacroValue01(1);
        case KRateModulationSource::Macro3:
            return mRunningVoice.mpParamProvider->SynthParamProvider_GetMacroValue01(2);
        case KRateModulationSource::Macro4:
            return mRunningVoice.mpParamProvider->SynthParamProvider_GetMacroValue01(3);
        case KRateModulationSource::Pedal:
            return float(mRunningVoice.mpParamProvider->SynthParamProvider_GetMidiCC(MidiCCValue::DamperPedal)) / 127;
        }
        CCASSERT(!"requesting an unsupported krate source");
        return 0;
    }

    // these will not track FREQUENCY, but rather MIDI NOTE. this lets us apply portamento to certain parameters but not
    // others. it also may have the benefit of feeling more linear.
    PortamentoCalc mPortamentoCalc[3];

    short convertWaveType(OscWaveformShape s)
    {
        switch (s)
        {
        case OscWaveformShape::Pulse:
            return WAVEFORM_PULSE;
        case OscWaveformShape::SawSync:
            return WAVEFORM_SAWTOOTH;
        case OscWaveformShape::VarTriangle:
            return WAVEFORM_TRIANGLE;
        case OscWaveformShape::Sine:
            break;
        }
        return WAVEFORM_SINE;
    };

    float CalcFreq(const SynthOscillatorSettings &osc, float detune, float krateFreqModN11, PortamentoCalc &portamento)
    {
        // we're in semis land... let's figure out the semitone.
        float midiNote = mRunningVoice.mNoteInfo.mMidiNote.GetMidiValue();

        float userPB = mRunningVoice.mpParamProvider->SynthParamProvider_GetPitchBendN11();
        float pbSemis = userPB * ((userPB > 0) ? osc.mPitchBendRangePositive.GetValue()
                                               : (-osc.mPitchBendRangeNegative.GetValue()));

        float paramSemis = osc.mFreqParam.GetMidiNote(midiNote, krateFreqModN11);

        float idealSemis = paramSemis + osc.mPitchFine.GetValue() + osc.mPitchSemis.GetValue() + detune;

        float retSemis = portamento.KStep(idealSemis, osc.mPortamentoTimeMS.GetValue());
        retSemis += pbSemis; // Pitchbend needs to be after portamento.

        float retHz = MIDINoteToFreq(retSemis);
        retHz *= osc.mFreqMultiplier.GetValue();
        retHz += osc.mFreqOffsetHz.GetValue();

        return Clamp(retHz, 0.0f, 22050.0f);
    };

    void Update()
    {
        auto newRunningVoice = mpMusicalState->GetLiveMusicalVoice(this->mRunningVoice);
        const auto &perf = this->mAppSettings->GetCurrentPerformancePatch();

        if (newRunningVoice.mIsActive)
        {
            this->mRunningVoice = newRunningVoice;
        }
        else
        {
            // not active; we cannot use newRunningVoice

            // we need to distinguish between:
            // 1- the synth params changing and the rug getting pulled from underneath,
            // 2- the note has been released and we should continue releasing it.
            // we use our envelope state to determine this. musicalstate forgets about notes
            // as soon as they're released.
            mRunningVoice.mNoteInfo.mIsPhysicallyHeld = false; // important for releaseability calc
            if (mEnvelopes[0].GetStage() != EnvelopeStage::Release)
            {
                // in release stage, we expect mIsActive to be false, and we continue with our existing
                // running voice until it's done.
                // otherwise it's over; bail.
                mOsc.mIsPlaying = false;
                return;
            }
        }

        const auto &patch = *mRunningVoice.mpSynthPatch;
        // const auto &perf = *mRunningVoice.mpPerf;

        float externalGain = 1.0f;
        switch (mRunningVoice.mSource.mType)
        {
        case MusicalEventSourceType::Harmonizer:

            // todo. because of loopstation we cannot just use the performance harmonizer patch.
            // probably need to add harm patch to musicalvoice just like synthpatch.

            // TODO: stereo sep of perf harm
            // TODO: stereo sep of harm voice
            // if (!perf.mHarmEnabled)
            // {
            //     mOsc.mIsPlaying = false;
            //     return;
            // }
            // mOsc.mIsPlaying = true;
            // externalGain = perf.mHarmGain;
            // individual harmonizer voices don't have their own volume (yet?)
            // externalGain =
            // mAppSettings->FindHarmPreset(perf.mHarmPreset).mVoiceSettings[mRunningVoice.mSource.mHarmonizerVoiceIndex].
            break;
        case MusicalEventSourceType::LivePlayA:
            // TODO: stereo sep of perf patches
            if (!perf.mSynthAEnabled.GetValue())
            {
                mOsc.mIsPlaying = false;
                return;
            }
            mOsc.mIsPlaying = true;
            externalGain = perf.mSynthAGain.GetValue();
            break;
        case MusicalEventSourceType::LivePlayB:
            // TODO: stereo sep of perf patches
            if (!perf.mSynthBEnabled.GetValue())
            {
                mOsc.mIsPlaying = false;
                return;
            }
            mOsc.mIsPlaying = true;
            externalGain = perf.mSynthBGain.GetValue();
            break;
        case MusicalEventSourceType::Loopstation:
            // TODO... how does this work?
            break;
        default:
            CCASSERT(!"unknown event source");
            break;
        }

        // apply ongoing params
        for (size_t i = 0; i < ENVELOPE_COUNT; ++i)
        {
            EnvelopeModulationValues modValues;
            auto &e = gModValuesByEnvelope[i];
            modValues.delayTime = mModMatrix.GetKRateDestinationValue(e.KRateDestination_DelayTime);
            modValues.attackTime = mModMatrix.GetKRateDestinationValue(e.KRateDestination_AttackTime);
            modValues.attackCurve = mModMatrix.GetKRateDestinationValue(e.KRateDestination_AttackCurve);
            modValues.holdTime = mModMatrix.GetKRateDestinationValue(e.KRateDestination_HoldTime);
            modValues.decayTime = mModMatrix.GetKRateDestinationValue(e.KRateDestination_DecayTime);
            modValues.decayCurve = mModMatrix.GetKRateDestinationValue(e.KRateDestination_DecayCurve);
            modValues.sustainLevel = mModMatrix.GetKRateDestinationValue(e.KRateDestination_SustainLevel);
            modValues.releaseTime = mModMatrix.GetKRateDestinationValue(e.KRateDestination_ReleaseTime);
            modValues.releaseCurve = mModMatrix.GetKRateDestinationValue(e.KRateDestination_ReleaseCurve);
            mEnvelopes[i].SetSpec(patch.mEnvelopes[i], modValues);
        }

        for (size_t i = 0; i < LFO_COUNT; ++i)
        {
            short wantsWaveType = convertWaveType(patch.mLFOs[i].mWaveShape.GetValue());
            if (mLfoWaveshapes[i] != wantsWaveType)
            {
                mLfos[i].begin(wantsWaveType);
                mLfos[i].amplitude(1.0f);
            }

            mLfos[i].frequency(patch.mLFOs[i].mSpeed.ToHertz(perf.mBPM.GetValue()));
        }

        // figure out which oscillators are enabled. Get a count and grab enabled indices.
        int oscEnabledCount = 0;
        // size_t enabledOscIndices[POLYBLEP_OSC_COUNT];
        for (size_t iosc = 0; iosc < POLYBLEP_OSC_COUNT; ++iosc)
        {
            if (!patch.mOsc[iosc].mEnabled.GetValue())
                continue;
            // enabledOscIndices[oscEnabledCount] = iosc;
            oscEnabledCount++;
        }

        float detune =
            patch.mDetune.GetValue() + mModMatrix.GetKRateDestinationValue(KRateModulationDestination::Detune);
        float spread = patch.mStereoSpread.GetValue();

        float detunes[POLYBLEP_OSC_COUNT] = {0};
        float spreads[POLYBLEP_OSC_COUNT] = {0};
        switch (oscEnabledCount)
        {
        case 0:
        case 1:
            break;
        case 2:
            detunes[0] = detune;
            detunes[1] = -detune;
            spreads[0] = spread;
            spreads[1] = -spread;
            break;
        case 3:
            detunes[0] = 0;
            detunes[1] = detune;
            detunes[2] = -detune;
            spreads[0] = spread * 0.2f; // when spread is != 0, I feel that there should be NO voices in the center. we
                                        // rotate around somehow.
            spreads[1] = spread * 0.8f;
            spreads[2] = -spread;
            break;
        default:
            CCASSERT(!"hardcoded osc count alert");
        }

        // apply oscillator params
        mOsc.mIsPlaying = oscEnabledCount > 0;

        size_t ienabledOsc = 0;

        mOsc.mOsc[0].mPMAmt0 = // 1->1
            patch.mFMStrength1To1.GetValue() +
            mModMatrix.GetKRateDestinationValue(KRateModulationDestination::FMStrength1To1);
        mOsc.mOsc[0].mPMAmt1 = // 2->1
            patch.mFMStrength2To1.GetValue() +
            mModMatrix.GetKRateDestinationValue(KRateModulationDestination::FMStrength2To1);
        mOsc.mOsc[0].mPMAmt2 = // 3->1
            patch.mFMStrength3To1.GetValue() +
            mModMatrix.GetKRateDestinationValue(KRateModulationDestination::FMStrength3To1);

        mOsc.mOsc[1].mPMAmt0 = // 1->2
            patch.mFMStrength1To2.GetValue() +
            mModMatrix.GetKRateDestinationValue(KRateModulationDestination::FMStrength1To2);
        mOsc.mOsc[1].mPMAmt1 = // 2->2
            patch.mFMStrength2To2.GetValue() +
            mModMatrix.GetKRateDestinationValue(KRateModulationDestination::FMStrength2To2);
        mOsc.mOsc[1].mPMAmt2 = // 3->2
            patch.mFMStrength3To2.GetValue() +
            mModMatrix.GetKRateDestinationValue(KRateModulationDestination::FMStrength3To2);

        mOsc.mOsc[2].mPMAmt0 = // 1->3
            patch.mFMStrength1To3.GetValue() +
            mModMatrix.GetKRateDestinationValue(KRateModulationDestination::FMStrength1To3);
        mOsc.mOsc[2].mPMAmt1 = // 2->3
            patch.mFMStrength2To3.GetValue() +
            mModMatrix.GetKRateDestinationValue(KRateModulationDestination::FMStrength2To3);
        mOsc.mOsc[2].mPMAmt2 = // 3->3
            patch.mFMStrength3To3.GetValue() +
            mModMatrix.GetKRateDestinationValue(KRateModulationDestination::FMStrength3To3);


        mOsc.mOsc[0].mRMAmt0 = // 1->1
            patch.mRingmodStrength1To2.GetValue() +
            mModMatrix.GetKRateDestinationValue(KRateModulationDestination::RingmodStrength1To1);
        mOsc.mOsc[0].mRMAmt1 = // 2->1
            patch.mRingmodStrength2To1.GetValue() +
            mModMatrix.GetKRateDestinationValue(KRateModulationDestination::RingmodStrength2To1);
        mOsc.mOsc[0].mRMAmt2 = // 3->1
            patch.mRingmodStrength3To1.GetValue() +
            mModMatrix.GetKRateDestinationValue(KRateModulationDestination::RingmodStrength3To1);

        mOsc.mOsc[1].mRMAmt0 = // 1->2
            patch.mRingmodStrength1To2.GetValue() +
            mModMatrix.GetKRateDestinationValue(KRateModulationDestination::RingmodStrength1To2);
        mOsc.mOsc[1].mRMAmt1 = // 2->2
            patch.mRingmodStrength2To2.GetValue() +
            mModMatrix.GetKRateDestinationValue(KRateModulationDestination::RingmodStrength2To2);
        mOsc.mOsc[1].mRMAmt2 = // 3->2
            patch.mRingmodStrength3To2.GetValue() +
            mModMatrix.GetKRateDestinationValue(KRateModulationDestination::RingmodStrength3To2);

        mOsc.mOsc[2].mRMAmt0 = // 1->3
            patch.mRingmodStrength1To3.GetValue() +
            mModMatrix.GetKRateDestinationValue(KRateModulationDestination::RingmodStrength1To3);
        mOsc.mOsc[2].mRMAmt1 = // 2->3
            patch.mRingmodStrength2To3.GetValue() +
            mModMatrix.GetKRateDestinationValue(KRateModulationDestination::RingmodStrength2To3);
        mOsc.mOsc[2].mRMAmt2 = // 3->3
            patch.mRingmodStrength3To3.GetValue() +
            mModMatrix.GetKRateDestinationValue(KRateModulationDestination::RingmodStrength3To3);


        for (size_t i = 0; i < POLYBLEP_OSC_COUNT; ++i)
        {
            mOsc.mOsc[i].mEnabled = patch.mOsc[i].mEnabled.GetValue();

            mOscMixerPanner.SetInputPanGainAndEnabled(
                i,
                patch.mPan.GetValue() + patch.mOsc[i].mPan.GetValue() + spreads[ienabledOsc],
                patch.mOsc[i]
                    .mVolume
                    .AddParam(mModMatrix.GetKRateDestinationValue(gModValuesByOscillator[i].KRateDestination_Volume))
                    .ToLinearGain(),
                true);

            if (!patch.mOsc[i].mEnabled.GetValue())
                continue;

            mOsc.mOsc[i].pulseWidth(patch.mOsc[i].mPulseWidth.GetValue());

            float freq =
                CalcFreq(patch.mOsc[i],
                         detunes[ienabledOsc],
                         mModMatrix.GetKRateDestinationValue(gModValuesByOscillator[i].KRateDestination_Frequency),
                         mPortamentoCalc[i]);

            // sd += String("[o") + i + " det:" + detunes[ienabledOsc] + " freq:" + freq + "]  ";

            mOsc.mOsc[i].SetBasicParams(
                patch.mOsc[i].mWaveform.GetValue(),
                freq,
                patch.mOsc[i].mPhase01.GetValue(),
                patch.mOsc[i].mPortamentoTimeMS.GetValue(),
                patch.mOsc[i].mHardSyncEnabled.GetValue(),
                patch.mOsc[i].mSyncFreqParam.GetFrequency(
                    mRunningVoice.mNoteInfo.mMidiNote.GetMidiValue(),
                    mModMatrix.GetKRateDestinationValue(gModValuesByOscillator[i].KRateDestination_SyncFrequency)),
                patch.mOverallRingmodStrength.GetValue() + mModMatrix.GetKRateDestinationValue(KRateModulationDestination::OverallRingmodStrength),
                patch.mOverallFMStrength.GetValue() + mModMatrix.GetKRateDestinationValue(KRateModulationDestination::OverallFMStrength)
                  );

            ienabledOsc++;
        }

        // if (mVoiceIndex == 0)
        //     Serial.println(sd);

        // TODO: if portamento is enabled for an oscillator, it should be accounted for here.
        float filterFreq = patch.mFilter.mFrequency.GetFrequency(
            MIDINoteToFreq(mRunningVoice.mNoteInfo.mMidiNote.GetMidiValue()),
            mModMatrix.GetKRateDestinationValue(KRateModulationDestination::FilterCutoff));
        mFilter.SetParams(patch.mFilter.mType.GetValue(),
                          filterFreq,
                          patch.mFilter.mQ.GetValue(),
                          patch.mFilter.mSaturation.GetValue());

        mFilter.EnableDCFilter(false, 10.0f);
        // mFilter.EnableDCFilter(patch.mDCFilterEnabled.GetValue(), patch.mDCFilterCutoff.GetValue());

        // for "mix" behavior, we can look to panning. the pan law applies here; you're effectively panning between the
        // dry signal and the various effects. But instead of a -1 to +1 range, it's going to be 0-1.
        auto verbGains = CalculatePanGain(patch.mDelayMix.GetValue() * 2 - 1);
        auto delayGains = CalculatePanGain(patch.mVerbMix.GetValue() * 2 - 1);

        float masterGain =
            patch.mMasterVolume.AddParam(mModMatrix.GetKRateDestinationValue(KRateModulationDestination::MasterVolume))
                .ToLinearGain();
        masterGain = std::max(0.0f, masterGain * externalGain);

        mSplitter.SetOutputGain(0, masterGain * std::get<0>(verbGains) * std::get<0>(delayGains));
        mSplitter.SetOutputGain(1, masterGain * std::get<1>(delayGains));
        mSplitter.SetOutputGain(2, masterGain * std::get<1>(verbGains));
    }

    // if there's already a note playing, it gets cut off
    void IncomingMusicalEvents_OnNoteOn(const MusicalVoice &v)
    {
        CCASSERT(v.mIsActive);

        bool isLegato = (mRunningVoice.mSource.Equals(v.mSource)) && (mReleaseTimestampMS == 0) &&
                        (mRunningVoice.mIsActive) &&
                        (v.mpSynthPatch->mVoicingMode.GetValue() == VoicingMode::Monophonic);

        // adjust running voice.
        mRunningVoice = v;
        // mRunningVoice.mSource = source;
        // mRunningVoice.mNoteInfo = noteInfo;
        mReleaseTimestampMS = 0; // important so we know the note is playing

        mModMatrix.SetSynthPatch(mRunningVoice.mpSynthPatch, this);

        for (size_t i = 0; i < POLYBLEP_OSC_COUNT; ++i)
        {
            if (mRunningVoice.mpSynthPatch->mOsc[i].mPhaseRestart.GetValue())
            {
                mOsc.mOsc[i].ResetPhase();
            }
        }

        for (size_t i = 0; i < ENVELOPE_COUNT; ++i)
        {
            if (!isLegato || (isLegato && mRunningVoice.mpSynthPatch->mEnvelopes[i].mLegatoRestart.GetValue()))
            {
                mEnvelopes[i].noteOn();
            }
        }
    }

    void IncomingMusicalEvents_OnNoteOff()
    {
        mReleaseTimestampMS = millis();
        // if (this->mVoiceIndex == 0) {
        // Serial.println(String("[") + mVoiceIndex + "] voice env.NoteOff. " +
        // mRunningVoice.mNoteInfo.mMidiNote.GetNoteDesc().mName);
        //}
        // even after a note off, the note continues to play through the envelope release stage. so don't really change
        // much here.
        for (auto &e : mEnvelopes)
        {
            e.noteOff();
        }
    }

    void IncomingMusicalEvents_OnNoteKill()
    {
        // Serial.println(String("killing voice ") + this->ToString());
        //  for when a hard stop should be done; skip release phase.
        for (auto &e : mEnvelopes)
        {
            e.AdvanceToStage(EnvelopeStage::Idle);
        }
        mRunningVoice = {};
    }

    // TODO: 3.x: note hasn't quite gotten off the ground
    // - 0.x: playing before release
    // - 1.x: after release
    // - 2.x: env is silent
    float CalcReleaseabilityForEnvelope(const EnvelopeNode &env) const
    {
        // silent envelope
        if (IsSilentGain(env.GetLastOutputLevel()))
        {
            return 2.0f + TimeTo01(millis() - mRunningVoice.mNoteInfo.mAttackTimestampMS);
        }

        // if playing, before any release, then 0-1
        if (mReleaseTimestampMS == 0)
        {
            return TimeTo01(millis() - mRunningVoice.mNoteInfo.mAttackTimestampMS);
        }

        // ok we're at least at release stage.
        auto timeSinceReleaseMS = millis() - mReleaseTimestampMS;
        return 1.0f + TimeTo01(timeSinceReleaseMS);
    }

    // a number explaining how happy this voice is to be released / note-off'd.
    // in order of prio, factors:
    // - 20 never played
    // - 10+x playing physically
    // - 00+x playing non-physically (sustain pedal)
    //
    // below that, calculate value based on envelope position, and add them together:
    // - 0.x: playing before release
    // - 1.x: after release
    // - 2.x: after idle
    // that means each envelope gives 0-3; use the min.
    float GetReleaseability() const
    {
        if (mRunningVoice.mSource.mType == MusicalEventSourceType::Null)
        {
            return 20;
        }

        float r = CalcReleaseabilityForEnvelope(mEnvelopes[0]);

        return r + ((mRunningVoice.mNoteInfo.mIsPhysicallyHeld) ? 0 : 10);
    }

    // only for display purposes
    bool IsConsideredPlaying_ForDisplay() const
    {
        if (mRunningVoice.mSource.mType == MusicalEventSourceType::Null)
        {
            return false;
        }

        if (mReleaseTimestampMS == 0)
        {
            return true;
        }

        return mEnvelopes[0].IsPlaying();
    }

    String ToString() const
    {
        // idle
        // p1 C#9 idle
        auto ret = String("v") + mVoiceIndex + " r" + GetReleaseability() + "> ";
        if (!mRunningVoice.mIsActive)
        {
            ret += "<idle>";
            return ret;
        }
        // synth patch exists.
        ret += String() // + String("p") + mRunningVoice.mSynthPatchIndex + " "
               + mRunningVoice.mNoteInfo.mMidiNote.ToString() + " " +
               gEnvelopeStageInfo.GetValueDisplayName(mEnvelopes[0].GetStage());
        return ret;
    }

    Voice(int16_t vid)
        : mPatchOutDryLeft(mSplitter, 0, gpSynthGraph->voiceMixerDryLeft, vid),
          mPatchOutDryRight(mSplitter, 1, gpSynthGraph->voiceMixerDryRight, vid),
          mPatchOutDelayLeft(mSplitter, 2, gpSynthGraph->delayInputMixerLeft, vid),
          mPatchOutDelayRight(mSplitter, 3, gpSynthGraph->delayInputMixerRight, vid),
          mPatchOutVerbLeft(mSplitter, 4, gpSynthGraph->verbInputMixerLeft, vid),
          mPatchOutVerbRight(mSplitter, 5, gpSynthGraph->verbInputMixerLeft, vid), mVoiceIndex(vid)
    {
    }
};

std::array<Voice, MAX_SYNTH_VOICES> gVoices{initialize_array_with_indices<Voice, MAX_SYNTH_VOICES>()};

} // namespace clarinoid
