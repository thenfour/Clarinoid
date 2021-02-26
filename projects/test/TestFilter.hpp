
#pragma once

#include "Test.hpp"
#include "AudioFile.h"
#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/basic/Music.hpp>
#include <clarinoid/synth/filters/filters.hpp>

// https://github.com/adamstark/AudioFile
// https://github.com/HiFi-LoFi/AudioFFT
void TestFilter()
{
  using clarinoid::filters::Real;

  // K35
  // Oberheim SEM 12
  // Moog Ladder lp2, lp4, etc
  // Diode
  // VA one pole

  //clarinoid::filters::OnePoleFilter filt;
  //filt.SetType(clarinoid::filters::FilterType::LP);

  //clarinoid::filters::SEM12Filter filt; // always LP
  //filt.SetResonance(clarinoid::filters::Real(0.5));

  //clarinoid::filters::DiodeFilter filt; // always lowpass
  //filt.SetResonance(clarinoid::filters::Real(0.6));
  //filt.SetSaturation(clarinoid::filters::Real(0.4));

  //clarinoid::filters::K35Filter filt;
  //filt.SetType(clarinoid::filters::FilterType::LP);
  //filt.SetResonance(0.0);
  //filt.SetSaturation(0.0);

  clarinoid::filters::MoogLadderFilter filt;
  filt.SetType(clarinoid::filters::FilterType::BP4);
  filt.SetResonance(2);
  filt.SetSaturation(Real(0.5));


  AudioFile<double> audioFile;
  std::string path = "C:\\root\\git\\thenfour\\Clarinoid\\trash\\untitled.wav";
  audioFile.load(path);
  //int channels = audioFile.getNumChannels();
  int numSamples = audioFile.getNumSamplesPerChannel();

  AudioFile<double>::AudioBuffer outpBuffer;
  outpBuffer.resize(2);
  outpBuffer[0].resize(numSamples);
  outpBuffer[1].resize(numSamples);

  double modVal01 = 0.0;
  for (int i = 0; i < numSamples; i++)
  {
    if ((i % 128) == 0) {
      // adjust filter cutoff
      modVal01 = cos((double)i / numSamples * clarinoid::filters::PITimes2 * 2)*.5 + .5;
      //filt.setSaturation(p * 3.0);
      filt.SetCutoffFrequency(clarinoid::filters::Real(modVal01 * 1500));
    }
    outpBuffer[1][i] = modVal01 / 3.0;
    double x = audioFile.samples[0][i];
    double y = (double)filt.ProcessSample(clarinoid::filters::Real(x));
    outpBuffer[0][i] = y * .5;
  }

  audioFile.setAudioBuffer(outpBuffer);
  audioFile.save(path + "-outp.wav");


}

