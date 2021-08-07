
#pragma once

#include "Test.hpp"
#include "AudioFile.h"
#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/basic/Music.hpp>
#include <clarinoid/synth/ModulationMatrixNode.hpp>

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
  patch.mModulations[2].mDest = clarinoid::AnyModulationDestination::VoiceFilterCutoff;
  patch.mModulations[2].mScaleN11 = -1.0f;
  m.SetSynthPatch(&patch, &provider);
  TestResetAudioStreams();
  clarinoid::fast::FillBufferWithConstant(-16384, gTestSrcBuffers[clarinoid::GetModulationSourceInfo(clarinoid::AnyModulationSource::Osc3FB).mIndexForRate].data);
  m.update();
  // test a-rate to k-rate
  Test(fabs(provider.voiceFilterCutoff - 0.5f) < 0.0001f);
}

