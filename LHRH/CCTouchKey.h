
// only compatible with teensy LC (touchRead)

#ifndef CCTOUCHKEY_H
#define CCTOUCHKEY_H

uint32_t mTouchKeyGlobalMax = 0xffffffff;
uint32_t mTouchKeyGlobalMin = 0;

class CCTouchKey 
{
  uint8_t mPin;
  bool mUseLocal;
  uint32_t mLocalMax;
  uint32_t mLocalMin;
public:
  CCTouchKey(uint8_t pin) :
    mPin(pin),
    mUseLocal(false),
    mLocalMax(0xffffffff),
    mLocalMin(0)
  {
  }

  
};

#endif
