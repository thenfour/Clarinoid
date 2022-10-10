// fork of AudioEffectEnvelope; the whole "Forced" state was causing glitching and unneeded behavior-babysitting for no
// benefit. so it's gone.
#pragma once

namespace clarinoid
{
enum class EnvelopeStage : uint8_t
{
    Idle,
    Delay,
    Attack,
    Hold,
    Decay,
    Sustain,
    Release,
};

EnumItemInfo<EnvelopeStage> gEnvelopeStageItems[7] = {
    {EnvelopeStage::Idle, "Idle"},
    {EnvelopeStage::Delay, "Delay"},
    {EnvelopeStage::Attack, "Attack"},
    {EnvelopeStage::Hold, "Hold"},
    {EnvelopeStage::Decay, "Decay"},
    {EnvelopeStage::Sustain, "Sustain"},
    {EnvelopeStage::Release, "Release"},
};

EnumInfo<EnvelopeStage> gEnvelopeStageInfo("EnvelopeStage", gEnvelopeStageItems);

class EnvelopeNode : public AudioStream
{
    static constexpr float SAMPLES_PER_MSEC = (AUDIO_SAMPLE_RATE_EXACT / 1000.0f);

  public:
    EnvelopeNode() : AudioStream(1, inputQueueArray)
    {
        state = EnvelopeStage::Idle;
        delay(0.0f); // default values...
        attack(10.5f);
        hold(2.5f);
        decay(35.0f);
        sustain(0.5f);
        release(300.0f);
    }
    void noteOn()
    {
        mult_hires = 0;
        count = delay_count;
        if (count > 0)
        {
            state = EnvelopeStage::Delay;
            inc_hires = 0;
        }
        else
        {
            state = EnvelopeStage::Attack;
            count = attack_count;
            inc_hires = 0x40000000 / (int32_t)count;
        }
    }
    void noteOff()
    {
        if (state != EnvelopeStage::Idle) // && state != STATE_FORCED)
        {
            state = EnvelopeStage::Release;
            count = release_count;
            inc_hires = (-mult_hires) / (int32_t)count;
        }
    }
    void delay(float milliseconds)
    {
        delay_count = milliseconds2count(milliseconds);
    }
    void attack(float milliseconds)
    {
        attack_count = milliseconds2count(milliseconds);
        if (attack_count == 0)
            attack_count = 1;
    }
    void hold(float milliseconds)
    {
        hold_count = milliseconds2count(milliseconds);
    }
    void decay(float milliseconds)
    {
        decay_count = milliseconds2count(milliseconds);
        if (decay_count == 0)
            decay_count = 1;
    }
    void sustain(float level)
    {
        if (level < 0.0f)
            level = 0;
        else if (level > 1.0f)
            level = 1.0f;
        sustain_mult = level * 1073741824.0f;
    }
    void release(float milliseconds)
    {
        release_count = milliseconds2count(milliseconds);
        if (release_count == 0)
            release_count = 1;
    }
    EnvelopeStage GetStage() const
    { // used by debug displays
        return this->state;
    }

    bool isPlaying() const
    {
        return this->state != EnvelopeStage::Idle;
    }
    using AudioStream::release;
    virtual void update(void)
    {
        audio_block_t *block;
        uint32_t *p, *end;
        uint32_t sample12, sample34, sample56, sample78, tmp1, tmp2;

        block = receiveWritable();
        if (!block)
            return;
        if (state == EnvelopeStage::Idle)
        {
            release(block);
            return;
        }
        p = (uint32_t *)(block->data);
        end = p + AUDIO_BLOCK_SAMPLES / 2;

        while (p < end)
        {
            // we only care about the state when completing a region
            if (count == 0)
            {
                if (state == EnvelopeStage::Attack)
                {
                    count = hold_count;
                    if (count > 0)
                    {
                        state = EnvelopeStage::Hold;
                        mult_hires = 0x40000000;
                        inc_hires = 0;
                    }
                    else
                    {
                        state = EnvelopeStage::Decay;
                        count = decay_count;
                        inc_hires = (sustain_mult - 0x40000000) / (int32_t)count;
                    }
                    continue;
                }
                else if (state == EnvelopeStage::Hold)
                {
                    state = EnvelopeStage::Decay;
                    count = decay_count;
                    inc_hires = (sustain_mult - 0x40000000) / (int32_t)count;
                    continue;
                }
                else if (state == EnvelopeStage::Decay)
                {
                    state = EnvelopeStage::Sustain;
                    count = 0xFFFF;
                    mult_hires = sustain_mult;
                    inc_hires = 0;
                }
                else if (state == EnvelopeStage::Sustain)
                {
                    count = 0xFFFF;
                }
                else if (state == EnvelopeStage::Release)
                {
                    state = EnvelopeStage::Idle;
                    while (p < end)
                    {
                        *p++ = 0;
                        *p++ = 0;
                        *p++ = 0;
                        *p++ = 0;
                    }
                    break;
                }
                else if (state == EnvelopeStage::Delay)
                {
                    state = EnvelopeStage::Attack;
                    count = attack_count;
                    inc_hires = 0x40000000 / count;
                    continue;
                }
            }

            int32_t mult = mult_hires >> 14;
            int32_t inc = inc_hires >> 17;
            // process 8 samples, using only mult and inc (16 bit resolution)
            sample12 = *p++;
            sample34 = *p++;
            sample56 = *p++;
            sample78 = *p++;
            p -= 4;
            mult += inc;
            tmp1 = signed_multiply_32x16b(mult, sample12);
            mult += inc;
            tmp2 = signed_multiply_32x16t(mult, sample12);
            sample12 = pack_16b_16b(tmp2, tmp1);
            mult += inc;
            tmp1 = signed_multiply_32x16b(mult, sample34);
            mult += inc;
            tmp2 = signed_multiply_32x16t(mult, sample34);
            sample34 = pack_16b_16b(tmp2, tmp1);
            mult += inc;
            tmp1 = signed_multiply_32x16b(mult, sample56);
            mult += inc;
            tmp2 = signed_multiply_32x16t(mult, sample56);
            sample56 = pack_16b_16b(tmp2, tmp1);
            mult += inc;
            tmp1 = signed_multiply_32x16b(mult, sample78);
            mult += inc;
            tmp2 = signed_multiply_32x16t(mult, sample78);
            sample78 = pack_16b_16b(tmp2, tmp1);
            *p++ = sample12;
            *p++ = sample34;
            *p++ = sample56;
            *p++ = sample78;
            // adjust the long-term gain using 30 bit resolution (fix #102)
            // https://github.com/PaulStoffregen/Audio/issues/102
            mult_hires += inc_hires;
            count--;
        }
        transmit(block);
        release(block);
    }

  private:
    uint16_t milliseconds2count(float milliseconds)
    {
        if (milliseconds < 0.0f)
            milliseconds = 0.0f;
        uint32_t c = ((uint32_t)(milliseconds * SAMPLES_PER_MSEC) + 7) >> 3;
        if (c > 65535)
            c = 65535; // allow up to 11.88 seconds
        return c;
    }
    audio_block_t *inputQueueArray[1];

    // state
    EnvelopeStage state;
    uint16_t count;     // how much time remains in this state, in 8 sample units
    int32_t mult_hires; // attenuation, 0=off, 0x40000000=unity gain
    int32_t inc_hires;  // amount to change mult_hires every 8 samples

    // settings
    uint16_t delay_count;
    uint16_t attack_count;
    uint16_t hold_count;
    uint16_t decay_count;
    int32_t sustain_mult;
    uint16_t release_count;
    // uint16_t release_forced_count;
};

} // namespace clarinoid
