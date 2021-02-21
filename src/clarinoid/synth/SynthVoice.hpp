
#pragma once

#ifdef CLARINOID_MODULE_TEST
#error not for x86 tests
#endif

#include <cfloat>

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/loopstation/LoopstationMemory.hpp>

#include "Patch.hpp"
#include "FilterNode.hpp"

namespace clarinoid
{

  // https://searchfox.org/mozilla-central/source/dom/media/webaudio/blink/Biquad.h
  class Biquad
  {
  public:
    Biquad();
    ~Biquad();

    void process(const float *sourceP, float *destP, size_t framesToProcess);

    // frequency is 0 - 1 normalized, resonance and dbGain are in decibels.
    // Q is a unitless quality factor.
    void setLowpassParams(float frequency, float resonance);

    // Resets filter state
    void reset();

  private:
    void setNormalizedCoefficients(float b0, float b1, float b2, float a0,
                                   float a1, float a2);

    // Filter coefficients. The filter is defined as
    //
    // y[n] + m_a1*y[n-1] + m_a2*y[n-2] = m_b0*x[n] + m_b1*x[n-1] + m_b2*x[n-2].
    float m_b0;
    float m_b1;
    float m_b2;
    float m_a1;
    float m_a2;

    // Filter memory
    //
    // Double precision for the output values is valuable because errors can
    // accumulate.  Input values are also stored as double so they need not be
    // converted again for computation.
    float m_x1; // input delayed by 1 sample
    float m_x2; // input delayed by 2 samples
    float m_y1; // output delayed by 1 sample
    float m_y2; // output delayed by 2 samples
  };

  Biquad::Biquad()
  {
    // Initialize as pass-thru (straight-wire, no filter effect)
    setNormalizedCoefficients(1, 0, 0, 1, 0, 0);

    reset(); // clear filter memory
  }

  Biquad::~Biquad() = default;

  void Biquad::process(const float *sourceP, float *destP,
                       size_t framesToProcess)
  {
    // Create local copies of member variables
    float x1 = m_x1;
    float x2 = m_x2;
    float y1 = m_y1;
    float y2 = m_y2;

    float b0 = m_b0;
    float b1 = m_b1;
    float b2 = m_b2;
    float a1 = m_a1;
    float a2 = m_a2;

    for (size_t i = 0; i < framesToProcess; ++i)
    {
      // FIXME: this can be optimized by pipelining the multiply adds...
      float x = sourceP[i];
      float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

      destP[i] = y;

      // Update state variables
      x2 = x1;
      x1 = x;
      y2 = y1;
      y1 = y;
    }

    // Avoid introducing a stream of subnormals when input is silent and the
    // tail approaches zero.
    if (x1 == 0.0 && x2 == 0.0 && (y1 != 0.0 || y2 != 0.0) &&
        fabs(y1) < FLT_MIN && fabs(y2) < FLT_MIN)
    {
      // Flush future values to zero (until there is new input).
      y1 = y2 = 0.0;
// Flush calculated values.
#ifndef HAVE_DENORMAL
      for (int i = framesToProcess; i-- && fabsf(destP[i]) < FLT_MIN;)
      {
        destP[i] = 0.0f;
      }
#endif
    }
    // Local variables back to member.
    m_x1 = x1;
    m_x2 = x2;
    m_y1 = y1;
    m_y2 = y2;
  }

  void Biquad::reset() { m_x1 = m_x2 = m_y1 = m_y2 = 0; }

