
#pragma once

#include "Test.hpp"
#include "AudioFile.h"
#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/basic/Music.hpp>
#include <clarinoid/synth/ModulationMatrixNode.hpp>
using namespace clarinoid;


void FillAudioBufferRamp(audio_block_t& buf16, audio_block_t& buf32, float a, float b)
{
  for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
    float p = float(i);
    p /= AUDIO_BLOCK_SAMPLES;
    buf16.data[i] = buf32.data[i] = clarinoid::fast::Sample32To16(clarinoid::Remap01ToRange(p, a, b));
  }
}

void FillAudioBufferRamp(audio_block_t& buf, float a, float b)
{
  for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
    float p = float(i);
    p /= AUDIO_BLOCK_SAMPLES;
    buf.data[i] = clarinoid::fast::Sample32To16(clarinoid::Remap01ToRange(p, a, b));
  }
}
// asserts that a buffer conforms to the given ramp. returns the delta.
float GetDistanceToRamp(audio_block_t& buf1, audio_block_t& buf2, float a, float b)
{
  float err = 0;
  for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
    float p = float(i);
    p /= AUDIO_BLOCK_SAMPLES;
    float correctVal = Clamp(clarinoid::Remap01ToRange(p, a, b), -1, 1);
    err += fabsf(clarinoid::fast::Sample16To32(buf1.data[i]) - correctVal) / AUDIO_BLOCK_SAMPLES;
    err += fabsf(clarinoid::fast::Sample16To32(buf2.data[i]) - correctVal) / AUDIO_BLOCK_SAMPLES;
  }
  return err;
}

float GetDistanceFromConstant(audio_block_t& buf1, float c)
{
  float err = 0;
  float correctVal = Clamp(c, -1, 1);
  for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
    err += fabsf(clarinoid::fast::Sample16To32(buf1.data[i]) - correctVal) / AUDIO_BLOCK_SAMPLES;
  }
  return err;
}

// asserts that a buffer conforms to the given ramp. returns the delta.
float GetDistanceToRamp(audio_block_t& buf1, float a, float b)
{
  float err = 0;
  for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
    float p = float(i);
    p /= AUDIO_BLOCK_SAMPLES;
    float correctVal = Clamp(clarinoid::Remap01ToRange(p, a, b), -1, 1);
    err += fabsf(clarinoid::fast::Sample16To32(buf1.data[i]) - correctVal) / AUDIO_BLOCK_SAMPLES;
  }
  return err;
}


struct StaticProvider :clarinoid::IModulationKRateProvider
{
  float mBreath = 0;
  float mPitch = 0;
  virtual float IModulationProvider_GetKRateModulationSourceValueN11(clarinoid::KRateModulationSource src) override {
    switch (src)
    {
    case clarinoid::KRateModulationSource::Breath:
      return mBreath;
    default:
    case clarinoid::KRateModulationSource::PitchStrip:
      break;
    }
    return mPitch;

  }

  float osc1Amp = 0;
  float osc1Freq = 0;
  float osc2Amp = 0;
  float osc2Freq = 0;
  float osc3Amp = 0;
  float osc3Freq = 0;
  float voiceFilterCutoff = 0;

  virtual void IModulationProvider_SetKRateModulationDestinationValueN11(clarinoid::KRateModulationDestination d, float val) {
    switch (d) {
    case clarinoid::KRateModulationDestination::Osc1Amplitude:
      osc1Amp = val;
      return;
    case clarinoid::KRateModulationDestination::Osc1Frequency:
      osc1Freq = val;
      return;
    case clarinoid::KRateModulationDestination::Osc2Amplitude:
      osc2Amp = val;
      return;
    case clarinoid::KRateModulationDestination::Osc2Frequency:
      osc2Freq = val;
      return;
    case clarinoid::KRateModulationDestination::Osc3Amplitude:
      osc3Amp = val;
      return;
    case clarinoid::KRateModulationDestination::Osc3Frequency:
      osc3Freq = val;
      return;
    case clarinoid::KRateModulationDestination::VoiceFilterCutoff:
      voiceFilterCutoff = val;
      return;
    }
  }
};


