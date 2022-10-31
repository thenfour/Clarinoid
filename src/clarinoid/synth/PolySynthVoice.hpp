
#pragma once

#ifdef CLARINOID_MODULE_TEST
#error not for x86 tests
#endif

#include <cfloat>

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/loopstation/LoopstationMemory.hpp>
#include <clarinoid/synth/EnvelopeNode.hpp>

#include "FilterNode.hpp"
#include "PannerNode.hpp"
#include "MultiMixer.hpp"
#include "ModulationMatrixNode.hpp"
#include "Patch.hpp"
#include "polyBlepOscillator.hpp"

namespace clarinoid
{
static constexpr float KRateFrequencyModulationMultiplier = 12.0f;

struct SynthVoiceState
{
    MusicalEventSource mSource;
    HeldNoteInfo mNoteInfo;
    SynthPreset *mSynthPatch = nullptr;
    int mSynthPatchIndex = -1;
    uint32_t mReleaseTimestampMS = 0; // 0 means the note is still active.
};

struct SynthGraph
{
    /*
    https://www.pjrc.com/teensy/gui/index.html
    // this is the after-oscillator processing.

    */
    // GUItool: begin automatically generated code
    AudioAmplifier delayFeedbackAmpLeft;  // xy=535,170
    AudioEffectDelay delayRight;          // xy=536,478
    AudioEffectDelay delayLeft;           // xy=537,261
    AudioAmplifier delayFeedbackAmpRight; // xy=537,390
    AudioAmplifier delayWetAmpLeft;       // xy=781,234
    AudioAmplifier delayWetAmpRight;      // xy=802,469
    AudioMixer4 verbInputMixer;           // xy=920,561
    AudioSynthWaveformSine metronomeOsc;  // xy=956,990
    AudioEffectFreeverbStereo verb;       // xy=1070,567
    AudioEffectEnvelope metronomeEnv;     // xy=1157,992
    AudioAmplifier verbWetAmpLeft;        // xy=1229,548
    AudioAmplifier verbWetAmpRight;       // xy=1236,586
    AudioMixer4 postMixerLeft;            // xy=1411,775
    AudioMixer4 postMixerRight;           // xy=1413,858
    AudioAmplifier ampLeft;               // xy=1573,776
    AudioAmplifier ampRight;              // xy=1576,857
    AudioOutputI2S i2s1;                  // xy=1758,809
    AudioAnalyzePeak peakL;               // xy=1801,588
    AudioAnalyzePeak peakR;               // xy=1851,598
    AudioConnection patchCord1 = {delayWetAmpLeft, 0, postMixerLeft, 1};
    AudioConnection patchCord2 = {delayWetAmpRight, 0, postMixerRight, 1};
    AudioConnection patchCord3 = {verbInputMixer, verb};
    AudioConnection patchCord4 = {metronomeOsc, metronomeEnv};
    AudioConnection patchCord5 = {verb, 0, verbWetAmpLeft, 0};
    AudioConnection patchCord6 = {verb, 1, verbWetAmpRight, 0};
    AudioConnection patchCord7 = {metronomeEnv, 0, postMixerRight, 3};
    AudioConnection patchCord8 = {metronomeEnv, 0, postMixerLeft, 3};
    AudioConnection patchCord9 = {verbWetAmpLeft, 0, postMixerLeft, 2};
    AudioConnection patchCord10 = {verbWetAmpRight, 0, postMixerRight, 2};
    AudioConnection patchCord11 = {postMixerLeft, ampLeft};
    AudioConnection patchCord12 = {postMixerRight, ampRight};
    AudioConnection patchCord13 = {ampLeft, peakL};
    AudioConnection patchCord14 = {ampRight, peakR};
    AudioConnection patchCord15 = {ampLeft, 0, i2s1, 1};
    AudioConnection patchCord16 = {ampRight, 0, i2s1, 0};
    // GUItool: end automatically generated code