  void Biquad::setLowpassParams(float cutoff, float resonance)
  {
    const float nyquist = 44100 * 0.5;
    cutoff = cutoff / nyquist;

    // Limit cutoff to 0 to 1.
    cutoff = std::max(0.0, std::min(cutoff, 1.0));

    if (cutoff == 1)
    {
      // When cutoff is 1, the z-transform is 1.
      setNormalizedCoefficients(1, 0, 0, 1, 0, 0);
    }
    else if (cutoff > 0)
    {
      // Compute biquad coefficients for lowpass filter
      float g = pow(10.0, -0.05 * resonance);
      float w0 = M_PI * cutoff;
      float cos_w0 = cos(w0);
      float alpha = 0.5 * sin(w0) * g;

      float b1 = 1.0 - cos_w0;
      float b0 = 0.5 * b1;
      float b2 = b0;
      float a0 = 1.0 + alpha;
      float a1 = -2.0 * cos_w0;
      float a2 = 1.0 - alpha;

      setNormalizedCoefficients(b0, b1, b2, a0, a1, a2);
    }
    else
    {
      // When cutoff is zero, nothing gets through the filter, so set
      // coefficients up correctly.
      setNormalizedCoefficients(0, 0, 0, 1, 0, 0);
    }
  }

  void Biquad::setNormalizedCoefficients(float b0, float b1, float b2,
                                         float a0, float a1, float a2)
  {
    float a0Inverse = 1 / a0;

    m_b0 = b0 * a0Inverse;
    m_b1 = b1 * a0Inverse;
    m_b2 = b2 * a0Inverse;
    m_a1 = a1 * a0Inverse;
    m_a2 = a2 * a0Inverse;
  }

  // https://www.musicdsp.org/en/latest/Filters/26-moog-vcf-variation-2.html
  // this does not work well at all in low frequencies so it's not suitable.
  struct Moog4P
  {
    float fc = 0.0f;
    float res = 0.0f;

    float out1 = 0.0f;
    float out2 = 0.0f;
    float out3 = 0.0f;
    float out4 = 0.0f;
    float in1 = 0.0f;
    float in2 = 0.0f;
    float in3 = 0.0f;
    float in4 = 0.0f;

    // in[x] and out[x] are member variables, init to 0.0 the controls:
    // fc = cutoff, nearly linear [0,1] -> [0, fs/2]
    // res = resonance [0, 4] -> [no resonance, self-oscillation]
    void setFrequencyAndQ(float freq, float Q)
    {
      fc = freq / 22050.0f;
      res = Q;
    }

    float process(float input)
    {
      float f = fc * 1.16f;
      float fb = res * (1.0f - 0.15f * f * f);
      input -= out4 * fb;
      input *= 0.35013f * (f * f) * (f * f);
      out1 = input + 0.3f * in1 + (1.0f - f) * out1; // Pole 1
      in1 = input;
      out2 = out1 + 0.3f * in2 + (1.0f - f) * out2; // Pole 2
      in2 = out1;
      out3 = out2 + 0.3f * in3 + (1.0f - f) * out3; // Pole 3
      in3 = out2;
      out4 = out3 + 0.3f * in4 + (1.0f - f) * out4; // Pole 4
      in4 = out3;
      return out4;
    }
  };

  // https://www.musicdsp.org/en/latest/Filters/24-moog-vcf.html
  class MoogFilter
  {
  public:
    MoogFilter();
    void init();
    void calc();
    float process(float x);
    ~MoogFilter();
    float getCutoff();
    void setCutoff(float c);
    float getRes();
    void setRes(float r);

  protected:
    float cutoff;
    float res;
    float fs;
    float y1, y2, y3, y4;
    float oldx;
    float oldy1, oldy2, oldy3;
    float x;
    float r;
    float p;
    float k;
  };

  MoogFilter::MoogFilter()
  {
    fs = 44100.0;

    init();
  }

  MoogFilter::~MoogFilter()
  {
  }

  void MoogFilter::init()
  {
    // initialize values
    y1 = y2 = y3 = y4 = oldx = oldy1 = oldy2 = oldy3 = 0;
    calc();
  };

  void MoogFilter::calc()
  {
    float f = (cutoff + cutoff) / fs; //[0 - 1]
    p = f * (1.8f - 0.8f * f);
    k = p + p - 1.f;

    float t = (1.f - p) * 1.386249f;
    float t2 = 12.f + t * t;
    r = res * (t2 + 6.f * t) / (t2 - 6.f * t);
  };