void TestPolarityMapping()
{
  auto ConvertPolarityOfBuffers = [&](audio_block_t& buf16, audio_block_t& buf32, ModulationPoleType srcPolarity, ModulationPolarityTreatment treatment) {
    auto c32 = VoiceModulationMatrixNode::GetPolarityConversion<VoiceModulationMatrixNode::PolarityConversionKernelFloat>(srcPolarity, treatment);
    for (auto& s : buf32.data) {
      s = fast::Sample32To16(c32.Transfer(fast::Sample16To32(s)));
    }
    auto c16 = VoiceModulationMatrixNode::GetPolarityConversion<VoiceModulationMatrixNode::PolarityConversionKernel15p16>(srcPolarity, treatment);
    for (auto& s : buf16.data) {
      s = c16.Transfer(s);
    }
  };
  audio_block_t buf16;
  audio_block_t buf32;
  float eps = 0.0001f;
  // N11 - N11 and inverted
  {
    FillAudioBufferRamp(buf16, buf32, -1, 1);
    TestFloatClose(GetDistanceToRamp(buf16, buf32, -1, 1), 0, eps);
    ConvertPolarityOfBuffers(buf16, buf32, ModulationPoleType::N11, ModulationPolarityTreatment::AsBipolar);
    TestFloatClose(GetDistanceToRamp(buf16, buf32, -1, 1), 0, eps);
    ConvertPolarityOfBuffers(buf16, buf32, ModulationPoleType::N11, ModulationPolarityTreatment::AsBipolarInverted);
    TestFloatClose(GetDistanceToRamp(buf16, buf32, 1, -1), 0, eps);
  }
  // N11 - POSITIVE
  {
    FillAudioBufferRamp(buf16, buf32, -1, 1);
    ConvertPolarityOfBuffers(buf16, buf32, ModulationPoleType::N11, ModulationPolarityTreatment::AsPositive01);
    TestFloatClose(GetDistanceToRamp(buf16, buf32, 0, 1), 0, eps);
  }
  // N11 - POSITIVE inverted
  {
    FillAudioBufferRamp(buf16, buf32, -1, 1);
    ConvertPolarityOfBuffers(buf16, buf32, ModulationPoleType::N11, ModulationPolarityTreatment::AsPositive01Inverted);
    TestFloatClose(GetDistanceToRamp(buf16, buf32, 1, 0), 0, eps);
  }

  // positive - N11
  {
    FillAudioBufferRamp(buf16, buf32, 0, 1);
    TestFloatClose(GetDistanceToRamp(buf16, buf32, 0, 1), 0, eps);
    ConvertPolarityOfBuffers(buf16, buf32, ModulationPoleType::Positive01, ModulationPolarityTreatment::AsBipolar);
    TestFloatClose(GetDistanceToRamp(buf16, buf32, -1, 1), 0, eps);
  }

  // positive - N11 inverted
  {
    FillAudioBufferRamp(buf16, buf32, 0, 1);
    TestFloatClose(GetDistanceToRamp(buf16, buf32, 0, 1), 0, eps);
    ConvertPolarityOfBuffers(buf16, buf32, ModulationPoleType::Positive01, ModulationPolarityTreatment::AsBipolarInverted);
    TestFloatClose(GetDistanceToRamp(buf16, buf32, 1, -1), 0, eps);
  }

  // positive - positive & positive inv
  {
    FillAudioBufferRamp(buf16, buf32, 0, 1);
    TestFloatClose(GetDistanceToRamp(buf16, buf32, 0, 1), 0, eps);
    ConvertPolarityOfBuffers(buf16, buf32, ModulationPoleType::Positive01, ModulationPolarityTreatment::AsPositive01);
    TestFloatClose(GetDistanceToRamp(buf16, buf32, 0, 1), 0, eps);
    ConvertPolarityOfBuffers(buf16, buf32, ModulationPoleType::Positive01, ModulationPolarityTreatment::AsPositive01Inverted);
    TestFloatClose(GetDistanceToRamp(buf16, buf32, 1, 0), 0, eps);
  }
}

