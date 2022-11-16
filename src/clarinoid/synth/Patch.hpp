// CCPatch is provided to allow default construction.

#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

struct NullAudioStream : AudioStream
{
   NullAudioStream() : AudioStream(0, nullptr)
   {
   }
   virtual void update(void) override
   {
   }
};

static constexpr auto aexppp = sizeof(AudioStream);

static NullAudioStream gNullAudioStreamA;
static NullAudioStream gNullAudioStreamB;

struct CCPatch : public AudioConnection
{
   CCPatch(AudioStream &source, uint8_t sourceOutput, AudioStream &destination, uint8_t destinationInput)
       : AudioConnection(source, sourceOutput, destination, destinationInput)
   {
   }
   CCPatch() : AudioConnection(gNullAudioStreamA, gNullAudioStreamB)
   {
       //
   }
};

} // namespace clarinoid