  float MoogFilter::process(float input)
  {
    // process input
    x = input - r * y4;

    //Four cascaded onepole filters (bilinear transform)
    y1 = x * p + oldx * p - k * y1;
    y2 = y1 * p + oldy1 * p - k * y2;
    y3 = y2 * p + oldy2 * p - k * y3;
    y4 = y3 * p + oldy3 * p - k * y4;

    //Clipper band limited sigmoid
    y4 -= (y4 * y4 * y4) / 6.f;

    oldx = x;
    oldy1 = y1;
    oldy2 = y2;
    oldy3 = y3;
    return y4;
  }

  float MoogFilter::getCutoff()
  {
    return cutoff;
  }

  void MoogFilter::setCutoff(float c)
  {
    cutoff = c;
    calc();
  }

  float MoogFilter::getRes()
  {
    return res;
  }

  void MoogFilter::setRes(float r)
  {
    res = r;
    calc();
  }

  class FilterNode : public AudioStream
  {
    //MoogFilter mFilterL;
    //Moog4P mFilter;
    Biquad mFilter;

  public:
    FilterNode() : AudioStream(1, inputQueueArray)
    {
    }

    virtual void update() override
    {
      audio_block_t *block = receiveWritable();
      if (!block)
        return;
      int16_t *p = block->data;
      //int16_t *end = block->data + AUDIO_BLOCK_SAMPLES;
      float tempBufferSource[AUDIO_BLOCK_SAMPLES];
      float tempBufferDest[AUDIO_BLOCK_SAMPLES];
      for(size_t i = 0; i < AUDIO_BLOCK_SAMPLES; ++ i)
      {
        tempBufferSource[i] = (float)p[i] / 32767.0f;
      }
      mFilter.process(tempBufferSource, tempBufferDest, AUDIO_BLOCK_SAMPLES);
      for(size_t i = 0; i < AUDIO_BLOCK_SAMPLES; ++ i)
      {
        float f = tempBufferDest[i];
        if (f <= -1) p[i] = -32767;
        else if (f >= 1) p[i] = 32767;
        else p[i] = (int16_t)(f * 32767);
      }

      transmit(block);
      release(block);
    }

    void SetFrequencyAndQ(float freq, float Q)
    {
      mFilter.setLowpassParams(freq, Q);
      //mFilter.setFrequencyAndQ(freq, Q);
      // mFilter.setCutoff(freq);
      // mFilterL.setRes(Q);
    }

    audio_block_t *inputQueueArray[1];
  };

  namespace CCSynthGraph
  {
    /*
https://www.pjrc.com/teensy/gui/index.html
// this is the after-oscillator processing.

*/

    // GUItool: begin automatically generated code
    AudioMixer4 voiceMix2;               //xy=787.2500190734863,825.0000057220459
    AudioMixer4 voiceMix4;               //xy=794.7500190734863,1000.0000171661377
    AudioMixer4 voiceMix1;               //xy=796.0000190734863,751.250020980835
    AudioMixer4 voiceMix3;               //xy=803.5000267028809,915.0000076293945
    AudioMixer4 voiceMixOutp;            //xy=1067.2500267028809,872.5000228881836
    AudioMixer4 waveMixer;               //xy=1289.7500343322754,873.5000228881836
    AudioEffectFreeverbStereo verb;      //xy=1459.50004196167,777.5000190734863
    AudioSynthWaveformSine metronomeOsc; //xy=1478.0000381469727,964.5001258850098
    AudioAmplifier verbWetAmpLeft;       //xy=1654.2500457763672,774.7500228881836
    AudioEffectEnvelope metronomeEnv;    //xy=1679.0000457763672,966.0000286102295
    AudioMixer4 postMixerLeft;           //xy=1942,885
    AudioAmplifier ampLeft;              //xy=2151.5000648498535,891.2500267028809
    AudioOutputI2S i2s1;                 //xy=2389.750068664551,901.2500267028809
    AudioAnalyzePeak peak1;              //xy=2393.500068664551,1005.0000290870667
    AudioAnalyzeRMS rms1;                //xy=2394.750068664551,1046.2500305175781
    AudioConnection patchCord1(voiceMix2, 0, voiceMixOutp, 1);
    AudioConnection patchCord2(voiceMix4, 0, voiceMixOutp, 3);
    AudioConnection patchCord3(voiceMix1, 0, voiceMixOutp, 0);
    AudioConnection patchCord4(voiceMix3, 0, voiceMixOutp, 2);
    AudioConnection patchCord5(voiceMixOutp, 0, waveMixer, 1);
    AudioConnection patchCord6(waveMixer, 0, postMixerLeft, 1);
    AudioConnection patchCord7(waveMixer, verb);
    AudioConnection patchCord8(verb, 0, verbWetAmpLeft, 0);
    AudioConnection patchCord9(metronomeOsc, metronomeEnv);
    AudioConnection patchCord10(verbWetAmpLeft, 0, postMixerLeft, 0);
    AudioConnection patchCord11(metronomeEnv, 0, postMixerLeft, 2);
    AudioConnection patchCord12(postMixerLeft, ampLeft);
    AudioConnection patchCord13(ampLeft, 0, i2s1, 0);
    AudioConnection patchCord14(ampLeft, peak1);
    AudioConnection patchCord15(ampLeft, rms1);
    AudioConnection patchCord16(ampLeft, 0, i2s1, 1);
    // GUItool: end automatically generated code

  } // namespace CCSynthGraph

