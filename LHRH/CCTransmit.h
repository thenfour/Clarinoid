
#ifndef CCTRANSMIT_H
#define CCTRANSMIT_H

#include "CCUtil.h"
#include <FastCRC.h>

#define CCEWI_BAUD 115200

struct ChecksummablePayload
{
  bool key1;
  bool key2;
  bool key3;
  bool key4;
  bool key5;
  bool key6;
  bool octave1;
  bool octave2;
  bool octave3;
  bool octave4;
  bool button1;
  bool button2;
  float pressure1;
  float pressure2;
};

// payload is the same between both LH/RH modules.
struct LHRHPayload
{
  uint16_t payloadSize; // for verification
  ChecksummablePayload data;
  
  // some system stats
  uint32_t framerate;
  uint32_t capGlobalMin;
  uint32_t capGlobalMax;
  uint32_t pressure1Min;
  uint32_t pressure1Max;
  uint32_t pressure2Min;
  uint32_t pressure2Max;
  
  char version[sizeof(EWI_LHRH_VERSION)];
  uint32_t serial;
  uint32_t dataChecksum;
};

uint32_t gTXSerial = 0;

class CCTransmit : IUpdateObject
{
  FastCRC32 CRC32;
  framerateCalculator mFramerate;
  HardwareSerial& mSerial;
  
public:
  explicit CCTransmit(HardwareSerial& serial) :
    mSerial(serial)
  {
  }

  virtual void setup() {
    mSerial.begin(CCEWI_BAUD);
  }

  virtual void loop() {
    mFramerate.onFrame();
  }

  void Send(LHRHPayload& payload) {
    payload.dataChecksum = CRC32.crc32((const uint8_t*)&payload.data, sizeof(payload.data));
    payload.framerate = mFramerate.getFPS();
    payload.serial = gTXSerial;
    mSerial.write((uint8_t*)&payload, sizeof(payload));
    gTXSerial ++;
  }
};

#endif
