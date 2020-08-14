
#ifndef CCTRANSMIT_H
#define CCTRANSMIT_H

#include "CCUtil.h"
#include <FastCRC.h>

#define CCEWI_BAUD 9600

// important to keep this well under HardwareSerial.cpp:RX_BUFFER_SIZE/2.
// We want there ALWAYS to be a complete packet in the buffer, not split up.
// and consider that the buffer could have like
// 2. [whole packet]
// 3. [99% of a packet]
// so it must be able to hold 2 packets. space is therefore a premium.
#define RX_BUFFER_SIZE 64
struct ChecksummablePayload
{
  bool key1 : 1;
  bool key2 : 1;
  bool key3 : 1;
  bool key4 : 1;
  bool key5 : 1;
  bool key6 : 3; // pad for easier debugging

  bool octave1 : 1;
  bool octave2 : 1;
  bool octave3 : 1;
  bool octave4 : 1;
  bool button1 : 1;
  bool button2 : 3; // pad for easier debugging

  uint16_t pressure1 = 0;
  uint16_t pressure2 = 0;
};

#define LHRHPAYLOAD_MODULE_LH 1
#define LHRHPAYLOAD_MODULE_RH 2

// payload is the same between both LH/RH modules.
struct LHRHPayload
{
  // since i don't want to do some crazy 7-bit encoding or so, i want a marker that's
  // long enough to never misfire because it could be mixed with any other data in this struct.
  uint8_t beginMarker[7] = { 0xf1, 0x07, 0x6c, 0xee, 0x6f, 0x73, 0x43 }; // NOTE: to simplify the search algorithm, make sure these are all different bytes. that way i don't have to backtrack.
  uint8_t payloadSize = sizeof(LHRHPayload); // for verification

  ChecksummablePayload data = {0};
  
  uint16_t framerate = 0;
  uint16_t serial = 0;
  uint32_t dataChecksum = 0;
};

inline String ToString(const LHRHPayload& p)
{
  char format[800];
  sprintf(format, "mk:[%02x %02x %02x %02x %02x %02x %02x] sz:[%d] #:[%d] chk:[%08x] d:[k%c%c%c%c%c%c o%c%c%c%c b%c%c p%04x p%04x]",
    (int)p.beginMarker[0], (int)p.beginMarker[1],
    (int)p.beginMarker[2], (int)p.beginMarker[3],
    (int)p.beginMarker[4], (int)p.beginMarker[5],
    (int)p.beginMarker[6],
    (int)p.payloadSize,
    (int)p.serial,
    (int)p.dataChecksum,
    p.data.key1 ? '1' : '-',
    p.data.key2 ? '2' : '-',
    p.data.key3 ? '3' : '-',
    p.data.key4 ? '4' : '-',
    p.data.key5 ? '5' : '-',
    p.data.key6 ? '6' : '-',

    p.data.octave1 ? '1' : '-',
    p.data.octave2 ? '2' : '-',
    p.data.octave3 ? '3' : '-',
    p.data.octave4 ? '4' : '-',

    p.data.button1 ? '1' : '-',
    p.data.button2 ? '2' : '-',

    p.data.pressure1,
    p.data.pressure2
    );
  return format;
}

uint32_t CalcChecksum(const LHRHPayload& p)
{
  static FastCRC32 CRC32;
  return CRC32.crc32((const uint8_t*)&p.data, sizeof(p.data));
}

static_assert(sizeof(LHRHPayload) < (RX_BUFFER_SIZE / 2), "");

uint16_t gTXSerial = 0;

class CCTransmit : IUpdateObject
{
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
    payload.dataChecksum = CalcChecksum(payload);
    payload.framerate = mFramerate.getFPS();
    payload.serial = gTXSerial;
    mSerial.write((uint8_t*)&payload, sizeof(payload));
    gTXSerial ++;
  }
};

#endif