  struct Voice
  {
    AudioBandlimitedOsci mOsc;
    CCPatch mPatchOsc1ToMix;
    CCPatch mPatchOsc2ToMix;
    CCPatch mPatchOsc3ToMix;
    AudioMixer4 mOscMixer;     // mixes down the 3 oscillators
    CCPatch mPatchMixToFilter; // then into filter.
    //AudioFilterStateVariable mFilter;
    //AudioFilterBiquad mFilter;
    ::clarinoid::FilterNode mFilter;
    CCPatch mPatchOut;

    MusicalVoice mRunningVoice;
    SynthPreset *mPreset = nullptr;

    AppSettings *mAppSettings;

    void EnsurePatchConnections(AppSettings *appSettings)
    {
      mAppSettings = appSettings;

      mPatchOsc1ToMix.connect();
      mPatchOsc2ToMix.connect();
      mPatchOsc3ToMix.connect();
      mPatchMixToFilter.connect();
      mPatchOut.connect();
    }

    static float CalcFilterCutoffFreq(float breath01, float midiNote, float keyTrackingAmt, float freqMin, float freqMax)
    {
      // perform breath & key tracking for filter. we will basically multiply the effects.
      // velocity we will only track between notes
      // from 7jam code: const halfKeyScaleRangeSemis = 12 * 4;
      // from 7jam code: let ks = 1.0 - DF.remap(this.midiNote, 60.0 /* middle C */ - halfKeyScaleRangeSemis, 60.0 + halfKeyScaleRangeSemis, ksAmt, -ksAmt); // when vsAmt is 0, the range of vsAmt,-vsAmt is 0. hence making this 1.0-x
      float filterKS = map(midiNote, 20, 120, 0.0f, 1.0f);    // map midi note to full ks effect
      filterKS = map(keyTrackingAmt, 0, 1.0f, 1.0, filterKS); // map ks amt 0-1 to 1-fulleffect

      float filterP = filterKS * breath01;
      filterP = ClampInclusive(filterP, 0.0f, 1.0f);

      float filterFreq = map(filterP, 0.0f, 1.0f, freqMin, freqMax);
      return filterFreq;
    }

