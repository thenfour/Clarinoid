
#ifndef CCSYNTHUTILS_H
#define CCSYNTHUTILS_H

#include "CCUtil.h"

class MIDINotes
{
public:
  float frequencies[127];
  // TODO: names

  MIDINotes() {
    float a = 440; // a is 440 hz...
    for (int x = 0; x < 127; ++x)
    {
      frequencies[x] = (a / 32.0f) * powf(2.0f, (((float)x - 9.0f) / 12.0f));
    }
  } 
  
};

MIDINotes gMIDINotes;

inline float MIDINoteToFreq(float x) {
  //return gMIDINotes.frequencies[note];
    float a = 440; // a is 440 hz...
    return (a / 32.0f) * powf(2.0f, (((float)x - 9.0f) / 12.0f));
}

#endif