void TestModulationCurveMapping()
{
  const float eps = 0.0001f * AUDIO_BLOCK_SAMPLES; // high epsilon because it's over the course of an entire buffer.
  audio_block_t buf;
  int ylin = gModCurveLUT.LinearYIndex;

  // assumes it should be a ramp -1 to 1, and calculates a value that represents the shape of the curve
  auto CalculateRampShape = [&]() {
    float ret = 0;
    for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES / 2; ++i) {
      //float p = i;
      //p /= AUDIO_BLOCK_SAMPLES;
      //p = p * 2 - 1;
      // assumes the first half of the buffer is negative
      ret += -fast::Sample16To32(buf.data[i]);
      ret += fast::Sample16To32(buf.data[AUDIO_BLOCK_SAMPLES - i - 1]); // and 2nd half is positive
    }
    return ret;
  };

  // test linearity of 0
  float linearSum = 0.0f;
  float linearShape = 0;
  {
    FillAudioBufferRamp(buf, -1, 1);
    auto state = gModCurveLUT.BeginLookupI(ylin);
    for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
      buf.data[i] = gModCurveLUT.Transfer16(buf.data[i], state);
      linearSum += fast::Sample16To32(buf.data[i]);
    }
    TestFloatClose(linearSum, 0, 1.1f); // the total sum should be roughly balanced across poles, but it's hard to balance that.
    TestFloatClose(GetDistanceToRamp(buf, -1, 1), 0, eps);
    linearShape = CalculateRampShape();
  }

  float neg1Shape = 0;
  {
    float yminus1sum = 0;
    // test that this is the correct relative shape to linear
    FillAudioBufferRamp(buf, -1, 1);
    auto state = gModCurveLUT.BeginLookupI(ylin - 1);// gModCurveLUT.LutSizeY * 0.5f );
    for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
      buf.data[i] = gModCurveLUT.Transfer16(buf.data[i], state);
      yminus1sum += fast::Sample16To32(buf.data[i]);
    }
    TestFloatClose(yminus1sum, 0, 1.1f); // the total sum should be roughly balanced across poles, but it's hard to balance that.
    neg1Shape = CalculateRampShape();
  }

  float pos1Shape = 0;
  {
    float yplus1sum = 0;
    // test that this is the correct relative shape to linear
    FillAudioBufferRamp(buf, -1, 1);
    auto state = gModCurveLUT.BeginLookupI(ylin + 1);// gModCurveLUT.LutSizeY * 0.5f );
    for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
      buf.data[i] = gModCurveLUT.Transfer16(buf.data[i], state);
      yplus1sum += fast::Sample16To32(buf.data[i]);
    }
    TestFloatClose(yplus1sum, 0, 1.1f); // the total sum should be roughly balanced across poles, but it's hard to balance that.
    pos1Shape = CalculateRampShape();
  }

  Test(pos1Shape > linearShape);
  Test(neg1Shape < linearShape);

  // test that extents are always extents, and zero is always 0.
  for (size_t i = 0; i < gModCurveLUT.LutSizeY; ++i) {
    auto state = gModCurveLUT.BeginLookupI(i);
    float v;
    v = gModCurveLUT.Transfer32(-1, state);
    TestFloatClose(v, -1, 0.01f); // these are curves; we can be pretty loose. 0.01 is about -40db snr.
    v = gModCurveLUT.Transfer32(0, state);
    TestFloatClose(v, 0, 0.01f);
    v = gModCurveLUT.Transfer32(1, state);
    TestFloatClose(v, 1, 0.01f);
  }
}

// also checks value saturation
void TestCombiningModulations1()
{
  VoiceModulationMatrixNode m;
  SynthPreset patch;
  StaticProvider provider;
  m.SetSynthPatch(&patch, &provider);

  // Osc1PulseWidth being modulated by both LFO1 and LFO2. (a-rate to a-rate)

  patch.mModulations[0].mSource = AnyModulationSource::LFO1;
  patch.mModulations[0].mSourcePolarity = ModulationPolarityTreatment::AsPositive01;
  patch.mModulations[0].mDest = AnyModulationDestination::Osc1PulseWidth;
  patch.mModulations[0].mScaleN11 = 0.8f;

  patch.mModulations[1].mSource = AnyModulationSource::LFO2;
  patch.mModulations[1].mSourcePolarity = ModulationPolarityTreatment::AsBipolar;
  patch.mModulations[1].mDest = AnyModulationDestination::Osc1PulseWidth;
  patch.mModulations[1].mScaleN11 = 0.6f;

  TestResetAudioStreams();

  // give LFOs ramps -1 to 1
  FillAudioBufferRamp(gTestSrcBuffers[GetModulationSourceInfo(AnyModulationSource::LFO1).mIndexForRate], -1, 1);
  FillAudioBufferRamp(gTestSrcBuffers[GetModulationSourceInfo(AnyModulationSource::LFO2).mIndexForRate], -1, 1);

  m.update();

  // test Osc1PulseWidth
  // i'm expecting LFO1 polarity to be scaled to 0-1, then * scale to 0,0.8
  // LFO2 -1,1 -> -1,1 then *scale to -.6,+.6.
  // and they're just added together, so the ramp will be -.6,+1.40
  auto& tb = gTestDestBuffers[GetModulationDestinationInfo(AnyModulationDestination::Osc1PulseWidth).mIndexForRate];
  float f = GetDistanceToRamp(tb, -.6f, 1.40f);
  TestFloatClose(f, 0, 0.02f);
}


