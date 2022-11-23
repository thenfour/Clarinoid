#pragma once

class AudioSynthWaveformModulated2 : public AudioStream
{
  public:
    AudioSynthWaveformModulated2(void)
        : AudioStream(2, inputQueueArray), phase_accumulator(0), phase_increment(0), modulation_factor(32768),
          magnitude(0), arbdata(NULL), sample(0), tone_offset(0), tone_type(WAVEFORM_SINE), modulation_type(1)
    {
    }

    void frequency(float freq)
    {
        if (freq < 0.0f)
        {
            freq = 0.0;
        }
        else if (freq > AUDIO_SAMPLE_RATE_EXACT / 2.0f)
        {
            freq = AUDIO_SAMPLE_RATE_EXACT / 2.0f;
        }
        phase_increment = freq * (4294967296.0f / AUDIO_SAMPLE_RATE_EXACT);
        if (phase_increment > 0x7FFE0000u)
            phase_increment = 0x7FFE0000;
    }
    void amplitude(float n)
    { // 0 to 1.0
        if (n < 0)
        {
            n = 0;
        }
        else if (n > 1.0f)
        {
            n = 1.0f;
        }
        magnitude = n * 65536.0f;
    }
    void offset(float n)
    {
        if (n < -1.0f)
        {
            n = -1.0f;
        }
        else if (n > 1.0f)
        {
            n = 1.0f;
        }
        tone_offset = n * 32767.0f;
    }
    void begin(short t_type)
    {
        tone_type = t_type;
        if (t_type == WAVEFORM_BANDLIMIT_SQUARE)
            band_limit_waveform.init_square(phase_increment);
        else if (t_type == WAVEFORM_BANDLIMIT_PULSE)
            band_limit_waveform.init_pulse(phase_increment, 0x80000000u);
        else if (t_type == WAVEFORM_BANDLIMIT_SAWTOOTH || t_type == WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE)
            band_limit_waveform.init_sawtooth(phase_increment);
    }
    void begin(float t_amp, float t_freq, short t_type)
    {
        amplitude(t_amp);
        frequency(t_freq);
        begin(t_type);
    }
    void arbitraryWaveform(const int16_t *data, float maxFreq)
    {
        arbdata = data;
    }
    // void frequencyModulation(float octaves) {
    // 	if (octaves > 12.0f) {
    // 		octaves = 12.0f;
    // 	} else if (octaves < 0.1f) {
    // 		octaves = 0.1f;
    // 	}
    // 	modulation_factor = octaves * 4096.0f;
    // 	modulation_type = 0;
    // }
    void phaseModulation(float degrees)
    {
        if (degrees > 9000.0f)
        {
            degrees = 9000.0f;
        }
        else if (degrees < 30.0f)
        {
            degrees = 30.0f;
        }
        modulation_factor = degrees * (float)(65536.0 / 180.0);
        modulation_type = 1;
    }

    void phaseModulationFeedback(float degrees)
    {
        mPhaseModFeedbackAmt = degrees * (float)(65536.0 / 180.0);
    }

    void ResetPhase()
    {
        // todo: band-limit
        phase_accumulator = 0;
    }

    virtual void update(void);

  private:
    audio_block_t *inputQueueArray[2];
    uint32_t phase_accumulator;
    uint32_t phase_increment;
    uint32_t modulation_factor;
    int16_t mFBN1; // previous sample
    uint32_t mPhaseModFeedbackAmt;
    int32_t magnitude;
    const int16_t *arbdata;
    uint32_t phasedata[AUDIO_BLOCK_SAMPLES];
    int16_t sample; // for WAVEFORM_SAMPLE_HOLD
    int16_t tone_offset;
    uint8_t tone_type;
    uint8_t modulation_type;
    BandLimitedWaveform band_limit_waveform;
};