    void Update(const MusicalVoice &mv)
    {
      mPreset = &mAppSettings->FindSynthPreset(mv.mSynthPatch);
      bool voiceOrPatchChanged = (mRunningVoice.mVoiceId != mv.mVoiceId) || (mRunningVoice.mSynthPatch != mv.mSynthPatch);
      if (voiceOrPatchChanged)
      {
        // init synth patch.
        mOsc.waveform(1, (uint8_t)mPreset->mOsc1Waveform);
        mOsc.waveform(2, (uint8_t)mPreset->mOsc2Waveform);
        mOsc.waveform(3, (uint8_t)mPreset->mOsc3Waveform);

        mOsc.pulseWidth(1, mPreset->mOsc1PulseWidth);
        mOsc.pulseWidth(2, mPreset->mOsc2PulseWidth);
        mOsc.pulseWidth(3, mPreset->mOsc3PulseWidth);

        mOsc.removeNote();
        mOsc.addNote(); // this enables portamento. we never need to do note-offs because this synth just plays a continuous single note.
      }

      if (mv.IsPlaying())
      {
        mOsc.amplitude(1, mPreset->mOsc1Gain);
        mOsc.amplitude(2, mPreset->mOsc2Gain);
        mOsc.amplitude(3, mPreset->mOsc3Gain);
      }
      else
      {
        mOsc.amplitude(1, 0.0);
        mOsc.amplitude(2, 0.0);
        mOsc.amplitude(3, 0.0);
      }

      auto transition = CalculateTransitionEvents(mRunningVoice, mv);
      if (transition.mNeedsNoteOn)
      {
        mOsc.addNote(); // this enables portamento. we never need to do note-offs because this synth just plays a continuous single note.
      }
      else if (transition.mNeedsNoteOff)
      {
        mOsc.removeNote();
      }

      // update
      float midiNote = (float)mv.mMidiNote + mv.mPitchBendN11.GetFloatVal() * pitchBendRange;

      mOsc.portamentoTime(1, mPreset->mPortamentoTime);
      mOsc.portamentoTime(2, mPreset->mPortamentoTime);
      mOsc.portamentoTime(3, mPreset->mPortamentoTime);

      mOsc.frequency(1, MIDINoteToFreq(midiNote + mPreset->mOsc1PitchFine + mPreset->mOsc1PitchSemis - mPreset->mDetune));
      mOsc.frequency(3, MIDINoteToFreq(midiNote + mPreset->mOsc3PitchFine + mPreset->mOsc3PitchSemis + mPreset->mDetune));

      if (mPreset->mSync)
      {
        float freq = MIDINoteToFreq(midiNote + mPreset->mOsc2PitchFine + mPreset->mOsc2PitchSemis);
        float freqSync = map(mv.mBreath01.GetFloatVal(), 0.0f, 1.0f, freq * mPreset->mSyncMultMin, freq * mPreset->mSyncMultMax);
        mOsc.frequency(2, freqSync);
      }
      else
      {
        mOsc.frequency(2, MIDINoteToFreq(midiNote + mPreset->mOsc2PitchFine + mPreset->mOsc2PitchSemis));
      }

      // perform breath & key tracking for filter. we will basically multiply the effects.
      float filterFreq = CalcFilterCutoffFreq(
          mv.mBreath01.GetFloatVal(),
          midiNote,
          mPreset->mFilterKeytracking,
          mPreset->mFilterMinFreq,
          mPreset->mFilterMaxFreq);

      // SV filt...
      //mFilter.frequency(filterFreq);
      //mFilter.resonance(mPreset->mFilterQ); // 0.7 to 5.0

      // biquad...
      // for (uint32_t i = 0; i < 4; ++ i) {
      //   mFilter.setLowpass(i, filterFreq, mPreset->mFilterQ);
      // }

      // moog
      mFilter.SetFrequencyAndQ(filterFreq, mPreset->mFilterQ);

      mRunningVoice = mv;
    }

    bool IsPlaying() const
    {
      // this function lets us delay for example, if there's a release stage (theoretically)
      return mRunningVoice.IsPlaying();
    }

    void Unassign()
    {
      mRunningVoice.mVoiceId = MAGIC_VOICE_ID_UNASSIGNED;
    }

    Voice(int16_t vid, AudioMixer4 &dest, int destPort) : mPatchOsc1ToMix(mOsc, 0, mOscMixer, 0),
                                                          mPatchOsc2ToMix(mOsc, 1, mOscMixer, 1),
                                                          mPatchOsc3ToMix(mOsc, 2, mOscMixer, 2),
                                                          mPatchMixToFilter(mOscMixer, 0, mFilter, 0),
                                                          mPatchOut(mFilter, 0, dest, destPort) //,
    {
    }
  };