// combines k-rate with a-rate -> arate
void TestCombiningModulations2()
{
  VoiceModulationMatrixNode m;
  SynthPreset patch;
  StaticProvider provider;
  m.SetSynthPatch(&patch, &provider);

  // Osc1PulseWidth being modulated by both LFO1 and LFO2. (a-rate to a-rate)
  // want to make sure that when combined with krate, we still get arate.

  patch.mModulations[0].mSource = AnyModulationSource::Breath;
  patch.mModulations[0].mSourcePolarity = ModulationPolarityTreatment::AsPositive01;
  patch.mModulations[0].mDest = AnyModulationDestination::Osc1PulseWidth;
  patch.mModulations[0].mScaleN11 = 0.8f;

  patch.mModulations[1].mSource = AnyModulationSource::LFO2;
  patch.mModulations[1].mSourcePolarity = ModulationPolarityTreatment::AsBipolar;
  patch.mModulations[1].mDest = AnyModulationDestination::Osc1PulseWidth;
  patch.mModulations[1].mScaleN11 = 0.6f;

  TestResetAudioStreams();

  // fill modulation signals
  provider.mBreath = 0.66f;
  // give LFOs ramps -1 to 1
  FillAudioBufferRamp(gTestSrcBuffers[GetModulationSourceInfo(AnyModulationSource::LFO2).mIndexForRate], -1, 1);

  m.update();

  // test Osc1PulseWidth
  // i'm expecting BREATH is 0.66 * 0.8 = .528
  // LFO2 -1,1 -> -1,1 then *scale to -.6,+.6.
  // and they're just added together, so the ramp will be -0.072 - 1.128
  auto& tb = gTestDestBuffers[GetModulationDestinationInfo(AnyModulationDestination::Osc1PulseWidth).mIndexForRate];
  float f = GetDistanceToRamp(tb, -0.072f, 1.128f);
  TestFloatClose(f, 0, 0.01f);
}

void TestAux_KAK()
{
  VoiceModulationMatrixNode m;
  SynthPreset patch;
  StaticProvider provider;
  m.SetSynthPatch(&patch, &provider);

  patch.mModulations[0].mSource = AnyModulationSource::Breath;
  patch.mModulations[0].mSourcePolarity = ModulationPolarityTreatment::AsPositive01;
  patch.mModulations[0].mDest = AnyModulationDestination::Osc1PulseWidth;
  patch.mModulations[0].mScaleN11 = 0.8f;

  patch.mModulations[0].mAuxSource = AnyModulationSource::PitchStrip;
  patch.mModulations[0].mAuxPolarity = ModulationPolarityTreatment::AsPositive01Inverted;
  patch.mModulations[0].mAuxAmount01 = 0.5f;

  TestResetAudioStreams();

  // fill modulation signals
  provider.mBreath = 0.66f;
  provider.mPitch = -0.75f;

  m.update();

  // i'm expecting breath .66 * 0.8 = +0.528
  // then, aux: pitch is -.75 (bipolar), and flipped to positive01 inverted.
  // inverted bipolar would be +.75, and compressed to positive is +.875
  // then * scale .5 = .4375
  // .528 * .4375 = .462
  auto& tb = gTestDestBuffers[GetModulationDestinationInfo(AnyModulationDestination::Osc1PulseWidth).mIndexForRate];
  float d = GetDistanceFromConstant(tb, 0.231f);
  TestFloatClose(d, 0, 0.01f);
}

