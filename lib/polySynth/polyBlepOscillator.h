/*
   Written by Florian Wirth, August 2020

   This is a module for the Teensy Audio Library which contains 3 independent
   oscillators with bandlimited basic waveforms, frequency modulation, PWM and hard-sync.

   The oscillators are based on two pieces of code from users at the kvraudio forum, namely:

   - The PolyBLEP algorithm is from the user "mystran" (https://www.kvraudio.com/forum/viewtopic.php?t=398553)
   - The hard-sync sawtooth is from the user "Tale" (https://www.kvraudio.com/forum/viewtopic.php?t=425054)

    The handling of portamento is from "Chip Audette's" OpenAudio_ArduinoLibrary
    (https://github.com/chipaudette/OpenAudio_ArduinoLibrary/blob/master/synth_waveform_F32.h)

    The frequency modulation uses Paul Stoffregens adaption of the exp2 algorithm from Laurent de Soras
    - Original algorithm (https://www.musicdsp.org/en/latest/Other/106-fast-exp2-approximation.html)
    - Pauls adaptation for Teensy (https://github.com/PaulStoffregen/Audio/blob/master/synth_waveform.cpp)

   This Software is published under the MIT License, use at your own risk
*/

#include <Audio.h>
#include "Arduino.h"
#include <arm_math.h>

#define BUFFERSIZE                  128





class AudioBandlimitedOsci : public AudioStream
{
  public:
    AudioBandlimitedOsci() : AudioStream(6, inputQueueArray) {
    }

    void amplitude(uint8_t oscillator, float a) {
      if (a < -1) {
        a = -1;
      } else if (a > 1.0) {
        a = 1.0;
      }
      switch (oscillator) {
        case 1:
          {
            osc1_gain = a;
          }
          break;

        case 2: {
            osc2_gain = a;
          }
          break;

        case 3: {
            osc3_gain = a;
          }
          break;
      }
    }

    void frequency(uint8_t oscillator, float freq) {
      if (freq < 0.0) {
        freq = 0.0;
      } else if (freq > AUDIO_SAMPLE_RATE_EXACT / 2) {
        freq = AUDIO_SAMPLE_RATE_EXACT / 2;
      }

      switch (oscillator) {
        case 1:
          { 
           
            if (osc1_portamentoSamples > 0 && notesPlaying > 0) {
              osc1_portamentoIncrement = (freq - frequency1) / osc1_portamentoSamples;
              osc1_currentPortamentoSample = 0;
            } else {
              frequency1 = freq;
            }
            
            
            osc1_dt = frequency1 / AUDIO_SAMPLE_RATE_EXACT;
          }
          break;

        case 2:
          {
            if (osc2_portamentoSamples > 0 && notesPlaying > 0) {
              osc2_portamentoIncrement = (freq - frequency2) / (float)osc2_portamentoSamples;
              osc2_currentPortamentoSample = 0;
            } else {
              frequency2 = freq;
            }
            osc2_dt = frequency2 / AUDIO_SAMPLE_RATE_EXACT;
          }
          break;

        case 3:
          {
            if (osc3_portamentoSamples > 0 && notesPlaying > 0) {
              osc3_portamentoIncrement = (freq - frequency3) / (float)osc3_portamentoSamples;
              osc3_currentPortamentoSample = 0;
            } else {
              frequency3 = freq;
            }
            osc3_dt = frequency3 / AUDIO_SAMPLE_RATE_EXACT;
          }
          break;
      }
    }

    void phase(uint8_t oscillator, float t) {
      /*
         Use this for phase-retrigger everytime you send a noteOn, if you want a consistent sound.
         Don't use this for hard-sync, it will cause aliasing.
         You can also set the phases of the oscillators to different starting points.
      */
      t -= floorf(t);

      switch (oscillator) {
        case 1: {
            osc1_t = t;
          }
          break;

        case 2: {
            osc2_t = t;
          }
          break;

        case 3: {
            osc3_t = t;
          }
          break;

      }
    }

    void waveform(uint8_t oscillator, uint8_t wform) {
      /*
         Following waveforms are available:

         osc1: sine(0), variable triangle(1), pulse(2)
         osc2: sine(0), variable triangle(1), pulse(2), hard-synced sawtooth(3)
         osc3: sine(0), variable triangle(1), pulse(2), hard-synced sawtooth(3)

         When choosing the synced saw for oscillator 2 or 3, oscillator 1 is always the master
         The hard-synced sawtooth does not have PWM
      */
      switch (oscillator) {
        case 1: {
            osc1_waveform = wform;
          }
          break;

        case 2: {
            osc2_waveform = wform;
          }
          break;

        case 3: {
            osc3_waveform = wform;
          }
          break;

      }
    }