  Voice gVoices[MAX_SYNTH_VOICES] =
      {
          {0, CCSynthGraph::voiceMix1, 0},
          {1, CCSynthGraph::voiceMix1, 1},
          {2, CCSynthGraph::voiceMix1, 2},
          {3, CCSynthGraph::voiceMix1, 3}, // 4
          {4, CCSynthGraph::voiceMix2, 0},
          {5, CCSynthGraph::voiceMix2, 1},
          // { 6, CCSynthGraph::voiceMix2, 2 },
          //{ 7, CCSynthGraph::voiceMix2, 3 }, // 8
          //    { 8, mix3, 0 },
          //    { 9, mix3, 1 },
          //    { 10, mix3, 2 },
          //    { 11, mix3, 3 }, // 12
          //    { 12, mix4, 0 },
          //    { 13, mix4, 1 },
          //    { 14, mix4, 2 },
          //    { 15, mix4, 3 },
  };

  struct SynthGraphControl
  {
    float mPrevMetronomeBeatFrac = 0;
    AppSettings *mAppSettings;
    Metronome *mMetronome;

    void Setup(AppSettings *appSettings, Metronome *metronome)
    {
      //AudioMemory(AUDIO_MEMORY_TO_ALLOCATE);
      AudioStream::initialize_memory(CLARINOID_AUDIO_MEMORY, SizeofStaticArray(CLARINOID_AUDIO_MEMORY));

      mAppSettings = appSettings;
      mMetronome = metronome;

      // for some reason patches really don't like to connect unless they are
      // last in the initialization order. Here's a workaround to force them to connect.
      for (auto &v : gVoices)
      {
        v.EnsurePatchConnections(appSettings);
      }

      //CCSynthGraph::audioShield.enable();
      //CCSynthGraph::audioShield.volume(.9); // headphone vol
      CCSynthGraph::ampLeft.gain(1);
      //CCSynthGraph::ampRight.gain(.9);
      delay(300); // why?

      CCSynthGraph::metronomeEnv.delay(0);
      CCSynthGraph::metronomeEnv.attack(0);
      CCSynthGraph::metronomeEnv.hold(0);
      CCSynthGraph::metronomeEnv.releaseNoteOn(0);
      CCSynthGraph::metronomeEnv.sustain(0);
    }

    // void SetGain(float f) {
    //   //Serial.println(String("SetGain: ") + f);
    //   CCSynthGraph::ampLeft.gain(f);
    //   //CCSynthGraph::ampRight.gain(f);
    // }

    void BeginUpdate()
    {
      //AudioNoInterrupts();// https://www.pjrc.com/teensy/td_libs_AudioProcessorUsage.html
    }

    void EndUpdate()
    {
      //AudioInterrupts();
    }

    void UpdatePostFx()
    {
      CCSynthGraph::verb.roomsize(.6f);
      CCSynthGraph::verb.damping(.7f);
      CCSynthGraph::verbWetAmpLeft.gain(mAppSettings->mSynthSettings.mReverbGain);
      CCSynthGraph::ampLeft.gain(mAppSettings->mSynthSettings.mMasterGain);

      if (!mAppSettings->mMetronomeOn)
      {
        CCSynthGraph::metronomeOsc.amplitude(0);
      }
      else
      {
        CCSynthGraph::metronomeEnv.decay(mAppSettings->mMetronomeDecayMS);
        CCSynthGraph::metronomeOsc.amplitude(mAppSettings->mMetronomeGain);
        CCSynthGraph::metronomeOsc.frequency(MIDINoteToFreq(mAppSettings->mMetronomeNote));

        float metronomeBeatFrac = mMetronome->GetBeatFrac();
        if (metronomeBeatFrac < mPrevMetronomeBeatFrac)
        { // beat boundary is when the frac drops back to 0
          CCSynthGraph::metronomeEnv.noteOn();
        }
        mPrevMetronomeBeatFrac = metronomeBeatFrac;
      }
    }
  };

  SynthGraphControl gSynthGraphControl;

} // namespace clarinoid