void TestAux_AAK() {
  VoiceModulationMatrixNode m;
  SynthPreset patch;
  StaticProvider provider;
  m.SetSynthPatch(&patch, &provider);

  patch.mModulations[0].mSource = AnyModulationSource::LFO1;
  patch.mModulations[0].mSourcePolarity = ModulationPolarityTreatment::AsBipolar;
  patch.mModulations[0].mDest = AnyModulationDestination::Osc1PulseWidth;
  patch.mModulations[0].mScaleN11 = 0.8f;

  patch.mModulations[0].mAuxSource = AnyModulationSource::PitchStrip;
  patch.mModulations[0].mAuxPolarity = ModulationPolarityTreatment::AsBipolar;
  patch.mModulations[0].mAuxAmount01 = 0.5f;

  TestResetAudioStreams();

  // fill modulation signals
  FillAudioBufferRamp(gTestSrcBuffers[GetModulationSourceInfo(AnyModulationSource::LFO1).mIndexForRate], -1, 1);
  provider.mPitch = -0.75f;

  m.update();

  // LFO1 is a ramp from -1 to 1, no polarity conversion. * scale = -.8,+.8
  // AUX pitch of -.75, scaled .5 = -.375
  // so ramp * aux = +.3,-.3
  auto& tb = gTestDestBuffers[GetModulationDestinationInfo(AnyModulationDestination::Osc1PulseWidth).mIndexForRate];
  float buf32[AUDIO_BLOCK_SAMPLES];
  fast::Sample16To32Buffer(tb.data, buf32);
  float d = GetDistanceToRamp(tb, 0.3f, -0.3f);
  TestFloatClose(d, 0, 0.01f);
}

void TestAux_AAA() {
  VoiceModulationMatrixNode m;
  SynthPreset patch;
  StaticProvider provider;
  m.SetSynthPatch(&patch, &provider);

  patch.mModulations[0].mSource = AnyModulationSource::LFO1;
  patch.mModulations[0].mSourcePolarity = ModulationPolarityTreatment::AsBipolar;
  patch.mModulations[0].mDest = AnyModulationDestination::Osc1PulseWidth;
  patch.mModulations[0].mScaleN11 = 0.8f;

  patch.mModulations[0].mAuxSource = AnyModulationSource::LFO2;
  patch.mModulations[0].mAuxPolarity = ModulationPolarityTreatment::AsBipolar;
  patch.mModulations[0].mAuxAmount01 = 0.5f;

  TestResetAudioStreams();

  // fill modulation signals
  FillAudioBufferRamp(gTestSrcBuffers[GetModulationSourceInfo(AnyModulationSource::LFO1).mIndexForRate], -1, 1);
  FillAudioBufferRamp(gTestSrcBuffers[GetModulationSourceInfo(AnyModulationSource::LFO2).mIndexForRate], -1, 1);

  m.update();

  // LFO1 is -1,1 *.8 = -.8,+.8
  // aux LFO2 is        -.5,+.5
  // when multiplied, taht's going to be +.4 -> 0 -> +.4
  auto& tb = gTestDestBuffers[GetModulationDestinationInfo(AnyModulationDestination::Osc1PulseWidth).mIndexForRate];
  float buf32[AUDIO_BLOCK_SAMPLES];
  fast::Sample16To32Buffer(tb.data, buf32);

  float err = 0;
  for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
    float refX = RemapToRange(float(i), 0, AUDIO_BLOCK_SAMPLES, -.8f, .8f);
    float refAux = RemapToRange(float(i), 0, AUDIO_BLOCK_SAMPLES, -.5f, .5f);
    float ref = refX * refAux;
    err += fabs(ref - buf32[i]) / AUDIO_BLOCK_SAMPLES;
  }

  TestFloatClose(err, 0, 0.01f);
}