    void pulseWidth(uint8_t oscillator, float pulseWidth) {
      //pulseWidth gets limited to [0.001, 0.999] later on
      switch (oscillator) {
        case 1: {
            pulseWidth1 = pulseWidth;
            osc1_pulseWidth = pulseWidth;
          }
          break;

        case 2: {
            pulseWidth2 = pulseWidth;
            osc2_pulseWidth = pulseWidth;
          }
          break;

        case 3: {
            pulseWidth3 = pulseWidth;
            osc3_pulseWidth = pulseWidth;
          }
          break;

      }
    }

    void portamentoTime(uint8_t oscillator, float seconds) {
      //set how long the oscillator sweeps up or down to the new frequency
      switch (oscillator) {
        case 1: {
            osc1_portamentoTime = seconds;
            osc1_portamentoSamples = floorf(seconds * AUDIO_SAMPLE_RATE_EXACT);
          }
          break;

        case 2: {
            osc2_portamentoTime = seconds;
            osc2_portamentoSamples = floorf(seconds * AUDIO_SAMPLE_RATE_EXACT);
          }
          break;

        case 3: {
            osc3_portamentoTime = seconds;
            osc3_portamentoSamples = floorf(seconds * AUDIO_SAMPLE_RATE_EXACT);
          }
          break;

      }
    }

    void fmAmount(uint8_t oscillator, float octaves) {
      if (octaves > 12.0) {
        octaves = 12.0;
      } else if (octaves < 0.1) {
        octaves = 0.1;
      }
      switch (oscillator) {
        case 1: {
            osc1_pitchModAmount = octaves * 4096.0;;
          }
          break;

        case 2: {
            osc2_pitchModAmount = octaves * 4096.0;;
          }
          break;

        case 3: {
            osc3_pitchModAmount = octaves * 4096.0;;
          }
          break;
      }
    }

    void pwmAmount(uint8_t oscillator, float amount) {
      switch (oscillator) {
        case 1: {
            osc1_pwmAmount = amount;
          }
          break;

        case 2: {
            osc2_pwmAmount = amount;
          }
          break;

        case 3: {
            osc3_pwmAmount = amount;
          }
          break;
      }
    }

    void addNote() {
      //Call with your "noteOn" when hitting a new note, this is needed for portamento
      notesPlaying++;
    }

    void removeNote() {
      //Call with your "noteOff" command
      if (notesPlaying > 0) {
        notesPlaying--;
      }
    }

    virtual void update(void);

    inline void osc1Step();
    inline void osc2Step();
    inline void osc2Sync(float x);
    inline void osc3Step();
    inline void osc3Sync(float x);


    audio_block_t *inputQueueArray[6];

    private:
    
    uint8_t notesPlaying;

    float frequency1 = 0;
    float pulseWidth1 = 0; 
    float osc1_freq = 0;
    float osc1_pulseWidth = 0.5;
    float osc1_gain = 0;
    uint8_t osc1_waveform = 0;
    float osc1_output = 0;
    float osc1_blepDelay = 0;
    float osc1_t = 0;
    float osc1_dt = 0;
    float osc1_widthDelay = 0;
    bool osc1_pulseStage = false;
    float osc1_pwmAmount = 0.5;
    uint32_t osc1_pitchModAmount = 4096;
    float osc1_portamentoTime = 0;
    float osc1_portamentoIncrement = 0;
    uint64_t osc1_portamentoSamples = 0;
    uint64_t osc1_currentPortamentoSample = 0;
    


    float frequency2 = 0;
    float pulseWidth2 = 0;
    float osc2_freq = 0;
    float osc2_pulseWidth = 0.5;
    float osc2_gain = 0;
    uint8_t osc2_waveform = 0;
    float osc2_output = 0;
    float osc2_blepDelay = 0;
    float osc2_t = 0;
    float osc2_dt = 0;
    float osc2_widthDelay = 0;
    bool osc2_pulseStage = false;
    float osc2_pwmAmount = 0.5;
    uint32_t osc2_pitchModAmount = 4096;
    float osc2_portamentoTime = 0;
    float osc2_portamentoIncrement = 0;
    uint64_t osc2_portamentoSamples = 0;
    uint64_t osc2_currentPortamentoSample = 0;
    

    float frequency3 = 0;
    float pulseWidth3 = 0;
    float osc3_freq = 0;
    float osc3_pulseWidth = 0.5;
    float osc3_gain = 0;
    uint8_t osc3_waveform = 0;
    float osc3_output = 0;
    float osc3_blepDelay = 0;
    float osc3_t = 0;
    float osc3_dt = 0;
    float osc3_widthDelay = 0;
    bool osc3_pulseStage = false;
    float osc3_pwmAmount = 0.5;
    uint32_t osc3_pitchModAmount = 4096;
    float osc3_portamentoTime = 0;
    float osc3_portamentoIncrement = 0;
    uint64_t osc3_portamentoSamples = 0;
    uint64_t osc3_currentPortamentoSample = 0;
    

};
