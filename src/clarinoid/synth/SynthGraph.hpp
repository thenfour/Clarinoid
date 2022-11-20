
#pragma once

#ifdef CLARINOID_MODULE_TEST
#error not for x86 tests
#endif

#include <clarinoid/basic/Basic.hpp>

#include "FilterNode.hpp"
#include "PannerNode.hpp"
#include "MultiMixer.hpp"
#include "Patch.hpp"

namespace clarinoid
{

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

struct SynthGraphControl
{
    float mPrevMetronomeBeatFrac = 0;
    AppSettings *mAppSettings;
    Metronome *mMetronome;
    //ISynthParamProvider *mParamProvider;

    void Setup(AppSettings *appSettings, Metronome *metronome/*, ISynthParamProvider *paramProvider*/)
    {
        AudioStream::initialize_memory(CLARINOID_AUDIO_MEMORY, SizeofStaticArray(CLARINOID_AUDIO_MEMORY));

        mAppSettings = appSettings;
        mMetronome = metronome;
        //mParamProvider = paramProvider;

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

        gpSynthGraph->delayFeedbackAmpLeft.gain(perf.mDelay.mFeedbackGain.GetValue());
        gpSynthGraph->delayFeedbackAmpRight.gain(perf.mDelay.mFeedbackGain.GetValue());
        gpSynthGraph->delayLeft.delay(0, perf.mDelay.mTime.ToMS(perf.mBPM.GetValue()) - perf.mDelay.mStereoSeparationDelayMS.GetValue() * .5f);
        gpSynthGraph->delayRight.delay(0, perf.mDelay.mTime.ToMS(perf.mBPM.GetValue()) + perf.mDelay.mStereoSeparationDelayMS.GetValue() * .5f);

        perf.mDelay.mFilter.mFrequency.mKTValue.SetValue(0); // KT not supported for master filter
        gpSynthGraph->delayFilterLeft.SetParams(
            perf.mDelay.mFilter.mType.GetValue(), perf.mDelay.mFilter.mFrequency.GetFrequency(64, 0), perf.mDelay.mFilter.mQ.GetValue(), perf.mDelay.mFilter.mSaturation.GetValue());
        gpSynthGraph->delayFilterRight.SetParams(
            perf.mDelay.mFilter.mType.GetValue(), perf.mDelay.mFilter.mFrequency.GetFrequency(64, 0), perf.mDelay.mFilter.mQ.GetValue(), perf.mDelay.mFilter.mSaturation.GetValue());

        gpSynthGraph->delayWetAmpLeft.gain(
            (perf.mMasterFXEnable.GetValue() && perf.mDelay.mEnabled.GetValue()) ? (perf.mDelay.mGain.GetValue() * perf.mMasterFXGain.GetValue()) : 0.0f);
        gpSynthGraph->delayWetAmpRight.gain(
            (perf.mMasterFXEnable.GetValue() && perf.mDelay.mEnabled.GetValue()) ? (perf.mDelay.mGain.GetValue() * perf.mMasterFXGain.GetValue()) : 0.0f);

        gpSynthGraph->verb.roomsize(perf.mReverb.mSize.GetValue());
        gpSynthGraph->verb.damping(perf.mReverb.mDamping.GetValue());

        gpSynthGraph->verbWetAmpLeft.gain(
            (perf.mMasterFXEnable.GetValue() && perf.mReverb.mEnabled.GetValue()) ? (perf.mReverb.mGain.GetValue() * perf.mMasterFXGain.GetValue()) : 0.0f);
        gpSynthGraph->verbWetAmpRight.gain(
            (perf.mMasterFXEnable.GetValue() && perf.mReverb.mEnabled.GetValue()) ? (perf.mReverb.mGain.GetValue() * perf.mMasterFXGain.GetValue()) : 0.0f);

        gpSynthGraph->ampLeft.gain(perf.mMasterGain.GetValue());
        gpSynthGraph->ampRight.gain(perf.mMasterGain.GetValue());

        if (!mAppSettings->mMetronome.mSoundOn.GetValue())
        {
            gpSynthGraph->metronomeOsc.amplitude(0);
        }
        else
        {
            gpSynthGraph->metronomeEnv.decay(mAppSettings->mMetronome.mDecayMS.GetValue());
            gpSynthGraph->metronomeOsc.amplitude(mAppSettings->mMetronome.mGain.GetValue());
            gpSynthGraph->metronomeOsc.frequency(MIDINoteToFreq(mAppSettings->mMetronome.mMidiNote.GetValue()));

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