void TestModulation()
{
  TestFloatEq(clarinoid::fast::Sample32To16(-2.0f), -32768);
  TestFloatEq(clarinoid::fast::Sample32To16(-1.0f), -32767);
  TestFloatEq(clarinoid::fast::Sample32To16(-.2f), -6553);
  TestFloatEq(clarinoid::fast::Sample32To16(.2f), 6553);
  TestFloatEq(clarinoid::fast::Sample32To16(0), 0);
  TestFloatEq(clarinoid::fast::Sample32To16(1.0f), 32767);
  TestFloatEq(clarinoid::fast::Sample32To16(2.0f), 32767);

  TestFloatEq(clarinoid::fast::Sample16To32(std::numeric_limits<int16_t>::min()), -1.0f);
  TestFloatEq(clarinoid::fast::Sample16To32(0), 0);
  TestFloatEq(clarinoid::fast::Sample16To32(std::numeric_limits<int16_t>::max()), 1.0f);


  clarinoid::VoiceModulationMatrixNode m;
  clarinoid::SynthPreset patch;
  patch.mModulations[0].mSource = clarinoid::AnyModulationSource::Breath;
  patch.mModulations[0].mDest = clarinoid::AnyModulationDestination::VoiceFilterCutoff;
  patch.mModulations[0].mScaleN11 = 0.3f;

  // test k-rate to a-rate
  patch.mModulations[1].mSource = clarinoid::AnyModulationSource::Breath;
  patch.mModulations[1].mDest = clarinoid::AnyModulationDestination::Osc1Phase; // index 1, breath=0.2 to osc1phase, * -1.0f = -6553
  patch.mModulations[1].mScaleN11 = -1.0f;

  // test a-rate to a-rate
  patch.mModulations[3].mSource = clarinoid::AnyModulationSource::Osc2FB;
  patch.mModulations[3].mSourcePolarity = clarinoid::ModulationPolarityTreatment::AsBipolar;
  patch.mModulations[3].mDest = clarinoid::AnyModulationDestination::Osc3Phase; // index 5, osc2fb=12000 (.366) * 0.85 to osc2phase = ~ 10200
  patch.mModulations[3].mScaleN11 = 0.85f;

  StaticProvider provider;
  provider.mBreath = 0.2f;
  m.SetSynthPatch(&patch, &provider);
  TestResetAudioStreams();
  // set up some a-rate buffers.
  clarinoid::fast::FillBufferWithConstant(12000, gTestSrcBuffers[clarinoid::GetModulationSourceInfo(clarinoid::AnyModulationSource::Osc2FB).mIndexForRate].data);
  m.update();
  TestFloatEq(provider.voiceFilterCutoff, 0.3f * 0.2f); // test k-rate to k-rate.

// test k-rate to a-rate
  TestFloatEq(gTestTransmittedBuffers[clarinoid::GetModulationDestinationInfo(clarinoid::AnyModulationDestination::Osc1Phase).mIndexForRate].data[0], -6553);
  TestFloatEq(gTestTransmittedBuffers[clarinoid::GetModulationDestinationInfo(clarinoid::AnyModulationDestination::Osc1Phase).mIndexForRate].data[127], -6553);

  // test a-rate to a-rate
  TestFloatEq(gTestTransmittedBuffers[clarinoid::GetModulationDestinationInfo(clarinoid::AnyModulationDestination::Osc3Phase).mIndexForRate].data[0],
    10199);

  patch = clarinoid::SynthPreset();
  patch.mModulations[2].mSource = clarinoid::AnyModulationSource::Osc3FB;
  patch.mModulations[2].mSourcePolarity = clarinoid::ModulationPolarityTreatment::AsBipolar;
  patch.mModulations[2].mDest = clarinoid::AnyModulationDestination::VoiceFilterCutoff;
  patch.mModulations[2].mScaleN11 = -1.0f;
  m.SetSynthPatch(&patch, &provider);
  TestResetAudioStreams();
  clarinoid::fast::FillBufferWithConstant(-16384, gTestSrcBuffers[clarinoid::GetModulationSourceInfo(clarinoid::AnyModulationSource::Osc3FB).mIndexForRate].data);
  m.update();
  // test a-rate to k-rate
  Test(fabs(provider.voiceFilterCutoff - 0.5f) < 0.0001f);

  TestPolarityMapping();

  TestModulationCurveMapping();
  TestCombiningModulations1();
  TestCombiningModulations2();

  TestAux_KAK(); // also checks polarity mapping
  TestAux_AAK();
  TestAux_AAA();
}

