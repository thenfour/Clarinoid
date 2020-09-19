#include "polyBlepOscillator.h"



void AudioBandlimitedOsci::update() {
  audio_block_t *out1;
  audio_block_t *out2;
  audio_block_t *out3;
  audio_block_t * fm1;
  audio_block_t * pwm1;
  audio_block_t * fm2;
  audio_block_t * pwm2;
  audio_block_t * fm3;
  audio_block_t * pwm3;

  out1 = allocate();
  if (!out1) {
    return;
  }
  out2 = allocate();
  if (!out2) {
    return;
  }
  out3 = allocate();
  if (!out3) {
    return;
  }

  fm1 = receiveReadOnly(0);
  pwm1 = receiveReadOnly(1);
  fm2 = receiveReadOnly(2);
  pwm2 = receiveReadOnly(3);
  fm3 = receiveReadOnly(4);
  pwm3 = receiveReadOnly(5);

  for (uint16_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {


    //oscillator 1:
    if (osc1_portamentoSamples > 0 && osc1_currentPortamentoSample++ < osc1_portamentoSamples) {
      frequency1 += osc1_portamentoIncrement;
    }

    //frequency Modulation:
    if (fm1 && osc1_pitchModAmount > 0) {
      int32_t n = fm1->data[i] * osc1_pitchModAmount;
      int32_t ipart = n >> 27;
      n = n & 0x7FFFFFF;
      n = (n + 134217728) << 3;
      n = multiply_32x32_rshift32_rounded(n, n);
      n = multiply_32x32_rshift32_rounded(n, 715827883) << 3;
      n = n + 715827882;

      uint32_t scale = n >> (15 - ipart);
      osc1_freq = frequency1 * scale * 0.00003051757;
    } else {
      osc1_freq = frequency1;
    }

    osc1_dt = osc1_freq / AUDIO_SAMPLE_RATE_EXACT;

    //pulse Width Modulation:
    if (pwm1) {
      osc1_pulseWidth = pulseWidth1 + ((float)pwm1->data[i] * 0.00003051757) * osc1_pwmAmount;
    }
    if (osc1_pulseWidth < 0.001) {
      osc1_pulseWidth = 0.001;
    } else if (osc1_pulseWidth > 0.999) {
      osc1_pulseWidth = 0.999;
    }

    //oscillator 2:
    if (osc2_portamentoSamples > 0 && osc2_currentPortamentoSample++ < osc2_portamentoSamples) {
      frequency2 += osc2_portamentoIncrement;
    }

    //frequency Modulation:
    if (fm2 && osc2_pitchModAmount > 0.0f) {

      int32_t n = fm2->data[i] * osc2_pitchModAmount;
      int32_t ipart = n >> 27;
      n = n & 0x7FFFFFF;
      n = (n + 134217728) << 3;
      n = multiply_32x32_rshift32_rounded(n, n);
      n = multiply_32x32_rshift32_rounded(n, 715827883) << 3;
      n = n + 715827882;

      uint32_t scale = n >> (15 - ipart);
      osc2_freq = frequency2 * scale * 0.00003051757;
    } else {
      osc2_freq = frequency2;
    }

    osc2_dt = osc2_freq / AUDIO_SAMPLE_RATE_EXACT;

    //pulse Width Modulation:
    if (pwm2) {
      osc2_pulseWidth = pulseWidth2 + ((float)pwm2->data[i] * 0.00003051757) * osc2_pwmAmount;
    }
    if (osc2_pulseWidth < 0.001) {
      osc2_pulseWidth = 0.001;
    } else if (osc2_pulseWidth > 0.999) {
      osc2_pulseWidth = 0.999;
    }

    //oscillator 3:
    if (osc3_portamentoSamples > 0 && osc3_currentPortamentoSample++ < osc3_portamentoSamples) {
      frequency3 += osc3_portamentoIncrement;
    }

    //frequency Modulation:
    if (fm3 && osc3_pitchModAmount > 0.0f) {

      int32_t n = fm3->data[i] * osc3_pitchModAmount;
      int32_t ipart = n >> 27;
      n = n & 0x7FFFFFF;
      n = (n + 134217728) << 3;
      n = multiply_32x32_rshift32_rounded(n, n);
      n = multiply_32x32_rshift32_rounded(n, 715827883) << 3;
      n = n + 715827882;

      uint32_t scale = n >> (15 - ipart);
      osc3_freq = frequency3 * scale * 0.00003051757;
    } else {
      osc3_freq = frequency3;
    }

    osc3_dt = osc3_freq / AUDIO_SAMPLE_RATE_EXACT;

    //pulse Width Modulation:
    if (pwm3) {
      osc3_pulseWidth = pulseWidth3 + ((float)pwm3->data[i] * 0.00003051757) * osc3_pwmAmount;
    }
    if (osc3_pulseWidth < 0.001) {
      osc3_pulseWidth = 0.001;
    } else if (osc3_pulseWidth > 0.999) {
      osc3_pulseWidth = 0.999;
    }

    osc1Step(); // This steps actually all oscillators.

    out1->data[i] = (int16_t)(osc1_output *  32768.0 * osc1_gain);
    out2->data[i] = (int16_t)(osc2_output *  32768.0 * osc2_gain);
    out3->data[i] = (int16_t)(osc3_output *  32768.0 * osc3_gain);
  }

  transmit(out1, 0);
  release(out1);

  transmit(out2, 1);
  release(out2);

  transmit(out3, 2);
  release(out3);

  if (fm1) {
    release(fm1);
  }
  if (pwm1) {
    release(pwm1);
  }
  if (fm2) {
    release(fm2);
  }
  if (pwm2) {
    release(pwm2);
  }
  if (fm3) {
    release(fm3);
  }
  if (pwm3) {
    release(pwm3);
  }
}


inline float blamp0(float x) {
  return 1 / 3.0 * x * x * x;
}

inline float blamp1(float x) {
  x = x - 1;
  return -1 / 3.0 * x * x * x;
}

inline float blep0(float x) {
  return x * x;
}

inline float blep1(float x) {
  x = 1 - x ;
  return -x * x;
}

inline void AudioBandlimitedOsci::osc3Step() {

  osc3_output = osc3_blepDelay;
  osc3_blepDelay = 0;

  osc3_t += osc3_dt;

  //triangle and sawtooth wave
  switch (osc3_waveform) {
    case 0: {
        /*
          The sine function is a bit processor intensive, I just added it for the sake of completeness.
          If you depend on fast sine oscillators, use the ones in the Teensy Audio Library, sinewaves are naturally bandlimited
        */
        osc3_t -= floorf(osc3_t);
        osc3_output = arm_sin_f32(osc3_t * TWO_PI);
      }
      break;

    case 1: {
        while (true) {

          if (!osc3_pulseStage) {
            if (osc3_t < osc3_pulseWidth) break;

            float x = (osc3_t - osc3_pulseWidth) / (osc3_widthDelay - osc3_pulseWidth + osc3_dt);
            float scale = osc3_dt / (osc3_pulseWidth - osc3_pulseWidth * osc3_pulseWidth);

            osc3_output   -=  scale * blamp0(x);
            osc3_blepDelay  -=  scale * blamp1(x);

            osc3_pulseStage = true;
          }
          if (osc3_pulseStage) {
            if (osc3_t < 1) break;

            float x = (osc3_t - 1) / osc3_dt;

            float scale = osc3_dt / (osc3_pulseWidth - osc3_pulseWidth * osc3_pulseWidth);

            osc3_output   +=   scale * blamp0(x);
            osc3_blepDelay  +=   scale * blamp1(x);

            osc3_pulseStage = false;
            osc3_t -= 1;
          }
        }

        float naiveWave;

        if (osc3_t <= osc3_pulseWidth) {
          naiveWave = 2 * osc3_t / osc3_pulseWidth - 1;
        } else {
          naiveWave =  - 2 * (osc3_t - osc3_pulseWidth) / (1 - osc3_pulseWidth) + 1;
        }

        osc3_blepDelay += naiveWave;

        osc3_widthDelay = osc3_pulseWidth;
      }
      break;

    case 2:
      {
        //Pulse code
        while (true)
        {
          if (!osc3_pulseStage)
          {
            if (osc3_t < osc3_pulseWidth) break;

            float x = (osc3_t - osc3_pulseWidth) / (osc3_widthDelay - osc3_pulseWidth + osc3_dt);

            osc3_output       -= blep0(x);
            osc3_blepDelay    -= blep1(x);

            osc3_pulseStage = true;
          }
          if (osc3_pulseStage)
          {
            if (osc3_t < 1) break;

            float x = (osc3_t - 1) / osc3_dt;

            osc3_output    += blep0(x);
            osc3_blepDelay += blep1(x);

            osc3_pulseStage = false;
            osc3_t -= 1;
          }
        }

        float naiveWave;

        if (osc3_pulseStage) {
          naiveWave = -1.0;
        } else {
          naiveWave = 1.0;
        }

        osc3_blepDelay += naiveWave;

        osc3_widthDelay = osc3_pulseWidth;
      }
      break;

    case 3: {
        //sync saw code
        osc3_t -= floorf(osc3_t);

        if (osc3_t < osc3_dt) {

          float x = osc3_t / osc3_dt;
          osc3_output -= 0.5 * blep0(x);
          osc3_blepDelay -= 0.5 * blep1(x);
        }

        osc3_blepDelay += osc3_t;

        osc3_output = osc3_output * 2 - 1;
      }
      break;

    default : {
        osc3_t -= floorf(osc3_t);
        osc3_output = 0;
      }
  }
}

inline void AudioBandlimitedOsci::osc3Sync(float x) {

  osc3_output = osc3_blepDelay;
  osc3_blepDelay = 0;

  float scale = osc3_dt / osc1_dt;
  scale -= floorf(scale);
  if (scale <= 0) {
    scale = 1;
  }

  osc3_output   -= 0.5 * scale * blep0(x);
  osc3_blepDelay  -= 0.5 * scale * blep1(x);

  //increase slave phase by partial sample
  float dt = (1 - x) * osc3_dt;
  osc3_t += dt;
  osc3_t -= floorf(osc3_t);

  if (osc3_t < dt) {
    osc3_t += x * osc3_dt;
    osc3_t -= floorf(osc3_t);

    //process transition for the slave
    float x2 = osc3_t / osc3_dt;
    osc3_output   -= 0.5 * blep0(x2);
    osc3_blepDelay  -= 0.5 * blep1(x2);
  }

  //reset slave phase:
  osc3_t = x * osc3_dt;

  osc3_blepDelay += osc3_t;

  osc3_output = osc3_output * 2 - 1;

}

inline void AudioBandlimitedOsci::osc2Step() {

  osc2_output = osc2_blepDelay;
  osc2_blepDelay = 0;

  osc2_t += osc2_dt;

  //triangle and sawtooth wave
  switch (osc2_waveform) {
    case 0: {
        osc2_t -= floorf(osc2_t);
        osc2_output = arm_sin_f32(osc2_t * TWO_PI);
      }
      break;

    case 1: {
        while (true) {

          if (!osc2_pulseStage) {
            if (osc2_t < osc2_pulseWidth) break;

            float x = (osc2_t - osc2_pulseWidth) / (osc2_widthDelay - osc2_pulseWidth + osc2_dt);
            float scale = osc2_dt / (osc2_pulseWidth - osc2_pulseWidth * osc2_pulseWidth);

            osc2_output   -=  scale * blamp0(x);
            osc2_blepDelay  -=  scale * blamp1(x);

            osc2_pulseStage = true;
          }
          if (osc2_pulseStage) {
            if (osc2_t < 1) break;

            float x = (osc2_t - 1) / osc2_dt;

            float scale = osc2_dt / (osc2_pulseWidth - osc2_pulseWidth * osc2_pulseWidth);

            osc2_output   +=   scale * blamp0(x);
            osc2_blepDelay  +=   scale * blamp1(x);

            osc2_pulseStage = false;
            osc2_t -= 1;
          }
        }

        float naiveWave;

        if (osc2_t <= osc2_pulseWidth) {
          naiveWave = 2 * osc2_t / osc2_pulseWidth - 1;
        } else {
          naiveWave =  - 2 * (osc2_t - osc2_pulseWidth) / (1 - osc2_pulseWidth) + 1;
        }

        osc2_blepDelay += naiveWave;

        osc2_widthDelay = osc2_pulseWidth;
      }
      break;

    case 2:
      {
        //Pulse code
        while (true)
        {
          if (!osc2_pulseStage)
          {
            if (osc2_t < osc2_pulseWidth) break;

            float x = (osc2_t - osc2_pulseWidth) / (osc2_widthDelay - osc2_pulseWidth + osc2_dt);

            osc2_output       -= blep0(x);
            osc2_blepDelay    -= blep1(x);

            osc2_pulseStage = true;
          }
          if (osc2_pulseStage)
          {
            if (osc2_t < 1) break;

            float x = (osc2_t - 1) / osc2_dt;

            osc2_output    += blep0(x);
            osc2_blepDelay += blep1(x);

            osc2_pulseStage = false;
            osc2_t -= 1;
          }
        }

        float naiveWave;

        if (osc2_pulseStage) {
          naiveWave = -1.0;
        } else {
          naiveWave = 1.0;
        }

        osc2_blepDelay += naiveWave;

        osc2_widthDelay = osc2_pulseWidth;
      }
      break;

    case 3: {
        //sync saw code
        osc2_t -= floorf(osc2_t);

        if (osc2_t < osc2_dt) {

          float x = osc2_t / osc2_dt;
          osc2_output -= 0.5 * blep0(x);
          osc2_blepDelay -= 0.5 * blep1(x);
        }

        osc2_blepDelay += osc2_t;

        osc2_output = osc2_output * 2 - 1;
      }
      break;

    default : {
        osc2_t -= floorf(osc2_t);
        osc2_output = 0;
      }
  }
}

inline void AudioBandlimitedOsci::osc2Sync(float x) {


  osc2_output = osc2_blepDelay;
  osc2_blepDelay = 0;

  float scale = osc2_dt / osc1_dt;
  scale -= floorf(scale);
  if (scale <= 0) {
    scale = 1;
  }

  osc2_output   -= scale * 0.5 * blep0(x);
  osc2_blepDelay  -= scale * 0.5 * blep1(x);

  //increase slave phase by partial sample
  float dt = (1 - x) * osc2_dt;
  osc2_t += dt;
  osc2_t -= floorf(osc2_t);

  if (osc2_t < dt && scale < 1) {
    osc2_t += x * osc2_dt;
    osc2_t -= floorf(osc2_t);

    //process transition for the slave
    float x2 = osc2_t / osc2_dt;
    osc2_output   -= 0.5 * blep0(x2);
    osc2_blepDelay  -= 0.5 * blep1(x2);
  }

  //reset slave phase:
  osc2_t = x * osc2_dt;

  osc2_blepDelay += osc2_t;


  osc2_output = osc2_output * 2 - 1;


}


inline void AudioBandlimitedOsci::osc1Step() {

  osc1_output = osc1_blepDelay;
  osc1_blepDelay = 0;

  osc1_t += osc1_dt;


  if (osc2_waveform != 2) {
    osc2Step();
  } else if (osc1_t < 1) {
    osc2Step();
  }

  if (osc3_waveform != 2) {
    osc3Step();
  } else if (osc1_t < 1) {
    osc3Step();
  }

  //triangle and sawtooth wave
  switch (osc1_waveform) {
    case 0: {
        osc1_t -= floorf(osc1_t);
        osc1_output = arm_sin_f32(osc1_t * TWO_PI);
      }
      break;

    case 1 :
      {
        while (true) {

          if (!osc1_pulseStage) {
            if (osc1_t < osc1_pulseWidth) break;

            float x = (osc1_t - osc1_pulseWidth) / (osc1_widthDelay - osc1_pulseWidth + osc1_dt);
            float scale = osc1_dt / (osc1_pulseWidth - osc1_pulseWidth * osc1_pulseWidth);

            osc1_output   -=  scale * blamp0(x);
            osc1_blepDelay  -=  scale * blamp1(x);

            osc1_pulseStage = true;
          }
          if (osc1_pulseStage) {
            if (osc1_t < 1) break;

            float x = (osc1_t - 1) / osc1_dt;

            if (osc2_waveform == 3) {
              osc2Sync(x);
            }
            if (osc3_waveform == 3) {
              osc3Sync(x);
            }

            float scale = osc1_dt / (osc1_pulseWidth - osc1_pulseWidth * osc1_pulseWidth);

            osc1_output   +=   scale * blamp0(x);
            osc1_blepDelay  +=   scale * blamp1(x);

            osc1_pulseStage = false;
            osc1_t -= 1;
          }
        }

        float naiveWave;

        if (osc1_t <= osc1_pulseWidth) {
          naiveWave = 2 * osc1_t / osc1_pulseWidth - 1;
        } else {
          naiveWave =  - 2 * (osc1_t - osc1_pulseWidth) / (1 - osc1_pulseWidth) + 1;
        }

        osc1_blepDelay += naiveWave;

        osc1_widthDelay = osc1_pulseWidth;
      }
      break;

    case 2:
      {
        //Pulse code
        while (true)
        {
          if (!osc1_pulseStage)
          {
            if (osc1_t < osc1_pulseWidth) break;

            float x = (osc1_t - osc1_pulseWidth) / (osc1_widthDelay - osc1_pulseWidth + osc1_dt);

            osc1_output       -= blep0(x);
            osc1_blepDelay    -= blep1(x);

            osc1_pulseStage = true;
          }
          if (osc1_pulseStage)
          {
            if (osc1_t < 1) break;

            float x = (osc1_t - 1) / osc1_dt;

            if (osc2_waveform == 3) {
              osc2Sync(x);
            }
            if (osc3_waveform == 3) {
              osc3Sync(x);
            }

            osc1_output    += blep0(x);
            osc1_blepDelay += blep1(x);

            osc1_pulseStage = false;
            osc1_t -= 1;
          }
        }
        float naiveWave;

        if (osc1_pulseStage) {
          naiveWave = -1.0;
        } else {
          naiveWave = 1.0;
        }

        osc1_blepDelay += naiveWave;

        osc1_widthDelay = osc1_pulseWidth;
        break;
      }

    default : {
        osc1_t -= floorf(osc1_t);
        osc1_output = 0;
      }
  }

}
