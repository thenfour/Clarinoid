
#pragma once

#include <clarinoid/basic/Basic.hpp>
#include <clarinoid/application/MusicalState.hpp>


struct CCPatch : public AudioConnection
{
  CCPatch(AudioStream &source, uint8_t sourceOutput, AudioStream &destination, uint8_t destinationInput) :
    AudioConnection(source, sourceOutput, destination, destinationInput)
  {
  }
};

