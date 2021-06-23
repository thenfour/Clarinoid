
#pragma once

#include "AudioBufferUtils.hpp"

namespace clarinoid
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <size_t NInputs>
struct MultiMixerNode : public AudioStream
{
    MultiMixerNode() : AudioStream(NInputs, inputQueueArray)
    {
    }

    virtual void update() override
    {
        audio_block_t *out = nullptr;

        for (size_t ichannel = 0; ichannel < NInputs; ++ichannel)
        {
            if (!out)
            {
                // take the 1st connected channel as writable. no mixing yet to be applied.
                out = receiveWritable(ichannel); // may return null if not connected!
            }
            else
            {
                // subsequent iterations, receive read only & apply to out buffer.
                audio_block_t *in = receiveReadOnly(ichannel);
                if (in)
                {
                    audioBufferMixInPlace(out->data, in->data);
                    release(in);
                }
            }
        }

        if (out)
        {
            transmit(out);
            release(out);
        }
    }

    audio_block_t *inputQueueArray[NInputs];
};


} // namespace clarinoid