    // insert the delay filters.
    // ...[delayLeft]---->[delayFilterLeft]--------------->[delayFeedbackAmpLeft]...
    //                                     \-------------->[delayWetAmpLeft]...
    ::clarinoid::FilterNode delayFilterLeft;
    AudioConnection mPatchDelayToFilterLeft = {delayLeft, 0, delayFilterLeft, 0};
    AudioConnection mPatchDelayFilterToFeedbackAmpLeft = {delayFilterLeft, 0, delayFeedbackAmpLeft, 0};

    ::clarinoid::FilterNode delayFilterRight;
    AudioConnection mPatchDelayToFilterRight = {delayRight, 0, delayFilterRight, 0};
    AudioConnection mPatchDelayFilterToFeedbackAmpRight = {delayFilterRight, 0, delayFeedbackAmpRight, 0};

    // delay output connection
    AudioConnection mPatchDelayFilterToAmpLeft = {delayFilterLeft, 0, delayWetAmpLeft, 0};
    AudioConnection mPatchDelayFilterToAmpRight = {delayFilterRight, 0, delayWetAmpRight, 0};

    // voice mixer & dry output connection
    ::clarinoid::MultiMixerNode<MAX_SYNTH_VOICES> voiceMixerDryLeft;  // all voices input here.
    ::clarinoid::MultiMixerNode<MAX_SYNTH_VOICES> voiceMixerDryRight; // all voices input here.
    AudioConnection patchVoicesDryToOutpLeft = {voiceMixerDryLeft, 0, postMixerLeft, 0};
    AudioConnection patchVoicesDryToOutpRight = {voiceMixerDryRight, 0, postMixerRight, 0};

    // delay input mix
    ::clarinoid::MultiMixerNode<MAX_SYNTH_VOICES + 1>
        delayInputMixerLeft; // +1 to account for the delay feedback signal
    ::clarinoid::MultiMixerNode<MAX_SYNTH_VOICES + 1>
        delayInputMixerRight; // +1 to account for the delay feedback signal
    AudioConnection patchDelayInputMixToDelayL = {delayInputMixerLeft, 0, delayLeft, 0};
    AudioConnection patchDelayInputMixToDelayR = {delayInputMixerRight, 0, delayRight, 0};

    // delay fb
    AudioConnection patchDelayFBBackToInputLeft = {delayFeedbackAmpLeft, 0, delayInputMixerLeft, MAX_SYNTH_VOICES};
    AudioConnection patchDelayFBBackToInputRight = {delayFeedbackAmpRight, 0, delayInputMixerRight, MAX_SYNTH_VOICES};

    // verb input mix
    ::clarinoid::MultiMixerNode<MAX_SYNTH_VOICES + 1> verbInputMixerLeft;  // all voices input here + 1 for delay line
    ::clarinoid::MultiMixerNode<MAX_SYNTH_VOICES + 1> verbInputMixerRight; // all voices input here + 1 for delay line
    AudioConnection patchVerbInputToVerbL = {verbInputMixerLeft, 0, verbInputMixer, 0};
    AudioConnection patchVerbInputToVerbR = {verbInputMixerRight, 0, verbInputMixer, 1};

    // delay verb
    AudioConnection patchDelayVerbInputLeft = {delayFilterLeft, 0, verbInputMixerLeft, MAX_SYNTH_VOICES};
    AudioConnection patchDelayVerbInputRight = {delayFilterRight, 0, verbInputMixerRight, MAX_SYNTH_VOICES};

}; // namespace CCSynthGraph

// dynamic allocate to ensure it goes into RAM2
SynthGraph *gpSynthGraph = nullptr;

StaticInit __synthGraphInit([]() { gpSynthGraph = new SynthGraph(); });

struct Voice : IModulationProvider
{
    AudioBandlimitedOsci mOsc;

    // Modulation sources
    VoiceModulationMatrixNode mModMatrix;
    EnvelopeNode mEnvelopes[ENVELOPE_COUNT];
    AudioSynthWaveform mLfo1;
    AudioSynthWaveform mLfo2;