void AudioSynthWaveformModulated2::update(void)
{
    audio_block_t *block, *moddata, *shapedata;
    int16_t *bp, *end;
    int32_t val1, val2;
    int16_t magnitude15;
    uint32_t i, ph, index, index2, scale, priorphase;
    const uint32_t inc = phase_increment;

    moddata = receiveReadOnly(0);
    shapedata = receiveReadOnly(1);

    // Pre-compute the phase angle for every output sample of this update
    ph = phase_accumulator;
    priorphase = phasedata[AUDIO_BLOCK_SAMPLES - 1];
    if (moddata && modulation_type == 0)
    {
        // // Frequency Modulation
        // bp = moddata->data;
        // for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
        // 	int32_t n = (*bp++) * modulation_factor; // n is # of octaves to mod
        // 	int32_t ipart = n >> 27; // 4 integer bits
        // 	n &= 0x7FFFFFF;          // 27 fractional bits
        // 	#ifdef IMPROVE_EXPONENTIAL_ACCURACY
        // 	// exp2 polynomial suggested by Stefan Stenzel on "music-dsp"
        // 	// mail list, Wed, 3 Sep 2014 10:08:55 +0200
        // 	int32_t x = n << 3;
        // 	n = multiply_accumulate_32x32_rshift32_rounded(536870912, x, 1494202713);
        // 	int32_t sq = multiply_32x32_rshift32_rounded(x, x);
        // 	n = multiply_accumulate_32x32_rshift32_rounded(n, sq, 1934101615);
        // 	n = n + (multiply_32x32_rshift32_rounded(sq,
        // 		multiply_32x32_rshift32_rounded(x, 1358044250)) << 1);
        // 	n = n << 1;
        // 	#else
        // 	// exp2 algorithm by Laurent de Soras
        // 	// https://www.musicdsp.org/en/latest/Other/106-fast-exp2-approximation.html
        // 	n = (n + 134217728) << 3;

        // 	n = multiply_32x32_rshift32_rounded(n, n);
        // 	n = multiply_32x32_rshift32_rounded(n, 715827883) << 3;
        // 	n = n + 715827882;
        // 	#endif
        // 	uint32_t scale = n >> (14 - ipart);
        // 	uint64_t phstep = (uint64_t)inc * scale;
        // 	uint32_t phstep_msw = phstep >> 32;
        // 	if (phstep_msw < 0x7FFE) {
        // 		ph += phstep >> 16;
        // 	} else {
        // 		ph += 0x7FFE0000;
        // 	}
        // 	phasedata[i] = ph;
        // }
        // release(moddata);
    }
    else if (moddata)
    {
        // Phase Modulation
        bp = moddata->data;
        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            // more than +/- 180 deg shift by 32 bit overflow of "n"
            uint32_t n = ((uint32_t)(*bp++)) * modulation_factor;
            phasedata[i] = ph + n;
            ph += inc;
        }
        release(moddata);
    }
    else
    {
        // No Modulation Input
        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            phasedata[i] = ph;
            ph += inc;
        }
    }
    phase_accumulator = ph;

    // If the amplitude is zero, no output, but phase still increments properly
    if (magnitude == 0)
    {
        if (shapedata)
            release(shapedata);
        return;
    }
    block = allocate();
    if (!block)
    {
        if (shapedata)
            release(shapedata);
        return;
    }
    bp = block->data;

    // Now generate the output samples using the pre-computed phase angles
    switch (tone_type)
    {
    case WAVEFORM_SINE:
        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            ph = phasedata[i] + mFBN1 * mPhaseModFeedbackAmt; // overflow is expected; full range is 360deg so integer
                                                              // over/underflow represents phase wrap
            index = ph >> 24;
            val1 = AudioWaveformSine[index];
            val2 = AudioWaveformSine[index + 1];
            scale = (ph >> 8) & 0xFFFF;
            val2 *= scale;
            val1 *= 0x10000 - scale;
            *bp = multiply_32x32_rshift32(val1 + val2, magnitude);
            mFBN1 = *bp;
            ++bp;
        }
        break;

    case WAVEFORM_ARBITRARY:
        if (!arbdata)
        {
            release(block);
            if (shapedata)
                release(shapedata);
            return;
        }
        // len = 256
        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            ph = phasedata[i] + mFBN1 * mPhaseModFeedbackAmt; // overflow is expected; full range is 360deg so integer
                                                              // over/underflow represents phase wrap
            index = ph >> 24;
            index2 = index + 1;
            if (index2 >= 256)
                index2 = 0;
            val1 = *(arbdata + index);
            val2 = *(arbdata + index2);
            scale = (ph >> 8) & 0xFFFF;
            val2 *= scale;
            val1 *= 0x10000 - scale;
            *bp = multiply_32x32_rshift32(val1 + val2, magnitude);
            mFBN1 = *bp;
            ++bp;
        }
        break;

    case WAVEFORM_PULSE:
        if (shapedata)
        {
            magnitude15 = signed_saturate_rshift(magnitude, 16, 1);
            for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
            {
                uint32_t width = ((shapedata->data[i] + 0x8000) & 0xFFFF) << 16;
                ph = phasedata[i] + mFBN1 * mPhaseModFeedbackAmt; // overflow is expected; full range is 360deg so
                                                                  // integer over/underflow represents phase wrap
                if (ph < width)
                {
                    *bp = magnitude15;
                }
                else
                {
                    *bp = -magnitude15;
                }
                mFBN1 = *bp;
                ++bp;
            }
            break;
        } // else fall through to orginary square without shape modulation

    case WAVEFORM_SQUARE:
        magnitude15 = signed_saturate_rshift(magnitude, 16, 1);
        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            ph = phasedata[i] + mFBN1 * mPhaseModFeedbackAmt; // overflow is expected; full range is 360deg so
                                                              // integer over/underflow represents phase wrap
            if (ph & 0x80000000)
            {
                *bp = -magnitude15;
            }
            else
            {
                *bp = magnitude15;
            }
            mFBN1 = *bp;
            ++bp;
        }
        break;

    case WAVEFORM_BANDLIMIT_PULSE:
        if (shapedata)
        {
            for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
            {
                uint32_t width = ((shapedata->data[i] + 0x8000) & 0xFFFF) << 16;
                ph = phasedata[i] + mFBN1 * mPhaseModFeedbackAmt; // overflow is expected; full range is 360deg so
                                                                  // integer over/underflow represents phase wrap
                int32_t val = band_limit_waveform.generate_pulse(ph, width, i);
                *bp = (int16_t)((val * magnitude) >> 16);
                mFBN1 = *bp;
                ++bp;
            }
            break;
        } // else fall through to orginary square without shape modulation

    case WAVEFORM_BANDLIMIT_SQUARE:
        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            ph = phasedata[i] + mFBN1 * mPhaseModFeedbackAmt; // overflow is expected; full range is 360deg so
                                                              // integer over/underflow represents phase wrap
            int32_t val = band_limit_waveform.generate_square(ph, i);
            *bp = (int16_t)((val * magnitude) >> 16);
            mFBN1 = *bp;
            ++bp;
        }
        break;

    case WAVEFORM_SAWTOOTH:
        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            ph = phasedata[i] + mFBN1 * mPhaseModFeedbackAmt; // overflow is expected; full range is 360deg so
                                                              // integer over/underflow represents phase wrap
            *bp = signed_multiply_32x16t(magnitude, ph);
            mFBN1 = *bp;
            ++bp;
        }
        break;

    case WAVEFORM_SAWTOOTH_REVERSE:
        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            ph = phasedata[i] + mFBN1 * mPhaseModFeedbackAmt; // overflow is expected; full range is 360deg so
                                                              // integer over/underflow represents phase wrap
            *bp = signed_multiply_32x16t(0xFFFFFFFFu - magnitude, ph);
            mFBN1 = *bp;
            ++bp;
        }
        break;

    case WAVEFORM_BANDLIMIT_SAWTOOTH:
    case WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE:
        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            ph = phasedata[i] + mFBN1 * mPhaseModFeedbackAmt; // overflow is expected; full range is 360deg so
                                                              // integer over/underflow represents phase wrap
            int16_t val = band_limit_waveform.generate_sawtooth(ph, i);
            val = (int16_t)((val * magnitude) >> 16);
            *bp = tone_type == WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE ? (int16_t)-val : (int16_t) + val;
            mFBN1 = *bp;
            ++bp;
        }
        break;

    case WAVEFORM_TRIANGLE_VARIABLE:
        if (shapedata)
        {
            for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
            {
                uint32_t width = (shapedata->data[i] + 0x8000) & 0xFFFF;
                uint32_t rise = 0xFFFFFFFF / width;
                uint32_t fall = 0xFFFFFFFF / (0xFFFF - width);
                uint32_t halfwidth = width << 15;
                uint32_t n;
                ph = phasedata[i] + mFBN1 * mPhaseModFeedbackAmt; // overflow is expected; full range is 360deg so
                                                                  // integer over/underflow represents phase wrap
                if (ph < halfwidth)
                {
                    n = (ph >> 16) * rise;
                    *bp = ((n >> 16) * magnitude) >> 16;
                }
                else if (ph < 0xFFFFFFFF - halfwidth)
                {
                    n = 0x7FFFFFFF - (((ph - halfwidth) >> 16) * fall);
                    *bp = (((int32_t)n >> 16) * magnitude) >> 16;
                }
                else
                {
                    n = ((ph + halfwidth) >> 16) * rise + 0x80000000;
                    *bp = (((int32_t)n >> 16) * magnitude) >> 16;
                }
                // ph += inc;
                mFBN1 = *bp;
                ++bp;
            }
            break;
        } // else fall through to orginary triangle without shape modulation

    case WAVEFORM_TRIANGLE:
        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            ph = phasedata[i] + mFBN1 * mPhaseModFeedbackAmt; // overflow is expected; full range is 360deg so
                                                              // integer over/underflow represents phase wrap
            uint32_t phtop = ph >> 30;
            if (phtop == 1 || phtop == 2)
            {
                *bp = ((0xFFFF - (ph >> 15)) * magnitude) >> 16;
            }
            else
            {
                *bp = (((int32_t)ph >> 15) * magnitude) >> 16;
            }
            mFBN1 = *bp;
            ++bp;
        }
        break;
    case WAVEFORM_SAMPLE_HOLD:
        for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
        {
            ph = phasedata[i] + mFBN1 * mPhaseModFeedbackAmt; // overflow is expected; full range is 360deg so
                                                              // integer over/underflow represents phase wrap
            if (ph < priorphase)
            { // does not work for phase modulation
                sample = random(magnitude) - (magnitude >> 1);
            }
            priorphase = ph;
            *bp = sample;
            mFBN1 = *bp;
            ++bp;
        }
        break;
    }

    if (tone_offset)
    {
        bp = block->data;
        end = bp + AUDIO_BLOCK_SAMPLES;
        do
        {
            val1 = *bp;
            *bp++ = signed_saturate_rshift(val1 + tone_offset, 16, 0);
        } while (bp < end);
    }
    if (shapedata)
        release(shapedata);
    transmit(block, 0);
    release(block);
}
