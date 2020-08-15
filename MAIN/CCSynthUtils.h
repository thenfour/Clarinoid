
// don't use a LUT because we want to support pitch bend and glides and stuff. using a LUT + interpolation would be asinine.
#ifndef CCSYNTHUTILS_H
#define CCSYNTHUTILS_H

#include "Shared_CCUtil.h"

inline float MIDINoteToFreq(float x) {
    float a = 440;
    return (a / 32.0f) * powf(2.0f, (((float)x - 9.0f) / 12.0f));
}

#endif