    // have to track these because they're private members of AudioWaveform.
    short mLfo1Waveshape = 0xff; // invalid so 1st run will always set the shape.
    short mLfo2Waveshape = 0xff; // invalid so 1st run will always set the shape.

    virtual std::pair<AudioStream *, size_t> IModulationProvider_GetARateSourcePort(ARateModulationSource src) override
    {
        switch (src)
        {
        case ARateModulationSource::ENV1:
            return {&mEnvelopes[0], 0};
        case ARateModulationSource::ENV2:
            return {&mEnvelopes[1], 0};
        case ARateModulationSource::LFO1:
            return {&mLfo1, 0};
        case ARateModulationSource::LFO2:
            return {&mLfo2, 0};
        case ARateModulationSource::Osc1FB:
            return {&mOsc, 0};
        case ARateModulationSource::Osc2FB:
            return {&mOsc, 1};
        case ARateModulationSource::Osc3FB:
            return {&mOsc, 2};
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
        case ARateModulationDestination::Osc1Phase:
            return {&mOsc, (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::pm1};
        case ARateModulationDestination::Osc2Phase:
            return {&mOsc, (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::pm2};
        case ARateModulationDestination::Osc3Phase:
            return {&mOsc, (uint8_t)AudioBandlimitedOsci::INPUT_INDEX::pm3};
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
    SynthVoiceState mRunningVoice;
    AppSettings *mAppSettings;
    ISynthParamProvider *mParamProvider;
    bool mTouched = false;

    void EnsurePatchConnections(AppSettings *appSettings, ISynthParamProvider *paramProvider)
    {
        mAppSettings = appSettings;
        mParamProvider = paramProvider;

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
            return mParamProvider->SynthParamProvider_GetBreath01();
        case KRateModulationSource::PitchStrip:
            return mParamProvider->SynthParamProvider_GetPitchBendN11();
        case KRateModulationSource::Velocity:
            return mRunningVoice.mNoteInfo.mVelocity01;
        case KRateModulationSource::NoteValue:
            return float(mRunningVoice.mNoteInfo.mMidiNote.GetMidiValue()) / 127;
        case KRateModulationSource::RandomTrigger:
            return mRunningVoice.mNoteInfo.mRandomTrigger01;
        case KRateModulationSource::ModWheel:
            return float(mParamProvider->SynthParamProvider_GetMidiCC(MidiCCValue::ModWheel)) / 127;
        case KRateModulationSource::Macro1:
            return mParamProvider->SynthParamProvider_GetMacroValue01(0);
        case KRateModulationSource::Macro2:
            return mParamProvider->SynthParamProvider_GetMacroValue01(1);
        case KRateModulationSource::Macro3:
            return mParamProvider->SynthParamProvider_GetMacroValue01(2);
        case KRateModulationSource::Macro4:
            return mParamProvider->SynthParamProvider_GetMacroValue01(3);
        case KRateModulationSource::Pedal:
            return float(mParamProvider->SynthParamProvider_GetMidiCC(MidiCCValue::DamperPedal)) / 127;
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

    void ApplyPatchToGraph()
    {
        const auto &patch = *mRunningVoice.mSynthPatch;
        const auto &perf = mAppSettings->GetCurrentPerformancePatch();

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

        short wantsWaveType1 = convertWaveType(patch.mLFO1.mWaveShape);
        short wantsWaveType2 = convertWaveType(patch.mLFO2.mWaveShape);
        if (mLfo1Waveshape != wantsWaveType1)
        {
            mLfo1.begin(wantsWaveType1);
            mLfo1.amplitude(1.0f);
        }
        if (mLfo2Waveshape != wantsWaveType2)
        {
            mLfo2.begin(wantsWaveType2);
            mLfo2.amplitude(1.0f);
        }

        mLfo1.frequency(patch.mLFO1.mTime.ToHertz(perf.mBPM));
        mLfo2.frequency(patch.mLFO2.mTime.ToHertz(perf.mBPM));
    }

    float CalcFreq(const SynthOscillatorSettings &osc, float detune, float krateFreqModN11, PortamentoCalc &portamento)
    {
        // we're in semis land... let's figure out the semitone.
        float midiNote = mRunningVoice.mNoteInfo.mMidiNote.GetMidiValue();

        float userPB = mParamProvider->SynthParamProvider_GetPitchBendN11();
        float pbSemis = userPB * ((userPB > 0) ? osc.mPitchBendRangePositive : (-osc.mPitchBendRangeNegative));

        float paramSemis = osc.mFreqParam.GetMidiNote(midiNote, krateFreqModN11);

        float idealSemis = paramSemis + osc.mPitchFine + osc.mPitchSemis + detune;

        float retSemis = portamento.KStep(idealSemis, osc.mPortamentoTimeMS);
        retSemis += pbSemis; // Pitchbend needs to be after portamento.

        float retHz = MIDINoteToFreq(retSemis);
        retHz *= osc.mFreqMultiplier;
        retHz += osc.mFreqOffsetHz;

        return Clamp(retHz, 0.0f, 22050.0f);
    };

    void Update(USBMidiMusicalState &ms)
    {
        // make sure this gets updated; it doesn't get automatically syncd
        this->mRunningVoice.mNoteInfo.mIsPhysicallyHeld =
            ms.isPhysicallyPressed(mRunningVoice.mNoteInfo.mLiveNoteSequenceID);
        // basically the only time this will become -1 is at startup
        if (mRunningVoice.mSynthPatch == nullptr)
        {
            mOsc.mIsPlaying = false;
            return;
        }

        mOsc.mIsPlaying = true;

        const auto &patch = *mRunningVoice.mSynthPatch;
        const auto &perf = mAppSettings->GetCurrentPerformancePatch();

        float externalGain = 1.0f;
        switch (mRunningVoice.mSource.mType)
        {
        case MusicalEventSourceType::Harmonizer:
            // TODO: stereo sep of perf harm
            // TODO: stereo sep of harm voice
            if (!perf.mHarmEnabled)
            {
                mOsc.mIsPlaying = false;
                return;
            }
            externalGain = perf.mHarmGain;
            // individual harmonizer voices don't have their own volume (yet?)
            // externalGain =
            // mAppSettings->FindHarmPreset(perf.mHarmPreset).mVoiceSettings[mRunningVoice.mSource.mHarmonizerVoiceIndex].
            break;
        case MusicalEventSourceType::LivePlayA:
            // TODO: stereo sep of perf patches
            if (!perf.mSynthAEnabled)
            {
                mOsc.mIsPlaying = false;
                return;
            }
            externalGain = perf.mSynthAGain;
            break;
        case MusicalEventSourceType::LivePlayB:
            // TODO: stereo sep of perf patches
            if (!perf.mSynthBEnabled)
            {
                mOsc.mIsPlaying = false;
                return;
            }
            externalGain = perf.mSynthBGain;
            break;
        case MusicalEventSourceType::Loopstation:
            // TODO... how does this work?
            break;
        default:
            break;
        }

        // apply ongoing params
        ApplyPatchToGraph();

        // figure out which oscillators are enabled. Get a count and grab enabled indices.
        int oscEnabledCount = 0;
        // size_t enabledOscIndices[POLYBLEP_OSC_COUNT];
        for (size_t iosc = 0; iosc < POLYBLEP_OSC_COUNT; ++iosc)
        {
            if (!patch.mOsc[iosc].mEnabled)
                break;
            // enabledOscIndices[oscEnabledCount] = iosc;
            oscEnabledCount++;
        }

        float detune = patch.mDetune + mModMatrix.GetKRateDestinationValue(KRateModulationDestination::Detune);
        float spread = patch.mStereoSpread;

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
            spreads[0] = 0;
            spreads[1] = spread;
            spreads[2] = -spread;
            break;
        default:
            CCASSERT(!"hardcoded osc count alert");
        }

        // apply oscillator params
        mOsc.mIsPlaying = oscEnabledCount > 0;

        size_t ienabledOsc = 0;

        // String sd = String("v") + mVoiceIndex + ": ";

        for (size_t i = 0; i < POLYBLEP_OSC_COUNT; ++i)
        {
            mOsc.mOsc[i].mEnabled = patch.mOsc[i].mEnabled;

            mOscMixerPanner.SetInputPanGainAndEnabled(
                i,
                patch.mPan + patch.mOsc[i].mPan + spreads[ienabledOsc],
                patch.mOsc[i]
                    .mVolume
                    .AddParam(mModMatrix.GetKRateDestinationValue(gModValuesByOscillator[i].KRateDestination_Volume))
                    .ToLinearGain(),
                true);

            if (!patch.mOsc[i].mEnabled)
                continue;

            mOsc.mOsc[i].pulseWidth(patch.mOsc[i].mPulseWidth);
            mOsc.mOsc[i].mPMMultiplier = patch.mOverallFMStrength + mModMatrix.GetKRateDestinationValue(
                                                                        KRateModulationDestination::OverallFMStrength);

            mOsc.mOsc[i].mPMFeedbackAmt =
                patch.mOsc[i].mFMFeedbackGain +
                mModMatrix.GetKRateDestinationValue(gModValuesByOscillator[i].KRateDestination_FMFeedback);

            float freq =
                CalcFreq(patch.mOsc[i],
                         detunes[ienabledOsc],
                         mModMatrix.GetKRateDestinationValue(gModValuesByOscillator[i].KRateDestination_Frequency),
                         mPortamentoCalc[i]);

            // sd += String("[o") + i + " det:" + detunes[ienabledOsc] + " freq:" + freq + "]  ";

            mOsc.mOsc[i].SetBasicParams(
                patch.mOsc[i].mWaveform,
                freq,
                patch.mOsc[i].mPhase01,
                patch.mOsc[i].mPortamentoTimeMS,
                patch.mOsc[i].mSyncFreqParam.GetFrequency(
                    mRunningVoice.mNoteInfo.mMidiNote.GetMidiValue(),
                    mModMatrix.GetKRateDestinationValue(gModValuesByOscillator[i].KRateDestination_SyncFrequency)));

            ienabledOsc++;
        }

        // if (mVoiceIndex == 0)
        //     Serial.println(sd);

        // TODO: if portamento is enabled for an oscillator, it should be accounted for here.
        float filterFreq = mRunningVoice.mSynthPatch->mFilterFreqParam.GetFrequency(
            MIDINoteToFreq(mRunningVoice.mNoteInfo.mMidiNote.GetMidiValue()),
            mModMatrix.GetKRateDestinationValue(KRateModulationDestination::FilterCutoff));
        mFilter.SetParams(patch.mFilterType, filterFreq, patch.mFilterQ, patch.mFilterSaturation);
        mFilter.EnableDCFilter(patch.mDCFilterEnabled, patch.mDCFilterCutoff);

        // for "mix" behavior, we can look to panning. the pan law applies here; you're effectively panning between the
        // dry signal and the various effects. But instead of a -1 to +1 range, it's going to be 0-1.
        auto verbGains = CalculatePanGain(patch.mDelayMix * 2 - 1);
        auto delayGains = CalculatePanGain(patch.mVerbMix * 2 - 1);

        float masterGain =
            patch.mMasterVolume.AddParam(mModMatrix.GetKRateDestinationValue(KRateModulationDestination::MasterVolume))
                .ToLinearGain();
        masterGain = std::max(0.0f, masterGain * externalGain);

        mSplitter.SetOutputGain(0, masterGain * std::get<0>(verbGains) * std::get<0>(delayGains));
        mSplitter.SetOutputGain(1, masterGain * std::get<1>(delayGains));
        mSplitter.SetOutputGain(2, masterGain * std::get<1>(verbGains));
    }

    // if there's already a note playing, it gets cut off
    void IncomingMusicalEvents_OnNoteOn(const MusicalEventSource &source,
                                        const HeldNoteInfo &noteInfo,
                                        uint16_t synthPatchIndex)
    {
        auto newSynthPatch = &mAppSettings->FindSynthPreset(synthPatchIndex);

        bool isLegato = (mRunningVoice.mSource.Equals(source)) && (mRunningVoice.mReleaseTimestampMS == 0) &&
                        (newSynthPatch->mVoicingMode == VoicingMode::Monophonic);

        // i don't actually think this is necessary. to be confirmed.
        for (size_t i = 0; i < ENVELOPE_COUNT; ++i)
        {
            if (!isLegato || (isLegato && mRunningVoice.mSynthPatch->mEnvelopes[i].mLegatoRestart))
                mEnvelopes[i].noteOff();
        }

        // adjust running voice.
        mRunningVoice.mSource = source;
        mRunningVoice.mNoteInfo = noteInfo;
        mRunningVoice.mReleaseTimestampMS = 0; // important so we know the note is playing

        if (mRunningVoice.mSynthPatch != newSynthPatch)
        {
            mRunningVoice.mSynthPatchIndex = synthPatchIndex;
            mRunningVoice.mSynthPatch = newSynthPatch;
            mModMatrix.SetSynthPatch(newSynthPatch, this);
        }

        for (size_t i = 0; i < POLYBLEP_OSC_COUNT; ++i)
        {
            if (mRunningVoice.mSynthPatch->mOsc[i].mPhaseRestart)
            {
                mOsc.mOsc[i].ResetPhase();
            }
        }

        for (size_t i = 0; i < ENVELOPE_COUNT; ++i)
        {
            if (!isLegato || (isLegato && mRunningVoice.mSynthPatch->mEnvelopes[i].mLegatoRestart))
                mEnvelopes[i].noteOn();
        }

        if (mRunningVoice.mSynthPatch->mLFO1.mPhaseRestart)
        {
            mLfo1.begin(convertWaveType(mRunningVoice.mSynthPatch->mLFO1.mWaveShape));
            mLfo1.amplitude(1.0f);
        }

        if (mRunningVoice.mSynthPatch->mLFO2.mPhaseRestart)
        {
            mLfo2.begin(convertWaveType(mRunningVoice.mSynthPatch->mLFO2.mWaveShape));
            mLfo2.amplitude(1.0f);
        }
    }

    void IncomingMusicalEvents_OnNoteOff()
    {
        mRunningVoice.mReleaseTimestampMS = millis();
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
        if (mRunningVoice.mReleaseTimestampMS == 0)
        {
            return TimeTo01(millis() - mRunningVoice.mNoteInfo.mAttackTimestampMS);
        }

        // ok we're at least at release stage.
        auto timeSinceReleaseMS = millis() - mRunningVoice.mReleaseTimestampMS;
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

        if (mRunningVoice.mReleaseTimestampMS == 0)
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
        if (!mRunningVoice.mSynthPatch)
        {
            ret += "<idle>";
            return ret;
        }
        // synth patch exists.
        ret += String() // + String("p") + mRunningVoice.mSynthPatchIndex + " "
               + mRunningVoice.mNoteInfo.mMidiNote.ToString() + " " +
               gEnvelopeStageInfo.GetValueString(mEnvelopes[0].GetStage());
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

Voice gVoices[MAX_SYNTH_VOICES] = {VOICE_INITIALIZER};

struct SynthGraphControl
{
    float mPrevMetronomeBeatFrac = 0;
    AppSettings *mAppSettings;
    Metronome *mMetronome;
    ISynthParamProvider *mParamProvider;

    void Setup(AppSettings *appSettings, Metronome *metronome, ISynthParamProvider *paramProvider)
    {
        AudioStream::initialize_memory(CLARINOID_AUDIO_MEMORY, SizeofStaticArray(CLARINOID_AUDIO_MEMORY));

        mAppSettings = appSettings;
        mMetronome = metronome;
        mParamProvider = paramProvider;

        // for some reason patches really don't like to connect unless they are
        // last in the initialization order. Here's a workaround to force them to
        // connect.
        for (auto &v : gVoices)
        {
            v.EnsurePatchConnections(appSettings, paramProvider);
        }

        gpSynthGraph->ampLeft.gain(1);
        gpSynthGraph->ampRight.gain(1);

        gpSynthGraph->metronomeEnv.delay(0);
        gpSynthGraph->metronomeEnv.attack(0);
        gpSynthGraph->metronomeEnv.hold(0);
        gpSynthGraph->metronomeEnv.releaseNoteOn(0);
        gpSynthGraph->metronomeEnv.sustain(0);
    }

    void UpdatePostFx()
    {
        auto &perf = mAppSettings->GetCurrentPerformancePatch();

        gpSynthGraph->delayFeedbackAmpLeft.gain(perf.mDelayFeedbackLevel);
        gpSynthGraph->delayFeedbackAmpRight.gain(perf.mDelayFeedbackLevel);
        gpSynthGraph->delayLeft.delay(0, perf.mDelayTime.ToMS(perf.mBPM) - perf.mDelayStereoSep * .5f);
        gpSynthGraph->delayRight.delay(0, perf.mDelayTime.ToMS(perf.mBPM) + perf.mDelayStereoSep * .5f);

        gpSynthGraph->delayFilterLeft.SetParams(
            perf.mDelayFilterType, perf.mDelayCutoffFrequency, perf.mDelayQ, perf.mDelaySaturation);
        gpSynthGraph->delayFilterRight.SetParams(
            perf.mDelayFilterType, perf.mDelayCutoffFrequency, perf.mDelayQ, perf.mDelaySaturation);

        gpSynthGraph->delayWetAmpLeft.gain(
            (perf.mMasterFXEnable && perf.mDelayEnabled) ? (perf.mDelayGain * perf.mMasterFXGain) : 0.0f);
        gpSynthGraph->delayWetAmpRight.gain(
            (perf.mMasterFXEnable && perf.mDelayEnabled) ? (perf.mDelayGain * perf.mMasterFXGain) : 0.0f);

        gpSynthGraph->verb.roomsize(perf.mReverbSize);
        gpSynthGraph->verb.damping(perf.mReverbDamping);

        gpSynthGraph->verbWetAmpLeft.gain(
            (perf.mMasterFXEnable && perf.mReverbEnabled) ? (perf.mReverbGain * perf.mMasterFXGain) : 0.0f);
        gpSynthGraph->verbWetAmpRight.gain(
            (perf.mMasterFXEnable && perf.mReverbEnabled) ? (perf.mReverbGain * perf.mMasterFXGain) : 0.0f);

        gpSynthGraph->ampLeft.gain(perf.mMasterGain);
        gpSynthGraph->ampRight.gain(perf.mMasterGain);

        if (!mAppSettings->mMetronomeSoundOn)
        {
            gpSynthGraph->metronomeOsc.amplitude(0);
        }
        else
        {
            gpSynthGraph->metronomeEnv.decay(mAppSettings->mMetronomeDecayMS);
            gpSynthGraph->metronomeOsc.amplitude(mAppSettings->mMetronomeGain);
            gpSynthGraph->metronomeOsc.frequency(MIDINoteToFreq(mAppSettings->mMetronomeNote));

            float metronomeBeatFrac = mMetronome->GetBeatFrac();
            if (metronomeBeatFrac < mPrevMetronomeBeatFrac)
            { // beat boundary is when the frac drops back
              // to 0
                gpSynthGraph->metronomeEnv.noteOn();
            }
            mPrevMetronomeBeatFrac = metronomeBeatFrac;
        }
    }
};

SynthGraphControl gSynthGraphControl;

} // namespace clarinoid
