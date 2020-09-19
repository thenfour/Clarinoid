#include <Audio.h>
#include "polyBlepOscillator.h"

AudioSynthWaveform lfo1;
AudioSynthWaveform lfo2;
AudioSynthWaveform lfo3;
AudioSynthWaveform lfo4;
AudioSynthWaveform lfo5;


AudioBandlimitedOsci osci;

AudioMixer4 mixer;

AudioOutputI2S out;
AudioControlSGTL5000 audioShield;

AudioConnection mod1 (lfo1, 0, osci, 0);
AudioConnection mod2 (lfo2, 0, osci, 2);
AudioConnection mod3 (lfo3, 0, osci, 4);

AudioConnection mod4 (lfo4, 0, osci, 1);
AudioConnection mod5 (lfo5, 0, osci, 5);


AudioConnection a (osci, 0, mixer, 0);
AudioConnection b (osci, 1, mixer, 1);
AudioConnection c (osci, 2, mixer, 2);

AudioConnection d (mixer, 0, out, 0);
AudioConnection e (mixer, 0, out, 1);


void setup() {
  Serial.begin(9600);
  AudioMemory(20);
  audioShield.enable();
  audioShield.volume(0.6);

  
  lfo1.begin(WAVEFORM_SINE);
  lfo1.frequency(3);
  lfo1.amplitude(0.03);

  lfo2.begin(WAVEFORM_SAWTOOTH);
  lfo2.frequency(2);
  lfo2.amplitude(0.4);

  lfo3.begin(WAVEFORM_SINE);
  lfo3.frequency(0.2);
  lfo3.amplitude(1);

  lfo4.begin(WAVEFORM_SINE);
  lfo4.frequency(0.05);
  lfo4.amplitude(1);

  osci.waveform(1, 1);
  osci.waveform(2, 3);
  osci.waveform(3, 3);

  osci.pulseWidth(1, 0.5);
  osci.pulseWidth(2, 0.5);
  osci.pulseWidth(3, 0.5);
 
  osci.fmAmount(2, 2);
  osci.fmAmount(3, 2);

  osci.frequency(1, 100);
  osci.frequency(2, 500);
  osci.frequency(3, 600);

  osci.amplitude(1, 0.005);
  osci.amplitude(2, 0.003);
  osci.amplitude(3, 0.002);
  
}




void loop() {
  
  delay(1000);
  Serial.println("Processor usage Max: " + String(AudioProcessorUsageMax()));
  Serial.println("Processor usage of the osci: " + String(osci.processorUsageMax()));
  Serial.println("Memory usage Max:" + String(AudioMemoryUsageMax()));
  Serial.println();
  
}
